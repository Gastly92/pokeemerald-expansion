#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h"

// B_FRONTIER_BATTLE_INFO. The Speed Tiers page's foe range starts from base Speed alone (0 IV /
// 0 EV / hindering nature up to 31 / 252 / boosting) and is then scaled by everything the player
// can see, and nothing they cannot. A level 100 Wobbuffet (base 33) spans 63-181.

SINGLE_BATTLE_TEST("Frontier INFO speed: an unrevealed Choice Scarf is left out of the range")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_SCARF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 63);
        EXPECT_EQ(hi, 181);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: the foe's Speed stages scale the range")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AGILITY); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_AGILITY); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: the foe's Tailwind doubles the range, benched foes included")
{
    u32 lo, hi;

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TAILWIND); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TAILWIND); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
        // Not yet seen, but the range is still priced as if it came in now.
        GetFoeSpeedRange(1, &lo, &hi);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a benched foe does not carry the active foe's stat stages")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AGILITY); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_AGILITY); }
    } THEN {
        GetFoeSpeedRange(1, &lo, &hi);
        EXPECT_EQ(lo, 63);
        EXPECT_EQ(hi, 181);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: Unburden doubles the range once it fires")
{
    u32 lo, hi;

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SUPER_FANG) == EFFECT_FIXED_PERCENT_DAMAGE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SUPER_FANG); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_UNBURDEN); Item(ITEM_SITRUS_BERRY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SUPER_FANG); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 126);
        EXPECT_EQ(hi, 362);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a revealed Choice Scarf scales the range")
{
    u32 lo, hi;

    GIVEN {
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE); // Wide Lens reveals the foe's item
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_WIDE_LENS); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_SCARF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        ApplyAccuracyItemReveals();
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 94);
        EXPECT_EQ(hi, 271);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO speed: a foe Transformed into your mon has your mon's exact Speed")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_GARCHOMP) { Speed(250); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_DITTO) { Ability(ABILITY_IMPOSTER); Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        GetFoeSpeedRange(0, &lo, &hi);
        EXPECT_EQ(lo, 250);
        EXPECT_EQ(hi, 250);
    }
}
