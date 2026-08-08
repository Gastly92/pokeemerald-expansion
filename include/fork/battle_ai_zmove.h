#ifndef GUARD_FORK_BATTLE_AI_ZMOVE_H
#define GUARD_FORK_BATTLE_AI_ZMOVE_H

#include "battle.h"

// FORK: AI flag for spending the one-per-battle Z-Move deliberately.
//
// Upstream's ShouldUseZMove is effectively "yes" for any damaging move that the plain
// move would not already KO with - it never asks whether the Z-Move buys anything, so
// the AI burns it turn one on whatever move it happened to pick, including into a
// resist. Mirrors AI_FLAG_SMART_TERA, which exists for exactly the same reason
// ("default is to always tera whenever available").
//
// Defined here rather than in include/constants/battle_ai.h so upstream edits to that
// header never conflict with it. Bit 34 is AI_FLAG_SMART_SPECIES_LOGIC.
#define AI_FLAG_SMART_Z_MOVE ((u64)1 << 35)

bool32 AI_ShouldSpendZMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move chosenMove);

#endif // GUARD_FORK_BATTLE_AI_ZMOVE_H
