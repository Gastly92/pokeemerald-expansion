#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"
#include "fork/species_tiers.h"

// Mythical: the holder's max HP doubles while it is on the field, like Dynamax, and the boost is undone
// whenever it leaves. A custom ability is a real ability, so the chosen-ability tests need no WITH_CONFIG;
// the innate tests toggle FEATURE_INNATE_ABILITIES.

TEST("Mythical: every TIER_MYTHICAL species carries the Mythical innate")
{
    u32 missing = 0;

    for (enum Species species = 1; species < NUM_SPECIES; species++)
    {
        if (GetSpeciesTier(species) != TIER_MYTHICAL || SpeciesHasInnate(species, ABILITY_MYTHICAL))
            continue;
        DebugPrintf("TIER_MYTHICAL species %S lacks the Mythical innate", GetSpeciesName(species));
        missing++;
    }
    EXPECT_EQ(missing, 0);
}

TEST("Mythical: cannot be suppressed, broken, copied or swapped")
{
    EXPECT(gAbilitiesInfo[ABILITY_MYTHICAL].cantBeSuppressed);
    EXPECT(!gAbilitiesInfo[ABILITY_MYTHICAL].breakable);
    EXPECT(gAbilitiesInfo[ABILITY_MYTHICAL].cantBeCopied);
    EXPECT(gAbilitiesInfo[ABILITY_MYTHICAL].cantBeSwapped);
    EXPECT(gAbilitiesInfo[ABILITY_MYTHICAL].cantBeTraced);
}

SINGLE_BATTLE_TEST("Mythical announces itself and doubles HP and max HP on entry", u16 hp, u16 maxHP)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; }
    PARAMETRIZE { ability = ABILITY_MYTHICAL; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        if (ability == ABILITY_MYTHICAL) {
            ABILITY_POPUP(player, ABILITY_MYTHICAL);
            MESSAGE("Wobbuffet's mythical presence fills the battlefield!");
        } else {
            NOT MESSAGE("Wobbuffet's mythical presence fills the battlefield!");
        }
    } THEN {
        results[i].hp = player->hp;
        results[i].maxHP = player->maxHP;
    } FINALLY {
        EXPECT_EQ(results[1].maxHP, results[0].maxHP * 2);
        EXPECT_EQ(results[1].hp, results[0].hp * 2);
    }
}

SINGLE_BATTLE_TEST("Mythical keeps the holder's HP fraction when it doubles")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(50); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
        EXPECT_EQ(player->hp, 100);
    }
}

SINGLE_BATTLE_TEST("Mythical's boost is undone on switch-out and re-applied on the next entry")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(150); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_MYTHICAL);
        MESSAGE("Wobbuffet's mythical presence fills the battlefield!");
        SWITCH_OUT_MESSAGE("Wobbuffet");
        SWITCH_OUT_MESSAGE("Wynaut");
        ABILITY_POPUP(player, ABILITY_MYTHICAL);
        MESSAGE("Wobbuffet's mythical presence fills the battlefield!");
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
        EXPECT_EQ(player->hp, 300);
    }
}

SINGLE_BATTLE_TEST("Mythical's boost is undone when the holder is forced out, and re-applied when it returns")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(150); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_ROAR); }
        TURN { SWITCH(player, 0); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ROAR, opponent);
        SWITCH_OUT_MESSAGE("Wynaut");
        ABILITY_POPUP(player, ABILITY_MYTHICAL);
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
        EXPECT_EQ(player->hp, 300);
    }
}

SINGLE_BATTLE_TEST("Mythical survives Dynamax ending", u16 maxHPAfter)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; }
    PARAMETRIZE { ability = ABILITY_MYTHICAL; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, gimmick: GIMMICK_DYNAMAX); }
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        MESSAGE("Wobbuffet used Max Strike!");
        MESSAGE("Wobbuffet used Max Strike!");
        MESSAGE("Wobbuffet used Max Strike!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
    } THEN {
        results[i].maxHPAfter = player->maxHP;
    } FINALLY {
        EXPECT_EQ(results[1].maxHPAfter, results[0].maxHPAfter * 2);
    }
}

SINGLE_BATTLE_TEST("Mythical multiplies with Dynamax's HP boost", u16 maxHP)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; }
    PARAMETRIZE { ability = ABILITY_MYTHICAL; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
    } THEN {
        results[i].maxHP = player->maxHP;
    } FINALLY {
        EXPECT_EQ(results[1].maxHP, results[0].maxHP * 2);
    }
}

SINGLE_BATTLE_TEST("Mythical: Leftovers heals 1/16 of the unboosted max HP, like Dynamax")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(160); HP(80); Item(ITEM_LEFTOVERS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        HP_BAR(player, damage: -10);
    } THEN {
        EXPECT_EQ(player->hp, 170);
        EXPECT_EQ(player->maxHP, 320);
    }
}

SINGLE_BATTLE_TEST("Mythical still doubles HP under Neutralizing Gas")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(200); }
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); }
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_NEUTRALIZING_GAS);
        MESSAGE("Wynaut's mythical presence fills the battlefield!");
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
    }
}

SINGLE_BATTLE_TEST("Mythical cannot be removed by Gastro Acid")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GASTRO_ACID) == EFFECT_GASTRO_ACID);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet used Gastro Acid!");
        MESSAGE("But it failed!");
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
    }
}

SINGLE_BATTLE_TEST("Mythical's boost holds against a Mold Breaker attacker")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); MaxHP(200); HP(200); }
        OPPONENT(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
    } THEN {
        EXPECT_EQ(player->maxHP, 400);
    }
}

SINGLE_BATTLE_TEST("Mythical: a Ditto that transforms into a Mythical does not get the boost")
{
    u16 dittoMaxHP;
    GIVEN {
        PLAYER(SPECIES_DITTO) { Ability(ABILITY_IMPOSTER); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_MYTHICAL); }
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_IMPOSTER);
        NOT MESSAGE("Ditto's mythical presence fills the battlefield!");
    } THEN {
        dittoMaxHP = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP);
        EXPECT_EQ(player->maxHP, dittoMaxHP);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mythical doubles HP on entry", u16 maxHP)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEWTWO, ABILITY_MYTHICAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_MEWTWO) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_MYTHICAL); // pop-up shows the innate, not the chosen ability
            MESSAGE("Mewtwo's mythical presence fills the battlefield!");
        } else {
            NOT MESSAGE("Mewtwo's mythical presence fills the battlefield!");
        }
    } THEN {
        results[i].maxHP = player->maxHP;
    } FINALLY {
        EXPECT_EQ(results[1].maxHP, results[0].maxHP * 2);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mythical survives Gastro Acid")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MEWTWO, ABILITY_MYTHICAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MEWTWO) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_GASTRO_ACID, opponent);
    } THEN {
        // THEN runs after the battle ends, when the party slot has already been reverted.
        EXPECT(player->volatiles.gastroAcid);
        EXPECT_EQ(player->maxHP, GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP) * 2);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Mythical keeps its boost through Primal Reversion", u16 maxHP)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_KYOGRE_PRIMAL, ABILITY_MYTHICAL));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_KYOGRE) { Item(ITEM_BLUE_ORB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_PRIMAL_REVERSION, player);
    } THEN {
        EXPECT_EQ(player->species, SPECIES_KYOGRE_PRIMAL);
        results[i].maxHP = player->maxHP;
    } FINALLY {
        EXPECT_EQ(results[1].maxHP, results[0].maxHP * 2);
    }
}
