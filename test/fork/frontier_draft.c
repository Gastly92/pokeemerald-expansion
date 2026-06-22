#include "global.h"
#include "test/test.h"
#include "data.h"
#include "fork/frontier_draft.h"
#include "constants/abilities.h"

// FORK: guards IllusionMonRejectsSlot (src/fork/frontier_draft.c). Illusion
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
