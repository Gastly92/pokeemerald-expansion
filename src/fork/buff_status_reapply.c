#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_ai_util.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_util.h"
#include "move.h"
#include "pokemon.h"
#include "config/buff.h"
#include "fork/innate_abilities.h" // BattlerHasAbility
#include "fork/buff_status_reapply.h"

// BUFF_REBURN / BUFF_REPOISON / BUFF_REPARALYZE. Re-using a status move on a foe that already has that
// status turns the would-be failure into a bonus, the way BUFF_LEECH_SEED's re-drain
// does for Leech Seed. See fork-docs/FORK.md.

enum StatusReapply
{
    STATUS_REAPPLY_NONE,
    STATUS_REAPPLY_BURN,          // burn move, burned foe: 1/BUFF_REBURN_DENOMINATOR tick
    STATUS_REAPPLY_POISON,        // poison move, poisoned or badly poisoned foe: 1/8 tick
    STATUS_REAPPLY_TOXIC_ADVANCE, // Toxic, badly poisoned foe: advance the counter and tick
    STATUS_REAPPLY_TOXIC_UPGRADE, // Toxic, poisoned foe: upgrade to bad poison and tick
    STATUS_REAPPLY_PARALYSIS,     // paralysis move, paralyzed foe: drain PP from every move
};

// TRUE when the status in `hiddenBits` is the only thing stopping `effect` from
// landing on `battlerDef`, i.e. every other block (type, ability, terrain,
// Safeguard...) would let a fresh status through. Reuses CanSetNonVolatileStatus by
// hiding the existing status for the duration of a side-effect-free CHECK_TRIGGER
// query, so the re-apply respects exactly the blocks a first application does.
static bool32 WouldStatusLandIfCured(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum MoveEffect effect, u32 hiddenBits)
{
    u32 status1 = gBattleMons[battlerDef].status1;
    bool32 lands;

    gBattleMons[battlerDef].status1 &= ~hiddenBits;
    lands = CanSetNonVolatileStatus(battlerAtk, battlerDef, abilityAtk, abilityDef, effect, CHECK_TRIGGER);
    gBattleMons[battlerDef].status1 = status1;
    return lands;
}

static bool32 HasPpToDrain(enum BattlerId battler)
{
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        if (gBattleMons[battler].moves[i] != MOVE_NONE && gBattleMons[battler].pp[i] > 0)
            return TRUE;
    }
    return FALSE;
}

// TRUE when draining would empty at least one of `battler`'s moves.
static bool32 DrainEmptiesAMove(enum BattlerId battler)
{
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        if (gBattleMons[battler].moves[i] != MOVE_NONE
         && gBattleMons[battler].pp[i] > 0
         && gBattleMons[battler].pp[i] <= BUFF_REPARALYZE_PP)
            return TRUE;
    }
    return FALSE;
}

// Drains BUFF_REPARALYZE_PP from each of `battler`'s moves (never below 0) and syncs
// the party copy in one request. Mimicked moves and a Transformed battler's moves are
// temporary, so - as with Spite - their drain is not written back to the party.
static void DrainParalysisPp(enum BattlerId battler)
{
    struct Pokemon *mon = GetBattlerMon(battler);
    u8 data[MAX_MON_MOVES + 1];

    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        u32 drain = min(gBattleMons[battler].pp[i], BUFF_REPARALYZE_PP);

        gBattleMons[battler].pp[i] -= drain;
        if (drain > 0 && gBattleMons[battler].pp[i] == 0 && gBattleMons[battler].moves[i] == gLastMoves[battler])
            CancelMultiTurnMoves(battler); // an emptied locked-in move stops, as with Spite

        if (gBattleMons[battler].volatiles.mimickedMoves & (1u << i))
            data[i] = GetMonData(mon, MON_DATA_PP1 + i);
        else
            data[i] = gBattleMons[battler].pp[i];
    }
    data[MAX_MON_MOVES] = gBattleMons[battler].ppBonuses;

    if (!gBattleMons[battler].volatiles.transformed)
    {
        BtlController_EmitSetMonData(battler, B_COMM_TO_CONTROLLER, REQUEST_PP_DATA_BATTLE, 0, sizeof(data), data);
        MarkBattlerForControllerExec(battler);
    }
}

static enum StatusReapply GetStatusReapply(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum Move move)
{
    enum MoveEffect effect = GetMoveNonVolatileStatus(move);
    u32 status1 = gBattleMons[battlerDef].status1;

    // The move has to reach the target to stoke its status, so a Substitute blocks it.
    if (!IsBattlerPresent(battlerDef) || DoesSubstituteBlockMove(battlerAtk, battlerDef, move))
        return STATUS_REAPPLY_NONE;
    // The burn and poison ticks are indirect damage, which Magic Guard ignores.
    if ((effect == MOVE_EFFECT_BURN || effect == MOVE_EFFECT_POISON || effect == MOVE_EFFECT_TOXIC)
     && (abilityDef == ABILITY_MAGIC_GUARD || BattlerHasAbility(battlerDef, ABILITY_MAGIC_GUARD))) // innate-aware Magic Guard
        return STATUS_REAPPLY_NONE;

    switch (effect)
    {
    case MOVE_EFFECT_BURN:
        if (GetConfig(BUFF_REBURN)
         && (status1 & STATUS1_BURN)
         && WouldStatusLandIfCured(battlerAtk, battlerDef, abilityAtk, abilityDef, effect, STATUS1_BURN))
            return STATUS_REAPPLY_BURN;
        break;
    case MOVE_EFFECT_POISON:
    case MOVE_EFFECT_TOXIC:
        // Poison Heal would turn the tick into a heal, so it just fails as in vanilla.
        if (!GetConfig(BUFF_REPOISON)
         || !(status1 & STATUS1_PSN_ANY)
         || abilityDef == ABILITY_POISON_HEAL
         || BattlerHasAbility(battlerDef, ABILITY_POISON_HEAL) // innate-aware Poison Heal
         || !WouldStatusLandIfCured(battlerAtk, battlerDef, abilityAtk, abilityDef, effect, STATUS1_PSN_ANY | STATUS1_TOXIC_COUNTER))
            break;
        if (effect == MOVE_EFFECT_POISON)
            return STATUS_REAPPLY_POISON;
        if (status1 & STATUS1_TOXIC_POISON)
            return STATUS_REAPPLY_TOXIC_ADVANCE;
        return STATUS_REAPPLY_TOXIC_UPGRADE;
    case MOVE_EFFECT_PARALYSIS:
        if (GetConfig(BUFF_REPARALYZE)
         && (status1 & STATUS1_PARALYSIS)
         && HasPpToDrain(battlerDef)
         && WouldStatusLandIfCured(battlerAtk, battlerDef, abilityAtk, abilityDef, effect, STATUS1_PARALYSIS))
            return STATUS_REAPPLY_PARALYSIS;
        break;
    default:
        break;
    }
    return STATUS_REAPPLY_NONE;
}

// The Toxic counter `battler` will have after one more tick (capped like the end-turn tick).
static u32 GetNextToxicCounter(enum BattlerId battler)
{
    u32 counter = (gBattleMons[battler].status1 & STATUS1_TOXIC_COUNTER) >> 8;

    if (!(gBattleMons[battler].status1 & STATUS1_TOXIC_POISON))
        return 1; // a fresh upgrade ticks as Toxic's first turn
    if (counter < 15)
        counter++;
    return counter;
}

static s32 GetStatusReapplyDamage(enum BattlerId battler, enum StatusReapply kind)
{
    s32 damage = 0;

    switch (kind)
    {
    case STATUS_REAPPLY_BURN:
        damage = GetNonDynamaxMaxHP(battler) / BUFF_REBURN_DENOMINATOR;
        if (BattlerHasAbility(battler, ABILITY_HEATPROOF)) // innate-aware, halves like the end-turn tick
            damage /= 2;
        break;
    case STATUS_REAPPLY_POISON:
        damage = GetNonDynamaxMaxHP(battler) / 8;
        break;
    case STATUS_REAPPLY_TOXIC_ADVANCE:
    case STATUS_REAPPLY_TOXIC_UPGRADE:
        damage = (GetNonDynamaxMaxHP(battler) / 16) * GetNextToxicCounter(battler);
        break;
    default:
        return 0;
    }
    if (damage == 0)
        damage = 1;
    return damage;
}

s32 GetReburnDamage(enum BattlerId battler)
{
    return GetStatusReapplyDamage(battler, STATUS_REAPPLY_BURN);
}

bool32 TryStatusMoveReapply(void)
{
    enum StatusReapply kind = GetStatusReapply(gBattlerAttacker, gBattlerTarget, GetBattlerAbility(gBattlerAttacker), GetBattlerAbility(gBattlerTarget), gCurrentMove);

    if (kind == STATUS_REAPPLY_NONE)
        return FALSE;

    if (kind != STATUS_REAPPLY_PARALYSIS)
        SetPassiveDamageAmount(gBattlerTarget, GetStatusReapplyDamage(gBattlerTarget, kind));
    switch (kind)
    {
    case STATUS_REAPPLY_BURN:
        gBattlescriptCurrInstr = BattleScript_ReburnTick;
        break;
    case STATUS_REAPPLY_POISON:
        gBattlescriptCurrInstr = BattleScript_RepoisonTick;
        break;
    case STATUS_REAPPLY_TOXIC_ADVANCE:
    {
        u32 counter = GetNextToxicCounter(gBattlerTarget); // read before clearing the old count
        gBattleMons[gBattlerTarget].status1 &= ~STATUS1_TOXIC_COUNTER;
        gBattleMons[gBattlerTarget].status1 |= STATUS1_TOXIC_TURN(counter);
        gBattlescriptCurrInstr = BattleScript_RepoisonTick;
        break;
    }
    case STATUS_REAPPLY_TOXIC_UPGRADE:
        // Drop the regular poison and pre-load Toxic's first-turn counter: the script's
        // setnonvolatilestatus then ORs in STATUS1_TOXIC_POISON through the normal path
        // (message, status icon, Poison Puppeteer), and the tick consumes turn 1.
        gBattleMons[gBattlerTarget].status1 &= ~(STATUS1_PSN_ANY | STATUS1_TOXIC_COUNTER);
        gBattleMons[gBattlerTarget].status1 |= STATUS1_TOXIC_TURN(1);
        gBattlescriptCurrInstr = BattleScript_ToxicUpgradeTick;
        break;
    case STATUS_REAPPLY_PARALYSIS:
        DrainParalysisPp(gBattlerTarget);
        PREPARE_BYTE_NUMBER_BUFFER(gBattleTextBuff1, 1, BUFF_REPARALYZE_PP)
        gBattlescriptCurrInstr = BattleScript_ReparalyzeDrain;
        break;
    default:
        break;
    }
    return TRUE;
}

static void IncreaseStatusReapplyScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum StatusReapply kind, s32 *score)
{
    if (kind == STATUS_REAPPLY_NONE)
        return;

    if (kind == STATUS_REAPPLY_PARALYSIS)
    {
        // A PP drain only really bites when it empties a move.
        if (DrainEmptiesAMove(battlerDef))
            ADJUST_SCORE_PTR(DECENT_EFFECT);
        else
            ADJUST_SCORE_PTR(WEAK_EFFECT);
    }
    else if (gBattleMons[battlerDef].hp <= GetStatusReapplyDamage(battlerDef, kind))
        ADJUST_SCORE_PTR(BEST_EFFECT);
    else if (kind == STATUS_REAPPLY_TOXIC_UPGRADE)
        ADJUST_SCORE_PTR(DECENT_EFFECT); // a lasting upgrade, not just chip
    else
        ADJUST_SCORE_PTR(WEAK_EFFECT);
}

bool32 AI_CanReburn(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move)
{
    return GetMoveNonVolatileStatus(move) == MOVE_EFFECT_BURN
        && GetStatusReapply(battlerAtk, battlerDef, ABILITY_NONE, defAbility, move) != STATUS_REAPPLY_NONE;
}

// AI: a re-burn is a small guaranteed chip, worth more when the tick finishes the foe.
void IncreaseReburnScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score)
{
    if (AI_CanReburn(battlerAtk, battlerDef, gAiLogicData->abilities[battlerDef], move))
        IncreaseStatusReapplyScore(battlerAtk, battlerDef, STATUS_REAPPLY_BURN, score);
}

bool32 AI_CanRepoison(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move)
{
    enum MoveEffect effect = GetMoveNonVolatileStatus(move);

    // The attacker's ability matters here: Corrosion poisons Poison and Steel types.
    return (effect == MOVE_EFFECT_POISON || effect == MOVE_EFFECT_TOXIC)
        && GetStatusReapply(battlerAtk, battlerDef, gAiLogicData->abilities[battlerAtk], defAbility, move) != STATUS_REAPPLY_NONE;
}

// AI: re-poisoning chips (or KOs); Toxic on a regularly poisoned foe is a real upgrade.
void IncreaseRepoisonScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score)
{
    if (AI_CanRepoison(battlerAtk, battlerDef, gAiLogicData->abilities[battlerDef], move))
        IncreaseStatusReapplyScore(battlerAtk, battlerDef,
                                   GetStatusReapply(battlerAtk, battlerDef, gAiLogicData->abilities[battlerAtk], gAiLogicData->abilities[battlerDef], move),
                                   score);
}

bool32 AI_CanReparalyze(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability defAbility, enum Move move)
{
    return GetMoveNonVolatileStatus(move) == MOVE_EFFECT_PARALYSIS
        && GetStatusReapply(battlerAtk, battlerDef, ABILITY_NONE, defAbility, move) == STATUS_REAPPLY_PARALYSIS;
}

// AI: a PP drain on a paralyzed foe, worth more when it empties one of its moves.
void IncreaseReparalyzeScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 *score)
{
    if (AI_CanReparalyze(battlerAtk, battlerDef, gAiLogicData->abilities[battlerDef], move))
        IncreaseStatusReapplyScore(battlerAtk, battlerDef, STATUS_REAPPLY_PARALYSIS, score);
}
