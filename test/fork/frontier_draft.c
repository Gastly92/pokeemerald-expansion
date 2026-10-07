#include "global.h"
#include "test/test.h"
#include "data.h"
#include "battle_factory.h"
#include "battle_frontier.h"
#include "event_data.h"
#include "frontier_util.h"
#include "pokemon.h"
#include "constants/frontier_util.h"
#include "fork/frontier_extended_mons.h"
#include "constants/battle_factory.h"
#include "constants/battle_frontier.h"
#include "fork/frontier_draft.h"
#include "fork/species_tiers.h"
#include "constants/abilities.h"

// Guards IllusionMonRejectsSlot (src/fork/frontier_draft.c). Illusion
// disguises its holder as the team's last conscious party member, so an Illusion
// mon drafted into the final slot has nothing to copy and the disguise never
// forms (GetIllusionMonPartyId bails). The draft loops use this helper to keep
// Illusion mons out of that slot; this test pins its slot/ability logic.

static const struct TrainerMon sIllusionMon = { .species = SPECIES_ZOROARK, .ability = ABILITY_ILLUSION };
static const struct TrainerMon sPlainMon    = { .species = SPECIES_PIKACHU, .ability = ABILITY_STATIC };

TEST("Frontier draft: Illusion mon is rejected only from the last slot")
{
    // 6v6: reject in slot 5, allow in every earlier slot.
    EXPECT(IllusionMonRejectsSlot(5, 6, &sIllusionMon));
    EXPECT(!IllusionMonRejectsSlot(0, 6, &sIllusionMon));
    EXPECT(!IllusionMonRejectsSlot(4, 6, &sIllusionMon));

    // 3v3 (and shorter multi/doubles teams): "last slot" tracks partySize - 1.
    EXPECT(IllusionMonRejectsSlot(2, 3, &sIllusionMon));
    EXPECT(!IllusionMonRejectsSlot(1, 3, &sIllusionMon));
}

TEST("Frontier draft: a non-Illusion mon is never rejected for slot placement")
{
    EXPECT(!IllusionMonRejectsSlot(5, 6, &sPlainMon));
    EXPECT(!IllusionMonRejectsSlot(2, 3, &sPlainMon));
    EXPECT(!IllusionMonRejectsSlot(0, 6, &sPlainMon));
}

// Species Clause by dex number (SpeciesListHasDexNum): upstream's draft and party
// checks compare exact species ids, which let two formes with their own ids share a
// team. These pin the dex-number comparison the draft loops and the player-side
// checks now use.
TEST("Frontier draft: Species Clause treats formes as one Pokémon")
{
    static const enum Species team[] = { SPECIES_SILVALLY_FIRE, SPECIES_NINETALES, SPECIES_NIDORAN_F };

    EXPECT(SpeciesListHasDexNum(team, ARRAY_COUNT(team), SPECIES_SILVALLY_WATER));
    EXPECT(SpeciesListHasDexNum(team, ARRAY_COUNT(team), SPECIES_SILVALLY_FIRE));
    EXPECT(SpeciesListHasDexNum(team, ARRAY_COUNT(team), SPECIES_NINETALES_ALOLA));
    // Different dex numbers stay distinct, even when the names say otherwise.
    EXPECT(!SpeciesListHasDexNum(team, ARRAY_COUNT(team), SPECIES_NIDORAN_M));
    EXPECT(!SpeciesListHasDexNum(team, ARRAY_COUNT(team), SPECIES_ARCEUS_FIRE));
    // Only the first `count` entries are the team.
    EXPECT(!SpeciesListHasDexNum(team, 1, SPECIES_NINETALES_ALOLA));
}

static u16 FirstRosterIdOf(enum Species species)
{
    u32 i;
    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        if (gFrontierExtendedMons[i].species == species)
            return i;
    }
    return 0;
}

// Rented Pokémon whose *other* formes are in the roster, so a regression to the exact
// species-id check has something to slip through (Arceus would not do: mythicals are
// already banned from ordinary opponent slots by the tier quota).
static const enum Species sMultiFormeRentals[] =
{
    SPECIES_SILVALLY_FIRE, SPECIES_ROTOM_WASH, SPECIES_ORICORIO, SPECIES_LYCANROC,
    SPECIES_TAUROS, SPECIES_NINETALES, SPECIES_RAICHU, SPECIES_MAROWAK,
    SPECIES_SLOWBRO, SPECIES_SAMUROTT, SPECIES_TYPHLOSION, SPECIES_DECIDUEYE,
};

// End to end through the Factory's own generators: the player's rental choices never repeat
// a dex number, and an opponent never fields any forme of a rented Pokémon nor two formes of
// one Pokémon.
TEST("Frontier draft: Factory rentals and opponents obey the dex-number Species Clause")
{
    u32 run, i, j;
    u32 numRented = min(ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons), ARRAY_COUNT(sMultiFormeRentals));

    gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_50;
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_FACTORY);
    VarSet(VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_SINGLES);
    gFacilityTrainerMons = gFrontierExtendedMons;

    for (run = 0; run < 100; run++)
    {
        gSpecialVar_0x8004 = BATTLE_FACTORY_FUNC_GENERATE_RENTAL_MONS;
        CallBattleFactoryFunction();
        for (i = 0; i < PARTY_SIZE; i++)
        {
            for (j = i + 1; j < PARTY_SIZE; j++)
                EXPECT_NE(SpeciesToNationalPokedexNum(gFrontierExtendedMons[gSaveBlock2Ptr->frontier.rentalMons[i].monId].species),
                          SpeciesToNationalPokedexNum(gFrontierExtendedMons[gSaveBlock2Ptr->frontier.rentalMons[j].monId].species));
        }
    }

    for (i = 0; i < ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons); i++)
        gSaveBlock2Ptr->frontier.rentalMons[i].monId = FirstRosterIdOf(sMultiFormeRentals[i % ARRAY_COUNT(sMultiFormeRentals)]);

    for (run = 0; run < 300; run++)
    {
        enum Species team[FRONTIER_PARTY_SIZE];

        gSpecialVar_0x8004 = BATTLE_FACTORY_FUNC_GENERATE_OPPONENT_MONS;
        CallBattleFactoryFunction();
        for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        {
            team[i] = gFrontierExtendedMons[gFrontierTempParty[i]].species;
            EXPECT(!SpeciesListHasDexNum(sMultiFormeRentals, numRented, team[i]));
            EXPECT(!SpeciesListHasDexNum(team, i, team[i]));
        }
    }
}

// UPSTREAM: regression test for the player-side Species Clause in AppendIfValid
// (src/frontier_util.c). Multis need two eligible Pokémon; two Rotom formes are only one,
// so the entry check must report the party ineligible (gSpecialVar_0x8004 == TRUE). With
// the old exact species-id comparison they counted as two and the party was let in.
static bool32 PartyIsIneligibleForMultis(enum Species first, enum Species second)
{
    ZeroPlayerPartyMons();
    CreateMon(&gParties[B_TRAINER_PLAYER][0], first, 50, 0, OTID_STRUCT_PRESET(0x12345678));
    CreateMon(&gParties[B_TRAINER_PLAYER][1], second, 50, 0, OTID_STRUCT_PRESET(0x12345678));
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    VarSet(VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_MULTIS);
    gSpecialVar_Result = FRONTIER_LVL_50;
    gSpecialVar_0x8004 = FRONTIER_UTIL_FUNC_CHECK_INELIGIBLE;
    CallFrontierUtilFunc();
    return gSpecialVar_0x8004;
}

TEST("Frontier draft: two formes of one Pokémon don't make a party eligible")
{
    EXPECT(PartyIsIneligibleForMultis(SPECIES_ROTOM_WASH, SPECIES_ROTOM_HEAT));
    EXPECT(PartyIsIneligibleForMultis(SPECIES_NINETALES, SPECIES_NINETALES_ALOLA));
    // Control: two different Pokémon are eligible.
    EXPECT(!PartyIsIneligibleForMultis(SPECIES_ROTOM_WASH, SPECIES_NINETALES));
}

// The endless Factory's swap after a Frontier Brain match drafts from rentalMons[3..5],
// which factory_setopponentmons fills from gFrontierTempParty. GenerateOpponentMons seeds
// that with a regular team before FillFactoryBrainParty builds the Brain's real one, so the
// Brain has to overwrite it, or the player is offered a team they never fought.
TEST("Factory: the swap after the Frontier Brain offers the Brain's team")
{
    u32 run, i;

    gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_50;
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_FACTORY);
    VarSet(VAR_FRONTIER_BATTLE_MODE, FRONTIER_MODE_SINGLES);
    gFacilityTrainerMons = gFrontierExtendedMons;

    for (run = 0; run < 20; run++)
    {
        bool32 hasLegendary = FALSE;

        gSpecialVar_0x8004 = BATTLE_FACTORY_FUNC_GENERATE_RENTAL_MONS;
        CallBattleFactoryFunction();
        gSpecialVar_0x8004 = BATTLE_FACTORY_FUNC_GENERATE_OPPONENT_MONS;
        CallBattleFactoryFunction();
        FillFactoryBrainParty();
        gSpecialVar_0x8004 = BATTLE_FACTORY_FUNC_SET_OPPONENT_MONS;
        CallBattleFactoryFunction();

        for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        {
            enum Species offered = gFrontierExtendedMons[gSaveBlock2Ptr->frontier.rentalMons[FRONTIER_PARTY_SIZE + i].monId].species;
            EXPECT_EQ(offered, GetMonData(&gParties[B_TRAINER_OPPONENT_A][i], MON_DATA_SPECIES));
            if (GetSpeciesTier(offered) == TIER_LEGENDARY)
                hasLegendary = TRUE;
        }
    #if B_FRONTIER_EXTENDED_MONS
        EXPECT(hasLegendary);
    #else
        (void)hasLegendary;
    #endif
    }
}
