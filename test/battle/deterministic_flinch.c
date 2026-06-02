#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_FLINCH flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE). Under the flag a
// move's flinch additional effect lands every time, except when the target was
// flinched on the previous turn (anti flinch-lock). Iron Head (100% accuracy, 30%
// flinch) is used so accuracy never confounds the result.

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: a flinch move flinches a target that was not flinched last turn")
{
    // Stock is a 30% flinch here; deterministically it is guaranteed on a fresh target.
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_IRON_HEAD, MOVE_EFFECT_FLINCH));
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: a target flinched last turn cannot be flinched again (no flinch-lock)")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 1: flinched (so its Celebrate never fires this turn).
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        // Turn 2: was flinched last turn, so it is NOT flinched again — it gets to move.
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: flinch is governed by the flinch rule, not super-effectiveness")
{
    // With DETERMINISTIC_ADDITIONAL_EFFECTS also on, a flinch effect is still routed
    // through DETERMINISTIC_FLINCH (not the super-effective/STAB rule), so Iron Head
    // flinches a neutral target it is not super effective against.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_IRON_HEAD); } // Steel vs Psychic = 1x
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: Serene Grace does not lift the anti flinch-lock rule")
{
    // The secondary-chance boosters guarantee non-flinch effects, but flinch keeps
    // its own anti-lock rule, so a Serene Grace flincher still can't flinch a target
    // it flinched last turn.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SERENE_GRACE); Speed(100); Moves(MOVE_IRON_HEAD); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_IRON_HEAD); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // Turn 1: flinched.
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        // Turn 2: was flinched last turn, so Serene Grace notwithstanding it moves.
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_FLINCH: Fake Out is exempt from the anti-lock rule and flinches every turn it can be used")
{
    // Fake Out only works the turn its user switches in, so it can never chain
    // flinches; it is exempt from the not-flinched-last-turn rule. After a switch,
    // a fresh Fake Out flinches again even though the target was flinched recently.
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FAKE_OUT) == EFFECT_FIRST_TURN_ONLY);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_FAKE_OUT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    }
}
