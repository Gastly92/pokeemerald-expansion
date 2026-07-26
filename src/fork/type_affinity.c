#include "global.h"
#include "fork/type_affinity.h"
#include "battle.h"
#include "battle_util.h"
#include "constants/abilities.h"
#include "constants/pokemon.h"

// FORK: the "Affinity" ability family (see include/fork/type_affinity.h and
// fork-docs/NEW_ABILITIES.md). An Affinity ability grants the holder a latent THIRD type in
// battle. The grant is applied at the single canonical type accessor GetBattlerTypes()
// (src/battle_util.c), exactly as FEATURE_NEW_TYPES injects at GetSpeciesType(), so the added
// type flows to STAB, defensive matchups, IS_BATTLER_OF_TYPE and the AI from one hook. A
// switch-in ability popup + message announces it (ABILITYEFFECT_ON_SWITCHIN in battle_util.c).
//
// GetBattlerTypes() runs on the AI's hot path (its thinking time is bounded by
// test/battle/ai/ai_thinking_time.c), so the common no-Affinity case must be cheap: the raw
// chosen-ability read below (an inlined switch on a struct field) early-outs before the
// pricier suppression-aware GetBattlerAbility() lookup, which only Affinity holders reach.

void TryApplyTypeAffinity(u32 battler, enum Type types[3])
{
    // Cheap gate: almost no battler holds an Affinity ability, so this returns immediately for
    // the vast majority of type reads without paying for GetBattlerAbility().
    if (GetAbilityAffinityType(gBattleMons[battler].ability) == TYPE_MYSTERY)
        return;

    // Confirmed Affinity holder (rare). Resolve suppression-aware so Gastro Acid / Neutralizing
    // Gas turn the ability -- and its granted type -- off.
    enum Type affinity = GetAbilityAffinityType(GetBattlerAbility(battler));
    if (affinity == TYPE_MYSTERY)
        return;

    // Only fill an EMPTY third slot, so a type-changing move that already wrote types[2]
    // (Trick-or-Treat, Soak, Reflect Type) keeps precedence. Skip if the mon is already that
    // type in a base slot, to avoid a redundant duplicate.
    if (types[2] != TYPE_MYSTERY || types[0] == affinity || types[1] == affinity)
        return;
    types[2] = affinity;
}
