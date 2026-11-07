#include "global.h"
#include "fork/mind_over_matter.h"
#include "fork/innate_abilities.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_ai_record.h"
#include "battle_ai_util.h"
#include "battle_util.h"
#include "move.h"

// The Mind Over Matter ability -- see include/fork/mind_over_matter.h and fork-docs/NEW_ABILITIES.md.

bool32 HasMindOverMatterProtection(enum BattlerId battler, enum Ability chosenAbility)
{
    if (chosenAbility != ABILITY_MIND_OVER_MATTER && !IsInnateActive(battler, ABILITY_MIND_OVER_MATTER))
        return FALSE;
    return IsBattlersEntryTurn(battler);
}

void ActivateMindOverMatter(enum BattlerId battler, enum Ability chosenAbility, bool32 updateFlags)
{
    RecordAbilityBattle(battler, ABILITY_MIND_OVER_MATTER);
    gLastUsedAbility = ABILITY_MIND_OVER_MATTER;
    // An innate shows itself in the pop-up, not the chosen ability (the innate Sturdy precedent).
    if (chosenAbility != ABILITY_MIND_OVER_MATTER)
        gBattleScripting.abilityPopupOverwrite = ABILITY_MIND_OVER_MATTER;
    gBattleStruct->moveResultFlags[battler] |= MOVE_RESULT_STURDIED;
    // One save per entry: close the window now rather than at move end, so the next strike of a
    // multi-hit move connects.
    if (updateFlags)
        gBattleStruct->battlerState[battler].facedFoeAction = TRUE;
}

bool32 AI_MindOverMatterEndures(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Ability atkAbility = gAiLogicData->abilities[battlerAtk];

    // Only the first strike is endured, so anything that hits twice gets through.
    if (IsMultiHitMove(move) || GetMoveStrikeCount(move) > 1 || BattlerHasAbility(battlerAtk, ABILITY_PARENTAL_BOND))
        return FALSE;
    if (DoesBattlerIgnoreAbilityChecks(battlerAtk, atkAbility, move))
        return FALSE;
    return HasMindOverMatterProtection(battlerDef, gAiLogicData->abilities[battlerDef]);
}

bool32 SwitchinHasMindOverMatter(enum BattlerId battler, enum Ability chosenAbility)
{
    return chosenAbility == ABILITY_MIND_OVER_MATTER
        || (GetConfig(FEATURE_INNATE_ABILITIES) && SpeciesHasInnate(gBattleMons[battler].species, ABILITY_MIND_OVER_MATTER));
}
