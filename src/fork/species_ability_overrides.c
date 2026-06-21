#include "global.h"
#include "fork/species_ability_overrides.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species ability overrides (sibling to src/innate_abilities.c).
// See include/species_ability_overrides.h for the full rationale. In short: this
// gives a small set of innate-Levitate/Regenerator species a real, selectable
// SECOND ability so a Battle Factory set can run it alongside the innate, without
// editing upstream gSpeciesInfo. GetSpeciesAbility() (src/pokemon.c) consults this
// table first; where no row matches, the upstream ability data is used unchanged.
//
// Each row replaces ability SLOT `slot` for `species` with `ability`. The replaced
// slot is usually either empty in the upstream data (a free normal slot on an
// ability-locked species) or holds an ability that is redundant because it is now
// granted innately (e.g. the Regenerator slot on Tangrowth/Audino/Alomomola). One
// row (Sceptile) instead replaces a real Hidden Ability — its Overgrow is innately
// latched, so a frontier slot is free, and its HA (Unburden) is dead weight on the
// roster's non-consumable-item sets, so the slot is repurposed for a flavorful pick.
// The roster test test/frontier_extended_roster.c verifies every set's chosen ability
// resolves to a real slot through this hook, so an out-of-place row fails CI.

struct SpeciesAbilityOverride
{
    u16 species;
    u8 slot;             // ability slot (0..NUM_ABILITY_SLOTS-1) this row replaces
    enum Ability ability;
};

// Sorted by National Pokédex number (shown in each row's trailing comment); formes share their
// base's number and follow it. Adding a row: drop it at its dex position with a trailing `// <dex>`.
//
// PICK A STABLE CHOSEN ABILITY — cross-reference it against fork-docs/INNATE_ABILITIES_PROGRESS.md.
// Prefer an ability that will NEVER be wired as an innate (marked :x: there — e.g. Lightning Rod,
// Soundproof, Water Absorb, Sheer Force) over one still PENDING (:white_large_square:). A pending
// ability is on track to become an innate, and the moment it is, the Step 3.5 sweep
// (INNATE_ABILITIES.md) has to revisit every set and override that hands it out — so a
// :white_large_square: pick is future churn baked in, while a :x: pick is stable for good. Sceptile's
// LIGHTNING_ROD is the model. (Separately, the slot a row *frees* must already be redundant via an
// *implemented* :white_check_mark: innate — that's the row's whole premise; noted in each comment.)
// The table below was audited on this rule: every row hands out a :x: (never-an-innate) ability,
// except CARNIVINE and TORNADUS_THERIAN, whose picks (Chlorophyll, Prankster) are already
// *implemented* :white_check_mark: innates and so are likewise stable.
static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    { // 0003
        SPECIES_VENUSAUR, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0250
        SPECIES_HO_OH, 1,
        ABILITY_FLAME_BODY
    },
    { // 0251
        SPECIES_CELEBI, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0254
        SPECIES_SCEPTILE, 2,
        ABILITY_LIGHTNING_ROD
    },
    { // 0330
        SPECIES_FLYGON, 1, 
        ABILITY_SAND_STREAM
    },
    { // 0337
        SPECIES_LUNATONE, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0338
        SPECIES_SOLROCK, 1,
        ABILITY_DROUGHT
    },
    { // 0344
        SPECIES_CLAYDOL, 1, 
        ABILITY_SAND_STREAM
    },
    { // 0348
        SPECIES_ARMALDO, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0358
        SPECIES_CHIMECHO, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0380
        SPECIES_LATIAS, 1,
        ABILITY_ILLUSION
    },
    { // 0381
        SPECIES_LATIOS, 1,
        ABILITY_ILLUSION
    },
    { // 0386
        SPECIES_DEOXYS_ATTACK, 1,
        ABILITY_TRACE
    },
    { // 0386
        SPECIES_DEOXYS_DEFENSE, 1,
        ABILITY_TRACE
    },
    { // 0386
        SPECIES_DEOXYS_SPEED, 1, 
        ABILITY_TRACE
    },
    { // 0389
        SPECIES_TORTERRA, 1,
        ABILITY_SAND_STREAM
    },
    { // 0429
        SPECIES_MISMAGIUS, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0455
        SPECIES_CARNIVINE, 1, 
        ABILITY_CHLOROPHYLL
    },
    { // 0465
        SPECIES_TANGROWTH, 2,
        ABILITY_SAP_SIPPER
    },
    { // 0479
        SPECIES_ROTOM, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_HEAT, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_WASH, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_FROST, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_FAN, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_MOW, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0480
        SPECIES_UXIE, 1,
        ABILITY_TRACE
    },
    { // 0481
        SPECIES_MESPRIT, 1,
        ABILITY_MOODY
    },
    { // 0482
        SPECIES_AZELF, 1,
        ABILITY_VICTORY_STAR
    },
    { // 0487
        SPECIES_GIRATINA_ORIGIN, 1, 
        ABILITY_DRAGONS_MAW
    },
    { // 0488
        SPECIES_CRESSELIA, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0492
        SPECIES_SHAYMIN, 1,
        ABILITY_EFFECT_SPORE
    },
    { // 0503
        SPECIES_SAMUROTT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0531
        SPECIES_AUDINO, 1,
        ABILITY_CUTE_CHARM
    },
    { // 0594
        SPECIES_ALOMOMOLA, 2, 
        ABILITY_WATER_ABSORB
    },
    { // 0604
        SPECIES_EELEKTROSS, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0615
        SPECIES_CRYOGONAL, 1, 
        ABILITY_SNOW_WARNING
    },
    { // 0635
        SPECIES_HYDREIGON, 1, 
        ABILITY_SHEER_FORCE },
    { // 0641
        SPECIES_TORNADUS_THERIAN, 1, 
        ABILITY_PRANKSTER
    },
    { // 0646
        SPECIES_KYUREM, 1, 
        ABILITY_SNOW_WARNING
    },
    { // 0738
        SPECIES_VIKAVOLT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0776
        SPECIES_TURTONATOR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0890
        SPECIES_ETERNATUS, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0911
        SPECIES_SKELEDIRGE, 1,
        ABILITY_MUMMY
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE, 1, 
        ABILITY_EARTH_EATER
    },
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
