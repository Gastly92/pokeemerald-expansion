#ifndef GUARD_FORK_BUFF_BERRIES_H
#define GUARD_FORK_BUFF_BERRIES_H

#include "config_changes.h"
#include "fpmath.h"

// BUFF_RESIST_BERRIES / BUFF_PINCH_BERRIES / BUFF_RETALIATION_BERRIES /
// BUFF_DAMAGED_STAT_BERRIES (config/buff.h). Kept `static inline` here so
// each call site in the upstream files (GetDefenderItemsModifier() in src/battle_util.c,
// StatRaiseBerry(), RandomStatRaiseBerry(), TryJabocaBerry(), TryRowapBerry() and
// DamagedStatBoostBerryEffect() in src/battle_hold_effects.c) stays a one-line
// swap of a constant.
//
// Both take whether the holder's Ripen (ability or innate) is active, which the caller
// already works out, so the Ripen check is not duplicated here.

// Damage multiplier a resist Berry applies to the hit it fires on. Stock: 0.5x, Ripen
// 0.25x. Buffed: 1/BUFF_RESIST_BERRY_DIVISOR, and Ripen still halves that again.
static inline uq4_12_t GetResistBerryModifier(bool32 ripen)
{
    if (GetConfig(BUFF_RESIST_BERRIES))
        return ripen ? UQ_4_12(1.0 / (2 * BUFF_RESIST_BERRY_DIVISOR)) : UQ_4_12(1.0 / BUFF_RESIST_BERRY_DIVISOR);
    return ripen ? UQ_4_12(0.25) : UQ_4_12(0.5);
}

// Stat stages a single-stat pinch Berry (Liechi, Ganlon, Salac, Petaya, Apicot) raises.
// Stock: +1, Ripen +2. Buffed: +BUFF_PINCH_BERRY_STAGES, and Ripen still doubles it.
static inline u32 GetPinchBerryStatStages(bool32 ripen)
{
    if (GetConfig(BUFF_PINCH_BERRIES))
        return ripen ? BUFF_PINCH_BERRY_STAGES * 2 : BUFF_PINCH_BERRY_STAGES;
    return ripen ? 2 : 1;
}

// Stat stages Starf Berry raises. Stock: +2, Ripen +4 (one more than the single-stat Berries,
// paying for the stat being random -- or, under DETERMINISTIC_HOLD_EFFECTS, the holder's
// highest rather than chosen). Buffed: the same as the single-stat Berries, so Starf trades
// control of the stat for adaptability rather than for fewer stages.
static inline u32 GetStarfBerryStatStages(bool32 ripen)
{
    if (GetConfig(BUFF_PINCH_BERRIES))
        return GetPinchBerryStatStages(ripen);
    return ripen ? 4 : 2;
}

// Recoil Jaboca / Rowap Berry deal to the attacker, from its non-Dynamax max HP. Stock: 1/8,
// Ripen 1/4. Buffed: 1/BUFF_RETALIATION_BERRY_DENOMINATOR, and Ripen still doubles it.
static inline s32 GetRetaliationBerryDamage(s32 maxHP, bool32 ripen)
{
    s32 damage = maxHP / (GetConfig(BUFF_RETALIATION_BERRIES) ? BUFF_RETALIATION_BERRY_DENOMINATOR : 8);
    return ripen ? damage * 2 : damage;
}

// Stat stages Kee / Maranga Berry raise. Stock: +1, Ripen +2. Buffed:
// +BUFF_DAMAGED_STAT_BERRY_STAGES, and Ripen still doubles it.
static inline u32 GetDamagedStatBerryStages(bool32 ripen)
{
    if (GetConfig(BUFF_DAMAGED_STAT_BERRIES))
        return ripen ? BUFF_DAMAGED_STAT_BERRY_STAGES * 2 : BUFF_DAMAGED_STAT_BERRY_STAGES;
    return ripen ? 2 : 1;
}

#endif // GUARD_FORK_BUFF_BERRIES_H
