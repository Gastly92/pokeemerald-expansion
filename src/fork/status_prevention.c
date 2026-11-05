#include "global.h"
#include "battle.h"
#include "battle_hold_effects.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "constants/battle_string_ids.h"
#include "fork/innate_abilities.h"
#include "fork/status_prevention.h"

// Announcing blocked status side effects -- see include/fork/status_prevention.h.

static EWRAM_DATA enum BattlerId sBlockerBattler = 0;
static EWRAM_DATA enum Ability sBlockerAbility = ABILITY_NONE;

// Indexed by StatusPreventedMessage, read by printfromtable in BattleScript_StatusPrevented*.
enum StatusPreventedMessage
{
    STATUS_PREVENTED_PARALYSIS,
    STATUS_PREVENTED_POISON,
    STATUS_PREVENTED_BURN,
    STATUS_PREVENTED_SLEEP,
    STATUS_PREVENTED_FREEZE,
    STATUS_PREVENTED_FROSTBITE,
};

const u16 gStatusPreventedStringIds[] =
{
    [STATUS_PREVENTED_PARALYSIS] = STRINGID_PARALYSISPREVENTED,
    [STATUS_PREVENTED_POISON]    = STRINGID_POISONPREVENTED,
    [STATUS_PREVENTED_BURN]      = STRINGID_BURNPREVENTED,
    [STATUS_PREVENTED_SLEEP]     = STRINGID_SLEEPPREVENTED,
    [STATUS_PREVENTED_FREEZE]    = STRINGID_FREEZEPREVENTED,
    [STATUS_PREVENTED_FROSTBITE] = STRINGID_FROSTBITEPREVENTED,
};

// Returns FALSE for anything that is not a single non-volatile status (Tri Attack's and Dire
// Claw's random pick is announced on the inner call, once the status has been chosen).
static bool32 GetStatusPreventedMessage(enum MoveEffect moveEffect, u32 *message)
{
    switch (moveEffect)
    {
    case MOVE_EFFECT_PARALYSIS:
        *message = STATUS_PREVENTED_PARALYSIS;
        return TRUE;
    case MOVE_EFFECT_POISON:
    case MOVE_EFFECT_TOXIC:
        *message = STATUS_PREVENTED_POISON;
        return TRUE;
    case MOVE_EFFECT_BURN:
        *message = STATUS_PREVENTED_BURN;
        return TRUE;
    case MOVE_EFFECT_SLEEP:
        *message = STATUS_PREVENTED_SLEEP;
        return TRUE;
    case MOVE_EFFECT_FREEZE:
        *message = STATUS_PREVENTED_FREEZE;
        return TRUE;
    case MOVE_EFFECT_FROSTBITE:
        *message = STATUS_PREVENTED_FROSTBITE;
        return TRUE;
    default:
        return FALSE;
    }
}

// The conditions the blocked-effect branch in SetMoveEffect runs ahead of, plus the target's
// own status checks: a status that could not have landed anyway is not worth a message.
static bool32 WouldStatusLand(struct BattleCalcValues *cv, struct SetEffect *se)
{
    enum BattlerId battler = se->effectBattler;

    return IsBattlerAlive(battler)
        && !DoesSubstituteBlockMove(cv->battlerAtk, battler, gCurrentMove)
        && (se->bypassSheerForce || !IsSheerForceAffected(cv->move, cv->abilities[cv->battlerAtk]))
        && !IsSafeguardProtected(cv->battlerAtk, battler, cv->abilities[cv->battlerAtk])
        && CanSetNonVolatileStatus(cv->battlerAtk, battler, cv->abilities[cv->battlerAtk],
                                   cv->abilities[battler], se->moveEffect, CHECK_TRIGGER);
}

// An innate shows itself in the pop-up rather than the chosen ability (the Intimidate precedent).
static void PrepareAbilityMessage(enum BattlerId battler, enum Ability ability)
{
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gEffectBattler = gBattlerAbility = battler;
    PREPARE_ABILITY_BUFFER(gBattleTextBuff1, ability);
}

void NoteNonVolatileStatusBlocker(enum BattlerId battler, enum Ability ability)
{
    sBlockerBattler = battler;
    sBlockerAbility = ability;
}

void TryAnnounceBlockedStatusEffect(struct BattleCalcValues *cv, struct SetEffect *se, bool32 affectsUser)
{
    enum BattlerId battler = se->effectBattler;
    const u8 *script;
    u32 message;

    if (!GetConfig(FEATURE_ANNOUNCE_STATUS_PREVENTION)
     || se->primary
     || affectsUser
     || !GetStatusPreventedMessage(se->moveEffect, &message)
     || !WouldStatusLand(cv, se))
        return;

    // Same order as IsMoveEffectBlockedByTarget, so the blocker named is the one that blocked it.
    if (cv->abilities[battler] == ABILITY_SHIELD_DUST)
    {
        PrepareAbilityMessage(battler, ABILITY_SHIELD_DUST);
        script = BattleScript_StatusPreventedByAbility;
    }
    else if (GetBattlerHoldEffect(battler) == HOLD_EFFECT_COVERT_CLOAK)
    {
        gEffectBattler = battler;
        PREPARE_ITEM_BUFFER(gBattleTextBuff1, gBattleMons[battler].item);
        script = BattleScript_StatusPreventedByItem;
    }
    else if (IsInnateActive(battler, ABILITY_SHIELD_DUST))
    {
        PrepareAbilityMessage(battler, ABILITY_SHIELD_DUST);
        script = BattleScript_StatusPreventedByAbility;
    }
    else
    {
        return;
    }

    gBattleCommunication[MULTISTRING_CHOOSER] = message;
    // The blocked branch then hands over to se->script, which now plays the message and returns.
    BattleScriptPush(se->script);
    se->script = script;
}

// CanSetNonVolatileStatus checks the type- and status-specific abilities (Limber, Immunity, ...)
// before an existing status or Misty Terrain, so it names Limber even for a target that could
// not have been paralyzed anyway. Those two are ruled out here instead.
static bool32 WasOnlyAbilityBlocking(struct BattleCalcValues *cv, enum BattlerId battler)
{
    return !(gBattleMons[battler].status1 & STATUS1_ANY)
        && !IsMistyTerrainAffected(battler, cv->abilities[battler], GetBattlerHoldEffect(battler), gFieldTimers.terrain);
}

void TryAnnounceStatusImmunity(struct BattleCalcValues *cv, struct SetEffect *se)
{
    u32 message;

    // sBlockerAbility was just written by the CanSetNonVolatileStatus call that refused. Only an
    // ability counts: an existing status, a type immunity, Safeguard or a terrain stay quiet.
    if (!GetConfig(FEATURE_ANNOUNCE_STATUS_PREVENTION)
     || se->primary
     || cv->battlerAtk == se->effectBattler
     || sBlockerAbility == ABILITY_NONE
     || !GetStatusPreventedMessage(se->moveEffect, &message)
     || !WasOnlyAbilityBlocking(cv, se->effectBattler))
        return;

    // The blocker can be the target's ally (Flower Veil, Sweet Veil, Pastel Veil).
    PrepareAbilityMessage(sBlockerBattler, sBlockerAbility);
    gBattleCommunication[MULTISTRING_CHOOSER] = message;
    BattleScriptPush(gBattlescriptCurrInstr);
    gBattlescriptCurrInstr = BattleScript_StatusPreventedByAbility;
}
