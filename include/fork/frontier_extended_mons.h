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

// FORK: readable roster-authoring helpers for frontier_extended_mons.c. These
// only make the data file easier to edit (e.g. on mobile); they compile down to
// exactly the same values the raw struct fields expect, so they're zero-cost.

// --- EVS(): named-field EV spreads ---------------------------------------------
// struct TrainerMon.ev is a `const u8 *` into a 6-byte array in the fixed order
// [hp, atk, def, spatk, spdef, speed] (see CreateFacilityMon in battle_frontier.c
// and TRAINER_PARTY_EVS in data.h). EVS() lets you write only the stats you care
// about, by name, in any order — unset stats default to 0:
//     .ev = EVS(.hp = 252, .def = 252, .spd = 4),
// is identical to the old TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4). Field names use
// the standard competitive shorthand: hp / atk / def / spa (Sp. Atk) / spd
// (Sp. Def) / spe (Speed). The struct is laid out in the array's order, so taking
// its address and reading it as `const u8[6]` yields the same six bytes the engine
// reads. The compound literal has static storage at file scope, so its address is
// a valid constant initializer (exactly like TRAINER_PARTY_EVS's array literal).
struct EvSpread { u8 hp, atk, def, spa, spd, spe; };
#define EVS(...) ((const u8 *)&(const struct EvSpread){__VA_ARGS__})

// --- NATURE(): natures by what they do, not by name ----------------------------
// Instead of remembering that "Bold" is +Def/-Atk, write the effect directly:
//     .nature = NATURE(DEF_UP, ATK_DOWN),   // == NATURE_BOLD
// First argument is the boosted stat (XXX_UP), second is the lowered stat
// (YYY_DOWN), using ATK / DEF / SPA / SPD / SPE. All 20 stat-changing natures are
// covered; the five neutral natures have no up/down pair, so for those keep the
// plain constant (NATURE_HARDY etc.) — NATURE_NEUTRAL is provided as a clear alias.
#define NATURE(up, down) NATURE_PASTE_(up, down)
#define NATURE_PASTE_(up, down) NATURE_##up##_##down

#define NATURE_NEUTRAL          NATURE_HARDY

#define NATURE_ATK_UP_DEF_DOWN  NATURE_LONELY
#define NATURE_ATK_UP_SPE_DOWN  NATURE_BRAVE
#define NATURE_ATK_UP_SPA_DOWN  NATURE_ADAMANT
#define NATURE_ATK_UP_SPD_DOWN  NATURE_NAUGHTY
#define NATURE_DEF_UP_ATK_DOWN  NATURE_BOLD
#define NATURE_DEF_UP_SPE_DOWN  NATURE_RELAXED
#define NATURE_DEF_UP_SPA_DOWN  NATURE_IMPISH
#define NATURE_DEF_UP_SPD_DOWN  NATURE_LAX
#define NATURE_SPE_UP_ATK_DOWN  NATURE_TIMID
#define NATURE_SPE_UP_DEF_DOWN  NATURE_HASTY
#define NATURE_SPE_UP_SPA_DOWN  NATURE_JOLLY
#define NATURE_SPE_UP_SPD_DOWN  NATURE_NAIVE
#define NATURE_SPA_UP_ATK_DOWN  NATURE_MODEST
#define NATURE_SPA_UP_DEF_DOWN  NATURE_MILD
#define NATURE_SPA_UP_SPE_DOWN  NATURE_QUIET
#define NATURE_SPA_UP_SPD_DOWN  NATURE_RASH
#define NATURE_SPD_UP_ATK_DOWN  NATURE_CALM
#define NATURE_SPD_UP_DEF_DOWN  NATURE_GENTLE
#define NATURE_SPD_UP_SPE_DOWN  NATURE_SASSY
#define NATURE_SPD_UP_SPA_DOWN  NATURE_CAREFUL

// FORK: fork-owned Battle Factory roster overhaul (B_FRONTIER_EXTENDED_MONS).
// gFrontierExtendedMons replaces the vanilla gBattleFrontierMons on the Battle
// Factory's code paths when the flag is on. Defined in
// src/frontier_extended_mons.c. The count is exported as a runtime value (the
// list grows generation by generation, so it is not a compile-time constant the
// rest of the code hard-codes).
extern const struct TrainerMon gFrontierExtendedMons[];
extern const u16 gFrontierExtendedMonsCount;

// FORK: uniform, format-aware draw from the competitive roster, shared by any
// facility (Battle Factory, Battle Tower) whose opponents pull from it.
u16 GetRandomFrontierExtendedMonId(void);

#endif // GUARD_FRONTIER_EXTENDED_MONS_H
