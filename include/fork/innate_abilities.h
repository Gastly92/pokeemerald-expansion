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
// SCOPE — innates are supported one ability at a time, via an explicit allowlist.
// Making an *arbitrary* ability work as an innate would require routing every
// "does this battler have ability X?" check through BattlerHasAbility() across
// hundreds of upstream-owned sites — a large, perpetually merge-conflict-prone
// sweep. Instead this fork wires up the innate behavior of one ability at a time
// and only allows species to declare innates from that supported set. Today the
// set is LEVITATE (a passive Ground immunity, handled inside src/battle_util.c),
// REGENERATOR (a silent 1/3-HP heal fired at the single switch-out site in
// src/battle_script_commands.c), UNAWARE (a passive calc modifier that ignores
// the foe's stat-stage changes, handled at the damage/accuracy calc sites in
// src/battle_util.c), and STURDY (endures a lethal hit at full HP + OHKO-move immunity,
// handled at the two effect sites in src/battle_util.c). NOTE: innates are intentionally a *pure boon* — never a 1:1 copy of the real
// ability when the real one carries a downside. An innate Levitate grants Ground / entry-hazard
// immunity like the real thing, but the fork also keeps the mon grounded for the beneficial ground
// interactions (field terrain, Toxic Spikes absorption) via IsBattlerGroundedForBenefit(); an innate
// Unaware ignores the foe's stat *boosts* but keeps the foe's stat *drops* (the favorable half) via
// InnateUnawareBoonStage(), where a real Unaware would ignore the drop too and take more damage for it.
// Where the real ability is already a clean upside (no downside to drop), the innate is a plain 1:1
// copy — Sturdy is such a case: an innate Sturdy endures/blocks OHKOs exactly like the real ability.
// See the ALLOWLIST note in src/innate_abilities.c. To add another ability: wire its specific effect
// (boon-only where the real ability has a downside), extend the allowlist comment in src/innate_abilities.c, and add a test.
// The step-by-step extension playbook lives in fork-docs/INNATE_ABILITIES.md.
//
// This header exposes only the raw data lookups (no battle/suppression logic).
// The battle-facing predicate that decides whether an innate is *currently
// active* on a battler — honoring Gastro Acid, Neutralizing Gas, Mold Breaker,
// Ability Shield, etc., exactly like a real ability — is BattlerHasAbility() /
// IsInnateActive() in battle_util.h / src/battle_util.c.

// TRUE if `species` declares `ability` as an innate. Pure data lookup: does not
// consider battle state or ability suppression. ABILITY_NONE never matches.
bool32 SpeciesHasInnate(u16 species, enum Ability ability);

// Returns the innate ability at 0-based `index` for `species`, or ABILITY_NONE if
// the slot is past the end / the species has no innates. Lets callers iterate a
// species' innates without needing to know how many it has.
enum Ability GetSpeciesInnate(u16 species, u32 index);

#endif // GUARD_INNATE_ABILITIES_H
