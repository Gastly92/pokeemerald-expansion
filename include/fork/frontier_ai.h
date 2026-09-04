#ifndef GUARD_FORK_FRONTIER_AI_H
#define GUARD_FORK_FRONTIER_AI_H

// FORK: the Battle Frontier's two-tier AI difficulty. Upstream picks facility AI
// flags per facility (a per-challenge ramp in GetAiScriptsInBattleFactory, a flat
// basic preset for everything else); this fork instead picks them by *opponent
// role*, so the hardest preset is reserved for the fights that should be the
// wall of a run and every routine opponent sits one tier below it.
//
// Both presets are configured in include/config/frontier.h
// (B_FRONTIER_HARD_AI_FLAGS / B_FRONTIER_REGULAR_AI_FLAGS) and the whole feature
// is gated on B_FRONTIER_HARD_AI. Live under B_FRONTIER_HARD_AI only; with the
// flag off, callers keep their vanilla paths. See src/fork/frontier_ai.c.

#include "constants/trainers.h"

// TRUE for the opponents that get the boss tier: the Frontier Brain of any
// facility, and the Battle Tower's fork-owned gym-leader bosses.
bool32 IsFrontierBossTrainer(u16 trainerId);

// The AI flag set for a Frontier opponent: the boss tier for the trainers above,
// the regular tier for everyone else.
u64 GetFrontierAiFlags(u16 trainerId);

#endif // GUARD_FORK_FRONTIER_AI_H
