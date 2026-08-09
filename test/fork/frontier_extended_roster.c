#include "global.h"
#include "test/test.h"
#include "data.h"
#include "battle_frontier.h"
#include "config_changes.h"
#include "fork/frontier_extended_mons.h"
#include "fork/innate_abilities.h"
#include "fork/species_ability_overrides.h"
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

    // The fork ability layer (innates + the species ability-override table) is gated by
    // FEATURE_INNATE_ABILITIES, which TestInitConfigData force-disables by default. The real
    // frontier (CreateFacilityMon) runs with it on, so opt in to resolve overridden slots.
    SetConfig(CONFIG_FEATURE_INNATE_ABILITIES, TRUE);

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

// FORK: a frontier set's .ability is the mon's single CHOSEN (observable) ability,
// layered ON TOP of that species' always-on innates (fork/innate_abilities.c). When the
// chosen ability is ITSELF one of the species' innates, the chosen slot is REDUNDANT: the
// mon would already have that ability from its innate, so the one observable pick is wasted
// (the set intends a second, visible trait and gets a duplicate instead).
//
// Every such set has been given a real, non-innate chosen ability -- repointed to a free real
// slot, or handed a stable pick via a fork-owned override in src/fork/species_ability_overrides.c
// (an ability marked :x: in INNATE_ABILITIES_PROGRESS.md -- never itself an innate -- or an
// already-implemented innate the species does not carry). Species that once had NO repurposable
// slot (every real ability innate AND every slot pinned by a battle test -- Snorlax, Kangaskhan,
// Pinsir, Sableye, Clefable, Slowbro, Camerupt, ...) are now convertible too: the override table
// is gated by FEATURE_INNATE_ABILITIES and thus invisible to upstream tests (see
// GetSpeciesAbilityOverride), so a row can repurpose a slot an upstream test pins without
// perturbing it -- only flag-on fork tests observe the override, and each species kept a free slot
// clear of those. So the whole roster now carries a real chosen ability; ABILITY_NONE is banned
// (see the next test).
//
// This test asserts the invariant holds for the WHOLE roster with no exceptions: a chosen
// ability is either ABILITY_NONE or NOT one of the species' innates. It fails loudly if a new
// set ever reintroduces redundancy.
TEST("Frontier extended roster: no set's chosen ability duplicates a species innate")
{
    u32 i;
    u32 redundant = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];
        enum Ability ability = set->ability;

        // ABILITY_NONE lets the Factory pick; a chosen ability that is NOT an innate is the
        // goal (a real, observable second trait), so both are fine.
        if (ability == ABILITY_NONE)
            continue;
        if (!SpeciesHasInnate(set->species, ability))
            continue;

        redundant++;
        Test_MgbaPrintf("roster[%d] %S: chosen %S duplicates a species innate -- give it a non-innate chosen ability (fork override / repoint) or ABILITY_NONE",
                        i,
                        gSpeciesInfo[set->species].speciesName,
                        gAbilitiesInfo[ability].name);
    }

    EXPECT_EQ(redundant, 0);
}

// FORK: the same redundancy invariant as the test above, but checked at its SOURCE -- the
// fork-owned override table (src/fork/species_ability_overrides.c) rather than the roster that
// consumes it. An override row exists to hand a species a real, OBSERVABLE second trait alongside
// its always-on innates, so a row granting an ability the species already has innately defeats its
// own purpose: the chosen slot resolves to a duplicate and the mon shows one trait instead of two.
//
// The roster test above only sees a bad row once some set actually selects that slot, so a
// redundant row could sit in the table indefinitely -- and would then be inherited by the next set
// authored on that species (exactly the trap a line review walks into: propose an override against
// an innate, then build a set on it). This test sweeps EVERY species x slot through
// GetSpeciesAbilityOverride and fails on any row that duplicates an innate, independent of roster
// coverage. Species with no override row return ABILITY_NONE and are skipped.
TEST("Frontier extended roster: no species ability override duplicates a species innate")
{
    u32 species, slot;
    u32 redundant = 0;
    u32 rowsSeen = 0;

    // GetSpeciesAbilityOverride returns ABILITY_NONE for every species while the flag is off
    // (TestInitConfigData force-disables it), which would make this test vacuously pass.
    SetConfig(CONFIG_FEATURE_INNATE_ABILITIES, TRUE);

    for (species = 1; species < NUM_SPECIES; species++)
    {
        for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            enum Ability ability = GetSpeciesAbilityOverride(species, slot);

            if (ability == ABILITY_NONE)
                continue;

            rowsSeen++;
            if (!SpeciesHasInnate(species, ability))
                continue;

            redundant++;
            Test_MgbaPrintf("override %S slot %d: %S is already an innate of this species -- the chosen slot would resolve to a duplicate; pick a non-innate ability for the row or drop it",
                            gSpeciesInfo[species].speciesName,
                            slot,
                            gAbilitiesInfo[ability].name);
        }
    }

    // Guard against a vacuous pass: if the flag gating on GetSpeciesAbilityOverride ever changes
    // so that it stops resolving here, the sweep above would silently check nothing.
    EXPECT_GT(rowsSeen, 0);
    EXPECT_EQ(redundant, 0);
}

// FORK: ABILITY_NONE on a set means "let the Factory pick the ability at draft", which yields a
// non-deterministic, unlabeled ability -- and for the fork's all-innate species it can only ever
// land on a redundant innate. Every set must instead name a real, deliberate chosen ability. This
// was previously impossible for species whose every real slot was pinned by an upstream battle test
// (the override that would free a slot also rewrote it inside that test); gating the override table
// behind FEATURE_INNATE_ABILITIES fixed that (the override is invisible to flag-off upstream tests),
// so the escape hatch is gone. This test bans ABILITY_NONE outright and fails loudly if a new set
// reintroduces it -- give the set a fork override (src/fork/species_ability_overrides.c) instead.
TEST("Frontier extended roster: no set uses ABILITY_NONE (every set names a real chosen ability)")
{
    u32 i;
    u32 noneCount = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        const struct TrainerMon *set = &gFrontierExtendedMons[i];

        if (set->ability != ABILITY_NONE)
            continue;

        noneCount++;
        Test_MgbaPrintf("roster[%d] %S: uses ABILITY_NONE -- give it a real chosen ability via a fork override (species_ability_overrides.c)",
                        i,
                        gSpeciesInfo[set->species].speciesName);
    }

    EXPECT_EQ(noneCount, 0);
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
