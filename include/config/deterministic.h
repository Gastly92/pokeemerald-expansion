#ifndef GUARD_CONFIG_DETERMINISTIC_H
#define GUARD_CONFIG_DETERMINISTIC_H

// FORK: fork-owned config file. These DETERMINISTIC_* flags are an ongoing
// fork project to strip random chance out of the game piece by piece, so that
// outcomes are decided by player choices and state rather than RNG. Each flag
// removes one specific source of randomness; FALSE = stock
// pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered
// into the runtime config system (DETERMINISTIC_CONFIG_DEFINITIONS in
// constants/config_changes.h), so code reads them via GetConfig(DETERMINISTIC_X)
// and battle tests can toggle them per-test with WITH_CONFIG(DETERMINISTIC_X,
// TRUE/FALSE). The test baseline forces every flag off (see TestInitConfigData)
// so the inherited suite keeps exercising stock behavior; the dedicated
// test/battle/deterministic_*.c files opt in explicitly. To add a new flag: add
// a #define here and one line in DETERMINISTIC_CONFIG_DEFINITIONS.

// When TRUE, critical hits no longer occur from the random crit-chance roll.
// A hit still crits when it is *guaranteed* by a non-random source: moves that
// always crit (e.g. Frost Breath, Surging Strikes), Laser Focus, the Merciless
// ability vs. a poisoned target, and crit-stage stacks high enough to reach 1/1
// odds. Crit-blocking (Battle Armor, Shell Armor, Lucky Chant) is unaffected.
// See IsCriticalHit() in src/battle_util.c.
#define DETERMINISTIC_CRITICAL_HITS TRUE

// When TRUE, the post-calc damage roll no longer multiplies damage by a random
// 85%-100%. Instead the multiplier is fixed and scales with the battle's turn
// count: it is DETERMINISTIC_DAMAGE_BASE_PERCENT on the first turn and grows by
// DETERMINISTIC_DAMAGE_TURN_INCREMENT for every turn that has passed thereafter.
// With the defaults below that is 92% on turn 1, 93% on turn 2, 94% on turn 3,
// and so on. It is intentionally *uncapped*, so from turn 9 on it exceeds 100%
// and a move can deal more than its stock maximum. The AI's damage prediction is
// taught the same value, so min/median/max/random rolls all collapse to this
// single deterministic figure. See DoMoveDamageCalcVars in src/battle_util.c and
// the roll helpers in src/battle_ai_util.c.
#define DETERMINISTIC_DAMAGE TRUE

// Tuning for DETERMINISTIC_DAMAGE (ignored when it is FALSE). BASE_PERCENT is the
// first-turn multiplier as a percentage; TURN_INCREMENT is how many percentage
// points it climbs for each turn that has since elapsed. Set TURN_INCREMENT to 0
// for a flat multiplier with no per-turn ramp.
#define DETERMINISTIC_DAMAGE_BASE_PERCENT 92
#define DETERMINISTIC_DAMAGE_TURN_INCREMENT 1

// When TRUE, paralysis stops being a coin-flip and becomes a flat, predictable
// tax. It no longer randomly costs the battler its turn (the 25% full-paralysis
// roll in CancelerParalyzed) and no longer cuts Speed (the drop in
// GetBattlerTotalSpeedStat). In their place every move the paralyzed battler uses
// pays two deterministic penalties: it costs DETERMINISTIC_PARALYSIS_PP_TAX extra
// PP (CancelerPPDeduction) and its priority is lowered by
// DETERMINISTIC_PARALYSIS_PRIORITY_TAX (GetBattleMovePriority) — so a paralyzed
// mon moves later in its priority bracket and burns PP faster, but never freezes
// up and keeps its full Speed. Quick Feet, whose niche is shrugging off the
// paralysis Speed drop, is exempt from both taxes. See
// src/battle_move_resolution.c and src/battle_main.c.
#define DETERMINISTIC_PARALYSIS TRUE

// Tuning for DETERMINISTIC_PARALYSIS (ignored when it is FALSE). PP_TAX is how
// many extra PP each move costs while paralyzed; PRIORITY_TAX is how many points
// of priority each move loses. Set either to 0 to drop that one penalty (e.g.
// PRIORITY_TAX 0 keeps only the PP tax). Mirrors DETERMINISTIC_DAMAGE's
// toggle + compile-time tuning split.
#define DETERMINISTIC_PARALYSIS_PP_TAX 1
#define DETERMINISTIC_PARALYSIS_PRIORITY_TAX 1

#endif // GUARD_CONFIG_DETERMINISTIC_H
