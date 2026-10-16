#include "global.h"
#include "test/battle.h"
#include "battle_frontier.h"
#include "fork/free_gimmicks.h"
#include "fork/frontier_extended_mons.h"

// Coverage for FEATURE_FREE_GIMMICKS (config/feature.h). The flag drops the
// held-item / key-item / charge requirements for the battle transformation
// gimmicks and lets a mon pick from several via the move-selection picker. The
// flag defaults off in the test baseline (see TestInitConfigData) so the inherited
// gimmick suite keeps exercising the stock item-gated behavior; each test here
// opts in with WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE).

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a mon can Mega Evolve without holding a Mega Stone")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(gSpeciesInfo[SPECIES_VENUSAUR].isMegaEvolution == FALSE);
        PLAYER(SPECIES_VENUSAUR); // no item
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
        MESSAGE("Venusaur has Mega Evolved into Mega Venusaur!");
    } THEN {
        EXPECT_EQ(player->species, SPECIES_VENUSAUR_MEGA);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a physically inclined mon picks the X Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(200); SpAttack(100); } // physical -> X
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_X);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a specially inclined mon picks the Y Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(100); SpAttack(200); } // special -> Y
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_Y);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: an equal-stat mon defaults to the X Mega form")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CHARIZARD) { Attack(150); SpAttack(150); } // tie -> X
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        EXPECT_EQ(player->species, SPECIES_CHARIZARD_MEGA_X);
    }
}

// Every species with two Megas, and the stone (hence form) FindMegaStoneForStats should pick for a
// physical and a special build. The pick follows the Megas' own base stats, not their letters, so
// it is not always "X physical": Mega Absol Z out-leans Mega Absol physically. `unreachedStone`
// names a form no roster set Mega Evolves into, which the roster test below would otherwise flag.
struct TwoMegaPick
{
    u16 species;
    u16 physicalStone;
    u16 specialStone;
    u16 unreachedStone;
};

static const struct TwoMegaPick sTwoMegaPicks[] =
{
    { SPECIES_CHARIZARD, ITEM_CHARIZARDITE_X, ITEM_CHARIZARDITE_Y, ITEM_NONE },
    { SPECIES_RAICHU,    ITEM_RAICHUNITE_X,   ITEM_RAICHUNITE_Y,   ITEM_NONE },
    { SPECIES_MEWTWO,    ITEM_MEWTWONITE_X,   ITEM_MEWTWONITE_Y,   ITEM_NONE },
    // All three Absol sets are physical, so only a special Absol would ever reach plain Mega Absol.
    { SPECIES_ABSOL,     ITEM_ABSOLITE_Z,     ITEM_ABSOLITE,       ITEM_ABSOLITE },
    { SPECIES_GARCHOMP,  ITEM_GARCHOMPITE,    ITEM_GARCHOMPITE_Z,  ITEM_NONE },
    { SPECIES_LUCARIO,   ITEM_LUCARIONITE,    ITEM_LUCARIONITE_Z,  ITEM_NONE },
};

static u32 CountMegaStones(u32 species)
{
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    u32 i, count = 0;

    if (formChanges == NULL)
        return 0;
    for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM)
            count++;
    }
    return count;
}

static const struct TwoMegaPick *FindTwoMegaPick(u32 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sTwoMegaPicks); i++)
    {
        if (sTwoMegaPicks[i].species == species)
            return &sTwoMegaPicks[i];
    }
    return NULL;
}

TEST("FREE_GIMMICKS: every species with two Megas picks the right one from its stats")
{
    u32 i, species;

    for (i = 0; i < ARRAY_COUNT(sTwoMegaPicks); i++)
    {
        EXPECT_EQ(FindMegaStoneForStats(sTwoMegaPicks[i].species, 200, 100), sTwoMegaPicks[i].physicalStone);
        EXPECT_EQ(FindMegaStoneForStats(sTwoMegaPicks[i].species, 100, 200), sTwoMegaPicks[i].specialStone);
        EXPECT_EQ(FindMegaStoneForStats(sTwoMegaPicks[i].species, 150, 150), sTwoMegaPicks[i].physicalStone);
    }

    // The table must be complete: a species upstream gives a second Mega fails here until it gets a
    // row, so its pick is a decision rather than whatever the base stats happen to say. Mega, Primal
    // and G-Max forms share their base's form-change table, so only the base species is checked.
    for (species = 1; species < NUM_SPECIES; species++)
    {
        if (!IsSpeciesEnabled(species) || gSpeciesInfo[species].isMegaEvolution
         || gSpeciesInfo[species].isPrimalReversion || gSpeciesInfo[species].isGigantamax)
            continue;
        if (CountMegaStones(species) < 2)
            continue;
        if (FindTwoMegaPick(species) == NULL)
            Test_MgbaPrintf("%S (species %d) has two Megas but no sTwoMegaPicks row", gSpeciesInfo[species].speciesName, species);
        EXPECT(FindTwoMegaPick(species) != NULL);
    }
}

// The roster side of the same question: the pick reads the drafted mon's real Attack and Sp. Atk,
// so a species' sets decide which of its Megas can ever appear in the Frontier. Every Mega must be
// reached by at least one set (or be named in unreachedStone), and each set must pick the same form
// at level 50 and level 100, so a spread that sits on the boundary cannot flip between the two.
// Because it builds real mons, this also catches a set that does not get the nature it names: it
// first failed on Mewtwo's Jolly set, which CreateFacilityMon turned into Lax (a u32 underflow when
// moving a zero personality to any nature above 12; upstream fixed it in #10906).
TEST("FREE_GIMMICKS: the roster's sets reach every Mega of a two-Mega species")
{
    u32 i, j;
    struct Pokemon mon;

    for (i = 0; i < ARRAY_COUNT(sTwoMegaPicks); i++)
    {
        const struct TwoMegaPick *pick = &sTwoMegaPicks[i];
        bool32 reachedPhysical = FALSE, reachedSpecial = FALSE;

        for (j = 0; j < gFrontierExtendedMonsCount; j++)
        {
            u16 stoneAt50, stoneAt100;

            if (gFrontierExtendedMons[j].species != pick->species)
                continue;
            CreateFacilityMon(&gFrontierExtendedMons[j], 50, MAX_PER_STAT_IVS, 0, 0, &mon);
            stoneAt50 = FindMegaStoneForStats(pick->species, GetMonData(&mon, MON_DATA_ATK), GetMonData(&mon, MON_DATA_SPATK));
            CreateFacilityMon(&gFrontierExtendedMons[j], 100, MAX_PER_STAT_IVS, 0, 0, &mon);
            stoneAt100 = FindMegaStoneForStats(pick->species, GetMonData(&mon, MON_DATA_ATK), GetMonData(&mon, MON_DATA_SPATK));

            if (stoneAt50 != stoneAt100)
                Test_MgbaPrintf("%S set %d picks a different Mega at level 50 and 100", gSpeciesInfo[pick->species].speciesName, j);
            EXPECT_EQ(stoneAt50, stoneAt100);
            if (stoneAt50 == pick->physicalStone)
                reachedPhysical = TRUE;
            if (stoneAt50 == pick->specialStone)
                reachedSpecial = TRUE;
        }

        // An unreachedStone entry must still be true, or it is stale and should be removed.
        if (reachedPhysical == (pick->unreachedStone == pick->physicalStone)
         || reachedSpecial == (pick->unreachedStone == pick->specialStone))
            Test_MgbaPrintf("%S: physical Mega reached=%d, special Mega reached=%d", gSpeciesInfo[pick->species].speciesName, reachedPhysical, reachedSpecial);
        EXPECT_EQ(reachedPhysical, pick->unreachedStone != pick->physicalStone);
        EXPECT_EQ(reachedSpecial, pick->unreachedStone != pick->specialStone);
    }
}

// Regression: with item-free Mega Evolution the held Mega Stone no longer gates
// the gimmick, which previously let a transformed mon (Ditto/Imposter, which copies its
// target's species but not its item) arm Mega Evolution. The form change is then refused
// by CanBattlerFormChange (B_TRANSFORM_FORM_CHANGES >= GEN_5) AFTER the gimmick was marked
// used, so nothing happened yet the Mega was spent. CanMegaEvolve now blocks transformed
// mons up front, so the option is never offered and the gimmick is preserved.
SINGLE_BATTLE_TEST("FREE_GIMMICKS: a transformed mon cannot Mega Evolve")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        ASSUME(GetConfig(B_TRANSFORM_FORM_CHANGES) >= GEN_5);
        // Transform copies the target's moves into battle while the move-slot the
        // test resolves comes from Ditto's party moveset, so align Celebrate to the
        // same slot (1) in both so turn 2's MOVE is legal after transforming.
        PLAYER(SPECIES_DITTO) { Moves(MOVE_TRANSFORM, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_VENUSAUR) { Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRANSFORM); }
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } SCENE {
        MESSAGE("Ditto transformed into Venusaur!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, player);
            MESSAGE("Venusaur has Mega Evolved into Mega Venusaur!");
        }
    } THEN {
        // Stays the transformed (non-Mega) Venusaur; the Mega gimmick is never consumed.
        EXPECT_EQ(player->species, SPECIES_VENUSAUR);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a mon can use a Z-Move without holding a Z-Crystal")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_PIKACHU) { Moves(MOVE_THUNDERBOLT); } // no item
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        MESSAGE("Pikachu surrounded itself with its Z-Power!");
        MESSAGE("Pikachu unleashes its full-force Z-Move!");
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: the player can Terastallize without a Tera Orb")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { TeraType(TYPE_GHOST); } // no Tera Orb / charge flag
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_TERA); }
    } SCENE {
        MESSAGE("Wobbuffet terastallized into the Ghost type!");
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: the player can Dynamax without a Dynamax Band")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); } // no Dynamax Band / B_FLAG
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        // Dynamax turns the chosen move into its Max Move.
        MESSAGE("Wobbuffet used Max Strike!");
    }
}

// A mon with every move blocked still reaches the move menu when a Z-Move or Dynamax would
// unlock one (src/fork/free_gimmick_struggle.c). The setup is the reported case:
// choice-locked into Tackle, then Tackle is disabled.
SINGLE_BATTLE_TEST("FREE_GIMMICKS: a choice-locked mon whose move is disabled can still Z-Move it")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_Z_MOVE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BREAKNECK_BLITZ, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a choice-locked mon whose move is disabled can still Dynamax")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MAX_STRIKE, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: picking a blocked move without arming a gimmick still Struggles")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: with the flag off a choice-locked mon whose move is disabled goes straight to Struggle")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, allowed: FALSE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
    }
}
