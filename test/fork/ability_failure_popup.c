#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Coverage for FEATURE_ANNOUNCE_ABILITY_FAILURES (config/feature.h, src/fork/ability_failure_popup.c).
// Feature flags default off in the test baseline, so each test opts in with WITH_CONFIG; the
// stock plain "But it failed!" paths stay covered by the upstream tests.

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: Guard Dog shows its pop-up when it blocks Whirlwind")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, announce);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_GUARD_DOG); }
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_WHIRLWIND); }
    } SCENE {
        MESSAGE("Wobbuffet used Whirlwind!");
        if (announce)
            ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
        else
            NOT ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
        MESSAGE("But it failed!");
        NOT MESSAGE("The opposing Charmander was dragged out!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: innate Guard Dog shows its pop-up when it blocks Roar")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_OKIDOGI, ABILITY_GUARD_DOG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_TOXIC_CHAIN); } // chosen differs from the innate
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_ROAR); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
        MESSAGE("But it failed!");
        NOT MESSAGE("The opposing Charmander was dragged out!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: Guard Dog shows its pop-up when it blocks Dragon Tail's switch")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, announce);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_GUARD_DOG); }
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_TAIL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_TAIL, player);
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
            NOT MESSAGE("The opposing Charmander was dragged out!");
        }
        else
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
                MESSAGE("The opposing Charmander was dragged out!");
            }
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: Water Veil shows its pop-up when it blocks Psycho Shift")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, announce);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_GOLDEEN) { Ability(ABILITY_WATER_VEIL); }
    } WHEN {
        TURN { MOVE(player, MOVE_PSYCHO_SHIFT); }
    } SCENE {
        MESSAGE("Wobbuffet used Psycho Shift!");
        if (announce)
            ABILITY_POPUP(opponent, ABILITY_WATER_VEIL);
        else
            NOT ABILITY_POPUP(opponent, ABILITY_WATER_VEIL);
        MESSAGE("But it failed!");
    } THEN {
        EXPECT(player->status1 & STATUS1_BURN);
        EXPECT_EQ(opponent->status1, 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: Psycho Shift into an already statused target shows no pop-up")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_GOLDEEN) { Ability(ABILITY_WATER_VEIL); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_PSYCHO_SHIFT); }
    } SCENE {
        MESSAGE("Wobbuffet used Psycho Shift!");
        NOT ABILITY_POPUP(opponent, ABILITY_WATER_VEIL);
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: Leaf Guard shows its pop-up when it blocks Rest in sun")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(B_LEAF_GUARD_PREVENTS_REST, GEN_5);
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, announce);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); MOVE(player, MOVE_REST); }
    } SCENE {
        MESSAGE("Leafeon used Rest!");
        if (announce)
            ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        else
            NOT ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("But it failed!");
    } THEN {
        EXPECT_EQ(player->status1, 0);
    }
}

DOUBLE_BATTLE_TEST("FEATURE_ANNOUNCE_ABILITY_FAILURES: an ally's Sweet Veil shows its pop-up when it blocks Rest")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_ABILITY_FAILURES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        PLAYER(SPECIES_SWIRLIX) { Ability(ABILITY_SWEET_VEIL); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_REST); }
    } SCENE {
        MESSAGE("Wobbuffet used Rest!");
        ABILITY_POPUP(playerRight, ABILITY_SWEET_VEIL);
        MESSAGE("But it failed!");
    } THEN {
        EXPECT_EQ(playerLeft->status1, 0);
    }
}
