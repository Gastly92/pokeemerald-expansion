#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_script_commands.h" // FORK: HasBattlerActedThisTurn (Zoom Lens's moving-second window)
#include "config_changes.h"
#include "move.h"
#include "random.h"
#include "fork/deterministic_moves.h"
#include "constants/battle.h"
#include "constants/battle_move_effects.h"
#include "constants/pokemon.h"

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
                bool32 superEffective = (gBattleStruct->moveResultFlags[battlerDef] & MOVE_RESULT_SUPER_EFFECTIVE) != 0;
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
