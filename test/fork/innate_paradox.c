#include "global.h"
#include "test/battle.h"

// Innate Protosynthesis / Quark Drive (FEATURE_INNATE_ABILITIES). Both are innate on their canon
// Paradox carriers only, so each one's observable slot holds a species ability override instead
// (Walking Wake -> Water Absorb, Iron Moth -> Toxic Debris, ...). These tests force that override as
// the chosen ability and check the innate still does everything the real ability does. The wiring is
// documented in src/fork/innate_abilities.c (GetInnateParadoxAbility and its neighbours).

ASSUMPTIONS
{
    ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
    ASSUME(GetMoveCategory(MOVE_ROUND) == DAMAGE_CATEGORY_SPECIAL);
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Protosynthesis activates in harsh sunlight")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WALKING_WAKE) { Ability(ABILITY_WATER_ABSORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SUNNY_DAY); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, player);
        ABILITY_POPUP(player, ABILITY_PROTOSYNTHESIS);
        MESSAGE("The harsh sunlight activated Walking Wake's Protosynthesis!");
        MESSAGE("Walking Wake's Sp. Atk was heightened!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Protosynthesis does nothing with the feature off")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_WALKING_WAKE) { Ability(ABILITY_WATER_ABSORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SUNNY_DAY); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, player);
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_PROTOSYNTHESIS);
            MESSAGE("The harsh sunlight activated Walking Wake's Protosynthesis!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Quark Drive activates on switch-in to Electric Terrain")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_IRON_MOTH) { Ability(ABILITY_TOXIC_DEBRIS); }
        OPPONENT(SPECIES_TAPU_KOKO) { Ability(ABILITY_ELECTRIC_SURGE); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_ELECTRIC_SURGE);
        ABILITY_POPUP(player, ABILITY_QUARK_DRIVE);
        MESSAGE("The Electric Terrain activated Iron Moth's Quark Drive!");
        MESSAGE("Iron Moth's Sp. Atk was heightened!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Protosynthesis boosts the highest attacking stat by 30% in sun")
{
    s16 damage[2];

    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SLITHER_WING) { Ability(ABILITY_FLAME_BODY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.3), damage[1]);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Quark Drive raises Speed in Electric Terrain")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_IRON_VALIANT) { Ability(ABILITY_TRACE); Attack(100); Defense(100); SpAttack(100); SpDefense(100); Speed(110); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(120); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_ELECTRIC_TERRAIN); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ELECTRIC_TERRAIN, opponent);
        ABILITY_POPUP(player, ABILITY_QUARK_DRIVE);
        MESSAGE("Iron Valiant's Speed was heightened!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        // 110 Speed x1.5 = 165 now outruns the opponent's 120.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Booster Energy activates an innate Quark Drive")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_IRON_MOTH) { Ability(ABILITY_TOXIC_DEBRIS); Item(ITEM_BOOSTER_ENERGY); Attack(100); Defense(100); SpAttack(110); SpDefense(100); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); }
    } WHEN {
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        ABILITY_POPUP(player, ABILITY_QUARK_DRIVE);
        MESSAGE("Iron Moth used its Booster Energy to activate Quark Drive!");
        MESSAGE("Iron Moth's Sp. Atk was heightened!");
    } THEN {
        EXPECT(player->item == ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Booster Energy's innate Protosynthesis boost raises Sp. Atk by 30%", s16 damage)
{
    enum Item item;

    PARAMETRIZE { item = ITEM_NONE; }
    PARAMETRIZE { item = ITEM_BOOSTER_ENERGY; }

    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_RAGING_BOLT) { Ability(ABILITY_VOLT_ABSORB); Item(item); Attack(100); Defense(100); Speed(100); SpAttack(110); SpDefense(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_ROUND); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ROUND, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Protosynthesis")
{
    s16 damage[2];

    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        ASSUME(GetMoveEffect(MOVE_GASTRO_ACID) == EFFECT_GASTRO_ACID);
        PLAYER(SPECIES_SLITHER_WING) { Ability(ABILITY_FLAME_BODY); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(200); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUNNY_DAY, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_GASTRO_ACID, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        EXPECT_MUL_EQ(damage[1], Q_4_12(1.3), damage[0]);
    }
}
