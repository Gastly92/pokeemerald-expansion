#include "global.h"
#include "battle.h"
#include "battle_controllers.h" // BattlerHasAi
#include "battle_gimmick.h"
#include "battle_util.h"
#include "config/feature.h"
#include "constants/battle_util.h"
#include "constants/config_changes.h"
#include "fork/free_gimmick_struggle.h"

// The selection restrictions each gimmick does NOT lift, mirroring the
// dynamaxBypassCheck / zMoveBypassCheck gates in TrySetCantSelectMoveBattleScript
// (src/battle_util.c). A move that clears its gimmick's mask can be picked once the
// gimmick is armed. MOVE_LIMITATION_UNUSABLE is kept for the Z-Move because it bundles
// Fake Out / Spit Up (not lifted) with Burn Up / Last Resort (lifted); keeping it errs
// toward the vanilla Struggle.
#define Z_MOVE_KEPT_LIMITATIONS (MOVE_LIMITATION_ZEROMOVE | MOVE_LIMITATION_PP | MOVE_LIMITATION_CHOICE_ITEM \
                               | MOVE_LIMITATION_ASSAULT_VEST | MOVE_LIMITATION_STUFF_CHEEKS              \
                               | MOVE_LIMITATION_CANT_USE_TWICE | MOVE_LIMITATION_UNUSABLE)
#define DYNAMAX_KEPT_LIMITATIONS (MOVE_LIMITATION_ZEROMOVE | MOVE_LIMITATION_PP | MOVE_LIMITATION_TAUNT \
                                | MOVE_LIMITATION_ASSAULT_VEST | MOVE_LIMITATION_CANT_USE_TWICE)

static bool32 GimmickUnlocksAMove(enum BattlerId battler, enum Gimmick gimmick, u32 keptLimitations)
{
    return (GetGimmickCandidates(battler) & (1u << gimmick))
        && CheckMoveLimitations(battler, 0, keptLimitations) != ALL_MOVES_MASK;
}

static bool32 IsBypassGimmickArmed(enum BattlerId battler)
{
    enum Gimmick gimmick = gBattleStruct->gimmick.usableGimmick[battler];

    return (gBattleResources->bufferB[battler][2] & RET_GIMMICK)
        && (gimmick == GIMMICK_Z_MOVE || gimmick == GIMMICK_DYNAMAX);
}

bool32 ShouldOpenMoveMenuForGimmick(enum BattlerId battler)
{
    if (!GetConfig(FEATURE_FREE_GIMMICKS)
     || BattlerHasAi(battler)
     || (gBattleTypeFlags & BATTLE_TYPE_PALACE))
        return FALSE;

    if (!GimmickUnlocksAMove(battler, GIMMICK_Z_MOVE, Z_MOVE_KEPT_LIMITATIONS)
     && !GimmickUnlocksAMove(battler, GIMMICK_DYNAMAX, DYNAMAX_KEPT_LIMITATIONS))
        return FALSE;

    // AreAllMovesUnusable has already set the battler up to Struggle; undo both halves.
    // noValidMoves would suppress the gimmick armed in the menu (see its checks on
    // gimmick.toActivate in src/battle_main.c), and a selection script left pending
    // trips Cmd_end's assert when the turn's battle scripts run.
    gProtectStructs[battler].noValidMoves = FALSE;
    gSelectionBattleScripts[battler] = NULL;
    return TRUE;
}

bool32 ShouldStruggleFromGimmickMoveMenu(enum BattlerId battler)
{
    // Re-runs AreAllMovesUnusable, which re-arms noValidMoves and the "no moves left"
    // selection script when it returns TRUE.
    return GetConfig(FEATURE_FREE_GIMMICKS)
        && !BattlerHasAi(battler)
        && !(gBattleTypeFlags & BATTLE_TYPE_PALACE)
        && !IsBypassGimmickArmed(battler)
        && AreAllMovesUnusable(battler);
}
