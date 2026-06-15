#include "global.h"
#include "test/test.h"
#include "data.h"
#include "fork/frontier_extended_mons.h"
#include "fork/innate_abilities.h"
#include "constants/abilities.h"

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
