#include "global.h"
#include "test/battle.h"

// Coverage for FEATURE_CURSED_BODY_IGNORES_Z_MOVES (config/feature.h). Stock Cursed Body
// "disables" a damaging Z-Move, which prints a message but matches no move slot. With the
// feature on it stays silent there, and still disables the base move of a status Z-Move.

SINGLE_BATTLE_TEST("FEATURE_CURSED_BODY_IGNORES_Z_MOVES: Cursed Body does not trigger on a damaging Z-Move")
{
    bool32 ignore;
    PARAMETRIZE { ignore = TRUE; }
    PARAMETRIZE { ignore = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_CURSED_BODY_IGNORES_Z_MOVES, ignore);
        ASSUME(GetMoveType(MOVE_WATER_GUN) == gItemsInfo[ITEM_WATERIUM_Z].secondaryId);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WATERIUM_Z); }
        OPPONENT(SPECIES_JELLICENT) { Ability(ABILITY_CURSED_BODY); }
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN, gimmick: GIMMICK_Z_MOVE, WITH_RNG(RNG_CURSED_BODY, 1)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HYDRO_VORTEX, player);
        HP_BAR(opponent);
        if (ignore)
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
                MESSAGE("Wobbuffet's Hydro Vortex was disabled!");
            }
        }
        else
        {
            ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
            MESSAGE("Wobbuffet's Hydro Vortex was disabled!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_CURSED_BODY_IGNORES_Z_MOVES: Cursed Body still disables the base move of a status Z-Move")
{
    GIVEN {
        WITH_CONFIG(FEATURE_CURSED_BODY_IGNORES_Z_MOVES, TRUE);
        ASSUME(GetMoveEffect(MOVE_NATURE_POWER) == EFFECT_NATURE_POWER);
        ASSUME(GetMoveTerrainType(MOVE_ELECTRIC_TERRAIN) == B_TERRAIN_ELECTRIC);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_NORMALIUM_Z); }
        OPPONENT(SPECIES_FRILLISH) { Ability(ABILITY_CURSED_BODY); }
    } WHEN {
        TURN { MOVE(player, MOVE_ELECTRIC_TERRAIN); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_NATURE_POWER, gimmick: GIMMICK_Z_MOVE, WITH_RNG(RNG_CURSED_BODY, 1)); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ZMOVE_ACTIVATE, player);
        HP_BAR(opponent);
        ABILITY_POPUP(opponent, ABILITY_CURSED_BODY);
        MESSAGE("Wobbuffet's Nature Power was disabled!");
    } THEN {
        u32 disabledMove = player->volatiles.disabledMove;
        EXPECT_EQ(disabledMove, MOVE_NATURE_POWER);
    }
}
