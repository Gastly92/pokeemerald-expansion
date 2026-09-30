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

#include "battle_gimmick.h"
#include "constants/battle.h"
#include "constants/trainers.h"

// TRUE for the opponents that get the boss tier: the Frontier Brain of any
// facility, and the Battle Tower's fork-owned gym-leader bosses.
bool32 IsFrontierBossTrainer(u16 trainerId);

// The AI flag set for a Frontier opponent: the boss tier for the trainers above,
// the regular tier for everyone else.
u64 GetFrontierAiFlags(u16 trainerId);

// FORK (FEATURE_FREE_GIMMICKS): whether a Frontier opponent may use a gimmick.
// Bosses (above) may use them all; a regular opponent may use everything except
// B_FRONTIER_BOSS_ONLY_GIMMICKS (config/frontier.h: Mega, Z-Move, Dynamax and Tera).
bool32 FrontierOpponentMayUseGimmick(u16 trainerId, enum Gimmick gimmick);

// TRUE during the Battle Factory's set-milestone battle (the 10th, 20th, ... win of
// a streak), whose opponent is seeded with a guaranteed legendary. That opponent is
// a regular facility trainer id, so IsFrontierBossTrainer cannot see it.
bool32 IsFactoryMilestoneBattle(void);

// Script side of the same milestone, asked from the Factory's pre-battle room before
// the battle exists (so it reads the win streak, not gBattleTypeFlags). Sets
// VAR_RESULT to TRUE when the *upcoming* match is the milestone, so the attendant can
// warn the player. Called via `callnative`.
void Script_IsNextFactoryBattleMilestone(void);

// The battler-level check CanActivateGimmick uses: TRUE when this battler is a
// regular Frontier opponent's mon and `gimmick` is boss-only. Never TRUE for the
// player's side, outside the Frontier, with FEATURE_FREE_GIMMICKS off, for a boss
// trainer, or in the Factory's milestone battle (IsFactoryMilestoneBattle).
bool32 IsGimmickWithheldFromFrontierOpponent(enum BattlerId battler, enum Gimmick gimmick);

#endif // GUARD_FORK_FRONTIER_AI_H
