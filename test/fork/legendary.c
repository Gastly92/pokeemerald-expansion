#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"
#include "fork/species_tiers.h"

// Legendary: Mythical's twin (src/fork/mythical.c), raising max HP by 50% instead of doubling it. The shared
// mechanics (Neutralizing Gas, Gastro Acid, Mold Breaker, Ditto, Regenerator, the AI) are covered once in
// test/fork/mythical.c; these cover what the x1.5 multiplier changes, chiefly that it rounds and unrounds exactly.

TEST("Legendary: every TIER_LEGENDARY species carries the Legendary innate")
{
    u32 missing = 0;

    for (enum Species species = 1; species < NUM_SPECIES; species++)
    {
        if (GetSpeciesTier(species) != TIER_LEGENDARY || SpeciesHasInnate(species, ABILITY_LEGENDARY))
            continue;
        DebugPrintf("TIER_LEGENDARY species %S lacks the Legendary innate", GetSpeciesName(species));
        missing++;
    }
    EXPECT_EQ(missing, 0);
}

TEST("Legendary: no species carries both Legendary and Mythical")
{
    u32 both = 0;

    for (enum Species species = 1; species < NUM_SPECIES; species++)
    {
        if (!SpeciesHasInnate(species, ABILITY_LEGENDARY) || !SpeciesHasInnate(species, ABILITY_MYTHICAL))
            continue;
        DebugPrintf("%S carries both Legendary and Mythical", GetSpeciesName(species));
        both++;
    }
    EXPECT_EQ(both, 0);
}

TEST("Legendary: cannot be suppressed, broken, copied or swapped")
{
    EXPECT(gAbilitiesInfo[ABILITY_LEGENDARY].cantBeSuppressed);
    EXPECT(!gAbilitiesInfo[ABILITY_LEGENDARY].breakable);
    EXPECT(gAbilitiesInfo[ABILITY_LEGENDARY].cantBeCopied);
    EXPECT(gAbilitiesInfo[ABILITY_LEGENDARY].cantBeSwapped);
    EXPECT(gAbilitiesInfo[ABILITY_LEGENDARY].cantBeTraced);
}

SINGLE_BATTLE_TEST("Legendary announces itself and raises HP and max HP by 50% on entry")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LEGENDARY); MaxHP(200); HP(50); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LEGENDARY);
        MESSAGE("Wobbuffet's legendary presence fills the battlefield!");
        NOT MESSAGE("Wobbuffet's mythical presence fills the battlefield!");
    } THEN {
        EXPECT_EQ(player->maxHP, 300);
        EXPECT_EQ(player->hp, 75);
    }
}

SINGLE_BATTLE_TEST("Legendary rounds an odd max HP down, and a full-HP holder stays at full")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LEGENDARY); MaxHP(201); HP(201); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_EQ(player->maxHP, 301);
        EXPECT_EQ(player->hp, 301);
    }
}

SINGLE_BATTLE_TEST("Legendary's boost is undone exactly on switch-out and re-applied on the next entry")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LEGENDARY); MaxHP(201); HP(151); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LEGENDARY);
        SWITCH_OUT_MESSAGE("Wobbuffet");
        SWITCH_OUT_MESSAGE("Wynaut");
        ABILITY_POPUP(player, ABILITY_LEGENDARY);
        SWITCH_OUT_MESSAGE("Wobbuffet");
    } THEN {
        // 201/151 -> 301/226 on entry -> back to exactly 201/151, twice over.
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP), 201);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 151);
    }
}

SINGLE_BATTLE_TEST("Legendary multiplies with Dynamax's HP boost and survives Dynamax ending", u16 maxHPAfter)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; }
    PARAMETRIZE { ability = ABILITY_LEGENDARY; }
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
        EXPECT_EQ(results[1].maxHPAfter, results[0].maxHPAfter * 3 / 2);
    }
}

SINGLE_BATTLE_TEST("Legendary: Leftovers heals 1/16 of the unboosted max HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_LEGENDARY); MaxHP(160); HP(80); Item(ITEM_LEFTOVERS); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        HP_BAR(player, damage: -10);
    } THEN {
        EXPECT_EQ(player->hp, 130);
        EXPECT_EQ(player->maxHP, 240);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Legendary raises HP by 50% on entry", u16 maxHP)
{
    bool32 enabled;
    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_REGIROCK, ABILITY_LEGENDARY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_REGIROCK) { Ability(ABILITY_CLEAR_BODY); MaxHP(200); HP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_LEGENDARY); // pop-up shows the innate, not the chosen ability
            MESSAGE("Regirock's legendary presence fills the battlefield!");
        } else {
            NOT MESSAGE("Regirock's legendary presence fills the battlefield!");
        }
    } THEN {
        EXPECT_EQ(player->maxHP, enabled ? 300 : 200);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Legendary keeps its x1.5 boost through Mega Evolution")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_LATIOS, ABILITY_LEGENDARY));
        ASSUME(SpeciesHasInnate(SPECIES_LATIOS_MEGA, ABILITY_LEGENDARY));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_LATIOS) { Item(ITEM_LATIOSITE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
    } THEN {
        // THEN runs after the battle ends, when the party slot has already been reverted.
        EXPECT_EQ(player->species, SPECIES_LATIOS_MEGA);
        EXPECT_EQ(player->maxHP, GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MAX_HP) * 3 / 2);
    }
}
