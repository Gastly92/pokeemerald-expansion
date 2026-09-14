#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_FLAT_HP_ITEMS flag (config/buff.h). BUFF_* flags default
// off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE) and the stock flat-HP behavior is pinned here
// with the flag explicitly off.
//
// The buff converts the two FLAT healers to a fraction of max HP, because a flat number
// does not survive the jump to the frontier's Level 50: stock Oran heals 10 HP and Berry
// Juice 20, against Sitrus Berry's 25% in the same slot.
//
// Every test below starts at full HP and takes a Super Fang, which halves current HP
// exactly. That both puts the holder on the <= 1/2 max HP threshold the berry needs and
// keeps the damage deterministic, so the expected final HP is an exact number rather than
// a range: 300 max HP -> 150 after Super Fang, then the heal under test.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffect == HOLD_EFFECT_RESTORE_HP);
    ASSUME(gItemsInfo[ITEM_BERRY_JUICE].holdEffect == HOLD_EFFECT_RESTORE_HP);
    ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffectParam == 10);
    ASSUME(gItemsInfo[ITEM_BERRY_JUICE].holdEffectParam == 20);
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Oran Berry heals 1/6 of max HP instead of a flat 10")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_ORAN_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 300/6 = 50. Stock would have healed 10.
        EXPECT_EQ(player->hp, 200);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Berry Juice heals 1/3 of max HP instead of a flat 20")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_BERRY_JUICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 300/3 = 100. Berry Juice is deliberately the bigger of the
        // two: at 1/3 it out-heals even Sitrus Berry's 25%, which is the identity a
        // once-per-battle item found in one place should have.
        EXPECT_EQ(player->hp, 250);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS off: Oran Berry heals its stock flat 10 HP")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_ORAN_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        EXPECT_EQ(player->hp, 160);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS off: Berry Juice heals its stock flat 20 HP")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_BERRY_JUICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        EXPECT_EQ(player->hp, 170);
    }
}

// Sitrus Berry is the reason the buff is keyed on the two ITEMS rather than on
// HOLD_EFFECT_RESTORE_HP. Sitrus shares that hold effect whenever
// I_SITRUS_BERRY_HEAL < GEN_4 (config/item.h); this build sits at GEN_LATEST, which routes
// it through HOLD_EFFECT_RESTORE_PCT_HP instead. Pinning it here means that if the config
// ever moves, this test fails rather than the roster's third most-used item being silently
// rebalanced by a flag that never mentioned it.
SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Sitrus Berry is untouched and still heals 25%")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_SITRUS_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 25% of 300 = 75.
        EXPECT_EQ(player->hp, 225);
    }
}

// Oran is a Berry, so Ripen doubles it; Berry Juice is POCKET_ITEMS and is not a Berry, so
// Ripen does not touch it. That asymmetry is stock behavior, and the buff is applied before
// the Ripen doubling rather than instead of it.
SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Ripen doubles the scaled Oran Berry heal")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_APPLIN) { Ability(ABILITY_RIPEN); MaxHP(300); HP(300); Item(ITEM_ORAN_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + (300/6) * 2 = 100.
        EXPECT_EQ(player->hp, 250);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Ripen leaves Berry Juice alone, since it is not a Berry")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_APPLIN) { Ability(ABILITY_RIPEN); MaxHP(300); HP(300); Item(ITEM_BERRY_JUICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 300/3 = 100, NOT doubled.
        EXPECT_EQ(player->hp, 250);
    }
}
