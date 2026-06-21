#include "global.h"
#include "test/test.h"
#include "fork/species_tiers.h"
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
