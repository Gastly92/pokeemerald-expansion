#include "global.h"
#include "innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species->innate table (FEATURE_INNATE_ABILITIES). Kept here
// instead of in gSpeciesInfo so upstream syncs never touch it and the upstream
// species data stays untouched. Each row lists a species and up to
// MAX_INNATE_ABILITIES innate abilities that are always active on top of that
// species' normal chosen ability; unused slots are ABILITY_NONE. To give a
// species an innate, add or extend its row here — no other file needs editing.
//
// This is a deliberately small seed list; expand it as the fork's roster grows.
// Innates are additive passives, never the battler's copyable/swappable identity
// (see config/feature.h and BattlerHasAbility() for the exact semantics).
struct SpeciesInnates
{
    u16 species;
    enum Ability innates[MAX_INNATE_ABILITIES];
};

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // The Beldum line hovers via magnetic force, so it gains an innate Levitate on
    // top of its native ability (Clear Body / Light Metal). This realizes the
    // concept the TODO illustrates with "Flygon's innate Levitate" — Flygon and
    // Vibrava already carry Levitate as their real ability, so an innate there
    // would be a no-op; the Beldum line lacks native Levitate, so the innate is
    // actually observable (and is what test/battle/innate_abilities.c exercises).
    { SPECIES_BELDUM,    { ABILITY_LEVITATE } },
    { SPECIES_METANG,    { ABILITY_LEVITATE } },
    { SPECIES_METAGROSS, { ABILITY_LEVITATE } },

    // The Aggron line is a hulking steel dinosaur: it gains an innate Intimidate
    // (an *active*, on-switch-in ability) on top of its native Sturdy/Rock Head/
    // Heavy Metal, demonstrating that innates fire their entry effects, not just
    // passive trait checks.
    { SPECIES_ARON,    { ABILITY_INTIMIDATE } },
    { SPECIES_LAIRON,  { ABILITY_INTIMIDATE } },
    { SPECIES_AGGRON,  { ABILITY_INTIMIDATE } },
};

bool32 SpeciesHasInnate(u16 species, enum Ability ability)
{
    u32 i, slot;

    if (ability == ABILITY_NONE)
        return FALSE;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        if (sSpeciesInnates[i].species != species)
            continue;

        for (slot = 0; slot < MAX_INNATE_ABILITIES; slot++)
        {
            if (sSpeciesInnates[i].innates[slot] == ability)
                return TRUE;
        }
        return FALSE; // species matched; it simply doesn't have this innate
    }

    return FALSE;
}

enum Ability GetSpeciesInnate(u16 species, u32 index)
{
    u32 i;

    if (index >= MAX_INNATE_ABILITIES)
        return ABILITY_NONE;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        if (sSpeciesInnates[i].species == species)
            return sSpeciesInnates[i].innates[index];
    }

    return ABILITY_NONE;
}
