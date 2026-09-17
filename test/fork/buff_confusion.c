#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_CONFUSION_SELF_DAMAGE flag (config/buff.h), which raises the
// confusion self-hit from the stock 40 BP to BUFF_CONFUSION_SELF_DAMAGE_POWER (80). BUFF_* flags
// default off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG(BUFF_CONFUSION_SELF_DAMAGE, TRUE/FALSE) explicitly, and the stock 40-BP behavior
// stays covered by the inherited tests in test/battle/volatiles/confusion.c (buff off).
//
// DETERMINISTIC_STATUS is likewise pinned per test rather than left to the baseline: the flag has
// to hold on BOTH confusion paths - the fork's one-time tax and upstream's per-action roll - so
// each is exercised on its own. Wobbuffet on both sides keeps Attack and Defense identical, so a
// self-hit and a hit from the mirror match are the same calculation and the base power is the
// only thing separating the runs.

SINGLE_BATTLE_TEST("BUFF_CONFUSION_SELF_DAMAGE: the one-time DETERMINISTIC_STATUS self-hit doubles", s16 damage)
{
    u32 buff;

    PARAMETRIZE { buff = TRUE; }
    PARAMETRIZE { buff = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_CONFUSION_SELF_DAMAGE, buff);
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!");
        HP_BAR(player, captureDamage: &results[i].damage);
        MESSAGE("Wobbuffet used Celebrate!"); // still acts - the buff is damage, not a lockout
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_CONFUSION_SELF_DAMAGE: the stock per-action self-hit doubles too", s16 damage)
{
    u32 buff;

    PARAMETRIZE { buff = TRUE; }
    PARAMETRIZE { buff = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_CONFUSION_SELF_DAMAGE, buff);
        WITH_CONFIG(DETERMINISTIC_STATUS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        // WITH_RNG forces the roll the stock path makes on each confused action, so the test
        // measures the hit rather than re-testing the chance (that is confusion.c's job).
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_CELEBRATE, WITH_RNG(RNG_CONFUSION, TRUE)); }
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!");
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2.0), results[0].damage);
    }
}

// The absolute check: with the buff on the self-hit is worth exactly two Scratches (40 BP each),
// i.e. 80 BP. Mirrors "Confusion adds a 50/33% chance to hit self with 40 power" in
// test/battle/volatiles/confusion.c, which is the same assertion with the buff off.
SINGLE_BATTLE_TEST("BUFF_CONFUSION_SELF_DAMAGE: the buffed self-hit is worth 80 power")
{
    s16 damage[2];

    GIVEN {
        ASSUME(GetMovePower(MOVE_SCRATCH) == 40);
        WITH_CONFIG(BUFF_CONFUSION_SELF_DAMAGE, TRUE);
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH, WITH_RNG(RNG_DAMAGE_MODIFIER, 0)); MOVE(player, MOVE_CONFUSE_RAY); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        HP_BAR(player, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CONFUSE_RAY, player);
        MESSAGE("The opposing Wobbuffet became confused!");
        MESSAGE("The opposing Wobbuffet is confused!");
        MESSAGE("It hurt itself in its confusion!");
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        EXPECT_MUL_EQ(damage[0], Q_4_12(2.0), damage[1]);
    }
}

// A bigger self-hit reaches the DETERMINISTIC_STATUS KO fallback more often, so pin that branch:
// when the self-hit would KO, the move is denied and the battler faints, exactly as stock.
SINGLE_BATTLE_TEST("BUFF_CONFUSION_SELF_DAMAGE: a confused battler that would be KO'd by the self-hit still faints without acting")
{
    GIVEN {
        WITH_CONFIG(BUFF_CONFUSION_SELF_DAMAGE, TRUE);
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); HP(1); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CONFUSE_RAY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CONFUSE_RAY); MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("Wobbuffet became confused!");
        MESSAGE("It hurt itself in its confusion!");
        HP_BAR(player);
        MESSAGE("Wobbuffet fainted!");
        NOT MESSAGE("Wobbuffet used Tackle!"); // the KO fallback ends the move
    }
}
