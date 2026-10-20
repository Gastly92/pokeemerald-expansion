#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h"

// B_FRONTIER_BATTLE_INFO. Each foe row on the Overview page leads with the foe's type icons:
// the displayed species' types, or its Tera type alone once it has Terastallized.

SINGLE_BATTLE_TEST("Frontier INFO overview: a dual-type foe shows both types")
{
    enum Type types[2];

    GIVEN {
        ASSUME(GetSpeciesType(SPECIES_CHARIZARD, 0) == TYPE_FIRE);
        ASSUME(GetSpeciesType(SPECIES_CHARIZARD, 1) == TYPE_FLYING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHARIZARD);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeOverviewTypes(0, types), 2);
        EXPECT_EQ(types[0], TYPE_FIRE);
        EXPECT_EQ(types[1], TYPE_FLYING);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO overview: a mono-type foe shows one type")
{
    enum Type types[2];

    GIVEN {
        ASSUME(GetSpeciesType(SPECIES_WOBBUFFET, 0) == TYPE_PSYCHIC);
        ASSUME(GetSpeciesType(SPECIES_WOBBUFFET, 1) == TYPE_PSYCHIC);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeOverviewTypes(0, types), 1);
        EXPECT_EQ(types[0], TYPE_PSYCHIC);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO overview: a Terastallized foe shows its Tera type alone")
{
    enum Type types[2];

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHARIZARD) { TeraType(TYPE_GRASS); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } THEN {
        EXPECT_EQ(GetFoeOverviewTypes(0, types), 1);
        EXPECT_EQ(types[0], TYPE_GRASS);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO overview: a Tera Stellar foe keeps its species' types")
{
    enum Type types[2];

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHARIZARD) { TeraType(TYPE_STELLAR); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } THEN {
        EXPECT_EQ(GetFoeOverviewTypes(0, types), 2);
        EXPECT_EQ(types[0], TYPE_FIRE);
        EXPECT_EQ(types[1], TYPE_FLYING);
    }
}

SINGLE_BATTLE_TEST("Frontier INFO overview: an Illusion foe shows the disguise's types")
{
    enum Type types[2];

    GIVEN {
        ASSUME(GetSpeciesType(SPECIES_MACHAMP, 0) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_MACHAMP, 1) == TYPE_FIGHTING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); }
        OPPONENT(SPECIES_MACHAMP);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetFoeOverviewTypes(0, types), 1);
        EXPECT_EQ(types[0], TYPE_FIGHTING);
    }
}
