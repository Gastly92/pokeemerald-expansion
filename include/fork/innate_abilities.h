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
// supported set is:
//   LEVITATE, REGENERATOR, UNAWARE, STURDY, NATURAL_CURE, PRANKSTER,
//   OVERGROW / BLAZE / TORRENT / SWARM (pinch), SWIFT_SWIM / CHLOROPHYLL /
//   SAND_RUSH / SLUSH_RUSH (weather speed), FILTER, PRESSURE, STENCH,
//   BATTLE_ARMOR / SHELL_ARMOR, SPEED_BOOST, LIMBER, CUTE_CHARM, OBLIVIOUS,
//   SAND_VEIL / SNOW_CLOAK, COMPOUND_EYES / KEEN_EYE / ILLUMINATE,
//   INSOMNIA / VITAL_SPIRIT / SWEET_VEIL, EARLY_BIRD, IMMUNITY / PASTEL_VEIL,
//   THICK_FAT, TECHNICIAN,
//   IRON_FIST / RECKLESS / STRONG_JAW / TOUGH_CLAWS / SHARPNESS / MEGA_LAUNCHER /
//   STEELWORKER / STEELY_SPIRIT / ROCKY_PAYLOAD / SAND_FORCE / ANALYTIC /
//   ADAPTABILITY / PUNK_ROCK / STAKEOUT (offensive move-power boosters, Batch A),
//   SERENE_GRACE (doubles the holder's moves' additional-effect chances),
//   MULTISCALE / SOLID_ROCK / FUR_COAT / ICE_SCALES / HEATPROOF / FRIEND_GUARD /
//   WATER_BUBBLE (defensive damage reducers, Batch B),
//   GUTS / MARVEL_SCALE / QUICK_FEET / TOXIC_BOOST / FLARE_BOOST (status-conditional
//   stat boosts, Batch N),
//   SUPER_LUCK / SNIPER / MERCILESS (crit-rate / crit-damage modifiers, Batch O),
//   SHIELD_DUST / TINTED_LENS / SCRAPPY / WONDER_SKIN / TANGLED_FEET (accuracy /
//   type-effectiveness / effect-chance modifiers, Batch P),
//   GALE_WINGS / TRIAGE (priority granters, Batch Q),
//   SURGE_SURFER / GRASS_PELT (terrain modifiers, Batch R),
//   HUGE_POWER / PURE_POWER (double physical Attack, Batch C),
//   CLEAR_BODY / WHITE_SMOKE / HYPER_CUTTER / BIG_PECKS (stat-drop protection, Batch D+E),
//   DAZZLING / QUEENLY_MAJESTY / ARMOR_TAIL (priority-move block, Batch F),
//   PROPELLER_TAIL / STALWART (redirection-ignore, Batch G),
//   SHADOW_TAG / ARENA_TRAP / MAGNET_PULL (trapping, Batch H),
//   MAGMA_ARMOR / WATER_VEIL / OWN_TEMPO / INNER_FOCUS / LEAF_GUARD / OVERCOAT
//   (status-condition immunities, Batch I),
//   SUCTION_CUPS / GUARD_DOG / ROCK_HEAD / LONG_REACH / SKILL_LINK / INFILTRATOR /
//   CORROSION / STICKY_HOLD / UNSEEN_FIST / PIERCING_DRILL / HEAVY_METAL / LIGHT_METAL
//   (miscellaneous single-site traits, Batch S),
//   RAIN_DISH / ICE_BODY / SHED_SKIN / HYDRATION / HEALER / HARVEST / CUD_CHEW / PICKUP /
//   BAD_DREAMS / POISON_HEAL (end-of-turn effects, Batch J — first nine reuse the Speed
//   Boost end-turn driver; Poison Heal replaces the poison-damage step),
//   GLUTTONY / RIPEN / CHEEK_POUCH / UNBURDEN (berry/item synergy, Batch T — Gluttony's
//   1/2-HP pinch-Berry threshold, Ripen's doubled Berry effects, Cheek Pouch's heal on
//   Berry eat, Unburden's doubled Speed after item loss),
//   ROUGH_SKIN / IRON_BARBS / GOOEY / TANGLING_HAIR (on-hit contact reactions, Batch K first
//   sub-group — the first active ON-HIT innates, fired through a new re-entrant on-hit driver
//   modeled on the Speed Boost end-turn driver: Rough Skin / Iron Barbs chip a contact attacker
//   1/8 max HP, Gooey / Tangling Hair lower a contact attacker's Speed).
//
// NOTE: innates are intentionally a *pure boon* — never a 1:1 copy of the real
// ability when the real one carries a downside. E.g. an innate Levitate grants Ground /
// entry-hazard immunity but the fork keeps the mon grounded for the beneficial ground
// interactions (field terrain, Toxic Spikes absorption); an innate Unaware ignores the
// foe's stat *boosts* but keeps the foe's *drops* (the favorable half); an innate
// Prankster keeps +1 status-move priority but drops the real ability's Dark-type immunity
// cost. Where the real ability is already a clean upside (Sturdy, Natural Cure, ...), the
// innate is a plain 1:1 copy. Suppression parity (Gastro Acid / Neutralizing Gas / Mold
// Breaker / Ability Shield / not-on-field) ALWAYS matches the real ability via
// IsInnateActive(), regardless of any effect divergence.
//
// The exact per-ability semantics — effect sites, the pure-boon divergences, the AI
// wiring, and the species-selection rationale — plus the step-by-step extension playbook
// live in fork-docs/INNATE_ABILITIES.md (the "Per-ability wiring reference" appendix; grep
// it for `### ABILITY_NAME`). To add another ability: wire its specific effect (boon-only
// where the real ability has a downside), add its species rows + reference block, extend
// the compact allowlist in src/fork/innate_abilities.c and this list, and add a test.
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

// Raw-table accessors for table-integrity tests ONLY (test/fork/innate_abilities.c):
// they walk the underlying rows by position, so a duplicate species row (which the
// species-keyed lookups above hide) stays observable. GetSpeciesInnatesEntryCount() is
// the number of rows; GetSpeciesInnatesEntry() returns row `row`'s ABILITY_NONE-
// terminated innate list and writes its species to *outSpecies (NULL if out of range).
// Battle/AI code must use SpeciesHasInnate / GetSpeciesInnate, never these.
u32 GetSpeciesInnatesEntryCount(void);
const enum Ability *GetSpeciesInnatesEntry(u32 row, u16 *outSpecies);

// FORK: end-turn innate driver. Fires `battler`'s active, scripted end-turn innates
// (today only Speed Boost: +1 Speed). Hooked from THIRD_EVENT_BLOCK_ABILITIES_INNATE
// in the end-turn loop (src/battle_end_turn.c), right after the chosen-ability block.
// Re-entrant: *index is the per-battler resume cursor into the innate list — fires one
// effect per call (leaving *index past it) and returns TRUE, or returns FALSE once the
// list is exhausted, so a battler with several end-turn innates fires them across passes.
// See the definition in src/fork/innate_abilities.c for the suppression/double-fire guards.
bool32 TryActivateInnateEndTurnEffects(enum BattlerId battler, u32 *index);

// FORK: on-hit innate driver. Fires `battler`'s active, scripted on-hit innates — the
// contact-reaction class: Rough Skin / Iron Barbs (chip a contact attacker) and Gooey /
// Tangling Hair (drop a contact attacker's Speed). Hooked from MOVEEND_ABILITIES_INNATE in
// the move-end loop (src/battle_move_resolution.c), right after the chosen-ability contact
// block. `battler` is the holder that was hit (gBattlerTarget); `move` is the move that hit it.
// Re-entrant: *index is the per-battler resume cursor into the innate list — fires one effect
// per call (leaving *index past it) and returns TRUE, or returns FALSE once the list is
// exhausted, so a battler with several on-hit innates fires them across passes. See the
// definition in src/fork/innate_abilities.c for the suppression/double-fire guards.
bool32 TryActivateInnateOnHitEffects(enum BattlerId battler, u32 *index, enum Move move);

#endif // GUARD_INNATE_ABILITIES_H
