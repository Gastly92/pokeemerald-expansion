#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_PARALYSIS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE). The stock
// paralysis behavior (random full-paralysis miss + Speed cut) runs unmodified
// (determinism off) in status1/paralysis.c.

ASSUMPTIONS
{
    ASSUME(GetMovePriority(MOVE_QUICK_ATTACK) == 1);
    ASSUME(GetMovePP(MOVE_POUND) == 35);
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: a paralyzed battler never loses its turn")
{
    // Stock is a 25% miss here (see status1/paralysis.c); deterministically it is 0%.
    PASSES_RANDOMLY(0, 100, RNG_PARALYSIS);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet couldn't move because it's paralyzed!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: paralysis no longer cuts Speed")
{
    // The player's Quick Attack is dropped to priority 0 by paralysis (see the
    // priority test below), tying the opponent's priority-0 Celebrate, so turn
    // order comes down to Speed alone. Full Speed 100 beats the opponent's 70; the
    // stock Gen 7+ halving (to 50) would let the opponent move first, so the player
    // moving first proves the Speed cut is gone.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        WITH_CONFIG(B_PARALYSIS_SPEED, GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); Speed(100); Moves(MOVE_QUICK_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(70); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_QUICK_ATTACK); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Quick Attack!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: paralysis costs every move 1 extra PP")
{
    u32 status;
    u32 expectedPP;
    PARAMETRIZE { status = STATUS1_NONE;      expectedPP = 34; } // 35 - 1 (normal)
    PARAMETRIZE { status = STATUS1_PARALYSIS; expectedPP = 33; } // 35 - 1 - 1 (paralyzed)
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(status); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: paralysis lowers move priority by 1")
{
    // Both use Quick Attack (+1). The paralyzed player is faster (100 vs 50), so
    // if priorities were equal it would move first; instead its priority drops to
    // 0 while the opponent's stays +1, so the slower opponent moves first.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_PARALYSIS); Speed(100); Moves(MOVE_QUICK_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_QUICK_ATTACK); }
    } WHEN {
        TURN { MOVE(player, MOVE_QUICK_ATTACK); MOVE(opponent, MOVE_QUICK_ATTACK); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Quick Attack!");
        MESSAGE("Wobbuffet used Quick Attack!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: Quick Feet is exempt from the priority tax")
{
    // A normal paralyzed mon's priority-0 move drops to -1 and would lose to the
    // opponent's priority-0 Growl regardless of Speed. Quick Feet is exempt, so the
    // player's Celebrate stays at priority 0 and its (Quick-Feet-boosted) Speed
    // beats the opponent — moving first proves the priority drop did not apply.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_QUICK_FEET); Status1(STATUS1_PARALYSIS); Speed(100); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(70); Moves(MOVE_GROWL); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_GROWL); }
    } SCENE {
        MESSAGE("Wobbuffet used Celebrate!");
        MESSAGE("The opposing Wobbuffet used Growl!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_PARALYSIS: Quick Feet is exempt from the PP tax")
{
    // Quick Feet shrugs off the paralysis taxes, so its move costs the normal 1 PP
    // (35 - 1 = 34) rather than the paralyzed 2 (35 - 1 - 1 = 33).
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_PARALYSIS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_QUICK_FEET); Status1(STATUS1_PARALYSIS); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 34);
    }
}
