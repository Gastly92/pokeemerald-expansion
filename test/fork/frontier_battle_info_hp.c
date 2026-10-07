#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h"

// B_FRONTIER_BATTLE_INFO. The Foe page's "HP n%" rounds DOWN, like the move menu's damage
// range, so the two can be compared directly; it never reads 0% for a mon that is still up.

SINGLE_BATTLE_TEST("Frontier INFO: the foe's HP % rounds down and never shows 0 for a living mon")
{
    u32 hp, expected;

    PARAMETRIZE { hp = 200; expected = 100; }
    PARAMETRIZE { hp = 199; expected = 99; }  // 99.5% reads 99, not 100
    PARAMETRIZE { hp = 101; expected = 50; }  // 50.5% reads 50
    PARAMETRIZE { hp = 1;   expected = 1; }   // 0.5% reads 1, not 0

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(200); HP(hp); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeHpPercent(0), expected);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO: a benched foe's HP % is read from its party slot")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { MaxHP(300); HP(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeHpPercent(1), 33);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO: a fainted foe has no HP %")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WYNAUT) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } THEN {
        EXPECT_EQ(GetFoeHpPercent(0), 0);
    }
}
