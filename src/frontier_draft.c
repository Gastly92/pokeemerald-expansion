#include "global.h"
#include "frontier_draft.h"
#include "item.h"
#include "random.h"
#include "species_tiers.h"
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
