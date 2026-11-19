#ifndef GUARD_FORK_BUFF_TYPE_HIT_ITEMS_H
#define GUARD_FORK_BUFF_TYPE_HIT_ITEMS_H

#include "config_changes.h"

// BUFF_TYPE_HIT_STAT_ITEMS (config/buff.h). Kept `static inline` so each call site in
// src/battle_hold_effects.c (TrySnowball(), TryLuminousMoss(), TryCellBattery(),
// TryAbsorbBulb()) stays a one-line swap of the stock constant 1.

// Stat stages Cell Battery, Absorb Bulb, Snowball and Luminous Moss raise. Stock: +1.
// Buffed: +BUFF_TYPE_HIT_STAT_ITEM_STAGES.
static inline u32 GetTypeHitStatItemStages(void)
{
    if (GetConfig(BUFF_TYPE_HIT_STAT_ITEMS))
        return BUFF_TYPE_HIT_STAT_ITEM_STAGES;
    return 1;
}

#endif // GUARD_FORK_BUFF_TYPE_HIT_ITEMS_H
