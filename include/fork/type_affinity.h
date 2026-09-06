#ifndef GUARD_FORK_TYPE_AFFINITY_H
#define GUARD_FORK_TYPE_AFFINITY_H

#include "constants/abilities.h"
#include "constants/pokemon.h" // enum Type

// FORK: the "Affinity" ability family. An Affinity ability is a mon's CHOSEN ability that
// grants it a latent third type in battle (Psychic Affinity -> Psychic, etc.). Unlike an
// innate (a pure boon), it carries the added type's weaknesses as well as its STAB, so it
// is deliberately a chosen ability. See fork-docs/NEW_ABILITIES.md and src/fork/type_affinity.c.

// Returns the type an Affinity ability grants, or TYPE_MYSTERY if `ability` isn't one.
// `static inline` so the (near-always-negative) gate inlines into its callers -- notably
// GetBattlerTypes(), which the AI runs on a hot path where added cost is measured
// (test/battle/ai/ai_thinking_time.c).
static inline enum Type GetAbilityAffinityType(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_PSYCHIC_AFFINITY:
        return TYPE_PSYCHIC;
    case ABILITY_WATER_AFFINITY:
        return TYPE_WATER;
    // Add a family member here: one `case` returning the type it grants. Everything else
    // (the type injection, the switch-in message) is shared by the whole family.
    default:
        return TYPE_MYSTERY;
    }
}

// If `battler` holds an Affinity ability, writes its granted type into an empty third slot
// of `types` (leaving a real third type set by a move alone). Called from GetBattlerTypes().
void TryApplyTypeAffinity(u32 battler, enum Type types[3]);

#endif // GUARD_FORK_TYPE_AFFINITY_H
