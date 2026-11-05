#include "global.h"
#include "test/battle.h"

// Regression test for the UPSTREAM: guard in ChangeTypeBasedOnTerrain (src/battle_util.c).
// Switch-in always runs the on-terrain ability check; with no terrain up, Mimicry used to
// read the empty B_TERRAIN_NONE table entry and set both types to TYPE_NONE.

SINGLE_BATTLE_TEST("Mimicry does not activate or change types on switch-in with no terrain")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_STUNFISK_GALAR) { Ability(ABILITY_MIMICRY); }
    } WHEN {
        TURN { }
    } SCENE {
        NOT ABILITY_POPUP(opponent, ABILITY_MIMICRY);
    } THEN {
        EXPECT_EQ(gBattleMons[B_POSITION_OPPONENT_LEFT].types[0], TYPE_GROUND);
        EXPECT_EQ(gBattleMons[B_POSITION_OPPONENT_LEFT].types[1], TYPE_STEEL);
    }
}

SINGLE_BATTLE_TEST("Mimicry still activates on switch-in when terrain is already up")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_STUNFISK_GALAR) { Ability(ABILITY_MIMICRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_ELECTRIC_TERRAIN); }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_MIMICRY);
        MESSAGE("The opposing Stunfisk's type changed to Electric!");
    } THEN {
        EXPECT_EQ(gBattleMons[B_POSITION_OPPONENT_LEFT].types[0], TYPE_ELECTRIC);
        EXPECT_EQ(gBattleMons[B_POSITION_OPPONENT_LEFT].types[1], TYPE_ELECTRIC);
    }
}
