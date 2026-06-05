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
// King's Rock × DETERMINISTIC_FLINCH × DETERMINISTIC_ADDITIONAL_EFFECTS — the full
// production stack (all four flags on, as they ship). Verifies the multi-turn /
// doubles interactions a flinch-move holder runs into.
// ---------------------------------------------------------------------------

// Scenario 1: holder is faster and the flinch move is super effective every turn.
//   T1 — the move's own flinch lands (SE gate passes, no prior flinch); the rock has
//        nothing to add, so it is NOT consumed.
//   T2 — the move's own flinch is suppressed by the anti-lock cap (target flinched last
//        turn), so the rock fills in (it bypasses the cap like Fake Out) and IS consumed.
//   T3 — the target flinched last turn (via the rock), and the rock is now gone, so the
//        move's own flinch is again capped: NO flinch, the target acts.
//   T4 — the target did not flinch last turn, so the move's own flinch lands again.
SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: King's Rock fills the anti-lock gap for one turn, then the move flinch-locks every other turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE); // Rock Slide always lands
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_REGICE) { Speed(1); } // pure Ice: 2x weak to Rock, very bulky
    } WHEN {
        TURN { MOVE(player, MOVE_ROCK_SLIDE); } // T1: foe flinched (by the move)
        TURN { MOVE(player, MOVE_ROCK_SLIDE); } // T2: foe flinched (by King's Rock)
        TURN { MOVE(player, MOVE_ROCK_SLIDE); MOVE(opponent, MOVE_CELEBRATE); } // T3: foe acts
        TURN { MOVE(player, MOVE_ROCK_SLIDE); } // T4: foe flinched (by the move)
    } SCENE {
        // T1: the move flinches; the rock is untouched (no "used up" yet).
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The opposing Regice flinched and couldn't move!");
        // T2: the rock fills the anti-lock gap and is spent right here.
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The King's Rock was used up…");
        MESSAGE("The opposing Regice flinched and couldn't move!");
        // T3: capped, no rock left — the foe acts (proof it was not flinched).
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The opposing Regice used Celebrate!");
        // T4: the move flinches again.
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The opposing Regice flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// Scenario 2: holder is faster and the flinch move is NOT super effective — its own
// flinch is gated out, so King's Rock supplies the flinch and is consumed.
SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: faster holder, non-super-effective flinch move — King's Rock supplies the flinch and is consumed")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); } // Psychic: neutral to Rock
    } WHEN {
        TURN { MOVE(player, MOVE_ROCK_SLIDE); }
    } SCENE {
        MESSAGE("The King's Rock was used up…");
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// Scenario 3: holder is slower — the foe has already acted by the time the move hits, so
// no flinch can land (neither the move's nor the rock's), and the rock is not consumed,
// even though the move is super effective.
SINGLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: slower holder never flinches and never spends King's Rock")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Item(ITEM_KINGS_ROCK); } // slower
        OPPONENT(SPECIES_REGICE) { Speed(100); } // pure Ice: super effective, but acts first
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_ROCK_SLIDE); }
    } SCENE {
        NONE_OF {
            MESSAGE("The King's Rock was used up…");
            MESSAGE("The opposing Regice flinched and couldn't move!");
        }
    } THEN {
        EXPECT_EQ(player->item, ITEM_KINGS_ROCK);
    }
}

// Scenario 4a (doubles): the holder's spread flinch move is NOT super effective on
// either foe, so both foes' own flinches are gated out. King's Rock is evaluated once
// per target (MOVEEND_ITEM_EFFECTS_ATTACKER_1 runs before MOVEEND_NEXT_TARGET), so it
// supplies a flinch to BOTH foes, then is consumed ONCE (MOVEEND_DETERMINISTIC_HOLD_CONSUME
// runs after the target loop).
DOUBLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: in doubles a spread flinch move whose flinch is gated out lets King's Rock flinch both foes, consumed once")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        PLAYER(SPECIES_ZIGZAGOON) { Speed(99); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }  // Psychic: neutral to Rock
        OPPONENT(SPECIES_ALAKAZAM) { Speed(2); }   // Psychic: neutral to Rock; acts first of the foes
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_ROCK_SLIDE); MOVE(playerRight, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The King's Rock was used up…"); // consumed exactly once (after both targets)
        // Flinch messages print when each foe tries to act, i.e. in foe speed order.
        MESSAGE("The opposing Alakazam flinched and couldn't move!");
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(playerLeft->item, ITEM_NONE);
    }
}

// Scenario 4b (doubles): the spread flinch move IS super effective on both foes, so each
// foe's own flinch lands and King's Rock has nothing to add — it stays unspent.
DOUBLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: in doubles a super-effective spread flinch move flinches both foes itself and never spends King's Rock")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Item(ITEM_KINGS_ROCK); }
        PLAYER(SPECIES_ZIGZAGOON) { Speed(99); }
        OPPONENT(SPECIES_REGICE) { Speed(1); }   // pure Ice: super effective
        OPPONENT(SPECIES_ARTICUNO) { Speed(2); } // Ice/Flying: super effective; acts first of the foes
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_ROCK_SLIDE); MOVE(playerRight, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Rock Slide!");
        NONE_OF { MESSAGE("The King's Rock was used up…"); } // the move flinches both itself; rock unspent
        // Flinch messages print when each foe tries to act, i.e. in foe speed order.
        MESSAGE("The opposing Articuno flinched and couldn't move!");
        MESSAGE("The opposing Regice flinched and couldn't move!");
    } THEN {
        EXPECT_EQ(playerLeft->item, ITEM_KINGS_ROCK);
    }
}

// Scenario 4c (doubles): one foe is FASTER than the holder and one is SLOWER. A flinch
// (whether from the move or from King's Rock) needs the target to not have acted yet, so
// the faster foe — which already took its turn before Rock Slide hits — is never flinched;
// only the slower foe is. King's Rock is still spent (the slower foe triggered it). Shown
// with a neutral hit so the rock, not the move, is the flincher.
DOUBLE_BATTLE_TEST("DETERMINISTIC_HOLD_EFFECTS: in doubles a foe faster than the holder is never flinched, only the slower foe")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_ROCK_SLIDE, MOVE_EFFECT_FLINCH));
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Item(ITEM_KINGS_ROCK); }
        PLAYER(SPECIES_ZIGZAGOON) { Speed(49); }
        OPPONENT(SPECIES_ALAKAZAM) { Speed(100); } // Psychic, neutral to Rock; FASTER than the holder
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }  // Psychic, neutral to Rock; SLOWER than the holder
    } WHEN {
        // The fast foe acts before Rock Slide; the slow foe is flinched and never acts.
        TURN { MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(playerLeft, MOVE_ROCK_SLIDE); MOVE(playerRight, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Alakazam used Celebrate!"); // fast foe already acted...
        MESSAGE("Wobbuffet used Rock Slide!");
        MESSAGE("The King's Rock was used up…");          // spent on the slow foe
        MESSAGE("The opposing Wobbuffet flinched and couldn't move!"); // only the slow foe flinches
        NONE_OF { MESSAGE("The opposing Alakazam flinched and couldn't move!"); } // the fast foe never does
    } THEN {
        EXPECT_EQ(playerLeft->item, ITEM_NONE);
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
