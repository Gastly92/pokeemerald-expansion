#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_controllers.h"
#include "battle_factory.h"
#include "constants/abilities.h"
#include "constants/hold_effects.h"
#include "constants/battle_ai.h"

void RecordLastUsedMoveBy(enum BattlerId battlerId, enum Move move)
{
    u8 *index = &gBattleHistory->moveHistoryIndex[battlerId];

    if (++(*index) >= AI_MOVE_HISTORY_COUNT)
        *index = 0;
    gBattleHistory->moveHistory[battlerId][*index] = move;
}

void RecordKnownMove(enum BattlerId battler, enum Move move)
{
    s32 moveIndex;

    for (moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
    {
        if (gBattleMons[battler].moves[moveIndex] == move)
            break;
    }

    if (moveIndex < MAX_MON_MOVES && gBattleHistory->usedMoves[battler][moveIndex] == MOVE_NONE)
    {
        gBattleHistory->usedMoves[battler][moveIndex] = move;
        gAiPartyData->mons[GetBattlerSide(battler)][gBattlerPartyIndexes[battler]].moves[moveIndex] = move;
    }
}

void RecordAllMoves(enum BattlerId battler)
{
    memcpy(gAiPartyData->mons[GetBattlerSide(battler)][gBattlerPartyIndexes[battler]].moves, gBattleMons[battler].moves, MAX_MON_MOVES * sizeof(u16));
}

// FORK: B_FRONTIER_BATTLE_INFO marks an ability/item as "revealed" only when the record
// happens during real battle execution. RecordAbility/ItemEffectBattle are also called all
// through the AI's *speculative* move evaluation (gAiLogicData->aiCalcInProgress) — e.g. the AI
// guessing the foe's ability, and GetBattleMovePriority's Prankster check, which the engine runs
// for every status move at battle start and after every switch-in. The player never witnesses
// those, so counting them as "seen" leaked the foe's ability (most visibly Prankster, the only
// priority ability recorded via the turn-order path) before it was ever used. Genuine reveals
// (ability pop-ups, a fired hold effect, a move resolving) all happen with aiCalcInProgress clear.
static bool32 BattleInfoCanRevealNow(void)
{
    return (gAiLogicData == NULL || !gAiLogicData->aiCalcInProgress);
}

void RecordAbilityBattle(enum BattlerId battlerId, enum Ability abilityId)
{
    gBattleHistory->abilities[battlerId] = abilityId;
    gAiPartyData->mons[GetBattlerSide(battlerId)][gBattlerPartyIndexes[battlerId]].ability = abilityId;
    if (BattleInfoCanRevealNow())
        gBattleStruct->infoAbilityRevealed[GetBattlerSide(battlerId)] |= 1u << gBattlerPartyIndexes[battlerId];
}

void RecordItemEffectBattle(enum BattlerId battlerId, enum HoldEffect itemEffect)
{
    gBattleHistory->itemEffects[battlerId] = itemEffect;
    gAiPartyData->mons[GetBattlerSide(battlerId)][gBattlerPartyIndexes[battlerId]].heldEffect = itemEffect;
    if (BattleInfoCanRevealNow())
        gBattleStruct->infoItemRevealed[GetBattlerSide(battlerId)] |= 1u << gBattlerPartyIndexes[battlerId];
}

void ClearBattlerAbilityHistory(enum BattlerId battlerId)
{
    gBattleHistory->abilities[battlerId] = ABILITY_NONE;
}

void ClearBattlerMoveHistory(enum BattlerId battlerId)
{
    memset(gBattleHistory->usedMoves[battlerId], 0, sizeof(gBattleHistory->usedMoves[battlerId]));
    memset(gBattleHistory->moveHistory[battlerId], 0, sizeof(gBattleHistory->moveHistory[battlerId]));
    gBattleHistory->moveHistoryIndex[battlerId] = 0;
}

void ClearBattlerItemEffectHistory(enum BattlerId battlerId)
{
    gBattleHistory->itemEffects[battlerId] = HOLD_EFFECT_NONE;
}
