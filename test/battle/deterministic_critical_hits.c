#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_CRITICAL_HITS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE). The stock
// random-rate crit tests run unmodified (determinism off) in crit_chance.c etc.

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: unmodified moves never land a random critical hit")
{
    // Stock behavior is ~6.25% here (see crit_chance.c); deterministically it is 0%.
    PASSES_RANDOMLY(0, 16, RNG_CRITICAL_HIT);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: a high but sub-100% crit ratio still never crits")
{
    // Focus Energy raises the crit stage to a partial-chance tier (not 1/1), so
    // it must never crit under the flag even though stock would roll for it.
    PASSES_RANDOMLY(0, 16, RNG_CRITICAL_HIT);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveEffect(MOVE_FOCUS_ENERGY) == EFFECT_FOCUS_ENERGY);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_FOCUS_ENERGY); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: always-crit moves still crit")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(MoveAlwaysCrits(MOVE_STORM_THROW));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_STORM_THROW); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: Laser Focus still guarantees a crit")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveEffect(MOVE_LASER_FOCUS) == EFFECT_LASER_FOCUS);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_LASER_FOCUS); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: Merciless still guarantees a crit on a poisoned target")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_MAREANIE) { Ability(ABILITY_MERCILESS); }
        OPPONENT(SPECIES_WOBBUFFET) { Status1(STATUS1_POISON); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: stacking the crit ratio to 1/1 still crits")
{
    // Slash (+1) + Super Luck (+1) + Scope Lens (+1) = stage 3, which is 1/1 in
    // Gen6+. A guaranteed crit from stacking is not random, so it still lands.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        WITH_CONFIG(B_CRIT_CHANCE, GEN_LATEST);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(gItemsInfo[ITEM_SCOPE_LENS].holdEffect == HOLD_EFFECT_SCOPE_LENS);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

