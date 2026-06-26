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
// src/battle_util.c), STURDY (endures a lethal hit at full HP + OHKO-move immunity,
// handled at the two effect sites in src/battle_util.c), NATURAL_CURE (silently
// cures the holder's status on switch-out, fired at the single switch-out site in
// src/battle_script_commands.c like Regenerator), PRANKSTER (gives the holder's
// status moves +1 priority, handled at the single move-priority site in src/battle_main.c),
// the pinch abilities OVERGROW / BLAZE / TORRENT / SWARM (+50% to the matching move type at
// low HP, handled in CalcAttackStat in src/battle_util.c, with a latch so the boost persists once
// reached — see the ALLOWLIST note in src/fork/innate_abilities.c), and the weather speed-doublers
// SWIFT_SWIM / CHLOROPHYLL / SAND_RUSH / SLUSH_RUSH (x2 Speed in rain / sun / sandstorm / snow, handled
// at the GetBattlerTotalSpeedStat calc site in src/battle_main.c; Sand Rush also shrugs off sandstorm
// damage, mirrored in src/battle_end_turn.c and the AI's sandstorm-damage predictors), and FILTER
// (reduces supereffective damage taken by 25%, handled at the GetDefenderAbilitiesModifier calc site
// in src/battle_util.c — a clean-upside 1:1 copy like Sturdy), and PRESSURE (the holder's foes spend
// 1 extra PP per move used against it, handled at the two PP-deduction sites in
// src/battle_move_resolution.c and src/battle_util.c — a clean-upside 1:1 copy with no AI wiring needed),
// and STENCH (a 10% on-hit flinch — guaranteed first-turn under DETERMINISTIC_ABILITIES — handled at the
// ABILITYEFFECT_MOVE_END_ATTACKER on-hit site in src/battle_util.c, run additively beside the chosen-ability
// dispatch so an innate holder flinches like the real ability; a clean-upside 1:1 copy with no AI wiring needed),
// and SPEED_BOOST (+1 Speed at the end of every turn — the fork's first ACTIVE, scripted end-turn innate, so
// it needs the end-turn driver TryActivateInnateEndTurnEffects, hooked from THIRD_EVENT_BLOCK_ABILITIES_INNATE
// in src/battle_end_turn.c; it delegates to the upstream ABILITYEFFECT_ENDTURN handler so the stat change /
// script / pop-up match the real ability — a clean-upside 1:1 copy, AI made innate-aware only at the two foe
// "don't lower its Speed" reads in src/battle_ai_util.c and src/battle_stat_change.c),
// and LIMBER (the holder cannot be paralyzed, handled at the paralysis block site in
// CanSetNonVolatileStatus and the switch-in cure site in TryImmunityAbilityHealStatus in
// src/battle_util.c, plus the out-of-battle Battle Pike status room in src/battle_pike.c — a
// clean-upside 1:1 copy; AI is correct for free since its paralysis checks funnel through
// CanSetNonVolatileStatus, whose fork clause reads the real battler),
// and CUTE_CHARM (a 30% chance to infatuate an opposite-gender attacker on contact — guaranteed
// regardless of gender under DETERMINISTIC_ABILITIES — handled at the ABILITYEFFECT_MOVE_END on-hit
// site in src/battle_util.c, run additively beside the chosen-ability dispatch via TryCuteCharmInfatuate
// so an innate holder infatuates like the real ability and the pop-up is overwritten to Cute Charm; a
// clean-upside 1:1 copy, AI made innate-aware only at the DETERMINISTIC_ABILITIES contact-punish
// predictor in src/battle_ai_util.c),
// and OBLIVIOUS (the holder cannot be infatuated or Taunted (GEN_6+) and is unaffected by Intimidate (GEN_8+),
// a passive trait wired innate-aware at the scattered immunity sites in src/battle_script_commands.c and
// src/battle_stat_change.c and src/battle_util.c — the infatuation/Taunt/Captivate/Intimidate blocks and the
// switch-in cure, each overwriting the pop-up to Oblivious; a clean-upside 1:1 copy, AI made innate-aware at the
// foe Attract/Intimidate/Cute-Charm reads in src/battle_ai_util.c and src/battle_ai_switch.c),
// and the weather evasion abilities SAND_VEIL / SNOW_CLOAK (+25% evasion in sandstorm / hail-or-snow plus
// immunity to that weather's chip damage, passive calc modifiers handled at the GetTotalAccuracy accuracy
// site in src/battle_util.c with the chip immunity mirrored in src/battle_end_turn.c — clean-upside 1:1
// copies; on-field accuracy prediction is innate-aware for free, with the AI's weather-damage and
// weather-setting reads in src/battle_ai_util.c / src/battle_ai_switch.c / src/battle_ai_field_statuses.c
// credited too),
// and the accuracy abilities COMPOUND_EYES / KEEN_EYE / ILLUMINATE (the fork models all three as "ignore the
// target's evasion" — Compound Eyes' real +30% accuracy is repurposed to match Keen Eye / Illuminate — which
// under DETERMINISTIC_ACCURACY_EVASION is a PP-economy boon; wired at GetTotalAccuracy and
// GetAccEvasionStageDelta in src/battle_util.c as a PURE BOON, ignoring only a foe's evasion *boost* and
// keeping its *drop*, like InnateUnawareBoonStage. Keen Eye / Illuminate ALSO keep the holder's own accuracy
// from being lowered, wired at IsAbilityBlocked in src/battle_stat_change.c with the pop-up overwritten to the
// innate. Illuminate's in-battle effect is gated to B_ILLUMINATE_EFFECT >= GEN_9 like the real ability, and the
// AI's "don't lower its accuracy" read in src/battle_ai_util.c is credited too),
// and the sleep-immunity abilities INSOMNIA / VITAL_SPIRIT / SWEET_VEIL (the holder — and, for Sweet Veil,
// its whole side — cannot be put to sleep or made drowsy), all wired at the single MOVE_EFFECT_SLEEP
// chokepoint in CanSetNonVolatileStatus (src/battle_util.c) that every sleep path funnels through, plus the
// end-turn drowsy->sleep site (src/battle_end_turn.c) and the out-of-battle Battle Pike sleep room
// (src/battle_pike.c). PURE-BOON DIVERGENCE: unlike the real ability, an innate one does NOT block the
// holder's own Rest (Rest heals + sleeps normally); the switch-in sleep-cure is intentionally dropped so the
// post-move cure hook can't un-sleep a fresh Rest into a free Recover. AI is innate-aware for free via the
// shared CanBeSlept chokepoint. Sweet Veil's side-wide check uses the new fork helper IsInnateOnSide(),
// and EARLY_BIRD (the holder wakes from sleep twice as fast, a clean-upside 1:1 copy wired at the two
// sleep-counter sites in src/battle_move_resolution.c and src/battle_util2.c, with the AI's three Early Bird
// reads in src/battle_ai_util.c / src/battle_ai_main.c / src/battle_ai_switch.c made innate-aware), and
// the poison-immunity abilities IMMUNITY / PASTEL_VEIL (the holder — and, for Pastel Veil, its whole
// side — cannot be poisoned or badly poisoned), wired at the MOVE_EFFECT_POISON / MOVE_EFFECT_TOXIC
// chokepoint in CanSetNonVolatileStatus (src/battle_util.c), the switch-in self-cure in
// TryImmunityAbilityHealStatus (src/battle_util.c), the out-of-battle Battle Pike poison room
// (src/battle_pike.c), and the AI's Toxic Spikes switch-in damage prediction (src/battle_ai_switch.c);
// a clean-upside 1:1 copy of the real ability, with the real Pastel Veil's switch-in ally-cure not
// replicated for an innate holder (see the ALLOWLIST note in src/fork/innate_abilities.c).
// NOTE: innates are intentionally a *pure boon* — never a 1:1 copy of the real
// ability when the real one carries a downside. An innate Levitate grants Ground / entry-hazard
// immunity like the real thing, but the fork also keeps the mon grounded for the beneficial ground
// interactions (field terrain, Toxic Spikes absorption) via IsBattlerGroundedForBenefit(); an innate
// Unaware ignores the foe's stat *boosts* but keeps the foe's stat *drops* (the favorable half) via
// InnateUnawareBoonStage(), where a real Unaware would ignore the drop too and take more damage for it.
// Where the real ability is already a clean upside (no downside to drop), the innate is a plain 1:1
// copy — Sturdy and Natural Cure are such cases: an innate Sturdy endures/blocks OHKOs, and an innate
// Natural Cure cures status on switch-out, each exactly like the real ability. An innate Prankster
// keeps the +1 status-move priority but drops the real ability's Dark-type immunity cost (its boosted
// status moves still land on Dark-types), so it too is a pure boon, not a 1:1 copy.
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

#endif // GUARD_INNATE_ABILITIES_H
