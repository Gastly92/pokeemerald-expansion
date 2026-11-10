#ifndef GUARD_FORK_MULTI_HIT_PP_H
#define GUARD_FORK_MULTI_HIT_PP_H

// FEATURE_MULTI_HIT_PP_PER_HIT (include/config/feature.h): every strike of a multi-hit move
// costs PP. See src/fork/multi_hit_pp.c and fork-docs/MULTI_HIT_PP.md.

// At least as many strikes as any move throws (Population Bomb's 10), for "cap whatever the count is".
#define MAX_PER_HIT_PP_STRIKES 10

// TRUE for a move whose strikes are each charged: a 2-5 hit move, a fixed multi-strike move,
// Population Bomb and Beat Up. FALSE for everything else, Parental Bond's second hit included.
bool32 IsPerHitPPMove(enum Move move);

// The battle-side hooks, all in src/battle_move_resolution.c:
//   CancelerPPDeduction calls MultiHitPP_Reset() on entry and MultiHitPP_RecordUse() after it
//   has paid for the first strike; MoveEndMultihitMove calls MultiHitPP_TryChargeNextHit()
//   before each further strike, and MultiHitPP_Reset() once the move is done.
void MultiHitPP_Reset(void);
void MultiHitPP_RecordUse(enum BattlerId battlerAtk, enum Move move, u32 movePosition, s32 cost, s32 refundPerHit);
bool32 MultiHitPP_TryChargeNextHit(enum BattlerId battlerAtk, enum Move move);

// How many strikes the move would land this turn against battlerDef: the nominal count
// (Skill Link, Loaded Dice, the DETERMINISTIC_MOVE_RESULTS counts, Beat Up's party) capped by
// the PP the attacker can pay. 1 for a single-hit move. A random 2-5 roll projects its
// maximum. The move menu shows it beside the power and prices GetProjectedMovePPCost with it.
u32 GetProjectedMultiHitCount(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// The strikes, out of `hits`, that the attacker can pay for when it uses `move` on battlerDef
// from its current PP. Returns `hits` when the feature is off or the move is not per-hit.
// Used by the AI's damage simulation (and so the damage preview) to price only those strikes.
u32 GetMultiHitPPCappedHits(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, u32 hits);

#endif // GUARD_FORK_MULTI_HIT_PP_H
