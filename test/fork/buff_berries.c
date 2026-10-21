#include "global.h"
#include "test/battle.h"

// Coverage for the BUFF_RESIST_BERRIES, BUFF_PINCH_BERRIES, BUFF_RETALIATION_BERRIES and
// BUFF_DAMAGED_STAT_BERRIES flags (config/buff.h). BUFF_*
// flags default off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG and the stock behavior is pinned with the flag explicitly off. The stock numbers
// are also covered at length by test/battle/hold_effect/resist_berry.c, attack_up.c,
// jaboca_berry.c, rowap_berry.c, kee_berry.c and maranga_berry.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_OCCA_BERRY) == HOLD_EFFECT_RESIST_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_YACHE_BERRY) == HOLD_EFFECT_RESIST_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_LIECHI_BERRY) == HOLD_EFFECT_ATTACK_UP);
    ASSUME(GetItemHoldEffect(ITEM_STARF_BERRY) == HOLD_EFFECT_RANDOM_STAT_UP);
    ASSUME(GetItemHoldEffect(ITEM_JABOCA_BERRY) == HOLD_EFFECT_JABOCA_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_ROWAP_BERRY) == HOLD_EFFECT_ROWAP_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_KEE_BERRY) == HOLD_EFFECT_KEE_BERRY);
    ASSUME(GetItemHoldEffect(ITEM_MARANGA_BERRY) == HOLD_EFFECT_MARANGA_BERRY);
    ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
    ASSUME(GetMoveCategory(MOVE_SWIFT) == DAMAGE_CATEGORY_SPECIAL);
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

SINGLE_BATTLE_TEST("BUFF_RETALIATION_BERRIES: Jaboca and Rowap Berry cost the attacker 1/4 of its max HP")
{
    s16 damage;
    u32 config;
    enum Item item;
    enum Move move;

    PARAMETRIZE { config = TRUE;  item = ITEM_JABOCA_BERRY; move = MOVE_SCRATCH; }
    PARAMETRIZE { config = TRUE;  item = ITEM_ROWAP_BERRY;  move = MOVE_SWIFT; }
    PARAMETRIZE { config = FALSE; item = ITEM_JABOCA_BERRY; move = MOVE_SCRATCH; }
    PARAMETRIZE { config = FALSE; item = ITEM_ROWAP_BERRY;  move = MOVE_SWIFT; }

    GIVEN {
        WITH_CONFIG(BUFF_RETALIATION_BERRIES, config);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(item); }
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
        HP_BAR(player, captureDamage: &damage);
    } THEN {
        EXPECT_EQ(damage, player->maxHP / (config ? 4 : 8));
    }
}

SINGLE_BATTLE_TEST("BUFF_RETALIATION_BERRIES: Ripen still doubles Jaboca Berry, to 1/2")
{
    s16 damage;
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_RETALIATION_BERRIES, config);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_APPLIN) { Ability(ABILITY_RIPEN); Item(ITEM_JABOCA_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
        HP_BAR(player, captureDamage: &damage);
    } THEN {
        EXPECT_EQ(damage, (player->maxHP / (config ? 4 : 8)) * 2);
    }
}

SINGLE_BATTLE_TEST("BUFF_RETALIATION_BERRIES: Magic Guard still blocks the recoil")
{
    GIVEN {
        WITH_CONFIG(BUFF_RETALIATION_BERRIES, TRUE);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_MAGIC_GUARD); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_JABOCA_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(opponent);
        NOT HP_BAR(player);
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

SINGLE_BATTLE_TEST("BUFF_DAMAGED_STAT_BERRIES: Kee and Maranga Berry raise their stat by 2")
{
    u32 config;
    enum Item item;
    enum Move move;
    enum Stat stat;

    PARAMETRIZE { config = TRUE;  item = ITEM_KEE_BERRY;     move = MOVE_SCRATCH; stat = STAT_DEF; }
    PARAMETRIZE { config = TRUE;  item = ITEM_MARANGA_BERRY; move = MOVE_SWIFT;   stat = STAT_SPDEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_KEE_BERRY;     move = MOVE_SCRATCH; stat = STAT_DEF; }
    PARAMETRIZE { config = FALSE; item = ITEM_MARANGA_BERRY; move = MOVE_SWIFT;   stat = STAT_SPDEF; }

    GIVEN {
        WITH_CONFIG(BUFF_DAMAGED_STAT_BERRIES, config);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(item); }
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
    } THEN {
        EXPECT_EQ(opponent->item, ITEM_NONE);
        EXPECT_EQ(opponent->statStages[stat], DEFAULT_STAT_STAGE + (config ? 2 : 1));
    }
}

SINGLE_BATTLE_TEST("BUFF_DAMAGED_STAT_BERRIES: Ripen still doubles Kee Berry, to +4")
{
    u32 config;

    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }

    GIVEN {
        WITH_CONFIG(BUFF_DAMAGED_STAT_BERRIES, config);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_APPLIN) { Ability(ABILITY_RIPEN); Item(ITEM_KEE_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(opponent);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_DEF], DEFAULT_STAT_STAGE + (config ? 4 : 2));
    }
}
