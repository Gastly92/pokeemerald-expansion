#include "global.h"
#include "test/battle.h"

// Coverage for the BUFF_RESIST_BERRIES and BUFF_PINCH_BERRIES flags (config/buff.h). BUFF_*
// flags default off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG and the stock behavior is pinned with the flag explicitly off. The stock numbers
// are also covered at length by test/battle/hold_effect/resist_berry.c and attack_up.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_OCCA_BERRY) == HOLD_EFFECT_RESIST_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_YACHE_BERRY) == HOLD_EFFECT_RESIST_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_LIECHI_BERRY) == HOLD_EFFECT_ATTACK_UP);
    ASSUME(GetItemHoldEffect(ITEM_STARF_BERRY) == HOLD_EFFECT_RANDOM_STAT_UP);
    ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
    ASSUME(GetMoveType(MOVE_AURORA_BEAM) == TYPE_ICE);
}

SINGLE_BATTLE_TEST("BUFF_RESIST_BERRIES: a resist Berry cuts a super-effective hit to 1/4", s16 damage)
{
    u32 config;
    enum Item item;

    PARAMETRIZE { config = TRUE;  item = ITEM_NONE; }
    PARAMETRIZE { config = TRUE;  item = ITEM_OCCA_BERRY; }
    PARAMETRIZE { config = FALSE; item = ITEM_NONE; }
    PARAMETRIZE { config = FALSE; item = ITEM_OCCA_BERRY; }

    GIVEN {
        WITH_CONFIG(BUFF_RESIST_BERRIES, config);
        ASSUME(GetSpeciesType(SPECIES_GOGOAT, 0) == TYPE_GRASS);
        ASSUME(GetSpeciesType(SPECIES_GOGOAT, 1) == TYPE_GRASS);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GOGOAT) { Item(item); }
    } WHEN {
        TURN { MOVE(player, MOVE_EMBER); }
    } SCENE {
        if (item != ITEM_NONE)
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.25), results[1].damage);
        EXPECT_MUL_EQ(results[2].damage, Q_4_12(0.5), results[3].damage); // stock
    }
}

SINGLE_BATTLE_TEST("BUFF_RESIST_BERRIES: Ripen still doubles the reduction, to 1/8", s16 damage)
{
    u32 config;
    enum Item item;

    PARAMETRIZE { config = TRUE;  item = ITEM_NONE; }
    PARAMETRIZE { config = TRUE;  item = ITEM_YACHE_BERRY; }
    PARAMETRIZE { config = FALSE; item = ITEM_NONE; }
    PARAMETRIZE { config = FALSE; item = ITEM_YACHE_BERRY; }

    GIVEN {
        WITH_CONFIG(BUFF_RESIST_BERRIES, config);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_APPLIN) { Ability(ABILITY_RIPEN); Item(item); }
    } WHEN {
        TURN { MOVE(player, MOVE_AURORA_BEAM); }
    } SCENE {
        if (item != ITEM_NONE)
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.125), results[1].damage);
        EXPECT_MUL_EQ(results[2].damage, Q_4_12(0.25), results[3].damage); // stock
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES: Liechi Berry raises Attack by three stages at 1/4 HP")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_LIECHI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        MESSAGE("The Liechi Berry drastically boosted Wobbuffet's Attack!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 3);
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES: Ripen doubles it, to +6")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, TRUE);
        PLAYER(SPECIES_APPLIN) { MaxHP(160); HP(80); Ability(ABILITY_RIPEN); Item(ITEM_LIECHI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_RIPEN);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], MAX_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES off: Liechi Berry raises Attack by its stock one stage")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_LIECHI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES: Starf Berry raises the highest stat by three stages")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Attack(300); Item(ITEM_STARF_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } THEN {
        // DETERMINISTIC_HOLD_EFFECTS picks the highest stat, which Attack(300) makes Attack.
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 3);
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES: Starf Berry with Ripen raises the highest stat to +6")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, TRUE);
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_APPLIN) { MaxHP(160); HP(80); Attack(300); Ability(ABILITY_RIPEN); Item(ITEM_STARF_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], MAX_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("BUFF_PINCH_BERRIES off: Starf Berry raises the highest stat by its stock two stages")
{
    GIVEN {
        WITH_CONFIG(BUFF_PINCH_BERRIES, FALSE);
        WITH_CONFIG(DETERMINISTIC_HOLD_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Attack(300); Item(ITEM_STARF_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
    }
}
