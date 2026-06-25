#include "global.h"
#include "config_changes.h"
#include "fork/frontier_draft.h"
#include "data.h"
#include "item.h"
#include "pokemon.h"
#include "random.h"
#include "fork/species_tiers.h"
#include "constants/abilities.h"
#include "constants/form_change_types.h"
#include "constants/hold_effects.h"

// FORK: shared Battle Frontier competitive-draft rules, moved here from the
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

    // FORK: with item-free gimmicks the held Mega Stone / Z-Crystal no longer gates
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

// Per-team tier quota, using the fork's species tier map (GetSpeciesTier /
// species_tiers.h). Each party slot has a "slot tier":
//   - TIER_NORMAL slot: ordinary draft pick — legendaries and mythicals are
//     banned, and at most ONE pseudo (pseudo-legendary / Ultra Beast / Paradox /
//     Treasure of Ruin) is allowed on the whole team.
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
void ApplyDraftGimmickReadiness(const struct TrainerMon *fmon, struct Pokemon *dst)
{
    u32 dynamaxLevel = (fmon->dynamaxLevel > 0) ? fmon->dynamaxLevel : MAX_DYNAMAX_LEVEL;
    SetMonData(dst, MON_DATA_DYNAMAX_LEVEL, &dynamaxLevel);

    if (fmon->gigantamaxFactor || DoesSpeciesHaveFormChangeMethod(fmon->species, FORM_CHANGE_BATTLE_GIGANTAMAX))
    {
        u32 gigantamaxFactor = TRUE;
        SetMonData(dst, MON_DATA_GIGANTAMAX_FACTOR, &gigantamaxFactor);
    }
}
