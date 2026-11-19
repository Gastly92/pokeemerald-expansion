#include "global.h"
#include "test/battle.h"

// Coverage for BUFF_TYPE_HIT_STAT_ITEMS (config/buff.h): Cell Battery, Absorb Bulb, Snowball and
// Luminous Moss raise their stat +2 instead of +1. BUFF_* flags default off in the test baseline
// (see TestInitConfigData), so each test opts in with WITH_CONFIG and the stock behavior is pinned
// with the flag explicitly off. The stock behavior is also covered by
// test/battle/hold_effect/cell_battery.c, absorb_bulb.c, snowball.c and luminous_moss.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_CELL_BATTERY) == HOLD_EFFECT_CELL_BATTERY);
    ASSUME(GetItemHoldEffect(ITEM_ABSORB_BULB) == HOLD_EFFECT_ABSORB_BULB);
    ASSUME(GetItemHoldEffect(ITEM_SNOWBALL) == HOLD_EFFECT_SNOWBALL);
    ASSUME(GetItemHoldEffect(ITEM_LUMINOUS_MOSS) == HOLD_EFFECT_LUMINOUS_MOSS);
    ASSUME(GetMoveType(MOVE_THUNDER_SHOCK) == TYPE_ELECTRIC);
    ASSUME(GetMoveType(MOVE_BUBBLE) == TYPE_WATER);
    ASSUME(GetMoveType(MOVE_ICE_BEAM) == TYPE_ICE);
}

SINGLE_BATTLE_TEST("BUFF_TYPE_HIT_STAT_ITEMS: Cell Battery, Absorb Bulb, Snowball and Luminous Moss raise their stat by 2")
{
    u32 config;
    enum Item item;
    enum Move move;
    enum Stat stat;

    PARAMETRIZE { config = TRUE;  item = ITEM_CELL_BATTERY;  move = MOVE_THUNDER_SHOCK; stat = STAT_ATK; }
    PARAMETRIZE { config = TRUE;  item = ITEM_ABSORB_BULB;   move = MOVE_BUBBLE;        stat = STAT_SPATK; }
    PARAMETRIZE { config = TRUE;  item = ITEM_SNOWBALL;      move = MOVE_ICE_BEAM;      stat = STAT_ATK; }
    PARAMETRIZE { config = TRUE;  item = ITEM_LUMINOUS_MOSS; move = MOVE_BUBBLE;        stat = STAT_SPDEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_CELL_BATTERY;  move = MOVE_THUNDER_SHOCK; stat = STAT_ATK; }
    PARAMETRIZE { config = FALSE; item = ITEM_ABSORB_BULB;   move = MOVE_BUBBLE;        stat = STAT_SPATK; }
    PARAMETRIZE { config = FALSE; item = ITEM_SNOWBALL;      move = MOVE_ICE_BEAM;      stat = STAT_ATK; }
    PARAMETRIZE { config = FALSE; item = ITEM_LUMINOUS_MOSS; move = MOVE_BUBBLE;        stat = STAT_SPDEF; }

    GIVEN {
        WITH_CONFIG(BUFF_TYPE_HIT_STAT_ITEMS, config);
        PLAYER(SPECIES_WOBBUFFET) { HP(400); MaxHP(400); SpDefense(400); Item(item); }
        OPPONENT(SPECIES_WOBBUFFET) { SpAttack(1); }
    } WHEN {
        TURN { MOVE(opponent, move); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        HP_BAR(player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
        EXPECT_EQ(player->statStages[stat], DEFAULT_STAT_STAGE + (config ? 2 : 1));
    }
}

SINGLE_BATTLE_TEST("BUFF_TYPE_HIT_STAT_ITEMS: Simple doubles and Contrary inverts the buffed Cell Battery")
{
    u32 config;
    enum Species species;
    enum Ability ability;
    s32 expectedDelta;

    PARAMETRIZE { config = TRUE;  species = SPECIES_BIBAREL; ability = ABILITY_SIMPLE;   expectedDelta = 4; }
    PARAMETRIZE { config = TRUE;  species = SPECIES_INKAY;   ability = ABILITY_CONTRARY; expectedDelta = -2; }
    PARAMETRIZE { config = FALSE; species = SPECIES_BIBAREL; ability = ABILITY_SIMPLE;   expectedDelta = 2; }
    PARAMETRIZE { config = FALSE; species = SPECIES_INKAY;   ability = ABILITY_CONTRARY; expectedDelta = -1; }

    GIVEN {
        WITH_CONFIG(BUFF_TYPE_HIT_STAT_ITEMS, config);
        PLAYER(species) { Ability(ability); HP(400); MaxHP(400); SpDefense(400); Item(ITEM_CELL_BATTERY); }
        OPPONENT(SPECIES_WOBBUFFET) { SpAttack(1); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_SHOCK); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + expectedDelta);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

// BUFF_FIELD_STAT_ITEMS (config/buff.h): the terrain seeds and Room Service change their stat
// by 2 instead of 1. Stock behavior is also covered by test/battle/hold_effect/terrain_seed.c
// and room_service.c.

SINGLE_BATTLE_TEST("BUFF_FIELD_STAT_ITEMS: the terrain seeds raise their stat by 2")
{
    u32 config;
    enum Item item;
    enum Species species;
    enum Ability ability;
    enum Stat stat;

    PARAMETRIZE { config = TRUE;  item = ITEM_ELECTRIC_SEED; species = SPECIES_TAPU_KOKO; ability = ABILITY_ELECTRIC_SURGE; stat = STAT_DEF; }
    PARAMETRIZE { config = TRUE;  item = ITEM_GRASSY_SEED;   species = SPECIES_TAPU_BULU; ability = ABILITY_GRASSY_SURGE;   stat = STAT_DEF; }
    PARAMETRIZE { config = TRUE;  item = ITEM_MISTY_SEED;    species = SPECIES_TAPU_FINI; ability = ABILITY_MISTY_SURGE;    stat = STAT_SPDEF; }
    PARAMETRIZE { config = TRUE;  item = ITEM_PSYCHIC_SEED;  species = SPECIES_TAPU_LELE; ability = ABILITY_PSYCHIC_SURGE;  stat = STAT_SPDEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_ELECTRIC_SEED; species = SPECIES_TAPU_KOKO; ability = ABILITY_ELECTRIC_SURGE; stat = STAT_DEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_GRASSY_SEED;   species = SPECIES_TAPU_BULU; ability = ABILITY_GRASSY_SURGE;   stat = STAT_DEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_MISTY_SEED;    species = SPECIES_TAPU_FINI; ability = ABILITY_MISTY_SURGE;    stat = STAT_SPDEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_PSYCHIC_SEED;  species = SPECIES_TAPU_LELE; ability = ABILITY_PSYCHIC_SURGE;  stat = STAT_SPDEF; }

    GIVEN {
        WITH_CONFIG(BUFF_FIELD_STAT_ITEMS, config);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(species) { Ability(ability); }
    } WHEN {
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
        EXPECT_EQ(player->statStages[stat], DEFAULT_STAT_STAGE + (config ? 2 : 1));
    }
}

SINGLE_BATTLE_TEST("BUFF_FIELD_STAT_ITEMS: Room Service lowers Speed by 2 under Trick Room")
{
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_ROOM_SERVICE) == HOLD_EFFECT_ROOM_SERVICE);
        ASSUME(GetMoveEffect(MOVE_TRICK_ROOM) == EFFECT_TRICK_ROOM);
        WITH_CONFIG(BUFF_FIELD_STAT_ITEMS, config);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_ROOM_SERVICE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TRICK_ROOM); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRICK_ROOM, player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
    } THEN {
        EXPECT_EQ(player->item, ITEM_NONE);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE - (config ? 2 : 1));
    }
}
