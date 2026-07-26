#ifndef GUARD_FORK_TYPE_AFFINITY_H
#define GUARD_FORK_TYPE_AFFINITY_H

#include "constants/abilities.h"
#include "constants/pokemon.h" // enum Type

// FORK: the "Affinity" ability family. An Affinity ability is a mon's CHOSEN ability that
// grants it a latent third type in battle (Psychic Affinity -> Psychic, etc.). Unlike an
// innate (a pure boon), it carries the added type's weaknesses as well as its STAB, so it
// is deliberately a chosen ability. See fork-docs/NEW_ABILITIES.md and src/fork/type_affinity.c.

// Returns the type an Affinity ability grants, or TYPE_MYSTERY if `ability` isn't one.
enum Type GetAbilityAffinityType(enum Ability ability);

// If `battler` holds an Affinity ability, writes its granted type into an empty third slot
// of `types` (leaving a real third type set by a move alone). Called from GetBattlerTypes().
void TryApplyTypeAffinity(u32 battler, enum Type types[3]);

#endif // GUARD_FORK_TYPE_AFFINITY_H
