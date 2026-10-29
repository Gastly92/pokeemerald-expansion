#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Dark Void can be used by Mega Darkrai in Gen 7+")
{
    GIVEN {
        WITH_CONFIG(B_DARK_VOID_FAIL, GEN_7);
        PLAYER(SPECIES_DARKRAI) { Item(ITEM_DARKRANITE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DARK_VOID, gimmick: GIMMICK_MEGA, hit: TRUE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DARK_VOID, player);
    } THEN {
        EXPECT_EQ(player->species, SPECIES_DARKRAI_MEGA);
        EXPECT_NE(opponent->status1 & STATUS1_SLEEP, 0);
    }
}
