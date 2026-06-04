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
        TURN { MOVE(opponent, MOVE_SPORE); MOVE(player, MOVE_CELEBRATE); } // asleep (DETERMINISTIC_SLEEP_TURNS == 2)
        TURN { MOVE(player, MOVE_CELEBRATE); }                             // wakes on the 2nd turn
    } SCENE {
        MESSAGE("Wobbuffet is fast asleep.");
        MESSAGE("Wobbuffet woke up!");
        MESSAGE("Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: an attacking move makes a confused battler hit itself once, then clears")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_TACKLE); } // confused: hits itself, move cancelled
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE); }   // confusion cleared: Tackle lands
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!");
        MESSAGE("Wobbuffet used Tackle!");
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: a status move shakes off confusion for free")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("Wobbuffet snapped out of its confusion!");
        MESSAGE("Wobbuffet used Celebrate!");
    }
}
