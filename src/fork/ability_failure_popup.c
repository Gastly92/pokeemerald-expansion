#include "global.h"
#include "battle.h"
#include "battle_script_commands.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "fork/ability_failure_popup.h"
#include "fork/innate_abilities.h"
#include "fork/status_prevention.h"

// Pop-ups for ability-caused move failures -- see include/fork/ability_failure_popup.h.
//
// Each hook runs after upstream has already picked the fail script, so with the feature off nothing
// changes. With it on, the fail script is pushed and BattleScript_AbilityPopUp runs first; its
// closing `return` then lands on the fail script, which prints "But it failed!" as before.

bool32 TryPrepareAbilityFailurePopUp(enum BattlerId battler, enum Ability ability)
{
    if (!GetConfig(FEATURE_ANNOUNCE_ABILITY_FAILURES))
        return FALSE;

    gBattlerAbility = battler;
    // CreateAbilityPopUp reads the chosen slot, so name the innate when that is what acted.
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    return TRUE;
}

// gBattlescriptCurrInstr holds the fail script: play the pop-up first, then return to it.
static void PopUpBeforeCurrentScript(enum BattlerId battler, enum Ability ability)
{
    if (!TryPrepareAbilityFailurePopUp(battler, ability))
        return;
    BattleScriptPush(gBattlescriptCurrInstr);
    gBattlescriptCurrInstr = BattleScript_AbilityPopUp;
}

bool32 TryFailWithGuardDogPopUp(enum BattlerId battler, const u8 *failScript)
{
    if (GetBattlerAbility(battler) != ABILITY_GUARD_DOG && !IsInnateActive(battler, ABILITY_GUARD_DOG))
        return FALSE;
    if (!TryPrepareAbilityFailurePopUp(battler, ABILITY_GUARD_DOG))
        return FALSE;

    BattleScriptPush(failScript);
    gBattlescriptCurrInstr = BattleScript_AbilityPopUp;
    return TRUE;
}

static enum MoveEffect GetPsychoShiftEffect(u32 status1)
{
    if (status1 & (STATUS1_POISON | STATUS1_TOXIC_POISON))
        return MOVE_EFFECT_TOXIC; // BS_TryPsychoShift checks poison as Toxic too
    if (status1 & STATUS1_BURN)
        return MOVE_EFFECT_BURN;
    if (status1 & STATUS1_PARALYSIS)
        return MOVE_EFFECT_PARALYSIS;
    if (status1 & STATUS1_SLEEP)
        return MOVE_EFFECT_SLEEP;
    if (status1 & STATUS1_FROSTBITE)
        return MOVE_EFFECT_FREEZE_OR_FROSTBITE;
    return MOVE_EFFECT_NONE;
}

void TryAnnouncePsychoShiftBlocker(void)
{
    enum MoveEffect effect = GetPsychoShiftEffect(gBattleMons[gBattlerAttacker].status1);
    enum BattlerId blocker;
    enum Ability ability;

    if (effect == MOVE_EFFECT_NONE)
        return;

    // Redo the check BS_TryPsychoShift just failed, so IsNonVolatileStatusBlocked names the
    // blocker. Only an ability counts: an existing status, a type or a terrain stays plain.
    NoteNonVolatileStatusBlocker(gBattlerTarget, ABILITY_NONE);
    if (CanSetNonVolatileStatus(gBattlerAttacker, gBattlerTarget, GetBattlerAbility(gBattlerAttacker),
                                GetBattlerAbility(gBattlerTarget), effect, CHECK_TRIGGER))
        return;
    ability = GetNonVolatileStatusBlocker(&blocker);
    if (ability != ABILITY_NONE)
        PopUpBeforeCurrentScript(blocker, ability);
}

void TryAnnounceRestBlocker(enum BattlerId battler)
{
    enum Ability ability = GetBattlerAbility(battler);
    u32 sideBattler;

    // Same order as BS_JumpIfAbilityPreventsRest.
    if (GetConfig(B_LEAF_GUARD_PREVENTS_REST) >= GEN_5 && IsLeafGuardProtected(battler, ability))
        PopUpBeforeCurrentScript(battler, ABILITY_LEAF_GUARD);
    else if (IsShieldsDownProtected(battler, ability))
        PopUpBeforeCurrentScript(battler, ABILITY_SHIELDS_DOWN);
    else if ((sideBattler = IsAbilityOnSide(battler, ABILITY_SWEET_VEIL))
          || (sideBattler = IsInnateOnSide(battler, ABILITY_SWEET_VEIL)))
        PopUpBeforeCurrentScript(sideBattler - 1, ABILITY_SWEET_VEIL);
}
