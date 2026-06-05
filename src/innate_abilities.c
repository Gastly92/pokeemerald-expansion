#include "global.h"
#include "innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species->innate table (FEATURE_INNATE_ABILITIES). Kept here
// instead of in gSpeciesInfo so upstream syncs never touch it and the upstream
// species data stays untouched. Each row maps a species to an ABILITY_NONE-
// terminated list of innate abilities that are always active on top of that
// species' normal chosen ability. The list is variable-length (no fixed cap): add
// or remove entries freely, just keep the terminating ABILITY_NONE.
//
// ALLOWLIST — only abilities whose innate behavior has been deliberately wired in
// may appear here (see innate_abilities.h "SCOPE"). The fork grows this set one
// ability at a time. Supported innate abilities:
//   - ABILITY_LEVITATE — passive Ground-immunity / ungrounding, handled in
//     src/battle_util.c (IsBattlerUngroundedByAbilityItemOrEffect and the
//     type-effectiveness calc credit an innate Levitate exactly like the real one).
// Do NOT give a species an innate that is not on this list: nothing would honor it
// (no effect site activates it), so it would silently do nothing.

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// The Beldum line hovers via magnetic force, so it gains an innate Levitate on top
// of its native ability (Clear Body / Light Metal). Flygon and Vibrava already
// carry Levitate as their real ability, so an innate there would be a no-op; the
// Beldum line lacks native Levitate, so the innate is actually observable (and is
// what test/battle/innate_abilities.c exercises).
static const enum Ability sBeldumLineInnates[] = { ABILITY_LEVITATE, ABILITY_NONE };

static const struct SpeciesInnates sSpeciesInnates[] =
{
    { SPECIES_BELDUM,    sBeldumLineInnates },
    { SPECIES_METANG,    sBeldumLineInnates },
    { SPECIES_METAGROSS, sBeldumLineInnates },
};

static const enum Ability *GetSpeciesInnateList(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        if (sSpeciesInnates[i].species == species)
            return sSpeciesInnates[i].innates;
    }

    return NULL;
}

bool32 SpeciesHasInnate(u16 species, enum Ability ability)
{
    const enum Ability *list;
    u32 i;

    if (ability == ABILITY_NONE)
        return FALSE;

    list = GetSpeciesInnateList(species);
    if (list == NULL)
        return FALSE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (list[i] == ability)
            return TRUE;
    }

    return FALSE;
}

enum Ability GetSpeciesInnate(u16 species, u32 index)
{
    const enum Ability *list = GetSpeciesInnateList(species);
    u32 i;

    if (list == NULL)
        return ABILITY_NONE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (i == index)
            return list[i];
    }

    return ABILITY_NONE;
}
