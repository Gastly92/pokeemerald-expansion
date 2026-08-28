#ifndef GUARD_FORK_DETERMINISTIC_MOVES_H
#define GUARD_FORK_DETERMINISTIC_MOVES_H

// FORK: self-contained deterministic move-resolution predicates (config/deterministic.h).
//
// These helpers were extracted out of src/battle_util.c — a file we already diverge
// from heavily — to shrink that upstream-owned conflict surface. They qualify for the
// move because they are *pure* predicates: each takes primitive/move arguments and
// reads only public accessors (GetMove*, the type chart) and globals (gBattleStruct),
// touching no battle_util.c file-static state. The many one-line DETERMINISTIC_*/innate
// guards woven into battle_util.c's control flow stay there by necessity (they sit at
// the exact upstream branch they modify); only these standalone functions relocate.
//
// With every DETERMINISTIC_* flag off (the upstream default) these collapse to FALSE
// or the stock RNG roll, so behavior is unchanged in vanilla play.

#include "constants/battle.h"

// DETERMINISTIC_ADDITIONAL_EFFECTS: given a move's (dynamic) type and the pre-computed
// facts about this hit, decide whether its chance-based additional effect lands. Shared
// by the engine (TryTriggerAdditionalEffect) and the AI's prediction.
bool32 DeterministicAdditionalEffectApplies(enum Type moveType, bool32 isSuperEffective, bool32 isStab);

// DETERMINISTIC_ADDITIONAL_EFFECTS / DETERMINISTIC_FLINCH: resolves whether a move's
// chance-based additional effect (percentChance > 0) triggers this hit, replacing the
// stock RNG roll with the deterministic gate when the flag is on.
bool32 TryTriggerAdditionalEffect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, const struct AdditionalEffect *additionalEffect, u32 percentChance, u32 rngElement);

// DETERMINISTIC_ACCURACY_EVASION: TRUE when `move` locks the user into a Hyper Beam-style
// recharge turn (a damaging, non-sleep move that was exactly 50% accurate).
bool32 MoveGainsDeterministicRecharge(enum Move move);

// DETERMINISTIC_ACCURACY_EVASION: the accuracy `move` should be PRICED at by the max-PP
// scaling — its real accuracy, except for the two effects whose miss cost more than the
// wasted turn (EFFECT_TRIPLE_KICK, EFFECT_RECOIL_IF_MISS), which are discounted so the
// scaling charges them for the whole drawback the flag removes.
u32 DeterministicEffectiveAccuracy(enum Move move);

// BUFF_ACCURACY_ITEMS: how far the ATTACKER's accuracy hold item neutralises the
// DETERMINISTIC_ACCURACY_EVASION PP economy against one target. Wide Lens cancels the flat
// evasion taxes; Zoom Lens cancels those AND the target's evasion stat-stage boosts, but
// only on turns its holder moves second. Ordered so that a caller can test
// `>= ACCURACY_ITEM_RELIEF_TAXES` for "cancels the flat taxes".
enum AccuracyItemRelief
{
    ACCURACY_ITEM_RELIEF_NONE = 0, // no accuracy item, or Zoom Lens outside its window
    ACCURACY_ITEM_RELIEF_TAXES,    // Wide Lens: flat item/ability evasion taxes only
    ACCURACY_ITEM_RELIEF_FULL,     // Zoom Lens (moving second): taxes + evasion stage boosts
};

// BUFF_ACCURACY_ITEMS: the relief `battlerAtk`'s held accuracy item grants against
// `battlerDef` this turn. Returns ACCURACY_ITEM_RELIEF_NONE whenever BUFF_ACCURACY_ITEMS or
// DETERMINISTIC_ACCURACY_EVASION is off, so stock behavior is untouched.
u32 GetAccuracyItemRelief(enum BattlerId battlerAtk, enum BattlerId battlerDef);

#endif // GUARD_FORK_DETERMINISTIC_MOVES_H
