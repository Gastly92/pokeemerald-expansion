#include "global.h"
#include "fork/willpower.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_stat_change.h"
#include "battle_util.h"

// The Willpower ability -- see include/fork/willpower.h and fork-docs/NEW_ABILITIES.md.

bool32 TryActivateWillpower(enum BattlerId battler, enum Ability ability)
{
    // Stamina's conditions, word for word: a move that damaged the target (not its Substitute),
    // not the holder hitting itself, and room left to raise the stat.
    if (gBattlerAttacker == gBattlerTarget
     || !IsBattlerTurnDamaged(gBattlerTarget, EXCLUDING_SUBSTITUTES)
     || !IsBattlerAlive(battler)
     || !CompareStat(battler, STAT_SPDEF, MAX_STAT_STAGE, CMP_LESS_THAN, ability))
        return FALSE;

    gEffectBattler = gBattlerAbility = battler;
    SetStatChange(battler, STAT_SPDEF, 1);
    // An innate Willpower shows itself in the pop-up, not the chosen ability (the Stamina precedent).
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    BattleScriptCall(BattleScript_AbilityStatChange);
    return TRUE;
}
