#include "global.h"
#include "test/battle.h"

// Coverage for the BUFF_REBURN flag (config/buff.h). BUFF_* flags default off in
// the test baseline (see TestInitConfigData), so each test opts in explicitly.

ASSUMPTIONS
{
    ASSUME(GetMoveNonVolatileStatus(MOVE_WILL_O_WISP) == MOVE_EFFECT_BURN);
}

SINGLE_BATTLE_TEST("BUFF_REBURN: Will-O-Wisp on a burned foe deals an immediate 1/8 burn tick")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
        MESSAGE("The opposing Wobbuffet was hurt by its burn!");
        HP_BAR(opponent, damage: 20);
        NOT MESSAGE("The opposing Wobbuffet is already burned!");
        // ...and the normal end-turn burn still ticks.
        MESSAGE("The opposing Wobbuffet was hurt by its burn!");
        HP_BAR(opponent, damage: 10);
    }
}

SINGLE_BATTLE_TEST("BUFF_REBURN off: Will-O-Wisp on a burned foe fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, FALSE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
        MESSAGE("The opposing Wobbuffet is already burned!");
    }
}

SINGLE_BATTLE_TEST("BUFF_REBURN: Heatproof halves the re-burn tick")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BRONZONG) { Ability(ABILITY_HEATPROOF); MaxHP(160); HP(160); Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
        MESSAGE("The opposing Bronzong was hurt by its burn!");
        HP_BAR(opponent, damage: 10);
    }
}

SINGLE_BATTLE_TEST("BUFF_REBURN: a block that stops a fresh burn also stops the re-burn")
{
    u32 species, ability;
    PARAMETRIZE { species = SPECIES_CLEFABLE; ability = ABILITY_MAGIC_GUARD; }
    PARAMETRIZE { species = SPECIES_GOLDEEN;  ability = ABILITY_WATER_VEIL; }
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(ability); Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REBURN: Substitute blocks the re-burn")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(5); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REBURN: Will-O-Wisp on a foe with a different status still fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { MOVE(player, MOVE_WILL_O_WISP); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, player);
        MESSAGE("But it failed!");
    } THEN {
        EXPECT(opponent->status1 & STATUS1_PARALYSIS);
    }
}

AI_SINGLE_BATTLE_TEST("BUFF_REBURN: the AI re-burns an already-burned foe instead of a status move that fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WILL_O_WISP, MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WILL_O_WISP); }
    }
}

AI_SINGLE_BATTLE_TEST("BUFF_REBURN: the AI finishes a burned foe with Will-O-Wisp when the tick KOs")
{
    GIVEN {
        WITH_CONFIG(BUFF_REBURN, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(20); Status1(STATUS1_BURN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WILL_O_WISP, MOVE_CELEBRATE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WILL_O_WISP); }
    }
}
