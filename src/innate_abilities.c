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
// of its native ability (Clear Body / Light Metal) — this is the *observable* case
// the tests exercise, since those species lack a native Levitate.
//
// Every other entry below is a species that *already* carries Levitate as a real
// ability in the default config (any ability slot). Listing them here is intentional
// and currently redundant (a mon with primary == innate Levitate just passes the same
// `ability == LEVITATE || IsInnateActive(...)` short-circuit once — no double pop-up,
// no double recording). It is forward-looking: a later FEATURE_ flag will re-home
// these species onto a *different* primary ability, at which point this table becomes
// the live source of their Levitate. Keeping the list comprehensive now means that
// migration only has to touch gSpeciesInfo, not this file.
//
// NOTE: under the default config every natural-Levitate species carries it in slot 0
// (no mon has Levitate *only* in slot 1 / hidden). The Gengar line is a special case:
// at P_UPDATED_ABILITIES == GEN_LATEST its primary is Cursed Body, not Levitate, but it
// floats, so it's listed here as an *observable* innate (like the Beldum line). Gengar's
// MEGA form is deliberately omitted — Mega Gengar sinks into the ground (Shadow Tag), so
// it should stay grounded.
static const enum Ability sInnateLevitate[] = { ABILITY_LEVITATE, ABILITY_NONE };

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // Beldum line — innate-only (native ability is Clear Body / Light Metal).
    { SPECIES_BELDUM,            sInnateLevitate },
    { SPECIES_METANG,            sInnateLevitate },
    { SPECIES_METAGROSS,         sInnateLevitate },

    // Gen 1
    { SPECIES_GASTLY,            sInnateLevitate },
    { SPECIES_HAUNTER,           sInnateLevitate },
    { SPECIES_GENGAR,            sInnateLevitate }, // floats; primary is Cursed Body at GEN_LATEST, so observable
    { SPECIES_GENGAR_GMAX,       sInnateLevitate }, // a Gigantamaxed Gengar still floats (NOT Gengar-Mega, which is grounded)
    { SPECIES_KOFFING,           sInnateLevitate },
    { SPECIES_WEEZING,           sInnateLevitate },
    { SPECIES_WEEZING_GALAR,     sInnateLevitate },

    // Gen 2
    { SPECIES_MISDREAVUS,        sInnateLevitate },
    { SPECIES_MISMAGIUS,         sInnateLevitate },
    { SPECIES_UNOWN,             sInnateLevitate },
    { SPECIES_UNOWN_B,           sInnateLevitate },
    { SPECIES_UNOWN_C,           sInnateLevitate },
    { SPECIES_UNOWN_D,           sInnateLevitate },
    { SPECIES_UNOWN_E,           sInnateLevitate },
    { SPECIES_UNOWN_F,           sInnateLevitate },
    { SPECIES_UNOWN_G,           sInnateLevitate },
    { SPECIES_UNOWN_H,           sInnateLevitate },
    { SPECIES_UNOWN_I,           sInnateLevitate },
    { SPECIES_UNOWN_J,           sInnateLevitate },
    { SPECIES_UNOWN_K,           sInnateLevitate },
    { SPECIES_UNOWN_L,           sInnateLevitate },
    { SPECIES_UNOWN_M,           sInnateLevitate },
    { SPECIES_UNOWN_N,           sInnateLevitate },
    { SPECIES_UNOWN_O,           sInnateLevitate },
    { SPECIES_UNOWN_P,           sInnateLevitate },
    { SPECIES_UNOWN_Q,           sInnateLevitate },
    { SPECIES_UNOWN_R,           sInnateLevitate },
    { SPECIES_UNOWN_S,           sInnateLevitate },
    { SPECIES_UNOWN_T,           sInnateLevitate },
    { SPECIES_UNOWN_U,           sInnateLevitate },
    { SPECIES_UNOWN_V,           sInnateLevitate },
    { SPECIES_UNOWN_W,           sInnateLevitate },
    { SPECIES_UNOWN_X,           sInnateLevitate },
    { SPECIES_UNOWN_Y,           sInnateLevitate },
    { SPECIES_UNOWN_Z,           sInnateLevitate },
    { SPECIES_UNOWN_EXCLAMATION, sInnateLevitate },
    { SPECIES_UNOWN_QUESTION,    sInnateLevitate },

    // Gen 3
    { SPECIES_VIBRAVA,           sInnateLevitate },
    { SPECIES_FLYGON,            sInnateLevitate },
    { SPECIES_LUNATONE,          sInnateLevitate },
    { SPECIES_SOLROCK,           sInnateLevitate },
    { SPECIES_BALTOY,            sInnateLevitate },
    { SPECIES_CLAYDOL,           sInnateLevitate },
    { SPECIES_DUSKULL,           sInnateLevitate },
    { SPECIES_CHINGLING,         sInnateLevitate },
    { SPECIES_CHIMECHO,          sInnateLevitate },
    { SPECIES_CHIMECHO_MEGA,     sInnateLevitate },
    { SPECIES_LATIAS,            sInnateLevitate },
    { SPECIES_LATIAS_MEGA,       sInnateLevitate },
    { SPECIES_LATIOS,            sInnateLevitate },
    { SPECIES_LATIOS_MEGA,       sInnateLevitate },

    // Gen 4
    { SPECIES_BRONZOR,           sInnateLevitate },
    { SPECIES_BRONZONG,          sInnateLevitate },
    { SPECIES_CARNIVINE,         sInnateLevitate },
    { SPECIES_ROTOM,             sInnateLevitate },
    { SPECIES_ROTOM_HEAT,        sInnateLevitate },
    { SPECIES_ROTOM_WASH,        sInnateLevitate },
    { SPECIES_ROTOM_FROST,       sInnateLevitate },
    { SPECIES_ROTOM_FAN,         sInnateLevitate },
    { SPECIES_ROTOM_MOW,         sInnateLevitate },
    { SPECIES_UXIE,              sInnateLevitate },
    { SPECIES_MESPRIT,           sInnateLevitate },
    { SPECIES_AZELF,             sInnateLevitate },
    { SPECIES_GIRATINA_ORIGIN,   sInnateLevitate },
    { SPECIES_CRESSELIA,         sInnateLevitate },

    // Gen 5
    { SPECIES_TYNAMO,            sInnateLevitate },
    { SPECIES_EELEKTRIK,         sInnateLevitate },
    { SPECIES_EELEKTROSS,        sInnateLevitate },
    { SPECIES_EELEKTROSS_MEGA,   sInnateLevitate },
    { SPECIES_CRYOGONAL,         sInnateLevitate },
    { SPECIES_HYDREIGON,         sInnateLevitate },

    // Gen 6
    { SPECIES_DELPHOX_MEGA,      sInnateLevitate },

    // Gen 7
    { SPECIES_VIKAVOLT,          sInnateLevitate },
    { SPECIES_VIKAVOLT_TOTEM,    sInnateLevitate },
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
