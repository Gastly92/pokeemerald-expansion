#include "global.h"
#include "species_ability_overrides.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species ability overrides (sibling to src/innate_abilities.c).
// See include/species_ability_overrides.h for the full rationale. In short: this
// gives a small set of innate-Levitate/Regenerator species a real, selectable
// SECOND ability so a Battle Factory set can run it alongside the innate, without
// editing upstream gSpeciesInfo. GetSpeciesAbility() (src/pokemon.c) consults this
// table first; where no row matches, the upstream ability data is used unchanged.
//
// Each row replaces ability SLOT `slot` for `species` with `ability`. The chosen
// slot is always either empty in the upstream data (a free normal slot on an
// ability-locked species) or holds an ability that is redundant because it is now
// granted innately (e.g. the Regenerator slot on Tangrowth/Audino/Alomomola). The
// roster test test/frontier_extended_roster.c verifies every set's chosen ability
// resolves to a real slot through this hook, so an out-of-place row fails CI.

struct SpeciesAbilityOverride
{
    u16 species;
    u8 slot;             // ability slot (0..NUM_ABILITY_SLOTS-1) this row replaces
    enum Ability ability;
};

static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    // ── Innate Levitate, ability-locked: add a chosen ability in the empty slot 1.
    //    The mon floats on its innate Levitate and also runs the ability below.
    { SPECIES_LUNATONE,        1, ABILITY_SOLID_ROCK },
    { SPECIES_SOLROCK,         1, ABILITY_SOLID_ROCK },
    { SPECIES_CLAYDOL,         1, ABILITY_SOLID_ROCK },
    { SPECIES_CHIMECHO,        1, ABILITY_SOUNDPROOF },
    { SPECIES_MISMAGIUS,       1, ABILITY_INFILTRATOR },
    { SPECIES_CARNIVINE,       1, ABILITY_CHLOROPHYLL },
    { SPECIES_CRYOGONAL,       1, ABILITY_ICE_BODY },
    { SPECIES_VIKAVOLT,        1, ABILITY_COMPOUND_EYES },
    { SPECIES_EELEKTROSS,      1, ABILITY_LIGHTNING_ROD },
    { SPECIES_HYDREIGON,       1, ABILITY_SHEER_FORCE },
    { SPECIES_ROTOM,           1, ABILITY_LIGHTNING_ROD },
    { SPECIES_ROTOM_HEAT,      1, ABILITY_LIGHTNING_ROD },
    { SPECIES_ROTOM_WASH,      1, ABILITY_LIGHTNING_ROD },
    { SPECIES_ROTOM_FROST,     1, ABILITY_LIGHTNING_ROD },
    { SPECIES_ROTOM_FAN,       1, ABILITY_LIGHTNING_ROD },
    { SPECIES_ROTOM_MOW,       1, ABILITY_LIGHTNING_ROD },

    // ── Innate Levitate legendaries (strong, deliberate Frontier buffs).
    { SPECIES_LATIAS,          1, ABILITY_MAGIC_GUARD },
    { SPECIES_LATIOS,          1, ABILITY_MAGIC_GUARD },
    { SPECIES_UXIE,            1, ABILITY_MAGIC_GUARD },
    { SPECIES_MESPRIT,         1, ABILITY_MAGIC_GUARD },
    { SPECIES_AZELF,           1, ABILITY_MAGIC_GUARD },
    { SPECIES_CRESSELIA,       1, ABILITY_MULTISCALE },
    { SPECIES_GIRATINA_ORIGIN, 1, ABILITY_PRESSURE },

    // ── Innate Levitate, slots already full of (redundant) Levitate: replace the
    //    redundant slot-1 Levitate. The mon still floats on the innate.
    { SPECIES_FLYGON,          1, ABILITY_TOUGH_CLAWS },

    // ── Innate Regenerator, slots full: replace the now-redundant Regenerator slot
    //    (the heal comes from the innate). The chosen ability runs alongside it.
    { SPECIES_TANGROWTH,       2, ABILITY_THICK_FAT },    // slot 2 was Regenerator
    { SPECIES_AUDINO,          1, ABILITY_MAGIC_GUARD },  // slot 1 was Regenerator
    { SPECIES_ALOMOMOLA,       2, ABILITY_WATER_ABSORB }, // slot 2 was Regenerator
    { SPECIES_TORNADUS_THERIAN, 1, ABILITY_PRANKSTER },   // slot 1 was empty

    // ── Innate Sturdy, ability-locked: Cornerstone Ogerpon's ONLY ability is Sturdy, now granted
    //    innately, so its frontier set would otherwise waste its slot on the redundant Sturdy. Give
    //    it a chosen Defiant in the empty slot 1 — Ogerpon's signature ability, and a fitting +2 Atk
    //    punish for the Cornerstone Swords-Dance sweeper. It endures on the innate Sturdy regardless.
    { SPECIES_OGERPON_CORNERSTONE, 1, ABILITY_DEFIANT },
};

enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
    {
        if (sSpeciesAbilityOverrides[i].species == species
         && sSpeciesAbilityOverrides[i].slot == slot)
            return sSpeciesAbilityOverrides[i].ability;
    }

    return ABILITY_NONE;
}
