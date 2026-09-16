#include "global.h"
#include "test/test.h"
#include "fork/frontier_extended_mons.h"
#include "fork/species_tiers.h"
#include "pokemon.h"
#include "constants/species.h"

// FORK: guards the fork-owned species_tiers.c classification table. The table
// is split into one array per tier (see species_tiers.c), and GetSpeciesTier
// checks them in Mythical > Legendary > Pseudo priority order, returning the
// first match. That priority means a species accidentally listed twice
// (within one array, or copy-pasted into a second one) would not error — it
// would just silently resolve to whichever list is checked first. This test
// fails loudly on any such duplicate instead.
TEST("Species tiers: no species is listed more than once")
{
    u16 dupSpecies = SPECIES_NONE;
    bool32 hasDup = SpeciesTierListsOverlap(&dupSpecies);

    if (hasDup)
        Test_MgbaPrintf("species %d is listed more than once across the tier tables", dupSpecies);

    EXPECT(!hasDup);
}

// FORK: this is the documented reason the table is keyed by exact species id
// rather than by Pokedex number (see include/fork/species_tiers.h) — a forme
// can outrank or underrank its base species. Shaymin-Sky is the example used
// in that comment, so it doubles as a regression test for the rationale: if a
// future change collapsed formes to their base species, this would catch it.
TEST("Species tiers: a forme resolves independently of its base species")
{
    EXPECT_EQ(GetSpeciesTier(SPECIES_SHAYMIN), TIER_PSEUDO);
    EXPECT_EQ(GetSpeciesTier(SPECIES_SHAYMIN_SKY), TIER_LEGENDARY);
}

TEST("Species tiers: an unlisted species defaults to TIER_NORMAL")
{
    EXPECT_EQ(GetSpeciesTier(SPECIES_BULBASAUR), TIER_NORMAL);
}

// FORK: how *restricted* a tier is, which is NOT the enum's numeric order
// (TIER_PSEUDO is the highest enum value but the weakest restriction). Only the
// draft rules care about this ordering, and only in this test, so it lives here
// rather than in species_tiers.h. See TierRejectsCandidate in
// src/fork/frontier_draft.c for what each tier actually gates.
static u32 TierRestrictionRank(enum SpeciesTier tier)
{
    switch (tier)
    {
    case TIER_MYTHICAL:  return 3;  // one per Frontier Brain battle
    case TIER_LEGENDARY: return 2;  // one per boss battle
    case TIER_PSEUDO:    return 1;  // at most one per team
    case TIER_NORMAL:    return 0;  // unrestricted
    }
    return 0;
}

// FORK: regression test for the Arceus draft leak. The tier table is keyed by
// EXACT species id so a forme can be classified on its own merits, which means a
// forme left out of the table silently defaults to TIER_NORMAL — i.e. drafted as
// an ordinary mon with no quota at all. That is what happened to Arceus: only
// SPECIES_ARCEUS (the Normal plate) was listed as TIER_MYTHICAL, so all 17 plate
// formes in the extended roster were draftable into any party slot, and both
// teams in a regular Battle Factory/Tower battle could lead with one.
//
// Deliberate divergences always go the other way (Articuno-Galar outranks
// Articuno, Calyrex-Ice outranks Calyrex, Shaymin-Sky outranks Shaymin), so the
// invariant that catches the leak without an exception list is: a drafted forme
// is never LESS restricted than its base forme. A forme that should legitimately
// be more restricted than its base still passes.
TEST("Species tiers: no drafted forme is less restricted than its base forme")
{
    u32 i, offenders = 0;

    for (i = 0; i < gFrontierExtendedMonsCount; i++)
    {
        u32 species = gFrontierExtendedMons[i].species;
        u32 baseSpecies = GET_BASE_SPECIES_ID(species);
        enum SpeciesTier tier, baseTier;

        if (species == baseSpecies)
            continue;

        tier = GetSpeciesTier(species);
        baseTier = GetSpeciesTier(baseSpecies);
        if (TierRestrictionRank(tier) < TierRestrictionRank(baseTier))
        {
            Test_MgbaPrintf("roster entry %d: species %d is tier %d, but its base forme %d is tier %d",
                            i, species, tier, baseSpecies, baseTier);
            offenders++;
        }
    }

    EXPECT_EQ(offenders, 0);
}
