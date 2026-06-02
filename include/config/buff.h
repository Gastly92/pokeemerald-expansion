#ifndef GUARD_CONFIG_BUFF_H
#define GUARD_CONFIG_BUFF_H

// FORK: fork-owned config file. These BUFF_* flags are an ongoing fork project
// to rebalance items and other game functionality, usually as compensation for
// other changes we make (e.g. the DETERMINISTIC_* project trades random upsides
// away, so some items get buffed to keep battles balanced). Each flag improves
// one specific thing; FALSE = stock pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered
// into the runtime config system (BUFF_CONFIG_DEFINITIONS in
// constants/config_changes.h), so code reads them via GetConfig(BUFF_X) and
// battle tests can toggle them per-test with WITH_CONFIG(BUFF_X, TRUE/FALSE).
// The test baseline forces every flag off (see TestInitConfigData) so the
// inherited suite keeps exercising stock behavior; the dedicated
// test/battle/buff_*.c files opt in explicitly. To add a new flag: add a
// #define here and one line in BUFF_CONFIG_DEFINITIONS.

// When TRUE, Shell Bell recovery is buffed from the stock 1/8 of the damage dealt
// to 1/BUFF_SHELL_BELL_DENOMINATOR (1/4 by default). The toggle is registered in
// the runtime config system (BUFF_CONFIG_DEFINITIONS) so battle tests can flip it
// per-test; the magnitude lives in the plain compile-time constant below. This
// mirrors how DETERMINISTIC_DAMAGE pairs a registered toggle with unregistered
// BASE_PERCENT/TURN_INCREMENT tuning. See TryShellBell() in
// src/battle_hold_effects.c.
#define BUFF_SHELL_BELL TRUE

// Heal divisor used when BUFF_SHELL_BELL is on: HP recovered = damage dealt /
// BUFF_SHELL_BELL_DENOMINATOR (4 -> 1/4). Stock behavior (flag off) instead
// divides by the item's holdEffectParam (8 -> 1/8). Lower = more healing; must be
// nonzero. Ignored when BUFF_SHELL_BELL is FALSE.
#define BUFF_SHELL_BELL_DENOMINATOR 4

#endif // GUARD_CONFIG_BUFF_H
