#ifndef GUARD_FORK_BUFF_STATUS_REAPPLY_H
#define GUARD_FORK_BUFF_STATUS_REAPPLY_H

#include "battle.h"

// BUFF_REBURN -- a status move that re-applies a status the target already has
// does something instead of failing. Definitions in src/fork/buff_status_reapply.c.

extern const u8 BattleScript_ReburnTick[];

// Called from Cmd_trynonvolatilestatus before the vanilla checks. When the current
// move re-applies a status the target already has and the buff for that status can
// land, sets up the effect, points gBattlescriptCurrInstr at its script and returns
// TRUE. Otherwise returns FALSE and the vanilla "already <status>" path runs.
bool32 TryStatusMoveReapply(void);

// AI: TRUE when `move` would re-burn `battlerDef` for an immediate tick.
bool32 AI_CanReburn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move);
// AI: adds the score for a re-burn (no-op when the move would not re-burn).
void IncreaseReburnScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score);
// The HP a re-burn tick takes from `battler`.
s32 GetReburnDamage(enum BattlerId battler);

#endif // GUARD_FORK_BUFF_STATUS_REAPPLY_H
