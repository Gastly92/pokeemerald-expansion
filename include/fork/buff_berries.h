#ifndef GUARD_FORK_BUFF_BERRIES_H
#define GUARD_FORK_BUFF_BERRIES_H

#include "config_changes.h"
#include "fpmath.h"

// BUFF_RESIST_BERRIES / BUFF_PINCH_BERRIES (config/buff.h). Kept `static inline` here so
// each call site in the upstream files (GetDefenderItemsModifier() in src/battle_util.c,
// StatRaiseBerry() in src/battle_hold_effects.c) stays a one-line swap of a constant.
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
// Stock: +1, Ripen +2. Buffed: +BUFF_PINCH_BERRY_STAGES, and Ripen adds one stage rather
// than doubling, so a +2 Berry does not jump straight to +4.
static inline u32 GetPinchBerryStatStages(bool32 ripen)
{
    if (GetConfig(BUFF_PINCH_BERRIES))
        return ripen ? BUFF_PINCH_BERRY_STAGES + 1 : BUFF_PINCH_BERRY_STAGES;
    return ripen ? 2 : 1;
}

#endif // GUARD_FORK_BUFF_BERRIES_H
