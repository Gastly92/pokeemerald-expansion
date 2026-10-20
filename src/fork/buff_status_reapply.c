#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_dynamax.h"
#include "battle_script_commands.h"
#include "battle_util.h"
#include "move.h"
#include "config/buff.h"
#include "fork/innate_abilities.h" // BattlerHasAbility
#include "fork/buff_status_reapply.h"

// BUFF_REBURN. Re-using a status move on a foe that already has that status turns
// the would-be failure into a bonus, the way BUFF_LEECH_SEED's re-drain does for
// Leech Seed. See fork-docs/FORK.md.

// TRUE when `statusFlag` is the only thing stopping `effect` from landing on
// `battlerDef`, i.e. every other block (type, ability, terrain, Safeguard...) would
// let a fresh status through. Reuses CanSetNonVolatileStatus by hiding the existing
// status for the duration of a side-effect-free CHECK_TRIGGER query, so the
// re-apply respects exactly the blocks a first application does.
static bool32 WouldStatusLandIfCured(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum MoveEffect effect, u32 statusFlag)
{
    u32 status1 = gBattleMons[battlerDef].status1;
    bool32 lands;

    if (!(status1 & statusFlag))
        return FALSE;

    gBattleMons[battlerDef].status1 &= ~statusFlag;
    lands = CanSetNonVolatileStatus(battlerAtk, battlerDef, abilityAtk, abilityDef, effect, CHECK_TRIGGER);
    gBattleMons[battlerDef].status1 = status1;
    return lands;
}

static bool32 CanReburn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum Move move)
{
    return GetConfig(BUFF_REBURN)
        && GetMoveNonVolatileStatus(move) == MOVE_EFFECT_BURN
        && IsBattlerPresent(battlerDef)
        && abilityDef != ABILITY_MAGIC_GUARD
        && !BattlerHasAbility(battlerDef, ABILITY_MAGIC_GUARD) // innate-aware Magic Guard
        && !DoesSubstituteBlockMove(battlerAtk, battlerDef, move)
        && WouldStatusLandIfCured(battlerAtk, battlerDef, abilityAtk, abilityDef, MOVE_EFFECT_BURN, STATUS1_BURN);
}

s32 GetReburnDamage(enum BattlerId battler)
{
    s32 damage = GetNonDynamaxMaxHP(battler) / BUFF_REBURN_DENOMINATOR;

    if (BattlerHasAbility(battler, ABILITY_HEATPROOF)) // innate-aware, halves like the end-turn tick
        damage /= 2;
    if (damage == 0)
        damage = 1;
    return damage;
}

bool32 TryStatusMoveReapply(void)
{
    if (CanReburn(gBattlerAttacker, gBattlerTarget, GetBattlerAbility(gBattlerAttacker), GetBattlerAbility(gBattlerTarget), gCurrentMove))
    {
        SetPassiveDamageAmount(gBattlerTarget, GetReburnDamage(gBattlerTarget));
        gBattlescriptCurrInstr = BattleScript_ReburnTick;
        return TRUE;
    }
    return FALSE;
}

bool32 AI_CanReburn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move)
{
    return CanReburn(battlerAtk, battlerDef, ABILITY_NONE, defAbility, move);
}

// AI: a re-burn is a small guaranteed chip, worth more when the tick finishes the foe.
void IncreaseReburnScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score)
{
    if (!AI_CanReburn(battlerAtk, battlerDef, gAiLogicData->abilities[battlerDef], move))
        return;

    if (gBattleMons[battlerDef].hp <= GetReburnDamage(battlerDef))
        ADJUST_SCORE_PTR(BEST_EFFECT);
    else
        ADJUST_SCORE_PTR(WEAK_EFFECT);
}
