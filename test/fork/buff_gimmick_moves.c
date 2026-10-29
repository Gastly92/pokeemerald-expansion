#include "global.h"
#include "test/battle.h"

// BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS: Z-Moves and Max Moves ignore the target's positive
// Defense / Sp. Def stages like a critical hit. See include/fork/buff_gimmick_moves.h.
// Iron Defense is +2 Def, which halves physical damage when it counts.

SINGLE_BATTLE_TEST("BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS: a Z-Move ignores the target's Defense boosts", s16 damage)
{
    bool32 buff, boosted;
    PARAMETRIZE { buff = FALSE; boosted = FALSE; }
    PARAMETRIZE { buff = FALSE; boosted = TRUE; }
    PARAMETRIZE { buff = TRUE;  boosted = FALSE; }
    PARAMETRIZE { buff = TRUE;  boosted = TRUE; }
    GIVEN {
        WITH_CONFIG(BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS, buff);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Item(ITEM_NORMALIUM_Z); Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_CHANSEY) { Speed(100); MaxHP(700); HP(700); Moves(MOVE_IRON_DEFENSE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponent, boosted ? MOVE_IRON_DEFENSE : MOVE_CELEBRATE); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_Z_MOVE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BREAKNECK_BLITZ, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage); // stock: +2 Def halves it
        EXPECT_EQ(results[3].damage, results[2].damage);                  // buff: the boost is ignored
        EXPECT_EQ(results[2].damage, results[0].damage);                  // and an unboosted hit is unchanged
    }
}

SINGLE_BATTLE_TEST("BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS: a Max Move ignores the target's Sp. Def boosts", s16 damage)
{
    bool32 buff, boosted;
    PARAMETRIZE { buff = FALSE; boosted = FALSE; }
    PARAMETRIZE { buff = FALSE; boosted = TRUE; }
    PARAMETRIZE { buff = TRUE;  boosted = FALSE; }
    PARAMETRIZE { buff = TRUE;  boosted = TRUE; }
    GIVEN {
        WITH_CONFIG(BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS, buff);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_EMBER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_CHANSEY) { Speed(100); MaxHP(700); HP(700); Moves(MOVE_AMNESIA, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponent, boosted ? MOVE_AMNESIA : MOVE_CELEBRATE); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_EMBER, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MAX_FLARE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage);
        EXPECT_EQ(results[3].damage, results[2].damage);
        EXPECT_EQ(results[2].damage, results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS: Defense drops still count", s16 damage)
{
    bool32 lowered;
    PARAMETRIZE { lowered = FALSE; }
    PARAMETRIZE { lowered = TRUE; }
    GIVEN {
        WITH_CONFIG(BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_TACKLE, MOVE_SCREECH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_CHANSEY) { Speed(50); MaxHP(700); HP(700); }
    } WHEN {
        TURN { MOVE(player, lowered ? MOVE_SCREECH : MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MAX_STRIKE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[1].damage, results[0].damage); // -2 Def still roughly doubles the hit
    }
}

SINGLE_BATTLE_TEST("BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS: ordinary moves still respect Defense boosts", s16 damage)
{
    bool32 boosted;
    PARAMETRIZE { boosted = FALSE; }
    PARAMETRIZE { boosted = TRUE; }
    GIVEN {
        WITH_CONFIG(BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Item(ITEM_NORMALIUM_Z); Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_CHANSEY) { Speed(100); MaxHP(700); HP(700); Moves(MOVE_IRON_DEFENSE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponent, boosted ? MOVE_IRON_DEFENSE : MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage);
    }
}
