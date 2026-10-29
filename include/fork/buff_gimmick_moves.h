#ifndef GUARD_FORK_BUFF_GIMMICK_MOVES_H
#define GUARD_FORK_BUFF_GIMMICK_MOVES_H

#include "battle_gimmick.h"
#include "config_changes.h"

// BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS (config/buff.h): Z-Moves and Max Moves ignore the target's
// positive Defense / Sp. Def stages, the way a critical hit does. Kept `static inline` here so its
// one call site in CalcDefenseStat() (src/battle_util.c) stays a single line in that high-churn
// upstream file.
//
// Keyed off the attacker's active gimmick, the same signal CalcMoveBasePower's Z-Move / Dynamax
// branch uses: GIMMICK_Z_MOVE is only active for the Z-Move itself (cleared once it is used), and
// every damaging move of a Dynamaxed battler is a Max Move. The AI's damage simulation and the
// move-menu damage preview set the gimmick active before simulating, so both price the buff too.
static inline bool32 GimmickMoveIgnoresDefenseBoosts(enum BattlerId battlerAtk)
{
    enum Gimmick gimmick = GetActiveGimmick(battlerAtk);

    return GetConfig(BUFF_GIMMICK_MOVES_IGNORE_DEF_BOOSTS)
        && (gimmick == GIMMICK_Z_MOVE || gimmick == GIMMICK_DYNAMAX);
}

#endif // GUARD_FORK_BUFF_GIMMICK_MOVES_H
