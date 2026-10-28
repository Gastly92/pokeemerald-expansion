#include "global.h"
#include "test/battle.h"

// DETERMINISTIC_STATUS: a fixed 3-turn sleep would cost two of a Dynamax's three turns, so a
// Dynamaxed battler can't be put to sleep, and Dynamaxing wakes a sleeper. See
// src/fork/dynamax_sleep.c.

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: a Dynamaxed battler can't be put to sleep")
{
    bool32 deterministic;
    PARAMETRIZE { deterministic = FALSE; }
    PARAMETRIZE { deterministic = TRUE; }
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, deterministic);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_BRELOOM) { Speed(50); }
    } WHEN {
        // Celebrate would become Max Guard, so Dynamax on a Tackle (Max Strike) first.
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_SPORE); }
    } SCENE {
        MESSAGE("The opposing Breloom used Spore!");
        if (deterministic) {
            MESSAGE("It doesn't affect Wobbuffet…");
        } else {
            MESSAGE("Wobbuffet fell asleep!");
        }
    } THEN {
        if (deterministic)
            EXPECT(!(player->status1 & STATUS1_SLEEP));
        else
            EXPECT(player->status1 & STATUS1_SLEEP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: Yawn fails on a Dynamaxed battler")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_YAWN); }
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Yawn!");
        MESSAGE("It doesn't affect Wobbuffet…");
    } THEN {
        EXPECT(!(player->status1 & STATUS1_SLEEP));
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: Dynamaxing clears Yawn's drowsiness")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_YAWN); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Yawn!");
        MESSAGE("Wobbuffet grew drowsy!");
        MESSAGE("Wobbuffet used Max Strike!");
        NOT MESSAGE("Wobbuffet fell asleep!");
    } THEN {
        EXPECT(!(player->status1 & STATUS1_SLEEP));
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_STATUS: Dynamaxing wakes a sleeping battler, which then acts")
{
    bool32 deterministic;
    PARAMETRIZE { deterministic = FALSE; }
    PARAMETRIZE { deterministic = TRUE; }
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_STATUS, deterministic);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Status1(STATUS1_SLEEP_TURN(3)); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        if (deterministic) {
            MESSAGE("Wobbuffet woke up!");
            MESSAGE("Wobbuffet used Max Strike!");
        } else {
            MESSAGE("Wobbuffet is fast asleep.");
            NOT MESSAGE("Wobbuffet used Max Strike!");
        }
    } THEN {
        if (deterministic)
            EXPECT(!(player->status1 & STATUS1_SLEEP));
    }
}
