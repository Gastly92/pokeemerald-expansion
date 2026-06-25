#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

// FORK: a damaging Z-Move (Knock Off -> Black Hole Eclipse) keeps none of Knock
// Off's utility - it does not strip the target's item. With FEATURE_FREE_GIMMICKS
// the AI always has a Z-Move available, so it used to upgrade Knock Off every time
// and waste the one-shot Z-Move instead of removing a worthwhile item. ShouldUseZMove
// now keeps the regular Knock Off when the target holds an item worth removing and
// the Z-Move would not secure a KO the regular hit can't. These tests exercise the
// shared ShouldUseZMove path via a held Z-Crystal (deterministic, FREE_GIMMICKS off).

AI_SINGLE_BATTLE_TEST("AI keeps Knock Off instead of Black Hole Eclipse to strip a worthwhile item")
{
    enum Item targetItem;
    PARAMETRIZE { targetItem = ITEM_LEFTOVERS; }   // worthwhile -> keep Knock Off
    PARAMETRIZE { targetItem = ITEM_NONE; }         // nothing to strip -> use Z-Move
    PARAMETRIZE { targetItem = ITEM_LAGGING_TAIL; } // hurts the holder -> use Z-Move

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        ASSUME(GetMoveType(MOVE_KNOCK_OFF) == TYPE_DARK);
        ASSUME(GetMoveEffect(MOVE_KNOCK_OFF) == EFFECT_KNOCK_OFF);
        PLAYER(SPECIES_WOBBUFFET) { Item(targetItem); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_DARKINIUM_Z); Moves(MOVE_KNOCK_OFF); }
    } WHEN {
    if (targetItem == ITEM_LEFTOVERS)
        TURN { EXPECT_MOVE(opponent, MOVE_KNOCK_OFF, gimmick: GIMMICK_NONE); }
    else
        TURN { EXPECT_MOVE(opponent, MOVE_KNOCK_OFF, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI still upgrades Knock Off to Black Hole Eclipse when the Z-Move secures a KO")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        ASSUME(GetMoveEffect(MOVE_KNOCK_OFF) == EFFECT_KNOCK_OFF);
        // 120 BP Black Hole Eclipse (~112) KOs at this HP but 65 BP Knock Off (~93,
        // including its 1.5x boost vs an item holder) does not, so taking the guaranteed
        // KO is worth more than stripping the item. Wobbuffet is Psychic (2x weak to Dark);
        // both calcs scale together so the ~9-10 HP margin on each side is comfortable.
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_LEFTOVERS); HP(102); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_DARKINIUM_Z); Moves(MOVE_KNOCK_OFF); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_KNOCK_OFF, gimmick: GIMMICK_Z_MOVE); }
    }
}
