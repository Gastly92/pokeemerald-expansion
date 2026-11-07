#include "global.h"
#include "fork/psybubble.h"
#include "fork/innate_abilities.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_ai_record.h"
#include "battle_ai_util.h"
#include "battle_util.h"
#include "move.h"

// The Psybubble ability -- see include/fork/psybubble.h and fork-docs/NEW_ABILITIES.md.

bool32 HasPsybubbleProtection(enum BattlerId battler, enum Ability chosenAbility)
{
    if (chosenAbility != ABILITY_PSYBUBBLE && !IsInnateActive(battler, ABILITY_PSYBUBBLE))
        return FALSE;
    return IsBattlersEntryTurn(battler);
}

void ActivatePsybubble(enum BattlerId battler, enum Ability chosenAbility, bool32 updateFlags)
{
    RecordAbilityBattle(battler, ABILITY_PSYBUBBLE);
    gLastUsedAbility = ABILITY_PSYBUBBLE;
    // An innate shows itself in the pop-up, not the chosen ability (the innate Sturdy precedent).
    if (chosenAbility != ABILITY_PSYBUBBLE)
        gBattleScripting.abilityPopupOverwrite = ABILITY_PSYBUBBLE;
    gBattleStruct->moveResultFlags[battler] |= MOVE_RESULT_STURDIED;
    // One save per entry: close the window now rather than at move end, so the next strike of a
    // multi-hit move connects.
    if (updateFlags)
        gBattleStruct->battlerState[battler].facedFoeAction = TRUE;
}

bool32 AI_PsybubbleEndures(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Ability atkAbility = gAiLogicData->abilities[battlerAtk];

    // Only the first strike is endured, so anything that hits twice gets through.
    if (IsMultiHitMove(move) || GetMoveStrikeCount(move) > 1 || BattlerHasAbility(battlerAtk, ABILITY_PARENTAL_BOND))
        return FALSE;
    if (DoesBattlerIgnoreAbilityChecks(battlerAtk, atkAbility, move))
        return FALSE;
    return HasPsybubbleProtection(battlerDef, gAiLogicData->abilities[battlerDef]);
}

bool32 SwitchinHasPsybubble(enum BattlerId battler, enum Ability chosenAbility)
{
    return chosenAbility == ABILITY_PSYBUBBLE
        || (GetConfig(FEATURE_INNATE_ABILITIES) && SpeciesHasInnate(gBattleMons[battler].species, ABILITY_PSYBUBBLE));
}
