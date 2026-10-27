#include "global.h"
#include "test/battle.h"
#include "battle_interface.h"
#include "constants/characters.h"

// B_HEALTHBOX_HIDE_GENDER. The healthbox prints the name into gDisplayedStringBattle; with the
// flag on, no gender symbol is appended to it.

static bool32 DisplayedNameHasGenderSymbol(void)
{
    for (u32 i = 0; gDisplayedStringBattle[i] != EOS; i++)
    {
        if (gDisplayedStringBattle[i] == CHAR_MALE || gDisplayedStringBattle[i] == CHAR_FEMALE)
            return TRUE;
    }
    return FALSE;
}

SINGLE_BATTLE_TEST("Healthbox: no gender symbol after the mon's name")
{
    u32 gender;

    PARAMETRIZE { gender = MON_MALE; }
    PARAMETRIZE { gender = MON_FEMALE; }

    GIVEN {
        ASSUME(B_HEALTHBOX_HIDE_GENDER == TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Gender(gender); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Gender(gender); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        UpdateNickInHealthbox(gHealthboxSpriteIds[B_BATTLER_0], GetBattlerMon(B_BATTLER_0));
        EXPECT(!DisplayedNameHasGenderSymbol());
        UpdateNickInHealthbox(gHealthboxSpriteIds[B_BATTLER_1], GetBattlerMon(B_BATTLER_1));
        EXPECT(!DisplayedNameHasGenderSymbol());
    }
}
