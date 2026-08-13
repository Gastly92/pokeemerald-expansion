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

// --- IVS(): named-stat IV spreads, defaulting to 31 ----------------------------
// struct TrainerMon.iv is a u32 with six 5-bit fields packed in the order
// [hp, atk, def, speed, spatk, spdef] (TRAINER_PARTY_IVS in data.h). Writing that
// macro by hand is error-prone in two ways: Speed is its THIRD-from-last argument
// (not last, unlike EVS()'s field order), and a set that only wants one stat
// changed still has to spell out five 31s. IVS() lets you name just the stats you
// are changing; every stat you don't name defaults to 31:
//     .iv = IVS(SPE, 0),                    // == TRAINER_PARTY_IVS(31,31,31,0,31,31)
//     .iv = IVS(SPE, 0, ATK, 0),            // == TRAINER_PARTY_IVS(31,0,31,0,31,31)
// One to six STAT, value pairs in any order, naming stats with the same shorthand
// as EVS(): HP / ATK / DEF / SPA / SPD / SPE. The STATIC_ASSERTs below pin the
// encoding. At least one pair is required — to leave every IV at 31, omit the .iv
// field entirely (which is how the other ~1180 sets are written).
//
// Why this is `IVS(SPE, 0)` and not `IVS(.spe = 0)` matching EVS(): two separate
// reasons, both hard.
//  1. EVS() hands the engine a POINTER, so it can use a designated-initializer
//     compound literal — taking its address is a valid constant initializer. `.iv`
//     is a u32, so the same shape would mean READING members back out of a compound
//     literal, which is not a constant expression and cannot initialize ROM data.
//     That fails even if all six stats are named, so it is not about defaults.
//  2. Designated initializers zero-fill, so `{.hp = 31}` and `{.hp = 31, .spe = 0}`
//     are byte-identical. An omitted stat and an explicit `.spe = 0` are literally
//     the same bytes, so "unnamed means 31" and "explicit 0 means 0" cannot coexist
//     under ANY representation — and an explicit 0 is exactly what these sets need.
// Naming the stat as a token sidesteps both: which stats were given is known at
// preprocessing time, so the default mask below is built by the preprocessor and
// the whole thing folds to one u32 constant.
//
// Pair ORDER does not matter — each pair only touches its own 5-bit field, and both
// the mask and the value are combined with |. IVS(SPE, 0, ATK, 0) and
// IVS(ATK, 0, SPE, 0) are the same constant (sIvsAnyOrder below pins this).
//
// But DO NOT name the same stat twice: the values are OR'd, not overwritten, so
// IVS(SPE, 1, SPE, 2) silently yields a Speed IV of 3 rather than 1 or 2. There is
// no way to make that a compile error here, so just don't repeat a stat.
//
// Note the engine treats iv == 0 as "unset" and falls back to the facility default
// (`if (fmon->iv)` in CreateFacilityMon), so a spread that names all six stats as 0
// is silently ignored. That is inherited from TRAINER_PARTY_IVS, not new here.
#define IVS_SHIFT_HP   0
#define IVS_SHIFT_ATK  5
#define IVS_SHIFT_DEF  10
#define IVS_SHIFT_SPE  15
#define IVS_SHIFT_SPA  20
#define IVS_SHIFT_SPD  25
#define IVS_ALL_31_    0x3FFFFFFFu
#define IVS_MASK_(s)     (31u << IVS_SHIFT_##s)
#define IVS_VAL_(s, v)   (((v) & 31u) << IVS_SHIFT_##s)
#define IVS_BUILD_(masks, vals) ((IVS_ALL_31_ & ~(masks)) | (vals))

#define IVS_1_(s1, v1) IVS_BUILD_(IVS_MASK_(s1), IVS_VAL_(s1, v1))
#define IVS_2_(s1, v1, s2, v2) \
    IVS_BUILD_(IVS_MASK_(s1) | IVS_MASK_(s2), IVS_VAL_(s1, v1) | IVS_VAL_(s2, v2))
#define IVS_3_(s1, v1, s2, v2, s3, v3) \
    IVS_BUILD_(IVS_MASK_(s1) | IVS_MASK_(s2) | IVS_MASK_(s3), \
               IVS_VAL_(s1, v1) | IVS_VAL_(s2, v2) | IVS_VAL_(s3, v3))
#define IVS_4_(s1, v1, s2, v2, s3, v3, s4, v4) \
    IVS_BUILD_(IVS_MASK_(s1) | IVS_MASK_(s2) | IVS_MASK_(s3) | IVS_MASK_(s4), \
               IVS_VAL_(s1, v1) | IVS_VAL_(s2, v2) | IVS_VAL_(s3, v3) | IVS_VAL_(s4, v4))
#define IVS_5_(s1, v1, s2, v2, s3, v3, s4, v4, s5, v5) \
    IVS_BUILD_(IVS_MASK_(s1) | IVS_MASK_(s2) | IVS_MASK_(s3) | IVS_MASK_(s4) | IVS_MASK_(s5), \
               IVS_VAL_(s1, v1) | IVS_VAL_(s2, v2) | IVS_VAL_(s3, v3) | IVS_VAL_(s4, v4) \
               | IVS_VAL_(s5, v5))
#define IVS_6_(s1, v1, s2, v2, s3, v3, s4, v4, s5, v5, s6, v6) \
    IVS_BUILD_(IVS_MASK_(s1) | IVS_MASK_(s2) | IVS_MASK_(s3) | IVS_MASK_(s4) | IVS_MASK_(s5) \
               | IVS_MASK_(s6), \
               IVS_VAL_(s1, v1) | IVS_VAL_(s2, v2) | IVS_VAL_(s3, v3) | IVS_VAL_(s4, v4) \
               | IVS_VAL_(s5, v5) | IVS_VAL_(s6, v6))

// Picks IVS_<n>_ by counting the STAT, value pairs actually written.
#define IVS_PICK_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME, ...) NAME
#define IVS(...) IVS_PICK_(__VA_ARGS__, IVS_6_, IVS_6_, IVS_5_, IVS_5_, IVS_4_, IVS_4_, \
                           IVS_3_, IVS_3_, IVS_2_, IVS_2_, IVS_1_, IVS_1_, )(__VA_ARGS__)

STATIC_ASSERT(IVS(SPE, 0) == TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), sIvsMinSpeed);
STATIC_ASSERT(IVS(SPE, 0, ATK, 0) == TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31), sIvsMinSpeedAtk);
STATIC_ASSERT(IVS(HP, 31) == TRAINER_PARTY_IVS(31, 31, 31, 31, 31, 31), sIvsAllMax);
STATIC_ASSERT(IVS(HP, 1, ATK, 2, DEF, 3, SPE, 4, SPA, 5, SPD, 6)
              == TRAINER_PARTY_IVS(1, 2, 3, 4, 5, 6), sIvsFieldOrder);
STATIC_ASSERT(IVS(SPD, 7, SPA, 6, SPE, 5, DEF, 4, ATK, 3)
              == TRAINER_PARTY_IVS(31, 3, 4, 5, 6, 7), sIvsAnyOrder);

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
