#ifndef GUARD_CONFIG_NERF_H
#define GUARD_CONFIG_NERF_H

// fork-owned config file. These NERF_* flags are the counterpart of BUFF_* (config/buff.h):
// each one weakens a single thing that was too strong, often a combination the fork's own
// changes created (innates stacking two abilities, the DETERMINISTIC_* hit counts). FALSE =
// stock pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered into the
// runtime config system (NERF_CONFIG_DEFINITIONS in constants/config_changes.h), so code reads
// them via GetConfig(NERF_X) and battle tests can toggle them per-test with
// WITH_CONFIG(NERF_X, TRUE/FALSE). The test baseline forces every flag off (see
// TestInitConfigData) so the inherited suite keeps exercising stock behavior; the dedicated
// test/fork/ files opt in explicitly. To add a new flag: add a #define here and one line in
// NERF_CONFIG_DEFINITIONS.

// When TRUE, every hit of a multi-hit move costs PP, not just the first. A 2-5 hit move,
// a fixed multi-strike move (Double Kick, Triple Axel, Dragon Darts), Population Bomb and
// Beat Up each pay the move's full per-use cost again for every strike after the first, so
// Pressure, the paralysis and Halo taxes and the DETERMINISTIC_ACCURACY_EVASION economy all
// apply per hit (a one-off Micle Berry refund does not). Payment is per hit landed: a KO on
// hit 3 of 5 pays for 3, and a hit Protect blocks ends the move after 1. A strike the move
// can no longer afford is not thrown, so a 5-hit move with 2 PP left hits twice. A called or
// copied move (Metronome, Sleep Talk, Dancer, ...) still pays only its caller's single PP.
// Parental Bond is not a multi-hit move and is unaffected. The move menu, the damage preview
// and the AI all project the hits that can be afforded. Aimed at Skill Link, which otherwise
// turns a 3-hit move into a 5-hit one for free (and stacks with an innate Technician). See
// src/fork/multi_hit_pp.c and fork-docs/MULTI_HIT_PP.md.
#define NERF_MULTI_HIT_PP_PER_HIT TRUE

#endif // GUARD_CONFIG_NERF_H
