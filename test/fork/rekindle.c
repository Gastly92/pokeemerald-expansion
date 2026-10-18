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
        HP_BAR(player);
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
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
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
        HP_BAR(playerLeft);
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

// The innate tests use whichever species carries Rekindle first, so they test the wiring rather
// than any one species row. The carrier list itself lives in src/fork/innate_abilities.c.
static enum Species FindRekindleCarrier(void)
{
    enum Species species;
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (SpeciesHasInnate(species, ABILITY_REKINDLE))
            return species;
    }
    return SPECIES_NONE;
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rekindle heals the holder when it knocks out a foe")
{
    bool32 enabled;
    enum Species species = FindRekindleCarrier();
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(species != SPECIES_NONE);
        ASSUME(gSpeciesInfo[species].abilities[0] != ABILITY_REKINDLE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(species) { Ability(gSpeciesInfo[species].abilities[0]); HP(1); Moves(MOVE_SCRATCH); } // Rekindle only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_REKINDLE); // pop-up shows the innate, not the chosen ability
            HP_BAR(player);
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
        }
    } THEN {
        EXPECT_EQ(player->hp, enabled ? 1 + player->maxHP / REKINDLE_HEAL_FRACTION : 1);
    }
}

// Sheer Force skips the move-end steps that hold the KO abilities; the innate step is detoured
// back in by RedirectSheerForceSkipToInnates(), so an innate Rekindle still fires.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rekindle still heals after a Sheer Force boosted KO")
{
    enum Species species = FindRekindleCarrier();
    GIVEN {
        ASSUME(species != SPECIES_NONE);
        ASSUME(MoveIsAffectedBySheerForce(MOVE_FLAMETHROWER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(species) { Ability(ABILITY_SHEER_FORCE); HP(1); Moves(MOVE_FLAMETHROWER); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); SEND_OUT(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_REKINDLE);
        HP_BAR(player);
    } THEN {
        EXPECT_EQ(player->hp, 1 + player->maxHP / REKINDLE_HEAL_FRACTION);
    }
}
