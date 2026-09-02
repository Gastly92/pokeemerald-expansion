#include "global.h"
#include "test/battle.h"
#include "fork/species_ability_overrides.h"

// FORK: behavioural coverage for the fork-owned ability override table
// (src/fork/species_ability_overrides.c). The table is gated by
// FEATURE_INNATE_ABILITIES, which TestInitConfigData force-disables, so every
// test here opts in with WITH_CONFIG.
//
// The rule these tests protect: a Mega form's override row is read at the mon's
// EXISTING ability slot, so it silently rewrites the ability of every set that
// resolves to that slot. Where the row names Sheer Force, that deletes the
// additional effect of every move the set carries -- which is fatal for Fake Out,
// whose 40 BP makes the 1.3x boost worthless and whose flinch is the whole move.

SINGLE_BATTLE_TEST("Ability overrides: a Klutz Lopunny keeps Fake Out's flinch through Mega Evolution")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_LOPUNNY, 1) == ABILITY_KLUTZ);
        PLAYER(SPECIES_LOPUNNY) { Ability(ABILITY_KLUTZ); Item(ITEM_TOXIC_ORB); Moves(MOVE_FAKE_OUT); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT, gimmick: GIMMICK_MEGA); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FAKE_OUT, player);
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(player->species, SPECIES_LOPUNNY_MEGA);
        EXPECT_EQ(player->ability, ABILITY_KLUTZ);
    }
}

// The other half of the same table: slot 2 is where the roster's deliberate Sheer
// Force Lopunny lives, so that set must still come out of the Mega with Sheer Force.
SINGLE_BATTLE_TEST("Ability overrides: a Sheer Force Lopunny keeps Sheer Force through Mega Evolution")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(GetSpeciesAbility(SPECIES_LOPUNNY, 2) == ABILITY_SHEER_FORCE);
        PLAYER(SPECIES_LOPUNNY) { Ability(ABILITY_SHEER_FORCE); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_LOPUNNY_MEGA);
        EXPECT_EQ(player->ability, ABILITY_SHEER_FORCE);
    }
}

// Klutz suppresses hold EFFECTS, but Mega Evolution reads the raw held item
// (GetBattleFormChangeTargetSpecies), so the Battle Tower's Klutz + Lopunnite boss
// set still transforms. Guards the .ability repoint in src/fork/battle_tower_trainers.c.
SINGLE_BATTLE_TEST("Ability overrides: Klutz does not block a held Mega Stone")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_LOPUNNY) { Ability(ABILITY_KLUTZ); Item(ITEM_LOPUNNITE); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_LOPUNNY_MEGA);
    }
}
