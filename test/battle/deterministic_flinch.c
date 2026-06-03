#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_FLINCH flag (config/deterministic.h), which
// composes with DETERMINISTIC_ADDITIONAL_EFFECTS. Determinism flags default off in
// the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG. With both flags on (the shipped config), a flinch additional effect
// is first gated on super effective / STAB like any other effect, and then capped
// so it can't be re-applied to a foe that flinched last turn (anti flinch-lock).
// Bite (Dark, super effective vs Psychic) and Iron Head (Steel, neutral vs
// Psychic) are both 100% accuracy / 30% flinch, so accuracy never confounds the
// result and the only difference is the type matchup.

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: a super-effective-capable flinch move flinches only on a super effective hit")
{
    u32 move;
    bool32 flinches;
    PARAMETRIZE { move = MOVE_BITE;      flinches = TRUE;  } // Dark is super effective vs Psychic
    PARAMETRIZE { move = MOVE_IRON_HEAD; flinches = FALSE; } // Steel is neutral vs Psychic
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(move, MOVE_EFFECT_FLINCH));
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Attack(1); Speed(100); Moves(MOVE_BITE, MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        if (flinches)
            MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        else
            MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: a Normal flinch move flinches only from a STAB user")
{
    u32 species;
    bool32 flinches;
    // Stomp (Normal) can never be super effective, so it is gated on STAB.
    PARAMETRIZE { species = SPECIES_SNORLAX;   flinches = TRUE;  } // Normal: STAB
    PARAMETRIZE { species = SPECIES_WOBBUFFET; flinches = FALSE; } // Psychic: not STAB
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_STOMP, MOVE_EFFECT_FLINCH));
        ASSUME(GetMoveType(MOVE_STOMP) == TYPE_NORMAL);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(species) { Attack(1); Speed(100); Moves(MOVE_STOMP); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_STOMP); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        if (flinches)
            MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        else
            MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: a gated flinch still cannot lock a foe that flinched last turn")
{
    // Bite is super effective every turn, so the only thing stopping a second flinch
    // is the anti-lock cap.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Attack(1); Speed(100); Moves(MOVE_BITE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_BITE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_BITE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 1: super effective, not flinched last turn -> flinched.
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        // Turn 2: still super effective, but flinched last turn -> it gets to move.
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: Serene Grace guarantees a flinch even on a non-gated hit, but still cannot lock")
{
    // Serene Grace guarantees ALL secondaries, flinch included, so a neutral Iron Head
    // flinches despite failing the super-effective/STAB gate. The anti-lock cap still
    // applies, so the foe can't be flinched again the very next turn.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SERENE_GRACE); Attack(1); Speed(100); Moves(MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(600); HP(600); Defense(255); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 1: Serene Grace guarantees the flinch despite the neutral hit.
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        // Turn 2: flinched last turn -> anti-lock cap, it gets to move.
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: Fake Out (a guaranteed first-turn flincher) bypasses the gate and the anti-lock")
{
    // Fake Out is 100% flinch and first-turn-only, so it always flinches regardless
    // of the super-effective/STAB gate (Normal from a Psychic user, neutral target)
    // and the anti-lock rule.
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_FAKE_OUT); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}
