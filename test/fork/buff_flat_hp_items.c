#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_FLAT_HP_ITEMS flag (config/buff.h). BUFF_* flags default
// off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE) and the stock flat-HP behavior is pinned here
// with the flag explicitly off.
//
// The buff heals maxHP/BUFF_FLAT_HP_DENOMINATOR (1/4) instead of the stock flat 10 and 20
// HP, because a flat number does not survive the jump to the frontier's Level 50. Both
// items land on Sitrus Berry's 25% deliberately: only one of each item can appear per
// team, so two more items that heal what Sitrus heals are two more uncontested draft slots
// its 103 sets can move onto.
//
// Every test below takes a Super Fang, which halves current HP exactly. That both puts the
// holder under the <= 1/2 max HP threshold the berry needs and keeps the damage
// deterministic, so each expected HP is an exact number rather than a range.

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffect == HOLD_EFFECT_RESTORE_HP);
    ASSUME(gItemsInfo[ITEM_BERRY_JUICE].holdEffect == HOLD_EFFECT_RESTORE_HP);
    ASSUME(gItemsInfo[ITEM_ORAN_BERRY].holdEffectParam == 10);
    ASSUME(gItemsInfo[ITEM_BERRY_JUICE].holdEffectParam == 20);
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Oran Berry heals 1/4 of max HP instead of a flat 10")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_ORAN_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 300/4 = 75. Stock would have healed 10.
        EXPECT_EQ(player->hp, 225);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Berry Juice heals 1/4 of max HP instead of a flat 20")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_BERRY_JUICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // Same 75 as Oran and as Sitrus. Berry Juice is deliberately NOT given a bigger
        // heal: the drawback that would pay for one -- Ripen cannot double it and Harvest
        // cannot regrow it -- reaches only 2.3% of species, so a bigger number would just
        // be a strictly better Sitrus. Its identity comes from not being a Berry instead,
        // which the two Ripen tests below pin.
        EXPECT_EQ(player->hp, 225);
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
// rebalanced by a flag that never mentioned it. It also documents the target the other two
// items are matching: all three heal 75 of a 300 HP pool.
SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Sitrus Berry is untouched and heals the same 25%")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); Item(ITEM_SITRUS_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        EXPECT_EQ(player->hp, 225);
    }
}

// The pair below is where Berry Juice's identity actually lives. At an equal heal the two
// items differ only in that Oran is a Berry and Berry Juice is not, so Ripen doubles one and
// not the other.
//
// Both start at FULL HP, and that matters: an earlier draft of these started at 120/300,
// which is already under the <= 1/2 threshold, so the berry fired BEFORE Super Fang and the
// damage then halved the healed total (120 + 150 = 270, halved to 135). Starting full makes
// the Super Fang itself the thing that crosses the threshold, which is the ordering the items
// are actually used in.
SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Ripen doubles the scaled Oran Berry heal")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_APPLETUN) { Ability(ABILITY_RIPEN); MaxHP(300); HP(300); Item(ITEM_ORAN_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + (300/4) * 2 = 150. A Ripen-doubled Oran at this
        // denominator exactly undoes a Super Fang, which is the cleanest possible statement
        // of the doubling: without Ripen the same set would sit at 225.
        EXPECT_EQ(player->hp, 300);
    }
}

SINGLE_BATTLE_TEST("BUFF_FLAT_HP_ITEMS: Ripen leaves Berry Juice alone, since it is not a Berry")
{
    GIVEN {
        WITH_CONFIG(BUFF_FLAT_HP_ITEMS, TRUE);
        PLAYER(SPECIES_APPLETUN) { Ability(ABILITY_RIPEN); MaxHP(300); HP(300); Item(ITEM_BERRY_JUICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUPER_FANG); }
    } THEN {
        // 150 after Super Fang, + 300/4 = 75, NOT doubled -- 225 against the Oran set's 300
        // on the same turn, from the same number, purely because one is a Berry.
        EXPECT_EQ(player->hp, 225);
    }
}

