#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_GEMS flag (config/buff.h). BUFF_* flags default off in the
// test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG(BUFF_GEMS, TRUE/FALSE) explicitly.
//
// Wobbuffet is Psychic, so Ember is neither STAB nor resisted and the Gem multiplier is
// the only thing separating a parametrized pair. The flag changes the MAGNITUDE only --
// which move arms a Gem, and the fact that it is spent on use, are untouched, so the
// consumption test below is as much a guard on "we did not accidentally change that" as
// it is on the boost itself.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_FIRE_GEM].holdEffect == HOLD_EFFECT_GEMS);
    ASSUME(GetItemSecondaryId(ITEM_FIRE_GEM) == TYPE_FIRE);
    ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
    ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
}

SINGLE_BATTLE_TEST("BUFF_GEMS: a Gem raises its type's damage by 60%", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_GEM; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_GEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.6), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_GEMS: a Gem still does nothing for another type", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_GEM; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_GEMS, TRUE);
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

// The point of the whole design: the Gem is (boost x ONE turn) against a type item's
// (boost x every turn). If the boost ever stopped being one-shot, +60% would no longer be
// a niche beside the permanent items -- it would dominate them.
SINGLE_BATTLE_TEST("BUFF_GEMS: the Gem is still consumed, so the second move is unboosted", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_GEM; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        WITH_CONFIG(BUFF_GEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_GEMS off: a Gem gives the stock 30%", s16 damage)
{
    u32 item;

    PARAMETRIZE { item = ITEM_FIRE_GEM; }
    PARAMETRIZE { item = ITEM_NONE; }

    GIVEN {
        ASSUME(gItemsInfo[ITEM_FIRE_GEM].holdEffectParam == 30);
        WITH_CONFIG(BUFF_GEMS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EMBER, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.3), results[0].damage);
    }
}
