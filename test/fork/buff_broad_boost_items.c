#include "global.h"
#include "test/battle.h"

// Coverage for BUFF_BROAD_BOOST_ITEMS (config/buff.h): Muscle Band, Wise Glasses and Punching
// Glove boost by +20% instead of +10%. BUFF_* flags default off in the test baseline (see
// TestInitConfigData), so each test opts in with WITH_CONFIG and the stock number is pinned with
// the flag explicitly off. Stock behavior is also covered by test/battle/hold_effect/muscle_band.c,
// wise_glasses.c and punching_glove.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_MUSCLE_BAND) == HOLD_EFFECT_MUSCLE_BAND);
    ASSUME(GetItemHoldEffect(ITEM_WISE_GLASSES) == HOLD_EFFECT_WISE_GLASSES);
    ASSUME(GetItemHoldEffect(ITEM_PUNCHING_GLOVE) == HOLD_EFFECT_PUNCHING_GLOVE);
    ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
    ASSUME(GetMoveCategory(MOVE_WATER_GUN) == DAMAGE_CATEGORY_SPECIAL);
    ASSUME(IsPunchingMove(MOVE_MEGA_PUNCH));
}

SINGLE_BATTLE_TEST("BUFF_BROAD_BOOST_ITEMS: Muscle Band, Wise Glasses and Punching Glove boost damage by 20%", s16 damage)
{
    u32 config;
    enum Item item;
    enum Move move;

    PARAMETRIZE { config = TRUE;  item = ITEM_NONE;           move = MOVE_SCRATCH; }
    PARAMETRIZE { config = TRUE;  item = ITEM_MUSCLE_BAND;    move = MOVE_SCRATCH; }
    PARAMETRIZE { config = TRUE;  item = ITEM_NONE;           move = MOVE_WATER_GUN; }
    PARAMETRIZE { config = TRUE;  item = ITEM_WISE_GLASSES;   move = MOVE_WATER_GUN; }
    PARAMETRIZE { config = TRUE;  item = ITEM_NONE;           move = MOVE_MEGA_PUNCH; }
    PARAMETRIZE { config = TRUE;  item = ITEM_PUNCHING_GLOVE; move = MOVE_MEGA_PUNCH; }
    PARAMETRIZE { config = FALSE; item = ITEM_NONE;           move = MOVE_SCRATCH; }
    PARAMETRIZE { config = FALSE; item = ITEM_MUSCLE_BAND;    move = MOVE_SCRATCH; }
    PARAMETRIZE { config = FALSE; item = ITEM_NONE;           move = MOVE_WATER_GUN; }
    PARAMETRIZE { config = FALSE; item = ITEM_WISE_GLASSES;   move = MOVE_WATER_GUN; }
    PARAMETRIZE { config = FALSE; item = ITEM_NONE;           move = MOVE_MEGA_PUNCH; }
    PARAMETRIZE { config = FALSE; item = ITEM_PUNCHING_GLOVE; move = MOVE_MEGA_PUNCH; }

    GIVEN {
        WITH_CONFIG(BUFF_BROAD_BOOST_ITEMS, config);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.2), results[1].damage);
        EXPECT_MUL_EQ(results[2].damage, Q_4_12(1.2), results[3].damage);
        EXPECT_MUL_EQ(results[4].damage, Q_4_12(1.2), results[5].damage);
        EXPECT_MUL_EQ(results[6].damage, Q_4_12(1.1), results[7].damage);
        EXPECT_MUL_EQ(results[8].damage, Q_4_12(1.1), results[9].damage);
        EXPECT_MUL_EQ(results[10].damage, Q_4_12(1.1), results[11].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_BROAD_BOOST_ITEMS: Muscle Band still does not boost special damage", s16 damage)
{
    enum Item item;

    PARAMETRIZE { item = ITEM_NONE; }
    PARAMETRIZE { item = ITEM_MUSCLE_BAND; }

    GIVEN {
        WITH_CONFIG(BUFF_BROAD_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WATER_GUN, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}
