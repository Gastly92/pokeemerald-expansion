#ifndef GUARD_INNATE_ABILITIES_H
#define GUARD_INNATE_ABILITIES_H

// FORK: innate abilities (FEATURE_INNATE_ABILITIES, config/feature.h).
//
// Some species carry one or more *innate* abilities that are always active in
// addition to their single chosen ability. The species->innate mapping lives in
// a fork-owned table (src/innate_abilities.c) rather than in gSpeciesInfo, so it
// never conflicts on upstream sync and leaves the upstream species data
// untouched.
//
// This header exposes only the raw data lookups (no battle/suppression logic).
// The battle-facing predicate that decides whether an innate is *currently
// active* on a battler — honoring Gastro Acid, Neutralizing Gas, Mold Breaker,
// Ability Shield, etc., exactly like a real ability — is BattlerHasAbility() /
// IsInnateActive() in battle_util.h / src/battle_util.c.

// Maximum number of innate abilities a single species may declare.
#define MAX_INNATE_ABILITIES 3

// TRUE if `species` declares `ability` as an innate. Pure data lookup: does not
// consider battle state or ability suppression. ABILITY_NONE never matches.
bool32 SpeciesHasInnate(u16 species, enum Ability ability);

// Returns the innate ability in slot `index` (0..MAX_INNATE_ABILITIES-1) for
// `species`, or ABILITY_NONE if the slot is empty / out of range. Lets callers
// iterate a species' innates (summary screen, tests, etc.).
enum Ability GetSpeciesInnate(u16 species, u32 index);

#endif // GUARD_INNATE_ABILITIES_H
