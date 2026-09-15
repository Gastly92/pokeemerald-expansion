#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_script_commands.h" // FORK: HasBattlerActedThisTurn (Zoom Lens's moving-second window)
#include "config_changes.h"
#include "move.h"
#include "random.h"
#include "battle_ai_util.h" // FORK: AI_MoveMakesContact / AI_CanPoison / AI_GetWeather (AI deterministic predictions)
#include "battle_scripts.h" // FORK: BattleScript_DeterministicHoldEffectConsume
#include "fork/deterministic_moves.h"
#include "constants/battle.h"
#include "constants/battle_move_effects.h"
#include "constants/pokemon.h"
#include "fork/innate_abilities.h" // FORK: innate ability predicates

// FORK: self-contained deterministic move-resolution predicates, extracted from
// src/battle_util.c to keep that upstream-owned file's divergence small. See
// include/fork/deterministic_moves.h for why only these (pure) functions move while
// the in-place DETERMINISTIC_* guards stay in battle_util.c.

// DETERMINISTIC_ADDITIONAL_EFFECTS — a move type "can be super effective" if the stock
// type chart gives it a 2x-or-better matchup against at least one type. In the vanilla
// chart that is every type except Normal, which is never super effective; the loop keeps
// it correct for custom charts.
static bool32 MoveTypeCanBeSuperEffective(enum Type moveType)
{
    for (enum Type defType = 0; defType < NUMBER_OF_MON_TYPES; defType++)
    {
        if (gTypeEffectivenessTable[moveType][defType] >= UQ_4_12(2.0))
            return TRUE;
    }
    return FALSE;
}

// DETERMINISTIC_ADDITIONAL_EFFECTS — given a move's (dynamic) type and the pre-computed
// facts about this hit, decide whether its chance-based additional effect lands. Types
// that can be super effective gate on the hit actually being super effective; types that
// never can (Normal) gate on STAB instead. Callers supply isSuperEffective/isStab so this
// works both at run time (from the move result flags) and in the AI's prediction (from
// its own type calc). Flinch is routed through DETERMINISTIC_FLINCH and never reaches here.
bool32 DeterministicAdditionalEffectApplies(enum Type moveType, bool32 isSuperEffective, bool32 isStab)
{
    if (MoveTypeCanBeSuperEffective(moveType))
        return isSuperEffective;
    return isStab;
}

// DETERMINISTIC_ADDITIONAL_EFFECTS / DETERMINISTIC_FLINCH — resolves whether a move's
// chance-based additional effect (percentChance > 0) triggers this hit. With the relevant
// flag on, the RNG roll is replaced by a state-based rule; otherwise (or for guaranteed
// >= 100% effects) it falls back to the stock RandomPercentage roll on rngElement.
// percentChance is the already-computed (Serene Grace / Rainbow-adjusted) chance.
//
// The two flags COMPOSE for flinch: DETERMINISTIC_ADDITIONAL_EFFECTS decides the base
// trigger via the super-effective/STAB gate (flinch is gated exactly like any other
// effect — Iron Head only flinches on a super effective hit, Stomp only from a Normal
// user), and DETERMINISTIC_FLINCH then adds the anti-lock cap on top (a foe flinched last
// turn can't be flinched again), so a gated flinch still can't stunlock.
bool32 TryTriggerAdditionalEffect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, const struct AdditionalEffect *additionalEffect, u32 percentChance, u32 rngElement)
{
    // Guaranteed effects (>= 100%) always land; the deterministic flags only
    // replace genuinely chance-based rolls.
    if (percentChance < 100)
    {
        bool32 isFlinch = (additionalEffect->moveEffect == MOVE_EFFECT_FLINCH);
        bool32 triggers;

        // Base trigger: the deterministic super-effective/STAB gate under
        // DETERMINISTIC_ADDITIONAL_EFFECTS, else the stock random roll.
        if (GetConfig(DETERMINISTIC_ADDITIONAL_EFFECTS))
        {
            // The stock "double the secondary chance" boosters — Serene Grace and the
            // Pledge Rainbow — instead make the effect certain: if the computed chance
            // was boosted above the move's base chance, it bypasses the
            // super-effective/STAB gate and always lands. This includes flinch, which
            // still keeps its anti-lock cap below — so a boosted flinch lands even on a
            // neutral/resisted hit, but still can't be re-applied next turn (no lock).
            if (percentChance > additionalEffect->chance)
            {
                triggers = TRUE;
            }
            else
            {
                enum Type moveType = GetBattleMoveType(move);
                // NB: upstream's 1.17.0 sync split 4x off from 2x into MOVE_RESULT_EXTREMELY_EFFECTIVE,
                // leaving MOVE_RESULT_SUPER_EFFECTIVE meaning exactly 2x. The gate means "super
                // effective or better", so it reads the union flag; the AI mirror of this test
                // (AI_IsAdditionalEffectReliable) already uses a >= 2.0 modifier comparison.
                bool32 superEffective = (gBattleStruct->moveResultFlags[battlerDef] & MOVE_RESULT_HIGH_EFFECTIVENESS) != 0;
                triggers = DeterministicAdditionalEffectApplies(moveType, superEffective, IS_BATTLER_OF_TYPE(battlerAtk, moveType));
            }
        }
        else
        {
            triggers = RandomPercentage(rngElement, percentChance);
        }

        // DETERMINISTIC_FLINCH anti-lock cap: a foe flinched last turn can't be
        // flinched again. Fake Out (and any first-turn-only flincher) can't be used
        // on consecutive turns, so it can't chain and is exempt.
        if (triggers && isFlinch && GetConfig(DETERMINISTIC_FLINCH)
         && GetMoveEffect(move) != EFFECT_FIRST_TURN_ONLY
         && gBattleStruct->battlerState[battlerDef].flinchedLastTurn)
            triggers = FALSE;

        return triggers;
    }
    return RandomPercentage(rngElement, percentChance);
}

// DETERMINISTIC_ACCURACY_EVASION — TRUE when `move` locks the user into a Hyper Beam-style
// recharge turn: a damaging (non-sleep) move that was exactly 50% accurate. Shared by the
// move-end recharge hook (MOVEEND_DETERMINISTIC_RECHARGE) and the AI so they agree on
// which moves now recharge.
bool32 MoveGainsDeterministicRecharge(enum Move move)
{
    return GetConfig(DETERMINISTIC_ACCURACY_EVASION)
        && GetMoveAccuracy(move) == 50
        && !IsBattleMoveStatus(move)
        && GetMoveNonVolatileStatus(move) != MOVE_EFFECT_SLEEP;
}

// DETERMINISTIC_ACCURACY_EVASION — the accuracy a move should be PRICED at by the max-PP
// scaling in CalculatePPWithBonus, which is not always the accuracy it is fought at.
//
// For almost every move the two are the same: one use, one roll, so scaling max PP by the
// accuracy amortizes exactly the uses that used to be wasted. Two effects break that
// assumption because their miss cost more than the wasted turn, and the flag deletes that
// extra cost for free:
//   - EFFECT_TRIPLE_KICK rolls accuracy once per strike, so a nominally 90% Triple Axel
//     really landed its full combo 73% of the time (0.9^3) and delivered 78% of its max
//     damage on average. Pricing it at 90 charges it for one roll out of three.
//   - EFFECT_RECOIL_IF_MISS staked half the user's max HP on the roll. The crash survives
//     only where the target is genuinely unaffected (Protect / type immunity), so the
//     whiff case — the common one — costs nothing now.
// Both are charged by pricing them at DETERMINISTIC_EXTRA_MISS_COST_PERCENT of their real
// accuracy. Keying off the EFFECT rather than a move list means anything upstream adds to
// either effect is priced correctly without touching this.
//
// EFFECT_POPULATION_BOMB also rolls per strike but is deliberately absent: it already pays
// on the strike-count axis (DETERMINISTIC_POPULATION_BOMB_COUNT cuts it from 10 strikes to
// 5), and charging it on both axes would gut it.
u32 DeterministicEffectiveAccuracy(enum Move move)
{
    u32 accuracy = GetMoveAccuracy(move);
    enum BattleMoveEffects moveEffect = GetMoveEffect(move);

    if (moveEffect == EFFECT_TRIPLE_KICK || moveEffect == EFFECT_RECOIL_IF_MISS)
        accuracy = (accuracy * DETERMINISTIC_EXTRA_MISS_COST_PERCENT) / 100;

    return accuracy;
}

// BUFF_ACCURACY_ITEMS — how far the attacker's held accuracy item neutralises the
// DETERMINISTIC_ACCURACY_EVASION PP economy against one target.
//
// That flag turned accuracy/evasion from a hit/miss roll into a PP surcharge, but only the
// DEFENDER's half came across: a target's BrightPowder / Sand Veil / Snow Cloak still taxes
// the attacker a PP (GetDeterministicMoveTargetPPTax), while Wide Lens and Zoom Lens were
// left multiplying an accuracy figure nothing reads (DoesMoveMissTarget returns FALSE before
// GetTotalAccuracy is ever called), leaving both items completely inert. This restores the
// attacker's half in the same currency, following how the fork already repurposed the
// accuracy-boosting ABILITIES -- Compound Eyes / Keen Eye / Illuminate became evasion-ignore
// in GetAccEvasionStageDelta rather than an accuracy multiplier.
//
// PURE BOON: the caller only ever uses this to cancel a penalty, never to grant a refund, so
// an accuracy item can bring a move back to its base 1 PP but never below. Against a target
// with no evasion trick there is nothing to cancel and the item does nothing.
//
// Note what ACCURACY_ITEM_RELIEF_FULL buys: the caller feeds it to GetAccEvasionStageDelta's
// ignorePenalties, which zeroes BOTH stat-stage penalties -- the target's evasion boosts and
// the holder's own accuracy drops. That is deliberate (it is the same switch No Guard and
// Micle Berry flip, reused rather than reimplemented), so Zoom Lens shrugs off a Sand Attack
// as well as a Double Team. Wide Lens stops at ACCURACY_ITEM_RELIEF_TAXES and gets neither.
u32 GetAccuracyItemRelief(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    // Both gates matter: the buff is meaningless without the PP economy it plugs into, and
    // with BUFF_ACCURACY_ITEMS off the items keep their (stock) accuracy multiplier instead.
    if (!GetConfig(BUFF_ACCURACY_ITEMS) || !GetConfig(DETERMINISTIC_ACCURACY_EVASION))
        return ACCURACY_ITEM_RELIEF_NONE;

    switch (GetBattlerHoldEffect(battlerAtk))
    {
    case HOLD_EFFECT_WIDE_LENS:
        return ACCURACY_ITEM_RELIEF_TAXES;
    case HOLD_EFFECT_ZOOM_LENS:
        // Zoom Lens's stock condition, lifted verbatim from GetTotalAccuracy() so the item
        // keeps its "I move second" identity: the target must already have acted this turn,
        // and not be on its switch-in turn (isFirstTurn == 2), which doesn't count as acting.
        if (HasBattlerActedThisTurn(battlerDef) && gBattleStruct->battlerState[battlerDef].isFirstTurn != 2)
            return ACCURACY_ITEM_RELIEF_FULL;
        return ACCURACY_ITEM_RELIEF_NONE;
    default:
        return ACCURACY_ITEM_RELIEF_NONE;
    }
}

// FORK: the DETERMINISTIC_ACCURACY_EVASION accuracy/evasion and PP-cost maths. Moved out
// of src/battle_util.c, where these ~180 lines sat in upstream's hottest-churn file.

// FORK: net accuracy/evasion stage advantage of battlerAtk over battlerDef, after the
// same overrides GetTotalAccuracy() applies to the hit calc, so DETERMINISTIC_ACCURACY_EVASION's
// PP economy inherits them: positive = attacker advantage (PP recovered), negative = the
// target is harder to hit (extra PP). Keen Eye / Unaware / Minds Eye / Illuminate (Gen 9)
// and — repurposed from their accuracy boosts — Compound Eyes / Victory Star all make the
// user ignore the target's evasion, as does a living PARTNER's Victory Star; Foresight /
// Miracle Eye / Unaware on the target and
// MoveIgnoresDefenseEvasionStages do the same. Unaware on the target also nullifies the
// user's accuracy boosts. No Guard on either battler is treated as ignorePenalties (see
// below). When ignorePenalties is set (Micle Berry), the user's accuracy
// drops and the target's evasion increases are ignored, so the move can still recover PP
// from boosts/evasion drops but is never taxed by them. Clamped to the same +-6 the
// hit-calc buff used.
s32 GetAccEvasionStageDelta(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability atkAbility, enum Ability defAbility, bool32 ignorePenalties)
{
    s8 accStage = gBattleMons[battlerAtk].statStages[STAT_ACC];
    s8 evasionStage = gBattleMons[battlerDef].statStages[STAT_EVASION];

    if (atkAbility == ABILITY_UNAWARE || atkAbility == ABILITY_KEEN_EYE || atkAbility == ABILITY_MINDS_EYE
            || atkAbility == ABILITY_COMPOUND_EYES || atkAbility == ABILITY_VICTORY_STAR
            || (GetConfig(B_ILLUMINATE_EFFECT) >= GEN_9 && atkAbility == ABILITY_ILLUMINATE))
        evasionStage = DEFAULT_STAT_STAGE;
    // FORK: an innate Compound Eyes / Keen Eye / Illuminate ignores the target's evasion in the deterministic
    // PP economy too (their accuracy boost is repurposed as evasion-ignore). PURE BOON: boost-only, like
    // InnateUnawareBoonStage; the boost guard also keeps the IsInnateActive lookups off the common no-boost
    // path. Gen 9+ for Illuminate.
    else if (evasionStage > DEFAULT_STAT_STAGE
            && (IsInnateActive(battlerAtk, ABILITY_COMPOUND_EYES)
             || IsInnateActive(battlerAtk, ABILITY_KEEN_EYE)
             || IsInnateActive(battlerAtk, ABILITY_MINDS_EYE) // FORK: Mind's Eye ignores the target's evasion (Keen Eye clone, Batch Y4)
             || (GetConfig(B_ILLUMINATE_EFFECT) >= GEN_9 && IsInnateActive(battlerAtk, ABILITY_ILLUMINATE))))
        evasionStage = DEFAULT_STAT_STAGE;
    else // FORK: an innate Unaware ignores the target's evasion boosts but keeps its drops (pure boon)
        evasionStage = InnateUnawareBoonStage(battlerAtk, evasionStage);
    // FORK: mirror GetTotalAccuracy()'s ally clause -- a living partner's Victory Star boosts this
    // battler's accuracy, so (repurposed as evasion-ignore, exactly like the user's OWN Victory Star
    // above) it makes the user ignore the target's evasion in the PP economy too. PURE BOON: the
    // boost guard keeps a foe's evasion DROP recovering PP, and keeps the partner lookup off the
    // common no-boost path. No IsInnateActive(): Victory Star is never an innate (:x: in
    // absent from sImplementedInnates[] in test/fork/innate_abilities.c).
    if (evasionStage > DEFAULT_STAT_STAGE)
    {
        enum BattlerId atkAlly = GetPartnerBattler(battlerAtk);
        if (IsBattlerAlive(atkAlly) && GetBattlerAbility(atkAlly) == ABILITY_VICTORY_STAR)
            evasionStage = DEFAULT_STAT_STAGE;
    }
    if (MoveIgnoresDefenseEvasionStages(move))
        evasionStage = DEFAULT_STAT_STAGE;
    if (gBattleMons[battlerDef].volatiles.foresight || gBattleMons[battlerDef].volatiles.miracleEye)
        evasionStage = DEFAULT_STAT_STAGE;
    if (defAbility == ABILITY_UNAWARE)
        accStage = DEFAULT_STAT_STAGE;
    else // FORK: an innate Unaware ignores the attacker's accuracy boosts but keeps its drops (pure boon)
        accStage = InnateUnawareBoonStage(battlerDef, accStage);

    // FORK: No Guard makes accuracy 100% for AND against its holder, so neither the user's
    // accuracy drops nor the target's evasion increases can cost it PP -- the same shape as
    // the Micle Berry case, so it reuses it. PURE BOON: the user's accuracy boosts and the
    // target's evasion drops still recover PP. No IsInnateActive() check: No Guard is never
    // an innate (absent from sImplementedInnates[] in test/fork/innate_abilities.c).
    if (atkAbility == ABILITY_NO_GUARD || defAbility == ABILITY_NO_GUARD)
        ignorePenalties = TRUE;

    if (ignorePenalties)
    {
        if (accStage < DEFAULT_STAT_STAGE)
            accStage = DEFAULT_STAT_STAGE;     // ignore the user's accuracy drops
        if (evasionStage > DEFAULT_STAT_STAGE)
            evasionStage = DEFAULT_STAT_STAGE; // ignore the target's evasion increases
    }

    s32 delta = (accStage - DEFAULT_STAT_STAGE) - (evasionStage - DEFAULT_STAT_STAGE);
    if (delta > MAX_STAT_STAGE - DEFAULT_STAT_STAGE)
        delta = MAX_STAT_STAGE - DEFAULT_STAT_STAGE;
    if (delta < MIN_STAT_STAGE - DEFAULT_STAT_STAGE)
        delta = MIN_STAT_STAGE - DEFAULT_STAT_STAGE;
    return delta;
}

// FORK: flat extra PP that a single target imposes on `move` under DETERMINISTIC_ACCURACY_EVASION,
// stacking additively on top of the accuracy/evasion stage economy with no cap. BrightPowder /
// Lax Incense, Sand Veil (in sand), Snow Cloak (in hail/snow) and Tangled Feet (while confused)
// each add 1 PP to OFFENSIVE moves; Wonder Skin adds 1 PP to STATUS moves. No Guard on
// either battler zeroes the whole tax, since its 100% accuracy overrides every source here.
u32 GetDeterministicMoveTargetPPTax(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability defAbility, enum HoldEffect defHoldEffect)
{
    u32 tax = 0;

    // FORK: No Guard's 100% accuracy, for and against its holder, overrides every evasion
    // source below, so the move pays none of these taxes. defAbility is already in hand, so
    // it is tested first to keep the GetBattlerAbility() lookup off the common path.
    if (defAbility == ABILITY_NO_GUARD || GetBattlerAbility(battlerAtk) == ABILITY_NO_GUARD)
        return 0;

    // FORK: BUFF_ACCURACY_ITEMS -- the attacker's Wide Lens (always) or Zoom Lens (on turns it
    // moves second) cancels the flat evasion taxes below, restoring the attacker's half of the
    // accuracy axis that DETERMINISTIC_ACCURACY_EVASION otherwise dropped. Returns
    // ACCURACY_ITEM_RELIEF_NONE with either flag off, so stock behavior is untouched.
    if (GetAccuracyItemRelief(battlerAtk, battlerDef) >= ACCURACY_ITEM_RELIEF_TAXES)
        return 0;

    if (IsBattleMoveStatus(move))
    {
        if (defAbility == ABILITY_WONDER_SKIN || IsInnateActive(battlerDef, ABILITY_WONDER_SKIN)) // FORK: credit an innate Wonder Skin too
            tax++;
    }
    else
    {
        u32 weather = GetAttackerWeather(battlerAtk, GetBattlerHoldEffect(battlerAtk), GetBattlerAbility(battlerAtk), GetWeather());
        if (defHoldEffect == HOLD_EFFECT_EVASION_UP)
            tax++;
        if ((weather & B_WEATHER_SANDSTORM) // FORK: credit an innate Sand Veil too
         && (defAbility == ABILITY_SAND_VEIL || IsInnateActive(battlerDef, ABILITY_SAND_VEIL)))
            tax++;
        if ((weather & B_WEATHER_ICY_ANY) // FORK: credit an innate Snow Cloak too
         && (defAbility == ABILITY_SNOW_CLOAK || IsInnateActive(battlerDef, ABILITY_SNOW_CLOAK)))
            tax++;
        if ((defAbility == ABILITY_TANGLED_FEET || IsInnateActive(battlerDef, ABILITY_TANGLED_FEET)) // FORK: credit an innate Tangled Feet too
         && gBattleMons[battlerDef].volatiles.confusionTimer)
            tax++;
    }
    return tax;
}

// FORK: projected net PP the attacker's move will cost *this turn*, for the in-battle
// move-info display under DETERMINISTIC_ACCURACY_EVASION (where the move always hits, so
// accuracy is meaningless and the PP economy is what matters). Mirrors the deduction in
// CancelerPPDeduction (src/battle_move_resolution.c): base 1 PP, Pressure, the paralysis
// PP tax, the accuracy/evasion stage economy and the flat item/ability taxes, minus the
// PP recovered from a net accuracy advantage / Micle Berry. Targets aren't chosen yet at
// move-select time, so single-target moves project against the first live foe. The signed
// result can be negative when the move would net-recover PP. Keep in sync with CancelerPPDeduction.
s32 GetProjectedMovePPCost(enum BattlerId battlerAtk, enum Move move)
{
    s32 ppToDeduct = 1;
    s32 refund = 0;
    enum MoveTarget moveTarget = GetBattlerMoveTargetType(battlerAtk, move);
    enum Ability atkAbility = GetBattlerAbility(battlerAtk);

    // Representative single target: the first living opponent.
    enum BattlerId primaryDef = battlerAtk;
    for (u32 t = 0; t < gBattlersCount; t++)
    {
        if (!IsBattlerAlly(t, battlerAtk) && IsBattlerAlive(t))
        {
            primaryDef = t;
            break;
        }
    }

    // Pressure (matches CancelerPPDeduction).
    if (IsSpreadMove(moveTarget) || moveTarget == TARGET_ALL_BATTLERS
        || moveTarget == TARGET_FIELD || MoveForcesPressure(move))
    {
        for (u32 i = 0; i < gBattlersCount; i++)
        {
            if (!IsBattlerAlly(i, battlerAtk))
                ppToDeduct += BattlerHasAbility(i, ABILITY_PRESSURE); // FORK: innate-aware
        }
    }
    else if (moveTarget != TARGET_OPPONENTS_FIELD)
    {
        if (primaryDef != battlerAtk && BattlerHasAbility(primaryDef, ABILITY_PRESSURE)) // FORK: innate-aware
            ppToDeduct++;
    }

    if (gBattleMons[battlerAtk].status1 & STATUS1_PARALYSIS
        && atkAbility != ABILITY_QUICK_FEET
        && !BattlerHasAbility(battlerAtk, ABILITY_QUICK_FEET) // FORK: innate Quick Feet is exempt from the para PP tax, like the real ability
        && GetConfig(DETERMINISTIC_PARALYSIS))
        ppToDeduct += DETERMINISTIC_PARALYSIS_PP_TAX;

    if (GetConfig(DETERMINISTIC_ACCURACY_EVASION))
    {
        bool32 micleActive = gBattleStruct->battlerState[battlerAtk].usedMicleBerry;
        bool32 singleTargetFoe = (moveTarget == TARGET_SELECTED || moveTarget == TARGET_OPPONENT
                               || moveTarget == TARGET_RANDOM || moveTarget == TARGET_DEPENDS
                               || moveTarget == TARGET_SMART);
        bool32 spreadFoe = (moveTarget == TARGET_BOTH || moveTarget == TARGET_FOES_AND_ALLY
                         || moveTarget == TARGET_ALL_BATTLERS);

        if ((singleTargetFoe || spreadFoe) && GetMoveAccuracy(move) != 0)
        {
            for (u32 t = 0; t < gBattlersCount; t++)
            {
                if (t == battlerAtk || IsBattlerAlly(t, battlerAtk) || !IsBattlerAlive(t))
                    continue;
                if (singleTargetFoe && t != primaryDef)
                    continue;
                enum Ability defAbility = GetBattlerAbility(t);
                // FORK: BUFF_ACCURACY_ITEMS -- mirrors CancelerPPDeduction. Zoom Lens's window
                // depends on turn order, which is not decided yet at move-select time, so this
                // projection reads it as closed and under-promises rather than over-promises.
                bool32 ignorePenalties = micleActive
                                      || GetAccuracyItemRelief(battlerAtk, t) == ACCURACY_ITEM_RELIEF_FULL;
                s32 delta = GetAccEvasionStageDelta(battlerAtk, t, move, atkAbility, defAbility, ignorePenalties);
                if (delta > 0)
                    refund += delta;
                else
                    ppToDeduct += -delta;
                ppToDeduct += GetDeterministicMoveTargetPPTax(battlerAtk, t, move, defAbility, GetBattlerHoldEffect(t));
            }
            if (atkAbility == ABILITY_HUSTLE && IsBattleMovePhysical(move))
                ppToDeduct++;
        }
        if (micleActive)
            refund++;
    }

    return ppToDeduct - refund;
}

// FORK: DETERMINISTIC_MOVE_RESULTS speed ties, moved out of src/battle_main.c.
//
// The DETERMINISTIC_HOLD_EFFECTS callnatives (BS_JumpIfHangOnItemNotConsumed,
// BS_JumpIfNotDeterministicHoldEffects) deliberately did NOT move here: they are battle
// script commands and expand src/battle_script_commands.c's file-local NATIVE_ARGS macro.
// Copying that macro would create exactly the silent-drift hazard this fork keeps getting
// bitten by, so they stay with the command infrastructure they belong to.

// FORK: DETERMINISTIC_MOVE_RESULTS breaks a speed tie by a fixed ladder instead of the
// random permutation: higher raw base Speed, then lighter weight, then higher
// remaining-HP%. Returns 1 if battlerAtk wins the tie, -1 if battlerDef wins, or 0 if
// every rung is also tied (caller falls back to the random order). Under Trick Room the
// whole ladder is inverted, mirroring the speed axis. Shared with the AI's turn-order
// prediction (AI_WhoStrikesFirst) so the two never disagree.
s32 DeterministicSpeedTieWins(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    s32 result = 0;
    u32 baseAtk = gSpeciesInfo[gBattleMons[battlerAtk].species].baseSpeed;
    u32 baseDef = gSpeciesInfo[gBattleMons[battlerDef].species].baseSpeed;
    if (baseAtk != baseDef)
    {
        result = (baseAtk > baseDef) ? 1 : -1;
    }
    else
    {
        // NB: upstream gave GetBattlerWeight ability/holdEffect params in the 1.17.0 sync.
        u32 weightAtk = GetBattlerWeight(battlerAtk, GetBattlerAbility(battlerAtk), GetBattlerHoldEffect(battlerAtk));
        u32 weightDef = GetBattlerWeight(battlerDef, GetBattlerAbility(battlerDef), GetBattlerHoldEffect(battlerDef));
        if (weightAtk != weightDef)
        {
            result = (weightAtk < weightDef) ? 1 : -1; // the lighter battler strikes first
        }
        else
        {
            // higher remaining-HP% strikes first (cross-multiply to avoid fractions)
            u32 lhs = gBattleMons[battlerAtk].hp * gBattleMons[battlerDef].maxHP;
            u32 rhs = gBattleMons[battlerDef].hp * gBattleMons[battlerAtk].maxHP;
            if (lhs != rhs)
                result = (lhs > rhs) ? 1 : -1;
        }
    }

    if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
        result = -result;
    return result;
}

// FORK: the MOVEEND deterministic handlers (from src/battle_move_resolution.c) and the
// AI's deterministic-ability predictions (from src/battle_ai_util.c).

// FORK: DETERMINISTIC_HOLD_EFFECTS — consume the attacker's crit/flinch entry item
// (Scope Lens / Razor Claw / Lucky Punch / Leek, King's Rock / Razor Fang) after the
// move it fired on. IsCriticalHit()/TryKingsRock() set the pending flag; we run a
// removeitem here once per move.
enum MoveEndResult MoveEndDeterministicHoldConsume(struct BattleCalcValues *cv)
{
    enum MoveEndResult result = MOVEEND_RESULT_CONTINUE;

    // FORK: DETERMINISTIC_HOLD_EFFECTS — the attacker just took an action, so every foe
    // still on the field has now weathered a foe's action since it entered. This closes
    // their Focus Band entry-turn window (see IsBattlersEntryTurn): a holder is protected
    // only on the turn it actually faces an attack, not the turn after. Done at move end so
    // the band can still fire during this very move (the holder's entry turn). Cheap and
    // harmless when the config is off, so left ungated.
    for (u32 i = 0; i < gBattlersCount; i++)
    {
        if (IsBattlerAlive(i) && GetBattlerSide(i) != GetBattlerSide(cv->battlerAtk))
            gBattleStruct->battlerState[i].facedFoeAction = TRUE;
    }

    if (GetConfig(DETERMINISTIC_HOLD_EFFECTS)
     && gBattleStruct->battlerState[cv->battlerAtk].deterministicHoldConsumePending)
    {
        gBattleStruct->battlerState[cv->battlerAtk].deterministicHoldConsumePending = FALSE;
        if (gBattleMons[cv->battlerAtk].item != ITEM_NONE)
        {
            gLastUsedItem = gBattleMons[cv->battlerAtk].item;
            BattleScriptCall(BattleScript_DeterministicHoldEffectConsume);
            result = MOVEEND_RESULT_RUN_SCRIPT;
        }
    }

    gBattleScripting.moveendState++;
    return result;
}

// FORK: under DETERMINISTIC_ACCURACY_EVASION a damaging move that was exactly 50%
// accurate (Zap Cannon, Inferno, DynamicPunch, ...) now requires a recharge turn like
// Hyper Beam — set via the same rechargeTimer/gLockedMoves state Hyper Beam uses, so
// CancelerRecharge forces the recharge next turn. Sleep moves (Dark Void) are handled
// as drowsiness instead, and the move must have actually connected.
enum MoveEndResult MoveEndDeterministicRecharge(struct BattleCalcValues *cv)
{
    if (MoveGainsDeterministicRecharge(cv->move)
     && gBattleMons[cv->battlerAtk].volatiles.rechargeTimer == 0
     && IsBattlerAlive(cv->battlerAtk)
     && !(gBattleStruct->moveResultFlags[cv->battlerDef] & MOVE_RESULT_NO_EFFECT))
    {
        gBattleMons[cv->battlerAtk].volatiles.rechargeTimer = 2;
        gLockedMoves[cv->battlerAtk] = cv->move;
    }

    gBattleScripting.moveendState++;
    return MOVEEND_RESULT_CONTINUE;
}

// Decide whether move having an additional effect for .
// FORK: DETERMINISTIC_ABILITIES — TRUE when the attacker's own always-on ability
// guarantees a beneficial poison on this damaging move: Poison Touch on a contact
// hit, or Toxic Chain on any damaging hit. Reuses AI_CanPoison so it respects the
// same immunity/effectiveness/substitute checks as a move's own poison effect.
bool32 AI_DeterministicAbilityGuaranteesStatus(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Ability abilityAtk, abilityDef;

    if (!GetConfig(DETERMINISTIC_ABILITIES) || IsBattleMoveStatus(move))
        return FALSE;

    abilityAtk = gAiLogicData->abilities[battlerAtk];
    abilityDef = gAiLogicData->abilities[battlerDef];

    if (abilityAtk == ABILITY_POISON_TOUCH
     && AI_MoveMakesContact(battlerAtk, battlerDef, abilityAtk, gAiLogicData->holdEffects[battlerAtk], move)
     && AI_CanPoison(battlerAtk, battlerDef, abilityDef, move, gAiLogicData->partnerMove))
        return TRUE;

    if (abilityAtk == ABILITY_TOXIC_CHAIN
     && AI_CanPoison(battlerAtk, battlerDef, abilityDef, move, gAiLogicData->partnerMove))
        return TRUE;

    return FALSE;
}

// FORK: DETERMINISTIC_ABILITIES — TRUE when making contact with battlerDef would
// guarantee a status on battlerAtk via the defender's always-on contact ability
// (Static/Flame Body/Poison Point/Effect Spore/Cute Charm) and the attacker can
// actually receive it. Used to treat such a contact move as a downside.
bool32 AI_DeterministicContactAbilityPunishes(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Ability abilityAtk, abilityDef;

    if (!GetConfig(DETERMINISTIC_ABILITIES))
        return FALSE;

    abilityAtk = gAiLogicData->abilities[battlerAtk];
    abilityDef = gAiLogicData->abilities[battlerDef];

    if (!AI_MoveMakesContact(battlerAtk, battlerDef, abilityAtk, gAiLogicData->holdEffects[battlerAtk], move))
        return FALSE;

    switch (abilityDef)
    {
    case ABILITY_STATIC:
        return CanBeParalyzed(battlerDef, battlerAtk, abilityAtk);
    case ABILITY_FLAME_BODY:
        return CanBeBurned(battlerDef, battlerAtk, abilityAtk);
    case ABILITY_POISON_POINT:
        return CanBePoisoned(battlerDef, battlerAtk, abilityDef, abilityAtk);
    case ABILITY_EFFECT_SPORE:
        // FORK: Effect Spore no longer applies a status under this flag — it lowers the
        // contact attacker's accuracy by one stage. Roles are reversed versus the usual
        // CanLowerStat() call: the DEFENDER (the Effect Spore holder) is the one doing the
        // lowering, so it is passed as the first argument.
        return CanLowerStat(battlerDef, battlerAtk, gAiLogicData, STAT_ACC);
    case ABILITY_CUTE_CHARM:
        return !gBattleMons[battlerAtk].volatiles.infatuation
            && abilityAtk != ABILITY_OBLIVIOUS
            && !IsInnateActive(battlerAtk, ABILITY_OBLIVIOUS) // FORK: an innate-Oblivious attacker resists Cute Charm
            && !IsAbilityOnSide(battlerAtk, ABILITY_AROMA_VEIL)
            && !IsInnateOnSide(battlerAtk, ABILITY_AROMA_VEIL); // FORK: innate Aroma Veil on the attacker's side (Batch U)
    default:
        break;
    }

    // FORK: innate Cute Charm (FEATURE_INNATE_ABILITIES) — when the defender carries Cute Charm
    // innately but its chosen ability differs, the switch above misses it, yet making contact still
    // risks infatuation, so treat it as a downside too. (Static / Flame Body / Poison Point are never
    // innates, so only Cute Charm needs this among the status set; BattlerHasAbility is a no-op with
    // the feature off.)
    if (abilityDef != ABILITY_CUTE_CHARM && BattlerHasAbility(battlerDef, ABILITY_CUTE_CHARM))
        return !gBattleMons[battlerAtk].volatiles.infatuation
            && abilityAtk != ABILITY_OBLIVIOUS
            && !IsInnateActive(battlerAtk, ABILITY_OBLIVIOUS) // FORK: an innate-Oblivious attacker resists Cute Charm
            && !IsAbilityOnSide(battlerAtk, ABILITY_AROMA_VEIL)
            && !IsInnateOnSide(battlerAtk, ABILITY_AROMA_VEIL); // FORK: innate Aroma Veil on the attacker's side (Batch U)

    // FORK: innate Effect Spore (Tier 5.10) — same shape as the Cute Charm clause above. The switch
    // keys off the chosen ability, so a holder whose Effect Spore is innate-only would be missed,
    // yet contact still guarantees the accuracy drop. IsInnateActive supplies the usual suppression.
    if (abilityDef != ABILITY_EFFECT_SPORE && IsInnateActive(battlerDef, ABILITY_EFFECT_SPORE))
        return CanLowerStat(battlerDef, battlerAtk, gAiLogicData, STAT_ACC);

    return FALSE;
}

// FORK: under DETERMINISTIC_ABILITIES, some abilities cure the holder's
// non-volatile status at the *end of every turn*, so inflicting one on a known
// holder of such an ability is always wasted - it is wiped before it can act. The
// engine still applies-then-cures (Synchronize, status-flash messaging, etc. fire
// normally); this only stops the AI valuing a doomed status. Shed Skin cures
// unconditionally; Hydration cures only while the target is being rained on (same
// condition as the engine in AbilityBattleEffects, but read through the AI's view
// of weather). Healer is deliberately not handled here - it cures the *partner*,
// not the holder, so it isn't keyed on the target's own ability (see notes).
bool32 StatusWillBeCuredDeterministically(enum BattlerId battlerDef, enum Ability defAbility)
{
    if (!GetConfig(DETERMINISTIC_ABILITIES))
        return FALSE;
    switch (defAbility)
    {
    case ABILITY_SHED_SKIN:
        return TRUE;
    case ABILITY_HYDRATION:
        return IsBattlerWeatherAffected(gAiLogicData->holdEffects[battlerDef], AI_GetWeather(), B_WEATHER_RAIN);
    default:
        return FALSE;
    }
}
