#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"
#include "fork/rekindle.h"

// Rekindle: each battler the holder knocks out with a move heals it for 1/4 of its max HP.
// A custom ability is a real ability, so the chosen-ability tests need no WITH_CONFIG; only the
// innate test at the bottom toggles FEATURE_INNATE_ABILITIES.

SINGLE_BATTLE_TEST("Rekindle heals 1/4 of max HP when the holder knocks out a foe")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so the KO doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        ABILITY_POPUP(player, ABILITY_REKINDLE);
        MESSAGE("Wobbuffet's fighting spirit was rekindled!");
    } THEN {
        EXPECT_EQ(player->hp, 1 + player->maxHP / REKINDLE_HEAL_FRACTION);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate at full HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_REKINDLE);
            MESSAGE("Wobbuffet's fighting spirit was rekindled!");
        }
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

SINGLE_BATTLE_TEST("Rekindle never heals past max HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); MaxHP(100); HP(99); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_REKINDLE);
    } THEN {
        EXPECT_EQ(player->hp, 100);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate under Heal Block")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_HEAL_BLOCK) == EFFECT_HEAL_BLOCK);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Speed(1); Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(2); Moves(MOVE_HEAL_BLOCK, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(2); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_HEAL_BLOCK); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate when the knockout ends the battle")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    }
}

// Like Moxie, every battler the move knocks out counts -- the holder's own ally included.
DOUBLE_BATTLE_TEST("Rekindle heals once for each battler the holder knocks out, allies included")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_EARTHQUAKE) == TARGET_FOES_AND_ALLY);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); }
        PLAYER(SPECIES_SNORUNT) { HP(1); }
        OPPONENT(SPECIES_GLALIE) { HP(1); }
        OPPONENT(SPECIES_ABRA) { HP(1); }
        OPPONENT(SPECIES_ABRA);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_EARTHQUAKE); SEND_OUT(opponentLeft, 2); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EARTHQUAKE, playerLeft);
        ABILITY_POPUP(playerLeft, ABILITY_REKINDLE);
        MESSAGE("Wobbuffet's fighting spirit was rekindled!");
    } THEN {
        EXPECT_EQ(playerLeft->hp, 1 + 3 * (playerLeft->maxHP / REKINDLE_HEAL_FRACTION));
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate when a foe faints to indirect damage")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rekindle heals Charizard when it knocks out a foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CHARIZARD, ABILITY_REKINDLE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CHARIZARD) { Ability(ABILITY_BLAZE); HP(1); Moves(MOVE_SCRATCH); } // chosen Blaze; Rekindle only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_REKINDLE); // pop-up shows the innate, not the chosen Blaze
            MESSAGE("Charizard's fighting spirit was rekindled!");
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
        }
    } THEN {
        EXPECT_EQ(player->hp, enabled ? 1 + player->maxHP / REKINDLE_HEAL_FRACTION : 1);
    }
}

TEST("Rekindle is an innate of Charmeleon and every Charizard form")
{
    EXPECT(SpeciesHasInnate(SPECIES_CHARMELEON, ABILITY_REKINDLE));
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD, ABILITY_REKINDLE));
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD_MEGA_X, ABILITY_REKINDLE));
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD_MEGA_Y, ABILITY_REKINDLE));
    EXPECT(SpeciesHasInnate(SPECIES_CHARIZARD_GMAX, ABILITY_REKINDLE));
}
