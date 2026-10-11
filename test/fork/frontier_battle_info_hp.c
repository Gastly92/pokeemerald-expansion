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

// The Speed page's "You:" rows carry the same rounded-up figure. Rounding up is what makes it
// usable for Substitute: the move works iff the row reads 26% or more. These are the HP values
// of the 26/25/25 cases in the test below; this one checks the move agrees.
SINGLE_BATTLE_TEST("Frontier INFO: Substitute works exactly when your HP % reads 26 or more")
{
    u32 hp;
    bool32 works;

    PARAMETRIZE { hp = 101; works = TRUE; }  // 25.25% reads 26
    PARAMETRIZE { hp = 100; works = FALSE; } // exactly 1/4 reads 25, and is not enough
    PARAMETRIZE { hp = 99;  works = FALSE; } // 24.75% reads 25

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(400); HP(hp); Moves(MOVE_SUBSTITUTE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SUBSTITUTE); }
    } SCENE {
        if (works)
            ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, player);
        else
            NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, player);
    } THEN {
        if (works)
            EXPECT_EQ(gBattleMons[0].hp, hp - 100);
        else
            EXPECT_EQ(gBattleMons[0].hp, hp);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO: your HP % rounds up, and reads 100 only at full HP")
{
    u32 maxHP, hp, expected;

    PARAMETRIZE { maxHP = 200; hp = 200; expected = 100; }
    PARAMETRIZE { maxHP = 200; hp = 199; expected = 99; }
    PARAMETRIZE { maxHP = 400; hp = 101; expected = 26; } // the Substitute cases above
    PARAMETRIZE { maxHP = 400; hp = 100; expected = 25; }
    PARAMETRIZE { maxHP = 400; hp = 99;  expected = 25; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(maxHP); HP(hp); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetBattlerHpPercent(0), expected);
    }
}

// "You: " + the widest 12-character nickname + "  Spe 9999" must leave room for "HP 100%".
TEST("Frontier INFO: the Speed page's HP column clears the widest own row")
{
    u8 row[64], hpText[16];

    StringCopy(row, COMPOUND_STRING("You: WWWWWWWWWWWW  Spe 9999"));
    StringCopy(hpText, COMPOUND_STRING("HP 100%"));
    EXPECT_LT(GetStringWidth(FONT_NARROW, row, 0) + GetStringWidth(FONT_NARROW, hpText, 0) + 4, 28 * 8);
}
