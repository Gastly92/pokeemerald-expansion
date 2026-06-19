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
// EVERY row must be justified by an *implemented* innate (a :white_check_mark: in
// fork-docs/INNATE_ABILITIES_PROGRESS.md). The override only makes sense because the slot it
// replaces is redundant *now that the ability is granted innately* — so before adding/keeping a
// row, cross-reference the freeing ability against the progress doc. If that ability is still
// PENDING as an innate (:white_large_square: / :x:), the slot is NOT actually redundant yet and the
// row doesn't belong here: the species would just lose a real ability for nothing. (The *chosen*
// ability the row hands out has no such constraint — it is a normal real ability, regardless of its
// own innate status.) The freeing innate is noted in each row's comment.
static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    { SPECIES_VENUSAUR,            1, ABILITY_THICK_FAT },     // 3 Overgrow + Chlorophyll BOTH innate (weather-doubler pinch case): empty slot 1 → its Mega's signature Thick Fat
    { SPECIES_CELEBI,              1, ABILITY_TRIAGE },        // 251 sole-Natural-Cure (innate): empty slot 1 → forest life-energy priority on its Giga Drain / Recover
    { SPECIES_SCEPTILE,            2, ABILITY_LIGHTNING_ROD }, // 254 Overgrow innate (latched); its HA Unburden is dead weight on the roster's non-consumable-item sets, so replace it with its Mega's signature Lightning Rod
    { SPECIES_FLYGON,              1, ABILITY_TOUGH_CLAWS },   // 330 innate Levitate; slot-1 Levitate was redundant → Tough Claws (still floats on the innate)
    { SPECIES_LUNATONE,            1, ABILITY_SOLID_ROCK },    // 337 ability-locked innate Levitate: empty slot 1 → chosen ability (floats on the innate, runs this too)
    { SPECIES_SOLROCK,             1, ABILITY_SOLID_ROCK },    // 338 "
    { SPECIES_CLAYDOL,             1, ABILITY_SOLID_ROCK },    // 344 "
    { SPECIES_CHIMECHO,            1, ABILITY_SOUNDPROOF },    // 358 "
    { SPECIES_LATIAS,              1, ABILITY_MAGIC_GUARD },   // 380 innate-Levitate legendary (deliberate Frontier buff)
    { SPECIES_LATIOS,              1, ABILITY_MAGIC_GUARD },   // 381 "
    { SPECIES_MISMAGIUS,           1, ABILITY_INFILTRATOR },   // 429 ability-locked innate Levitate
    { SPECIES_CARNIVINE,           1, ABILITY_CHLOROPHYLL },   // 455 "
    { SPECIES_TANGROWTH,           2, ABILITY_THICK_FAT },     // 465 innate Regenerator; slot 2 was Regenerator → Thick Fat (heal comes from the innate)
    { SPECIES_ROTOM,               1, ABILITY_LIGHTNING_ROD }, // 479 ability-locked innate Levitate
    { SPECIES_ROTOM_HEAT,          1, ABILITY_LIGHTNING_ROD }, // 479 "
    { SPECIES_ROTOM_WASH,          1, ABILITY_LIGHTNING_ROD }, // 479 "
    { SPECIES_ROTOM_FROST,         1, ABILITY_LIGHTNING_ROD }, // 479 "
    { SPECIES_ROTOM_FAN,           1, ABILITY_LIGHTNING_ROD }, // 479 "
    { SPECIES_ROTOM_MOW,           1, ABILITY_LIGHTNING_ROD }, // 479 "
    { SPECIES_UXIE,                1, ABILITY_MAGIC_GUARD },   // 480 innate-Levitate legendary (lake trio buff)
    { SPECIES_MESPRIT,             1, ABILITY_MAGIC_GUARD },   // 481 "
    { SPECIES_AZELF,               1, ABILITY_MAGIC_GUARD },   // 482 "
    { SPECIES_GIRATINA_ORIGIN,     1, ABILITY_PRESSURE },      // 487 innate-Levitate legendary (Origin forme floats)
    { SPECIES_CRESSELIA,           1, ABILITY_MULTISCALE },    // 488 innate-Levitate legendary (deliberate Frontier buff)
    { SPECIES_SHAYMIN,             1, ABILITY_SERENE_GRACE },  // 492 sole-Natural-Cure (innate): empty slot 1 → its Sky forme's ability (doubles Seed Flare's SpD drop)
    { SPECIES_AUDINO,              1, ABILITY_MAGIC_GUARD },   // 531 innate Regenerator; slot 1 was Regenerator → Magic Guard
    { SPECIES_ALOMOMOLA,           2, ABILITY_WATER_ABSORB },  // 594 innate Regenerator; slot 2 was Regenerator → Water Absorb
    { SPECIES_EELEKTROSS,          1, ABILITY_LIGHTNING_ROD }, // 604 ability-locked innate Levitate
    { SPECIES_CRYOGONAL,           1, ABILITY_ICE_BODY },      // 615 "
    { SPECIES_HYDREIGON,           1, ABILITY_SHEER_FORCE },   // 635 "
    { SPECIES_TORNADUS_THERIAN,    1, ABILITY_PRANKSTER },     // 641 sole-Regenerator (innate): empty slot 1 → its Incarnate forme's Prankster
    { SPECIES_VIKAVOLT,            1, ABILITY_COMPOUND_EYES },  // 738 ability-locked innate Levitate
    { SPECIES_SKELEDIRGE,          1, ABILITY_CURSED_BODY },   // 911 Blaze + Unaware BOTH innate (pinch case): empty slot 1 → Cursed Body, fitting its Fire/Ghost "singer" theme
    { SPECIES_OGERPON_CORNERSTONE, 1, ABILITY_DEFIANT },       // 1017 sole-Sturdy (innate): empty slot 1 → Ogerpon's signature Defiant (endures on the innate Sturdy regardless)
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
