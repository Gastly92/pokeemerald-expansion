#include "global.h"
#include "config_changes.h"
#include "fork/frontier_draft.h"
#include "battle_frontier.h"
#include "data.h"
#include "item.h"
#include "pokemon.h"
#include "random.h"
#include "fork/species_tiers.h"
#include "constants/abilities.h"
#include "constants/form_change_types.h"
#include "constants/hold_effects.h"

// Shared Battle Frontier competitive-draft rules, moved here from the
// upstream src/battle_factory.c (see frontier_draft.h). Reused by the Battle
// Factory and the Battle Tower; only exercised under B_FRONTIER_EXTENDED_MONS.

// A generated team may hold at most one Mega Stone and at most one Z-Crystal (only
// one Mega Evolution / Z-Move is usable per battle). The plain same-item dup check
// doesn't catch two *different* mega stones or two *different* Z-crystals, so this
// guards that gap. Returns TRUE if adding newItem would give the team a second mon
// with the same gimmick class.
bool32 TeamHasGimmickItemConflict(const u16 *heldItems, u32 count, u16 newItem)
{
    enum HoldEffect newEffect = GetItemHoldEffect(newItem);
    u32 i;

    // With item-free gimmicks the held Mega Stone / Z-Crystal no longer gates
    // the gimmick (any mon may Mega/Z), so a team is no longer limited to one of
    // each. Balance is maintained through the species tier map instead.
    if (GetConfig(FEATURE_FREE_GIMMICKS))
        return FALSE;

    if (newEffect != HOLD_EFFECT_MEGA_STONE && newEffect != HOLD_EFFECT_Z_CRYSTAL)
        return FALSE;

    for (i = 0; i < count; i++)
    {
        if (heldItems[i] != ITEM_NONE && GetItemHoldEffect(heldItems[i]) == newEffect)
            return TRUE;
    }
    return FALSE;
}

// Species Clause by National Dex number. Upstream's draft loops compare exact
// species ids, which was the whole clause in vanilla Emerald (one id per Pokémon)
// but lets two formes with their own ids through — and the extended roster carries
// many (Silvally and Arceus have one set per type forme). The draft loops call
// these alongside upstream's exact-id checks; see fork-docs/FRONTIER_ROSTER.md.
bool32 SpeciesListHasDexNum(const enum Species *list, u32 count, enum Species species)
{
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(species);
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (SpeciesToNationalPokedexNum(list[i]) == dexNum)
            return TRUE;
    }
    return FALSE;
}

bool32 FactoryRentalsHaveDexNum(enum Species species)
{
    enum NationalDexOrder dexNum = SpeciesToNationalPokedexNum(species);
    u32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons); i++)
    {
        enum Species rental = gFacilityTrainerMons[gSaveBlock2Ptr->frontier.rentalMons[i].monId].species;
        if (SpeciesToNationalPokedexNum(rental) == dexNum)
            return TRUE;
    }
    return FALSE;
}

// Per-team tier quota, using the fork's species tier map (GetSpeciesTier /
// species_tiers.h). Each party slot has a "slot tier":
//   - TIER_NORMAL slot: ordinary draft pick — legendaries and mythicals are
//     banned, and at most ONE TIER_PSEUDO mon is allowed on the whole team.
//   - any other slot tier: a *forced* slot that must be filled by a mon of exactly
//     that tier (used to seed a set-milestone opponent with a legendary, or the
//     Frontier Brain with a legendary + a mythical).
// Returns TRUE if a candidate of `candTier` may NOT fill a `slotTier` slot given
// how many pseudos the team already holds — i.e. the candidate is rejected.
bool32 TierRejectsCandidate(enum SpeciesTier slotTier, enum SpeciesTier candTier, u32 pseudoCount)
{
    if (slotTier != TIER_NORMAL)
        return candTier != slotTier;

    if (candTier == TIER_LEGENDARY || candTier == TIER_MYTHICAL)
        return TRUE;
    if (candTier == TIER_PSEUDO && pseudoCount >= 1)
        return TRUE;
    return FALSE;
}

// Reserve one random, not-yet-reserved party slot in slotTiers[] (sized
// FRONTIER_PARTY_SIZE, pre-filled with TIER_NORMAL) for a forced tier, so the
// guaranteed legendary/mythical lands at a random position. Calling it twice with
// different tiers yields two distinct slots.
void ReserveForcedTierSlot(enum SpeciesTier *slotTiers, enum SpeciesTier tier)
{
    u32 slot;

    do
        slot = Random() % FRONTIER_PARTY_SIZE;
    while (slotTiers[slot] != TIER_NORMAL);
    slotTiers[slot] = tier;
}

// Illusion (Zoroark, and this fork's Eon duo) disguises its holder as the team's
// *last* conscious party member, so it has nothing to copy when it is itself the
// final filled slot — GetIllusionMonPartyId() bails in that case and the disguise
// never forms (see battle_util.c). Keep drafted Illusion mons out of the last
// slot: a draft loop rejects such a candidate there, so it stays eligible for
// every earlier slot and the disguise always has a mon behind it. We read
// fmon->ability directly, which is exactly what CreateFacilityMon assigns. The
// last slot is `partySize - 1` so this is correct for 3v3, 6v6, and the shorter
// teams of multi/doubles facilities alike. Returns TRUE if `fmon` must be
// rejected for `slot`.
bool32 IllusionMonRejectsSlot(u32 slot, u32 partySize, const struct TrainerMon *fmon)
{
    return slot == partySize - 1 && fmon->ability == ABILITY_ILLUSION;
}

// Finalize a freshly-built facility mon's max-gimmick readiness. Called once from
// CreateFacilityMon (src/battle_frontier.c) so it covers every facility that drafts
// through that chokepoint, while the logic stays in this fork-owned file (the only
// upstream edit is the single additive call line). Two grants, each still honoring
// an explicit per-roster-entry override:
//   - Dynamax Level: default to the maximum (MAX_DYNAMAX_LEVEL) so a mon that
//     Dynamaxes gets the full HP boost. Roster entries leave .dynamaxLevel 0, which
//     would otherwise mean no boost; a nonzero .dynamaxLevel still wins.
//   - Gigantamax Factor: granted to any species that has a G-Max form, so
//     gmax-capable mons Gigantamax instead of plain Dynamaxing with no per-entry
//     annotation; an explicit .gigantamaxFactor still forces it on.
//
// CreateFacilityMon first writes upstream's AI markers: BLOCK_AI_DYNAMAX as the
// Dynamax Level unless the entry sets .shouldUseDynamax, and TYPE_MYSTERY as the
// Tera Type unless it sets .teraType. Roster entries set neither, so with
// FEATURE_FREE_GIMMICKS (every AI mon may use every gimmick) both are undone here:
// a Mystery Tera Type would Terastallize into no type at all, and the blocked
// Dynamax Level would give no HP boost. With the feature off, upstream's markers
// stand, so the AI never Dynamaxes/Teras a set that did not ask for it.
void ApplyDraftGimmickReadiness(const struct TrainerMon *fmon, struct Pokemon *dst)
{
    if (fmon->shouldUseDynamax || GetConfig(FEATURE_FREE_GIMMICKS))
    {
        u32 dynamaxLevel = (fmon->dynamaxLevel > 0) ? fmon->dynamaxLevel : MAX_DYNAMAX_LEVEL;
        SetMonData(dst, MON_DATA_DYNAMAX_LEVEL, &dynamaxLevel);
    }

    if (!fmon->teraType && GetConfig(FEATURE_FREE_GIMMICKS))
    {
        // TYPE_NONE = "unmodified": GetMonData derives it from the personality.
        u32 teraType = TYPE_NONE;
        SetMonData(dst, MON_DATA_TERA_TYPE, &teraType);
    }

    if (fmon->gigantamaxFactor || DoesSpeciesHaveFormChangeMethod(fmon->species, FORM_CHANGE_BATTLE_GIGANTAMAX))
    {
        u32 gigantamaxFactor = TRUE;
        SetMonData(dst, MON_DATA_GIGANTAMAX_FACTOR, &gigantamaxFactor);
    }
}

// The Battle Factory swap screen (CopySwappedMonData, src/battle_factory_screen.c)
// zeroes the swapped-in mon's friendship, because upstream builds every Factory
// Return as Frustration. B_FRONTIER_PREFER_RETURN makes CreateFacilityMon build
// Return at max friendship instead, so a zeroed swap-in would carry a 1 BP Return
// until the party is next rebuilt (only on resuming a saved challenge). Put it back
// to MAX_FRIENDSHIP, matching what CreateFacilityMon gives every other facility mon.
void ApplySwappedMonFriendship(struct Pokemon *mon)
{
#if B_FRONTIER_PREFER_RETURN
    u32 friendship = MAX_FRIENDSHIP;
    SetMonData(mon, MON_DATA_FRIENDSHIP, &friendship);
#else
    (void)mon;
#endif
}

void ApplyFrontierMaxPP(struct Pokemon *mon)
{
#if B_FRONTIER_MAX_PP
    // SetMonMoveSlot assigns base PP, so afterwards max the PP Up bonus on every
    // slot (gPPUpGetMask[j] holds the 3-PP-Up mask for slot j; OR'd together that's
    // full bonus on all four) and recompute each move's PP.
    u32 j;
    u8 ppBonuses = 0;
    for (j = 0; j < MAX_MON_MOVES; j++)
        ppBonuses |= gPPUpGetMask[j];
    SetMonData(mon, MON_DATA_PP_BONUSES, &ppBonuses);
    for (j = 0; j < MAX_MON_MOVES; j++)
    {
        u8 pp = CalculatePPWithBonus(GetMonData(mon, MON_DATA_MOVE1 + j), ppBonuses, j);
        SetMonData(mon, MON_DATA_PP1 + j, &pp);
    }
#else
    (void)mon;
#endif
}

// Upstream's CreateRentalMon rebuilds the Factory parties from the save (rentalMons:
// monId, personality, abilityNum, ivs) without going through CreateFacilityMon, so
// none of that function's fork edits run there. Without this the rebuilt team would
// carry Frustration at 0 friendship, base PP, uniform IVs (a 0-Speed Trick Room set
// rebuilt at its saved Atk IV) and no free-gimmick readiness.
void ApplyRentalMonForkExtras(const struct TrainerMon *fmon, struct Pokemon *dst)
{
#if B_FRONTIER_PREFER_RETURN
    for (u32 j = 0; j < MAX_MON_MOVES; j++)
    {
        if (GetMonData(dst, MON_DATA_MOVE1 + j) == MOVE_FRUSTRATION)
            SetMonMoveSlot(dst, MOVE_RETURN, j);
    }
    ApplySwappedMonFriendship(dst);
#endif
    ApplyFrontierMaxPP(dst);

    if (fmon->iv)
        SetMonData(dst, MON_DATA_IVS, &(fmon->iv));

    if (fmon->ball == BALL_STRANGE)
    {
        u32 ball = BALL_POKE;
        SetMonData(dst, MON_DATA_POKEBALL, &ball);
    }

    ApplyDraftGimmickReadiness(fmon, dst);
    CalculateMonStats(dst);
}
