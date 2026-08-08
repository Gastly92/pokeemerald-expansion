#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_controllers.h" // BattlerHasAi, GetBattlerTrainer
#include "battle_gimmick.h"
#include "config/feature.h"
#include "constants/config_changes.h"
#include "test_runner.h"
#include "fork/battle_ai_gimmick.h"

static const enum Gimmick sAiGimmickPreference[] = { AI_GIMMICK_PREFERENCE_ORDER };

// FORK: pick this battler's gimmick from its full candidate set instead of leaving it on
// the enum-order default AssignUsableGimmicks seeded. Only meaningful with item-free
// gimmicks, where a mon routinely has more than one candidate. Ultra Burst is left alone
// when it is available, because it is not a gimmick in its own right - it is the form
// change that unlocks Necrozma's Z-Move, so overriding it would cost the mon both.
static void AI_SelectBestGimmick(enum BattlerId battler)
{
    u32 candidates = GetGimmickCandidates(battler);

    if (candidates == 0 || CountGimmickCandidates(battler) < 2)
        return;

    #if TESTING
    // Battle tests fix the intended gimmick through the DSL; honour it over the preference.
    if (TestRunner_Battle_GetChosenGimmick(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]) != GIMMICK_NONE)
        return;
    #endif

    if (candidates & (1u << GIMMICK_ULTRA_BURST))
        return;

    for (u32 i = 0; i < ARRAY_COUNT(sAiGimmickPreference); i++)
    {
        enum Gimmick gimmick = sAiGimmickPreference[i];

        if ((candidates & (1u << gimmick)) && !HasTrainerUsedGimmick(battler, gimmick))
        {
            gBattleStruct->gimmick.usableGimmick[battler] = gimmick;
            return;
        }
    }
}

// FORK: run the pick for every AI battler once per turn, immediately after
// AssignUsableGimmicks and *before* SetAiLogicDataForTurn. The AI caches a full turn of
// simulated damage with its selected gimmick forced on, so choosing later (e.g. during
// BattleAI_ChooseMoveIndex) would score every move against the gimmick it is no longer
// going to use.
void AI_SelectGimmicksForTurn(void)
{
    if (!GetConfig(FEATURE_FREE_GIMMICKS))
        return;

    for (enum BattlerId battler = 0; battler < gBattlersCount; ++battler)
    {
        if (IsBattlerAlive(battler) && BattlerHasAi(battler))
            AI_SelectBestGimmick(battler);
    }
}
