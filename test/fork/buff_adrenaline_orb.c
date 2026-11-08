#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Coverage for BUFF_ADRENALINE_ORB (config/buff.h). BUFF_* flags default off in the test
// baseline (see TestInitConfigData), so each test opts in with WITH_CONFIG. Stock behavior is
// covered by test/battle/hold_effect/adrenaline_orb.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_ADRENALINE_ORB) == HOLD_EFFECT_ADRENALINE_ORB);
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: Intimidate raises Speed by 2 instead of 1")
{
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, config);
        PLAYER(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_ADRENALINE_ORB); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_INTIMIDATE);
        MESSAGE("The opposing Wobbuffet's Attack fell!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, opponent);
        if (config)
            MESSAGE("The opposing Wobbuffet's Speed rose sharply!");
        else
            MESSAGE("The opposing Wobbuffet's Speed rose!");
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + (config ? 2 : 1));
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: any stat drop from an opponent triggers it, once")
{
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, config);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ADRENALINE_ORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_ICY_WIND); }
        TURN { MOVE(opponent, MOVE_ICY_WIND); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICY_WIND, opponent);
        MESSAGE("Wobbuffet's Speed fell!");
        if (config)
        {
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
            MESSAGE("Wobbuffet's Speed rose sharply!");
        }
        else
        {
            NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICY_WIND, opponent);
        MESSAGE("Wobbuffet's Speed fell!");
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
    } THEN {
        // Flag on: -1 +2 -1 = 0. Flag off: -1 -1 = -2, orb kept.
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + (config ? 0 : -2));
        EXPECT_EQ(player->item, config ? ITEM_NONE : ITEM_ADRENALINE_ORB);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: a blocked drop does not trigger or use up the orb")
{
    enum Ability ability;
    u32 species;

    PARAMETRIZE { species = SPECIES_METANG;  ability = ABILITY_CLEAR_BODY; }
    PARAMETRIZE { species = SPECIES_SPINDA;  ability = ABILITY_CONTRARY; }

    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, TRUE);
        PLAYER(SPECIES_EKANS) { Ability(ABILITY_INTIMIDATE); }
        OPPONENT(species) { Ability(ability); Item(ITEM_ADRENALINE_ORB); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_INTIMIDATE);
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, opponent);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
        EXPECT_EQ(opponent->item, ITEM_ADRENALINE_ORB);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: a drop blocked by Mist does not trigger the orb")
{
    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ADRENALINE_ORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_MIST); MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MIST, player);
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->item, ITEM_ADRENALINE_ORB);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: a self-inflicted drop does not trigger the orb")
{
    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ADRENALINE_ORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CLOSE_COMBAT); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CLOSE_COMBAT, player);
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->item, ITEM_ADRENALINE_ORB);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: a foe's Sticky Web triggers the orb")
{
    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT) { Item(ITEM_ADRENALINE_ORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_STICKY_WEB); }
        TURN { SWITCH(player, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STICKY_WEB, opponent);
        MESSAGE("Wynaut was caught in a sticky web!");
        MESSAGE("Wynaut's Speed fell!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        MESSAGE("Wynaut's Speed rose sharply!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("BUFF_ADRENALINE_ORB: innate Intimidate triggers the orb")
{
    GIVEN {
        WITH_CONFIG(BUFF_ADRENALINE_ORB, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_MAWILE, ABILITY_INTIMIDATE));
        PLAYER(SPECIES_MAWILE) { Ability(ABILITY_HYPER_CUTTER); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_ADRENALINE_ORB); }
    } WHEN {
        TURN { }
    } SCENE {
        MESSAGE("The opposing Wobbuffet's Attack fell!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, opponent);
        MESSAGE("The opposing Wobbuffet's Speed rose sharply!");
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
    }
}
