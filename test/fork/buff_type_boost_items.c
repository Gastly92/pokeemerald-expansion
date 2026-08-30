#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_TYPE_BOOST_ITEMS flag (config/buff.h). BUFF_* flags
// default off in the test baseline (see TestInitConfigData), so each test opts in
// with WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE/FALSE) explicitly. Wobbuffet is
// Psychic, so Ember is neither STAB nor resisted and the multiplier is the only
// thing separating the two runs.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_CHARCOAL].holdEffect == HOLD_EFFECT_TYPE_POWER);
    ASSUME(gItemsInfo[ITEM_FLAME_PLATE].holdEffect == HOLD_EFFECT_PLATE);
    ASSUME(GetItemSecondaryId(ITEM_CHARCOAL) == TYPE_FIRE);
    ASSUME(GetItemSecondaryId(ITEM_FLAME_PLATE) == TYPE_FIRE);
    ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
}

SINGLE_BATTLE_TEST("BUFF_TYPE_BOOST_ITEMS: a type-boosting item raises its type's damage by 40%", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_CHARCOAL; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_TYPE_BOOST_ITEMS: a Plate gets the same 40% as the generic items", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FLAME_PLATE; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.4), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_TYPE_BOOST_ITEMS: a type-boosting item still does nothing for another type", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_CHARCOAL; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, TRUE);
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

SINGLE_BATTLE_TEST("BUFF_TYPE_BOOST_ITEMS off: a type-boosting item gives the stock 20%", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_CHARCOAL; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_TYPE_BOOST_ITEMS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.2), results[0].damage);
    }
}
