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

static bool32 IsGimmickAvailable(enum BattlerId battler, enum Gimmick gimmick)
{
    return (GetGimmickCandidates(battler) & (1u << gimmick))
        && !HasTrainerUsedGimmick(battler, gimmick);
}

// Whether any of this battler's moves would KO a foe outright with `gimmick` active.
// usableGimmick is what AI_CalcDamage reads to decide which gimmick to simulate, so the
// candidate is swapped in around the calc and restored afterwards.
static bool32 GimmickSecuresKO(enum BattlerId battler, enum Gimmick gimmick)
{
    enum Gimmick saved = gBattleStruct->gimmick.usableGimmick[battler];
    enum Move *moves = GetMovesArray(battler);
    bool32 securesKO = FALSE;

    gBattleStruct->gimmick.usableGimmick[battler] = gimmick;

    for (enum BattlerId target = 0; target < gBattlersCount && !securesKO; target++)
    {
        if (!IsBattlerAlive(target) || GetBattlerSide(target) == GetBattlerSide(battler))
            continue;

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            uq4_12_t effectiveness;
            struct SimulatedDamage dmg;

            if (moves[moveIndex] == MOVE_NONE || GetMovePower(moves[moveIndex]) == 0)
                continue;

            dmg = AI_CalcDamageSaveBattlers(moves[moveIndex], battler, target, &effectiveness, USE_GIMMICK, NO_GIMMICK);
            if (dmg.minimum >= gBattleMons[target].hp)
            {
                securesKO = TRUE;
                break;
            }
        }
    }

    gBattleStruct->gimmick.usableGimmick[battler] = saved;
    return securesKO;
}

// FORK: pick this battler's gimmick from its full candidate set instead of leaving it on
// the enum-order default AssignUsableGimmicks seeded. Only meaningful with item-free
// gimmicks, where a mon routinely has more than one candidate.
//
// Two passes, because the fork allows each gimmick type only once per trainer per battle
// (see fork-docs/FORK.md), which makes every one of them a resource worth placing well:
//   1. Any candidate that turns this turn into a KO wins - tempo is worth more than
//      holding the resource, and a KO now is the one benefit that cannot be deferred.
//   2. Otherwise fall back to AI_GIMMICK_PREFERENCE_ORDER, which puts the persistent
//      form changes ahead of the single-use Z-Move, so the Z-Move stays in reserve.
// Ultra Burst is left alone when available, because it is not a gimmick in its own right -
// it is the form change that unlocks Necrozma's Z-Move, so overriding it costs the mon both.
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
        if (IsGimmickAvailable(battler, sAiGimmickPreference[i])
         && GimmickSecuresKO(battler, sAiGimmickPreference[i]))
        {
            gBattleStruct->gimmick.usableGimmick[battler] = sAiGimmickPreference[i];
            return;
        }
    }

    for (u32 i = 0; i < ARRAY_COUNT(sAiGimmickPreference); i++)
    {
        if (IsGimmickAvailable(battler, sAiGimmickPreference[i]))
        {
            gBattleStruct->gimmick.usableGimmick[battler] = sAiGimmickPreference[i];
            return;
        }
    }
}

// FORK: run the pick for every AI battler once per turn, from inside SetAiLogicDataForTurn
// between the battler-data pass and the move-damage pass. It needs the former (the KO check
// runs real damage calcs, which read gAiLogicData's abilities and hold effects) and must
// precede the latter, which caches a whole turn of simulated damage with the selected
// gimmick forced on - choosing afterwards would score every move against a gimmick the AI
// is no longer going to use.
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
