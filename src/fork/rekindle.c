#include "global.h"
#include "fork/rekindle.h"
#include "battle.h"
#include "battle_dynamax.h"
#include "battle_script_commands.h"
#include "battle_scripts.h"
#include "battle_util.h"

// The Rekindle ability -- see include/fork/rekindle.h and fork-docs/NEW_ABILITIES.md.

bool32 TryActivateRekindle(enum BattlerId battler, enum Ability ability, u32 numFainted)
{
    s32 healAmount;

    // NoAliveMonsForEitherParty: the KO ended the battle, so there is nothing left to heal for.
    if (numFainted == 0 || NoAliveMonsForEitherParty())
        return FALSE;
    if (!IsBattlerAlive(battler) || IsBattlerAtMaxHp(battler) || gBattleMons[battler].volatiles.healBlockTimer)
        return FALSE;

    // Based on the non-Dynamax max HP, like Rain Dish, and capped at what is missing so the
    // healthbar never overshoots.
    healAmount = (GetNonDynamaxMaxHP(battler) / REKINDLE_HEAL_FRACTION) * numFainted;
    if (healAmount > gBattleMons[battler].maxHP - gBattleMons[battler].hp)
        healAmount = gBattleMons[battler].maxHP - gBattleMons[battler].hp;

    gLastUsedAbility = ability;
    // An innate Rekindle shows itself in the pop-up, not the chosen ability (the Moxie precedent).
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gBattlerAbility = battler;
    SetHealAmount(battler, healAmount);
    // The same pop-up + HP bar as Rain Dish / Ice Body, with no message of its own.
    BattleScriptCall(BattleScript_AbilityHpHeal);
    return TRUE;
}
