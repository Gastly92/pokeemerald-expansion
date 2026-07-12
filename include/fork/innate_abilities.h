#ifndef GUARD_INNATE_ABILITIES_H
#define GUARD_INNATE_ABILITIES_H

#include "battle_util.h" // FORK: enum AbilityEffect (TryActivateInnateSwitchInEffects' phase selector)

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
//   ROUGH_SKIN / IRON_BARBS / GOOEY / TANGLING_HAIR (on-hit contact reactions) + AFTERMATH /
//   INNARDS_OUT (on-faint retaliation) (Batch K — active ON-HIT innates fired through a new
//   re-entrant on-hit driver modeled on the Speed Boost end-turn driver: Rough Skin / Iron Barbs
//   chip a contact attacker 1/8 max HP, Gooey / Tangling Hair lower a contact attacker's Speed,
//   Aftermath chips a contact attacker 1/4 max HP when it KOs the holder, Innards Out deals the
//   attacker the holder's lost HP when any move KOs it; STEAM_ENGINE / THERMAL_EXCHANGE / WIND_POWER
//   raise Speed / Attack / charge the next Electric move when the holder is hit by a Fire-or-Water /
//   Fire / wind move, Thermal Exchange also granting burn immunity; CURSED_BODY disables the move
//   that hit the holder, 30% / always under DETERMINISTIC_ABILITIES;
//   PICKPOCKET / MAGICIAN / LIQUID_OOZE (Batch K fifth/final sub-group — Pickpocket steals a contact
//   attacker's held item, wired at the dedicated MoveEndPickpocket step; Magician steals a held item off
//   a target the holder damages, via a new ATTACKER-side on-hit driver hooked at
//   MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE; Liquid Ooze is a passive calc modifier that makes
//   HP-draining moves damage the attacker instead of healing it),
//   INTIMIDATE (switch-in Attack drop, Batch L first sub-group — the FIRST active SWITCH-IN innate,
//   fired through a new re-entrant switch-in driver (TryActivateInnateSwitchInEffects) hooked at the new
//   FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE switch-in step, delegating to the upstream
//   ABILITYEFFECT_ON_SWITCHIN case so the Attack drop / script / pop-up match the real ability),
//   ANTICIPATION / FOREWARN / FRISK (switch-in information reveals, Batch L second sub-group — 1:1
//   clean-upside copies that reuse the same switch-in driver; each shows a switch-in message and, for
//   Frisk/Forewarn, reveals a foe's item/move — no AI wiring needed, no pure-boon divergence),
//   DOWNLOAD / SUPERSWEET_SYRUP (switch-in stat changes, Batch L third sub-group — 1:1 clean-upside
//   copies that reuse the same switch-in driver; Download raises the holder's Attack or Sp. Atk toward
//   the foe's weaker defense, Supersweet Syrup lowers every foe's evasiveness once per battle, with the
//   AI's switch-in stat simulation made innate-aware),
//   UNNERVE / HOSPITALITY (switch-in effects, Batch L fourth/final sub-group — 1:1 clean-upside copies
//   that reuse the same switch-in driver at their own switch-in phases; Unnerve denies every opposing
//   battler its Berries (fired through the ABILITYEFFECT_UNNERVE phase, with the AI's Berry-heal read
//   made innate-aware), Hospitality heals the ally 1/4 max HP in doubles (fired through the
//   ABILITYEFFECT_DEPENDS_ON_ALLY phase — no AI wiring needed, no pure-boon divergence)),
//   DEFIANT / COMPETITIVE (stat-drop reactions, Batch M first sub-group — 1:1 clean-upside copies wired
//   at the single scripted reaction site BS_TryDefiantRattled: when a foe lowers one of the holder's stats
//   (a move, Intimidate, or Sticky Web), Defiant raises its Attack and Competitive its Sp. Atk by 2 stages,
//   with the pop-up overwritten to the innate and the AI's don't-lower-a-reactive-foe heuristics made
//   innate-aware; innate Rattled, which also reacts through that site but only to Intimidate, is a later
//   sub-group),
//   JUSTIFIED / STAMINA / WATER_COMPACTION / ANGER_POINT (on-hit stat boosts, Batch M second sub-group —
//   1:1 clean-upside copies that reuse the existing on-hit driver: when the holder is hit, Justified raises
//   Attack +1 on a Dark move, Stamina raises Defense +1 on any move, Water Compaction raises Defense +2 on a
//   Water move, Anger Point maxes Attack on a critical hit — each delegating to the upstream
//   ABILITYEFFECT_MOVE_END case, with the pop-up overwritten to the innate and the AI's partner-fire /
//   avoid-boosting heuristics made innate-aware),
//   RATTLED / STEADFAST (fear-response Speed boosts, Batch M third sub-group — both 1:1 clean-upside copies:
//   the holder's Speed rises +1 when frightened. Rattled reacts to a Dark/Ghost/Bug hit (reusing the on-hit
//   driver) and to Intimidate (credited at BS_TryDefiantRattled beside Defiant/Competitive, Gen8+ only);
//   Steadfast reacts to flinching (made innate-aware at the CancelerFlinch site). Pop-up overwritten to the
//   innate; the Rattled AI reads (avoid-a-Dark/Ghost/Bug-hit, doubles partner-fire, Intimidate-cycling switch)
//   are innate-aware, Steadfast needs none. Dropped on the Riolu/Lucario line as a contradiction — their innate
//   Inner Focus prevents flinching, so an innate Steadfast could never fire),
//   MOXIE / BERSERK / SOUL_HEART (KO / on-damage / on-faint stat boosts, Batch M fourth/final sub-group — all
//   1:1 clean-upside copies: Moxie raises Attack +1 per foe it knocks out (a one-line addition to the
//   attacker-side on-hit driver, ABILITYEFFECT_MOVE_END_FOES_FAINTED); Berserk raises Sp. Atk +1 when an attack
//   drops the holder's HP to 1/2 or less (a small NEW on-damage driver hooked at MOVEEND_COLOR_CHANGE_INNATE,
//   delegating to ABILITYEFFECT_COLOR_CHANGE); Soul-Heart raises Sp. Atk +1 whenever any Pokémon faints (made
//   innate-aware at the BS_TryActivateSoulheart native command). Pop-up overwritten to the innate; Moxie's two
//   AI reads are innate-aware, Berserk/Soul-Heart need none. Completes Batch M),
//   BATTERY / POWER_SPOT / TELEPATHY / AROMA_VEIL / FLOWER_VEIL (ally-support, Batch U — all 1:1 clean-upside
//   copies, canon-only: Battery / Power Spot boost an ally's (special / all) moves +30% (calc modifiers beside
//   partner Steely Spirit, AI-free); Telepathy nullifies an ally's damaging move (type-eff calc); Aroma Veil
//   shields the side from mental status — infatuation / Taunt / Disable / Encore / Torment / Heal Block — wired via
//   the new IsInnateOnSide() companion at the C guards + centrally in Cmd_jumpifability's side cases (the only
//   script jumpifability form, Aroma-Veil-only); Flower Veil shields Grass allies from non-volatile status AND stat
//   drops (IsFlowerVeilProtected / StatChange_IsFlowerVeilProtected made innate-aware). Pop-up overwritten to the
//   innate; the dedicated AI side reads are innate-aware via AI_IsInnateOnSide. Completes Batch U),
//   CHILLING_NEIGH / GRIM_NEIGH / ELECTROMORPHOSIS (promoted-from-rejected clones, Batch Y sub-group Y1 — all
//   1:1 clean-upside copies, canon-only: Chilling Neigh / Grim Neigh raise the holder's Attack / Sp. Atk +1 per
//   foe it KOs — Moxie clones, one-line additions to the attacker-side on-hit driver reusing the shared
//   ABILITYEFFECT_MOVE_END_FOES_FAINTED case; Electromorphosis charges the next Electric move when hit by any
//   damaging move — a Wind Power clone minus the wind gate, a one-line addition to the target-side on-hit driver
//   reusing the shared ABILITYEFFECT_MOVE_END case. Moxie-type AI reads credit an innate Chilling/Grim Neigh via
//   IsMoxieTypeInnateActive(); Electromorphosis needs no AI wiring. Sole-ability legends Glastrier / Spectrier
//   take the innate + a fork override; Bellibolt (Static/Damp) leaves it observable),
//   TRANSISTOR / DRAGONS_MAW (promoted-from-rejected clones, Batch Y sub-group Y2 — both 1:1 clean-upside
//   copies, canon-only: flat type-power-booster clones of Steelworker / Rocky Payload (Batch A) wired as two
//   lines in CalcAttackStat — Transistor boosts the holder's Electric moves (x1.3 GEN_9+, else x1.5), Dragon's
//   Maw its Dragon moves x1.5. AI-free (shared damage calc). Sole-ability Regi legends: Transistor -> Regieleki,
//   Dragon's Maw -> Regidrago, each taking the innate + a fork chosen override (Lightning Rod / Adaptability) so
//   the innate is observable and the frontier set is freed),
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

// FORK: attacker-side on-hit innate driver. Fires `battler`'s active, scripted attacker-side
// on-hit innates — today only Magician (steal a held item off a target the holder damaged).
// Hooked from MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE in the move-end loop
// (src/battle_move_resolution.c), right after the chosen-ability foes-fainted block. `battler` is
// the attacker (gBattlerAttacker); `move` is the move it used. Re-entrant exactly like
// TryActivateInnateOnHitEffects: *index is the per-battler resume cursor, firing one effect per
// call and returning TRUE, or FALSE once the list is exhausted. Delegates to the upstream
// ABILITYEFFECT_MOVE_END_FOES_FAINTED case so the steal / script / pop-up match the real ability.
bool32 TryActivateInnateOnHitAttackerEffects(enum BattlerId battler, u32 *index, enum Move move);

// FORK: on-damage innate driver. Fires `battler`'s active, scripted on-damage innates — today only
// Berserk (raises Sp. Atk +1 when an attack drops the holder's HP to 1/2 or less). Hooked from
// MOVEEND_COLOR_CHANGE_INNATE in the move-end loop (src/battle_move_resolution.c), right after the
// chosen-ability color-change block, which the caller iterates over every damaged battler. Re-entrant
// exactly like TryActivateInnateOnHitEffects: *index is the per-battler resume cursor, firing one
// effect per call and returning TRUE, or FALSE once the list is exhausted. Delegates to the upstream
// ABILITYEFFECT_COLOR_CHANGE case so the stat change / script / pop-up match the real ability.
bool32 TryActivateInnateOnDamageEffects(enum BattlerId battler, u32 *index);

// FORK: switch-in innate driver. Fires `battler`'s active, scripted switch-in innates — Intimidate
// (lowers every opposing battler's Attack by 1 stage), the Anticipation / Forewarn / Frisk information
// reveals, Download / Supersweet Syrup (switch-in stat changes), Unnerve (denies foes their Berries) and
// Hospitality (heals the ally in doubles). Hooked from three switch-in phases (src/battle_switch_in.c),
// each right after its chosen-ability counterpart; `abilityEffect` selects which phase this call handles,
// so each innate fires at the same point the real ability would. `shouldTrigger` mirrors the
// chosen-ability call's switch-in gate. Re-entrant: *index is the per-battler resume cursor into the
// innate list — fires one effect per call (leaving *index past it) and returns TRUE, or returns FALSE once
// the list is exhausted, so a battler with several switch-in innates for one phase fires them across
// passes. Delegates to the upstream switch-in case for `abilityEffect` so the stat change / heal / script
// / pop-up match the real ability.
bool32 TryActivateInnateSwitchInEffects(enum BattlerId battler, u32 *index, bool32 shouldTrigger, enum AbilityEffect abilityEffect);

#endif // GUARD_INNATE_ABILITIES_H
