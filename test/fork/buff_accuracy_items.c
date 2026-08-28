#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h" // FORK: ApplyAccuracyItemReveals (BUFF_ACCURACY_ITEMS_REVEAL)

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

// FORK: BUFF_ACCURACY_ITEMS_REVEAL — the lenses as instruments for *seeing*, feeding the
// B_FRONTIER_BATTLE_INFO viewer's reveal bits (fork-docs/BATTLE_INFO.md). Wide Lens is
// breadth (every seen foe's held item), Zoom Lens is depth (one foe's ability and full
// moveset, once it has been watched using a move).
//
// These call ApplyAccuracyItemReveals() directly — the same thing OpenFrontierBattleInfo()
// does — and assert the reveal bits, rather than driving the viewer UI.

#define ALL_MOVE_SLOTS ((1u << MAX_MON_MOVES) - 1)

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: Wide Lens reveals a seen foe's held item")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); } // never announces itself
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u);
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: Wide Lens reveals breadth only, not the foe's ability")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT_EQ(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u, 0);
        EXPECT_NE(gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][0], ALL_MOVE_SLOTS);
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: Zoom Lens reveals a foe it watched act, but not one it did not")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ZOOM_LENS); }
        OPPONENT(SPECIES_WOBBUFFET); // foe 0 — acts, so it can be studied
        OPPONENT(SPECIES_WYNAUT);    // foe 1 — switches in but never moves
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); SWITCH(opponent, 1); }
    } THEN {
        ApplyAccuracyItemReveals();
        // Foe 0 was watched using a move: ability and the whole moveset open up.
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT_EQ(gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][0], ALL_MOVE_SLOTS);
        // Foe 1 has been seen but never acted: depth needs an observation to build on.
        EXPECT_EQ(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 2u, 0);
        EXPECT_EQ(gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][1], 0);
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: Zoom Lens reveals depth only, not the foe's held item")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ZOOM_LENS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT_EQ(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u, 0);
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: without the flag the lenses reveal nothing")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT_EQ(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u, 0);
    }
}

SINGLE_BATTLE_TEST("BUFF_ACCURACY_ITEMS_REVEAL: a holder-less side reveals nothing")
{
    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_LEFTOVERS); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT_EQ(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u, 0);
        EXPECT_EQ(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u, 0);
    }
}

#undef ALL_MOVE_SLOTS
