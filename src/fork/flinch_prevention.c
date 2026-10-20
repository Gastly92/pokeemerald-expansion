#include "global.h"
#include "battle.h"
#include "battle_gimmick.h"
#include "battle_hold_effects.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "fork/flinch_prevention.h"
#include "fork/innate_abilities.h"

// Announcing blocked flinches -- see include/fork/flinch_prevention.h.

// The same conditions under which the flinch handler would set the flinched volatile, plus a
// Substitute and Sheer Force, which the blocked-effect branch runs ahead of. A flinch that
// could not have landed anyway is not worth a message.
static bool32 WouldFlinchLand(struct BattleCalcValues *cv, struct SetEffect *se)
{
    enum BattlerId battler = se->effectBattler;

    return IsBattlerAlive(battler)
        && !HasBattlerActedThisTurn(battler)
        && !gBattleMons[battler].volatiles.flinched
        && GetActiveGimmick(battler) != GIMMICK_DYNAMAX
        && !DoesSubstituteBlockMove(cv->battlerAtk, battler, gCurrentMove)
        && (se->primary || se->bypassSheerForce || !IsSheerForceAffected(cv->move, cv->abilities[cv->battlerAtk]));
}

// An innate shows itself in the pop-up rather than the chosen ability (the Intimidate precedent).
static void PrepareAbilityMessage(enum BattlerId battler, enum Ability ability)
{
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gEffectBattler = gBattlerAbility = battler;
    PREPARE_ABILITY_BUFFER(gBattleTextBuff1, ability);
}

void TryAnnounceBlockedFlinch(struct BattleCalcValues *cv, struct SetEffect *se, bool32 affectsUser)
{
    enum BattlerId battler = se->effectBattler;
    const u8 *script;

    if (!GetConfig(FEATURE_ANNOUNCE_FLINCH_PREVENTION)
     || se->moveEffect != MOVE_EFFECT_FLINCH
     || se->primary
     || affectsUser
     || !WouldFlinchLand(cv, se))
        return;

    // Same order as IsMoveEffectBlockedByTarget, so the blocker named is the one that blocked it.
    if (cv->abilities[battler] == ABILITY_SHIELD_DUST)
    {
        PrepareAbilityMessage(battler, ABILITY_SHIELD_DUST);
        script = BattleScript_FlinchPreventedByAbility;
    }
    else if (GetBattlerHoldEffect(battler) == HOLD_EFFECT_COVERT_CLOAK)
    {
        gEffectBattler = battler;
        PREPARE_ITEM_BUFFER(gBattleTextBuff1, gBattleMons[battler].item);
        script = BattleScript_FlinchPreventedByItem;
    }
    else if (IsInnateActive(battler, ABILITY_SHIELD_DUST))
    {
        PrepareAbilityMessage(battler, ABILITY_SHIELD_DUST);
        script = BattleScript_FlinchPreventedByAbility;
    }
    else
    {
        return;
    }

    // The blocked branch then hands over to se->script, which now plays the message and returns.
    BattleScriptPush(se->script);
    se->script = script;
}

void TryAnnounceInnerFocusFlinch(struct BattleCalcValues *cv, struct SetEffect *se)
{
    if (!GetConfig(FEATURE_ANNOUNCE_FLINCH_PREVENTION) || !WouldFlinchLand(cv, se))
        return;

    PrepareAbilityMessage(se->effectBattler, ABILITY_INNER_FOCUS);
    BattleScriptPush(gBattlescriptCurrInstr);
    gBattlescriptCurrInstr = BattleScript_FlinchPreventedByAbility;
}
