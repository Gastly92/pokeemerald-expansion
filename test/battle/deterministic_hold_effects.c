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
        MESSAGE("The Quick Claw was used up…"); // consumption is announced
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

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Scope Lens guarantees a critical hit on the holder's first attack, then is consumed")
{
    GIVEN {
        // Compose with DETERMINISTIC_CRITICAL_HITS (the shipped default): the random
        // crit is gone, so the only crit comes from the Scope Lens.
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        // It is the first *attack* that crits, not the first turn: a status move on
        // turn 1 doesn't spend the lens, so the turn-2 attack is the one that crits.
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Celebrate!"); // turn 1: status move, lens unspent
        MESSAGE("Wobbuffet used Scratch!");   // turn 2: first attack...
        MESSAGE("A critical hit!");           // ...crits
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: a crit-boosting item only crits on the first attack")
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
        MESSAGE("A critical hit!"); // first attack
        MESSAGE("The Scope Lens was used up…"); // consumption is announced
        NONE_OF { MESSAGE("A critical hit!"); } // item consumed, crits are gone
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: Lansat Berry makes the holder's next attack a guaranteed critical hit")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_CRITICAL_HITS, TRUE);
        ASSUME(gItemsInfo[ITEM_LANSAT_BERRY].holdEffect == HOLD_EFFECT_CRITICAL_UP);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_LANSAT_BERRY); HP(101); MaxHP(400); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        // Turn 1 drops the holder past the HP threshold, eating the berry; the next
        // attack (turn 2) is then a guaranteed crit.
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Celebrate!"); // turn 1: holder didn't attack
        MESSAGE("Wobbuffet used Scratch!");   // turn 2: next attack...
        MESSAGE("A critical hit!");           // ...is the berry-granted crit
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// ---------------------------------------------------------------------------
// Flinch items (King's Rock family)
// ---------------------------------------------------------------------------

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock flinches the holder's first qualifying attack, then is consumed")
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
        // Consumed at the holder's move-end, before the flinched target's action.
        MESSAGE("The King's Rock was used up…"); // consumption is announced
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
        NONE_OF { MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); }
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock flinch is the first attack, not the first turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        // A non-attacking turn 1 doesn't spend the rock; the first attack (turn 2) flinches.
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Celebrate!");   // turn 1: no attack
        MESSAGE("Wobbuffet used Scratch!");     // turn 2: first attack...
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // ...flinches
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock is not spent when the move's own flinch lands")
{
    GIVEN {
        // With DETERMINISTIC_ADDITIONAL_EFFECTS a flinch move's own flinch is gated on a
        // super-effective hit. Iron Head is super effective here, so it flinches on its
        // own and the rock has nothing to add — and stays unspent.
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_IRON_HEAD, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_ARTICUNO) { Speed(1); } // Ice/Flying: Steel is super effective
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); }
    } SCENE {
        MESSAGE("The opposing Articuno flinched and couldn't move!"); // the move's own flinch
        NONE_OF { MESSAGE("The King's Rock was used up…"); }
    } THEN {
        // The move's own flinch landed, so the rock is never spent.
        EXPECT_EQ(player->item, ITEM_KINGS_ROCK);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock supplies the flinch when the move's own flinch is gated out")
{
    GIVEN {
        // Iron Head is not super effective here, so DETERMINISTIC_ADDITIONAL_EFFECTS gates
        // out its own flinch. King's Rock must still guarantee a flinch (rather than
        // wrongly bowing out just because the move *can* flinch), then is consumed.
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_IRON_HEAD, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); } // Psychic: Steel is neutral, flinch gated out
    } WHEN {
        TURN { MOVE(player, MOVE_IRON_HEAD); }
    } SCENE {
        // Consumed at the holder's move-end, before the flinched target's action.
        MESSAGE("The King's Rock was used up…");
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock flinches a target that flinched last turn (bypasses the anti-lock cap)")
{
    GIVEN {
        // DETERMINISTIC_FLINCH's anti-lock cap would normally block a flinch on a foe
        // that flinched last turn; like Fake Out, King's Rock bypasses it.
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_FAKE_OUT, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        // Turn 1 flinches the target via Fake Out (not King's Rock, which a flinching
        // move never spends), so it enters turn 2 having flinched last turn.
        TURN { MOVE(player, MOVE_FAKE_OUT); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 1, Fake Out
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // turn 2, King's Rock
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock is not consumed when the faster foe already acted (no flinch lands)")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        ASSUME(gItemsInfo[ITEM_KINGS_ROCK].holdEffect == HOLD_EFFECT_FLINCH);
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Item(ITEM_KINGS_ROCK); } // slower: foe acts before the rock hits
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        // The foe has already acted, so the flinch can't land — and the item must survive.
        NONE_OF {
            MESSAGE("The King's Rock was used up…");
            MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
        }
    } THEN {
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
