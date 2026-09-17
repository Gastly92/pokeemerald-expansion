#ifndef GUARD_FORK_BUFF_CONFUSION_H
#define GUARD_FORK_BUFF_CONFUSION_H

#include "config_changes.h"

// FORK: BUFF_CONFUSION_SELF_DAMAGE (config/buff.h) -- base power of the typeless self-hit a
// confused battler takes. Kept `static inline` in a fork header rather than in a .c so the
// call sites in CancelerConfused() (src/battle_move_resolution.c) stay one-liners and no fork
// function has to live in that high-churn upstream file.
//
// Depends on upstream's `struct DamageContext.fixedBasePower` hook, which is how both
// confusion self-hit sites express "40 BP typeless"; it is a u32:8 bitfield, so the power must
// stay <= 255. If upstream ever stops routing the self-hit through fixedBasePower, re-hook
// wherever the self-hit's power moves rather than deleting this.
static inline u32 GetConfusionSelfDamagePower(void)
{
    // 40 is upstream's hardcoded confusion self-hit power; it has no named constant there.
    return GetConfig(BUFF_CONFUSION_SELF_DAMAGE) ? BUFF_CONFUSION_SELF_DAMAGE_POWER : 40;
}

#endif // GUARD_FORK_BUFF_CONFUSION_H
