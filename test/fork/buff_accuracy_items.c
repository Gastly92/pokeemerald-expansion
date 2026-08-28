#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_ACCURACY_ITEMS flag (config/buff.h). BUFF_* flags default
// off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE) -- and, since the buff only exists inside the
// PP economy, WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE) alongside it.
//
// The buff restores the ATTACKER's half of the accuracy axis, which
// DETERMINISTIC_ACCURACY_EVASION had dropped: Wide Lens cancels the flat evasion taxes,
// Zoom Lens cancels those AND the target's evasion stat-stage boosts but only while its
// holder moves second. Both are PURE BOONS -- they cancel a penalty and never refund, so
// a move never costs less than its base 1 PP.
//
// Pound has 35 PP and 100% accuracy, so it is exempt from the max-PP accuracy scaling and
// every expectation below reads as 35 minus exactly the PP the economy charged.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_WIDE_LENS].holdEffect == HOLD_EFFECT_WIDE_LENS);
    ASSUME(gItemsInfo[ITEM_ZOOM_LENS].holdEffect == HOLD_EFFECT_ZOOM_LENS);
    ASSUME(gItemsInfo[ITEM_BRIGHT_POWDER].holdEffect == HOLD_EFFECT_EVASION_UP);
    ASSUME(GetMovePP(MOVE_POUND) == 35);
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: Wide Lens cancels a target's BrightPowder PP tax")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_BRIGHT_POWDER); }
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 34); // 35 - 1 (base); the BrightPowder tax is cancelled
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: without the flag Wide Lens is inert")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_BRIGHT_POWDER); }
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // 35 - 1 (base) - 1 (BrightPowder), the pre-buff cost
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: Wide Lens never refunds PP against an untricky target")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 34); // pure boon: still the base 1 PP, never less
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: Wide Lens does not cancel a raised evasion stage")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // 35 - 1 (base) - 1 (+1 evasion): stages are Zoom Lens's half
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: Zoom Lens cancels a raised evasion stage while moving second")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ZOOM_LENS); Speed(20); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); } // opponent is faster, so it has acted
    } THEN {
        EXPECT_EQ(player->pp[0], 34); // 35 - 1 (base); the +1 evasion stage is ignored
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS: Zoom Lens gives nothing while moving first")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ZOOM_LENS); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); } // player is faster, window closed
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // 35 - 1 (base) - 1 (+1 evasion): no relief this turn
    }
}
