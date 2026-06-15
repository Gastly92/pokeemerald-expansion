#ifndef GUARD_SPECIES_ABILITY_OVERRIDES_H
#define GUARD_SPECIES_ABILITY_OVERRIDES_H

// FORK: fork-owned species ability overrides (sibling to innate_abilities.h).
//
// Replaces what gSpeciesInfo[species].abilities[slot] reports for a handful of
// species, WITHOUT editing the upstream species data. The override is consulted
// by GetSpeciesAbility() (src/pokemon.c), the single accessor every ability read
// funnels through, so an overridden ability behaves like a real one everywhere
// (battle, summary screen, AI, the Battle Factory's ability picker).
//
// WHY: several Battle Factory sets (src/frontier_extended_mons.c) carry an innate
// Levitate/Regenerator (FEATURE_INNATE_ABILITIES) yet want a *second*, chosen
// ability on top of it — e.g. a Rotom that floats on its innate Levitate AND runs
// Lightning Rod. Those species are "ability-locked" in the upstream data (their
// only real ability is the one now provided innately), so the chosen ability has
// no slot to live in and CreateFacilityMon would silently fall back to slot 0.
// This table gives that chosen ability a real slot, so the set runs both. Keeping
// the data here (not in gSpeciesInfo) means upstream syncs never touch it.
//
// Returns the override ability for (species, slot), or ABILITY_NONE if there is
// none (in which case the caller uses the upstream gSpeciesInfo value).
enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot);

#endif // GUARD_SPECIES_ABILITY_OVERRIDES_H
