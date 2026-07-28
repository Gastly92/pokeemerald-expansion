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

DOUBLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: Dragon Cheer arms a one-shot crit on the ally's next attack only")
{
    // Like Focus Energy, Dragon Cheer arms the ally's next attack (reusing Laser Focus's
    // volatile) instead of a dead crit-stage boost. Parametrized over both arming branches:
    // a non-Dragon ally gets the +1 (dragonCheer volatile), a Dragon ally the +2 (focusEnergy).
    u32 allySpecies;
    PARAMETRIZE { allySpecies = SPECIES_WOBBUFFET; } // non-Dragon: dragonCheer volatile
    PARAMETRIZE { allySpecies = SPECIES_DRATINI; }   // Dragon: focusEnergy volatile
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveEffect(MOVE_DRAGON_CHEER) == EFFECT_DRAGON_CHEER);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_DRAGON_CHEER, MOVE_CELEBRATE); }
        PLAYER(allySpecies) { Speed(99); MaxHP(600); HP(600); Moves(MOVE_SCRATCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_CHEER, target: playerRight); MOVE(playerRight, MOVE_CELEBRATE);
               MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_SCRATCH, target: opponentLeft);
               MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_SCRATCH, target: opponentLeft);
               MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 1: the ally is cheered but doesn't attack.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_CHEER, playerLeft);
        // Turn 2: the armed next-attack crit lands.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerRight);
        MESSAGE("A critical hit!");
        // Turn 3: the volatile has cleared, so no crit follows.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerRight);
        NONE_OF { MESSAGE("A critical hit!"); }
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: a typed high-crit move crits only on a super effective hit, not on STAB")
{
    // High-crit moves reuse the deterministic secondary-effect gate: a type that CAN be
    // super effective (Dark) crits only on a super effective hit -- STAB alone is not enough.
    u32 playerSpecies, targetSpecies;
    bool32 crits;
    PARAMETRIZE { playerSpecies = SPECIES_WOBBUFFET; targetSpecies = SPECIES_WOBBUFFET; crits = TRUE;  } // non-Dark user, Psychic target -> super effective
    PARAMETRIZE { playerSpecies = SPECIES_UMBREON;   targetSpecies = SPECIES_ZANGOOSE; crits = FALSE; } // Dark STAB user, neutral target -> no crit
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveCriticalHitStage(MOVE_NIGHT_SLASH) == 1);
        ASSUME(GetMoveType(MOVE_NIGHT_SLASH) == TYPE_DARK);
        PLAYER(playerSpecies) { Speed(100); }
        OPPONENT(targetSpecies) { Speed(1); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_NIGHT_SLASH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NIGHT_SLASH, player);
        if (crits)
            MESSAGE("A critical hit!");
        else
            NONE_OF { MESSAGE("A critical hit!"); }
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_CRITICAL_HITS: a Normal high-crit move crits from a STAB user only")
{
    // Normal can never be super effective, so its high-crit moves fall back to the STAB
    // gate (matching Normal-type deterministic secondary effects): Slash crits from a
    // Normal user, and does nothing from an off-type user.
    u32 playerSpecies;
    bool32 crits;
    PARAMETRIZE { playerSpecies = SPECIES_ZANGOOSE;  crits = TRUE;  } // Normal: STAB Slash -> crit
    PARAMETRIZE { playerSpecies = SPECIES_WOBBUFFET; crits = FALSE; } // Psychic: no STAB, Normal never SE -> no crit
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(GetMoveCriticalHitStage(MOVE_SLASH) == 1);
        ASSUME(GetMoveType(MOVE_SLASH) == TYPE_NORMAL);
        PLAYER(playerSpecies) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SLASH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SLASH, player);
        if (crits)
            MESSAGE("A critical hit!");
        else
            NONE_OF { MESSAGE("A critical hit!"); }
    }
}

