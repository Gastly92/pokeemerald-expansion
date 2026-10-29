#include "global.h"
#include "test/test.h"
#include "battle_environment.h"
#include "battle_frontier.h"
#include "fork/frontier_battle_bg.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_trainers.h"

// B_FRONTIER_THEMED_BACKGROUNDS. The battle test runner forces its own environment, so the
// table is checked directly rather than through a battle.

static bool32 HasBackgroundArt(u32 environment)
{
    return gBattleEnvironmentInfo[environment].background.tileset != NULL
        && gBattleEnvironmentInfo[environment].background.tilemap != NULL;
}

TEST("Themed Frontier backgrounds: every Frontier trainer's class has a theme with real art")
{
    u32 i;
    for (i = 0; i < FRONTIER_TRAINERS_COUNT; i++)
    {
        u32 environment = GetFrontierEnvironmentForFacilityClass(gBattleFrontierTrainers[i].facilityClass);
        EXPECT_NE(environment, BATTLE_ENVIRONMENT_FRONTIER);
        EXPECT(HasBackgroundArt(environment));
    }
}

TEST("Themed Frontier backgrounds: every Frontier Brain has an arena with real art")
{
    u32 i;
    for (i = 0; i < NUM_FRONTIER_FACILITIES; i++)
    {
        u32 environment = GetFrontierBrainEnvironment(i);
        EXPECT_NE(environment, BATTLE_ENVIRONMENT_FRONTIER);
        EXPECT(HasBackgroundArt(environment));
    }
}

TEST("Themed Frontier backgrounds: every background with art is used by some trainer or Brain")
{
    u32 i, environment;
    bool32 used[BATTLE_ENVIRONMENT_COUNT] = {0};

    for (i = 0; i < FRONTIER_TRAINERS_COUNT; i++)
        used[GetFrontierEnvironmentForFacilityClass(gBattleFrontierTrainers[i].facilityClass)] = TRUE;
    for (i = 0; i < NUM_FRONTIER_FACILITIES; i++)
        used[GetFrontierBrainEnvironment(i)] = TRUE;
    used[BATTLE_ENVIRONMENT_LEADER] = TRUE;   // Tower gym-leader bosses
    used[BATTLE_ENVIRONMENT_FRONTIER] = TRUE; // the fallback

    for (environment = 0; environment < BATTLE_ENVIRONMENT_COUNT; environment++)
    {
        if (HasBackgroundArt(environment))
            EXPECT(used[environment]);
    }
}

TEST("Themed Frontier backgrounds: a class without a theme keeps the Frontier backdrop")
{
    EXPECT_EQ(GetFrontierEnvironmentForFacilityClass(FACILITY_CLASS_AQUA_GRUNT_M), BATTLE_ENVIRONMENT_FRONTIER);
    EXPECT_EQ(GetFrontierEnvironmentForFacilityClass(FACILITY_CLASSES_COUNT), BATTLE_ENVIRONMENT_FRONTIER);
}

TEST("Themed Frontier backgrounds: sample classes get their theme")
{
    EXPECT_EQ(GetFrontierEnvironmentForFacilityClass(FACILITY_CLASS_BUG_CATCHER), BATTLE_ENVIRONMENT_LONG_GRASS);
    EXPECT_EQ(GetFrontierEnvironmentForFacilityClass(FACILITY_CLASS_HIKER), BATTLE_ENVIRONMENT_MOUNTAIN);
    EXPECT_EQ(GetFrontierEnvironmentForFacilityClass(FACILITY_CLASS_YOUNGSTER), BATTLE_ENVIRONMENT_GRASS);
    EXPECT_EQ(GetFrontierBrainEnvironment(FRONTIER_FACILITY_FACTORY), BATTLE_ENVIRONMENT_CHAMPION);
}
