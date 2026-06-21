#include "global.h"
#include "fork/new_types.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// FORK: fork-owned species->types override table (FEATURE_NEW_TYPES). Kept here
// instead of in gSpeciesInfo so upstream syncs never touch it and the upstream
// species data stays untouched. Each row maps a species to its two replacement
// types (slot 0 and slot 1); a pure (single-type) species uses the same type in
// both slots, mirroring how gSpeciesInfo stores types via MON_TYPES().
//
// The override is consulted by GetSpeciesType() (src/pokemon.c), the single
// canonical type accessor, so a re-typing here applies everywhere: battle type
// matchups/STAB, the type icons, the Pokédex, the summary screen, and
// IsSpeciesOfType(). To add a species: add a row below. Forms are independent
// species, so each form (base / regional) needs its own row.

struct SpeciesTypeOverride
{
    u16 species;
    enum Type types[2]; // slot 0, slot 1 (equal for a pure type)
};

static const struct SpeciesTypeOverride sSpeciesTypeOverrides[] =
{
    { // 0077
        SPECIES_PONYTA_GALAR,
        { TYPE_FIRE, TYPE_FAIRY }
    },
    { // 0078
        SPECIES_RAPIDASH_GALAR,
        { TYPE_FIRE, TYPE_FAIRY }
    },
};

bool32 GetSpeciesTypeOverride(enum Species species, u8 slot, enum Type *outType)
{
    u32 i;

    if (slot > 1)
        return FALSE;

    for (i = 0; i < ARRAY_COUNT(sSpeciesTypeOverrides); i++)
    {
        if (sSpeciesTypeOverrides[i].species == species)
        {
            *outType = sSpeciesTypeOverrides[i].types[slot];
            return TRUE;
        }
    }

    return FALSE;
}
