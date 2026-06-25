#ifndef GUARD_FRONTIER_DRAFT_H
#define GUARD_FRONTIER_DRAFT_H

#include "fork/species_tiers.h"

// FORK: shared Battle Frontier competitive-draft rules. Extracted out of the
// upstream src/battle_factory.c into this fork-owned file so the rules carry no
// upstream merge-conflict surface and can be reused by any facility that drafts
// teams from the extended roster (the Battle Factory and the Battle Tower). See
// src/frontier_draft.c. Used under B_FRONTIER_EXTENDED_MONS.
struct TrainerMon;

bool32 TeamHasGimmickItemConflict(const u16 *heldItems, u32 count, u16 newItem);
bool32 TierRejectsCandidate(enum SpeciesTier slotTier, enum SpeciesTier candTier, u32 pseudoCount);
void ReserveForcedTierSlot(enum SpeciesTier *slotTiers, enum SpeciesTier tier);
// Reject an Illusion-ability candidate from the team's last party slot (where the
// disguise can't form); harmless for non-Illusion mons / non-last slots.
bool32 IllusionMonRejectsSlot(u32 slot, u32 partySize, const struct TrainerMon *fmon);
// Grant a freshly-drafted facility mon its max-gimmick readiness (maximum Dynamax
// Level by default, plus the Gigantamax Factor for gmax-capable species). Called
// from CreateFacilityMon; each grant honors an explicit per-entry override.
struct Pokemon;
void ApplyDraftGimmickReadiness(const struct TrainerMon *fmon, struct Pokemon *dst);

#endif // GUARD_FRONTIER_DRAFT_H
