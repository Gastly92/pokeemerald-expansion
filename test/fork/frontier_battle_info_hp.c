#include "global.h"
#include "test/battle.h"
#include "string_util.h"
#include "text.h"
#include "fork/frontier_battle_info.h"
#include "constants/characters.h"

// B_FRONTIER_BATTLE_INFO. The Foe page's "HP n%" rounds UP, against the damage preview's
// rounded-down range, so "damage >= HP" on screen is always a real KO. Below full it caps at 99.

SINGLE_BATTLE_TEST("Frontier INFO: the foe's HP % rounds up, and reads 100 only at full HP")
{
    u32 hp, expected;

    PARAMETRIZE { hp = 200; expected = 100; }
    PARAMETRIZE { hp = 199; expected = 99; }  // 99.5% would round up to 100; capped below full
    PARAMETRIZE { hp = 101; expected = 51; }  // 50.5% reads 51
    PARAMETRIZE { hp = 100; expected = 50; }  // exact stays exact
    PARAMETRIZE { hp = 1;   expected = 1; }   // 0.5% reads 1

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
        EXPECT_EQ(GetFoeHpPercent(1), 34); // 33.3% rounds up
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

// The Speed page prints "HP n%" right-aligned on each foe row. Measured, not estimated: the
// widest species name in the widest row form must still leave room for "HP 100%".
TEST("Frontier INFO: the Speed page's HP column clears the widest foe row")
{
    u8 row[64], hpText[16];
    u8 *p;
    u32 widest = 0;

    for (enum Species species = 1; species < NUM_SPECIES; species++)
    {
        u32 w;

        if (!IsSpeciesEnabled(species))
            continue;
        p = StringCopy(row, COMPOUND_STRING("Foe 6: "));
        p = StringCopy(p, GetSpeciesName(species));
        p = StringCopy(p, COMPOUND_STRING(" 999-999 "));
        *p++ = CHAR_UP_ARROW;
        *p = EOS;
        w = GetStringWidth(FONT_NARROW, row, 0);
        if (w > widest)
            widest = w;
    }
    StringCopy(hpText, COMPOUND_STRING("HP 100%"));
    // 224px window, with a few px of gap between the two.
    EXPECT_LT(widest + GetStringWidth(FONT_NARROW, hpText, 0) + 4, 28 * 8);
}

// The case that motivated rounding up: a 46.2% hit on a foe at 46.6%. Rounded the same way both
// would read 46% and look like a KO; the hit leaves it standing.
SINGLE_BATTLE_TEST("Frontier INFO: the HP % never reads as KO'd by a hit that leaves the foe standing")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(1000); HP(466); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        u32 damagePct = 462 * 100 / 1000; // the preview's rounded-down 46.2%
        EXPECT_EQ(damagePct, 46);
        EXPECT_GT(GetFoeHpPercent(0), damagePct);
    }
}
