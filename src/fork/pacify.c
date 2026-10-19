#include "global.h"
#include "fork/pacify.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_stat_change.h"
#include "battle_util.h"

// The Pacify ability -- see include/fork/pacify.h and fork-docs/NEW_ABILITIES.md.

bool32 TryActivatePacify(enum BattlerId battler, enum Ability ability)
{
    bool32 anyFoe = FALSE;

    // Intimidate's own loop, with Sp. Atk in place of Attack and no intimidateActivated flag, so the
    // reactions keyed on Intimidate by name stay out of it.
    for (enum BattlerId i = 0; i < gBattlersCount; i++)
    {
        if (IsBattlerAlly(battler, i) || !IsBattlerAlive(i))
            continue;
        SetStatChange(i, STAT_SPATK, -1);
        anyFoe = TRUE;
    }
    if (!anyFoe)
        return FALSE;

    // An innate Pacify shows itself in the pop-up, not the chosen ability (the Intimidate precedent).
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gEffectBattler = gBattlerAbility = battler;
    // Upstream's recommended script for an Intimidate-like drop: pop-up, then the queued stat changes,
    // which run every foe through the ordinary stat-drop checks.
    BattleScriptCall(BattleScript_AbilityStatChange);
    return TRUE;
}
