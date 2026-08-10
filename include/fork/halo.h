#ifndef GUARD_FORK_HALO_H
#define GUARD_FORK_HALO_H

// FORK: "Halo" -- a custom ability whose effect is a FIELD-WIDE aura rather than a personal
// defence. While a Halo holder is on the field, no single hit may take more than
// HALO_DAMAGE_CAP_PERCENT% of its target's max HP -- for every battler present, foes, allies
// and the holder itself alike. The holder alone pays for it: each move it uses costs
// HALO_PP_TAX extra PP, so the aura is upkeep rather than a free boon (which is what makes it
// a chosen ability and never a "pure boon" innate -- see fork-docs/INNATE_ABILITIES.md).
//
// The cap is applied at the end of ApplyModifiersAfterDmgRoll() (src/battle_util.c), the one
// site both the live damage calc and the AI's simulation funnel through, so the AI predicts
// capped damage for free. Being in the per-hit calc also makes the cap PER HIT: a multi-hit
// move (always 3 hits under DETERMINISTIC_MOVE_RESULTS, 5 with Skill Link / Loaded Dice) can
// still exceed the cap in a turn, which is deliberate counterplay. Fixed-damage moves run
// through DoFixedDamageMoveCalc() instead and are exempt for the same reason.
// See fork-docs/NEW_ABILITIES.md.
//
// PERFORMANCE -- read this before moving the gate. Because the aura is field-wide, answering
// "is a Halo up?" means scanning every battler, and ApplyModifiersAfterDmgRoll() sits on the
// AI's measured hot path: the AI re-runs it FOUR times against the same DamageContext (the
// min/median/max/random rolls in AI_CalcDamage, src/battle_ai_util.c) and its total thinking
// time is asserted by test/battle/ai/ai_thinking_time.c. A per-call scan -- even an inlined one
// that early-outs -- put three of those assertions over budget. So the scan is resolved ONCE
// per context into the cached ctx->haloOnField bit (set in DoMoveDamageCalcVars beside the
// existing FORK ctx->innatesEnabled flag, which exists for exactly the same reason), and the
// hot path pays only a single bitfield test. Do not "simplify" that back into a live scan.
// Two things that look like optimisations and are NOT: hoisting a `maxHP * PERCENT / 100`
// pre-check ahead of the scan (the multiply/divide costs more per call than the scan it skips
// -- measured), and inlining the scan into the header (better than an out-of-line call, but
// still not enough in doubles, where there are four battlers to walk).

// The most any one hit may take, as a percentage of the target's max HP. 40 is deliberately the
// same ceiling the fork already applies to (formerly one-hit-KO) OHKO moves via
// DETERMINISTIC_OHKO_MAX_HP_PERCENT, and it is what makes the ability legible: 40 * 2 < 100 but
// 40 * 3 > 100, so a Halo'd battler can never be knocked out in fewer than three hits.
#define HALO_DAMAGE_CAP_PERCENT 40

// Extra PP every move by the Halo holder costs, on top of the usual 1 (and on top of Pressure
// and the DETERMINISTIC_PARALYSIS tax, which both stack additively with it). At 1 this halves
// the holder's effective PP: a 5-PP recovery move maxes out at 8 PP, i.e. 4 uses rather than 8.
#define HALO_PP_TAX 1

// TRUE when a battler with an active (suppression-aware) Halo is on the field. Called once per
// DamageContext to fill ctx->haloOnField -- never per damage roll; see the performance note.
bool32 IsHaloOnField(void);

// Clamps `dmg` dealt to `battlerDef` to the Halo cap. Callers must gate on ctx->haloOnField.
s32 ApplyHaloDamageCap(u32 battlerDef, s32 dmg);

#endif // GUARD_FORK_HALO_H
