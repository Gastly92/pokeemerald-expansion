#include "global.h"
#include "test/test.h"
#include "fork/frontier_ai.h"
#include "fork/battle_tower_trainers.h"
#include "fork/battle_ai_species_overrides.h" // AI_FLAG_SMART_SPECIES_LOGIC
#include "fork/battle_ai_zmove.h"             // AI_FLAG_SMART_Z_MOVE
#include "constants/battle_ai.h"
#include "constants/battle_frontier_trainers.h"
#include "constants/trainers.h"

// FORK: guards the Frontier's two-tier AI difficulty (src/fork/frontier_ai.c).
// The top preset used to be the baseline for every facility battle, which left a
// boss fight playing no better than the routine opponent before it. It is now
// reserved for the Frontier Brain and the Battle Tower's gym-leader bosses;
// everyone else runs one tier down. These tests pin the role split and the
// "regular is never stronger than boss" relationship between the two presets.

TEST("Frontier AI: the Frontier Brain gets the boss tier")
{
    EXPECT(IsFrontierBossTrainer(TRAINER_FRONTIER_BRAIN));
    EXPECT_EQ(GetFrontierAiFlags(TRAINER_FRONTIER_BRAIN), B_FRONTIER_HARD_AI_FLAGS);
}

TEST("Frontier AI: every Battle Tower gym-leader boss gets the boss tier")
{
    for (u32 i = 0; i < GetTowerBossCount(); i++)
    {
        u16 trainerId = TOWER_BOSS_TRAINER_FIRST + i;

        EXPECT(IsFrontierBossTrainer(trainerId));
        EXPECT_EQ(GetFrontierAiFlags(trainerId), B_FRONTIER_HARD_AI_FLAGS);
    }
}

TEST("Frontier AI: a regular facility opponent gets the tier below the boss tier")
{
    // Facility trainers occupy ids below FRONTIER_TRAINERS_COUNT, well clear of
    // both the Brain and the fork's boss id range.
    EXPECT(!IsFrontierBossTrainer(0));
    EXPECT(!IsFrontierBossTrainer(FRONTIER_TRAINERS_COUNT - 1));
    EXPECT_EQ(GetFrontierAiFlags(0), B_FRONTIER_REGULAR_AI_FLAGS);
    EXPECT_NE(B_FRONTIER_REGULAR_AI_FLAGS, B_FRONTIER_HARD_AI_FLAGS);
}

TEST("Frontier AI: the id just past the boss range is not a boss")
{
    EXPECT(!IsFrontierBossTrainer(TOWER_BOSS_TRAINER_FIRST + GetTowerBossCount()));
}

TEST("Frontier AI: the regular tier drops the boss tier's information advantage")
{
    // The knowledge levers, in descending order of how much they hand the AI.
    EXPECT(B_FRONTIER_HARD_AI_FLAGS & AI_FLAG_OMNISCIENT);
    EXPECT(!(B_FRONTIER_REGULAR_AI_FLAGS & AI_FLAG_OMNISCIENT));

    // ...replaced by upstream's restricted knowledge set, so the regular tier
    // still plays around STAB and common status moves without reading the team.
    EXPECT_EQ(B_FRONTIER_REGULAR_AI_FLAGS & AI_FLAG_ASSUMPTIONS, AI_FLAG_ASSUMPTIONS);

    // Anti-stall and the fork's per-species misplay patches are boss-only too.
    EXPECT(!(B_FRONTIER_REGULAR_AI_FLAGS & AI_FLAG_PP_STALL_PREVENTION));
    EXPECT(!(B_FRONTIER_REGULAR_AI_FLAGS & AI_FLAG_SMART_SPECIES_LOGIC));
}

TEST("Frontier AI: the regular tier still plays its team competently")
{
    // A tier below the boss is not a tier of throwing the battle: regular
    // opponents keep basic move choice, smart switching, and the gimmick
    // discipline that stops them burning Tera / their one Z-Move on turn one.
    u64 competence = AI_FLAG_BASIC_TRAINER | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES
                   | AI_FLAG_SMART_TERA | AI_FLAG_SMART_Z_MOVE;

    EXPECT_EQ(B_FRONTIER_REGULAR_AI_FLAGS & competence, competence);

    // ...and every one of those is shared with the boss tier, so the two presets
    // differ only in the boss's favour.
    EXPECT_EQ(B_FRONTIER_HARD_AI_FLAGS & competence, competence);
}
