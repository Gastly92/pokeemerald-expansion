#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Pacify: Intimidate's Sp. Atk twin -- every opposing battler's Sp. Atk drops by 1 when the holder
// enters battle. A custom ability is a real ability, so the chosen-ability tests need no WITH_CONFIG;
// only the innate tests at the bottom toggle FEATURE_INNATE_ABILITIES.

ASSUMPTIONS
{
    ASSUME(GetMoveCategory(MOVE_GUST) == DAMAGE_CATEGORY_SPECIAL);
}

SINGLE_BATTLE_TEST("Pacify lowers the foe's Sp. Atk when the holder switches in", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PACIFY; }
    PARAMETRIZE { ability = ABILITY_SHED_SKIN; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ARBOK) { Ability(ability); }
    } WHEN {
        TURN { SWITCH(opponent, 1); }
        TURN { MOVE(player, MOVE_GUST); }
    } SCENE {
        if (ability == ABILITY_PACIFY) {
            ABILITY_POPUP(opponent, ABILITY_PACIFY);
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
            MESSAGE("Wobbuffet's Sp. Atk fell!");
        }
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - (ability == ABILITY_PACIFY ? 1 : 0));
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

DOUBLE_BATTLE_TEST("Pacify lowers both foes' Sp. Atk and leaves the holder's partner alone")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_PACIFY); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_PACIFY);
        MESSAGE("The opposing Wobbuffet's Sp. Atk fell!");
        MESSAGE("The opposing Wobbuffet's Sp. Atk fell!");
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(opponentRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(playerRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Pacify is blocked by Clear Body like any other stat drop")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_PACIFY); }
        OPPONENT(SPECIES_METANG) { Ability(ABILITY_CLEAR_BODY); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PACIFY);
        ABILITY_POPUP(opponent, ABILITY_CLEAR_BODY);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Pacify triggers the foe's Competitive like any other stat drop")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_PACIFY); }
        OPPONENT(SPECIES_MILOTIC) { Ability(ABILITY_COMPETITIVE); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PACIFY);
        MESSAGE("The opposing Milotic's Sp. Atk fell!");
        ABILITY_POPUP(opponent, ABILITY_COMPETITIVE);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - 1 + 2);
    }
}

// Rattled reacts to Intimidate by name; Pacify is an ordinary stat drop, so it does not.
SINGLE_BATTLE_TEST("Pacify does not trigger the foe's Rattled")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_PACIFY); }
        OPPONENT(SPECIES_SUDOWOODO) { Ability(ABILITY_RATTLED); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PACIFY);
        MESSAGE("The opposing Sudowoodo's Sp. Atk fell!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_RATTLED); }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

// The innate tests use whichever species carries Pacify first, so they test the wiring rather than
// any one species row. The carrier list itself lives in src/fork/innate_abilities.c.
static enum Species FindPacifyCarrier(void)
{
    enum Species species;
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (SpeciesHasInnate(species, ABILITY_PACIFY))
            return species;
    }
    return SPECIES_NONE;
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Pacify lowers the foe's Sp. Atk on entry")
{
    bool32 enabled;
    enum Species species = FindPacifyCarrier();
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(species != SPECIES_NONE);
        ASSUME(gSpeciesInfo[species].abilities[0] != ABILITY_PACIFY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(species) { Ability(gSpeciesInfo[species].abilities[0]); } // Pacify only as innate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_PACIFY); // pop-up shows the innate, not the chosen ability
            MESSAGE("The opposing Wobbuffet's Sp. Atk fell!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_PACIFY); }
        }
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE - (enabled ? 1 : 0));
    }
}
