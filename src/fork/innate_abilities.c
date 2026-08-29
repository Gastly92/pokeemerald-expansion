#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "fork/innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species->innate table (FEATURE_INNATE_ABILITIES). Kept here instead of in
// gSpeciesInfo so upstream syncs never touch it. Each row maps a species to a list of innate
// abilities always active on top of its normal chosen ability; write each row with the
// INNATES(...) macro below.
//
// ALLOWLIST: only abilities whose innate behavior is actually wired at an effect site may appear.
// sImplementedInnates[] in test/fork/innate_abilities.c is the CI-enforced source of truth — a row
// naming an unwired ability fails the build (rather than silently doing nothing at runtime). When
// you wire a new ability, in the SAME edit: add its `### ABILITY_NAME` block to
// fork-docs/INNATE_ABILITIES.md (per-ability semantics, divergences, and AI/species rationale live
// there), add it to the SCOPE list in include/fork/innate_abilities.h, and add it to
// sImplementedInnates[].
//
// FORMS ARE KEYED EXACTLY (no base-species fallback): a Mega / Gigantamax / regional / forme variant
// gets innates ONLY if it has its own row, since gBattleMons[].species becomes the form constant after
// a form change. Megas are a PURE BOON — each mirrors its base's list so the base creature's trait
// persists (e.g. Mega Venusaur keeps Overgrow / Chlorophyll though its real ability is Thick Fat).
// Exceptions: grounded Megas must not float — Mega Gengar mirrors the base row minus Levitate, and
// Mega Mewtwo X keeps only Pressure (dropping base Mewtwo's Levitate).

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// A species with SEVERAL innates lists them inline at its row with INNATES(...) instead of needing a
// named combination array per pairing (which doesn't scale as the allowlist grows). The compound
// literal has static storage at file scope; the terminator is appended automatically.
#define INNATES(...) (const enum Ability[]){ __VA_ARGS__, ABILITY_NONE }

static const struct SpeciesInnates sSpeciesInnates[] =
{
    { // 0001
        SPECIES_BULBASAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW
        )
    },
    { // 0002
        SPECIES_IVYSAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW
        )
    },
    { // 0003
        SPECIES_VENUSAUR,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR_MEGA,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR,
            ABILITY_THICK_FAT
        )
    },
    { // 0003
        SPECIES_VENUSAUR_GMAX,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0004
        SPECIES_CHARMANDER,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MAGMA_ARMOR
        )
    },
    { // 0005
        SPECIES_CHARMELEON,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MAGMA_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0006
        SPECIES_CHARIZARD,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_JUSTIFIED,
            ABILITY_MAGMA_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_X,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_JUSTIFIED,
            ABILITY_MAGMA_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_Y,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_JUSTIFIED,
            ABILITY_MAGMA_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0006
        SPECIES_CHARIZARD_GMAX,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_JUSTIFIED,
            ABILITY_MAGMA_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0007
        SPECIES_SQUIRTLE,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0008
        SPECIES_WARTORTLE,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SHELL_ARMOR,
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_MEGA,
        INNATES(
            ABILITY_MEGA_LAUNCHER,
            ABILITY_RAIN_DISH,
            ABILITY_SHELL_ARMOR,
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_GMAX,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SHELL_ARMOR,
            ABILITY_SKILL_LINK,
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0010
        SPECIES_CATERPIE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_SHIELD_DUST,
            ABILITY_STENCH
        )
    },
    { // 0011
        SPECIES_METAPOD,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0012
        SPECIES_BUTTERFREE,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_FRISK,
            ABILITY_SHIELD_DUST,
            ABILITY_TINTED_LENS
        )
    },
    { // 0012
        SPECIES_BUTTERFREE_GMAX,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_FRISK,
            ABILITY_SHIELD_DUST,
            ABILITY_TINTED_LENS
        )
    },
    { // 0013
        SPECIES_WEEDLE,
        INNATES(
            ABILITY_FRISK,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0014
        SPECIES_KAKUNA,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_STURDY,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0015
        SPECIES_BEEDRILL,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MERCILESS,
            ABILITY_SNIPER,
            ABILITY_STAKEOUT,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0015
        SPECIES_BEEDRILL_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_LEVITATE,
            ABILITY_MERCILESS,
            ABILITY_SKILL_LINK,
            ABILITY_SNIPER,
            ABILITY_STAKEOUT,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0016
        SPECIES_PIDGEY,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0017
        SPECIES_PIDGEOTTO,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_STAKEOUT,
            ABILITY_TANGLED_FEET,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0018
        SPECIES_PIDGEOT,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_CUTE_CHARM,
            ABILITY_KEEN_EYE,
            ABILITY_STAKEOUT,
            ABILITY_TANGLED_FEET,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0018
        SPECIES_PIDGEOT_MEGA,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_CUTE_CHARM,
            ABILITY_KEEN_EYE,
            ABILITY_STAKEOUT,
            ABILITY_TANGLED_FEET,
            ABILITY_TOUGH_CLAWS,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0019
        SPECIES_RATTATA,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_GUTS,
            ABILITY_QUICK_FEET
        )
    },
    { // 0019
        SPECIES_RATTATA_ALOLA,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_INFILTRATOR,
            ABILITY_PICKPOCKET,
            ABILITY_STRONG_JAW,
            ABILITY_THICK_FAT
        )
    },
    { // 0020
        SPECIES_RATICATE,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_GUTS,
            ABILITY_QUICK_FEET,
            ABILITY_STRONG_JAW
        )
    },
    { // 0020
        SPECIES_RATICATE_ALOLA,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_INFILTRATOR,
            ABILITY_INTIMIDATE,
            ABILITY_PICKPOCKET,
            ABILITY_STRONG_JAW,
            ABILITY_THICK_FAT
        )
    },
    { // 0021
        SPECIES_SPEAROW,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0022
        SPECIES_FEAROW,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_MOXIE,
            ABILITY_SNIPER
        )
    },
    { // 0023
        SPECIES_EKANS,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_LIMBER,
            ABILITY_SHED_SKIN,
            ABILITY_UNNERVE
        )
    },
    { // 0024
        SPECIES_ARBOK,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_INTIMIDATE,
            ABILITY_LIMBER,
            ABILITY_SHED_SKIN,
            ABILITY_SUCTION_CUPS,
            ABILITY_UNNERVE
        )
    },
    { // 0025
        SPECIES_PIKACHU,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_CUTE_CHARM,
            ABILITY_RECKLESS
        )
    },
    { // 0025
        SPECIES_PIKACHU_GMAX,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_CUTE_CHARM,
            ABILITY_RECKLESS
        )
    },
    { // 0026
        SPECIES_RAICHU,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_RECKLESS
        )
    },
    { // 0026
        SPECIES_RAICHU_ALOLA,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_LEVITATE,
            ABILITY_SURGE_SURFER,
            ABILITY_TELEPATHY
        )
    },
    { // 0026
        SPECIES_RAICHU_MEGA_X,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_LEVITATE,
            ABILITY_RECKLESS
        )
    },
    { // 0026
        SPECIES_RAICHU_MEGA_Y,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_SNIPER
        )
    },
    { // 0027
        SPECIES_SANDSHREW,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0027
        SPECIES_SANDSHREW_ALOLA,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0028
        SPECIES_SANDSLASH,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_HEATPROOF,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0028
        SPECIES_SANDSLASH_ALOLA,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_HYPER_CUTTER,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0029
        SPECIES_NIDORAN_F,
        INNATES(
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0030
        SPECIES_NIDORINA,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_RATTLED,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0031
        SPECIES_NIDOQUEEN,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_ROCK_HEAD,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0032
        SPECIES_NIDORAN_M,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_FRISK,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0033
        SPECIES_NIDORINO,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0034
        SPECIES_NIDOKING,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0035
        SPECIES_CLEFAIRY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_MAGIC_GUARD,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0036
        SPECIES_CLEFABLE,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_SERENE_GRACE,
            ABILITY_UNAWARE
        )
    },
    { // 0036
        SPECIES_CLEFABLE_MEGA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_BOUNCE,
            ABILITY_MAGIC_GUARD,
            ABILITY_SERENE_GRACE,
            ABILITY_UNAWARE
        )
    },
    { // 0037
        SPECIES_VULPIX,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR
        )
    },
    { // 0037
        SPECIES_VULPIX_ALOLA,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_HEATPROOF,
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0038
        SPECIES_NINETALES,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_MAGIC_BOUNCE,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0038
        SPECIES_NINETALES_ALOLA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_FRIEND_GUARD,
            ABILITY_HEATPROOF,
            ABILITY_ICE_BODY,
            ABILITY_JUSTIFIED,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0039
        SPECIES_JIGGLYPUFF,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_INSOMNIA,
            ABILITY_THICK_FAT
        )
    },
    { // 0040
        SPECIES_WIGGLYTUFF,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_THICK_FAT
        )
    },
    { // 0041
        SPECIES_ZUBAT,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_INNER_FOCUS,
            ABILITY_UNAWARE
        )
    },
    { // 0042
        SPECIES_GOLBAT,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_INNER_FOCUS,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW
        )
    },
    { // 0043
        SPECIES_ODDISH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_STENCH
        )
    },
    { // 0044
        SPECIES_GLOOM,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH
        )
    },
    { // 0045
        SPECIES_VILEPLUME,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEAF_GUARD,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH
        )
    },
    { // 0046
        SPECIES_PARAS,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_HEALER,
            ABILITY_POISON_HEAL,
            ABILITY_REGENERATOR
        )
    },
    { // 0047
        SPECIES_PARASECT,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_HEALER,
            ABILITY_POISON_HEAL,
            ABILITY_REGENERATOR,
            ABILITY_SWARM
        )
    },
    { // 0048
        SPECIES_VENONAT,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_SHIELD_DUST,
            ABILITY_TINTED_LENS
        )
    },
    { // 0049
        SPECIES_VENOMOTH,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_LEVITATE,
            ABILITY_SHIELD_DUST,
            ABILITY_TINTED_LENS,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0050
        SPECIES_DIGLETT,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0050
        SPECIES_DIGLETT_ALOLA,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL,
            ABILITY_TANGLING_HAIR,
            ABILITY_TELEPATHY
        )
    },
    { // 0051
        SPECIES_DUGTRIO,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL,
            ABILITY_TELEPATHY,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0051
        SPECIES_DUGTRIO_ALOLA,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_BATTLE_ARMOR,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL,
            ABILITY_TANGLING_HAIR,
            ABILITY_TELEPATHY
        )
    },
    { // 0052
        SPECIES_MEOWTH,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_PICKPOCKET,
            ABILITY_PICKUP,
            ABILITY_TECHNICIAN,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0052
        SPECIES_MEOWTH_ALOLA,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_PICKPOCKET,
            ABILITY_PICKUP,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN
        )
    },
    { // 0052
        SPECIES_MEOWTH_GALAR,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_PICKUP,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0052
        SPECIES_MEOWTH_GMAX,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_PICKPOCKET,
            ABILITY_PICKUP,
            ABILITY_TECHNICIAN,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0053
        SPECIES_PERSIAN,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LIMBER,
            ABILITY_PICKPOCKET,
            ABILITY_TECHNICIAN,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0053
        SPECIES_PERSIAN_ALOLA,
        INNATES(
            ABILITY_FUR_COAT,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN,
            ABILITY_UNNERVE
        )
    },
    { // 0054
        SPECIES_PSYDUCK,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_SWIFT_SWIM,
            ABILITY_TELEPATHY,
            ABILITY_UNAWARE
        )
    },
    { // 0055
        SPECIES_GOLDUCK,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_SWIFT_SWIM,
            ABILITY_TELEPATHY,
            ABILITY_UNAWARE
        )
    },
    { // 0056
        SPECIES_MANKEY,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_DEFIANT,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0057
        SPECIES_PRIMEAPE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_DEFIANT,
            ABILITY_UNAWARE,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0058
        SPECIES_GROWLITHE,
        INNATES(
            ABILITY_FRISK,
            ABILITY_GUARD_DOG,
            ABILITY_INTIMIDATE,
            ABILITY_JUSTIFIED
        )
    },
    { // 0058
        SPECIES_GROWLITHE_HISUI,
        INNATES(
            ABILITY_GUARD_DOG,
            ABILITY_INTIMIDATE,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0059
        SPECIES_ARCANINE,
        INNATES(
            ABILITY_FRISK,
            ABILITY_GUARD_DOG,
            ABILITY_INTIMIDATE,
            ABILITY_JUSTIFIED,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0059
        SPECIES_ARCANINE_HISUI,
        INNATES(
            ABILITY_GUARD_DOG,
            ABILITY_INTIMIDATE,
            ABILITY_ROCK_HEAD,
            ABILITY_STRONG_JAW
        )
    },
    { // 0060
        SPECIES_POLIWAG,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0061
        SPECIES_POLIWHIRL,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_LIMBER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0062
        SPECIES_POLIWRATH,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_LIMBER,
            ABILITY_SWIFT_SWIM,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0063
        SPECIES_ABRA,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_MAGIC_GUARD,
            ABILITY_TELEPATHY
        )
    },
    { // 0064
        SPECIES_KADABRA,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_MAGIC_GUARD,
            ABILITY_TELEPATHY
        )
    },
    { // 0065
        SPECIES_ALAKAZAM,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_TELEPATHY
        )
    },
    { // 0065
        SPECIES_ALAKAZAM_MEGA,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_TELEPATHY
        )
    },
    { // 0066
        SPECIES_MACHOP,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST,
            ABILITY_STEADFAST
        )
    },
    { // 0067
        SPECIES_MACHOKE,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST,
            ABILITY_STEADFAST
        )
    },
    { // 0068
        SPECIES_MACHAMP,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST,
            ABILITY_SCRAPPY,
            ABILITY_STEADFAST
        )
    },
    { // 0068
        SPECIES_MACHAMP_GMAX,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST,
            ABILITY_SCRAPPY,
            ABILITY_STEADFAST
        )
    },
    { // 0069
        SPECIES_BELLSPROUT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GLUTTONY,
            ABILITY_LIQUID_OOZE
        )
    },
    { // 0070
        SPECIES_WEEPINBELL,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GLUTTONY,
            ABILITY_LIQUID_OOZE,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0071
        SPECIES_VICTREEBEL,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GLUTTONY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STAKEOUT
        )
    },
    { // 0071
        SPECIES_VICTREEBEL_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GLUTTONY,
            ABILITY_INNARDS_OUT,
            ABILITY_LIQUID_OOZE,
            ABILITY_STAKEOUT
        )
    },
    { // 0072
        SPECIES_TENTACOOL,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_HYDRATION,
            ABILITY_LIQUID_OOZE,
            ABILITY_RAIN_DISH,
            ABILITY_REGENERATOR
        )
    },
    { // 0073
        SPECIES_TENTACRUEL,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_HYDRATION,
            ABILITY_ILLUMINATE,
            ABILITY_LIQUID_OOZE,
            ABILITY_RAIN_DISH,
            ABILITY_REGENERATOR
        )
    },
    { // 0074
        SPECIES_GEODUDE,
        INNATES(
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0074
        SPECIES_GEODUDE_ALOLA,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER_ALOLA,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL,
            ABILITY_SHED_SKIN,
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM_ALOLA,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0077
        SPECIES_PONYTA,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0077
        SPECIES_PONYTA_GALAR,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_PASTEL_VEIL,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0078
        SPECIES_RAPIDASH,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0078
        SPECIES_RAPIDASH_GALAR,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_PASTEL_VEIL,
            ABILITY_SHARPNESS,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0079
        SPECIES_SLOWPOKE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR
        )
    },
    { // 0079
        SPECIES_SLOWPOKE_GALAR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_MEGA,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_GALAR,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_QUICK_DRAW,
            ABILITY_REGENERATOR
        )
    },
    { // 0081
        SPECIES_MAGNEMITE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_MAGNET_PULL,
            ABILITY_STURDY
        )
    },
    { // 0082
        SPECIES_MAGNETON,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_MAGNET_PULL,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0083
        SPECIES_FARFETCHD,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_INNER_FOCUS,
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0083
        SPECIES_FARFETCHD_GALAR,
        INNATES(
            ABILITY_JUSTIFIED,
            ABILITY_SCRAPPY,
            ABILITY_SHARPNESS,
            ABILITY_STEADFAST
        )
    },
    { // 0084
        SPECIES_DODUO,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_EARLY_BIRD,
            ABILITY_SPEED_BOOST,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0085
        SPECIES_DODRIO,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_EARLY_BIRD,
            ABILITY_SPEED_BOOST,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0086
        SPECIES_SEEL,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_THICK_FAT
        )
    },
    { // 0087
        SPECIES_DEWGONG,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_THICK_FAT
        )
    },
    { // 0088
        SPECIES_GRIMER,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_GOOEY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0088
        SPECIES_GRIMER_ALOLA,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_GOOEY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0089
        SPECIES_MUK,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_GOOEY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0089
        SPECIES_MUK_ALOLA,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_GOOEY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0090
        SPECIES_SHELLDER,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SHELL_ARMOR,
            ABILITY_SKILL_LINK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0091
        SPECIES_CLOYSTER,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OVERCOAT,
            ABILITY_SHELL_ARMOR,
            ABILITY_SKILL_LINK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0092
        SPECIES_GASTLY,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE
        )
    },
    { // 0093
        SPECIES_HAUNTER,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0094
        SPECIES_GENGAR,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0094
        SPECIES_GENGAR_MEGA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_PRANKSTER,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0094
        SPECIES_GENGAR_GMAX,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0095
        SPECIES_ONIX,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0096
        SPECIES_DROWZEE,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_INFILTRATOR,
            ABILITY_INNER_FOCUS,
            ABILITY_INSOMNIA,
            ABILITY_TELEPATHY
        )
    },
    { // 0097
        SPECIES_HYPNO,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_INFILTRATOR,
            ABILITY_INNER_FOCUS,
            ABILITY_INSOMNIA,
            ABILITY_TELEPATHY
        )
    },
    { // 0098
        SPECIES_KRABBY,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0099
        SPECIES_KINGLER,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0099
        SPECIES_KINGLER_GMAX,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0100
        SPECIES_VOLTORB,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_OVERCOAT
        )
    },
    { // 0100
        SPECIES_VOLTORB_HISUI,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_HARVEST,
            ABILITY_OVERCOAT
        )
    },
    { // 0101
        SPECIES_ELECTRODE,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_OVERCOAT
        )
    },
    { // 0101
        SPECIES_ELECTRODE_HISUI,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_HARVEST,
            ABILITY_OVERCOAT
        )
    },
    { // 0102
        SPECIES_EXEGGCUTE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_HARVEST,
            ABILITY_LEAF_GUARD,
            ABILITY_REGENERATOR
        )
    },
    { // 0103
        SPECIES_EXEGGUTOR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_HARVEST,
            ABILITY_LEAF_GUARD,
            ABILITY_REGENERATOR
        )
    },
    { // 0103
        SPECIES_EXEGGUTOR_ALOLA,
        INNATES(
            ABILITY_FRISK,
            ABILITY_HARVEST,
            ABILITY_LEAF_GUARD,
            ABILITY_REGENERATOR
        )
    },
    { // 0104
        SPECIES_CUBONE,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_INNER_FOCUS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0105
        SPECIES_MAROWAK,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_INNER_FOCUS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0105
        SPECIES_MAROWAK_ALOLA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INNER_FOCUS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0105
        SPECIES_MAROWAK_ALOLA_TOTEM,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_INNER_FOCUS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0106
        SPECIES_HITMONLEE,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_RECKLESS,
            ABILITY_UNBURDEN
        )
    },
    { // 0107
        SPECIES_HITMONCHAN,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INNER_FOCUS,
            ABILITY_IRON_FIST,
            ABILITY_KEEN_EYE
        )
    },
    { // 0108
        SPECIES_LICKITUNG,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_STICKY_HOLD,
            ABILITY_UNAWARE
        )
    },
    { // 0109
        SPECIES_KOFFING,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING_GALAR,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_GLUTTONY,
            ABILITY_LEVITATE,
            ABILITY_OVERCOAT
        )
    },
    { // 0111
        SPECIES_RHYHORN,
        INNATES(
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0112
        SPECIES_RHYDON,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0113
        SPECIES_CHANSEY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_HEALER,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0114
        SPECIES_TANGELA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_REGENERATOR,
            ABILITY_SHED_SKIN
        )
    },
    { // 0115
        SPECIES_KANGASKHAN,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_INNER_FOCUS,
            ABILITY_SCRAPPY
        )
    },
    { // 0115
        SPECIES_KANGASKHAN_MEGA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_INNER_FOCUS,
            ABILITY_SCRAPPY
        )
    },
    { // 0116
        SPECIES_HORSEA,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0117
        SPECIES_SEADRA,
        INNATES(
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0118
        SPECIES_GOLDEEN,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0119
        SPECIES_SEAKING,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0120
        SPECIES_STARYU,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE_MEGA,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_HUGE_POWER,
            ABILITY_ILLUMINATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0122
        SPECIES_MR_MIME,
        INNATES(
            ABILITY_FILTER,
            ABILITY_MAGIC_BOUNCE,
            ABILITY_TECHNICIAN
        )
    },
    { // 0122
        SPECIES_MR_MIME_GALAR,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0123
        SPECIES_SCYTHER,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_STEADFAST,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0124
        SPECIES_JYNX,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FOREWARN,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0125
        SPECIES_ELECTABUZZ,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0126
        SPECIES_MAGMAR,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0127
        SPECIES_PINSIR,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_MOLD_BREAKER,
            ABILITY_MOXIE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0127
        SPECIES_PINSIR_MEGA,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_MOLD_BREAKER,
            ABILITY_MOXIE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0128
        SPECIES_TAUROS,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_INTIMIDATE,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_COMBAT,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_CUD_CHEW,
            ABILITY_INTIMIDATE,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_BLAZE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_CUD_CHEW,
            ABILITY_INTIMIDATE,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0128
        SPECIES_TAUROS_PALDEA_AQUA,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_CUD_CHEW,
            ABILITY_INTIMIDATE,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0129
        SPECIES_MAGIKARP,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0130
        SPECIES_GYARADOS,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_STRONG_JAW
        )
    },
    { // 0130
        SPECIES_GYARADOS_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOLD_BREAKER,
            ABILITY_MOXIE,
            ABILITY_STRONG_JAW
        )
    },
    { // 0131
        SPECIES_LAPRAS,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_ICE_BODY,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0131
        SPECIES_LAPRAS_GMAX,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_ICE_BODY,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0132
        SPECIES_DITTO,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0133
        SPECIES_EEVEE,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ANTICIPATION
        )
    },
    { // 0133
        SPECIES_EEVEE_GMAX,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ANTICIPATION
        )
    },
    { // 0133
        SPECIES_EEVEE_STARTER,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ANTICIPATION
        )
    },
    { // 0134
        SPECIES_VAPOREON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_HYDRATION
        )
    },
    { // 0135
        SPECIES_JOLTEON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_QUICK_FEET
        )
    },
    { // 0136
        SPECIES_FLAREON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_GUTS
        )
    },
    { // 0137
        SPECIES_PORYGON,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_DOWNLOAD,
            ABILITY_LEVITATE
        )
    },
    { // 0138
        SPECIES_OMANYTE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SOLID_ROCK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0139
        SPECIES_OMASTAR,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SOLID_ROCK,
            ABILITY_SUCTION_CUPS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0140
        SPECIES_KABUTO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SOLID_ROCK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0141
        SPECIES_KABUTOPS,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SHARPNESS,
            ABILITY_SOLID_ROCK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0142
        SPECIES_AERODACTYL,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK,
            ABILITY_UNNERVE
        )
    },
    { // 0142
        SPECIES_AERODACTYL_MEGA,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0143
        SPECIES_SNORLAX,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_IMMUNITY,
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0143
        SPECIES_SNORLAX_GMAX,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_IMMUNITY,
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0144
        SPECIES_ARTICUNO,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_PRESSURE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0144
        SPECIES_ARTICUNO_GALAR,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_PRESSURE
        )
    },
    { // 0145
        SPECIES_ZAPDOS,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0145
        SPECIES_ZAPDOS_GALAR,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_PRESSURE
        )
    },
    { // 0146
        SPECIES_MOLTRES,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0146
        SPECIES_MOLTRES_GALAR,
        INNATES(
            ABILITY_BERSERK,
            ABILITY_PRESSURE
        )
    },
    { // 0147
        SPECIES_DRATINI,
        INNATES(
            ABILITY_MARVEL_SCALE,
            ABILITY_SHED_SKIN
        )
    },
    { // 0148
        SPECIES_DRAGONAIR,
        INNATES(
            ABILITY_MARVEL_SCALE,
            ABILITY_SHED_SKIN
        )
    },
    { // 0149
        SPECIES_DRAGONITE,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_MULTISCALE
        )
    },
    { // 0149
        SPECIES_DRAGONITE_MEGA,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_MULTISCALE
        )
    },
    { // 0150
        SPECIES_MEWTWO,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_UNNERVE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_X,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_STEADFAST,
            ABILITY_UNNERVE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_Y,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_UNNERVE
        )
    },
    { // 0151
        SPECIES_MEW,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE
        )
    },
    { // 0152
        SPECIES_CHIKORITA,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0153
        SPECIES_BAYLEEF,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0154
        SPECIES_MEGANIUM,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_MEGA_SOL
        )
    },
    { // 0154
        SPECIES_MEGANIUM_MEGA,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_MEGA_SOL
        )
    },
    { // 0155
        SPECIES_CYNDAQUIL,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0156
        SPECIES_QUILAVA,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION_HISUI,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE,
            ABILITY_FRISK
        )
    },
    { // 0158
        SPECIES_TOTODILE,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STRONG_JAW,
            ABILITY_TORRENT
        )
    },
    { // 0159
        SPECIES_CROCONAW,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STRONG_JAW,
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STRONG_JAW,
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STRONG_JAW,
            ABILITY_TORRENT
        )
    },
    { // 0161
        SPECIES_SENTRET,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE
        )
    },
    { // 0162
        SPECIES_FURRET,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE
        )
    },
    { // 0163
        SPECIES_HOOTHOOT,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE,
            ABILITY_STAKEOUT,
            ABILITY_TINTED_LENS
        )
    },
    { // 0164
        SPECIES_NOCTOWL,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_INSOMNIA,
            ABILITY_KEEN_EYE,
            ABILITY_STAKEOUT,
            ABILITY_TINTED_LENS
        )
    },
    { // 0165
        SPECIES_LEDYBA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_FRIEND_GUARD,
            ABILITY_RATTLED,
            ABILITY_SWARM
        )
    },
    { // 0166
        SPECIES_LEDIAN,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_FRIEND_GUARD,
            ABILITY_IRON_FIST,
            ABILITY_SWARM
        )
    },
    { // 0167
        SPECIES_SPINARAK,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_SNIPER,
            ABILITY_STAKEOUT,
            ABILITY_SWARM
        )
    },
    { // 0168
        SPECIES_ARIADOS,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_SNIPER,
            ABILITY_STAKEOUT,
            ABILITY_SWARM
        )
    },
    { // 0169
        SPECIES_CROBAT,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_INNER_FOCUS,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0170
        SPECIES_CHINCHOU,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_ILLUMINATE
        )
    },
    { // 0171
        SPECIES_LANTURN,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_ILLUMINATE
        )
    },
    { // 0172
        SPECIES_PICHU,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_CUTE_CHARM,
            ABILITY_RATTLED
        )
    },
    { // 0173
        SPECIES_CLEFFA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_MAGIC_GUARD,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0174
        SPECIES_IGGLYBUFF,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_CUTE_CHARM,
            ABILITY_FRIEND_GUARD,
            ABILITY_INSOMNIA,
            ABILITY_THICK_FAT
        )
    },
    { // 0175
        SPECIES_TOGEPI,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0176
        SPECIES_TOGETIC,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0177
        SPECIES_NATU,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_FOREWARN,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0178
        SPECIES_XATU,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_FOREWARN,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0179
        SPECIES_MAREEP,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_THICK_FAT
        )
    },
    { // 0180
        SPECIES_FLAAFFY,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_THICK_FAT
        )
    },
    { // 0181
        SPECIES_AMPHAROS,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_THICK_FAT
        )
    },
    { // 0181
        SPECIES_AMPHAROS_MEGA,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_MOLD_BREAKER,
            ABILITY_THICK_FAT
        )
    },
    { // 0182
        SPECIES_BELLOSSOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_DANCER,
            ABILITY_HEALER,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0183
        SPECIES_MARILL,
        INNATES(
            ABILITY_HUGE_POWER,
            ABILITY_SWIFT_SWIM,
            ABILITY_THICK_FAT
        )
    },
    { // 0184
        SPECIES_AZUMARILL,
        INNATES(
            ABILITY_HUGE_POWER,
            ABILITY_SWIFT_SWIM,
            ABILITY_THICK_FAT
        )
    },
    { // 0185
        SPECIES_SUDOWOODO,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0186
        SPECIES_POLITOED,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_RAIN_DISH
        )
    },
    { // 0187
        SPECIES_HOPPIP,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_INFILTRATOR,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0188
        SPECIES_SKIPLOOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_INFILTRATOR,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0189
        SPECIES_JUMPLUFF,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_INFILTRATOR,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0190
        SPECIES_AIPOM,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_PICKUP,
            ABILITY_PRANKSTER,
            ABILITY_SKILL_LINK
        )
    },
    { // 0191
        SPECIES_SUNKERN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0192
        SPECIES_SUNFLORA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0193
        SPECIES_YANMA,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_FRISK,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0194
        SPECIES_WOOPER,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0194
        SPECIES_WOOPER_PALDEA,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0195
        SPECIES_QUAGSIRE,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0196
        SPECIES_ESPEON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_FOREWARN,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0197
        SPECIES_UMBREON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_INNER_FOCUS
        )
    },
    { // 0198
        SPECIES_MURKROW,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0199
        SPECIES_SLOWKING,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR
        )
    },
    { // 0199
        SPECIES_SLOWKING_GALAR,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_OWN_TEMPO,
            ABILITY_REGENERATOR
        )
    },
    { // 0200
        SPECIES_MISDREAVUS,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0401
        SPECIES_KRICKETOT,
        INNATES(
            ABILITY_SHED_SKIN
        )
    },
    { // 0417
        SPECIES_PACHIRISU,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_PICKUP
        )
    },
    { // 0489
        SPECIES_PHIONE,
        INNATES(
            ABILITY_HYDRATION
        )
    },
    { // 0490
        SPECIES_MANAPHY,
        INNATES(
            ABILITY_HYDRATION
        )
    },
    { // 0559
        SPECIES_SCRAGGY,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_SHED_SKIN
        )
    },
    { // 0560
        SPECIES_SCRAFTY,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_SHED_SKIN
        )
    },
    { // 0560
        SPECIES_SCRAFTY_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_SHED_SKIN
        )
    },
    { // 0664
        SPECIES_SCATTERBUG_POKEBALL,
        INNATES(
            ABILITY_SHED_SKIN
        )
    },
    { // 0682
        SPECIES_SPRITZEE,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER
        )
    },
    { // 0683
        SPECIES_AROMATISSE,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_HEALER
        )
    },
    { // 0702
        SPECIES_DEDENNE,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_PICKUP
        )
    },
    { // 0704
        SPECIES_GOOMY,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_HYDRATION
        )
    },
    { // 0705
        SPECIES_SLIGGOO,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_HYDRATION
        )
    },
    { // 0706
        SPECIES_GOODRA,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_HYDRATION
        )
    },
    { // 0755
        SPECIES_MORELULL,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_ILLUMINATE,
            ABILITY_RAIN_DISH
        )
    },
    { // 0756
        SPECIES_SHIINOTIC,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_ILLUMINATE,
            ABILITY_RAIN_DISH
        )
    },
    { // 0856
        SPECIES_HATENNA,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0857
        SPECIES_HATTREM,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0858
        SPECIES_HATTERENE,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0858
        SPECIES_HATTERENE_GMAX,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_HEALER,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0928
        SPECIES_SMOLIV,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_HARVEST
        )
    },
    { // 0929
        SPECIES_DOLLIV,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_HARVEST
        )
    },
    { // 0930
        SPECIES_ARBOLIVA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_HARVEST
        )
    },
    { // 0971
        SPECIES_GREAVARD,
        INNATES(
            ABILITY_PICKUP
        )
    },
    { // 0996
        SPECIES_FRIGIBAX,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_THERMAL_EXCHANGE
        )
    },
    { // 0997
        SPECIES_ARCTIBAX,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_THERMAL_EXCHANGE
        )
    },
    { // 0998
        SPECIES_BAXCALIBUR,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_THERMAL_EXCHANGE
        )
    },
    { // 0998
        SPECIES_BAXCALIBUR_MEGA,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_THERMAL_EXCHANGE
        )
    },
    { // 0201
        SPECIES_UNOWN,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_B,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_C,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_D,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_E,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_F,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_G,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_H,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_I,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_J,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_K,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_L,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_M,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_N,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_O,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_P,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_Q,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_R,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_S,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_T,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_U,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_V,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_W,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_X,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_Y,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_Z,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_EXCLAMATION,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0201
        SPECIES_UNOWN_QUESTION,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0202
        SPECIES_WOBBUFFET,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_HARVEST,
            ABILITY_SHADOW_TAG,
            ABILITY_TELEPATHY
        )
    },
    { // 0203
        SPECIES_GIRAFARIG,
        INNATES(
            ABILITY_CUD_CHEW,
            ABILITY_EARLY_BIRD,
            ABILITY_INNER_FOCUS
        )
    },
    { // 0204
        SPECIES_PINECO,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_OVERCOAT,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0205
        SPECIES_FORRETRESS,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_OVERCOAT,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0206
        SPECIES_DUNSPARCE,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0207
        SPECIES_GLIGAR,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_IMMUNITY,
            ABILITY_INFILTRATOR,
            ABILITY_SAND_VEIL
        )
    },
    { // 0208
        SPECIES_STEELIX,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0208
        SPECIES_STEELIX_MEGA,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGNET_PULL,
            ABILITY_ROCK_HEAD,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0209
        SPECIES_SNUBBULL,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_RATTLED,
            ABILITY_STRONG_JAW
        )
    },
    { // 0210
        SPECIES_GRANBULL,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_QUICK_FEET,
            ABILITY_RATTLED,
            ABILITY_STRONG_JAW
        )
    },
    { // 0211
        SPECIES_QWILFISH,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0211
        SPECIES_QWILFISH_HISUI,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0212
        SPECIES_SCIZOR,
        INNATES(
            ABILITY_LIGHT_METAL,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0212
        SPECIES_SCIZOR_MEGA,
        INNATES(
            ABILITY_LIGHT_METAL,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN
        )
    },
    { // 0213
        SPECIES_SHUCKLE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_HARVEST,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0214
        SPECIES_HERACROSS,
        INNATES(
            ABILITY_GUTS,
            ABILITY_MOXIE,
            ABILITY_SWARM
        )
    },
    { // 0214
        SPECIES_HERACROSS_MEGA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_MOXIE,
            ABILITY_SKILL_LINK,
            ABILITY_SWARM
        )
    },
    { // 0215
        SPECIES_SNEASEL,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_KEEN_EYE,
            ABILITY_PICKPOCKET,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0215
        SPECIES_SNEASEL_HISUI,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_KEEN_EYE,
            ABILITY_PICKPOCKET,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0216
        SPECIES_TEDDIURSA,
        INNATES(
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET,
            ABILITY_THICK_FAT
        )
    },
    { // 0217
        SPECIES_URSARING,
        INNATES(
            ABILITY_GUTS,
            ABILITY_QUICK_FEET,
            ABILITY_THICK_FAT,
            ABILITY_UNNERVE
        )
    },
    { // 0218
        SPECIES_SLUGMA,
        INNATES(
            ABILITY_MAGMA_ARMOR
        )
    },
    { // 0219
        SPECIES_MAGCARGO,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0220
        SPECIES_SWINUB,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0221
        SPECIES_PILOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0222
        SPECIES_CORSOLA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0222
        SPECIES_CORSOLA_GALAR,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0864
        SPECIES_CURSOLA,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0223
        SPECIES_REMORAID,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_SNIPER
        )
    },
    { // 0224
        SPECIES_OCTILLERY,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_SNIPER,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0225
        SPECIES_DELIBIRD,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_PICKUP,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0226
        SPECIES_MANTINE,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0227
        SPECIES_SKARMORY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SHARPNESS,
            ABILITY_STURDY
        )
    },
    { // 0227
        SPECIES_SKARMORY_MEGA,
        INNATES(
            ABILITY_STALWART,
            ABILITY_STURDY
        )
    },
    { // 0228
        SPECIES_HOUNDOUR,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_UNNERVE
        )
    },
    { // 0229
        SPECIES_HOUNDOOM,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_UNNERVE
        )
    },
    { // 0229
        SPECIES_HOUNDOOM_MEGA,
        INNATES(
            ABILITY_EARLY_BIRD,
            ABILITY_UNNERVE
        )
    },
    { // 0230
        SPECIES_KINGDRA,
        INNATES(
            ABILITY_MARVEL_SCALE,
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0231
        SPECIES_PHANPY,
        INNATES(
            ABILITY_PICKUP,
            ABILITY_SAND_VEIL
        )
    },
    { // 0232
        SPECIES_DONPHAN,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY
        )
    },
    { // 0233
        SPECIES_PORYGON2,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_DOWNLOAD,
            ABILITY_LEVITATE
        )
    },
    { // 0234
        SPECIES_STANTLER,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INTIMIDATE
        )
    },
    { // 0235
        SPECIES_SMEARGLE,
        INNATES(
            ABILITY_OWN_TEMPO,
            ABILITY_TECHNICIAN
        )
    },
    { // 0236
        SPECIES_TYROGUE,
        INNATES(
            ABILITY_GUTS,
            ABILITY_STEADFAST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0237
        SPECIES_HITMONTOP,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STEADFAST,
            ABILITY_TECHNICIAN
        )
    },
    { // 0238
        SPECIES_SMOOCHUM,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_HYDRATION,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0239
        SPECIES_ELEKID,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0240
        SPECIES_MAGBY,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0241
        SPECIES_MILTANK,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SCRAPPY,
            ABILITY_THICK_FAT
        )
    },
    { // 0242
        SPECIES_BLISSEY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_HEALER,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0243
        SPECIES_RAIKOU,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE
        )
    },
    { // 0244
        SPECIES_ENTEI,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE
        )
    },
    { // 0245
        SPECIES_SUICUNE,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE
        )
    },
    { // 0246
        SPECIES_LARVITAR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SAND_VEIL
        )
    },
    { // 0247
        SPECIES_PUPITAR,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_SHED_SKIN
        )
    },
    { // 0248
        SPECIES_TYRANITAR,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_UNNERVE
        )
    },
    { // 0248
        SPECIES_TYRANITAR_MEGA,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_UNNERVE
        )
    },
    { // 0249
        SPECIES_LUGIA,
        INNATES(
            ABILITY_MULTISCALE,
            ABILITY_PRESSURE
        )
    },
    { // 0250
        SPECIES_HO_OH,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0251
        SPECIES_CELEBI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0252
        SPECIES_TREECKO,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_UNBURDEN
        )
    },
    { // 0253
        SPECIES_GROVYLE,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_UNBURDEN
        )
    },
    { // 0254
        SPECIES_SCEPTILE,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_UNBURDEN
        )
    },
    { // 0254
        SPECIES_SCEPTILE_MEGA,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_UNBURDEN
        )
    },
    { // 0255
        SPECIES_TORCHIC,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0256
        SPECIES_COMBUSKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN_MEGA,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_IRON_FIST,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0258
        SPECIES_MUDKIP,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0259
        SPECIES_MARSHTOMP,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT_MEGA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0261
        SPECIES_POOCHYENA,
        INNATES(
            ABILITY_QUICK_FEET,
            ABILITY_RATTLED
        )
    },
    { // 0262
        SPECIES_MIGHTYENA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_QUICK_FEET
        )
    },
    { // 0263
        SPECIES_ZIGZAGOON,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET
        )
    },
    { // 0263
        SPECIES_ZIGZAGOON_GALAR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET
        )
    },
    { // 0264
        SPECIES_LINOONE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET
        )
    },
    { // 0264
        SPECIES_LINOONE_GALAR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET
        )
    },
    { // 0265
        SPECIES_WURMPLE,
        INNATES(
            ABILITY_SHIELD_DUST
        )
    },
    { // 0266
        SPECIES_SILCOON,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0268
        SPECIES_CASCOON,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0267
        SPECIES_BEAUTIFLY,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0269
        SPECIES_DUSTOX,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_INSOMNIA,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0270
        SPECIES_LOTAD,
        INNATES(
            ABILITY_OWN_TEMPO,
            ABILITY_RAIN_DISH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0271
        SPECIES_LOMBRE,
        INNATES(
            ABILITY_OWN_TEMPO,
            ABILITY_RAIN_DISH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0272
        SPECIES_LUDICOLO,
        INNATES(
            ABILITY_DANCER,
            ABILITY_OWN_TEMPO,
            ABILITY_RAIN_DISH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0273
        SPECIES_SEEDOT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD,
            ABILITY_PICKPOCKET
        )
    },
    { // 0274
        SPECIES_NUZLEAF,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD,
            ABILITY_PICKPOCKET
        )
    },
    { // 0275
        SPECIES_SHIFTRY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EARLY_BIRD,
            ABILITY_PICKPOCKET
        )
    },
    { // 0276
        SPECIES_TAILLOW,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SCRAPPY
        )
    },
    { // 0277
        SPECIES_SWELLOW,
        INNATES(
            ABILITY_GUTS,
            ABILITY_SCRAPPY
        )
    },
    { // 0278
        SPECIES_WINGULL,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_KEEN_EYE,
            ABILITY_RAIN_DISH
        )
    },
    { // 0279
        SPECIES_PELIPPER,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_KEEN_EYE,
            ABILITY_RAIN_DISH
        )
    },
    { // 0280
        SPECIES_RALTS,
        INNATES(
            ABILITY_SERENE_GRACE,
            ABILITY_TELEPATHY
        )
    },
    { // 0281
        SPECIES_KIRLIA,
        INNATES(
            ABILITY_SERENE_GRACE,
            ABILITY_TELEPATHY
        )
    },
    { // 0282
        SPECIES_GARDEVOIR,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE,
            ABILITY_TELEPATHY
        )
    },
    { // 0282
        SPECIES_GARDEVOIR_MEGA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE,
            ABILITY_TELEPATHY
        )
    },
    { // 0283
        SPECIES_SURSKIT,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0284
        SPECIES_MASQUERAIN,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_UNNERVE
        )
    },
    { // 0285
        SPECIES_SHROOMISH,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_POISON_HEAL,
            ABILITY_QUICK_FEET
        )
    },
    { // 0286
        SPECIES_BRELOOM,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_POISON_HEAL,
            ABILITY_TECHNICIAN
        )
    },
    { // 0287
        SPECIES_SLAKOTH,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_THICK_FAT
        )
    },
    { // 0288
        SPECIES_VIGOROTH,
        INNATES(
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0289
        SPECIES_SLAKING,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_THICK_FAT
        )
    },
    { // 0290
        SPECIES_NINCADA,
        INNATES(
            ABILITY_COMPOUND_EYES
        )
    },
    { // 0291
        SPECIES_NINJASK,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0292
        SPECIES_SHEDINJA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0293
        SPECIES_WHISMUR,
        INNATES(
            ABILITY_RATTLED
        )
    },
    { // 0294
        SPECIES_LOUDRED,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0295
        SPECIES_EXPLOUD,
        INNATES(
            ABILITY_SCRAPPY
        )
    },
    { // 0296
        SPECIES_MAKUHITA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_THICK_FAT
        )
    },
    { // 0297
        SPECIES_HARIYAMA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_THICK_FAT
        )
    },
    { // 0298
        SPECIES_AZURILL,
        INNATES(
            ABILITY_HUGE_POWER,
            ABILITY_SWIFT_SWIM,
            ABILITY_THICK_FAT
        )
    },
    { // 0299
        SPECIES_NOSEPASS,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0300
        SPECIES_SKITTY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0301
        SPECIES_DELCATTY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0302
        SPECIES_SABLEYE,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER
        )
    },
    { // 0302
        SPECIES_SABLEYE_MEGA,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0303
        SPECIES_MAWILE,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_INTIMIDATE
        )
    },
    { // 0303
        SPECIES_MAWILE_MEGA,
        INNATES(
            ABILITY_HUGE_POWER,
            ABILITY_HYPER_CUTTER,
            ABILITY_INTIMIDATE
        )
    },
    { // 0304
        SPECIES_ARON,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_HEAVY_METAL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0305
        SPECIES_LAIRON,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_HEAVY_METAL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_HEAVY_METAL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON_MEGA,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_FILTER,
            ABILITY_HEAVY_METAL,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0307
        SPECIES_MEDITITE,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_PURE_POWER,
            ABILITY_TELEPATHY
        )
    },
    { // 0308
        SPECIES_MEDICHAM,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_PURE_POWER,
            ABILITY_TELEPATHY
        )
    },
    { // 0308
        SPECIES_MEDICHAM_MEGA,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_PURE_POWER,
            ABILITY_TELEPATHY
        )
    },
    { // 0309
        SPECIES_ELECTRIKE,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0310
        SPECIES_MANECTRIC,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0310
        SPECIES_MANECTRIC_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_QUICK_FEET
        )
    },
    { // 0311
        SPECIES_PLUSLE,
        INNATES(
            ABILITY_FRIEND_GUARD
        )
    },
    { // 0312
        SPECIES_MINUN,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_HEALER
        )
    },
    { // 0313
        SPECIES_VOLBEAT,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_PRANKSTER,
            ABILITY_SWARM
        )
    },
    { // 0314
        SPECIES_ILLUMISE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRANKSTER,
            ABILITY_TINTED_LENS
        )
    },
    { // 0315
        SPECIES_ROSELIA,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0316
        SPECIES_GULPIN,
        INNATES(
            ABILITY_CORROSION,
            ABILITY_GLUTTONY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0317
        SPECIES_SWALOT,
        INNATES(
            ABILITY_CORROSION,
            ABILITY_GLUTTONY,
            ABILITY_LIQUID_OOZE,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0318
        SPECIES_CARVANHA,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SPEED_BOOST,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0319
        SPECIES_SHARPEDO,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SPEED_BOOST,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0319
        SPECIES_SHARPEDO_MEGA,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SPEED_BOOST,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0320
        SPECIES_WAILMER,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE,
            ABILITY_THICK_FAT,
            ABILITY_WATER_VEIL
        )
    },
    { // 0321
        SPECIES_WAILORD,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE,
            ABILITY_THICK_FAT,
            ABILITY_WATER_VEIL
        )
    },
    { // 0322
        SPECIES_NUMEL,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_MAGMA_ARMOR,
            ABILITY_SOLID_ROCK,
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT_MEGA,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_MAGMA_ARMOR,
            ABILITY_SOLID_ROCK,
            ABILITY_UNAWARE
        )
    },
    { // 0324
        SPECIES_TORKOAL,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY,
            ABILITY_WHITE_SMOKE
        )
    },
    { // 0325
        SPECIES_SPOINK,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_OWN_TEMPO,
            ABILITY_TELEPATHY,
            ABILITY_THICK_FAT
        )
    },
    { // 0326
        SPECIES_GRUMPIG,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_OWN_TEMPO,
            ABILITY_TELEPATHY,
            ABILITY_THICK_FAT
        )
    },
    { // 0327
        SPECIES_SPINDA,
        INNATES(
            ABILITY_TANGLED_FEET
        )
    },
    { // 0328
        SPECIES_TRAPINCH,
        INNATES(
            ABILITY_ARENA_TRAP,
            ABILITY_HYPER_CUTTER,
            ABILITY_SAND_FORCE,
            ABILITY_STRONG_JAW
        )
    },
    { // 0329
        SPECIES_VIBRAVA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SAND_FORCE
        )
    },
    { // 0330
        SPECIES_FLYGON,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SAND_FORCE
        )
    },
    { // 0331
        SPECIES_CACNEA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_IRON_BARBS,
            ABILITY_SAND_VEIL
        )
    },
    { // 0332
        SPECIES_CACTURNE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_IRON_BARBS,
            ABILITY_SAND_VEIL
        )
    },
    { // 0333
        SPECIES_SWABLU,
        INNATES(
            ABILITY_HEALER,
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA,
        INNATES(
            ABILITY_HEALER,
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA_MEGA,
        INNATES(
            ABILITY_HEALER,
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0335
        SPECIES_ZANGOOSE,
        INNATES(
            ABILITY_QUICK_FEET,
            ABILITY_SCRAPPY,
            ABILITY_TOXIC_BOOST
        )
    },
    { // 0336
        SPECIES_SEVIPER,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LIMBER,
            ABILITY_SHARPNESS,
            ABILITY_SHED_SKIN
        )
    },
    { // 0337
        SPECIES_LUNATONE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0338
        SPECIES_SOLROCK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0339
        SPECIES_BARBOACH,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_GLUTTONY,
            ABILITY_HYDRATION,
            ABILITY_IMMUNITY,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0340
        SPECIES_WHISCASH,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_GLUTTONY,
            ABILITY_HYDRATION,
            ABILITY_IMMUNITY,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0341
        SPECIES_CORPHISH,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_HYPER_CUTTER,
            ABILITY_MOXIE,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0342
        SPECIES_CRAWDAUNT,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_HYPER_CUTTER,
            ABILITY_MOXIE,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0343
        SPECIES_BALTOY,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0344
        SPECIES_CLAYDOL,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0345
        SPECIES_LILEEP,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0346
        SPECIES_CRADILY,
        INNATES(
            ABILITY_RAIN_DISH,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0347
        SPECIES_ANORITH,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SHARPNESS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0348
        SPECIES_ARMALDO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SHARPNESS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0349
        SPECIES_FEEBAS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_OBLIVIOUS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0350
        SPECIES_MILOTIC,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_CUTE_CHARM,
            ABILITY_MARVEL_SCALE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0351
        SPECIES_CASTFORM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_RAIN_DISH,
            ABILITY_SLUSH_RUSH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0351
        SPECIES_CASTFORM_SUNNY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_RAIN_DISH,
            ABILITY_SLUSH_RUSH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0351
        SPECIES_CASTFORM_RAINY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_RAIN_DISH,
            ABILITY_SLUSH_RUSH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0351
        SPECIES_CASTFORM_SNOWY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_RAIN_DISH,
            ABILITY_SLUSH_RUSH,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0352
        SPECIES_KECLEON,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_KEEN_EYE
        )
    },
    { // 0353
        SPECIES_SHUPPET,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE_MEGA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0355
        SPECIES_DUSKULL,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INTIMIDATE,
            ABILITY_LEVITATE,
            ABILITY_SOUL_HEART
        )
    },
    { // 0356
        SPECIES_DUSCLOPS,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INTIMIDATE,
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_SOUL_HEART
        )
    },
    { // 0357
        SPECIES_TROPIUS,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_HARVEST
        )
    },
    { // 0358
        SPECIES_CHIMECHO,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0358
        SPECIES_CHIMECHO_MEGA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0359
        SPECIES_ABSOL,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_JUSTIFIED,
            ABILITY_PRESSURE,
            ABILITY_SHARPNESS,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_JUSTIFIED,
            ABILITY_PRESSURE,
            ABILITY_SHARPNESS,
            ABILITY_SUPER_LUCK,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA_Z,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_JUSTIFIED,
            ABILITY_PRESSURE,
            ABILITY_SHARPNESS,
            ABILITY_SUPER_LUCK,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0360
        SPECIES_WYNAUT,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_HARVEST,
            ABILITY_SHADOW_TAG,
            ABILITY_TELEPATHY
        )
    },
    { // 0361
        SPECIES_SNORUNT,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_INNER_FOCUS,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0362
        SPECIES_GLALIE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0362
        SPECIES_GLALIE_MEGA,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_INNER_FOCUS,
            ABILITY_LEVITATE,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0363
        SPECIES_SPHEAL,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0364
        SPECIES_SEALEO,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0365
        SPECIES_WALREIN,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_INTIMIDATE,
            ABILITY_OBLIVIOUS,
            ABILITY_THICK_FAT
        )
    },
    { // 0366
        SPECIES_CLAMPERL,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW
        )
    },
    { // 0367
        SPECIES_HUNTAIL,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0368
        SPECIES_GOREBYSS,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0369
        SPECIES_RELICANTH,
        INNATES(
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0370
        SPECIES_LUVDISC,
        INNATES(
            ABILITY_HEALER,
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0371
        SPECIES_BAGON,
        INNATES(
            ABILITY_ROCK_HEAD
        )
    },
    { // 0372
        SPECIES_SHELGON,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0373
        SPECIES_SALAMENCE,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0373
        SPECIES_SALAMENCE_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0374
        SPECIES_BELDUM,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_CLEAR_BODY,
            ABILITY_LIGHT_METAL,
            ABILITY_TELEPATHY
        )
    },
    { // 0375
        SPECIES_METANG,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_CLEAR_BODY,
            ABILITY_LIGHT_METAL,
            ABILITY_TELEPATHY
        )
    },
    { // 0376
        SPECIES_METAGROSS,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_CLEAR_BODY,
            ABILITY_LIGHT_METAL,
            ABILITY_TELEPATHY
        )
    },
    { // 0376
        SPECIES_METAGROSS_MEGA,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_CLEAR_BODY,
            ABILITY_LIGHT_METAL,
            ABILITY_TELEPATHY,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0377
        SPECIES_REGIROCK,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_REGENERATOR,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0378
        SPECIES_REGICE,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0379
        SPECIES_REGISTEEL,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LIGHT_METAL,
            ABILITY_LIMBER,
            ABILITY_STURDY
        )
    },
    { // 0380
        SPECIES_LATIAS,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0380
        SPECIES_LATIAS_MEGA,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0381
        SPECIES_LATIOS,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0381
        SPECIES_LATIOS_MEGA,
        INNATES(
            ABILITY_FOREWARN,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0382
        SPECIES_KYOGRE,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0382
        SPECIES_KYOGRE_PRIMAL,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0383
        SPECIES_GROUDON,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SAND_FORCE
        )
    },
    { // 0383
        SPECIES_GROUDON_PRIMAL,
        INNATES(
            ABILITY_MAGMA_ARMOR,
            ABILITY_SAND_FORCE
        )
    },
    { // 0384
        SPECIES_RAYQUAZA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0384
        SPECIES_RAYQUAZA_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0385
        SPECIES_JIRACHI,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0386
        SPECIES_DEOXYS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0386
        SPECIES_DEOXYS_ATTACK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0386
        SPECIES_DEOXYS_DEFENSE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0386
        SPECIES_DEOXYS_SPEED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0387
        SPECIES_TURTWIG,
        INNATES(
            ABILITY_HARVEST,
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0388
        SPECIES_GROTLE,
        INNATES(
            ABILITY_HARVEST,
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0389
        SPECIES_TORTERRA,
        INNATES(
            ABILITY_HARVEST,
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0390
        SPECIES_CHIMCHAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INNER_FOCUS,
            ABILITY_IRON_FIST,
            ABILITY_LIMBER
        )
    },
    { // 0391
        SPECIES_MONFERNO,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INNER_FOCUS,
            ABILITY_IRON_FIST,
            ABILITY_LIMBER
        )
    },
    { // 0392
        SPECIES_INFERNAPE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INNER_FOCUS,
            ABILITY_IRON_FIST,
            ABILITY_LIMBER
        )
    },
    { // 0393
        SPECIES_PIPLUP,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_TORRENT
        )
    },
    { // 0394
        SPECIES_PRINPLUP,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_TORRENT
        )
    },
    { // 0395
        SPECIES_EMPOLEON,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_TORRENT
        )
    },
    { // 0396
        SPECIES_STARLY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_RECKLESS
        )
    },
    { // 0397
        SPECIES_STARAVIA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_RECKLESS
        )
    },
    { // 0398
        SPECIES_STARAPTOR,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_MOXIE,
            ABILITY_RECKLESS
        )
    },
    { // 0398
        SPECIES_STARAPTOR_MEGA,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_MOXIE,
            ABILITY_RECKLESS
        )
    },
    { // 0399
        SPECIES_BIDOOF,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_UNAWARE
        )
    },
    { // 0400
        SPECIES_BIBAREL,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM,
            ABILITY_UNAWARE
        )
    },
    { // 0402
        SPECIES_KRICKETUNE,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_SWARM,
            ABILITY_TECHNICIAN,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0403
        SPECIES_SHINX,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0404
        SPECIES_LUXIO,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0405
        SPECIES_LUXRAY,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE,
            ABILITY_KEEN_EYE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0406
        SPECIES_BUDEW,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0407
        SPECIES_ROSERADE,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_TECHNICIAN
        )
    },
    { // 0408
        SPECIES_CRANIDOS,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0409
        SPECIES_RAMPARDOS,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0410
        SPECIES_SHIELDON,
        INNATES(
            ABILITY_FILTER,
            ABILITY_STAMINA,
            ABILITY_STURDY
        )
    },
    { // 0411
        SPECIES_BASTIODON,
        INNATES(
            ABILITY_FILTER,
            ABILITY_STAMINA,
            ABILITY_STURDY
        )
    },
    { // 0412
        SPECIES_BURMY_PLANT,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SHED_SKIN
        )
    },
    { // 0412
        SPECIES_BURMY_SANDY,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SHED_SKIN
        )
    },
    { // 0412
        SPECIES_BURMY_TRASH,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SHED_SKIN
        )
    },
    { // 0413
        SPECIES_WORMADAM_PLANT,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_OVERCOAT
        )
    },
    { // 0413
        SPECIES_WORMADAM_SANDY,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_OVERCOAT
        )
    },
    { // 0413
        SPECIES_WORMADAM_TRASH,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_OVERCOAT
        )
    },
    { // 0414
        SPECIES_MOTHIM,
        INNATES(
            ABILITY_MAGICIAN,
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0414
        SPECIES_MOTHIM_SANDY,
        INNATES(
            ABILITY_MAGICIAN,
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0414
        SPECIES_MOTHIM_TRASH,
        INNATES(
            ABILITY_MAGICIAN,
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0415
        SPECIES_COMBEE,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0416
        SPECIES_VESPIQUEN,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_PRESSURE,
            ABILITY_SWARM,
            ABILITY_UNNERVE
        )
    },
    { // 0418
        SPECIES_BUIZEL,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0419
        SPECIES_FLOATZEL,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0420
        SPECIES_CHERUBI,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0421
        SPECIES_CHERRIM_OVERCAST,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0421
        SPECIES_CHERRIM_SUNSHINE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0422
        SPECIES_SHELLOS_WEST,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0422
        SPECIES_SHELLOS_EAST,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0423
        SPECIES_GASTRODON_WEST,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0423
        SPECIES_GASTRODON_EAST,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0424
        SPECIES_AMBIPOM,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_PICKUP,
            ABILITY_PRANKSTER,
            ABILITY_SKILL_LINK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0425
        SPECIES_DRIFLOON,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_CURSED_BODY,
            ABILITY_FLARE_BOOST,
            ABILITY_UNBURDEN
        )
    },
    { // 0426
        SPECIES_DRIFBLIM,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_CURSED_BODY,
            ABILITY_FLARE_BOOST,
            ABILITY_UNBURDEN
        )
    },
    { // 0427
        SPECIES_BUNEARY,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY_MEGA,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER,
            ABILITY_SCRAPPY
        )
    },
    { // 0429
        SPECIES_MISMAGIUS,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0430
        SPECIES_HONCHKROW,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_MOXIE,
            ABILITY_PICKPOCKET,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0431
        SPECIES_GLAMEOW,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_LIMBER,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0432
        SPECIES_PURUGLY,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_INTIMIDATE,
            ABILITY_OWN_TEMPO,
            ABILITY_THICK_FAT
        )
    },
    { // 0433
        SPECIES_CHINGLING,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0434
        SPECIES_STUNKY,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_KEEN_EYE,
            ABILITY_STENCH
        )
    },
    { // 0435
        SPECIES_SKUNTANK,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_KEEN_EYE,
            ABILITY_STENCH
        )
    },
    { // 0436
        SPECIES_BRONZOR,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HEAVY_METAL,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0437
        SPECIES_BRONZONG,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HEAVY_METAL,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0438
        SPECIES_BONSLY,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_ROCK_HEAD,
            ABILITY_STURDY
        )
    },
    { // 0439
        SPECIES_MIME_JR,
        INNATES(
            ABILITY_FILTER,
            ABILITY_MAGIC_BOUNCE,
            ABILITY_TECHNICIAN
        )
    },
    { // 0440
        SPECIES_HAPPINY,
        INNATES(
            ABILITY_FRIEND_GUARD,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0441
        SPECIES_CHATOT,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0442
        SPECIES_SPIRITOMB,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_PRESSURE,
            ABILITY_SOUL_HEART
        )
    },
    { // 0443
        SPECIES_GIBLE,
        INNATES(
            ABILITY_ROUGH_SKIN,
            ABILITY_SAND_VEIL,
            ABILITY_STRONG_JAW
        )
    },
    { // 0444
        SPECIES_GABITE,
        INNATES(
            ABILITY_ROUGH_SKIN,
            ABILITY_SAND_VEIL,
            ABILITY_STRONG_JAW
        )
    },
    { // 0445
        SPECIES_GARCHOMP,
        INNATES(
            ABILITY_ROUGH_SKIN,
            ABILITY_SAND_VEIL,
            ABILITY_STRONG_JAW
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA,
        INNATES(
            ABILITY_ROUGH_SKIN,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_VEIL,
            ABILITY_STRONG_JAW
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z,
        INNATES(
            ABILITY_ROUGH_SKIN,
            ABILITY_SAND_VEIL,
            ABILITY_STRONG_JAW
        )
    },
    { // 0446
        SPECIES_MUNCHLAX,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_PICKUP,
            ABILITY_THICK_FAT,
            ABILITY_UNAWARE
        )
    },
    { // 0447
        SPECIES_RIOLU,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_PRANKSTER
        )
    },
    { // 0448
        SPECIES_LUCARIO_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_INNER_FOCUS,
            ABILITY_JUSTIFIED
        )
    },
    { // 0448
        SPECIES_LUCARIO,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_JUSTIFIED
        )
    },
    { // 0448
        SPECIES_LUCARIO_MEGA_Z,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_JUSTIFIED
        )
    },
    { // 0449
        SPECIES_HIPPOPOTAS,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SAND_FORCE
        )
    },
    { // 0450
        SPECIES_HIPPOWDON,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SAND_FORCE
        )
    },
    { // 0451
        SPECIES_SKORUPI,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0452
        SPECIES_DRAPION,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER
        )
    },
    { // 0453
        SPECIES_CROAGUNK,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LIMBER
        )
    },
    { // 0454
        SPECIES_TOXICROAK,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_LIMBER
        )
    },
    { // 0455
        SPECIES_CARNIVINE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0456
        SPECIES_FINNEON,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0457
        SPECIES_LUMINEON,
        INNATES(
            ABILITY_ILLUMINATE,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0458
        SPECIES_MANTYKE,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_VEIL
        )
    },
    { // 0459
        SPECIES_SNOVER,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0460
        SPECIES_ABOMASNOW,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0460
        SPECIES_ABOMASNOW_MEGA,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0461
        SPECIES_WEAVILE,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKPOCKET,
            ABILITY_PRESSURE,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0462
        SPECIES_MAGNEZONE,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_MAGNET_PULL,
            ABILITY_STURDY,
            ABILITY_TELEPATHY
        )
    },
    { // 0463
        SPECIES_LICKILICKY,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_OWN_TEMPO,
            ABILITY_STICKY_HOLD,
            ABILITY_UNAWARE
        )
    },
    { // 0464
        SPECIES_RHYPERIOR,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_RECKLESS,
            ABILITY_ROCK_HEAD,
            ABILITY_SOLID_ROCK
        )
    },
    { // 0465
        SPECIES_TANGROWTH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_REGENERATOR,
            ABILITY_SHED_SKIN
        )
    },
    { // 0466
        SPECIES_ELECTIVIRE,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0467
        SPECIES_MAGMORTAR,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_IRON_FIST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0468
        SPECIES_TOGEKISS,
        INNATES(
            ABILITY_HEALER,
            ABILITY_SERENE_GRACE,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0469
        SPECIES_YANMEGA,
        INNATES(
            ABILITY_FRISK,
            ABILITY_SPEED_BOOST,
            ABILITY_TINTED_LENS
        )
    },
    { // 0470
        SPECIES_LEAFEON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD
        )
    },
    { // 0471
        SPECIES_GLACEON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0472
        SPECIES_GLISCOR,
        INNATES(
            ABILITY_HYPER_CUTTER,
            ABILITY_INFILTRATOR,
            ABILITY_POISON_HEAL,
            ABILITY_SAND_VEIL
        )
    },
    { // 0473
        SPECIES_MAMOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0474
        SPECIES_PORYGON_Z,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_ANALYTIC,
            ABILITY_DOWNLOAD,
            ABILITY_LEVITATE
        )
    },
    { // 0475
        SPECIES_GALLADE,
        INNATES(
            ABILITY_JUSTIFIED,
            ABILITY_SHARPNESS,
            ABILITY_STEADFAST,
            ABILITY_TELEPATHY
        )
    },
    { // 0475
        SPECIES_GALLADE_MEGA,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_JUSTIFIED,
            ABILITY_SHARPNESS,
            ABILITY_STEADFAST,
            ABILITY_TELEPATHY
        )
    },
    { // 0476
        SPECIES_PROBOPASS,
        INNATES(
            ABILITY_MAGNET_PULL,
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0477
        SPECIES_DUSKNOIR,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INTIMIDATE,
            ABILITY_PRESSURE,
            ABILITY_SOUL_HEART
        )
    },
    { // 0478
        SPECIES_FROSLASS,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0478
        SPECIES_FROSLASS_MEGA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0479
        SPECIES_ROTOM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_HEAT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_WASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FROST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FAN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_MOW,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0480
        SPECIES_UXIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0481
        SPECIES_MESPRIT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0482
        SPECIES_AZELF,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0483
        SPECIES_DIALGA,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0483
        SPECIES_DIALGA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0484
        SPECIES_PALKIA,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0484
        SPECIES_PALKIA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0485
        SPECIES_HEATRAN,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0486
        SPECIES_REGIGIGAS,
        INNATES(
            ABILITY_CLEAR_BODY
        )
    },
    { // 0487
        SPECIES_GIRATINA_ALTERED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0487
        SPECIES_GIRATINA_ORIGIN,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0488
        SPECIES_CRESSELIA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0491
        SPECIES_DARKRAI,
        INNATES(
            ABILITY_BAD_DREAMS,
            ABILITY_LEVITATE
        )
    },
    { // 0491
        SPECIES_DARKRAI_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0492
        SPECIES_SHAYMIN_LAND,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0492
        SPECIES_SHAYMIN_SKY,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0493
        SPECIES_ARCEUS_NORMAL,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TELEPATHY
        )
    },
    { // 0494
        SPECIES_VICTINI,
        INNATES(
            ABILITY_SERENE_GRACE
        )
    },
    { // 0495
        SPECIES_SNIVY,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHED_SKIN
        )
    },
    { // 0496
        SPECIES_SERVINE,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHED_SKIN
        )
    },
    { // 0497
        SPECIES_SERPERIOR,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHED_SKIN
        )
    },
    { // 0498
        SPECIES_TEPIG,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_THICK_FAT
        )
    },
    { // 0499
        SPECIES_PIGNITE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_THICK_FAT
        )
    },
    { // 0500
        SPECIES_EMBOAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_RECKLESS,
            ABILITY_THICK_FAT
        )
    },
    { // 0500
        SPECIES_EMBOAR_MEGA,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MOLD_BREAKER,
            ABILITY_RECKLESS,
            ABILITY_THICK_FAT
        )
    },
    { // 0501
        SPECIES_OSHAWOTT,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0502
        SPECIES_DEWOTT,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT_HISUI,
        INNATES(
            ABILITY_SHARPNESS,
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0504
        SPECIES_PATRAT,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_KEEN_EYE
        )
    },
    { // 0505
        SPECIES_WATCHOG,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_ILLUMINATE,
            ABILITY_KEEN_EYE
        )
    },
    { // 0506
        SPECIES_LILLIPUP,
        INNATES(
            ABILITY_PICKUP,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0507
        SPECIES_HERDIER,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_SAND_RUSH,
            ABILITY_SCRAPPY,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0508
        SPECIES_STOUTLAND,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_SAND_RUSH,
            ABILITY_SCRAPPY,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0509
        SPECIES_PURRLOIN,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER,
            ABILITY_UNBURDEN
        )
    },
    { // 0510
        SPECIES_LIEPARD,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER,
            ABILITY_UNBURDEN
        )
    },
    { // 0511
        SPECIES_PANSAGE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_OVERGROW
        )
    },
    { // 0512
        SPECIES_SIMISAGE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_OVERGROW
        )
    },
    { // 0513
        SPECIES_PANSEAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_GLUTTONY
        )
    },
    { // 0514
        SPECIES_SIMISEAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_GLUTTONY
        )
    },
    { // 0515
        SPECIES_PANPOUR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_TORRENT
        )
    },
    { // 0516
        SPECIES_SIMIPOUR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_TORRENT
        )
    },
    { // 0517
        SPECIES_MUNNA,
        INNATES(
            ABILITY_BAD_DREAMS,
            ABILITY_FOREWARN,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0518
        SPECIES_MUSHARNA,
        INNATES(
            ABILITY_BAD_DREAMS,
            ABILITY_FOREWARN,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0519
        SPECIES_PIDOVE,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0520
        SPECIES_TRANQUILL,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0521
        SPECIES_UNFEZANT,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_SUPER_LUCK
        )
    },
    { // 0522
        SPECIES_BLITZLE,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0523
        SPECIES_ZEBSTRIKA,
        INNATES(
            ABILITY_QUICK_FEET
        )
    },
    { // 0524
        SPECIES_ROGGENROLA,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0525
        SPECIES_BOLDORE,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0526
        SPECIES_GIGALITH,
        INNATES(
            ABILITY_SAND_FORCE,
            ABILITY_STURDY
        )
    },
    { // 0527
        SPECIES_WOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0528
        SPECIES_SWOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0529
        SPECIES_DRILBUR,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_PIERCING_DRILL,
            ABILITY_SAND_FORCE,
            ABILITY_SAND_RUSH
        )
    },
    { // 0531
        SPECIES_AUDINO,
        INNATES(
            ABILITY_HEALER,
            ABILITY_REGENERATOR
        )
    },
    { // 0531
        SPECIES_AUDINO_MEGA,
        INNATES(
            ABILITY_HEALER,
            ABILITY_REGENERATOR
        )
    },
    { // 0532
        SPECIES_TIMBURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0533
        SPECIES_GURDURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0534
        SPECIES_CONKELDURR,
        INNATES(
            ABILITY_GUTS,
            ABILITY_IRON_FIST
        )
    },
    { // 0535
        SPECIES_TYMPOLE,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0536
        SPECIES_PALPITOAD,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0537
        SPECIES_SEISMITOAD,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0538
        SPECIES_THROH,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INNER_FOCUS,
            ABILITY_MOLD_BREAKER
        )
    },
    { // 0539
        SPECIES_SAWK,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_MOLD_BREAKER,
            ABILITY_STURDY
        )
    },
    { // 0540
        SPECIES_SEWADDLE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_OVERCOAT,
            ABILITY_SWARM
        )
    },
    { // 0541
        SPECIES_SWADLOON,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERCOAT,
            ABILITY_SWARM
        )
    },
    { // 0542
        SPECIES_LEAVANNY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_OVERCOAT,
            ABILITY_SWARM
        )
    },
    { // 0543
        SPECIES_VENIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0544
        SPECIES_WHIRLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0546
        SPECIES_COTTONEE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0547
        SPECIES_WHIMSICOTT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0548
        SPECIES_PETILIL,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0549
        SPECIES_LILLIGANT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_DANCER,
            ABILITY_LEAF_GUARD,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0549
        SPECIES_LILLIGANT_HISUI,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_DANCER,
            ABILITY_LEAF_GUARD,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0550
        SPECIES_BASCULIN_RED_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_MOLD_BREAKER,
            ABILITY_RECKLESS
        )
    },
    { // 0550
        SPECIES_BASCULIN_BLUE_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_MOLD_BREAKER,
            ABILITY_ROCK_HEAD
        )
    },
    { // 0550
        SPECIES_BASCULIN_WHITE_STRIPED,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_MOLD_BREAKER,
            ABILITY_RATTLED
        )
    },
    { // 0551
        SPECIES_SANDILE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE
        )
    },
    { // 0552
        SPECIES_KROKOROK,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE
        )
    },
    { // 0553
        SPECIES_KROOKODILE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_INTIMIDATE,
            ABILITY_MOXIE
        )
    },
    { // 0554
        SPECIES_DARUMAKA,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0554
        SPECIES_DARUMAKA_GALAR,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0555
        SPECIES_DARMANITAN,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0555
        SPECIES_DARMANITAN_ZEN,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0555
        SPECIES_DARMANITAN_GALAR_STANDARD,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0555
        SPECIES_DARMANITAN_GALAR_ZEN,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0556
        SPECIES_MARACTUS,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_DANCER
        )
    },
    { // 0557
        SPECIES_DWEBBLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0558
        SPECIES_CRUSTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0561
        SPECIES_SIGILYPH,
        INNATES(
            ABILITY_MAGIC_GUARD,
            ABILITY_TINTED_LENS,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0562
        SPECIES_YAMASK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0562
        SPECIES_YAMASK_GALAR,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0563
        SPECIES_COFAGRIGUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0564
        SPECIES_TIRTOUGA,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0565
        SPECIES_CARRACOSTA,
        INNATES(
            ABILITY_SOLID_ROCK,
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0566
        SPECIES_ARCHEN,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0567
        SPECIES_ARCHEOPS,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0568
        SPECIES_TRUBBISH,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0569
        SPECIES_GARBODOR,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0569
        SPECIES_GARBODOR_GMAX,
        INNATES(
            ABILITY_AFTERMATH,
            ABILITY_STENCH,
            ABILITY_STICKY_HOLD
        )
    },
    { // 0570
        SPECIES_ZORUA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0571
        SPECIES_ZOROARK,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0570
        SPECIES_ZORUA_HISUI,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0571
        SPECIES_ZOROARK_HISUI,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0572
        SPECIES_MINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SKILL_LINK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0573
        SPECIES_CINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SKILL_LINK,
            ABILITY_TECHNICIAN
        )
    },
    { // 0574
        SPECIES_GOTHITA,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_FRISK,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0575
        SPECIES_GOTHORITA,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_FRISK,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0576
        SPECIES_GOTHITELLE,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_FRISK,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0577
        SPECIES_SOLOSIS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_OVERCOAT,
            ABILITY_REGENERATOR
        )
    },
    { // 0578
        SPECIES_DUOSION,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_OVERCOAT,
            ABILITY_REGENERATOR
        )
    },
    { // 0579
        SPECIES_REUNICLUS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGIC_GUARD,
            ABILITY_OVERCOAT,
            ABILITY_REGENERATOR
        )
    },
    { // 0580
        SPECIES_DUCKLETT,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_HYDRATION,
            ABILITY_KEEN_EYE
        )
    },
    { // 0581
        SPECIES_SWANNA,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_HYDRATION,
            ABILITY_KEEN_EYE
        )
    },
    { // 0582
        SPECIES_VANILLITE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0583
        SPECIES_VANILLISH,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0584
        SPECIES_VANILLUXE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0585
        SPECIES_DEERLING_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0585
        SPECIES_DEERLING_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0587
        SPECIES_EMOLGA,
        INNATES(
            ABILITY_CHEEK_POUCH
        )
    },
    { // 0588
        SPECIES_KARRABLAST,
        INNATES(
            ABILITY_SHED_SKIN,
            ABILITY_SWARM
        )
    },
    { // 0589
        SPECIES_ESCAVALIER,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SHED_SKIN,
            ABILITY_SHELL_ARMOR,
            ABILITY_SWARM
        )
    },
    { // 0590
        SPECIES_FOONGUS,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_REGENERATOR
        )
    },
    { // 0591
        SPECIES_AMOONGUSS,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_REGENERATOR
        )
    },
    { // 0592
        SPECIES_FRILLISH,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0593
        SPECIES_JELLICENT,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0594
        SPECIES_ALOMOMOLA,
        INNATES(
            ABILITY_HEALER,
            ABILITY_HYDRATION,
            ABILITY_REGENERATOR
        )
    },
    { // 0595
        SPECIES_JOLTIK,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM,
            ABILITY_UNNERVE
        )
    },
    { // 0596
        SPECIES_GALVANTULA,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM,
            ABILITY_UNNERVE
        )
    },
    { // 0597
        SPECIES_FERROSEED,
        INNATES(
            ABILITY_IRON_BARBS
        )
    },
    { // 0598
        SPECIES_FERROTHORN,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_IRON_BARBS
        )
    },
    { // 0599
        SPECIES_KLINK,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0600
        SPECIES_KLANG,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0601
        SPECIES_KLINKLANG,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0602
        SPECIES_TYNAMO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0603
        SPECIES_EELEKTRIK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0605
        SPECIES_ELGYEM,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0606
        SPECIES_BEHEEYEM,
        INNATES(
            ABILITY_ANALYTIC,
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0607
        SPECIES_LITWICK,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0608
        SPECIES_LAMPENT,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0609
        SPECIES_CHANDELURE,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SHADOW_TAG
        )
    },
    { // 0610
        SPECIES_AXEW,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_UNNERVE
        )
    },
    { // 0611
        SPECIES_FRAXURE,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_UNNERVE
        )
    },
    { // 0612
        SPECIES_HAXORUS,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_UNNERVE
        )
    },
    { // 0613
        SPECIES_CUBCHOO,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0614
        SPECIES_BEARTIC,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0615
        SPECIES_CRYOGONAL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0616
        SPECIES_SHELMET,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_OVERCOAT,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0617
        SPECIES_ACCELGOR,
        INNATES(
            ABILITY_HYDRATION,
            ABILITY_OVERCOAT,
            ABILITY_STICKY_HOLD,
            ABILITY_UNBURDEN
        )
    },
    { // 0618
        SPECIES_STUNFISK,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_SAND_VEIL
        )
    },
    { // 0618
        SPECIES_STUNFISK_GALAR,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0619
        SPECIES_MIENFOO,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_RECKLESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0620
        SPECIES_MIENSHAO,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_RECKLESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0621
        SPECIES_DRUDDIGON,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0622
        SPECIES_GOLETT,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0623
        SPECIES_GOLURK,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0623
        SPECIES_GOLURK_MEGA,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_UNSEEN_FIST
        )
    },
    { // 0624
        SPECIES_PAWNIARD,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE
        )
    },
    { // 0625
        SPECIES_BISHARP,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE
        )
    },
    { // 0626
        SPECIES_BOUFFALANT,
        INNATES(
            ABILITY_RECKLESS
        )
    },
    { // 0627
        SPECIES_RUFFLET,
        INNATES(
            ABILITY_KEEN_EYE
        )
    },
    { // 0628
        SPECIES_BRAVIARY,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_KEEN_EYE
        )
    },
    { // 0628
        SPECIES_BRAVIARY_HISUI,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_TINTED_LENS
        )
    },
    { // 0629
        SPECIES_VULLABY,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_OVERCOAT
        )
    },
    { // 0630
        SPECIES_MANDIBUZZ,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_OVERCOAT
        )
    },
    { // 0631
        SPECIES_HEATMOR,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_WHITE_SMOKE
        )
    },
    { // 0632
        SPECIES_DURANT,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0633
        SPECIES_DEINO,
        INNATES(
            ABILITY_GLUTTONY
        )
    },
    { // 0634
        SPECIES_ZWEILOUS,
        INNATES(
            ABILITY_GLUTTONY
        )
    },
    { // 0635
        SPECIES_HYDREIGON,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_LEVITATE
        )
    },
    { // 0636
        SPECIES_LARVESTA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0637
        SPECIES_VOLCARONA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0638
        SPECIES_COBALION,
        INNATES(
            ABILITY_JUSTIFIED
        )
    },
    { // 0639
        SPECIES_TERRAKION,
        INNATES(
            ABILITY_JUSTIFIED
        )
    },
    { // 0640
        SPECIES_VIRIZION,
        INNATES(
            ABILITY_JUSTIFIED
        )
    },
    { // 0641
        SPECIES_TORNADUS_INCARNATE,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_PRANKSTER
        )
    },
    { // 0641
        SPECIES_TORNADUS_THERIAN,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0642
        SPECIES_THUNDURUS_INCARNATE,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_PRANKSTER
        )
    },
    { // 0642
        SPECIES_THUNDURUS_THERIAN,
        INNATES(
            ABILITY_DEFIANT
        )
    },
    { // 0643
        SPECIES_RESHIRAM,
        INNATES(
            ABILITY_TURBOBLAZE
        )
    },
    { // 0644
        SPECIES_ZEKROM,
        INNATES(
            ABILITY_TERAVOLT
        )
    },
    { // 0645
        SPECIES_LANDORUS_INCARNATE,
        INNATES(
            ABILITY_SAND_FORCE
        )
    },
    { // 0645
        SPECIES_LANDORUS_THERIAN,
        INNATES(
            ABILITY_INTIMIDATE
        )
    },
    { // 0646
        SPECIES_KYUREM,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 1104
        SPECIES_KYUREM_WHITE,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TURBOBLAZE
        )
    },
    { // 1105
        SPECIES_KYUREM_BLACK,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_TERAVOLT
        )
    },
    { // 0647
        SPECIES_KELDEO_ORDINARY,
        INNATES(
            ABILITY_JUSTIFIED
        )
    },
    { // 0647
        SPECIES_KELDEO_RESOLUTE,
        INNATES(
            ABILITY_JUSTIFIED
        )
    },
    { // 0648
        SPECIES_MELOETTA,
        INNATES(
            ABILITY_DANCER,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0648
        SPECIES_MELOETTA_PIROUETTE,
        INNATES(
            ABILITY_DANCER,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0649
        SPECIES_GENESECT,
        INNATES(
            ABILITY_DOWNLOAD
        )
    },
    { // 0649
        SPECIES_GENESECT_DOUSE,
        INNATES(
            ABILITY_DOWNLOAD
        )
    },
    { // 0649
        SPECIES_GENESECT_SHOCK,
        INNATES(
            ABILITY_DOWNLOAD
        )
    },
    { // 0649
        SPECIES_GENESECT_BURN,
        INNATES(
            ABILITY_DOWNLOAD
        )
    },
    { // 0649
        SPECIES_GENESECT_CHILL,
        INNATES(
            ABILITY_DOWNLOAD
        )
    },
    { // 0650
        SPECIES_CHESPIN,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_STURDY
        )
    },
    { // 0651
        SPECIES_QUILLADIN,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_STURDY
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_STURDY
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT_MEGA,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OVERGROW,
            ABILITY_STURDY
        )
    },
    { // 0653
        SPECIES_FENNEKIN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MAGICIAN
        )
    },
    { // 0654
        SPECIES_BRAIXEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MAGICIAN
        )
    },
    { // 0655
        SPECIES_DELPHOX,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_MAGICIAN
        )
    },
    { // 0655
        SPECIES_DELPHOX_MEGA,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_LEVITATE,
            ABILITY_MAGICIAN
        )
    },
    { // 0656
        SPECIES_FROAKIE,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0657
        SPECIES_FROGADIER,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA_ASH,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA_BATTLE_BOND,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA_MEGA,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_TECHNICIAN,
            ABILITY_TORRENT
        )
    },
    { // 0659
        SPECIES_BUNNELBY,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_HUGE_POWER,
            ABILITY_PICKUP
        )
    },
    { // 0660
        SPECIES_DIGGERSBY,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_HUGE_POWER,
            ABILITY_PICKUP
        )
    },
    { // 0661
        SPECIES_FLETCHLING,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_GALE_WINGS
        )
    },
    { // 0662
        SPECIES_FLETCHINDER,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_GALE_WINGS
        )
    },
    { // 0663
        SPECIES_TALONFLAME,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_GALE_WINGS
        )
    },
    { // 0664
        SPECIES_SCATTERBUG,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0665
        SPECIES_SPEWPA,
        INNATES(
            ABILITY_EFFECT_SPORE
        )
    },
    { // 0666
        SPECIES_VIVILLON,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_EFFECT_SPORE,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0667
        SPECIES_LITLEO,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_UNNERVE
        )
    },
    { // 0668
        SPECIES_PYROAR,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_UNNERVE
        )
    },
    { // 0668
        SPECIES_PYROAR_MEGA,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_UNNERVE
        )
    },
    { // 0669
        SPECIES_FLABEBE_RED,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0669
        SPECIES_FLABEBE_YELLOW,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0669
        SPECIES_FLABEBE_ORANGE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0669
        SPECIES_FLABEBE_BLUE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0669
        SPECIES_FLABEBE_WHITE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_RED,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_YELLOW,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_ORANGE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_BLUE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_WHITE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_ETERNAL,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0670
        SPECIES_FLOETTE_MEGA,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0671
        SPECIES_FLORGES_RED,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0671
        SPECIES_FLORGES_YELLOW,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0671
        SPECIES_FLORGES_ORANGE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0671
        SPECIES_FLORGES_BLUE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0671
        SPECIES_FLORGES_WHITE,
        INNATES(
            ABILITY_FLOWER_VEIL
        )
    },
    { // 0672
        SPECIES_SKIDDO,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GRASS_PELT,
            ABILITY_OVERCOAT
        )
    },
    { // 0673
        SPECIES_GOGOAT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_GRASS_PELT,
            ABILITY_OVERCOAT
        )
    },
    { // 0674
        SPECIES_PANCHAM,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_MOLD_BREAKER,
            ABILITY_SCRAPPY
        )
    },
    { // 0676
        SPECIES_PANGORO,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_MOLD_BREAKER,
            ABILITY_SCRAPPY
        )
    },
    { // 0676
        SPECIES_FURFROU,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_HEART,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_STAR,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DIAMOND,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DEBUTANTE,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_MATRON,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_DANDY,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_LA_REINE,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_KABUKI,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0676
        SPECIES_FURFROU_PHARAOH,
        INNATES(
            ABILITY_FUR_COAT
        )
    },
    { // 0677
        SPECIES_ESPURR,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_KEEN_EYE,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_KEEN_EYE,
            ABILITY_OWN_TEMPO,
            ABILITY_PRANKSTER
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M_MEGA,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_KEEN_EYE,
            ABILITY_OWN_TEMPO,
            ABILITY_PRANKSTER
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_F,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_INFILTRATOR,
            ABILITY_KEEN_EYE,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_F_MEGA,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_INFILTRATOR,
            ABILITY_KEEN_EYE,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0679
        SPECIES_HONEDGE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0680
        SPECIES_DOUBLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH_BLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0684
        SPECIES_SWIRLIX,
        INNATES(
            ABILITY_SWEET_VEIL,
            ABILITY_UNBURDEN
        )
    },
    { // 0685
        SPECIES_SLURPUFF,
        INNATES(
            ABILITY_SWEET_VEIL,
            ABILITY_UNBURDEN
        )
    },
    { // 0686
        SPECIES_INKAY,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0687
        SPECIES_MALAMAR,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0687
        SPECIES_MALAMAR_MEGA,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE,
            ABILITY_SUCTION_CUPS
        )
    },
    { // 0688
        SPECIES_BINACLE,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0689
        SPECIES_BARBARACLE,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0689
        SPECIES_BARBARACLE_MEGA,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_SNIPER,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0690
        SPECIES_SKRELP,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_LIQUID_OOZE
        )
    },
    { // 0691
        SPECIES_DRAGALGE,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_LIQUID_OOZE
        )
    },
    { // 0691
        SPECIES_DRAGALGE_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_LIQUID_OOZE,
            ABILITY_REGENERATOR
        )
    },
    { // 0692
        SPECIES_CLAUNCHER,
        INNATES(
            ABILITY_MEGA_LAUNCHER,
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0693
        SPECIES_CLAWITZER,
        INNATES(
            ABILITY_MEGA_LAUNCHER,
            ABILITY_SNIPER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0694
        SPECIES_HELIOPTILE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0695
        SPECIES_HELIOLISK,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0696
        SPECIES_TYRUNT,
        INNATES(
            ABILITY_STRONG_JAW,
            ABILITY_STURDY
        )
    },
    { // 0697
        SPECIES_TYRANTRUM,
        INNATES(
            ABILITY_ROCK_HEAD,
            ABILITY_STRONG_JAW,
            ABILITY_STURDY
        )
    },
    { // 0700
        SPECIES_SYLVEON,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_CUTE_CHARM
        )
    },
    { // 0698
        SPECIES_AMAURA,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0699
        SPECIES_AURORUS,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0701
        SPECIES_HAWLUCHA,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MOLD_BREAKER,
            ABILITY_UNBURDEN
        )
    },
    { // 0701
        SPECIES_HAWLUCHA_MEGA,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MOLD_BREAKER,
            ABILITY_UNBURDEN
        )
    },
    { // 0703
        SPECIES_CARBINK,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0705
        SPECIES_SLIGGOO_HISUI,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_HYDRATION,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0706
        SPECIES_GOODRA_HISUI,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_HYDRATION,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0707
        SPECIES_KLEFKI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGICIAN,
            ABILITY_PRANKSTER
        )
    },
    { // 0708
        SPECIES_PHANTUMP,
        INNATES(
            ABILITY_FRISK,
            ABILITY_HARVEST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0709
        SPECIES_TREVENANT,
        INNATES(
            ABILITY_FRISK,
            ABILITY_HARVEST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SMALL,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_LARGE,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SUPER,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0711
        SPECIES_GOURGEIST,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SMALL,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0711
        SPECIES_GOURGEIST_LARGE,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SUPER,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INSOMNIA,
            ABILITY_LEVITATE,
            ABILITY_PICKUP
        )
    },
    { // 0712
        SPECIES_BERGMITE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OWN_TEMPO,
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OWN_TEMPO,
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG_HISUI,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_OWN_TEMPO,
            ABILITY_STRONG_JAW,
            ABILITY_STURDY
        )
    },
    { // 0714
        SPECIES_NOIBAT,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INFILTRATOR,
            ABILITY_TELEPATHY
        )
    },
    { // 0715
        SPECIES_NOIVERN,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INFILTRATOR,
            ABILITY_TELEPATHY
        )
    },
    { // 0716
        SPECIES_XERNEAS_NEUTRAL,
        INNATES(
            ABILITY_FLOWER_VEIL,
            ABILITY_HEALER,
            ABILITY_PRESSURE
        )
    },
    { // 0716
        SPECIES_XERNEAS_ACTIVE,
        INNATES(
            ABILITY_FLOWER_VEIL,
            ABILITY_HEALER,
            ABILITY_PRESSURE
        )
    },
    { // 0717
        SPECIES_YVELTAL,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_UNNERVE
        )
    },
    { // 0718
        SPECIES_ZYGARDE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_MEGA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_10,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_10_POWER_CONSTRUCT,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_50_POWER_CONSTRUCT,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_COMPLETE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0719
        SPECIES_DIANCIE,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0719
        SPECIES_DIANCIE_MEGA,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_LEVITATE,
            ABILITY_MAGIC_BOUNCE
        )
    },
    { // 0720
        SPECIES_HOOPA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGICIAN,
            ABILITY_PRANKSTER
        )
    },
    { // 0720
        SPECIES_HOOPA_UNBOUND,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_MAGICIAN,
            ABILITY_PRANKSTER
        )
    },
    { // 0721
        SPECIES_VOLCANION,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_STEAM_ENGINE
        )
    },
    { // 0722
        SPECIES_ROWLET,
        INNATES(
            ABILITY_LONG_REACH,
            ABILITY_OVERGROW
        )
    },
    { // 0723
        SPECIES_DARTRIX,
        INNATES(
            ABILITY_LONG_REACH,
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE,
        INNATES(
            ABILITY_LONG_REACH,
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE_HISUI,
        INNATES(
            ABILITY_LONG_REACH,
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0725
        SPECIES_LITTEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0726
        SPECIES_TORRACAT,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0727
        SPECIES_INCINEROAR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_INTIMIDATE
        )
    },
    { // 0728
        SPECIES_POPPLIO,
        INNATES(
            ABILITY_DANCER,
            ABILITY_PUNK_ROCK,
            ABILITY_TORRENT
        )
    },
    { // 0729
        SPECIES_BRIONNE,
        INNATES(
            ABILITY_DANCER,
            ABILITY_PUNK_ROCK,
            ABILITY_TORRENT
        )
    },
    { // 0730
        SPECIES_PRIMARINA,
        INNATES(
            ABILITY_DANCER,
            ABILITY_PUNK_ROCK,
            ABILITY_TORRENT
        )
    },
    { // 0731
        SPECIES_PIKIPEK,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKUP,
            ABILITY_SKILL_LINK
        )
    },
    { // 0732
        SPECIES_TRUMBEAK,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKUP,
            ABILITY_SKILL_LINK
        )
    },
    { // 0733
        SPECIES_TOUCANNON,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PICKUP,
            ABILITY_SKILL_LINK
        )
    },
    { // 0734
        SPECIES_YUNGOOS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW
        )
    },
    { // 0735
        SPECIES_GUMSHOOS,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_STAKEOUT,
            ABILITY_STRONG_JAW
        )
    },
    { // 0735
        SPECIES_GUMSHOOS_TOTEM,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0736
        SPECIES_GRUBBIN,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0737
        SPECIES_CHARJABUG,
        INNATES(
            ABILITY_BATTERY,
            ABILITY_SWARM
        )
    },
    { // 0738
        SPECIES_VIKAVOLT,
        INNATES(
            ABILITY_BATTERY,
            ABILITY_LEVITATE,
            ABILITY_SWARM
        )
    },
    { // 0738
        SPECIES_VIKAVOLT_TOTEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0739
        SPECIES_CRABRAWLER,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_HYPER_CUTTER,
            ABILITY_IRON_FIST
        )
    },
    { // 0740
        SPECIES_CRABOMINABLE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_HYPER_CUTTER,
            ABILITY_IRON_FIST
        )
    },
    { // 0740
        SPECIES_CRABOMINABLE_MEGA,
        INNATES(
            ABILITY_IRON_FIST
        )
    },
    { // 0741
        SPECIES_ORICORIO_BAILE,
        INNATES(
            ABILITY_DANCER
        )
    },
    { // 0741
        SPECIES_ORICORIO_POM_POM,
        INNATES(
            ABILITY_DANCER
        )
    },
    { // 0741
        SPECIES_ORICORIO_PAU,
        INNATES(
            ABILITY_DANCER
        )
    },
    { // 0741
        SPECIES_ORICORIO_SENSU,
        INNATES(
            ABILITY_DANCER
        )
    },
    { // 0742
        SPECIES_CUTIEFLY,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_SHIELD_DUST,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0743
        SPECIES_RIBOMBEE,
        INNATES(
            ABILITY_EFFECT_SPORE,
            ABILITY_SHIELD_DUST,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0744
        SPECIES_ROCKRUFF,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STEADFAST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0744
        SPECIES_ROCKRUFF_OWN_TEMPO,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STEADFAST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0745
        SPECIES_LYCANROC_MIDDAY,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SAND_RUSH,
            ABILITY_STEADFAST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0745
        SPECIES_LYCANROC_MIDNIGHT,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STEADFAST,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0745
        SPECIES_LYCANROC_DUSK,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_STEADFAST,
            ABILITY_TOUGH_CLAWS,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0746
        SPECIES_WISHIWASHI_SOLO,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0746
        SPECIES_WISHIWASHI_SCHOOL,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0747
        SPECIES_MAREANIE,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MERCILESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0748
        SPECIES_TOXAPEX,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MERCILESS,
            ABILITY_REGENERATOR
        )
    },
    { // 0749
        SPECIES_MUDBRAY,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_OWN_TEMPO,
            ABILITY_STAMINA
        )
    },
    { // 0750
        SPECIES_MUDSDALE,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_OWN_TEMPO,
            ABILITY_STAMINA
        )
    },
    { // 0751
        SPECIES_DEWPIDER,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0752
        SPECIES_ARAQUANID,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_SWIFT_SWIM,
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0752
        SPECIES_ARAQUANID_TOTEM,
        INNATES(
            ABILITY_WATER_BUBBLE
        )
    },
    { // 0753
        SPECIES_FOMANTIS,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_SHARPNESS
        )
    },
    { // 0754
        SPECIES_LURANTIS,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEAF_GUARD,
            ABILITY_SHARPNESS
        )
    },
    { // 0757
        SPECIES_SALANDIT,
        INNATES(
            ABILITY_CORROSION,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0758
        SPECIES_SALAZZLE,
        INNATES(
            ABILITY_CORROSION,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0758
        SPECIES_SALAZZLE_TOTEM,
        INNATES(
            ABILITY_CORROSION
        )
    },
    { // 0759
        SPECIES_STUFFUL,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0760
        SPECIES_BEWEAR,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_UNNERVE
        )
    },
    { // 0761
        SPECIES_BOUNSWEET,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OBLIVIOUS,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0762
        SPECIES_STEENEE,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OBLIVIOUS,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0763
        SPECIES_TSAREENA,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_OBLIVIOUS,
            ABILITY_QUEENLY_MAJESTY,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0764
        SPECIES_COMFEY,
        INNATES(
            ABILITY_FLOWER_VEIL,
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE,
            ABILITY_TRIAGE
        )
    },
    { // 0765
        SPECIES_ORANGURU,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_TELEPATHY
        )
    },
    { // 0766
        SPECIES_PASSIMIAN,
        INNATES(
            ABILITY_DEFIANT
        )
    },
    { // 0767
        SPECIES_WIMPOD,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0768
        SPECIES_GOLISOPOD,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0769
        SPECIES_SANDYGAST,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_WATER_COMPACTION
        )
    },
    { // 0770
        SPECIES_PALOSSAND,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_WATER_COMPACTION
        )
    },
    { // 0771
        SPECIES_PYUKUMUKU,
        INNATES(
            ABILITY_INNARDS_OUT,
            ABILITY_UNAWARE
        )
    },
    { // 0772
        SPECIES_TYPE_NULL,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0773
        SPECIES_SILVALLY_NORMAL,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0773
        SPECIES_SILVALLY_DRAGON,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0773
        SPECIES_SILVALLY_FAIRY,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0773
        SPECIES_SILVALLY_STEEL,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0774
        SPECIES_MINIOR,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0775
        SPECIES_KOMALA,
        INNATES(
            ABILITY_COMATOSE,
            ABILITY_UNAWARE
        )
    },
    { // 0776
        SPECIES_TURTONATOR,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0777
        SPECIES_TOGEDEMARU,
        INNATES(
            ABILITY_IRON_BARBS,
            ABILITY_STURDY
        )
    },
    { // 0778
        SPECIES_MIMIKYU_DISGUISED,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0778
        SPECIES_MIMIKYU_BUSTED,
        INNATES(
            ABILITY_CURSED_BODY
        )
    },
    { // 0779
        SPECIES_BRUXISH,
        INNATES(
            ABILITY_DAZZLING,
            ABILITY_STRONG_JAW,
            ABILITY_WONDER_SKIN
        )
    },
    { // 0780
        SPECIES_DRAMPA,
        INNATES(
            ABILITY_BERSERK
        )
    },
    { // 0781
        SPECIES_DHELMISE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STEELWORKER
        )
    },
    { // 0782
        SPECIES_JANGMO_O,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_PUNK_ROCK
        )
    },
    { // 0783
        SPECIES_HAKAMO_O,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_PUNK_ROCK
        )
    },
    { // 0784
        SPECIES_KOMMO_O,
        INNATES(
            ABILITY_OVERCOAT,
            ABILITY_PUNK_ROCK
        )
    },
    { // 0785
        SPECIES_TAPU_KOKO,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0786
        SPECIES_TAPU_LELE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0787
        SPECIES_TAPU_BULU,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0788
        SPECIES_TAPU_FINI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TELEPATHY
        )
    },
    { // 0789
        SPECIES_COSMOG,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_UNAWARE
        )
    },
    { // 0790
        SPECIES_COSMOEM,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_UNAWARE
        )
    },
    { // 0791
        SPECIES_SOLGALEO,
        INNATES(
            ABILITY_FULL_METAL_BODY,
            ABILITY_LEVITATE,
            ABILITY_STURDY,
            ABILITY_UNAWARE
        )
    },
    { // 0792
        SPECIES_LUNALA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SHADOW_SHIELD,
            ABILITY_STURDY,
            ABILITY_UNAWARE
        )
    },
    { // 0793
        SPECIES_NIHILEGO,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0794
        SPECIES_BUZZWOLE,
        INNATES(
            ABILITY_BEAST_BOOST
        )
    },
    { // 0795
        SPECIES_PHEROMOSA,
        INNATES(
            ABILITY_BEAST_BOOST
        )
    },
    { // 0796
        SPECIES_XURKITREE,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0797
        SPECIES_CELESTEELA,
        INNATES(
            ABILITY_BEAST_BOOST
        )
    },
    { // 0798
        SPECIES_KARTANA,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0799
        SPECIES_GUZZLORD,
        INNATES(
            ABILITY_BEAST_BOOST
        )
    },
    { // 0800
        SPECIES_NECROZMA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRISM_ARMOR
        )
    },
    { // 0800
        SPECIES_NECROZMA_DUSK_MANE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRISM_ARMOR
        )
    },
    { // 0800
        SPECIES_NECROZMA_DAWN_WINGS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRISM_ARMOR
        )
    },
    { // 0800
        SPECIES_NECROZMA_ULTRA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NEUROFORCE
        )
    },
    { // 0801
        SPECIES_MAGEARNA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SOUL_HEART
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SOUL_HEART
        )
    },
    { // 0801
        SPECIES_MAGEARNA_MEGA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SOUL_HEART
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL_MEGA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_SOUL_HEART
        )
    },
    { // 0802
        SPECIES_MARSHADOW,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 0803
        SPECIES_POIPOLE,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0804
        SPECIES_NAGANADEL,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0805
        SPECIES_STAKATAKA,
        INNATES(
            ABILITY_BEAST_BOOST
        )
    },
    { // 0806
        SPECIES_BLACEPHALON,
        INNATES(
            ABILITY_BEAST_BOOST,
            ABILITY_LEVITATE
        )
    },
    { // 0807
        SPECIES_ZERAORA,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_LIMBER
        )
    },
    { // 0808
        SPECIES_MELTAN,
        INNATES(
            ABILITY_MAGNET_PULL
        )
    },
    { // 0809
        SPECIES_MELMETAL,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_MAGNET_PULL
        )
    },
    { // 0809
        SPECIES_MELMETAL_GMAX,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_MAGNET_PULL
        )
    },
    { // 0810
        SPECIES_GROOKEY,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0811
        SPECIES_THWACKEY,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0812
        SPECIES_RILLABOOM,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0812
        SPECIES_RILLABOOM_GMAX,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SCRAPPY
        )
    },
    { // 0813
        SPECIES_SCORBUNNY,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST,
            ABILITY_THICK_FAT
        )
    },
    { // 0814
        SPECIES_RABOOT,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST,
            ABILITY_THICK_FAT
        )
    },
    { // 0815
        SPECIES_CINDERACE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST,
            ABILITY_THICK_FAT
        )
    },
    { // 0815
        SPECIES_CINDERACE_GMAX,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST,
            ABILITY_THICK_FAT
        )
    },
    { // 0816
        SPECIES_SOBBLE,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0817
        SPECIES_DRIZZILE,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON_GMAX,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_SNIPER,
            ABILITY_TORRENT
        )
    },
    { // 0819
        SPECIES_SKWOVET,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_GLUTTONY,
            ABILITY_HARVEST
        )
    },
    { // 0820
        SPECIES_GREEDENT,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_GLUTTONY,
            ABILITY_HARVEST
        )
    },
    { // 0821
        SPECIES_ROOKIDEE,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_UNNERVE
        )
    },
    { // 0822
        SPECIES_CORVISQUIRE,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_UNNERVE
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_MIRROR_ARMOR,
            ABILITY_PRESSURE,
            ABILITY_UNNERVE
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT_GMAX,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_MIRROR_ARMOR,
            ABILITY_PRESSURE,
            ABILITY_UNNERVE
        )
    },
    { // 0824
        SPECIES_BLIPBUG,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM,
            ABILITY_TELEPATHY
        )
    },
    { // 0825
        SPECIES_DOTTLER,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SWARM,
            ABILITY_TELEPATHY
        )
    },
    { // 0826
        SPECIES_ORBEETLE,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_FRISK,
            ABILITY_SWARM,
            ABILITY_TELEPATHY
        )
    },
    { // 0826
        SPECIES_ORBEETLE_GMAX,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_FRISK,
            ABILITY_SWARM,
            ABILITY_TELEPATHY
        )
    },
    { // 0827
        SPECIES_NICKIT,
        INNATES(
            ABILITY_PRANKSTER,
            ABILITY_STAKEOUT,
            ABILITY_UNBURDEN
        )
    },
    { // 0828
        SPECIES_THIEVUL,
        INNATES(
            ABILITY_PRANKSTER,
            ABILITY_STAKEOUT,
            ABILITY_UNBURDEN
        )
    },
    { // 0829
        SPECIES_GOSSIFLEUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_REGENERATOR
        )
    },
    { // 0830
        SPECIES_ELDEGOSS,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_EFFECT_SPORE,
            ABILITY_REGENERATOR
        )
    },
    { // 0833
        SPECIES_CHEWTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STRONG_JAW,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0831
        SPECIES_WOOLOO,
        INNATES(
            ABILITY_STEADFAST,
            ABILITY_THICK_FAT
        )
    },
    { // 0832
        SPECIES_DUBWOOL,
        INNATES(
            ABILITY_STEADFAST,
            ABILITY_THICK_FAT
        )
    },
    { // 0835
        SPECIES_YAMPER,
        INNATES(
            ABILITY_QUICK_FEET,
            ABILITY_RATTLED
        )
    },
    { // 0836
        SPECIES_BOLTUND,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_QUICK_FEET,
            ABILITY_RATTLED,
            ABILITY_STRONG_JAW
        )
    },
    { // 0837
        SPECIES_ROLYCOLY,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGMA_ARMOR,
            ABILITY_STEAM_ENGINE
        )
    },
    { // 0838
        SPECIES_CARKOL,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGMA_ARMOR,
            ABILITY_STEAM_ENGINE
        )
    },
    { // 0839
        SPECIES_COALOSSAL,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGMA_ARMOR,
            ABILITY_STEAM_ENGINE
        )
    },
    { // 0839
        SPECIES_COALOSSAL_GMAX,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_MAGMA_ARMOR,
            ABILITY_STEAM_ENGINE
        )
    },
    { // 0840
        SPECIES_APPLIN,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_RIPEN
        )
    },
    { // 0841
        SPECIES_FLAPPLE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_RIPEN
        )
    },
    { // 0841
        SPECIES_FLAPPLE_GMAX,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_RIPEN
        )
    },
    { // 0842
        SPECIES_APPLETUN,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_RIPEN,
            ABILITY_THICK_FAT
        )
    },
    { // 0842
        SPECIES_APPLETUN_GMAX,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_RIPEN,
            ABILITY_THICK_FAT
        )
    },
    { // 0843
        SPECIES_SILICOBRA,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_SHED_SKIN
        )
    },
    { // 0844
        SPECIES_SANDACONDA,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_SHED_SKIN
        )
    },
    { // 0844
        SPECIES_SANDACONDA_GMAX,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_SHED_SKIN
        )
    },
    { // 0845
        SPECIES_CRAMORANT,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_OBLIVIOUS
        )
    },
    { // 0846
        SPECIES_ARROKUDA,
        INNATES(
            ABILITY_PROPELLER_TAIL,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0847
        SPECIES_BARRASKEWDA,
        INNATES(
            ABILITY_PROPELLER_TAIL,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0848
        SPECIES_TOXEL,
        INNATES(
            ABILITY_RATTLED
        )
    },
    { // 0849
        SPECIES_TOXTRICITY,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_AMPED_GMAX,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_LOW_KEY,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN
        )
    },
    { // 0849
        SPECIES_TOXTRICITY_LOW_KEY_GMAX,
        INNATES(
            ABILITY_PUNK_ROCK,
            ABILITY_RATTLED,
            ABILITY_TECHNICIAN
        )
    },
    { // 0850
        SPECIES_SIZZLIPEDE,
        INNATES(
            ABILITY_SWARM,
            ABILITY_WHITE_SMOKE
        )
    },
    { // 0851
        SPECIES_CENTISKORCH,
        INNATES(
            ABILITY_SWARM,
            ABILITY_WHITE_SMOKE
        )
    },
    { // 0851
        SPECIES_CENTISKORCH_GMAX,
        INNATES(
            ABILITY_SWARM,
            ABILITY_WHITE_SMOKE
        )
    },
    { // 0852
        SPECIES_CLOBBOPUS,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_LIMBER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0853
        SPECIES_GRAPPLOCT,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_LIMBER,
            ABILITY_TECHNICIAN
        )
    },
    { // 0854
        SPECIES_SINISTEA,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0854
        SPECIES_SINISTEA_ANTIQUE,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST_ANTIQUE,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0859
        SPECIES_IMPIDIMP,
        INNATES(
            ABILITY_FRISK,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER
        )
    },
    { // 0860
        SPECIES_MORGREM,
        INNATES(
            ABILITY_FRISK,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL,
        INNATES(
            ABILITY_FRISK,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL_GMAX,
        INNATES(
            ABILITY_FRISK,
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER
        )
    },
    { // 0862
        SPECIES_OBSTAGOON,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_GLUTTONY,
            ABILITY_GUTS,
            ABILITY_PICKUP,
            ABILITY_QUICK_FEET,
            ABILITY_RECKLESS
        )
    },
    { // 0863
        SPECIES_PERRSERKER,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_MOXIE,
            ABILITY_PICKUP,
            ABILITY_STEELY_SPIRIT,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNNERVE
        )
    },
    { // 0865
        SPECIES_SIRFETCHD,
        INNATES(
            ABILITY_JUSTIFIED,
            ABILITY_SCRAPPY,
            ABILITY_SHARPNESS,
            ABILITY_STEADFAST
        )
    },
    { // 0866
        SPECIES_MR_RIME,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH,
            ABILITY_TANGLED_FEET,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0867
        SPECIES_RUNERIGUS,
        INNATES(
            ABILITY_CURSED_BODY,
            ABILITY_LEVITATE
        )
    },
    { // 0868
        SPECIES_MILCERY,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0869
        SPECIES_ALCREMIE_STRAWBERRY_VANILLA_CREAM,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0869
        SPECIES_ALCREMIE_GMAX,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_SWEET_VEIL
        )
    },
    { // 0870
        SPECIES_FALINKS,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_DEFIANT
        )
    },
    { // 0870
        SPECIES_FALINKS_MEGA,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_DEFIANT
        )
    },
    { // 0871
        SPECIES_PINCURCHIN,
        INNATES(
            ABILITY_ROUGH_SKIN
        )
    },
    { // 0872
        SPECIES_SNOM,
        INNATES(
            ABILITY_ICE_SCALES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0873
        SPECIES_FROSMOTH,
        INNATES(
            ABILITY_ICE_SCALES,
            ABILITY_SHIELD_DUST
        )
    },
    { // 0874
        SPECIES_STONJOURNER,
        INNATES(
            ABILITY_POWER_SPOT,
            ABILITY_STURDY
        )
    },
    { // 0875
        SPECIES_EISCUE_ICE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0875
        SPECIES_EISCUE_NOICE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0876
        SPECIES_INDEEDEE_F,
        INNATES(
            ABILITY_OWN_TEMPO
        )
    },
    { // 0876
        SPECIES_INDEEDEE_M,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0877
        SPECIES_MORPEKO_FULL_BELLY,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_GLUTTONY
        )
    },
    { // 0877
        SPECIES_MORPEKO_HANGRY,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_GLUTTONY
        )
    },
    { // 0878
        SPECIES_CUFANT,
        INNATES(
            ABILITY_HEAVY_METAL,
            ABILITY_STURDY
        )
    },
    { // 0879
        SPECIES_COPPERAJAH,
        INNATES(
            ABILITY_HEAVY_METAL,
            ABILITY_STURDY
        )
    },
    { // 0879
        SPECIES_COPPERAJAH_GMAX,
        INNATES(
            ABILITY_HEAVY_METAL,
            ABILITY_STURDY
        )
    },
    { // 0880
        SPECIES_DRACOZOLT,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0881
        SPECIES_ARCTOZOLT,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0882
        SPECIES_DRACOVISH,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_STRONG_JAW
        )
    },
    { // 0883
        SPECIES_ARCTOVISH,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0884
        SPECIES_DURALUDON,
        INNATES(
            ABILITY_LIGHT_METAL,
            ABILITY_STALWART
        )
    },
    { // 0884
        SPECIES_DURALUDON_GMAX,
        INNATES(
            ABILITY_LIGHT_METAL,
            ABILITY_STALWART
        )
    },
    { // 0885
        SPECIES_DREEPY,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE
        )
    },
    { // 0886
        SPECIES_DRAKLOAK,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE
        )
    },
    { // 0887
        SPECIES_DRAGAPULT,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_CURSED_BODY,
            ABILITY_INFILTRATOR,
            ABILITY_LEVITATE
        )
    },
    { // 0888
        SPECIES_ZACIAN,
        INNATES(
            ABILITY_INTREPID_SWORD
        )
    },
    { // 0888
        SPECIES_ZACIAN_CROWNED,
        INNATES(
            ABILITY_INTREPID_SWORD
        )
    },
    { // 0889
        SPECIES_ZAMAZENTA,
        INNATES(
            ABILITY_DAUNTLESS_SHIELD
        )
    },
    { // 0889
        SPECIES_ZAMAZENTA_CROWNED,
        INNATES(
            ABILITY_DAUNTLESS_SHIELD
        )
    },
    { // 0890
        SPECIES_ETERNATUS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0890
        SPECIES_ETERNATUS_ETERNAMAX,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0891
        SPECIES_KUBFU,
        INNATES(
            ABILITY_INNER_FOCUS
        )
    },
    { // 0892
        SPECIES_URSHIFU,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_UNSEEN_FIST
        )
    },
    { // 0892
        SPECIES_URSHIFU_RAPID_STRIKE,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_UNSEEN_FIST
        )
    },
    { // 0892
        SPECIES_URSHIFU_SINGLE_STRIKE_GMAX,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_UNSEEN_FIST
        )
    },
    { // 0892
        SPECIES_URSHIFU_RAPID_STRIKE_GMAX,
        INNATES(
            ABILITY_INNER_FOCUS,
            ABILITY_UNSEEN_FIST
        )
    },
    { // 0893
        SPECIES_ZARUDE,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0893
        SPECIES_ZARUDE_DADA,
        INNATES(
            ABILITY_LEAF_GUARD,
            ABILITY_TOUGH_CLAWS
        )
    },
    { // 0894
        SPECIES_REGIELEKI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_TRANSISTOR
        )
    },
    { // 0895
        SPECIES_REGIDRAGO,
        INNATES(
            ABILITY_DRAGONS_MAW
        )
    },
    { // 0896
        SPECIES_GLASTRIER,
        INNATES(
            ABILITY_CHILLING_NEIGH,
            ABILITY_ICE_BODY
        )
    },
    { // 0897
        SPECIES_SPECTRIER,
        INNATES(
            ABILITY_GRIM_NEIGH,
            ABILITY_INFILTRATOR
        )
    },
    { // 0898
        SPECIES_CALYREX,
        INNATES(
            ABILITY_UNNERVE
        )
    },
    { // 0898
        SPECIES_CALYREX_ICE,
        INNATES(
            ABILITY_ICE_BODY,
            ABILITY_UNNERVE
        )
    },
    { // 0898
        SPECIES_CALYREX_SHADOW,
        INNATES(
            ABILITY_INFILTRATOR,
            ABILITY_UNNERVE
        )
    },
    { // 0899
        SPECIES_WYRDEER,
        INNATES(
            ABILITY_FRISK,
            ABILITY_INTIMIDATE
        )
    },
    { // 0900
        SPECIES_KLEAVOR,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_SHARPNESS,
            ABILITY_SOLID_ROCK,
            ABILITY_SWARM
        )
    },
    { // 0901
        SPECIES_URSALUNA,
        INNATES(
            ABILITY_GUTS,
            ABILITY_THICK_FAT,
            ABILITY_UNNERVE
        )
    },
    { // 0901
        SPECIES_URSALUNA_BLOODMOON,
        INNATES(
            ABILITY_MINDS_EYE,
            ABILITY_THICK_FAT
        )
    },
    { // 0902
        SPECIES_BASCULEGION_M,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_MOLD_BREAKER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0902
        SPECIES_BASCULEGION_F,
        INNATES(
            ABILITY_ADAPTABILITY,
            ABILITY_MOLD_BREAKER,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0903
        SPECIES_SNEASLER,
        INNATES(
            ABILITY_KEEN_EYE,
            ABILITY_PRESSURE,
            ABILITY_TOUGH_CLAWS,
            ABILITY_UNBURDEN
        )
    },
    { // 0904
        SPECIES_OVERQWIL,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_ROUGH_SKIN,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0905
        SPECIES_ENAMORUS_INCARNATE,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_HEALER
        )
    },
    { // 0905
        SPECIES_ENAMORUS_THERIAN,
        INNATES(
            ABILITY_HEALER,
            ABILITY_OVERCOAT
        )
    },
    { // 0906
        SPECIES_SPRIGATITO,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MAGICIAN,
            ABILITY_OVERGROW
        )
    },
    { // 0907
        SPECIES_FLORAGATO,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MAGICIAN,
            ABILITY_OVERGROW
        )
    },
    { // 0908
        SPECIES_MEOWSCARADA,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_MAGICIAN,
            ABILITY_OVERGROW
        )
    },
    { // 0909
        SPECIES_FUECOCO,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0910
        SPECIES_CROCALOR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0911
        SPECIES_SKELEDIRGE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0912
        SPECIES_QUAXLY,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_TORRENT
        )
    },
    { // 0913
        SPECIES_QUAXWELL,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_TORRENT
        )
    },
    { // 0914
        SPECIES_QUAQUAVAL,
        INNATES(
            ABILITY_MOXIE,
            ABILITY_TORRENT
        )
    },
    { // 0915
        SPECIES_LECHONK,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_GLUTTONY,
            ABILITY_THICK_FAT
        )
    },
    { // 0916
        SPECIES_OINKOLOGNE_M,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_GLUTTONY,
            ABILITY_THICK_FAT
        )
    },
    { // 0916
        SPECIES_OINKOLOGNE_F,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_GLUTTONY,
            ABILITY_THICK_FAT
        )
    },
    { // 0917
        SPECIES_TAROUNTULA,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_STAKEOUT
        )
    },
    { // 0918
        SPECIES_SPIDOPS,
        INNATES(
            ABILITY_INSOMNIA,
            ABILITY_STAKEOUT
        )
    },
    { // 0919
        SPECIES_NYMBLE,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0920
        SPECIES_LOKIX,
        INNATES(
            ABILITY_SWARM,
            ABILITY_TINTED_LENS
        )
    },
    { // 0921
        SPECIES_PAWMI,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0922
        SPECIES_PAWMO,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0923
        SPECIES_PAWMOT,
        INNATES(
            ABILITY_IRON_FIST,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0924
        SPECIES_TANDEMAUS,
        INNATES(
            ABILITY_OWN_TEMPO,
            ABILITY_PICKUP
        )
    },
    { // 0925
        SPECIES_MAUSHOLD,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_FRIEND_GUARD,
            ABILITY_OWN_TEMPO,
            ABILITY_PICKUP,
            ABILITY_TECHNICIAN
        )
    },
    { // 0925
        SPECIES_MAUSHOLD_FOUR,
        INNATES(
            ABILITY_CHEEK_POUCH,
            ABILITY_FRIEND_GUARD,
            ABILITY_OWN_TEMPO,
            ABILITY_PICKUP,
            ABILITY_TECHNICIAN
        )
    },
    { // 0926
        SPECIES_FIDOUGH,
        INNATES(
            ABILITY_OWN_TEMPO
        )
    },
    { // 0927
        SPECIES_DACHSBUN,
        INNATES(
            ABILITY_AROMA_VEIL,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY_BLUE,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY_YELLOW,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE
        )
    },
    { // 0931
        SPECIES_SQUAWKABILLY_WHITE,
        INNATES(
            ABILITY_GUTS,
            ABILITY_INTIMIDATE
        )
    },
    { // 0932
        SPECIES_NACLI,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_PURIFYING_SALT,
            ABILITY_STURDY
        )
    },
    { // 0933
        SPECIES_NACLSTACK,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_PURIFYING_SALT,
            ABILITY_STURDY
        )
    },
    { // 0934
        SPECIES_GARGANACL,
        INNATES(
            ABILITY_CLEAR_BODY,
            ABILITY_PURIFYING_SALT,
            ABILITY_STURDY
        )
    },
    { // 0935
        SPECIES_CHARCADET,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0936
        SPECIES_ARMAROUGE,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_MEGA_LAUNCHER
        )
    },
    { // 0937
        SPECIES_CERULEDGE,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SHARPNESS
        )
    },
    { // 0938
        SPECIES_TADBULB,
        INNATES(
            ABILITY_OWN_TEMPO
        )
    },
    { // 0939
        SPECIES_BELLIBOLT,
        INNATES(
            ABILITY_ELECTROMORPHOSIS,
            ABILITY_OWN_TEMPO
        )
    },
    { // 0940
        SPECIES_WATTREL,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_WIND_POWER
        )
    },
    { // 0941
        SPECIES_KILOWATTREL,
        INNATES(
            ABILITY_COMPETITIVE,
            ABILITY_WIND_POWER
        )
    },
    { // 0942
        SPECIES_MASCHIFF,
        INNATES(
            ABILITY_INTIMIDATE,
            ABILITY_STAKEOUT
        )
    },
    { // 0943
        SPECIES_MABOSSTIFF,
        INNATES(
            ABILITY_GUARD_DOG,
            ABILITY_INTIMIDATE,
            ABILITY_STAKEOUT
        )
    },
    { // 0944
        SPECIES_SHROODLE,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER,
            ABILITY_UNBURDEN
        )
    },
    { // 0945
        SPECIES_GRAFAIAI,
        INNATES(
            ABILITY_PICKPOCKET,
            ABILITY_PRANKSTER,
            ABILITY_UNBURDEN
        )
    },
    { // 0946
        SPECIES_BRAMBLIN,
        INNATES(
            ABILITY_INFILTRATOR
        )
    },
    { // 0947
        SPECIES_BRAMBLEGHAST,
        INNATES(
            ABILITY_INFILTRATOR
        )
    },
    { // 0948
        SPECIES_TOEDSCOOL,
        INNATES(
            ABILITY_EFFECT_SPORE
        )
    },
    { // 0949
        SPECIES_TOEDSCRUEL,
        INNATES(
            ABILITY_EFFECT_SPORE
        )
    },
    { // 0950
        SPECIES_KLAWF,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0951
        SPECIES_CAPSAKID,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_INSOMNIA
        )
    },
    { // 0953
        SPECIES_RELLOR,
        INNATES(
            ABILITY_COMPOUND_EYES,
            ABILITY_SHED_SKIN
        )
    },
    { // 0954
        SPECIES_RABSCA,
        INNATES(
            ABILITY_TELEPATHY
        )
    },
    { // 0955
        SPECIES_FLITTLE,
        INNATES(
            ABILITY_ANTICIPATION,
            ABILITY_FRISK,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0956
        SPECIES_ESPATHRA,
        INNATES(
            ABILITY_OPPORTUNIST,
            ABILITY_FRISK,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0957
        SPECIES_TINKATINK,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_OWN_TEMPO,
            ABILITY_PICKPOCKET
        )
    },
    { // 0958
        SPECIES_TINKATUFF,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_OWN_TEMPO,
            ABILITY_PICKPOCKET
        )
    },
    { // 0959
        SPECIES_TINKATON,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_OWN_TEMPO,
            ABILITY_PICKPOCKET
        )
    },
    { // 0960
        SPECIES_WIGLETT,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_RATTLED,
            ABILITY_SAND_VEIL
        )
    },
    { // 0961
        SPECIES_WUGTRIO,
        INNATES(
            ABILITY_GOOEY,
            ABILITY_RATTLED,
            ABILITY_SAND_VEIL
        )
    },
    { // 0962
        SPECIES_BOMBIRDIER,
        INNATES(
            ABILITY_BIG_PECKS,
            ABILITY_KEEN_EYE,
            ABILITY_ROCKY_PAYLOAD
        )
    },
    { // 0965
        SPECIES_REVAVROOM,
        INNATES(
            ABILITY_FILTER,
            ABILITY_OVERCOAT
        )
    },
    { // 0965
        SPECIES_VAROOM,
        INNATES(
            ABILITY_OVERCOAT
        )
    },
    { // 0967
        SPECIES_CYCLIZAR,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHED_SKIN
        )
    },
    { // 0968
        SPECIES_ORTHWORM,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0969
        SPECIES_GLIMMET,
        INNATES(
            ABILITY_CORROSION
        )
    },
    { // 0970
        SPECIES_GLIMMORA_MEGA,
        INNATES(
            ABILITY_ADAPTABILITY
        )
    },
    { // 0970
        SPECIES_GLIMMORA,
        INNATES(
            ABILITY_CORROSION
        )
    },
    { // 0972
        SPECIES_HOUNDSTONE,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0973
        SPECIES_FLAMIGO,
        INNATES(
            ABILITY_SCRAPPY,
            ABILITY_TANGLED_FEET
        )
    },
    { // 0974
        SPECIES_CETODDLE,
        INNATES(
            ABILITY_SNOW_CLOAK,
            ABILITY_THICK_FAT
        )
    },
    { // 0975
        SPECIES_CETITAN,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_THICK_FAT
        )
    },
    { // 0976
        SPECIES_VELUZA,
        INNATES(
            ABILITY_MOLD_BREAKER,
            ABILITY_SHARPNESS
        )
    },
    { // 0977
        SPECIES_DONDOZO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_UNAWARE,
            ABILITY_WATER_VEIL
        )
    },
    { // 0979
        SPECIES_ANNIHILAPE,
        INNATES(
            ABILITY_ANGER_POINT,
            ABILITY_DEFIANT,
            ABILITY_INNER_FOCUS,
            ABILITY_UNAWARE,
            ABILITY_VITAL_SPIRIT
        )
    },
    { // 0980
        SPECIES_CLODSIRE,
        INNATES(
            ABILITY_IRON_BARBS,
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0981
        SPECIES_FARIGIRAF,
        INNATES(
            ABILITY_ARMOR_TAIL,
            ABILITY_CUD_CHEW,
            ABILITY_EARLY_BIRD,
            ABILITY_INNER_FOCUS
        )
    },
    { // 0982
        SPECIES_DUDUNSPARCE,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0982
        SPECIES_DUDUNSPARCE_THREE_SEGMENT,
        INNATES(
            ABILITY_RATTLED,
            ABILITY_SERENE_GRACE
        )
    },
    { // 0983
        SPECIES_KINGAMBIT,
        INNATES(
            ABILITY_DEFIANT,
            ABILITY_INNER_FOCUS,
            ABILITY_PRESSURE,
            ABILITY_SUPREME_OVERLORD
        )
    },
    { // 0987
        SPECIES_FLUTTER_MANE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0994
        SPECIES_IRON_MOTH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0999
        SPECIES_GIMMIGHOUL_ROAMING,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1000
        SPECIES_GHOLDENGO,
        INNATES(
            ABILITY_GOOD_AS_GOLD
        )
    },
    { // 1008
        SPECIES_MIRAIDON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1011
        SPECIES_DIPPLIN,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_STICKY_HOLD,
            ABILITY_SUPERSWEET_SYRUP
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HOSPITALITY,
            ABILITY_LEVITATE
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST_ARTISAN,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HOSPITALITY,
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HOSPITALITY,
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA_MASTERPIECE,
        INNATES(
            ABILITY_HEATPROOF,
            ABILITY_HOSPITALITY,
            ABILITY_LEVITATE
        )
    },
    { // 1014
        SPECIES_OKIDOGI,
        INNATES(
            ABILITY_GUARD_DOG
        )
    },
    { // 1015
        SPECIES_MUNKIDORI,
        INNATES(
            ABILITY_FRISK
        )
    },
    { // 1016
        SPECIES_FEZANDIPITI,
        INNATES(
            ABILITY_TECHNICIAN
        )
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 1017
        SPECIES_OGERPON_HEARTHFLAME,
        INNATES(
            ABILITY_MOLD_BREAKER
        )
    },
    { // 1018
        SPECIES_ARCHALUDON,
        INNATES(
            ABILITY_STALWART,
            ABILITY_STAMINA,
            ABILITY_STURDY
        )
    },
    { // 1019
        SPECIES_HYDRAPPLE,
        INNATES(
            ABILITY_GLUTTONY,
            ABILITY_REGENERATOR,
            ABILITY_STICKY_HOLD,
            ABILITY_SUPERSWEET_SYRUP
        )
    },
    { // 1025
        SPECIES_PECHARUNT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
};

// FORK: sublinear species->row lookup. The source table above stays sorted by National
// Dex number for humans (forms sit beside their base), which is NOT species-id order
// (form constants live at high ids), so it can't be binary-searched directly. Instead a
// row-index permutation sorted by species id is built lazily on first lookup (EWRAM bss,
// ~1 KB) and binary-searched thereafter. This lookup backs every SpeciesHasInnate /
// IsInnateActive call in the AI-hot battle calcs when the feature is ON, where the old
// linear walk of the whole table (~500 rows) was a real per-eval cost that CI never
// measures (tests force the feature off). Insertion sort is near-O(n) here because dex
// order is nearly species order — only form rows travel. Binary-search "any match" equals
// the documented "first match" because the "no species appears more than once" integrity
// test (test/fork/innate_abilities.c) forbids duplicate rows; the "species-keyed lookup
// matches the raw table" test guards this index against the raw rows.
// EWRAM_DATA is load-bearing: plain C statics' .bss lands in IWRAM (ld_script_modern.ld),
// where ~1 KB collides with the stack and corrupts memory (heap-magic asserts in malloc.c).
static EWRAM_DATA u16 sRowIndexSortedBySpecies[ARRAY_COUNT(sSpeciesInnates)] = {0};
static EWRAM_DATA bool8 sRowIndexBuilt = FALSE;

static void BuildRowIndexSortedBySpecies(void)
{
    u32 i, j;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        u16 species = sSpeciesInnates[i].species;

        for (j = i; j > 0 && sSpeciesInnates[sRowIndexSortedBySpecies[j - 1]].species > species; j--)
            sRowIndexSortedBySpecies[j] = sRowIndexSortedBySpecies[j - 1];
        sRowIndexSortedBySpecies[j] = i;
    }

    sRowIndexBuilt = TRUE;
}

static const enum Ability *GetSpeciesInnateList(u16 species)
{
    u32 lo, hi;

    if (!sRowIndexBuilt)
        BuildRowIndexSortedBySpecies();

    lo = 0;
    hi = ARRAY_COUNT(sSpeciesInnates);
    while (lo < hi)
    {
        u32 mid = (lo + hi) / 2;
        const struct SpeciesInnates *row = &sSpeciesInnates[sRowIndexSortedBySpecies[mid]];

        if (row->species == species)
            return row->innates;
        if (row->species < species)
            lo = mid + 1;
        else
            hi = mid;
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

// FORK: raw-table accessors for table-integrity tests (test/fork/innate_abilities.c).
// These walk the sSpeciesInnates rows directly (NOT keyed by species), so a duplicate
// species row — invisible to GetSpeciesInnateList, which returns the first match — is
// still observable. Not for battle logic: use SpeciesHasInnate / GetSpeciesInnate there.
u32 GetSpeciesInnatesEntryCount(void)
{
    return ARRAY_COUNT(sSpeciesInnates);
}

const enum Ability *GetSpeciesInnatesEntry(u32 row, u16 *outSpecies)
{
    if (row >= ARRAY_COUNT(sSpeciesInnates))
        return NULL;
    if (outSpecies != NULL)
        *outSpecies = sSpeciesInnates[row].species;
    return sSpeciesInnates[row].innates;
}

// Active, scripted innate abilities that fire at the end of every turn. The driver
// (TryActivateInnateEndTurnEffects) is already re-entrant, so a battler may carry
// more than one and each fires in turn. Each of these delegates to the existing
// upstream ABILITYEFFECT_ENDTURN case, so the stat change / heal / cure / item
// recovery / script / pop-up matches the real ability for free (the effect site in
// src/battle_util.c forces the pop-up to the innate when the chosen ability differs).
// Poison Heal is NOT here — it replaces the poison-damage step rather than adding an
// end-turn effect, so it is wired at the poison-damage site (src/battle_end_turn.c).
static bool32 IsActiveEndTurnInnate(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_SPEED_BOOST: // raises Speed +1
    case ABILITY_RAIN_DISH:   // heals 1/16 max HP in rain
    case ABILITY_ICE_BODY:    // heals 1/16 max HP in snow/hail
    case ABILITY_SHED_SKIN:   // 30% (always under DETERMINISTIC_ABILITIES) status self-cure
    case ABILITY_HYDRATION:   // cures status in rain
    case ABILITY_HEALER:      // 30% (always under DETERMINISTIC_ABILITIES) cure of an ally's status
    case ABILITY_HARVEST:     // recovers a used Berry
    case ABILITY_CUD_CHEW:    // re-eats a Berry the turn after eating it
    case ABILITY_PICKUP:      // picks up an item consumed this turn
    case ABILITY_BAD_DREAMS:  // chips sleeping foes 1/8 max HP
        return TRUE;
    default:
        return FALSE;
    }
}

// FORK: end-turn innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active,
// scripted end-turn innates (today only Speed Boost), hooked from the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step of the end-turn loop (src/battle_end_turn.c)
// right after the chosen-ability end-turn block.
//
// RE-ENTRANT: a battle script fires one at a time, so this resumes from a per-battler
// cursor. *index is the next innate-list slot to consider; the end-turn loop holds the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step (keeping the cursor) while this returns TRUE,
// and only advances the block once it returns FALSE (list exhausted). The caller resets
// the cursor to 0 for the next battler. Each fired effect leaves *index pointing past it,
// so a battler with several active end-turn innates fires them across successive turns of
// the loop. Returns TRUE if an effect fired this call.
//
// The effect is delegated to the upstream end-turn ability handler with the innate
// passed explicitly: AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, ...)
// sets gLastUsedAbility = innate and runs that ability's existing case, so the stat
// change / script / pop-up match the real ability exactly (the pop-up is overridden to
// show the innate at the Speed Boost effect site in src/battle_util.c, but only when the
// chosen ability differs). An innate equal to the chosen ability is skipped so the
// chosen-ability block (which already ran it) never boosts twice; IsInnateActive() applies
// the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field). An
// eligible innate that does nothing this turn (e.g. Speed already maxed) is stepped over
// without firing, so the scan continues to the battler's next end-turn innate.
bool32 TryActivateInnateEndTurnEffects(enum BattlerId battler, u32 *index)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveEndTurnInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability end-turn block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        // The upstream ABILITYEFFECT_ENDTURN cases show the pop-up on gBattlerAbility but do not all
        // set it (Speed Boost / Bad Dreams do; Pickup / Harvest / etc. rely on it already being the
        // holder). Earlier turn events (e.g. a foe eating a Berry) can leave it stale, so pin it to the
        // innate's owner here — harmless for the chosen path, which never goes through this driver.
        gBattlerAbility = battler;
        if (AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, MOVE_NONE, TRUE))
            return TRUE;
    }

    return FALSE;
}

// Active, scripted innate abilities that react when the HOLDER is hit by a move — the
// on-hit / on-contact class (contact-damage: Rough Skin / Iron Barbs; contact-Speed-drop:
// Gooey / Tangling Hair; on-faint retaliation: Aftermath / Innards Out, which fire from the
// same ABILITYEFFECT_MOVE_END step after the holder faints; on-hit stat/charge: Steam Engine /
// Thermal Exchange / Wind Power; move-disable: Cursed Body; on-hit stat boosts: Justified /
// Stamina / Water Compaction / Anger Point; contact-accuracy-drop: Effect Spore). The driver
// (TryActivateInnateOnHitEffects) is re-entrant, so a battler may carry more than one and each
// fires in turn. Each delegates to the existing upstream ABILITYEFFECT_MOVE_END case, so the
// recoil / retaliation damage / stat drop / script / pop-up matches the real ability for free
// (the effect site in src/battle_util.c forces the pop-up to the innate when the chosen ability
// differs, the Speed Boost precedent). Cute Charm is deliberately NOT here: it predates this
// driver and keeps its own inline precheck at the top of ABILITYEFFECT_MOVE_END, so listing it
// would fire it twice.
static bool32 IsActiveOnHitInnate(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_ROUGH_SKIN:    // chips a contact attacker 1/8 max HP
    case ABILITY_IRON_BARBS:    // chips a contact attacker 1/8 max HP
    case ABILITY_GOOEY:         // lowers a contact attacker's Speed by 1
    case ABILITY_TANGLING_HAIR: // lowers a contact attacker's Speed by 1
    case ABILITY_AFTERMATH:     // chips the attacker 1/4 max HP when a contact move KOs the holder
    case ABILITY_INNARDS_OUT:   // deals the attacker the holder's lost HP when a move KOs the holder
    case ABILITY_STEAM_ENGINE:  // raises Speed +6 when hit by a Fire/Water move
    case ABILITY_THERMAL_EXCHANGE: // raises Attack +1 when hit by a Fire move
    case ABILITY_WIND_POWER:    // charges the next Electric move when hit by a wind move
    case ABILITY_ELECTROMORPHOSIS: // charges the next Electric move when hit by any damaging move (Wind Power clone, no wind gate)
    case ABILITY_CURSED_BODY:   // 30% (always under DETERMINISTIC_ABILITIES) to disable the move that hit the holder
    case ABILITY_JUSTIFIED:     // raises Attack +1 when hit by a Dark move
    case ABILITY_STAMINA:       // raises Defense +1 when hit by any move
    case ABILITY_WATER_COMPACTION: // raises Defense +2 when hit by a Water move
    case ABILITY_ANGER_POINT:   // maxes Attack when the holder takes a critical hit
    case ABILITY_RATTLED:       // raises Speed +1 when hit by a Dark/Ghost/Bug move
    case ABILITY_EFFECT_SPORE:  // lowers a contact attacker's accuracy by 1 (powder-gated)
        return TRUE;
    default:
        return FALSE;
    }
}

// FORK: on-hit innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active, scripted
// on-hit innates (contact reactions), hooked from the MOVEEND_ABILITIES_INNATE step of the
// move-end loop (src/battle_move_resolution.c) right after the chosen-ability contact block.
//
// RE-ENTRANT, exactly like the end-turn driver: a battle script fires one at a time, so this
// resumes from a per-battler cursor. *index is the next innate-list slot to consider; the
// move-end loop holds the MOVEEND_ABILITIES_INNATE step (keeping the cursor) while this returns
// TRUE, and only advances once it returns FALSE (list exhausted), then resets the cursor. Each
// fired effect leaves *index past it, so a battler with several on-hit innates fires them across
// successive passes of the loop. Returns TRUE if an effect fired this call.
//
// The effect is delegated to the upstream contact-ability handler with the innate passed
// explicitly: AbilityBattleEffects(ABILITYEFFECT_MOVE_END, battler, innate, move, TRUE) sets
// gLastUsedAbility = innate and runs that ability's existing case (against gBattlerAttacker, the
// contact-maker), so the damage / stat drop / script / pop-up match the real ability. `battler`
// is the holder that was hit (gBattlerTarget). An innate equal to the chosen ability is skipped
// so the chosen-ability contact block (which already ran it) never fires twice; IsInnateActive()
// applies the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field). An
// eligible innate that does nothing this hit (e.g. attacker used a non-contact move, or already
// fainted) is stepped over without firing, so the scan continues to the next on-hit innate.
bool32 TryActivateInnateOnHitEffects(enum BattlerId battler, u32 *index, enum Move move)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveOnHitInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability contact block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        if (AbilityBattleEffects(ABILITYEFFECT_MOVE_END, battler, innate, move, TRUE))
            return TRUE;
    }

    return FALSE;
}

// Active, scripted innate abilities that react on the ATTACKER's side when it hits/faints a foe —
// today only Magician (steal a held item off a target the holder damaged). The driver
// (TryActivateInnateOnHitAttackerEffects) is re-entrant, mirroring the target-side on-hit driver,
// and delegates to the existing upstream ABILITYEFFECT_MOVE_END_FOES_FAINTED case, so the item
// steal / script / pop-up match the real ability for free (the effect site in src/battle_util.c
// forces the pop-up to the innate when the chosen ability differs, the Speed Boost precedent).
static bool32 IsActiveOnHitAttackerInnate(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_MAGICIAN: // steals a held item off a target it damaged, if not already holding one
    case ABILITY_MOXIE:    // raises Attack +1 for each foe the holder knocks out this move
    case ABILITY_CHILLING_NEIGH: // raises Attack +1 for each foe the holder knocks out this move (Moxie clone)
    case ABILITY_GRIM_NEIGH:     // raises Sp. Atk +1 for each foe the holder knocks out this move (Moxie clone)
    case ABILITY_BEAST_BOOST:    // raises the holder's HIGHEST stat +1 for each foe it knocks out this move (Moxie, best-stat edition)
        return TRUE;
    default:
        return FALSE;
    }
}

// FORK: attacker-side on-hit innate driver (FEATURE_INNATE_ABILITIES). Fires the attacker's active,
// scripted attacker-side on-hit innates (today only Magician), hooked from the
// MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE step of the move-end loop (src/battle_move_resolution.c)
// right after the chosen-ability foes-fainted block.
//
// RE-ENTRANT, exactly like the target-side on-hit driver: *index is the next innate-list slot to
// consider; the move-end loop holds the step (keeping the cursor) while this returns TRUE, and only
// advances once it returns FALSE (list exhausted), then resets the cursor. `battler` is the attacker
// (gBattlerAttacker); the delegated case reads it as the item thief.
//
// The effect is delegated to the upstream attacker-side handler with the innate passed explicitly:
// AbilityBattleEffects(ABILITYEFFECT_MOVE_END_FOES_FAINTED, battler, innate, move, TRUE) sets
// gLastUsedAbility = innate and runs that ability's existing case, so the item steal / script /
// pop-up match the real ability. An innate equal to the chosen ability is skipped so the
// chosen-ability foes-fainted block (which already ran it) never fires twice; IsInnateActive()
// applies the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field).
bool32 TryActivateInnateOnHitAttackerEffects(enum BattlerId battler, u32 *index, enum Move move)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveOnHitAttackerInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability foes-fainted block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        if (AbilityBattleEffects(ABILITYEFFECT_MOVE_END_FOES_FAINTED, battler, innate, move, TRUE))
            return TRUE;
    }

    return FALSE;
}

// Active, scripted innate abilities that react when the HOLDER takes damage from a move — the
// on-damage class, wired through the upstream ABILITYEFFECT_COLOR_CHANGE step rather than
// ABILITYEFFECT_MOVE_END (that step iterates every damaged battler, so a spread move triggers the
// reaction on each holder, matching the real ability). Today only Berserk (raises Sp. Atk +1 when an
// attack drops the holder's HP from above 1/2 to 1/2 or less).
static bool32 IsActiveOnDamageInnate(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_BERSERK: // raises Sp. Atk +1 when an attack drops the holder's HP to 1/2 or less
        return TRUE;
    default:
        return FALSE;
    }
}

// FORK: on-damage innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active, scripted
// on-damage innates (today only Berserk), hooked from the MOVEEND_COLOR_CHANGE_INNATE step of the
// move-end loop (src/battle_move_resolution.c) right after the chosen-ability MOVEEND_COLOR_CHANGE
// block, which iterates every damaged battler. `battler` is one damaged holder; the caller loops it
// over all battlers exactly like the chosen-ability color-change step.
//
// RE-ENTRANT, exactly like the target-side on-hit driver: *index is the next innate-list slot to
// consider; the move-end loop holds the step (keeping the cursor) while this returns TRUE, and only
// advances to the next battler (resetting the cursor) once it returns FALSE (this battler's list
// exhausted). The effect is delegated to the upstream on-damage handler with the innate passed
// explicitly: AbilityBattleEffects(ABILITYEFFECT_COLOR_CHANGE, battler, innate, MOVE_NONE, TRUE) sets
// gLastUsedAbility = innate and runs that ability's existing case, so the stat change / script /
// pop-up match the real ability (the effect site forces the pop-up to the innate when the chosen
// ability differs). An innate equal to the chosen ability is skipped so the chosen-ability
// color-change block (which already ran it) never fires twice; IsInnateActive() applies the usual
// suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field).
bool32 TryActivateInnateOnDamageEffects(enum BattlerId battler, u32 *index)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveOnDamageInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability color-change block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        if (AbilityBattleEffects(ABILITYEFFECT_COLOR_CHANGE, battler, innate, MOVE_NONE, TRUE))
            return TRUE;
    }

    return FALSE;
}

// Active, scripted innate abilities that fire when the HOLDER switches in — Intimidate (lowers every
// opposing battler's Attack by 1 stage), the information-reveal trio Anticipation / Forewarn / Frisk
// (each shows a switch-in message; Frisk/Forewarn reveal a foe's item/move, Anticipation warns of a
// super-effective or OHKO move), Download (raises the holder's Attack or Sp. Atk toward the foe's weaker
// defense), Supersweet Syrup (lowers every opposing battler's evasiveness by 1 stage, once per battle),
// Unnerve (denies opposing battlers their Berries), Hospitality (heals the ally 1/4 max HP in doubles)
// and Pastel Veil (cures the holder's and its ally's pre-existing poison on switch-in).
// The driver (TryActivateInnateSwitchInEffects) is re-entrant, so a battler may carry more than one and
// each fires in turn. Each delegates to the existing upstream switch-in case that runs the real ability,
// so the stat change / message / heal / script / pop-up matches the real ability for free (the effect site
// in src/battle_util.c forces the pop-up to the innate when the chosen ability differs, the Speed Boost
// precedent).
//
// Most switch-in innates run through ABILITYEFFECT_ON_SWITCHIN, but Unnerve and Hospitality live in their
// own upstream cases (ABILITYEFFECT_UNNERVE / ABILITYEFFECT_DEPENDS_ON_ALLY), dispatched at a different
// point of the switch-in sequence. This maps each active switch-in innate to the ABILITYEFFECT_* that runs
// it, so the driver — called once per native switch-in phase with that phase's abilityEffect — fires each
// innate at the same point the real ability would. A non-switch-in innate returns ABILITYEFFECT_ENDTURN, a
// value no switch-in phase ever passes, so it is skipped.
static enum AbilityEffect SwitchInInnateAbilityEffect(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_INTIMIDATE:       // lowers opposing battlers' Attack by 1 stage on switch-in
    case ABILITY_ANTICIPATION:     // shows a warning message if a foe knows a super-effective / OHKO move
    case ABILITY_FOREWARN:         // reveals one of a foe's strongest moves
    case ABILITY_FRISK:            // reveals the foes' held items
    case ABILITY_DOWNLOAD:         // raises the holder's Attack or Sp. Atk vs the foe's weaker defense
    case ABILITY_SUPERSWEET_SYRUP: // lowers opposing battlers' evasiveness by 1 stage (once per battle)
    case ABILITY_PASTEL_VEIL:      // cures the holder's and its ally's pre-existing poison on switch-in
    case ABILITY_SUPREME_OVERLORD: // latches a +10%/fallen-teammate move-power boost at switch-in
    case ABILITY_INTREPID_SWORD:   // raises the holder's Attack by 1 stage the first time it enters battle
    case ABILITY_DAUNTLESS_SHIELD: // raises the holder's Defense by 1 stage the first time it enters battle
        return ABILITYEFFECT_ON_SWITCHIN;
    case ABILITY_UNNERVE:          // denies opposing battlers their Berries
        return ABILITYEFFECT_UNNERVE;
    case ABILITY_HOSPITALITY:      // heals the ally 1/4 max HP on switch-in (doubles only)
        return ABILITYEFFECT_DEPENDS_ON_ALLY;
    default:
        return ABILITYEFFECT_ENDTURN; // not a switch-in innate — no switch-in phase passes this
    }
}

// FORK: switch-in innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active, scripted
// switch-in innates, hooked from three switch-in phases of the switch-in loop (src/battle_switch_in.c),
// each right after its chosen-ability counterpart: the ABILITYEFFECT_ON_SWITCHIN block
// (FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE, for Intimidate / Download / the reveal trio / Supersweet
// Syrup), the ABILITYEFFECT_UNNERVE event (SWITCH_IN_EVENTS_UNNERVE_INNATE, for Unnerve), and the
// ABILITYEFFECT_DEPENDS_ON_ALLY block (SECOND_EVENT_ABILITIES_INNATE, for Hospitality). `abilityEffect`
// selects which phase this call handles; only innates whose native switch-in effect matches it fire here
// (SwitchInInnateAbilityEffect), so each innate runs at the same point of the sequence the real ability
// would.
//
// RE-ENTRANT, exactly like the end-turn / on-hit drivers: a battle script fires one at a time, so this
// resumes from a per-battler cursor. *index is the next innate-list slot to consider; the switch-in loop
// holds the step (keeping the cursor) while this returns TRUE, and only advances once it returns FALSE
// (list exhausted), then resets the cursor. Each fired effect leaves *index past it, so a battler with
// several switch-in innates for one phase fires them across successive passes of the loop. Returns TRUE if
// an effect fired this call.
//
// `shouldTrigger` mirrors the chosen-ability switch-in call's gate
// (gBattleStruct->battlerState[battler].switchIn), so an innate switch-in effect only fires on a genuine
// switch-in exactly like the real ability. The effect is delegated to the upstream switch-in ability
// handler with the innate passed explicitly: AbilityBattleEffects(abilityEffect, battler, innate,
// MOVE_NONE, shouldTrigger) sets gLastUsedAbility = innate and runs that ability's existing case, so the
// stat change / heal / script / pop-up match the real ability. An innate equal to the chosen ability is
// skipped so the chosen-ability switch-in block (which already ran it) never fires twice; IsInnateActive()
// applies the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field).
bool32 TryActivateInnateSwitchInEffects(enum BattlerId battler, u32 *index, bool32 shouldTrigger, enum AbilityEffect abilityEffect)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (SwitchInInnateAbilityEffect(innate) != abilityEffect) // not a switch-in innate for this phase
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability switch-in block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        // The switch-in ability pop-up reads gBattlerAbility; pin it to the innate's owner so the pop-up
        // shows on the right battler (the chosen path never goes through this driver). End-turn precedent.
        gBattlerAbility = battler;
        if (AbilityBattleEffects(abilityEffect, battler, innate, MOVE_NONE, shouldTrigger))
            return TRUE;
    }

    return FALSE;
}

// FORK: innate-aware ability names in battle text (FEATURE_INNATE_ABILITIES).
//
// The ability-name placeholders — {B_ATK_ABILITY}, {B_DEF_ABILITY}, {B_SCR_ACTIVE_ABILITY},
// {B_EFF_ABILITY} — do NOT read a live global: they read a per-battler snapshot of
// gBattleMons[].ability taken when the string is queued (stringInfo->abilities[] in
// BtlController_EmitPrintString, src/battle_controllers.c), i.e. always the CHOSEN ability. An
// innate's own message therefore named the wrong ability: an innate Ice Body heal on an Alolan
// Ninetales printed "<mon>'s Snow Warning healed it a little bit!" while the pop-up — which has
// its own override — correctly said Ice Body.
//
// The pop-up's fix (gBattleScripting.abilityPopupOverwrite) can't be reused here, because
// BattleScript_AbilityPopUp clears it immediately after `showabilitypopup`, before the script
// reaches its `printstring`. gLastUsedAbility is the ability currently being processed
// (AbilityBattleEffects sets it to whichever ability it was handed, so the fork's innate drivers
// put the innate there) and is still live at emit time, so it is the signal used here: when it
// differs from a battler's chosen ability and that battler's species declares it as an innate,
// the message is about the innate — snapshot the innate for that battler.
//
// Same predicate as RecordAbilityBattle's witnessedIsInnate (src/battle_ai_record.c), and
// deliberately NOT IsInnateActive(): the effect has already fired, so its message must name it
// even if the innate would read as suppressed (Mold Breaker, Ability Shield) by the time the text
// is drawn. A battler whose chosen ability is the one being processed is left untouched, so stock
// text is byte-for-byte unchanged and the feature flag being off is a no-op.
//
// Caveat: a battler that carries innate X can have a message about its *chosen* ability rendered
// as X if gLastUsedAbility is still a stale X. In practice every ability that produces such a
// message sets gLastUsedAbility to itself first (that is how the message system already resolves
// the right name), so the stale window is not reachable from the scripted paths.
void ApplyInnateMessageAbilities(enum Ability *abilities)
{
    if (!GetConfig(FEATURE_INNATE_ABILITIES) || gLastUsedAbility == ABILITY_NONE)
        return;

    for (u32 battler = 0; battler < MAX_BATTLERS_COUNT; battler++)
    {
        if (abilities[battler] == gLastUsedAbility) // this battler's chosen ability is the one firing
            continue;
        if (!SpeciesHasInnate(gBattleMons[battler].species, gLastUsedAbility))
            continue;
        abilities[battler] = gLastUsedAbility;
    }
}
