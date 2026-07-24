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

#endif // GUARD_FORK_DETERMINISTIC_MOVES_H
