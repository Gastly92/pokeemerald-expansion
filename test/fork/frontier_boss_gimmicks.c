#include "global.h"
#include "test/test.h"
#include "battle.h"
#include "battle_gimmick.h"
#include "battle_setup.h"
#include "fork/frontier_ai.h"
#include "fork/battle_tower_trainers.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/trainers.h"

// FORK: guards B_FRONTIER_BOSS_ONLY_GIMMICKS (config/frontier.h). Under
// FEATURE_FREE_GIMMICKS a regular Frontier opponent may not Mega Evolve, Z-Move,
// Dynamax or Terastallize; the Frontier Brain, the Tower's gym-leader bosses and
// the Factory's milestone opponent (every 10th win) keep all four, and the
// player's side is never restricted.
// Battle tests can't put a battle in Frontier mode, so these drive the predicate
// CanActivateGimmick consults directly, with the battle globals set by hand.

TEST("Frontier boss gimmicks: a regular opponent may not Mega, Z-Move, Dynamax or Tera")
{
    EXPECT(!FrontierOpponentMayUseGimmick(0, GIMMICK_MEGA));
    EXPECT(!FrontierOpponentMayUseGimmick(0, GIMMICK_TERA));
    EXPECT(!FrontierOpponentMayUseGimmick(0, GIMMICK_Z_MOVE));
    EXPECT(!FrontierOpponentMayUseGimmick(0, GIMMICK_DYNAMAX));
}

TEST("Frontier boss gimmicks: the Frontier Brain and gym-leader bosses may use every gimmick")
{
    enum Gimmick gimmicks[] = { GIMMICK_MEGA, GIMMICK_TERA, GIMMICK_Z_MOVE, GIMMICK_DYNAMAX };

    for (u32 g = 0; g < ARRAY_COUNT(gimmicks); g++)
    {
        EXPECT(FrontierOpponentMayUseGimmick(TRAINER_FRONTIER_BRAIN, gimmicks[g]));
        for (u32 i = 0; i < GetTowerBossCount(); i++)
            EXPECT(FrontierOpponentMayUseGimmick(TOWER_BOSS_TRAINER_FIRST + i, gimmicks[g]));
    }
}

static void SetUpFrontierSingles(u16 opponentId)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_FRONTIER;
    gBattlerPositions[B_BATTLER_0] = B_POSITION_PLAYER_LEFT;
    gBattlerPositions[B_BATTLER_1] = B_POSITION_OPPONENT_LEFT;
    TRAINER_BATTLE_PARAM.opponentA = opponentId;
}

TEST("Frontier boss gimmicks: a regular opponent's battler is withheld every gimmick")
{
    SetConfig(CONFIG_FEATURE_FREE_GIMMICKS, TRUE);
    SetUpFrontierSingles(0);

    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_MEGA));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_TERA));

    gBattleTypeFlags = 0;
}

TEST("Frontier boss gimmicks: the player and the Brain are never withheld anything")
{
    SetConfig(CONFIG_FEATURE_FREE_GIMMICKS, TRUE);
    SetUpFrontierSingles(0);
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_0, GIMMICK_Z_MOVE));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_0, GIMMICK_DYNAMAX));

    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_0, GIMMICK_MEGA));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_0, GIMMICK_TERA));

    SetUpFrontierSingles(TRAINER_FRONTIER_BRAIN);
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_MEGA));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_TERA));

    gBattleTypeFlags = 0;
}

TEST("Frontier boss gimmicks: nothing is withheld outside the Frontier or without free gimmicks")
{
    SetConfig(CONFIG_FEATURE_FREE_GIMMICKS, TRUE);
    SetUpFrontierSingles(0);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER; // an ordinary trainer battle
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));

    SetConfig(CONFIG_FEATURE_FREE_GIMMICKS, FALSE);
    SetUpFrontierSingles(0);
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));

    gBattleTypeFlags = 0;
}

// The Factory's 10th/20th/... battle is not a boss trainer id (it is a random facility
// trainer seeded with a legendary), so it is recognised by its position in the set.
TEST("Frontier boss gimmicks: the Battle Factory's milestone opponent may use every gimmick")
{
    u8 savedBattleNum = gSaveBlock2Ptr->frontier.curChallengeBattleNum;
    u8 savedLvlMode = gSaveBlock2Ptr->frontier.lvlMode;

    SetConfig(CONFIG_FEATURE_FREE_GIMMICKS, TRUE);
    SetUpFrontierSingles(0);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_FACTORY;
    gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_OPEN;

    // An ordinary Factory battle: withheld.
    gSaveBlock2Ptr->frontier.curChallengeBattleNum = FRONTIER_STAGES_PER_CHALLENGE - 2;
    EXPECT(!IsFactoryMilestoneBattle());
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_MEGA));
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_TERA));

    // The last battle of the set: allowed.
    gSaveBlock2Ptr->frontier.curChallengeBattleNum = FRONTIER_STAGES_PER_CHALLENGE - 1;
    EXPECT(IsFactoryMilestoneBattle());
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_DYNAMAX));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_MEGA));
    EXPECT(!IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_TERA));

    // The same slot in another facility is not a Factory milestone.
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER;
    EXPECT(!IsFactoryMilestoneBattle());
    EXPECT(IsGimmickWithheldFromFrontierOpponent(B_BATTLER_1, GIMMICK_Z_MOVE));

    gSaveBlock2Ptr->frontier.curChallengeBattleNum = savedBattleNum;
    gSaveBlock2Ptr->frontier.lvlMode = savedLvlMode;
    gBattleTypeFlags = 0;
}
