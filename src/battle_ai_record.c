#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_controllers.h"
#include "battle_factory.h"
#include "constants/abilities.h"
#include "constants/hold_effects.h"
#include "constants/battle_ai.h"
#include "config_changes.h" // FORK: GetConfig(FEATURE_INNATE_ABILITIES)
#include "fork/innate_abilities.h" // FORK: SpeciesHasInnate

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
    {
        u32 side = GetBattlerSide(battlerId);
        u32 partyIndex = gBattlerPartyIndexes[battlerId];
        // FORK: B_FRONTIER_BATTLE_INFO reveals what the player actually SAW. When an ability
        // pop-up is showing an overwritten ability (an innate Levitate/Sturdy forces the
        // pop-up via gBattleScripting.abilityPopupOverwrite), the pop-up script's
        // `recordability` still hands us the mon's *chosen* ability — but the player saw the
        // overwrite, not the chosen one. Mirror the pop-up by revealing the overwrite when one
        // is set, so an innate reveal never leaks the chosen ability. The AI knowledge model
        // (gAiPartyData / gBattleHistory above) always uses the real abilityId, unchanged.
        enum Ability witnessed = (gBattleScripting.abilityPopupOverwrite != ABILITY_NONE)
                               ? gBattleScripting.abilityPopupOverwrite
                               : abilityId;
        // FORK: FEATURE_INNATE_ABILITIES — only the *chosen* ability is reveal-gated. Innates are a
        // static property of the species (the viewer shows them unconditionally, like the type
        // line), so a witnessed ability that is one of this species' innates — and not its chosen
        // ability — carries no information about the still-hidden chosen ability: don't mark the
        // chosen ability revealed (the viewer keeps showing "?", e.g. "? (+Levitate)"). The chosen
        // ability is gBattleMons[battler].ability (innates live in a separate species table, never
        // in the .ability slot), so a witnessed ability differing from it that the species declares
        // as an innate is an innate pop-up (e.g. an innate Levitate/Sturdy forcing the pop-up to its
        // name via abilityPopupOverwrite) and must not leak the chosen ability.
        bool32 witnessedIsInnate = GetConfig(FEATURE_INNATE_ABILITIES)
                                && witnessed != gBattleMons[battlerId].ability
                                && SpeciesHasInnate(gBattleMons[battlerId].species, witnessed);
        if (!witnessedIsInnate)
        {
            gBattleStruct->infoAbilityRevealed[side] |= 1u << partyIndex;
            // FORK: snapshot the witnessed ability for the B_FRONTIER_BATTLE_INFO viewer.
            // gAiPartyData->mons[].ability above is later clobbered by the AI's speculative
            // switch/move evaluation (a benched mon simulated in the active slot records its
            // own ability onto this slot), so the viewer must read this gated snapshot — taken
            // here, at the moment of a genuine reveal — rather than the AI's live knowledge model.
            gBattleStruct->infoRevealedAbility[side][partyIndex] = witnessed;
        }
    }
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
