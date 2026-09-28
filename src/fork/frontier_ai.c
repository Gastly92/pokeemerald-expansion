#include "global.h"
#include "fork/frontier_ai.h"
#include "fork/battle_ai_species_overrides.h" // AI_FLAG_SMART_SPECIES_LOGIC for B_FRONTIER_HARD_AI_FLAGS
#include "fork/battle_ai_zmove.h"             // AI_FLAG_SMART_Z_MOVE for B_FRONTIER_HARD_AI_FLAGS
#include "fork/battle_tower_trainers.h"       // IsTowerBossTrainerId
#include "battle.h"
#include "battle_controllers.h"               // GetBattlerTrainer
#include "battle_gimmick.h"                   // enum Gimmick for B_FRONTIER_BOSS_ONLY_GIMMICKS
#include "battle_setup.h"                    // TRAINER_BATTLE_PARAM
#include "constants/battle_frontier.h"        // FRONTIER_STAGES_PER_CHALLENGE
#include "constants/battle_ai.h"
#include "constants/trainers.h"

// FORK: which AI preset a Battle Frontier opponent runs, chosen by opponent role
// rather than by facility or win streak.
//
// The fork used to hand B_FRONTIER_HARD_AI_FLAGS to every Battle Factory
// opponent alike, which made the strongest AI in the game the *baseline* for a
// run — a boss battle then played no better than the routine fight before it.
// Now the top preset is reserved for the fights meant to be a wall (the Frontier
// Brain, the Tower's gym-leader bosses) and everything else runs one tier down.
//
// The two presets themselves are B_FRONTIER_HARD_AI_FLAGS and
// B_FRONTIER_REGULAR_AI_FLAGS in config/frontier.h; tune difficulty there, and
// change *who counts as a boss* here.

bool32 IsFrontierBossTrainer(u16 trainerId)
{
    // The Frontier Brain id is shared by every facility's Brain (the Factory
    // Head, the Salon Maiden, ...); the Tower's gym-leader bosses sit in the
    // fork-owned id range above TRAINER_PLAYER.
    //
    // The Battle Factory's milestone opponent (10th, 20th, ... win) is deliberately
    // NOT a boss here, so it keeps the regular AI tier: the player fights it with
    // rentals, whereas the Tower's gym leaders face a team the player built. It does
    // count as a boss for gimmicks, via IsFactoryMilestoneBattle. The intended split
    // is tabled in fork-docs/FRONTIER_ENDLESS.md, "Opponent tiers".
    return trainerId == TRAINER_FRONTIER_BRAIN || IsTowerBossTrainerId(trainerId);
}

u64 GetFrontierAiFlags(u16 trainerId)
{
    if (IsFrontierBossTrainer(trainerId))
        return B_FRONTIER_HARD_AI_FLAGS;

    return B_FRONTIER_REGULAR_AI_FLAGS;
}

bool32 FrontierOpponentMayUseGimmick(u16 trainerId, enum Gimmick gimmick)
{
    if (IsFrontierBossTrainer(trainerId))
        return TRUE;

    return !(B_FRONTIER_BOSS_ONLY_GIMMICKS & (1u << gimmick));
}

bool32 IsFactoryMilestoneBattle(void)
{
    if (!(gBattleTypeFlags & BATTLE_TYPE_FACTORY) || gSaveBlock2Ptr->frontier.lvlMode == FRONTIER_LVL_TENT)
        return FALSE;

    // The Factory's set-milestone opponent is a random facility trainer, not a boss id,
    // so the battle's position in its set is the only thing that marks it. That is
    // curChallengeBattleNum: GenerateOpponentMons writes it (derived from the win streak
    // under B_FRONTIER_ENDLESS) right before it seeds the milestone's legendary, and the
    // battle-room script only advances it after the win.
    return gSaveBlock2Ptr->frontier.curChallengeBattleNum == FRONTIER_STAGES_PER_CHALLENGE - 1;
}

bool32 IsGimmickWithheldFromFrontierOpponent(enum BattlerId battler, enum Gimmick gimmick)
{
    // Only free gimmicks are restricted: without the feature the item-gated
    // vanilla rules already decide who can use what.
    if (!GetConfig(FEATURE_FREE_GIMMICKS))
        return FALSE;
    if (!(gBattleTypeFlags & BATTLE_TYPE_FRONTIER) || IsOnPlayerSide(battler))
        return FALSE;

    if (IsFactoryMilestoneBattle())
        return FALSE;

    u16 trainerId = (GetBattlerTrainer(battler) == B_TRAINER_OPPONENT_B)
                  ? TRAINER_BATTLE_PARAM.opponentB
                  : TRAINER_BATTLE_PARAM.opponentA;

    return !FrontierOpponentMayUseGimmick(trainerId, gimmick);
}
