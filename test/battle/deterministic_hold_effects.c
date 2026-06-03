#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_HOLD_EFFECTS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE). The stock
// random-chance behavior of these items keeps being exercised by the per-item
// tests under test/battle/hold_effect/ (determinism off).

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_FOCUS_BAND].holdEffect == HOLD_EFFECT_FOCUS_BAND);
    ASSUME(gItemsInfo[ITEM_QUICK_CLAW].holdEffect == HOLD_EFFECT_QUICK_CLAW);
    ASSUME(gItemsInfo[ITEM_SCOPE_LENS].holdEffect == HOLD_EFFECT_SCOPE_LENS);
    ASSUME(gItemsInfo[ITEM_KINGS_ROCK].holdEffect == HOLD_EFFECT_FLINCH);
    ASSUME(gItemsInfo[ITEM_STARF_BERRY].holdEffect == HOLD_EFFECT_RANDOM_STAT_UP);
}

// ---------------------------------------------------------------------------
// Focus Band
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Focus Band always survives a lethal hit on the entry turn from any HP and is consumed")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        // Below max HP, so a stock Focus Sash would NOT save here.
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FOCUS_BAND); HP(1); MaxHP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet hung on using its Focus Band!");
    } THEN {
        EXPECT_EQ(opponent->hp, 1);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Focus Band does not save after the entry turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FOCUS_BAND); HP(1); MaxHP(200); }
    } WHEN {
        // The opponent's entry turn passes without it taking a lethal hit, so the
        // band never triggers (and is never consumed); the next turn it can't save.
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        NONE_OF { MESSAGE("The opposing Wobbuffet hung on using its Focus Band!"); }
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
        EXPECT_EQ(opponent->item, ITEM_FOCUS_BAND);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: a multi-hit move gets around Focus Band")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(GetMoveStrikeCount(MOVE_DOUBLE_KICK) == 2);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FOCUS_BAND); HP(1); MaxHP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        // First strike is survived, the band is consumed, the second strike KOs.
        MESSAGE("The opposing Wobbuffet hung on using its Focus Band!");
    } THEN {
        EXPECT_EQ(opponent->hp, 0);
        EXPECT_EQ(opponent->item, ITEM_NONE);
    }
}

// ---------------------------------------------------------------------------
// Quick Claw
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Quick Claw always moves first on the entry turn, then is consumed")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Item(ITEM_QUICK_CLAW); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        // Entry turn over and item consumed: the slow holder now moves last.
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet can act faster than normal, thanks to its Quick Claw!");
        MESSAGE("Wobbuffet used Celebrate!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        MESSAGE("Wobbuffet used Celebrate!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// ---------------------------------------------------------------------------
// Crit-boosting items (Scope Lens family)
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Scope Lens guarantees a critical hit on the entry turn, then is consumed")
{
    GIVEN {
        // Compose with DETERMINISTIC_CRITICAL_HITS (the shipped default): the random
        // crit is gone, so the only crit comes from the entry-turn Scope Lens.
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: a crit-boosting item no longer crits after its entry turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A critical hit!"); // entry turn
        NONE_OF { MESSAGE("A critical hit!"); } // item consumed, crits are gone
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// ---------------------------------------------------------------------------
// Flinch items (King's Rock family)
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock always flinches on the entry turn, then is consumed")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
        // Item consumed: no flinch the next turn, the target acts.
        TURN { MOVE(player, MOVE_SCRATCH); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); }
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock does not add a flinch to a move that already flinches")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_HEADBUTT, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_HEADBUTT); }
    } THEN {
        // The move's own flinch effect is untouched, so the band is never spent.
        EXPECT_EQ(player->item, ITEM_KINGS_ROCK);
    }
}

// ---------------------------------------------------------------------------
// Starf Berry
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Starf Berry raises the holder's highest stat")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        // Speed is set well above the holder's other (default) stats, so it is the pick.
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_STARF_BERRY); HP(101); MaxHP(400); Speed(300); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The Starf Berry sharply boosted Wobbuffet's Speed!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
        EXPECT_LE(player->hp * 4, player->maxHP);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(player->statStages[STAT_SPDEF], DEFAULT_STAT_STAGE);
    }
}
