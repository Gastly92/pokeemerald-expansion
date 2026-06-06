#ifndef GUARD_FRONTIER_EXTENDED_MONS_H
#define GUARD_FRONTIER_EXTENDED_MONS_H

#include "data.h"

// FORK: which battle format(s) an extended-roster set is suited for. Stored in
// the otherwise-unused struct TrainerMon.tags field (the Battle Factory never
// reads tags; only trainer_pools.c does, for regular NPC trainers). The Factory's
// mon selector (GetFactoryMonId) only draws a mon whose tags include the current
// battle mode, so a singles-only set never appears in a doubles challenge and
// vice versa. Every roster entry MUST set one of these (a 0/unset tags would
// match no mode and stall the selector). The roster must always contain enough
// FORMAT_SINGLES- and FORMAT_DOUBLES-eligible mons to fill a team in each mode.
#define FORMAT_SINGLES (1 << 0)
#define FORMAT_DOUBLES (1 << 1)
#define FORMAT_BOTH    (FORMAT_SINGLES | FORMAT_DOUBLES)

// FORK: fork-owned Battle Factory roster overhaul (B_FRONTIER_EXTENDED_MONS).
// gFrontierExtendedMons replaces the vanilla gBattleFrontierMons on the Battle
// Factory's code paths when the flag is on. Defined in
// src/frontier_extended_mons.c. The count is exported as a runtime value (the
// list grows generation by generation, so it is not a compile-time constant the
// rest of the code hard-codes).
extern const struct TrainerMon gFrontierExtendedMons[];
extern const u16 gFrontierExtendedMonsCount;

#endif // GUARD_FRONTIER_EXTENDED_MONS_H
