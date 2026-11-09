#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h"
#include "fork/innate_abilities.h"

// B_FRONTIER_BATTLE_INFO. The Overview page's foe Speed range starts from base Speed alone (0 IV /
// 0 EV / hindering nature up to 31 / 252 / boosting) and is then scaled by everything the player
// can see, and nothing they cannot. A level 100 Wobbuffet (base 33) spans 63-181.

SINGLE_BATTLE_TEST("Frontier INFO speed: an unrevealed Choice Scarf is left out of the range")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_SCARF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 63);
        EXPECT_EQ(hi, 181);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: the foe's Speed stages scale the range")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AGILITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_AGILITY); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: the foe's Tailwind doubles the range, benched foes included")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TAILWIND); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TAILWIND); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
        // Not yet seen, but the range is still priced as if it came in now.
        GetFoeSpeedRange(1, &lo, &hi, &tie);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a benched foe does not carry the active foe's stat stages")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AGILITY); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_AGILITY); }
    } THEN {
        GetFoeSpeedRange(1, &lo, &hi, &tie);
        EXPECT_EQ(lo, 63);
        EXPECT_EQ(hi, 181);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: Unburden doubles the range once it fires")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SUPER_FANG) == EFFECT_FIXED_PERCENT_DAMAGE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SUPER_FANG); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_UNBURDEN); Item(ITEM_SITRUS_BERRY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SUPER_FANG); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a revealed Choice Scarf scales the range")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE); // Wide Lens reveals the foe's item
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_SCARF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 94);
        EXPECT_EQ(hi, 271);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a foe Transformed into your mon has your mon's exact Speed")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        PLAYER(SPECIES_GARCHOMP) { Speed(250); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_DITTO) { Ability(ABILITY_IMPOSTER); Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 250);
        EXPECT_EQ(hi, 250);
    }
}

// Innates are species data, so they count from the moment the foe is seen, unlike its chosen ability.
SINGLE_BATTLE_TEST("Frontier INFO speed: an innate Swift Swim doubles the range in rain without a reveal")
{
    u32 lo, hi;
    s32 tie;

    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_SQUIRTLE, ABILITY_SWIFT_SWIM));
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_RAIN_DANCE); }
        OPPONENT(SPECIES_SQUIRTLE) { Ability(ABILITY_TORRENT); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u));
        GetFoeSpeedRange(0, &lo, &hi, &tie);
        EXPECT_EQ(lo, 162); // base 43: 81-203, doubled
        EXPECT_EQ(hi, 406);
    }
}

// The Overview page is in turn order: a foe sits above your row when it can act before you, read
// from the edge of its range that acts first - the top normally, the bottom under Trick Room.
// A level 100 Wobbuffet (base 33) spans 63-181.
SINGLE_BATTLE_TEST("Frontier INFO speed: a foe can act before you when its best case beats your Speed")
{
    u32 speed;
    bool32 trickRoom, expected;

    PARAMETRIZE { trickRoom = FALSE; speed = 200; expected = FALSE; } // faster than its top
    PARAMETRIZE { trickRoom = FALSE; speed = 100; expected = TRUE;  } // inside its range
    PARAMETRIZE { trickRoom = FALSE; speed = 50;  expected = TRUE;  } // slower than its bottom
    PARAMETRIZE { trickRoom = TRUE;  speed = 200; expected = TRUE;  }
    PARAMETRIZE { trickRoom = TRUE;  speed = 100; expected = TRUE;  }
    PARAMETRIZE { trickRoom = TRUE;  speed = 50;  expected = FALSE; } // slower than its bottom moves first

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        PLAYER(SPECIES_PIKACHU) { Speed(speed); Moves(MOVE_TRICK_ROOM, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, trickRoom ? MOVE_TRICK_ROOM : MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_LEFT), expected);
    }
}

// DETERMINISTIC_MOVE_RESULTS settles Speed ties by a ladder of public facts (higher base Speed,
// then lighter, then higher HP%), so a tie at the edge of a foe's range is settled, not a coin
// flip. Wobbuffet is base 33; Wynaut is base 23, Pikachu base 90.
SINGLE_BATTLE_TEST("Frontier INFO speed: a tie at the top of the range goes the way the tie ladder gives")
{
    bool32 deterministic;
    enum Species species;
    bool32 expected;

    PARAMETRIZE { deterministic = TRUE;  species = SPECIES_PIKACHU; expected = FALSE; } // you win the tie
    PARAMETRIZE { deterministic = TRUE;  species = SPECIES_WYNAUT;  expected = TRUE;  }
    PARAMETRIZE { deterministic = FALSE; species = SPECIES_PIKACHU; expected = TRUE;  } // a coin flip, so it can

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, deterministic);
        PLAYER(species) { Speed(181); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_LEFT), expected);
    }
}

// Under Trick Room the bottom of the range is the edge that acts first, and the tie ladder is
// reversed with turn order: the lower base Speed now wins it.
SINGLE_BATTLE_TEST("Frontier INFO speed: under Trick Room a tie at the bottom of the range follows the reversed ladder")
{
    enum Species species;
    bool32 expected;

    PARAMETRIZE { species = SPECIES_WYNAUT;  expected = FALSE; } // base 23 < 33: you act first
    PARAMETRIZE { species = SPECIES_PIKACHU; expected = TRUE;  }

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        PLAYER(species) { Speed(63); Moves(MOVE_TRICK_ROOM); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRICK_ROOM); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(gFieldStatuses & STATUS_FIELD_TRICK_ROOM);
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_LEFT), expected);
    }
}

// Same species, so the ladder reaches weight. A hidden Float Stone would make the foe lighter and
// win it; unrevealed, it is left out and the tie falls through to HP%, which you win.
SINGLE_BATTLE_TEST("Frontier INFO speed: the tie ladder only uses what the player knows")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetItemHoldEffect(ITEM_FLOAT_STONE) == HOLD_EFFECT_FLOAT_STONE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(181); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); HP(1); Item(ITEM_FLOAT_STONE); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_LEFT), FALSE);
    }
}

// In doubles each of your rows is placed by its own Speed, so a foe can be above one and below the other.
DOUBLE_BATTLE_TEST("Frontier INFO speed: each of your mons is compared at its own Speed")
{
    GIVEN {
        PLAYER(SPECIES_PIKACHU) { Speed(200); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE); MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_LEFT), FALSE);
        EXPECT_EQ(FoeMayActBefore(0, B_POSITION_PLAYER_RIGHT), TRUE);
    }
}

// Opened from the party menu, the Speed Tiers page places the cursor's mon as a "Switch:" row,
// priced as if it replaced the active mon now: its own item and the current field count, the
// active mon's stat stages do not.
SINGLE_BATTLE_TEST("Frontier INFO speed: a switch-in is priced with its own item, fresh stages and the current field")
{
    u32 move;
    u16 item;
    u32 expected;

    PARAMETRIZE { move = MOVE_CELEBRATE; item = ITEM_NONE;         expected = 100; }
    PARAMETRIZE { move = MOVE_CELEBRATE; item = ITEM_CHOICE_SCARF; expected = 150; }
    PARAMETRIZE { move = MOVE_AGILITY;   item = ITEM_NONE;         expected = 100; } // the active mon's +2 stays with it
    PARAMETRIZE { move = MOVE_TAILWIND;  item = ITEM_NONE;         expected = 200; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE, MOVE_AGILITY, MOVE_TAILWIND); }
        PLAYER(SPECIES_WYNAUT) { Speed(100); Item(item); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetSwitchCandidateSpeed(1, B_POSITION_PLAYER_LEFT), expected);
        // and the battler it borrowed is left untouched
        EXPECT_EQ(gBattleMons[B_POSITION_PLAYER_LEFT].species, SPECIES_WOBBUFFET);
    }
}
