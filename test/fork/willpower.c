#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Willpower: Stamina's Sp. Def twin -- the holder's Sp. Def rises by 1 whenever a move hits it.
// A custom ability is a real ability, so the chosen-ability tests need no WITH_CONFIG; only the
// innate tests at the bottom toggle FEATURE_INNATE_ABILITIES.

SINGLE_BATTLE_TEST("Willpower raises Sp. Def by 1 when hit by a move")
{
    s16 turnOneHit, turnTwoHit;
    enum Move move;

    PARAMETRIZE { move = MOVE_SCRATCH; }
    PARAMETRIZE { move = MOVE_GUST; }

    GIVEN {
        ASSUME(GetMoveCategory(MOVE_GUST) == DAMAGE_CATEGORY_SPECIAL);
        ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, move); }
        TURN { MOVE(opponent, move); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        HP_BAR(player, captureDamage: &turnOneHit);
        ABILITY_POPUP(player, ABILITY_WILLPOWER);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
        MESSAGE("Wobbuffet's Sp. Def rose!");
        ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        HP_BAR(player, captureDamage: &turnTwoHit);
        ABILITY_POPUP(player, ABILITY_WILLPOWER);
        MESSAGE("Wobbuffet's Sp. Def rose!");
    } THEN {
        // The raise fires on any hit, but only the special hit is softened by it.
        if (move == MOVE_GUST)
            EXPECT_MUL_EQ(turnTwoHit, Q_4_12(1.5), turnOneHit);
        else
            EXPECT_EQ(turnTwoHit, turnOneHit);
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Willpower activates for every hit of a multi hit move")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_KICK, player);
        ABILITY_POPUP(opponent, ABILITY_WILLPOWER);
        MESSAGE("The opposing Wobbuffet's Sp. Def rose!");
        ABILITY_POPUP(opponent, ABILITY_WILLPOWER);
        MESSAGE("The opposing Wobbuffet's Sp. Def rose!");
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 2);
    }
}

SINGLE_BATTLE_TEST("Willpower does not activate when Sp. Def is already maxed")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_AMNESIA) == EFFECT_STAT_CHANGE); // +2 Sp. Def, three times = +6
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); Speed(2); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_AMNESIA); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_AMNESIA); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_AMNESIA); MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        NONE_OF { ABILITY_POPUP(player, ABILITY_WILLPOWER); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPDEF], MAX_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Willpower does not activate when the hit lands on a Substitute")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); Speed(2); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SUBSTITUTE); MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        NONE_OF { ABILITY_POPUP(player, ABILITY_WILLPOWER); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);
    }
}

DOUBLE_BATTLE_TEST("Willpower activates for every holder hit by a spread move")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_EARTHQUAKE) == TARGET_FOES_AND_ALLY);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); Speed(10); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_WILLPOWER); Speed(5); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(15); }
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_EARTHQUAKE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EARTHQUAKE, opponentLeft);
        ABILITY_POPUP(playerLeft, ABILITY_WILLPOWER);
        ABILITY_POPUP(playerRight, ABILITY_WILLPOWER);
    } THEN {
        EXPECT_EQ(playerLeft->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(playerRight->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(opponentLeft->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);
    }
}

// The innate tests use whichever species carries Willpower first, so they test the wiring rather
// than any one species row. The carrier list itself lives in src/fork/innate_abilities.c.
static enum Species FindWillpowerCarrier(void)
{
    enum Species species;
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (SpeciesHasInnate(species, ABILITY_WILLPOWER))
            return species;
    }
    return SPECIES_NONE;
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Willpower raises Sp. Def when the holder is hit")
{
    bool32 enabled;
    enum Species species = FindWillpowerCarrier();
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(species != SPECIES_NONE);
        ASSUME(gSpeciesInfo[species].abilities[0] != ABILITY_WILLPOWER);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(species) { Ability(gSpeciesInfo[species].abilities[0]); } // Willpower only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        if (enabled)
            ABILITY_POPUP(player, ABILITY_WILLPOWER); // pop-up shows the innate, not the chosen ability
        else
            NONE_OF { ABILITY_POPUP(player, ABILITY_WILLPOWER); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE + (enabled ? 1 : 0));
    }
}
