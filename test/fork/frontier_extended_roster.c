#include "global.h"
#include "test/test.h"
#include "data.h"
#include "battle_frontier.h"
#include "fork/frontier_extended_mons.h"
#include "fork/innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: guards the fork-owned competitive Battle Factory roster
// (gFrontierExtendedMons, src/frontier_extended_mons.c). A set's .ability is
// resolved by CreateFacilityMon (src/battle_frontier.c) into a 2-bit ability
// *slot* on the mon, so it must be one of the species' real abilities. An
// off-list ability has no slot to live in and silently falls back to slot 0,
// quietly running a different ability than the set intends (this is how the
// Munkidori "Regenerator pivot" was actually battling as Toxic Chain). This
// test loops the whole roster and fails loudly on any such illegal entry.
TEST("Frontier extended roster: every set's ability is legal for its species")
{
    u32 i, slot;
    u32 illegalCount = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        enum Ability ability = set->ability;
        bool32 isLegal = FALSE;

        // ABILITY_NONE means "let the Factory pick", which is always valid.
        if (ability == ABILITY_NONE)
            continue;

        for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            if (GetSpeciesAbility(set->species, slot) == ability)
            {
                isLegal = TRUE;
                break;
            }
        }

        if (!isLegal)
        {
            illegalCount++;
            Test_MgbaPrintf("roster[%d] %S: illegal %S | slots %S/%S/%S | innL=%d innR=%d",
                            i,
                            gSpeciesInfo[set->species].speciesName,
                            gAbilitiesInfo[ability].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 0)].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 1)].name,
                            gAbilitiesInfo[GetSpeciesAbility(set->species, 2)].name,
                            SpeciesHasInnate(set->species, ABILITY_LEVITATE),
                            SpeciesHasInnate(set->species, ABILITY_REGENERATOR));
        }
    }

    EXPECT_EQ(illegalCount, 0);
}

// FORK: CreateFacilityMon grants the Gigantamax Factor at draft time to any mon
// whose species has a G-Max form, so gmax-capable Factory/Tower mons Gigantamax
// instead of plain Dynamaxing (without annotating each roster entry). A species
// with no G-Max form must NOT receive the factor.
TEST("Frontier extended roster: drafted mon gets Gigantamax Factor iff its species has a G-Max form")
{
    struct Pokemon mon;
    // Minimal sets; CreateFacilityMon reads species/moves, the rest may stay 0.
    const struct TrainerMon gmaxSet = { .species = SPECIES_CHARIZARD, .moves = { MOVE_TACKLE } };
    const struct TrainerMon plainSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE } };

    CreateFacilityMon(&gmaxSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT(GetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR));

    CreateFacilityMon(&plainSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT(!GetMonData(&mon, MON_DATA_GIGANTAMAX_FACTOR));
}

// FORK: CreateFacilityMon grants the maximum Dynamax Level at draft time so a
// Dynamaxed mon gets the full HP boost. An explicit per-entry .dynamaxLevel
// still overrides the default.
TEST("Frontier extended roster: drafted mon gets the maximum Dynamax Level by default")
{
    struct Pokemon mon;
    const struct TrainerMon defaultSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE } };
    const struct TrainerMon explicitSet = { .species = SPECIES_SALAMENCE, .moves = { MOVE_TACKLE }, .dynamaxLevel = 5 };

    CreateFacilityMon(&defaultSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DYNAMAX_LEVEL), MAX_DYNAMAX_LEVEL);

    CreateFacilityMon(&explicitSet, 50, MAX_PER_STAT_IVS, 0, 0, &mon);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_DYNAMAX_LEVEL), 5);
}
