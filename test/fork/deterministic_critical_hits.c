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
    // A lone high-crit move (Slash, +1 = stage 1 = 1/8) is a partial-chance tier, not
    // 1/1, so it must never crit under the flag even though stock would roll for it.
    PASSES_RANDOMLY(0, 16, RNG_CRITICAL_HIT);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: Focus Energy arms a one-shot crit on the next attack only")
{
    // Focus Energy is only +2 (stage 2, 1/2) so stock would still roll -- but under the
    // flag it arms a guaranteed crit on the NEXT attack (reusing Laser Focus's volatile),
    // which then clears, so the attack after it no longer crits.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveEffect(MOVE_FOCUS_ENERGY) == EFFECT_FOCUS_ENERGY);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); MaxHP(600); HP(600); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_FOCUS_ENERGY); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 2: the armed next-attack crit lands.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        MESSAGE("A critical hit!");
        // Turn 3: the volatile has cleared, so no crit follows.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        NONE_OF { MESSAGE("A critical hit!"); }
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: Super Luck guarantees a crit only on the user's first turn")
{
    // Mirrors the fork's deterministic Stench: a guaranteed proc on the first turn out,
    // then nothing on later turns (the lone +1 crit stage never reaches 1/1 on its own).
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_ABSOL) { Ability(ABILITY_SUPER_LUCK); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        // Turn 1 (first turn out): guaranteed crit.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        MESSAGE("A critical hit!");
        // Turn 2 (no longer the first turn): Scratch lands but no crit follows.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        NONE_OF { MESSAGE("A critical hit!"); }
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
    // Fired on turn 2 so it isolates crit-stacking from Super Luck's first-turn crit.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        WITH_CONFIG(B_CRIT_CHANCE, GEN_LATEST);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(gItemsInfo[ITEM_SCOPE_LENS].holdEffect == HOLD_EFFECT_SCOPE_LENS);
        PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SLASH); }
    } SCENE {
        MESSAGE("A critical hit!");
    }
}

