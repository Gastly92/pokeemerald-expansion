#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Psybubble (src/fork/psybubble.c): on the holder's entry turn it survives one
// lethal hit at 1 HP, from any HP. A real ability, so the Ability(...) tests need no WITH_CONFIG;
// the window (IsBattlersEntryTurn) is tracked with DETERMINISTIC_HOLD_EFFECTS off, which is the
// test baseline.

SINGLE_BATTLE_TEST("Psybubble endures a lethal hit on the entry turn from any HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, hp: 1);
        ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE);
        MESSAGE("The opposing Wobbuffet endured the hit!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Psybubble does not save after the entry turn")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE); }
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Psybubble saves only once per entry: a multi-hit move's next strike KOs")
{
    GIVEN {
        ASSUME(GetMoveStrikeCount(MOVE_DOUBLE_KICK) == 2);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE);
        MESSAGE("The opposing Wobbuffet fainted!");
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Psybubble saves on the switch-in turn itself")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ZIGZAGOON);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE);
        MESSAGE("The opposing Wobbuffet endured the hit!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Psybubble is bypassed by Mold Breaker")
{
    GIVEN {
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE); }
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Psybubble fires before a held Focus Band, which is kept")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(gItemsInfo[ITEM_FOCUS_BAND].holdEffect == HOLD_EFFECT_FOCUS_BAND);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PSYBUBBLE); Item(ITEM_FOCUS_BAND); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE);
        NONE_OF { MESSAGE("The opposing Wobbuffet hung on using its Focus Band!"); }
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
        EXPECT_EQ(opponent->item, ITEM_FOCUS_BAND);
    }
}

// The innate test uses whichever species carries Psybubble first, so it tests the wiring rather
// than any one species row. The carrier list itself lives in src/fork/innate_abilities.c. Ghost
// types are skipped because they are immune to the Seismic Toss used below.
static enum Species FindPsybubbleCarrier(void)
{
    enum Species species;
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (SpeciesHasInnate(species, ABILITY_PSYBUBBLE)
         && gSpeciesInfo[species].types[0] != TYPE_GHOST
         && gSpeciesInfo[species].types[1] != TYPE_GHOST)
            return species;
    }
    return SPECIES_NONE;
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Psybubble endures a lethal hit on entry")
{
    bool32 enabled;
    enum Species species = FindPsybubbleCarrier();
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(species != SPECIES_NONE);
        ASSUME(gSpeciesInfo[species].abilities[0] != ABILITY_PSYBUBBLE);
        ASSUME(GetMoveEffect(MOVE_SEISMIC_TOSS) == EFFECT_LEVEL_DAMAGE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(species) { Ability(gSpeciesInfo[species].abilities[0]); HP(1); } // Psybubble only as innate
    } WHEN {
        TURN { MOVE(player, MOVE_SEISMIC_TOSS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, player);
        if (enabled)
            ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE); // pop-up shows the innate, not the chosen ability
        else
            NONE_OF { ABILITY_POPUP(opponent, ABILITY_PSYBUBBLE); }
    } THEN {
        EXPECT_EQ(opponent->hp, enabled ? 1 : 0);
    }
}
