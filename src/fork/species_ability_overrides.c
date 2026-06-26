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
    { // 0028
        // Sandslash's only real abilities (Sand Veil, Sand Rush) are BOTH now innate, so its empty
        // slot 1 takes a flavorful chosen ability. Sand Stream is :x: (never an innate -> stable) and
        // self-synergistic: it sets the sandstorm that turns on Sandslash's own innate Sand Rush (Speed)
        // and Sand Veil (evasion). Precedent: Flygon/Claydol/Torterra also hand out Sand Stream.
        SPECIES_SANDSLASH, 1,
        ABILITY_SAND_STREAM
    },
    { // 0028
        // Sandslash-Alola's only real abilities (Snow Cloak, Slush Rush) are BOTH now innate, so its
        // empty slot 1 takes Snow Warning — :x: (never an innate -> stable) and self-synergistic: the
        // snow it sets turns on its own innate Slush Rush (Speed) and Snow Cloak (evasion). Snowy
        // counterpart to base Sandslash's Sand Stream above.
        SPECIES_SANDSLASH_ALOLA, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0144
        // Articuno's only real abilities (Pressure, Snow Cloak) are BOTH now innate, so its empty
        // slot 1 takes Snow Warning — :x: (never an innate -> stable) and flavorful (the legendary ice
        // bird heralds the blizzard), setting the snow that turns on its own innate Snow Cloak evasion.
        SPECIES_ARTICUNO, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0227
        // Skarmory's only non-drawback real abilities (Keen Eye, Sturdy) are BOTH now innate (Weak Armor,
        // slot 2, is a drawback on a wall), so its slot-1 Sturdy — now innate-redundant — takes Bulletproof.
        // Bulletproof is :x: (never an innate -> stable) and a clean defensive boon: this Spikes/Defog wall
        // shrugs off Focus Blast, Sludge Bomb, Energy Ball and the other ball/bomb moves that threaten it.
        SPECIES_SKARMORY, 1,
        ABILITY_BULLETPROOF
    },
    { // 0232
        // Donphan's only real abilities (Sturdy, Sand Veil) are BOTH now innate, so its empty slot 1
        // takes Sand Stream — :x: (never an innate -> stable) and self-synergistic: the sandstorm it
        // sets turns on its own innate Sand Veil evasion. Same pick as base Sandslash above.
        SPECIES_DONPHAN, 1,
        ABILITY_SAND_STREAM
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
    { // 0257
        // Blaziken's only real abilities (Blaze, Speed Boost) are BOTH now innate, so its empty
        // slot 1 takes a flavorful chosen ability for the frontier sets. Sheer Force is :x:
        // (never an innate -> stable) and complements its snowballing physical wallbreaker sets
        // (Close Combat / Blaze Kick / Thunder Punch all gain the +30% and drop their secondaries).
        SPECIES_BLAZIKEN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0313
        // Volbeat's three real abilities (Illuminate, Swarm, Prankster) are ALL now innate, so its slot-1
        // Swarm — now innate-redundant — takes Victory Star. Victory Star is :x: (never an innate -> stable)
        // and flavorful for the firefly's guiding light: it boosts its doubles allies' accuracy. (Azelf also
        // hands out Victory Star.)
        SPECIES_VOLBEAT, 1,
        ABILITY_VICTORY_STAR
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
    { // 0428
        // Lopunny's only real non-drawback abilities (Cute Charm, Limber) are BOTH now innate
        // (Klutz, slot 1, is a drawback), so its slot-2 Limber — now innate-redundant — takes a
        // flavorful chosen ability for the frontier set. Sheer Force is :x: (never an innate ->
        // stable) and a pure boon for its offensive Fake Out / Ice Punch breaker set.
        SPECIES_LOPUNNY, 2,
        ABILITY_SHEER_FORCE
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
    { // 0473
        // Mamoswine's three real abilities (Oblivious, Snow Cloak, Thick Fat) are ALL now innate, so
        // its slot-2 Thick Fat — now innate-redundant — is repurposed to Snow Warning. Snow Warning is
        // :x: (never an innate -> stable) and self-synergistic: the snow the prehistoric mammoth heralds
        // turns on its own innate Snow Cloak evasion. Same pick as Beartic/Articuno above.
        SPECIES_MAMOSWINE, 2,
        ABILITY_SNOW_WARNING
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
    { // 0614
        // Beartic's three real abilities (Snow Cloak, Slush Rush, Swift Swim) are ALL now innate, so
        // its slot-1 Slush Rush — now innate-redundant — is repurposed to Snow Warning, :x: (never an
        // innate -> stable) and self-synergistic: the snow it sets turns on its own innate Slush Rush
        // (Speed) and Snow Cloak (evasion).
        SPECIES_BEARTIC, 1,
        ABILITY_SNOW_WARNING
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
    { // 0685
        // Slurpuff's only real abilities are Sweet Veil (slot 0, now innate) and Unburden (slot 2, dead
        // weight on its non-consumable Leftovers wall set), so its empty slot 1 takes Unaware — an
        // already-implemented :white_check_mark: innate (stable, like Carnivine/Tornadus-Therian) and a pure
        // boon for its Calm Mind wall: it ignores the foe's stat boosts. (Slot 2 Unburden is left intact for
        // any future consumable-item set.)
        SPECIES_SLURPUFF, 1,
        ABILITY_UNAWARE
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
    { // 0952
        // Scovillain's only non-drawback real abilities (Chlorophyll, Insomnia) are BOTH now innate (Klutz,
        // slot 2, is a drawback that disables its Life Orb), so its slot-2 Klutz is repurposed to Sheer Force.
        // Sheer Force is :x: (never an innate -> stable) and a strong boon for its mixed sun attacker set:
        // Flamethrower / Earth Power gain +30% and drop their secondaries (and skip Life Orb recoil too).
        SPECIES_SCOVILLAIN, 2,
        ABILITY_SHEER_FORCE
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
