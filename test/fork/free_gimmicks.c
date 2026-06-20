#include "global.h"
#include "test/battle.h"

// FORK: coverage for FEATURE_FREE_GIMMICKS (config/feature.h). The flag drops the
// held-item / key-item / charge requirements for the battle transformation
// gimmicks and lets a mon pick from several via the move-selection picker. The
// flag defaults off in the test baseline (see TestInitConfigData) so the inherited
// gimmick suite keeps exercising the stock item-gated behavior; each test here
// opts in with WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE).

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a mon can Mega Evolve without holding a Mega Stone")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(gSpeciesInfo[SPECIES_VENUSAUR].isMegaEvolution == FALSE);
        PLAYER(SPECIES_VENUSAUR); // no item
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
        MESSAGE("Venusaur has Mega Evolved into Mega Venusaur!");
    } THEN {
        EXPECT_EQ(player->species, SPECIES_VENUSAUR_MEGA);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a physically inclined mon picks the X Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(200); SpAttack(100); } // physical -> X
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_X);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a specially inclined mon picks the Y Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(100); SpAttack(200); } // special -> Y
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_Y);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: an equal-stat mon defaults to the X Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(150); SpAttack(150); } // tie -> X
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_X);
    }
}

// FORK regression: with item-free Mega Evolution the held Mega Stone no longer gates
// the gimmick, which previously let a transformed mon (Ditto/Imposter, which copies its
// target's species but not its item) arm Mega Evolution. The form change is then refused
// by CanBattlerFormChange (B_TRANSFORM_FORM_CHANGES >= GEN_5) AFTER the gimmick was marked
// used, so nothing happened yet the Mega was spent. CanMegaEvolve now blocks transformed
// mons up front, so the option is never offered and the gimmick is preserved.
SINGLE_BATTLE_TEST("FREE_GIMMICKS: a transformed mon cannot Mega Evolve")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(GetConfig(B_TRANSFORM_FORM_CHANGES) >= GEN_5);
        // Transform copies the target's moves into battle while the move-slot the
        // test resolves comes from Ditto's party moveset, so align Celebrate to the
        // same slot (1) in both so turn 2's MOVE is legal after transforming.
        PLAYER(SPECIES_DITTO) { Moves(MOVE_TRANSFORM, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_VENUSAUR) { Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRANSFORM); }
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } SCENE {
        MESSAGE("Ditto transformed into Venusaur!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
            MESSAGE("Venusaur has Mega Evolved into Mega Venusaur!");
        }
    } THEN {
        // Stays the transformed (non-Mega) Venusaur; the Mega gimmick is never consumed.
        EXPECT_EQ(player->species, SPECIES_VENUSAUR);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a mon can use a Z-Move without holding a Z-Crystal")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_PIKACHU) { Moves(MOVE_THUNDERBOLT); } // no item
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        MESSAGE("Pikachu surrounded itself with its Z-Power!");
        MESSAGE("Pikachu unleashes its full-force Z-Move!");
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: the player can Terastallize without a Tera Orb")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { TeraType(TYPE_GHOST); } // no Tera Orb / charge flag
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } SCENE {
        MESSAGE("Wobbuffet terastallized into the Ghost type!");
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: the player can Dynamax without a Dynamax Band")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); } // no Dynamax Band / B_FLAG
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        // Dynamax turns the chosen move into its Max Move.
        MESSAGE("Wobbuffet used Max Strike!");
    }
}
