#include "global.h"
#include "test/test.h"
#include "event_data.h"
#include "fork/frontier_ai.h"
#include "constants/battle_frontier.h"

// Guards Script_IsNextFactoryBattleMilestone (src/fork/frontier_ai.c), which the
// Factory's pre-battle attendant asks before announcing the next match. It must flag
// exactly the upcoming match that GenerateOpponentMons drafts as the set milestone
// (Match No. 10, 20, ...), or the "formidable opponent" warning lands on the wrong fight.

#if B_FRONTIER_ENDLESS
static bool32 IsMilestoneAtStreak(u32 winStreak)
{
    u32 battleMode = VarGet(VAR_FRONTIER_BATTLE_MODE);
    u32 lvlMode = gSaveBlock2Ptr->frontier.lvlMode;
    u16 saved = gSaveBlock2Ptr->frontier.factoryWinStreaks[battleMode][lvlMode];
    bool32 result;

    gSaveBlock2Ptr->frontier.factoryWinStreaks[battleMode][lvlMode] = winStreak;
    Script_IsNextFactoryBattleMilestone();
    result = gSpecialVar_Result;
    gSaveBlock2Ptr->frontier.factoryWinStreaks[battleMode][lvlMode] = saved;
    return result;
}

TEST("Factory tough-match warning: only the 10th, 20th, ... match is flagged")
{
    // The win streak counts matches already won, so the upcoming match is streak + 1.
    EXPECT(IsMilestoneAtStreak(FRONTIER_STAGES_PER_CHALLENGE - 1));      // Match No. 10
    EXPECT(IsMilestoneAtStreak(2 * FRONTIER_STAGES_PER_CHALLENGE - 1));  // Match No. 20
    EXPECT(IsMilestoneAtStreak(10 * FRONTIER_STAGES_PER_CHALLENGE - 1)); // Match No. 100

    EXPECT(!IsMilestoneAtStreak(0));                                    // Match No. 1
    EXPECT(!IsMilestoneAtStreak(FRONTIER_STAGES_PER_CHALLENGE - 2));    // Match No. 9
    EXPECT(!IsMilestoneAtStreak(FRONTIER_STAGES_PER_CHALLENGE));        // Match No. 11
}
#endif
