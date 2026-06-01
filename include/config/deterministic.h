#ifndef GUARD_CONFIG_DETERMINISTIC_H
#define GUARD_CONFIG_DETERMINISTIC_H

// FORK: fork-owned config file. These DETERMINISTIC_* flags are an ongoing
// fork project to strip random chance out of the game piece by piece, so that
// outcomes are decided by player choices and state rather than RNG. Each flag
// is opt-in (default FALSE = stock pokeemerald-expansion behavior) and removes
// one specific source of randomness. New files never conflict on upstream sync;
// only the single #include in include/constants/global.h touches a shared file.

// When TRUE, critical hits no longer occur from the random crit-chance roll.
// A hit still crits when it is *guaranteed* by a non-random source: moves that
// always crit (e.g. Frost Breath, Surging Strikes), Laser Focus, the Merciless
// ability vs. a poisoned target, and crit-stage stacks high enough to reach 1/1
// odds. Crit-blocking (Battle Armor, Shell Armor, Lucky Chant) is unaffected.
// See IsCriticalHit() in src/battle_util.c.
#define DETERMINISTIC_CRITICAL_HITS FALSE

#endif // GUARD_CONFIG_DETERMINISTIC_H
