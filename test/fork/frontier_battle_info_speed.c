#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h"
#include "fork/innate_abilities.h"
#include "constants/characters.h"

// B_FRONTIER_BATTLE_INFO. The Speed Tiers page's foe range starts from base Speed alone (0 IV /
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

// DETERMINISTIC_MOVE_RESULTS settles Speed ties by a ladder of public facts (higher base Speed,
// then lighter, then higher HP%), so a tie at the edge of a foe's range gets the arrow it
// resolves to. Wobbuffet (base 33) spans 63-181; Wynaut is base 23, Pikachu base 90.
SINGLE_BATTLE_TEST("Frontier INFO speed: a tie at the top of the range shows the arrow the tie ladder gives")
{
    bool32 deterministic;
    enum Species species;
    u32 expected;

    PARAMETRIZE { deterministic = TRUE;  species = SPECIES_PIKACHU; expected = CHAR_DOWN_ARROW; }
    PARAMETRIZE { deterministic = TRUE;  species = SPECIES_WYNAUT;  expected = 0; }
    PARAMETRIZE { deterministic = FALSE; species = SPECIES_PIKACHU; expected = 0; } // a coin flip

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, deterministic);
        PLAYER(species) { Speed(181); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeSpeedGlyph(0), expected);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a tie at the bottom of the range shows the arrow the tie ladder gives")
{
    enum Species species;
    u32 expected;

    PARAMETRIZE { species = SPECIES_WYNAUT;  expected = CHAR_UP_ARROW; }
    PARAMETRIZE { species = SPECIES_PIKACHU; expected = 0; }

    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(species) { Speed(63); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeSpeedGlyph(0), expected);
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
        EXPECT_EQ(GetFoeSpeedGlyph(0), CHAR_DOWN_ARROW);
    }
}

// The glyphs stay on the number axis under Trick Room (the title's TRICK ROOM marker says to read
// them reversed), so a tie you win still reads as outspeeding.
SINGLE_BATTLE_TEST("Frontier INFO speed: a won tie still reads as a down arrow under Trick Room")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        PLAYER(SPECIES_PIKACHU) { Speed(181); Moves(MOVE_TRICK_ROOM); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRICK_ROOM); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(gFieldStatuses & STATUS_FIELD_TRICK_ROOM);
        EXPECT_EQ(GetFoeSpeedGlyph(0), CHAR_DOWN_ARROW);
    }
}
