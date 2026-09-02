#include "global.h"
#include "fork/species_ability_overrides.h"
#include "config_changes.h" // FORK: GetConfig(FEATURE_INNATE_ABILITIES) gates the override table
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
//
// GATED BY FEATURE_INNATE_ABILITIES (see GetSpeciesAbilityOverride below). Like innates and
// the fork's other runtime features, the table is FORCE-DISABLED in tests by default
// (TestInitConfigData), so upstream battle-ability tests see VANILLA slots and a row can no
// longer perturb them; a fork test that wants an override opts in with WITH_CONFIG. This is
// what lets a row repurpose a slot that an *upstream* test pins via Ability(...) or a default
// read -- that test never sees the override. Only *fork* tests that opt into the flag observe
// a repurposed slot, so the "SLOT CHOICE MATTERS" audit below now only needs to clear
// flag-on fork tests (the innate test, the frontier roster), not the whole upstream suite.

struct SpeciesAbilityOverride
{
    u16 species;
    u8 slot;             // ability slot (0..NUM_ABILITY_SLOTS-1) this row replaces
    enum Ability ability;
};

// Sorted by National Pokédex number (shown in each row's trailing comment); formes share their
// base's number and follow it. Adding a row: drop it at its dex position with a trailing `// <dex>`.
//
// PICK A NEVER-AN-INNATE CHOSEN ABILITY — cross-reference it against sImplementedInnates[] in
// test/fork/innate_abilities.c, the single source of truth. The ability MUST be ABSENT from that
// array (never wired as an innate — e.g. Lightning Rod, Soundproof, Water Absorb, Sheer Force). An
// innate-CAPABLE ability (one ON the array) is NOT a legal pick, even when this species
// does not currently carry it: an innate-capable ability belongs in an INNATES(...) row, where it is
// always-on and costs nothing, so spending the one observable slot on it both wastes that slot's
// only purpose (a trait the species can express no other way) and leaves a latent duplicate that
// collapses the moment a line review gives the species that ability innately. Sceptile's
// LIGHTNING_ROD is the model. (Separately, the slot a row *frees* must already be redundant via an
// *implemented* :white_check_mark: innate — that's the row's whole premise.)
//
// SOME ABILITIES ARE RESERVED TO ONE LINE and no row may name them, ILLUSION (Zorua/Zoroark) first.
// The never-an-innate rule above does not catch these -- Illusion is never-an-innate, so that gate is
// happy with it; what disqualifies it is identity. The disguise is that line's whole design, and the
// roster machinery assumes it (IllusionMonRejectsSlot in src/fork/frontier_draft.c, the INFO viewer's
// disguise-safe species read). Eight rows once borrowed it for their concealment flavor (Gengar,
// Sudowoodo, the Eon duo, Liepard, Marshadow, Grimmsnarl) and were re-pointed at ordinary picks.
// Enforced by TEST("Innate abilities: no ability override or frontier set names a reserved ability")
// in test/fork/innate_abilities.c; extend its sReservedAbilities[] to reserve another ability.
//
// MIGRATION COMPLETE — every row below conforms. The legacy rows that predated this rule (123 of
// them, roughly a third of the table) were converted in one sweep, and TEST("Innate abilities: no
// ability override or frontier set names an innate-capable ability") in
// test/fork/innate_abilities.c is no longer KNOWN_FAILING: it is a REAL CI GATE, so a row naming an
// innate-capable ability now fails the build. Run it after editing this table.
//
// SLOT CHOICE MATTERS — because the table is gated by FEATURE_INNATE_ABILITIES, a row REPLACES that
// slot's ability only where the flag is on: all real gameplay/frontier, plus any FORK test that opts
// in with WITH_CONFIG. Upstream tests run flag-off and see vanilla slots, so a row can now safely
// repurpose a slot that an upstream `Ability(ABILITY_X)` or default read pins. Filling an EMPTY slot
// (ABILITY_NONE) is always safe. Repurposing a REAL slot deletes that ability from the species in
// every flag-on context, so audit that no *fork* test with the flag on observes it: `Ability(ABILITY_X)`
// on the species in test/fork/ (chiefly the innate test) and the frontier roster. (Upstream test/battle/
// pins no longer matter for slot choice, but a repurposed slot still changes real gameplay, so keep the
// pick a redundant innate or empty slot as before.) The rows above that repurpose real slots (Sceptile,
// Lopunny, Bronzong, Mamoswine, Beartic, Carracosta, Scovillain, Sinistcha, Volbeat, Zangoose, Pidgeot,
// Chatot, Crawdaunt, Klinklang, Bombirdier) were each audited this way.
static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    { // 0003
        SPECIES_VENUSAUR, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 0,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 2,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_GMAX, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0006
        SPECIES_CHARIZARD, 1,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 0,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 1,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 2,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_GMAX, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0009
        SPECIES_BLASTOISE, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 0,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 2,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_GMAX, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0012
        SPECIES_BUTTERFREE, 1,
        ABILITY_PSYCHIC_AFFINITY
    },
    {
        SPECIES_BUTTERFREE_GMAX, 1,
        ABILITY_PSYCHIC_AFFINITY
    },
    { // 0015
        SPECIES_BEEDRILL, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 0,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0018
        SPECIES_PIDGEOT, 1,
        ABILITY_NO_GUARD
    },
    { // 0022
        SPECIES_FEAROW, 1,
        ABILITY_HUSTLE
    },
    { // 0024
        SPECIES_ARBOK, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0026
        SPECIES_RAICHU_ALOLA, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0028
        SPECIES_SANDSLASH, 1,
        ABILITY_SAND_STREAM
    },
    { // 0028
        SPECIES_SANDSLASH_ALOLA, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0036
        SPECIES_CLEFABLE, 1,
        ABILITY_MISTY_SURGE
    },
    {
        SPECIES_CLEFABLE_MEGA, 0,
        ABILITY_HALO
    },
    {
        SPECIES_CLEFABLE_MEGA, 1,
        ABILITY_HALO
    },
    {
        SPECIES_CLEFABLE_MEGA, 2,
        ABILITY_HALO
    },
    { // 0040
        SPECIES_WIGGLYTUFF, 0,
        ABILITY_HALO
    },
    {
        SPECIES_WIGGLYTUFF, 1,
        ABILITY_FLUFFY
    },
    { // 0045
        SPECIES_VILEPLUME, 1,
        ABILITY_POISON_POINT
    },
    { // 0049
        SPECIES_VENOMOTH, 2,
        ABILITY_PSYCHIC_AFFINITY
    },
    { // 0051
        SPECIES_DUGTRIO, 2,
        ABILITY_SAND_STREAM
    },
    { // 0051
        SPECIES_DUGTRIO_ALOLA, 0,
        ABILITY_EARTH_EATER
    },
    { // 0053
        SPECIES_PERSIAN, 2,
        ABILITY_TRACE
    },
    { // 0053
        SPECIES_PERSIAN_ALOLA, 2,
        ABILITY_DARK_AURA
    },
    { // 0055
        SPECIES_GOLDUCK, 2,
        ABILITY_PSYCHIC_AFFINITY
    },
    { // 0065
        SPECIES_ALAKAZAM, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0071
        SPECIES_VICTREEBEL, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_VICTREEBEL_MEGA, 0,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_VICTREEBEL_MEGA, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_VICTREEBEL_MEGA, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0073
        SPECIES_TENTACRUEL, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0076
        SPECIES_GOLEM, 2,
        ABILITY_SAND_STREAM
    },
    { // 0076
        SPECIES_GOLEM_ALOLA, 0,
        ABILITY_LIGHTNING_ROD
    },
    { // 0078
        SPECIES_RAPIDASH_GALAR, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0080
        SPECIES_SLOWBRO, 1,
        ABILITY_STORM_DRAIN
    },
    {
        SPECIES_SLOWBRO_MEGA, 0,
        ABILITY_STORM_DRAIN
    },
    {
        SPECIES_SLOWBRO_MEGA, 1,
        ABILITY_STORM_DRAIN
    },
    {
        SPECIES_SLOWBRO_MEGA, 2,
        ABILITY_STORM_DRAIN
    },
    { // 0080
        SPECIES_SLOWBRO_GALAR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0083
        SPECIES_FARFETCHD, 1,
        ABILITY_HUSTLE
    },
    { // 0085
        SPECIES_DODRIO, 0,
        ABILITY_HUSTLE
    },
    { // 0087
        SPECIES_DEWGONG, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0091
        SPECIES_CLOYSTER, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0094
        SPECIES_GENGAR, 1,
        ABILITY_MUMMY
    },
    {
        SPECIES_GENGAR_MEGA, 0,
        ABILITY_MUMMY
    },
    {
        SPECIES_GENGAR_MEGA, 1,
        ABILITY_MUMMY
    },
    {
        SPECIES_GENGAR_MEGA, 2,
        ABILITY_MUMMY
    },
    {
        SPECIES_GENGAR_GMAX, 1,
        ABILITY_MUMMY
    },
    { // 0097
        SPECIES_HYPNO, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0099
        SPECIES_KINGLER, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_KINGLER_GMAX, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0101
        SPECIES_ELECTRODE, 2,
        ABILITY_VOLT_ABSORB
    },
    {
        SPECIES_ELECTRODE_HISUI, 2,
        ABILITY_VOLT_ABSORB
    },
    { // 0103
        SPECIES_EXEGGUTOR, 0,
        ABILITY_SOLAR_POWER
    },
    {
        SPECIES_EXEGGUTOR, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0103
        SPECIES_EXEGGUTOR_ALOLA, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0105
        SPECIES_MAROWAK, 0,
        ABILITY_MUMMY
    },
    { // 0106
        SPECIES_HITMONLEE, 2,
        ABILITY_NO_GUARD
    },
    { // 0107
        SPECIES_HITMONCHAN, 2,
        ABILITY_NO_GUARD
    },
    { // 0110
        SPECIES_WEEZING, 2,
        ABILITY_POISON_POINT
    },
    { // 0113
        SPECIES_CHANSEY, 1,
        ABILITY_FLUFFY
    },
    { // 0115
        SPECIES_KANGASKHAN, 1,
        ABILITY_ANGER_SHELL
    },
    { // 0121
        SPECIES_STARMIE, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_STARMIE_MEGA, 0,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_STARMIE_MEGA, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_STARMIE_MEGA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0127
        SPECIES_PINSIR, 0,
        ABILITY_AERILATE
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_COMBAT, 1,
        ABILITY_NO_GUARD
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_BLAZE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_AQUA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0130
        SPECIES_GYARADOS, 1,
        ABILITY_MOTOR_DRIVE
    },
    {
        SPECIES_GYARADOS_MEGA, 0,
        ABILITY_MOTOR_DRIVE
    },
    {
        SPECIES_GYARADOS_MEGA, 1,
        ABILITY_MOTOR_DRIVE
    },
    {
        SPECIES_GYARADOS_MEGA, 2,
        ABILITY_MOTOR_DRIVE
    },
    { // 0142
        SPECIES_AERODACTYL, 2,
        ABILITY_HUSTLE
    },
    {
        SPECIES_AERODACTYL_MEGA, 0,
        ABILITY_HUSTLE
    },
    {
        SPECIES_AERODACTYL_MEGA, 1,
        ABILITY_HUSTLE
    },
    {
        SPECIES_AERODACTYL_MEGA, 2,
        ABILITY_HUSTLE
    },
    { // 0143
        SPECIES_SNORLAX, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0144
        SPECIES_ARTICUNO, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0144
        SPECIES_ARTICUNO_GALAR, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0145
        SPECIES_ZAPDOS, 1,
        ABILITY_VOLT_ABSORB
    },
    { // 0145
        SPECIES_ZAPDOS_GALAR, 1,
        ABILITY_HUSTLE
    },
    { // 0146
        SPECIES_MOLTRES_GALAR, 1,
        ABILITY_DARK_AURA
    },
    { // 0149
        SPECIES_DRAGONITE, 1,
        ABILITY_AERILATE
    },
    {
        SPECIES_DRAGONITE_MEGA, 0,
        ABILITY_AERILATE
    },
    {
        SPECIES_DRAGONITE_MEGA, 1,
        ABILITY_AERILATE
    },
    {
        SPECIES_DRAGONITE_MEGA, 2,
        ABILITY_AERILATE
    },
    { // 0150
        SPECIES_MEWTWO, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_X, 0,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_X, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_X, 2,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_Y, 0,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_Y, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_MEWTWO_MEGA_Y, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0154
        SPECIES_MEGANIUM, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_MEGANIUM_MEGA, 0,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_MEGANIUM_MEGA, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_MEGANIUM_MEGA, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0157
        SPECIES_TYPHLOSION, 1,
        ABILITY_DROUGHT
    },
    {
        SPECIES_TYPHLOSION_HISUI, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0162
        SPECIES_FURRET, 0,
        ABILITY_HUSTLE
    },
    { // 0164
        SPECIES_NOCTOWL, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0166
        SPECIES_LEDIAN, 0,
        ABILITY_VICTORY_STAR
    },
    { // 0168
        SPECIES_ARIADOS, 1,
        ABILITY_POISON_POINT
    },
    { // 0169
        SPECIES_CROBAT, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0171
        SPECIES_LANTURN, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0181
        SPECIES_AMPHAROS, 1,
        ABILITY_VOLT_ABSORB
    },
    {
        SPECIES_AMPHAROS_MEGA, 0,
        ABILITY_STATIC
    },
    {
        SPECIES_AMPHAROS_MEGA, 1,
        ABILITY_STATIC
    },
    {
        SPECIES_AMPHAROS_MEGA, 2,
        ABILITY_STATIC
    },
    { // 0182
        SPECIES_BELLOSSOM, 1,
        ABILITY_DROUGHT
    },
    { // 0185
        SPECIES_SUDOWOODO, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0189
        SPECIES_JUMPLUFF, 1,
        ABILITY_COTTON_DOWN
    },
    { // 0199
        SPECIES_SLOWKING, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0201
        SPECIES_UNOWN, 1,
        ABILITY_NEUTRALIZING_GAS
    },
    { // 0202
        SPECIES_WOBBUFFET, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0205
        SPECIES_FORRETRESS, 1,
        ABILITY_BULLETPROOF
    },
    { // 0208
        SPECIES_STEELIX, 1,
        ABILITY_SAND_STREAM
    },
    {
        SPECIES_STEELIX_MEGA, 0,
        ABILITY_SAND_STREAM
    },
    {
        SPECIES_STEELIX_MEGA, 1,
        ABILITY_SAND_STREAM
    },
    {
        SPECIES_STEELIX_MEGA, 2,
        ABILITY_SAND_STREAM
    },
    { // 0210
        SPECIES_GRANBULL, 2,
        ABILITY_HUSTLE
    },
    { // 0212
        SPECIES_SCIZOR, 1,
        ABILITY_WELL_BAKED_BODY
    },
    {
        SPECIES_SCIZOR_MEGA, 0,
        ABILITY_WELL_BAKED_BODY
    },
    {
        SPECIES_SCIZOR_MEGA, 1,
        ABILITY_WELL_BAKED_BODY
    },
    {
        SPECIES_SCIZOR_MEGA, 2,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0214
        SPECIES_HERACROSS, 2,
        ABILITY_NO_GUARD
    },
    { // 0217
        SPECIES_URSARING, 1,
        ABILITY_HUSTLE
    },
    { // 0222
        SPECIES_CORSOLA, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0224
        SPECIES_OCTILLERY, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0227
        SPECIES_SKARMORY, 1,
        ABILITY_BULLETPROOF
    },
    { // 0230
        SPECIES_KINGDRA, 0,
        ABILITY_DRIZZLE
    },
    { // 0232
        SPECIES_DONPHAN, 1,
        ABILITY_SAND_STREAM
    },
    { // 0237
        SPECIES_HITMONTOP, 2,
        ABILITY_NO_GUARD
    },
    { // 0242
        SPECIES_BLISSEY, 2,
        ABILITY_FLUFFY
    },
    { // 0243
        SPECIES_RAIKOU, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0244
        SPECIES_ENTEI, 1,
        ABILITY_FLAME_BODY
    },
    { // 0245
        SPECIES_SUICUNE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0249
        SPECIES_LUGIA, 1,
        ABILITY_STORM_DRAIN
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
        SPECIES_BLAZIKEN, 1,
        ABILITY_FLAME_BODY
    },
    {
        SPECIES_BLAZIKEN_MEGA, 0,
        ABILITY_FLAME_BODY
    },
    {
        SPECIES_BLAZIKEN_MEGA, 1,
        ABILITY_FLAME_BODY
    },
    {
        SPECIES_BLAZIKEN_MEGA, 2,
        ABILITY_FLAME_BODY
    },
    { // 0260
        SPECIES_SWAMPERT, 1,
        ABILITY_DRY_SKIN
    },
    {
        SPECIES_SWAMPERT_MEGA, 0,
        ABILITY_DRY_SKIN
    },
    {
        SPECIES_SWAMPERT_MEGA, 1,
        ABILITY_DRY_SKIN
    },
    {
        SPECIES_SWAMPERT_MEGA, 2,
        ABILITY_DRY_SKIN
    },
    { // 0262
        SPECIES_MIGHTYENA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0264
        SPECIES_LINOONE, 1,
        ABILITY_HUSTLE
    },
    { // 0267
        SPECIES_BEAUTIFLY, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0269
        SPECIES_DUSTOX, 1,
        ABILITY_POISON_POINT
    },
    { // 0272
        SPECIES_LUDICOLO, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0277
        SPECIES_SWELLOW, 1,
        ABILITY_WIND_RIDER
    },
    { // 0284
        SPECIES_MASQUERAIN, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0286
        SPECIES_BRELOOM, 1,
        ABILITY_HUSTLE
    },
    { // 0291
        SPECIES_NINJASK, 1,
        ABILITY_HUSTLE
    },
    { // 0302
        SPECIES_SABLEYE, 1,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_SABLEYE_MEGA, 0,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_SABLEYE_MEGA, 1,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_SABLEYE_MEGA, 2,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0303
        SPECIES_MAWILE_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_MAWILE_MEGA, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_MAWILE_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0306
        SPECIES_AGGRON, 1,
        ABILITY_BULLETPROOF
    },
    {
        SPECIES_AGGRON_MEGA, 0,
        ABILITY_BULLETPROOF
    },
    {
        SPECIES_AGGRON_MEGA, 1,
        ABILITY_BULLETPROOF
    },
    {
        SPECIES_AGGRON_MEGA, 2,
        ABILITY_BULLETPROOF
    },
    { // 0308
        SPECIES_MEDICHAM, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_MEDICHAM_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_MEDICHAM_MEGA, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_MEDICHAM_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0310
        SPECIES_MANECTRIC_MEGA, 0,
        ABILITY_LIGHTNING_ROD
    },
    {
        SPECIES_MANECTRIC_MEGA, 1,
        ABILITY_LIGHTNING_ROD
    },
    {
        SPECIES_MANECTRIC_MEGA, 2,
        ABILITY_LIGHTNING_ROD
    },
    { // 0313
        SPECIES_VOLBEAT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0314
        SPECIES_ILLUMISE, 0,
        ABILITY_STORM_DRAIN
    },
    { // 0317
        SPECIES_SWALOT, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0319
        SPECIES_SHARPEDO, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_SHARPEDO_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_SHARPEDO_MEGA, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_SHARPEDO_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0321
        SPECIES_WAILORD, 0,
        ABILITY_DRY_SKIN
    },
    {
        SPECIES_WAILORD, 2,
        ABILITY_DRIZZLE
    },
    { // 0323
        SPECIES_CAMERUPT, 0,
        ABILITY_FLAME_BODY
    },
    {
        SPECIES_CAMERUPT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0326
        SPECIES_GRUMPIG, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0330
        SPECIES_FLYGON, 1,
        ABILITY_SAND_STREAM
    },
    { // 0335
        SPECIES_ZANGOOSE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0336
        SPECIES_SEVIPER, 1,
        ABILITY_POISON_POINT
    },
    { // 0337
        SPECIES_LUNATONE, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0338
        SPECIES_SOLROCK, 1,
        ABILITY_DROUGHT
    },
    { // 0340
        SPECIES_WHISCASH, 2,
        ABILITY_STORM_DRAIN
    },
    { // 0342
        SPECIES_CRAWDAUNT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0344
        SPECIES_CLAYDOL, 1,
        ABILITY_SAND_STREAM
    },
    { // 0348
        SPECIES_ARMALDO, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0350
        SPECIES_MILOTIC, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0354
        SPECIES_BANETTE, 2,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_BANETTE_MEGA, 0,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_BANETTE_MEGA, 1,
        ABILITY_WANDERING_SPIRIT
    },
    {
        SPECIES_BANETTE_MEGA, 2,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0356
        SPECIES_DUSCLOPS, 1,
        ABILITY_MUMMY
    },
    { // 0358
        SPECIES_CHIMECHO, 1,
        ABILITY_SOUNDPROOF
    },
    {
        SPECIES_CHIMECHO_MEGA, 0,
        ABILITY_SOUNDPROOF
    },
    {
        SPECIES_CHIMECHO_MEGA, 1,
        ABILITY_SOUNDPROOF
    },
    {
        SPECIES_CHIMECHO_MEGA, 2,
        ABILITY_SOUNDPROOF
    },
    { // 0359
        SPECIES_ABSOL, 2,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA, 0,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA, 1,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA, 2,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA_Z, 0,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA_Z, 1,
        ABILITY_DARK_AURA
    },
    {
        SPECIES_ABSOL_MEGA_Z, 2,
        ABILITY_DARK_AURA
    },
    { // 0365
        SPECIES_WALREIN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0367
        SPECIES_HUNTAIL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0368
        SPECIES_GOREBYSS, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0369
        SPECIES_RELICANTH, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0370
        SPECIES_LUVDISC, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0373
        SPECIES_SALAMENCE, 1,
        ABILITY_ANGER_SHELL
    },
    { // 0376
        SPECIES_METAGROSS, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_METAGROSS_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_METAGROSS_MEGA, 1,
        ABILITY_SHEER_FORCE
    },
    {
        SPECIES_METAGROSS_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0377
        SPECIES_REGIROCK, 1,
        ABILITY_SAND_STREAM
    },
    { // 0378
        SPECIES_REGICE, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0379
        SPECIES_REGISTEEL, 1,
        ABILITY_BULLETPROOF
    },
    { // 0380
        SPECIES_LATIAS, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIAS_MEGA, 0,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIAS_MEGA, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIAS_MEGA, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0381
        SPECIES_LATIOS, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIOS_MEGA, 0,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIOS_MEGA, 1,
        ABILITY_SYNCHRONIZE
    },
    {
        SPECIES_LATIOS_MEGA, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0385
        SPECIES_JIRACHI, 1,
        ABILITY_BULLETPROOF
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
        ABILITY_GRASSY_SURGE
    },
    { // 0392
        SPECIES_INFERNAPE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0395
        SPECIES_EMPOLEON, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0398
        SPECIES_STARAPTOR, 1,
        ABILITY_HUSTLE
    },
    { // 0400
        SPECIES_BIBAREL, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0402
        SPECIES_KRICKETUNE, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0405
        SPECIES_LUXRAY, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0413
        SPECIES_WORMADAM_PLANT, 1,
        ABILITY_SAP_SIPPER
    },
    {
        SPECIES_WORMADAM_SANDY, 1,
        ABILITY_SAND_STREAM
    },
    {
        SPECIES_WORMADAM_TRASH, 1,
        ABILITY_TOXIC_DEBRIS
    },
    { // 0414
        SPECIES_MOTHIM, 1,
        ABILITY_WIND_RIDER
    },
    { // 0416
        SPECIES_VESPIQUEN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0419
        SPECIES_FLOATZEL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0424
        SPECIES_AMBIPOM, 1,
        ABILITY_HUSTLE
    },
    { // 0426
        SPECIES_DRIFBLIM, 0,
        ABILITY_CLOUD_NINE
    },
    { // 0428
        SPECIES_LOPUNNY, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0428
        SPECIES_LOPUNNY_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0428
        // Mirrors the base form's slot 1 (Klutz) instead of Sheer Force. Mega Lopunny's three
        // slots were all Sheer Force, so EVERY Lopunny came out of a Mega Evolution with it --
        // including the Klutz/Toxic Orb/Switcheroo set, whose Fake Out then never flinched
        // (Sheer Force eats the flinch) and whose Toxic Orb was no longer suppressed. Slots 0 and 2
        // stay Sheer Force: slot 2 is where the deliberate Sheer Force set lives, so that one
        // keeps its ability through the Mega. Scrappy, the upstream value here, is granted
        // innately to LOPUNNY_MEGA, which is what frees this slot.
        SPECIES_LOPUNNY_MEGA, 1,
        ABILITY_KLUTZ
    },
    { // 0428
        SPECIES_LOPUNNY_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0429
        SPECIES_MISMAGIUS, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0430
        SPECIES_HONCHKROW, 1,
        ABILITY_DARK_AURA
    },
    { // 0432
        SPECIES_PURUGLY, 0,
        ABILITY_HUSTLE
    },
    { // 0435
        SPECIES_SKUNTANK, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0437
        SPECIES_BRONZONG, 2,
        ABILITY_DRIZZLE
    },
    { // 0441
        SPECIES_CHATOT, 1,
        ABILITY_AERILATE
    },
    { // 0442
        SPECIES_SPIRITOMB, 1,
        ABILITY_MUMMY
    },
    { // 0445
        SPECIES_GARCHOMP, 1,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA, 0,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA, 1,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA, 2,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z, 0,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z, 1,
        ABILITY_SAND_STREAM
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z, 2,
        ABILITY_SAND_STREAM
    },
    { // 0448
        SPECIES_LUCARIO, 1,
        ABILITY_NO_GUARD
    },
    { // 0448
        SPECIES_LUCARIO, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA_Z, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA_Z, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0448
        SPECIES_LUCARIO_MEGA_Z, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0452
        SPECIES_DRAPION, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0455
        SPECIES_CARNIVINE, 1,
        ABILITY_SEED_SOWER
    },
    { // 0461
        SPECIES_WEAVILE, 1,
        ABILITY_DARK_AURA
    },
    { // 0462
        SPECIES_MAGNEZONE, 2,
        ABILITY_LIGHTNING_ROD
    },
    { // 0464
        SPECIES_RHYPERIOR, 1,
        ABILITY_BULLETPROOF
    },
    { // 0465
        SPECIES_TANGROWTH, 2,
        ABILITY_SAP_SIPPER
    },
    { // 0466
        SPECIES_ELECTIVIRE, 1,
        ABILITY_ELECTRIC_SURGE
    },
    { // 0467
        SPECIES_MAGMORTAR, 1,
        ABILITY_DROUGHT
    },
    { // 0468
        SPECIES_TOGEKISS, 1,
        ABILITY_HALO
    },
    { // 0469
        SPECIES_YANMEGA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0470
        SPECIES_LEAFEON, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0471
        SPECIES_GLACEON, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0472
        SPECIES_GLISCOR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0473
        SPECIES_MAMOSWINE, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0474
        SPECIES_PORYGON_Z, 2,
        ABILITY_SIMPLE
    },
    { // 0475
        SPECIES_GALLADE, 0,
        ABILITY_SIMPLE
    },
    {
        SPECIES_GALLADE_MEGA, 0,
        ABILITY_SIMPLE
    },
    {
        SPECIES_GALLADE_MEGA, 1,
        ABILITY_SIMPLE
    },
    {
        SPECIES_GALLADE_MEGA, 2,
        ABILITY_SIMPLE
    },
    { // 0476
        SPECIES_PROBOPASS, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0477
        SPECIES_DUSKNOIR, 1,
        ABILITY_MUMMY
    },
    { // 0478
        SPECIES_GLALIE, 0,
        ABILITY_SNOW_WARNING
    },
    {
        SPECIES_FROSLASS, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0479
        SPECIES_ROTOM, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_HEAT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_WASH, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_FROST, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_FAN, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_MOW, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0480
        SPECIES_UXIE, 1,
        ABILITY_TRACE
    },
    { // 0481
        SPECIES_MESPRIT, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0482
        SPECIES_AZELF, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0483
        SPECIES_DIALGA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0483
        SPECIES_DIALGA_ORIGIN, 1,
        ABILITY_BULLETPROOF
    },
    { // 0484
        SPECIES_PALKIA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0484
        SPECIES_PALKIA_ORIGIN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0487
        SPECIES_GIRATINA_ALTERED, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0487
        SPECIES_GIRATINA_ORIGIN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0488
        SPECIES_CRESSELIA, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0489
        SPECIES_PHIONE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0490
        SPECIES_MANAPHY, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0491
        SPECIES_DARKRAI, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0492
        SPECIES_SHAYMIN, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0492
        SPECIES_SHAYMIN_SKY, 1,
        ABILITY_WIND_RIDER
    },
    { // 0500
        SPECIES_EMBOAR, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0500
        SPECIES_EMBOAR_MEGA, 0,
        ABILITY_FLASH_FIRE
    },
    { // 0500
        SPECIES_EMBOAR_MEGA, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0500
        SPECIES_EMBOAR_MEGA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 0503
        SPECIES_SAMUROTT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0503
        SPECIES_SAMUROTT_HISUI, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0505
        SPECIES_WATCHOG, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0508
        SPECIES_STOUTLAND, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0510
        SPECIES_LIEPARD, 1,
        ABILITY_CONTRARY
    },
    { // 0512
        SPECIES_SIMISAGE, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0514
        SPECIES_SIMISEAR, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0516
        SPECIES_SIMIPOUR, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0521
        SPECIES_UNFEZANT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0530
        SPECIES_EXCADRILL, 2,
        ABILITY_SAND_STREAM
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA, 0,
        ABILITY_SAND_STREAM
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA, 1,
        ABILITY_SAND_STREAM
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA, 2,
        ABILITY_SAND_STREAM
    },
    { // 0531
        SPECIES_AUDINO, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0531
        SPECIES_AUDINO_MEGA, 0,
        ABILITY_SYNCHRONIZE
    },
    { // 0531
        SPECIES_AUDINO_MEGA, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0531
        SPECIES_AUDINO_MEGA, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0538
        SPECIES_THROH, 1,
        ABILITY_SIMPLE
    },
    { // 0539
        SPECIES_SAWK, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0542
        SPECIES_LEAVANNY, 2,
        ABILITY_HUSTLE
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA, 0,
        ABILITY_POISON_POINT
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA, 1,
        ABILITY_POISON_POINT
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA, 2,
        ABILITY_POISON_POINT
    },
    { // 0547
        SPECIES_WHIMSICOTT, 2,
        ABILITY_COTTON_DOWN
    },
    { // 0549
        SPECIES_LILLIGANT, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0550
        SPECIES_BASCULIN_RED_STRIPED, 1,
        ABILITY_ANGER_SHELL
    },
    {
        SPECIES_BASCULIN_BLUE_STRIPED, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0553
        SPECIES_KROOKODILE, 2,
        ABILITY_SAND_STREAM
    },
    { // 0560
        SPECIES_SCRAFTY, 1,
        ABILITY_FLUFFY
    },
    { // 0560
        SPECIES_SCRAFTY_MEGA, 0,
        ABILITY_FLUFFY
    },
    { // 0560
        SPECIES_SCRAFTY_MEGA, 1,
        ABILITY_FLUFFY
    },
    { // 0560
        SPECIES_SCRAFTY_MEGA, 2,
        ABILITY_FLUFFY
    },
    { // 0561
        SPECIES_SIGILYPH, 2,
        ABILITY_SIMPLE
    },
    { // 0565
        SPECIES_CARRACOSTA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0569
        SPECIES_GARBODOR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0569
        SPECIES_GARBODOR_GMAX, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0573
        SPECIES_CINCCINO, 1,
        ABILITY_HUSTLE
    },
    { // 0576
        SPECIES_GOTHITELLE, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0579
        SPECIES_REUNICLUS, 2,
        ABILITY_NO_GUARD
    },
    { // 0581
        SPECIES_SWANNA, 2,
        ABILITY_STORM_DRAIN
    },
    { // 0589
        SPECIES_ESCAVALIER, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0591
        SPECIES_AMOONGUSS, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0594
        SPECIES_ALOMOMOLA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0596
        SPECIES_GALVANTULA, 1,
        ABILITY_STATIC
    },
    { // 0598
        SPECIES_FERROTHORN, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0601
        SPECIES_KLINKLANG, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0604
        SPECIES_EELEKTROSS, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA, 0,
        ABILITY_FLASH_FIRE
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 0610
        SPECIES_HAXORUS, 0,
        ABILITY_ANGER_SHELL
    },
    { // 0614
        SPECIES_BEARTIC, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0615
        SPECIES_CRYOGONAL, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0617
        SPECIES_ACCELGOR, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0620
        SPECIES_MIENSHAO, 2,
        ABILITY_NO_GUARD
    },
    { // 0623
        SPECIES_GOLURK_MEGA, 0,
        ABILITY_NO_GUARD
    },
    { // 0623
        SPECIES_GOLURK_MEGA, 1,
        ABILITY_NO_GUARD
    },
    { // 0623
        SPECIES_GOLURK_MEGA, 2,
        ABILITY_NO_GUARD
    },
    { // 0630
        SPECIES_MANDIBUZZ, 2,
        ABILITY_WIND_RIDER
    },
    { // 0635
        SPECIES_HYDREIGON, 1,
        ABILITY_SHEER_FORCE },
    { // 0638
        SPECIES_COBALION, 1,
        ABILITY_BULLETPROOF
    },
    { // 0639
        SPECIES_TERRAKION, 1,
        ABILITY_SAND_STREAM
    },
    { // 0640
        SPECIES_VIRIZION, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0641
        SPECIES_TORNADUS_THERIAN, 1,
        ABILITY_WIND_RIDER
    },
    { // 0641
        SPECIES_TORNADUS, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0642
        SPECIES_THUNDURUS, 1,
        ABILITY_VOLT_ABSORB
    },
    { // 0643
        SPECIES_RESHIRAM, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0644
        SPECIES_ZEKROM, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0645
        SPECIES_LANDORUS_THERIAN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0646
        SPECIES_KYUREM, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0646
        SPECIES_KYUREM_WHITE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0646
        SPECIES_KYUREM_BLACK, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0647
        SPECIES_KELDEO, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0647
        SPECIES_KELDEO_RESOLUTE, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0648
        SPECIES_MELOETTA, 1,
        ABILITY_PIXILATE
    },
    { // 0648
        SPECIES_MELOETTA_PIROUETTE, 1,
        ABILITY_PIXILATE
    },
    { // 0649
        SPECIES_GENESECT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0649
        SPECIES_GENESECT_DOUSE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0649
        SPECIES_GENESECT_SHOCK, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0649
        SPECIES_GENESECT_BURN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0649
        SPECIES_GENESECT_CHILL, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0655
        SPECIES_DELPHOX, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0655
        SPECIES_DELPHOX_MEGA, 0,
        ABILITY_FLASH_FIRE
    },
    { // 0655
        SPECIES_DELPHOX_MEGA, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0655
        SPECIES_DELPHOX_MEGA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 0660
        SPECIES_DIGGERSBY, 2,
        ABILITY_EARTH_EATER
    },
    { // 0666
        SPECIES_VIVILLON, 1,
        ABILITY_WIND_RIDER
    },
    { // 0668
        SPECIES_PYROAR, 0,
        ABILITY_DROUGHT
    },
    { // 0675
        SPECIES_PANGORO, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0676
        SPECIES_FURFROU, 1,
        ABILITY_FLUFFY
    },
    { // 0678
        SPECIES_MEOWSTIC_M, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0678
        SPECIES_MEOWSTIC_F, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0683
        SPECIES_AROMATISSE, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0685
        SPECIES_SLURPUFF, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0689
        SPECIES_BARBARACLE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0689
        SPECIES_BARBARACLE_MEGA, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0689
        SPECIES_BARBARACLE_MEGA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0689
        SPECIES_BARBARACLE_MEGA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0691
        SPECIES_DRAGALGE_MEGA, 0,
        ABILITY_POISON_POINT
    },
    { // 0691
        SPECIES_DRAGALGE_MEGA, 1,
        ABILITY_POISON_POINT
    },
    { // 0691
        SPECIES_DRAGALGE_MEGA, 2,
        ABILITY_POISON_POINT
    },
    { // 0693
        SPECIES_CLAWITZER, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0697
        SPECIES_TYRANTRUM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0701
        SPECIES_HAWLUCHA, 1,
        ABILITY_HUSTLE
    },
    { // 0703
        SPECIES_CARBINK, 1,
        ABILITY_BULLETPROOF
    },
    { // 0707
        SPECIES_KLEFKI, 1,
        ABILITY_BULLETPROOF
    },
    { // 0709
        SPECIES_TREVENANT, 2,
        ABILITY_SAP_SIPPER
    },
    { // 0711
        SPECIES_GOURGEIST_SUPER, 0,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0713
        SPECIES_AVALUGG, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0713
        SPECIES_AVALUGG_HISUI, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0715
        SPECIES_NOIVERN, 2,
        ABILITY_SOUNDPROOF
    },
    { // 0719
        SPECIES_DIANCIE, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0719
        SPECIES_DIANCIE_MEGA, 0,
        ABILITY_MISTY_SURGE
    },
    { // 0719
        SPECIES_DIANCIE_MEGA, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0719
        SPECIES_DIANCIE_MEGA, 2,
        ABILITY_MISTY_SURGE
    },
    { // 0720
        SPECIES_HOOPA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0720
        SPECIES_HOOPA_UNBOUND, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0724
        SPECIES_DECIDUEYE_HISUI, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0724
        SPECIES_DECIDUEYE, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0727
        SPECIES_INCINEROAR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0735
        SPECIES_GUMSHOOS, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0738
        SPECIES_VIKAVOLT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0740
        SPECIES_CRABOMINABLE, 1,
        ABILITY_NO_GUARD
    },
    { // 0743
        SPECIES_RIBOMBEE, 0,
        ABILITY_FLUFFY
    },
    { // 0745
        SPECIES_LYCANROC_DUSK, 1,
        ABILITY_SAND_STREAM
    },
    { // 0745
        SPECIES_LYCANROC, 1,
        ABILITY_SAND_STREAM
    },
    { // 0748
        SPECIES_TOXAPEX, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0750
        SPECIES_MUDSDALE, 1,
        ABILITY_EARTH_EATER
    },
    { // 0758
        SPECIES_SALAZZLE, 1,
        ABILITY_FLAME_BODY
    },
    { // 0763
        SPECIES_TSAREENA, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0764
        SPECIES_COMFEY, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0770
        SPECIES_PALOSSAND, 1,
        ABILITY_EARTH_EATER
    },
    { // 0771
        SPECIES_PYUKUMUKU, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0775
        SPECIES_KOMALA, 1,
        ABILITY_HUSTLE
    },
    { // 0776
        SPECIES_TURTONATOR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0741
        SPECIES_ORICORIO, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0741
        SPECIES_ORICORIO_PAU, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0756
        SPECIES_SHIINOTIC, 2,
        ABILITY_MYCELIUM_MIGHT
    },
    { // 0779
        SPECIES_BRUXISH, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0781
        SPECIES_DHELMISE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0791
        SPECIES_SOLGALEO, 1,
        ABILITY_DROUGHT
    },
    { // 0792
        SPECIES_LUNALA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        SPECIES_NECROZMA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        SPECIES_NECROZMA_DUSK_MANE, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        SPECIES_NECROZMA_DAWN_WINGS, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0802
        SPECIES_MARSHADOW, 1,
        ABILITY_TRACE
    },
    { // 0793
        SPECIES_NIHILEGO, 1,
        ABILITY_TOXIC_DEBRIS
    },
    { // 0794
        SPECIES_BUZZWOLE, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0795
        SPECIES_PHEROMOSA, 1,
        ABILITY_LINGERING_AROMA
    },
    { // 0796
        SPECIES_XURKITREE, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0797
        SPECIES_CELESTEELA, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0798
        SPECIES_KARTANA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0799
        SPECIES_GUZZLORD, 1,
        ABILITY_EARTH_EATER
    },
    { // 0801
        SPECIES_MAGEARNA, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0804
        SPECIES_NAGANADEL, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0805
        SPECIES_STAKATAKA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0806
        SPECIES_BLACEPHALON, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0809
        SPECIES_MELMETAL, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0818
        SPECIES_INTELEON, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0820
        SPECIES_GREEDENT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0823
        SPECIES_CORVIKNIGHT, 1,
        ABILITY_BULLETPROOF
    },
    { // 0826
        SPECIES_ORBEETLE, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0834
        SPECIES_DREDNAW, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0836
        SPECIES_BOLTUND, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0842
        SPECIES_APPLETUN, 2,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0847
        SPECIES_BARRASKEWDA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0849
        SPECIES_TOXTRICITY, 1,
        ABILITY_VOLT_ABSORB
    },
    { // 0849
        SPECIES_TOXTRICITY_LOW_KEY, 0,
        ABILITY_VOLT_ABSORB
    },
    { // 0853
        SPECIES_GRAPPLOCT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0858
        SPECIES_HATTERENE, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0861
        SPECIES_GRIMMSNARL, 2,
        ABILITY_FLUFFY
    },
    { // 0862
        SPECIES_OBSTAGOON, 0,
        ABILITY_DARK_AURA
    },
    { // 0863
        SPECIES_PERRSERKER, 1,
        ABILITY_BULLETPROOF
    },
    { // 0865
        SPECIES_SIRFETCHD, 1,
        ABILITY_BULLETPROOF
    },
    { // 0866
        SPECIES_MR_RIME, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0869
        SPECIES_ALCREMIE, 1,
        ABILITY_PIXILATE
    },
    {
        SPECIES_ALCREMIE_GMAX, 1,
        ABILITY_PIXILATE
    },
    { // 0870
        SPECIES_FALINKS, 1,
        ABILITY_NO_GUARD
    },
    { // 0873
        SPECIES_FROSMOTH, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0874
        SPECIES_STONJOURNER, 1,
        ABILITY_SAND_STREAM
    },
    { // 0887
        SPECIES_DRAGAPULT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0888
        SPECIES_ZACIAN, 1,
        ABILITY_SWORD_OF_RUIN
    },
    { // 0888
        SPECIES_ZACIAN_CROWNED, 1,
        ABILITY_SWORD_OF_RUIN
    },
    { // 0889
        SPECIES_ZAMAZENTA, 1,
        ABILITY_TABLETS_OF_RUIN
    },
    { // 0889
        SPECIES_ZAMAZENTA_CROWNED, 1,
        ABILITY_TABLETS_OF_RUIN
    },
    { // 0890
        SPECIES_ETERNATUS, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0892
        SPECIES_URSHIFU, 1,
        ABILITY_DARK_AURA
    },
    { // 0892
        SPECIES_URSHIFU_RAPID_STRIKE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0893
        SPECIES_ZARUDE, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0894
        SPECIES_REGIELEKI, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0895
        SPECIES_REGIDRAGO, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0896
        SPECIES_GLASTRIER, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0897
        SPECIES_SPECTRIER, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0898
        SPECIES_CALYREX, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0900
        SPECIES_KLEAVOR, 2,
        ABILITY_HUSTLE
    },
    { // 0901
        SPECIES_URSALUNA_BLOODMOON, 1,
        ABILITY_EARTH_EATER
    },
    { // 0902
        SPECIES_BASCULEGION, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0902
        SPECIES_BASCULEGION_F, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0905
        SPECIES_ENAMORUS_THERIAN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0911
        SPECIES_SKELEDIRGE, 1,
        ABILITY_MUMMY
    },
    { // 0914
        SPECIES_QUAQUAVAL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0916
        SPECIES_OINKOLOGNE_F, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0918
        SPECIES_SPIDOPS, 1,
        ABILITY_TOXIC_DEBRIS
    },
    { // 0920
        SPECIES_LOKIX, 1,
        ABILITY_DARK_AURA
    },
    { // 0925
        SPECIES_MAUSHOLD, 1,
        ABILITY_NO_GUARD
    },
    { // 0934
        SPECIES_GARGANACL, 1,
        ABILITY_EARTH_EATER
    },
    { // 0943
        SPECIES_MABOSSTIFF, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0952
        SPECIES_SCOVILLAIN, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0956
        SPECIES_ESPATHRA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0959
        SPECIES_TINKATON, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0961
        SPECIES_WUGTRIO, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0962
        SPECIES_BOMBIRDIER, 1,
        ABILITY_WIND_RIDER
    },
    { // 0966
        SPECIES_REVAVROOM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0967
        SPECIES_CYCLIZAR, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0976
        SPECIES_VELUZA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0977
        SPECIES_DONDOZO, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0979
        SPECIES_ANNIHILAPE, 1,
        ABILITY_ANGER_SHELL
    },
    { // 0982
        SPECIES_DUDUNSPARCE, 0,
        ABILITY_SIMPLE
    },
    { // 0983
        SPECIES_KINGAMBIT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0998
        SPECIES_BAXCALIBUR, 1,
        ABILITY_SNOW_WARNING
    },
    { // 1000
        SPECIES_GHOLDENGO, 1,
        ABILITY_SHEER_FORCE
    },
    { // 1013
        SPECIES_SINISTCHA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 1017
        SPECIES_OGERPON_HEARTHFLAME, 1,
        ABILITY_FLASH_FIRE
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE, 1,
        ABILITY_EARTH_EATER
    },
    { // 1017
        SPECIES_OGERPON, 1,
        ABILITY_SEED_SOWER
    },
    { // 1018
        SPECIES_ARCHALUDON, 1,
        ABILITY_BULLETPROOF
    },
    { // 1019
        SPECIES_HYDRAPPLE, 2,
        ABILITY_GRASSY_SURGE
    },
};

// GetSpeciesAbility (src/pokemon.c) calls this on EVERY ability lookup, game-wide
// and in the AI's per-move hot path, but only the handful of species with a row
// above ever match — so a plain linear scan taxes every lookup for the whole
// roster (it was measurably inflating the AI's thinking time, and the frontier-slot sweep
// keeps growing the table). A one-time bitmap of "which species have any override
// row" lets the overwhelmingly common no-override case return in O(1); only a
// species that actually carries an override falls through to the short scan.
static u8 sSpeciesHasOverride[(NUM_SPECIES + 7) / 8];
static bool8 sSpeciesHasOverrideReady;

enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot)
{
    u32 i;
    bool32 foundSpecies = FALSE;

    // FORK: gate the override table behind FEATURE_INNATE_ABILITIES, exactly like innates
    // and the fork's other runtime features. TestInitConfigData() force-disables every fork
    // FEATURE flag by default, so upstream tests see VANILLA ability slots -- an override can
    // no longer rewrite a species' ability inside a test that does not opt in via WITH_CONFIG.
    // Real builds default the flag TRUE, so gameplay/frontier behavior is unchanged. Overrides
    // exist solely as the counterpart to innates (they hand a species a real chosen ability
    // precisely because innates made its real slots redundant), so they share the one flag.
    if (!GetConfig(FEATURE_INNATE_ABILITIES))
        return ABILITY_NONE;

    if (!sSpeciesHasOverrideReady)
    {
        for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
        {
            u16 s = sSpeciesAbilityOverrides[i].species;
            sSpeciesHasOverride[s / 8] |= 1 << (s % 8);
        }
        sSpeciesHasOverrideReady = TRUE;
    }

    if (species >= NUM_SPECIES
     || !(sSpeciesHasOverride[species / 8] & (1 << (species % 8))))
        return ABILITY_NONE;

    for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
    {
        if (sSpeciesAbilityOverrides[i].species != species)
        {
            // A species' rows are contiguous (the table is dex-sorted, with a species'
            // slots and formes kept together), so once we have stepped past this species'
            // block there is nothing left to find -- stop rather than scan the whole
            // table. Keeps this hot lookup cheap (it runs deep in AI evaluation) as the
            // override table grows.
            if (foundSpecies)
                break;
            continue;
        }
        foundSpecies = TRUE;
        if (sSpeciesAbilityOverrides[i].slot == slot)
            return sSpeciesAbilityOverrides[i].ability;
    }

    return ABILITY_NONE;
}
