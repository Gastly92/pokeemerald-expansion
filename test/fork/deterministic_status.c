#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_STATUS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_STATUS, TRUE). Stock infatuation/
// sleep/confusion behavior runs unmodified (determinism off) in the inherited suite.

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: Attract ignores gender and never immobilizes the target")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); Moves(MOVE_ATTRACT); }
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ATTRACT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Attract!");
        // Same-gender infatuation still lands (gender requirement dropped) ...
        MESSAGE("The opposing Wobbuffet is in love with Wobbuffet!");
        // ... and the infatuated foe still acts (no 50% immobilize roll).
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: infatuation wears off after a fixed number of turns")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(100); Moves(MOVE_ATTRACT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Gender(MON_MALE); Speed(50); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ATTRACT); MOVE(opponent, MOVE_CELEBRATE); }   // infatuated; lasts turn 1
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); } // lasts turn 2
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); } // cured before acting
    } SCENE {
        MESSAGE("The opposing Wobbuffet got over its infatuation!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: an infatuated attacker deals reduced damage to the loved target")
{
    s16 damage[2];
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        // Compare two hits by the same attacker in one battle, with the damage roll
        // pinned identically and crits off, so the only difference is the infatuation
        // applied before the second hit.
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_CHANSEY) { Speed(100); Moves(MOVE_CELEBRATE, MOVE_ATTRACT); } // bulky enough to survive both hits
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0), criticalHit: FALSE); } // not infatuated
        TURN { MOVE(opponent, MOVE_ATTRACT); MOVE(player, MOVE_TACKLE, WITH_RNG(RNG_DAMAGE_MODIFIER, 0), criticalHit: FALSE); }   // infatuated this turn
    } SCENE {
        HP_BAR(opponent, captureDamage: &damage[0]);
        MESSAGE("The opposing Chansey used Attract!");
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        // The infatuated hit deals roughly half the normal one.
        EXPECT_MUL_EQ(damage[1], Q_4_12(2.0), damage[0]);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: sleep always lasts a fixed number of turns")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_BRELOOM) { Speed(100); Moves(MOVE_SPORE); }
    } WHEN {
        // DETERMINISTIC_SLEEP_TURNS == 3, and the counter is decremented when the sleeper
        // tries to act, so the target misses 3 - 1 == 2 actions and acts on the wake turn.
        TURN { MOVE(opponent, MOVE_SPORE); MOVE(player, MOVE_CELEBRATE); } // asleep, 1st missed action
        TURN { MOVE(player, MOVE_CELEBRATE); }                             // asleep, 2nd missed action
        TURN { MOVE(player, MOVE_CELEBRATE); }                             // wakes AND acts
    } SCENE {
        MESSAGE("Wobbuffet is fast asleep.");
        MESSAGE("Wobbuffet is fast asleep.");
        MESSAGE("Wobbuffet woke up!");
        MESSAGE("Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: a confused battler self-hits once but still uses its move, then snaps out next turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY, MOVE_CELEBRATE); }
    } WHEN {
        // The fix: the confused battler takes its one-time self-hit but its chosen move
        // STILL executes (old behavior denied the move, which a faster foe could chain
        // into a lock). The confusion lingers and snaps out on the next action.
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_TACKLE); } // self-hit + Tackle
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE); }   // snaps out + Tackle
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!");         // turn 1: the one-time self-hit...
        MESSAGE("Wobbuffet used Tackle!");                   // ...but the move STILL executes (no lock)
        HP_BAR(opponent);
        MESSAGE("Wobbuffet snapped out of its confusion!");  // turn 2: snaps out, no second self-hit
        MESSAGE("Wobbuffet used Tackle!");
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: a faster foe re-confusing can't stop the battler acting or force a second self-hit")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_TACKLE); } // confused: self-hit + Tackle
        // The foe re-uses Confuse Ray, but the volatile still lingers, so it can't refresh
        // the confusion: the battler snaps out and Tackles instead of self-hitting again.
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("It hurt itself in its confusion!");        // turn 1 self-hit
        MESSAGE("Wobbuffet used Tackle!");
        HP_BAR(opponent);
        MESSAGE("Wobbuffet snapped out of its confusion!"); // turn 2: lingered then snapped out (not refreshed)
        MESSAGE("Wobbuffet used Tackle!");
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: the confusion self-hit applies regardless of move category")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE); } // self-hit, then Celebrate still runs
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!"); // status move no longer a free shake-off
        MESSAGE("Wobbuffet used Celebrate!");
    }
}
