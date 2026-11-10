#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_util.h"
#include "config_changes.h"
#include "move.h"
#include "pokemon.h"
#include "fork/deterministic_moves.h"
#include "fork/innate_abilities.h"
#include "fork/multi_hit_pp.h"
#include "constants/battle_move_effects.h"

// FEATURE_MULTI_HIT_PP_PER_HIT: every strike of a multi-hit move costs PP.
//
// The first strike is paid by CancelerPPDeduction exactly as any move is, and that call
// records what one use cost (MultiHitPP_RecordUse). Every later strike is paid for the moment
// MoveEndMultihitMove decides to throw it (MultiHitPP_TryChargeNextHit), so a move only ever
// pays for the strikes it actually throws: a KO on strike 3 stops the loop before strike 4 is
// charged, and a first strike that Protect blocks never reaches the loop at all. A strike the
// move cannot pay for in full is not thrown; that is how a 5-hit move with 2 PP left hits
// twice.
//
// Why at the loop and not by trimming gMultiHitCounter up front: EFFECT_TRIPLE_KICK reads
// its escalating power off gMultiHitCounter (CalcMoveBasePowerAfterModifiers), and Dragon
// Darts reads it to tell its second strike apart, so the counter has to start at its real
// value.

static struct {
    bool8 active;
    u8 battler;
    u8 movePosition;
    enum Move move;
    s16 cost;
    s16 refund;
} sMultiHitPP;

bool32 IsPerHitPPMove(enum Move move)
{
    return IsMultiHitMove(move)
        || GetMoveStrikeCount(move) > 1
        || GetMoveEffect(move) == EFFECT_BEAT_UP;
}

void MultiHitPP_Reset(void)
{
    sMultiHitPP.active = FALSE;
}

void MultiHitPP_RecordUse(enum BattlerId battlerAtk, enum Move move, u32 movePosition, s32 cost, s32 refundPerHit)
{
    sMultiHitPP.active = FALSE;
    if (!GetConfig(FEATURE_MULTI_HIT_PP_PER_HIT) || !IsPerHitPPMove(move) || movePosition >= MAX_MON_MOVES)
        return;

    sMultiHitPP.active = TRUE;
    sMultiHitPP.battler = battlerAtk;
    sMultiHitPP.movePosition = movePosition;
    sMultiHitPP.move = move;
    sMultiHitPP.cost = cost;
    sMultiHitPP.refund = refundPerHit;
}

// One strike's payment from `pp`. Returns FALSE (and leaves pp alone) when it cannot be paid
// in full. The refund is clamped to max PP, as CancelerPPDeduction clamps it.
static bool32 PayForStrike(u32 *pp, u32 maxPP, s32 cost, s32 refund)
{
    if ((s32)*pp < cost)
        return FALSE;
    *pp -= cost;
    if (refund > 0)
        *pp = min(*pp + refund, maxPP);
    return TRUE;
}

bool32 MultiHitPP_TryChargeNextHit(enum BattlerId battlerAtk, enum Move move)
{
    u32 pos = sMultiHitPP.movePosition;
    u32 pp, maxPP;

    if (!sMultiHitPP.active || sMultiHitPP.battler != battlerAtk || sMultiHitPP.move != move)
        return TRUE; // not a charged use (called move, Parental Bond, feature off): strike for free

    pp = gBattleMons[battlerAtk].pp[pos];
    maxPP = CalculatePPWithBonus(move, gBattleMons[battlerAtk].ppBonuses, pos);
    if (!PayForStrike(&pp, maxPP, sMultiHitPP.cost, sMultiHitPP.refund))
        return FALSE;

    gBattleMons[battlerAtk].pp[pos] = pp;
    if (MOVE_IS_PERMANENT(battlerAtk, pos))
    {
        BtlController_EmitSetMonData(battlerAtk, B_COMM_TO_CONTROLLER, REQUEST_PPMOVE1_BATTLE + pos, 0,
                                     sizeof(gBattleMons[battlerAtk].pp[pos]), &gBattleMons[battlerAtk].pp[pos]);
        MarkBattlerForControllerExec(battlerAtk);
    }
    return TRUE;
}

// The strike count CancelerMultihitMoves would set, read without rolling. Keep in sync with it.
static u32 GetNominalStrikeCount(enum BattlerId battlerAtk, enum Move move)
{
    enum BattleMoveEffects effect = GetMoveEffect(move);
    enum HoldEffect holdEffect = GetBattlerHoldEffect(battlerAtk);
    bool32 skillLink = BattlerHasAbility(battlerAtk, ABILITY_SKILL_LINK); // chosen or innate
    bool32 deterministic = GetConfig(DETERMINISTIC_MOVE_RESULTS);

    if (IsMultiHitMove(move))
    {
        if (effect == EFFECT_SPECIES_POWER_OVERRIDE
         && gBattleMons[battlerAtk].species == GetMoveSpeciesPowerOverride_Species(move))
            return GetMoveSpeciesPowerOverride_NumOfHits(move);
        if (skillLink)
            return deterministic ? DETERMINISTIC_MULTI_HIT_MAX_COUNT : 5;
        if (deterministic)
            return holdEffect == HOLD_EFFECT_LOADED_DICE ? DETERMINISTIC_MULTI_HIT_MAX_COUNT : DETERMINISTIC_MULTI_HIT_COUNT;
        return 5; // a random 2-5 roll: project the most it can hit, and so cost
    }

    if (GetMoveStrikeCount(move) > 1)
    {
        if (effect == EFFECT_POPULATION_BOMB && deterministic)
            return (skillLink || holdEffect == HOLD_EFFECT_LOADED_DICE)
                 ? DETERMINISTIC_POPULATION_BOMB_LOADED_DICE_COUNT
                 : DETERMINISTIC_POPULATION_BOMB_COUNT;
        return GetMoveStrikeCount(move);
    }

    if (effect == EFFECT_BEAT_UP)
    {
        struct Pokemon *party = GetBattlerParty(battlerAtk);
        u32 i, count = 0;

        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&party[i], MON_DATA_SPECIES) != SPECIES_NONE
             && GetMonData(&party[i], MON_DATA_HP)
             && !GetMonData(&party[i], MON_DATA_IS_EGG)
             && !GetMonData(&party[i], MON_DATA_STATUS))
                count++;
        }
        return count ? count : 1;
    }

    return 1;
}

u32 GetMultiHitPPCappedHits(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 hits)
{
    s32 cost, refund, oneTimeRefund;
    u32 pos, pp, maxPP, affordable;

    if (hits <= 1 || !GetConfig(FEATURE_MULTI_HIT_PP_PER_HIT) || !IsPerHitPPMove(move))
        return hits;

    for (pos = 0; pos < MAX_MON_MOVES; pos++)
    {
        if (gBattleMons[battlerAtk].moves[pos] == move)
            break;
    }
    if (pos == MAX_MON_MOVES)
        return hits; // not in the moveset (a called or hypothetical move): no PP to pay from

    GetProjectedMovePPCostParts(battlerAtk, battlerDef, move, &cost, &refund, &oneTimeRefund);
    pp = gBattleMons[battlerAtk].pp[pos];
    maxPP = CalculatePPWithBonus(move, gBattleMons[battlerAtk].ppBonuses, pos);

    // The first strike always goes out and spends what PP it can, as CancelerPPDeduction does.
    pp = (pp > (u32)cost) ? pp - cost : 0;
    if (refund + oneTimeRefund > 0)
        pp = min(pp + refund + oneTimeRefund, maxPP);

    for (affordable = 1; affordable < hits; affordable++)
    {
        if (!PayForStrike(&pp, maxPP, cost, refund))
            break;
    }
    return affordable;
}

u32 GetProjectedMultiHitCount(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    if (!GetConfig(FEATURE_MULTI_HIT_PP_PER_HIT) || !IsPerHitPPMove(move))
        return 1;
    return GetMultiHitPPCappedHits(battlerAtk, battlerDef, move, GetNominalStrikeCount(battlerAtk, move));
}
