#include "global.h"
#include "test/battle.h"

// Coverage for the BUFF_REPARALYZE flag (config/buff.h). BUFF_* flags default off in
// the test baseline (see TestInitConfigData), so each test opts in explicitly.

ASSUMPTIONS
{
    ASSUME(GetMoveNonVolatileStatus(MOVE_THUNDER_WAVE) == MOVE_EFFECT_PARALYSIS);
    ASSUME(GetMoveNonVolatileStatus(MOVE_GLARE) == MOVE_EFFECT_PARALYSIS);
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: Thunder Wave on a paralyzed foe drains 1 PP from each of its moves")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(5); Status1(STATUS1_PARALYSIS); MovesWithPP({MOVE_CELEBRATE, 10}, {MOVE_SPLASH, 10}, {MOVE_HARDEN, 10}, {MOVE_GROWL, 10}); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PRZ, opponent);
        MESSAGE("The opposing Wobbuffet's moves each lost 1 PP!");
    } THEN {
        // Slot 0 is Celebrate, which the opponent may also spend PP on; the rest only lose the drain.
        EXPECT_EQ(opponent->pp[1], 9);
        EXPECT_EQ(opponent->pp[2], 9);
        EXPECT_EQ(opponent->pp[3], 9);
        // ...and the drain is written back to the party, not just the battle copy.
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PP2), 9);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_PP4), 9);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: the drain never takes a move below 0 PP")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(5); Status1(STATUS1_PARALYSIS); MovesWithPP({MOVE_CELEBRATE, 10}, {MOVE_SPLASH, 0}, {MOVE_HARDEN, 1}); }
    } WHEN {
        TURN { MOVE(player, MOVE_GLARE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet's moves each lost 1 PP!");
    } THEN {
        EXPECT_EQ(opponent->pp[1], 0);
        EXPECT_EQ(opponent->pp[2], 0);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE off: Thunder Wave on a paralyzed foe fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, FALSE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: a block that stops a fresh paralysis also stops the drain")
{
    u32 species, ability;
    PARAMETRIZE { species = SPECIES_PIKACHU;   ability = ABILITY_STATIC; }  // Electric type
    PARAMETRIZE { species = SPECIES_SANDSHREW; ability = ABILITY_SAND_VEIL; } // Ground vs Thunder Wave
    PARAMETRIZE { species = SPECIES_PERSIAN;   ability = ABILITY_LIMBER; }
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(ability); Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: Magic Guard does not block the drain")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_CLEFABLE) { Speed(5); Ability(ABILITY_MAGIC_GUARD); Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
        MESSAGE("The opposing Clefable's moves each lost 1 PP!");
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: Substitute blocks the drain")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE); // no full-paralysis roll to stop the Substitute
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPARALYZE: a foe with a different status still makes Thunder Wave fail")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_BURN); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_WAVE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER_WAVE, player);
    }
}

AI_SINGLE_BATTLE_TEST("BUFF_REPARALYZE: the AI drains a paralyzed foe instead of a status move that fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPARALYZE, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDER_WAVE, MOVE_WILL_O_WISP); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_THUNDER_WAVE); }
    }
}
