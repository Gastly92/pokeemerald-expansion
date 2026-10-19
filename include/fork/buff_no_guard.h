#ifndef GUARD_FORK_BUFF_NO_GUARD_H
#define GUARD_FORK_BUFF_NO_GUARD_H

#include "battle_ai_record.h"
#include "battle_util.h"
#include "config_changes.h"
#include "move.h"

// BUFF_NO_GUARD (config/buff.h) -- "wild swings": a damaging move used by or against a No Guard
// battler turns the accuracy it gave up into power, (BUFF_NO_GUARD_POWER_BASE - accuracy) / 100.
// Kept `static inline` here so its one call site in CalcMoveBasePowerAfterModifiers()
// (src/battle_util.c) stays a single line in that high-churn upstream file.
//
// Reads ctx->abilities[], which the damage calc fills from the battler's real ability and the AI
// / damage preview fill from what is known, so the preview only credits a foe's No Guard once it
// has been revealed. No Guard is never an innate (absent from sImplementedInnates[] in
// test/fork/innate_abilities.c), so there is no IsInnateActive() half.
static inline uq4_12_t GetNoGuardWildSwingModifier(struct DamageContext *ctx)
{
    u32 accuracy = GetMoveAccuracy(ctx->move);
    enum BattlerId holder;

    if (!GetConfig(BUFF_NO_GUARD) || !GetConfig(DETERMINISTIC_ACCURACY_EVASION))
        return UQ_4_12(1.0);
    if (accuracy == 0 || accuracy >= 100 || GetMoveEffect(ctx->move) == EFFECT_OHKO)
        return UQ_4_12(1.0);
    if (ctx->abilities[ctx->battlerAtk] == ABILITY_NO_GUARD)
        holder = ctx->battlerAtk;
    else if (ctx->abilities[ctx->battlerDef] == ABILITY_NO_GUARD)
        holder = ctx->battlerDef;
    else
        return UQ_4_12(1.0);

    if (ctx->updateFlags)
        RecordAbilityBattle(holder, ABILITY_NO_GUARD);
    return (BUFF_NO_GUARD_POWER_BASE - accuracy) * UQ_4_12(1.0) / 100;
}

#endif // GUARD_FORK_BUFF_NO_GUARD_H
