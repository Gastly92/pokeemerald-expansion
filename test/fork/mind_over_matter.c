#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Mind Over Matter (src/fork/mind_over_matter.c): on the holder's entry turn it survives one
// lethal hit at 1 HP, from any HP. A real ability, so the Ability(...) tests need no WITH_CONFIG;
// the window (IsBattlersEntryTurn) is tracked with DETERMINISTIC_HOLD_EFFECTS off, which is the
// test baseline.

SINGLE_BATTLE_TEST("Mind Over Matter endures a lethal hit on the entry turn from any HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, hp: 1);
        ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER);
        MESSAGE("The opposing Wobbuffet endured the hit!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Mind Over Matter does not save after the entry turn")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER); }
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Mind Over Matter saves only once per entry: a multi-hit move's next strike KOs")
{
    GIVEN {
        ASSUME(GetMoveStrikeCount(MOVE_DOUBLE_KICK) == 2);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER);
        MESSAGE("The opposing Wobbuffet fainted!");
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Mind Over Matter saves on the switch-in turn itself")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ZIGZAGOON);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SWITCH(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER);
        MESSAGE("The opposing Wobbuffet endured the hit!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Mind Over Matter is bypassed by Mold Breaker")
{
    GIVEN {
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER); }
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
    }
}

SINGLE_BATTLE_TEST("Mind Over Matter fires before a held Focus Band, which is kept")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(gItemsInfo[ITEM_FOCUS_BAND].holdEffect == HOLD_EFFECT_FOCUS_BAND);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MIND_OVER_MATTER); Item(ITEM_FOCUS_BAND); MaxHP(200); HP(5); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER);
        NONE_OF { MESSAGE("The opposing Wobbuffet hung on using its Focus Band!"); }
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
        EXPECT_EQ(opponent->item, ITEM_FOCUS_BAND);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mew's innate Mind Over Matter endures a lethal hit on entry")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEW, ABILITY_MIND_OVER_MATTER));
        ASSUME(GetMoveEffect(MOVE_SEISMIC_TOSS) == EFFECT_LEVEL_DAMAGE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_MEW) { HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SEISMIC_TOSS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SEISMIC_TOSS, player);
        if (enabled) {
            ABILITY_POPUP(opponent, ABILITY_MIND_OVER_MATTER); // the innate, not the chosen Synchronize
            MESSAGE("The opposing Mew endured the hit!");
        } else {
            MESSAGE("The opposing Mew fainted!");
        }
    } THEN {
        EXPECT_EQ(opponent->hp, enabled ? 1 : 0);
    }
}
