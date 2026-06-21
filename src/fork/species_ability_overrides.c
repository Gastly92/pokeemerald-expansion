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
    { SPECIES_CHIMECHO,            1, ABILITY_SOUNDPROOF },     // 358 ability-locked innate Levitate: empty slot 1 → Soundproof
    { SPECIES_LATIAS,              1, ABILITY_ILLUSION },       // 380 innate-Levitate legendary: empty slot 1 → Illusion, the Eon refracts light to vanish/disguise
    { SPECIES_LATIOS,              1, ABILITY_ILLUSION },       // 381 innate-Levitate legendary: empty slot 1 → Illusion (light-bending Eon)
    { SPECIES_DEOXYS_ATTACK,       1, ABILITY_TRACE },          // 386 sole-Pressure (innate; also innate Levitate): empty slot 1 → Trace, the DNA alien adapts by copying the foe's ability
    { SPECIES_DEOXYS_DEFENSE,      1, ABILITY_TRACE },          // 386 "
    { SPECIES_DEOXYS_SPEED,        1, ABILITY_TRACE },          // 386 "
    { SPECIES_MISMAGIUS,           1, ABILITY_WANDERING_SPIRIT },// 429 ability-locked innate Levitate: empty slot 1 → Wandering Spirit, the roaming magical ghost swaps abilities on contact
    { SPECIES_CARNIVINE,           1, ABILITY_CHLOROPHYLL },    // 455 ability-locked innate Levitate: empty slot 1 → Chlorophyll
    { SPECIES_TANGROWTH,           2, ABILITY_SAP_SIPPER },     // 465 innate Regenerator; slot 2 was Regenerator → Sap Sipper, its vine tangle drinks Grass energy for +Atk (heal comes from the innate)
    { SPECIES_ROTOM,               1, ABILITY_LIGHTNING_ROD },  // 479 ability-locked innate Levitate
    { SPECIES_ROTOM_HEAT,          1, ABILITY_LIGHTNING_ROD },  // 479 "
    { SPECIES_ROTOM_WASH,          1, ABILITY_LIGHTNING_ROD },  // 479 "
    { SPECIES_ROTOM_FROST,         1, ABILITY_LIGHTNING_ROD },  // 479 "
    { SPECIES_ROTOM_FAN,           1, ABILITY_LIGHTNING_ROD },  // 479 "
    { SPECIES_ROTOM_MOW,           1, ABILITY_LIGHTNING_ROD },  // 479 "
    { SPECIES_UXIE,                1, ABILITY_TRACE },          // 480 innate-Levitate legendary: empty slot 1 → Trace, the Being of Knowledge reads/copies the foe
    { SPECIES_MESPRIT,             1, ABILITY_MOODY },          // 481 innate-Levitate legendary: empty slot 1 → Moody, the Being of Emotion's volatile moods
    { SPECIES_AZELF,               1, ABILITY_VICTORY_STAR },   // 482 innate-Levitate legendary: empty slot 1 → Victory Star, the Being of Willpower's will to win
    { SPECIES_GIRATINA_ORIGIN,     1, ABILITY_DRAGONS_MAW },    // 487 innate-Levitate legendary (Origin forme floats): empty slot 1 → Dragon's Maw, the Renegade's draconic might
    { SPECIES_CRESSELIA,           1, ABILITY_CLOUD_NINE },     // 488 innate-Levitate legendary: empty slot 1 → Cloud Nine, the serene lunar presence stills the weather
    { SPECIES_SHAYMIN,             1, ABILITY_EFFECT_SPORE },   // 492 sole-Natural-Cure (innate): empty slot 1 → Effect Spore, the flowery Gratitude hedgehog scatters spores (Sky forme keeps its real Serene Grace)
    { SPECIES_AUDINO,              1, ABILITY_CUTE_CHARM },     // 531 innate Regenerator; slot 1 was Regenerator → Cute Charm, the gentle nurse (heal comes from the innate)
    { SPECIES_ALOMOMOLA,           2, ABILITY_WATER_ABSORB },   // 594 innate Regenerator; slot 2 was Regenerator → Water Absorb
    { SPECIES_EELEKTROSS,          1, ABILITY_LIGHTNING_ROD },  // 604 ability-locked innate Levitate
    { SPECIES_CRYOGONAL,           1, ABILITY_SNOW_WARNING },   // 615 ability-locked innate Levitate: empty slot 1 → Snow Warning, the ice-crystal being radiates snow (Ice-type: +Def in snow, no chip)
    { SPECIES_HYDREIGON,           1, ABILITY_SHEER_FORCE },    // 635 ability-locked innate Levitate
    { SPECIES_TORNADUS_THERIAN,    1, ABILITY_PRANKSTER },      // 641 sole-Regenerator (innate): empty slot 1 → its Incarnate forme's Prankster
    { SPECIES_KYUREM,              1, ABILITY_SNOW_WARNING },   // 646 sole-Pressure (innate): empty slot 1 → Snow Warning, the boundary ice dragon brings an everlasting freeze
    { SPECIES_VIKAVOLT,            1, ABILITY_MOTOR_DRIVE },    // 738 ability-locked innate Levitate: empty slot 1 → Motor Drive, the electromagnetic beetle banks electricity into Speed
    { SPECIES_ETERNATUS,           1, ABILITY_POISON_TOUCH },   // 890 sole-Pressure (innate): empty slot 1 → Poison Touch, the toxic alien dragon poisons on contact
    { SPECIES_SKELEDIRGE,          1, ABILITY_MUMMY },          // 911 Blaze + Unaware BOTH innate (pinch case): empty slot 1 → Mummy, the ghostly fire-singer's curse spreads on contact
    { SPECIES_OGERPON_CORNERSTONE, 1, ABILITY_EARTH_EATER },   // 1017 sole-Sturdy (innate): empty slot 1 → Earth Eater (the stone-masked ogre is nourished by the earth; Ground immunity+heal covers its Rock weakness)
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
