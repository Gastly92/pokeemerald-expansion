#include "global.h"
#include "test/battle.h"

// Coverage for the BUFF_REPOISON flag (config/buff.h). BUFF_* flags default off in
// the test baseline (see TestInitConfigData), so each test opts in explicitly.

ASSUMPTIONS
{
    ASSUME(GetMoveNonVolatileStatus(MOVE_POISON_POWDER) == MOVE_EFFECT_POISON);
    ASSUME(GetMoveNonVolatileStatus(MOVE_TOXIC) == MOVE_EFFECT_TOXIC);
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Poison Powder on a poisoned foe deals an immediate 1/8 tick")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_POWDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_POWDER, player);
        MESSAGE("The opposing Wobbuffet was hurt by its poisoning!");
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_PSN, opponent);
        HP_BAR(opponent, damage: 20);
        // ...and the normal end-turn poison still ticks.
        MESSAGE("The opposing Wobbuffet was hurt by its poisoning!");
        HP_BAR(opponent, damage: 20);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_POISON);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Poison Powder on a badly poisoned foe ticks 1/8 and leaves the Toxic counter alone")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Status1(STATUS1_TOXIC_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POISON_POWDER); }
    } SCENE {
        HP_BAR(opponent, damage: 10); // turn 1 end: counter 0 -> 1
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_POWDER, player);
        HP_BAR(opponent, damage: 20); // regular 1/8, counter untouched
        HP_BAR(opponent, damage: 20); // turn 2 end: counter 1 -> 2
    } THEN {
        EXPECT_EQ(opponent->status1 & STATUS1_TOXIC_COUNTER, STATUS1_TOXIC_TURN(2));
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Toxic on a badly poisoned foe advances the counter and ticks")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Status1(STATUS1_TOXIC_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        HP_BAR(opponent, damage: 10); // turn 1 end: counter 0 -> 1
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player);
        MESSAGE("The opposing Wobbuffet was hurt by its poisoning!");
        HP_BAR(opponent, damage: 20); // counter 1 -> 2
        HP_BAR(opponent, damage: 30); // turn 2 end: counter 2 -> 3
    } THEN {
        EXPECT_EQ(opponent->status1 & STATUS1_TOXIC_COUNTER, STATUS1_TOXIC_TURN(3));
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Toxic on a poisoned foe upgrades it to bad poison and ticks")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(160); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player);
        MESSAGE("The opposing Wobbuffet was badly poisoned!");
        MESSAGE("The opposing Wobbuffet was hurt by its poisoning!");
        HP_BAR(opponent, damage: 10); // Toxic's first turn
        HP_BAR(opponent, damage: 20); // end turn: counter 1 -> 2
    } THEN {
        EXPECT(opponent->status1 & STATUS1_TOXIC_POISON);
        EXPECT(!(opponent->status1 & STATUS1_POISON));
        EXPECT_EQ(opponent->status1 & STATUS1_TOXIC_COUNTER, STATUS1_TOXIC_TURN(2));
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON off: Poison Powder on a poisoned foe fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, FALSE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_POWDER); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_POWDER, player);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Magic Guard, Poison Heal and a Steel type still make it fail")
{
    u32 species, ability;
    PARAMETRIZE { species = SPECIES_CLEFABLE;  ability = ABILITY_MAGIC_GUARD; }
    PARAMETRIZE { species = SPECIES_GLISCOR;   ability = ABILITY_POISON_HEAL; }
    PARAMETRIZE { species = SPECIES_SKARMORY;  ability = ABILITY_KEEN_EYE; }
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(ability); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_TOXIC); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player);
    } THEN {
        EXPECT(opponent->status1 & STATUS1_POISON);
    }
}

SINGLE_BATTLE_TEST("BUFF_REPOISON: Corrosion re-poisons a poisoned Steel type")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        PLAYER(SPECIES_SALANDIT) { Ability(ABILITY_CORROSION); }
        OPPONENT(SPECIES_SKARMORY) { MaxHP(160); HP(160); Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_POISON_POWDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_POISON_POWDER, player);
        HP_BAR(opponent, damage: 20);
    }
}

AI_SINGLE_BATTLE_TEST("BUFF_REPOISON: the AI uses Toxic to upgrade a poisoned foe instead of a status move that fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_REPOISON, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC, MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_TOXIC); }
    }
}
