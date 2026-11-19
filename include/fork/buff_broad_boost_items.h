#ifndef GUARD_FORK_BUFF_BROAD_BOOST_ITEMS_H
#define GUARD_FORK_BUFF_BROAD_BOOST_ITEMS_H

#include "config_changes.h"
#include "fpmath.h"

// BUFF_BROAD_BOOST_ITEMS (config/buff.h). Kept `static inline` so each call site in the
// attacker hold-effect switch in src/battle_util.c (Muscle Band, Wise Glasses, Punching Glove)
// stays a one-line wrap of the stock multiplier.

// Damage multiplier for Muscle Band, Wise Glasses and Punching Glove. Returns the caller's stock
// multiplier when the flag is off, +BUFF_BROAD_BOOST_ITEM_PERCENT when it is on.
static inline uq4_12_t GetBroadBoostItemModifier(uq4_12_t stockModifier)
{
    if (GetConfig(BUFF_BROAD_BOOST_ITEMS))
        return UQ_4_12(1.0 + BUFF_BROAD_BOOST_ITEM_PERCENT / 100.0);
    return stockModifier;
}

#endif // GUARD_FORK_BUFF_BROAD_BOOST_ITEMS_H
