#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "fork/battle_ai_zmove.h"
#include "fork/battle_ai_gimmick.h"

// FORK: AI_FLAG_SMART_Z_MOVE and AI gimmick selection.
//
// Upstream's ShouldUseZMove says yes to any damaging move the plain move would not
// already KO with, so the AI spends its one Z-Move on turn one regardless of whether the
// upgrade buys anything. AI_FLAG_SMART_Z_MOVE makes it spend the Z-Move only when it
// changes the outcome (fewer hits to KO) or when the user is about to faint anyway.

AI_SINGLE_BATTLE_TEST("AI SMART Z: saves the Z-Move when it does not change the outcome")
{
    u64 aiFlags;
    PARAMETRIZE { aiFlags = AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT; }
    PARAMETRIZE { aiFlags = AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_Z_MOVE; }
    GIVEN {
        AI_FLAGS(aiFlags);
        // Chansey is far too bulky for either version to matter this turn, and it cannot
        // threaten a KO back, so there is every reason to hold the Z-Move.
        PLAYER(SPECIES_CHANSEY) { MaxHP(700); HP(700); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FIRIUM_Z); Moves(MOVE_EMBER); }
    } WHEN {
        if (aiFlags & AI_FLAG_SMART_Z_MOVE)
            TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_EMBER, gimmick: GIMMICK_NONE); }
        else
            TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_EMBER, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI SMART Z: spends the Z-Move when it turns a 2HKO into a KO")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_Z_MOVE);
        // Tuned so plain Ember needs two turns but Inferno Overdrive does not.
        PLAYER(SPECIES_WOBBUFFET) { HP(60); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FIRIUM_Z); Moves(MOVE_EMBER); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_EMBER, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI SMART Z: a status move's Z-Move is declined when the boost is useless")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT);
        ASSUME(GetMoveZEffect(MOVE_SWORDS_DANCE) == Z_EFFECT_RESET_STATS);
        // Nothing has lowered the AI's stats, so Z-Swords Dance's reset buys nothing. This
        // path used to fall through to the unconditional "use it" at the end of
        // ShouldUseZMove, spending the Z-Move for no benefit.
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_NORMALIUM_Z); Moves(MOVE_SWORDS_DANCE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_SWORDS_DANCE, gimmick: GIMMICK_NONE); }
    }
}
