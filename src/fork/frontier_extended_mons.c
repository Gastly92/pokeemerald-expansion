#include "global.h"
#include "fork/frontier_extended_mons.h"
#include "event_data.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// FORK: fork-owned Battle Factory roster overhaul (B_FRONTIER_EXTENDED_MONS).
// A from-scratch roster of modern competitive sets that replaces the vanilla
// gBattleFrontierMons on the Battle Factory's code paths, kept in this new file so
// upstream syncs never touch it. Same struct TrainerMon format as
// gBattleFrontierMons, so every field CreateFacilityMon understands is available.
// The feature (draft rules, tier gating, format tags, innate handling, gimmick
// readiness, save-index caveat) is documented in full in fork-docs/FORK.md — see
// the "Extended frontier roster" row. This header only covers what you need to
// EDIT an entry; consult the docs for the why.
//
// Authoring an entry — the helpers are defined in frontier_extended_mons.h:
//  - Each entry opens with a `{ // NNNN` National-Dex comment (like
//    innate_abilities.c). Entries stay sorted by that number; see ORDER below.
//  - .ev   uses EVS(...): name only the stats you want, any order, rest default to
//          0 — EVS(.hp = 252, .def = 252, .spd = 4). Field names hp/atk/def/spa/
//          spd/spe (spd is Sp. Def, spe is Speed).
//  - .nature uses NATURE(UP, DOWN): boosted stat first, lowered second, from
//          ATK/DEF/SPA/SPD/SPE — NATURE(DEF_UP, ATK_DOWN) is Bold. Neutral natures
//          use NATURE_NEUTRAL (== NATURE_HARDY).
//  - .tags is REQUIRED: FORMAT_SINGLES / FORMAT_DOUBLES / FORMAT_BOTH gates which
//          battle format(s) the set can be drawn for.
//  - .iv and .ball are OPTIONAL. Omit .iv to keep the Factory's fixed IVs (31s
//          under B_FRONTIER_MAX_IVS); set .iv = IVS(SPE, 0) — naming only the
//          stats that change, the rest defaulting to 31 — for an intentional
//          spread (e.g. 0 Speed for Trick Room). Omit .ball for BALL_POKE; set it
//          only for a non-Poke look.
//  - .teraType sets the set's Terastallization type (Tera is active in this fork).
//
// Design intent — sets are tuned around this fork's DETERMINISTIC_* changes (no
// misses, crits only when guaranteed, one-shot Focus Sash, guaranteed multi-hit,
// paralysis as a priority tax, etc.), NOT the live metagame. See
// fork-docs/DETERMINISM.md for the exact rules that make e.g. Hydro Pump reliable
// or crit items a guaranteed first-hit crit.
//
// Innate abilities (FEATURE_INNATE_ABILITIES, src/fork/innate_abilities.c): a
// species keeps its innate ability in battle regardless of its .ability slot, so
// each set's .ability carries a *complementary* chosen ability that runs alongside
// the innate (e.g. Slowbro STORM_DRAIN + innate Regenerator, Rotom LIGHTNING_ROD +
// innate Levitate). Role comments naming an innate describe that innate-backed
// playstyle, not the .ability field. Every .ability must be a NEVER-AN-INNATE pick
// — absent from sImplementedInnates[] in test/fork/innate_abilities.c — since an
// innate-capable ability belongs in an INNATES(...) row instead; the per-species
// picks and their rationale live in the override rows in
// src/fork/species_ability_overrides.c, and the rule is documented in
// fork-docs/INNATE_ABILITIES.md ("Direction"). All three invariants are CI-gated:
// TEST("Innate abilities: no ability override or frontier set names an
// innate-capable ability") plus the roster tests in
// test/fork/frontier_extended_roster.c — those tests, not a comment, are the record.
//
// IMPORTANT: every .ability must resolve to a real ability slot for the species
// (CreateFacilityMon, src/battle_frontier.c, silently falls back to slot 0 on an
// unmatched ability). For an ability-locked innate species whose only real ability
// is the one now granted innately (Rotom, Hydreigon, the lake trio, ...), the
// complementary ability is made selectable via src/fork/species_ability_overrides.c
// rather than by editing upstream species data. test/fork/frontier_extended_roster.c
// enforces that every .ability is legal through that hook, so a typo can't silently
// downgrade.
//
// STATUS: Generations I-IX are all built out (~2-4 builds per reasonable species,
// offense/defense mix). B_FRONTIER_EXTENDED_MONS is TRUE.
// ORDER: sorted by National Pokédex number (see the Generation banners). All builds
// for one species are contiguous; alternate formes (megas share the base entry;
// Rotom/Therian/Hisuian/Paradox/etc. are their own entries) sort with their base
// species' dex number, so cross-gen evolutions live at their own dex slot (e.g.
// Magnezone/Electivire/Magmortar/Rhyperior/Tangrowth sit at #462/466/467/464/465,
// not next to their Gen I pre-evolutions). Insert a new species at its correct dex
// position. NOTE: saved rentals reference entries by array INDEX, so any mid-list
// insertion/removal invalidates an in-progress rented team in an existing save
// (appending past the end is the only save-safe edit).

const struct TrainerMon gFrontierExtendedMons[] =
{
    // ====================================
    // Generation I
    // ====================================

    // 0003
    {
        .species = SPECIES_VENUSAUR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_EARTH_POWER,
            MOVE_LEECH_SEED,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_TERRAIN_EXTENDER,
        .moves =
        {
            MOVE_POLLEN_PUFF,
            MOVE_RAGE_POWDER,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_LEECH_SEED,
            MOVE_STRENGTH_SAP,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_SOLAR_BEAM,
            MOVE_SLUDGE_BOMB,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spe = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0006
    {
        .species = SPECIES_CHARIZARD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_CHARIZARD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_SOLAR_BEAM,
            MOVE_HURRICANE,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CHARIZARD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_HEAT_WAVE,
            MOVE_HURRICANE,
            MOVE_TAILWIND,
            MOVE_PROTECT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_CHARIZARD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_NOBLE_ROAR,
            MOVE_FIRE_BLAST,
            MOVE_DRAGON_PULSE,
            MOVE_ROOST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0009
    {
        .species = SPECIES_BLASTOISE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BLASTOISE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_DAMP_ROCK,
        .moves =
        {
            MOVE_RAIN_DANCE,
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_CHILLING_WATER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_BLASTOISE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_SNIPE_SHOT,
            MOVE_ICY_WIND,
            MOVE_WIDE_GUARD,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BLASTOISE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_WATER_SPOUT,
            MOVE_FLASH_CANNON,
            MOVE_AURA_SPHERE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0012
    {
        .species = SPECIES_BUTTERFREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_BUTTERFREE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_INFESTATION,
            MOVE_SLEEP_POWDER,
            MOVE_SUBSTITUTE,
            MOVE_DREAM_EATER
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_BUTTERFREE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SAFETY_GOGGLES,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_SLEEP_POWDER,
            MOVE_TAILWIND,
            MOVE_POLLEN_PUFF
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0015
    {
        .species = SPECIES_BEEDRILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_TWINEEDLE,
            MOVE_CROSS_POISON,
            MOVE_KNOCK_OFF,
            MOVE_DRILL_RUN
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_BEEDRILL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_U_TURN,
            MOVE_FELL_STINGER,
            MOVE_CROSS_POISON,
            MOVE_DRILL_RUN
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_BEEDRILL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_PIN_MISSILE,
            MOVE_FIRST_IMPRESSION,
            MOVE_POISON_JAB,
            MOVE_PROTECT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0018
    {
        .species = SPECIES_PIDGEOT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_U_TURN,
            MOVE_HYPER_VOICE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_PIDGEOT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_FEATHER_DANCE,
            MOVE_ROOST,
            MOVE_TAILWIND,
            MOVE_AIR_CUTTER
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_PIDGEOT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHARP_BEAK,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_WHIRLWIND,
            MOVE_ROOST,
            MOVE_HEAT_WAVE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0020
    {
        .species = SPECIES_RATICATE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HYPER_FANG,
            MOVE_DRILL_RUN,
            MOVE_KNOCK_OFF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_RATICATE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_DRILL_RUN,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_RATICATE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_ENDEAVOR,
            MOVE_QUICK_ATTACK,
            MOVE_SUPER_FANG,
            MOVE_CRUNCH
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_RATICATE_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_CRUNCH,
            MOVE_SUCKER_PUNCH,
            MOVE_SUPER_FANG
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_RATICATE_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FIGY_BERRY,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_BEAT_UP,
            MOVE_KNOCK_OFF,
            MOVE_PARTING_SHOT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0022
    {
        .species = SPECIES_FEAROW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_DRILL_PECK,
            MOVE_DRILL_RUN,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_FEAROW,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_FEATHER_DANCE,
            MOVE_DRILL_PECK,
            MOVE_DRILL_RUN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_FEAROW,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_DRILL_PECK,
            MOVE_DRILL_RUN,
            MOVE_PURSUIT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0024
    {
        .species = SPECIES_ARBOK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_ARBOK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_WRAP,
            MOVE_COIL,
            MOVE_REST,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_ARBOK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_GLARE,
            MOVE_GUNK_SHOT,
            MOVE_SUCKER_PUNCH,
            MOVE_PROTECT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0025
    {
        .species = SPECIES_PIKACHU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_BALL,
        .moves =
        {
            MOVE_VOLT_TACKLE,
            MOVE_IRON_TAIL,
            MOVE_SURF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 252,
            .spa = 252,
            .spd = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0026
    {
        .species = SPECIES_RAICHU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDER,
            MOVE_SURF,
            MOVE_FOCUS_BLAST,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_RAICHU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_VOLT_TACKLE,
            MOVE_ICE_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_RAICHU_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_THUNDER,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_RAICHU_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_RAICHU_ALOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYSHOCK,
            MOVE_THUNDERBOLT,
            MOVE_SURF
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0028
    {
        .species = SPECIES_SANDSLASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_SANDSLASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_RAZOR_CLAW,
        .moves =
        {
            MOVE_DRILL_RUN,
            MOVE_STONE_EDGE,
            MOVE_NIGHT_SLASH,
            MOVE_CROSS_POISON
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SANDSLASH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SMOOTH_ROCK,
        .moves =
        {
            MOVE_ROCK_SLIDE,
            MOVE_DRILL_RUN,
            MOVE_SWORDS_DANCE,
            MOVE_PROTECT
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_SANDSLASH_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NEVER_MELT_ICE,
        .moves =
        {
            MOVE_TRIPLE_AXEL,
            MOVE_IRON_HEAD,
            MOVE_EARTHQUAKE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_SANDSLASH_ALOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ICY_ROCK,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_ICICLE_CRASH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0031
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC_SPIKES,
            MOVE_SLUDGE_WAVE,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 200,
            .spd = 56
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_DRILL_RUN,
            MOVE_POISON_JAB,
            MOVE_MEGAHORN,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHUCA_BERRY,
        .moves =
        {
            MOVE_WIDE_GUARD,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_HELPING_HAND
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0034
    {
        .species = SPECIES_NIDOKING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_NIDOKING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_WAVE,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_NIDOKING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_POISON_BARB,
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_NIDOKING,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_ROCK_SLIDE,
            MOVE_POISON_JAB,
            MOVE_DRILL_RUN,
            MOVE_PROTECT
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0036
    {
        .species = SPECIES_CLEFABLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_FLAMETHROWER,
            MOVE_CALM_MIND,
            MOVE_SOFT_BOILED
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CLEFABLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_DAZZLING_GLEAM,
            MOVE_FLAMETHROWER,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CLEFABLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_MOONBLAST,
            MOVE_MOONLIGHT,
            MOVE_LUNAR_DANCE
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CLEFABLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_COSMIC_POWER,
            MOVE_STORED_POWER,
            MOVE_MOONBLAST,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0038
    {
        .species = SPECIES_NINETALES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_SOLAR_BEAM,
            MOVE_SCORCHING_SANDS,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_NINETALES,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_PERISH_SONG,
            MOVE_SUBSTITUTE,
            MOVE_WILL_O_WISP,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_NINETALES,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_HEAT_WAVE,
            MOVE_WILL_O_WISP,
            MOVE_ENCORE,
            MOVE_PROTECT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_NINETALES_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_DAZZLING_GLEAM,
            MOVE_FREEZE_DRY,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_NINETALES_ALOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_FREEZE_DRY,
            MOVE_SHEER_COLD,
            MOVE_ENCORE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0040
    {
        .species = SPECIES_WIGGLYTUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_DAZZLING_GLEAM,
            MOVE_MOONLIGHT,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_HALO,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_WIGGLYTUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_SING,
            MOVE_DREAM_EATER,
            MOVE_HYPER_VOICE,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_HALO,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_WIGGLYTUFF,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_SING,
            MOVE_MOONLIGHT,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_HALO,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_WIGGLYTUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_CHARM,
            MOVE_HYPER_VOICE,
            MOVE_MOONLIGHT,
            MOVE_TOXIC
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0045
    {
        .species = SPECIES_VILEPLUME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_VILEPLUME,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_STRENGTH_SAP,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_VILEPLUME,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_POLLEN_PUFF,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0047
    {
        .species = SPECIES_PARASECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_SPORE,
            MOVE_SEED_BOMB,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_PARASECT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_SPORE,
            MOVE_SEED_BOMB,
            MOVE_LEECH_LIFE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_PARASECT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_SPORE,
            MOVE_RAGE_POWDER,
            MOVE_LEECH_SEED,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0049
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_PSYCHIC,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BRIGHT_POWDER,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_SLEEP_POWDER,
            MOVE_STRUGGLE_BUG,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_VENOSHOCK,
            MOVE_PSYCHIC,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_SLEEP_POWDER,
            MOVE_DREAM_EATER,
            MOVE_DRAINING_KISS,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0051
    {
        .species = SPECIES_DUGTRIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DUGTRIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH,
            MOVE_FISSURE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0051
    {
        .species = SPECIES_DUGTRIO_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_DUGTRIO_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SOFT_SAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_SWORDS_DANCE,
            MOVE_PROTECT
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0053
    {
        .species = SPECIES_PERSIAN_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_FAKE_OUT,
            MOVE_PLAY_ROUGH,
            MOVE_U_TURN
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_PERSIAN_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_FAKE_OUT,
            MOVE_PARTING_SHOT,
            MOVE_TAUNT
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .spd = 8,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_PERSIAN_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_SNARL,
            MOVE_HYPER_VOICE
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0053
    {
        .species = SPECIES_PERSIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_NASTY_PLOT,
            MOVE_HYPER_VOICE,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_PERSIAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_NONE,
        .moves =
        {
            MOVE_THIEF,
            MOVE_SLASH,
            MOVE_FAKE_OUT,
            MOVE_U_TURN
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0055
    {
        .species = SPECIES_GOLDUCK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_RAIN_DANCE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_PSYCHIC_AFFINITY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GOLDUCK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_CALM_MIND,
            MOVE_RECOVER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_GOLDUCK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_HELPING_HAND,
            MOVE_MUDDY_WATER,
            MOVE_RECOVER
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0059
    {
        .species = SPECIES_ARCANINE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_MORNING_SUN,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ARCANINE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_WILD_CHARGE,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ARCANINE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_SNARL,
            MOVE_PROTECT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_ARCANINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_FLAME_CHARGE,
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_CRUNCH
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0059
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_FLARE_BLITZ,
            MOVE_ROCK_SLIDE,
            MOVE_BURNING_BULWARK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PASSHO_BERRY,
        .moves =
        {
            MOVE_HOWL,
            MOVE_HEAD_SMASH,
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0062
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHESTO_BERRY,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_LIQUIDATION,
            MOVE_DRAIN_PUNCH,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_DARKEST_LARIAT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_HYPNOSIS,
            MOVE_DRAIN_PUNCH,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_FOCUS_PUNCH,
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0065
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_PSYSHOCK,
            MOVE_SHADOW_BALL,
            MOVE_TRICK
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_ENCORE
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_TERRAIN_EXTENDER,
        .moves =
        {
            MOVE_EXPANDING_FORCE,
            MOVE_SHADOW_BALL,
            MOVE_ALLY_SWITCH,
            MOVE_PROTECT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_EXPANDING_FORCE,
            MOVE_HELPING_HAND
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0068
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_BULLET_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ROCK_SLIDE,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_BIND,
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BLACK_BELT,
        .moves =
        {
            MOVE_COACHING,
            MOVE_WIDE_GUARD,
            MOVE_CLOSE_COMBAT,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0071
    {
        .species = SPECIES_VICTREEBEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_VICTREEBEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SLEEP_POWDER,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0073
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RAPID_SPIN,
            MOVE_TOXIC_SPIKES,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 124,
            .spe = 132
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SLUDGE_WAVE,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_INFESTATION,
            MOVE_TOXIC,
            MOVE_ACID_ARMOR,
            MOVE_BANEFUL_BUNKER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_ICY_WIND,
            MOVE_CHILLING_WATER,
            MOVE_MUDDY_WATER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0076
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_EXPLOSION,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CUSTAP_BERRY,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SMOOTH_ROCK,
        .moves =
        {
            MOVE_WIDE_GUARD,
            MOVE_ROCK_SLIDE,
            MOVE_EARTHQUAKE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0076
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_GALVANIZE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HARD_STONE,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_THUNDER_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_CURSE
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_ROCK_SLIDE,
            MOVE_BOLT_STRIKE,
            MOVE_PROTECT
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_MAGNET_RISE,
            MOVE_BOLT_STRIKE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0078
    {
        .species = SPECIES_RAPIDASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_HORSEPOWER,
            MOVE_WILD_CHARGE,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_RAPIDASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_MEGAHORN,
            MOVE_HORN_LEECH,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_RAPIDASH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_FOLLOW_ME,
            MOVE_WILL_O_WISP,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0078
    {
        .species = SPECIES_RAPIDASH_GALAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FAIRY_FEATHER,
        .moves =
        {
            MOVE_MORNING_SUN,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_RAPIDASH_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_PSYCHO_CUT,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_RAPIDASH_GALAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_WISE_GLASSES,
        .moves =
        {
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_HELPING_HAND,
            MOVE_HEALING_WISH
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0080
    {
        .species = SPECIES_SLOWBRO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_PSYSHOCK,
            MOVE_CALM_MIND,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SLOWBRO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FUTURE_SIGHT,
            MOVE_SLACK_OFF,
            MOVE_TELEPORT
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SLOWBRO,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_CHILLING_WATER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_FAIRY,
    },

    // 0080
    {
        .species = SPECIES_SLOWBRO_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_SHELL_SIDE_ARM,
            MOVE_PSYCHIC,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_SLOWBRO_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_SHELL_SIDE_ARM,
            MOVE_GUNK_SHOT,
            MOVE_ZEN_HEADBUTT,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0083
    {
        .species = SPECIES_FARFETCHD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEEK,
        .moves =
        {
            MOVE_FIRST_IMPRESSION,
            MOVE_BRAVE_BIRD,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_FARFETCHD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_FOLLOW_ME,
            MOVE_TAILWIND,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0085
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_DOUBLE_EDGE,
            MOVE_BRAVE_BIRD,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, DEF_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_TAILWIND,
            MOVE_BRAVE_BIRD,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0087
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_SURF,
            MOVE_SHEER_COLD,
            MOVE_PROTECT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_SURF,
            MOVE_AURORA_VEIL,
            MOVE_PROTECT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_WHIRLPOOL,
            MOVE_PERISH_SONG,
            MOVE_PROTECT,
            MOVE_SURF
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_SING,
            MOVE_SPARKLING_ARIA,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0089
    {
        .species = SPECIES_MUK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_POISON_JAB,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_ACID_ARMOR
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 84,
            .spd = 172
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_MUK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MUK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_PROTECT,
            MOVE_CLEAR_SMOG,
            MOVE_POISON_JAB,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .atk = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0089
    {
        .species = SPECIES_MUK_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_POWER_OF_ALCHEMY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_MUK_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_POISON_JAB,
            MOVE_CRUNCH,
            MOVE_CURSE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_MUK_ALOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FIGY_BERRY,
        .moves =
        {
            MOVE_RECYCLE,
            MOVE_PROTECT,
            MOVE_POISON_JAB,
            MOVE_CRUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .atk = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0091
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_ICICLE_SPEAR,
            MOVE_ROCK_BLAST,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_ICICLE_SPEAR,
            MOVE_ROCK_BLAST,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICICLE_SPEAR,
            MOVE_RAPID_SPIN,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_WIDE_GUARD,
            MOVE_ICY_WIND,
            MOVE_ICICLE_SPEAR,
            MOVE_ROCK_BLAST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ICY_ROCK,
        .moves =
        {
            MOVE_SNOWSCAPE,
            MOVE_AURORA_VEIL,
            MOVE_ICICLE_SPEAR,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0094
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_BOMB,
            MOVE_FOCUS_BLAST,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_BOMB,
            MOVE_FOCUS_BLAST,
            MOVE_TRICK
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_HEX,
            MOVE_SLUDGE_BOMB,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SPELL_TAG,
        .moves =
        {
            MOVE_HYPNOSIS,
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_BOMB,
            MOVE_ALLY_SWITCH
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_MEAN_LOOK,
            MOVE_PERISH_SONG,
            MOVE_PROTECT,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0097
    {
        .species = SPECIES_HYPNO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_QUICK_CLAW,
        .moves =
        {
            MOVE_HYPNOSIS,
            MOVE_PSYCHIC,
            MOVE_FOUL_PLAY,
            MOVE_WISH
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_HYPNO,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_FOLLOW_ME,
            MOVE_HYPNOSIS,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_HYPNO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_AGUAV_BERRY,
        .moves =
        {
            MOVE_MEAN_LOOK,
            MOVE_HYPNOSIS,
            MOVE_NIGHTMARE,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0099
    {
        .species = SPECIES_KINGLER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_ROCK_SLIDE,
            MOVE_KNOCK_OFF,
            MOVE_AGILITY
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KINGLER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_CRABHAMMER,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KINGLER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRIP_CLAW,
        .moves =
        {
            MOVE_CLAMP,
            MOVE_GUILLOTINE,
            MOVE_CRABHAMMER,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_KINGLER,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_WIDE_GUARD,
            MOVE_HELPING_HAND,
            MOVE_CRABHAMMER,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0101
    {
        .species = SPECIES_ELECTRODE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_LEAF_STORM,
            MOVE_VOLT_SWITCH,
            MOVE_TAUNT
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ELECTRODE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_THUNDER_WAVE,
            MOVE_TAUNT
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0101
    {
        .species = SPECIES_ELECTRODE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_THUNDER_WAVE,
            MOVE_TAUNT
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ELECTRODE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_THUNDERBOLT,
            MOVE_TAUNT
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0103
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_PSYCHIC,
            MOVE_SLEEP_POWDER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_LEAF_STORM,
            MOVE_PSYCHIC,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_SOLAR_BEAM,
            MOVE_PSYCHIC,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_SOLAR_POWER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0103
    {
        .species = SPECIES_EXEGGUTOR_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_LEAF_STORM,
            MOVE_DRAGON_HAMMER,
            MOVE_WOOD_HAMMER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_EXEGGUTOR_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_DRAGON_HAMMER,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 128,
            .spa = 128
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GRASS,
    },

    // 0105
    {
        .species = SPECIES_MAROWAK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THICK_CLUB,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_BONEMERANG
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_MAROWAK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THICK_CLUB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0105
    {
        .species = SPECIES_MAROWAK_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THICK_CLUB,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_SHADOW_BONE,
            MOVE_BONEMERANG,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .atk = 252,
            .def = 8
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_MAROWAK_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THICK_CLUB,
        .moves =
        {
            MOVE_SHADOW_BONE,
            MOVE_FLARE_BLITZ,
            MOVE_BONEMERANG,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .atk = 252,
            .def = 8
        ),
        .teraType = TYPE_GHOST,
    },

    // 0106
    {
        .species = SPECIES_HITMONLEE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_QUICK_CLAW,
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_MACH_PUNCH,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_HITMONLEE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FIGHTING_GEM,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_BLAZE_KICK
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0107
    {
        .species = SPECIES_HITMONCHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_THUNDER_PUNCH,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_HITMONCHAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PUNCHING_GLOVE,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0110
    {
        .species = SPECIES_WEEZING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_WILL_O_WISP,
            MOVE_PAIN_SPLIT,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_WEEZING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_NEUTRALIZING_GAS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0110
    {
        .species = SPECIES_WEEZING_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STRANGE_STEAM,
            MOVE_SLUDGE_BOMB,
            MOVE_PROTECT,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_WEEZING_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_WILL_O_WISP,
            MOVE_TOXIC_SPIKES,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_NEUTRALIZING_GAS,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_WEEZING_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STRANGE_STEAM,
            MOVE_SLUDGE_BOMB,
            MOVE_HAZE,
            MOVE_DEFOG
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0113
    {
        .species = SPECIES_CHANSEY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE,
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_TOXIC,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0115
    {
        .species = SPECIES_KANGASKHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SUCKER_PUNCH,
            MOVE_POWER_UP_PUNCH
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_KANGASKHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0119
    {
        .species = SPECIES_SEAKING,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_WATERFALL,
            MOVE_MEGAHORN,
            MOVE_DRILL_RUN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SEAKING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_MEGAHORN,
            MOVE_AGILITY,
            MOVE_ICICLE_CRASH
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0121
    {
        .species = SPECIES_STARMIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_PSYSHOCK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_STARMIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RAPID_SPIN,
            MOVE_RECOVER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_STARMIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_TRICK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0122
    {
        .species = SPECIES_MR_MIME,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_DAZZLING_GLEAM,
            MOVE_FOLLOW_ME
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MR_MIME,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TWISTED_SPOON,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_ICY_WIND,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0124
    {
        .species = SPECIES_JYNX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_LOVELY_KISS,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_JYNX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NEVER_MELT_ICE,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0127
    {
        .species = SPECIES_PINSIR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHARP_BEAK,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_RETURN,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_AERILATE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_PINSIR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_DOUBLE_EDGE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_AERILATE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0128
    {
        .species = SPECIES_TAUROS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_ZEN_HEADBUTT,
            MOVE_IRON_HEAD
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_TAUROS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0128
    {
        .species = SPECIES_TAUROS_PALDEA_COMBAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_TAUROS_PALDEA_COMBAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0128
    {
        .species = SPECIES_TAUROS_PALDEA_BLAZE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_FLARE_BLITZ,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TAUROS_PALDEA_BLAZE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_ROCK_SLIDE,
            MOVE_PROTECT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0128
    {
        .species = SPECIES_TAUROS_PALDEA_AQUA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_WAVE_CRASH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TAUROS_PALDEA_AQUA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MUSCLE_BAND,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0130
    {
        .species = SPECIES_GYARADOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GYARADOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_POWER_WHIP,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GYARADOS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FLYING_GEM,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_BOUNCE,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0131
    {
        .species = SPECIES_LAPRAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_SURF,
            MOVE_THUNDERBOLT,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_LAPRAS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_SNOWSCAPE,
            MOVE_AURORA_VEIL,
            MOVE_FREEZE_DRY,
            MOVE_SURF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0132
    {
        .species = SPECIES_DITTO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_TRANSFORM,
            MOVE_TRANSFORM,
            MOVE_TRANSFORM,
            MOVE_TRANSFORM
        },
        .ability = ABILITY_IMPOSTER,
        .nature = NATURE_NEUTRAL,
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0134
    {
        .species = SPECIES_VAPOREON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_VAPOREON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0135
    {
        .species = SPECIES_JOLTEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_SHADOW_BALL,
            MOVE_HYPER_VOICE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_JOLTEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_SHADOW_BALL,
            MOVE_ALLURING_VOICE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0136
    {
        .species = SPECIES_FLAREON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_DOUBLE_EDGE,
            MOVE_SUPERPOWER,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_FLAREON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_FLARE_BLITZ,
            MOVE_SUPERPOWER,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0139
    {
        .species = SPECIES_OMASTAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_HYDRO_PUMP,
            MOVE_POWER_GEM,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_OMASTAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HARD_STONE,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_POWER_GEM,
            MOVE_EARTH_POWER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0141
    {
        .species = SPECIES_KABUTOPS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_KABUTOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_AQUA_JET,
            MOVE_STONE_EDGE,
            MOVE_X_SCISSOR,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_JOLTEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_KINGS_ROCK,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_SHADOW_BALL,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0142
    {
        .species = SPECIES_AERODACTYL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_KINGS_ROCK,
        .moves =
        {
            MOVE_ROCK_SLIDE,
            MOVE_DUAL_WINGBEAT,
            MOVE_EARTHQUAKE,
            MOVE_AQUA_TAIL
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_AERODACTYL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HARD_STONE,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_TAUNT,
            MOVE_HEAD_SMASH,
            MOVE_DUAL_WINGBEAT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0143
    {
        .species = SPECIES_SNORLAX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_CURSE,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_SNORLAX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_CRUNCH,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 132,
            .atk = 252,
            .spd = 124
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_SNORLAX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FIGY_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0144
    {
        .species = SPECIES_ARTICUNO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_HAZE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_ARTICUNO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BLIZZARD,
            MOVE_HURRICANE,
            MOVE_FREEZE_DRY,
            MOVE_ROOST
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0144
    {
        .species = SPECIES_ARTICUNO_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FREEZING_GLARE,
            MOVE_HURRICANE,
            MOVE_CALM_MIND,
            MOVE_ROOST
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_ARTICUNO_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TWISTED_SPOON,
        .moves =
        {
            MOVE_FREEZING_GLARE,
            MOVE_PSYCHIC,
            MOVE_HURRICANE,
            MOVE_RECOVER
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0145
    {
        .species = SPECIES_ZAPDOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_ROOST
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ZAPDOS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE,
            MOVE_ROOST
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .spd = 176,
            .spe = 84
        ),
        .teraType = TYPE_GROUND,
    },

    // 0145
    {
        .species = SPECIES_ZAPDOS_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_THUNDEROUS_KICK,
            MOVE_CLOSE_COMBAT,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_ZAPDOS_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_THUNDEROUS_KICK,
            MOVE_BRAVE_BIRD,
            MOVE_BULK_UP,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0146
    {
        .species = SPECIES_MOLTRES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_MOLTRES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_HURRICANE,
            MOVE_SCORCHING_SANDS,
            MOVE_U_TURN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0146
    {
        .species = SPECIES_MOLTRES_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FIERY_WRATH,
            MOVE_HURRICANE,
            MOVE_NASTY_PLOT,
            MOVE_AGILITY
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MOLTRES_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_FIERY_WRATH,
            MOVE_AIR_SLASH,
            MOVE_NASTY_PLOT,
            MOVE_HURRICANE
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0149
    {
        .species = SPECIES_DRAGONITE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_EXTREME_SPEED,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_AERILATE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_DRAGONITE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_ROOST,
            MOVE_DRAGON_TAIL,
            MOVE_EXTREME_SPEED,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_AERILATE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0150
    {
        .species = SPECIES_MEWTWO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYSTRIKE,
            MOVE_AURA_SPHERE,
            MOVE_ICE_BEAM,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_MEWTWO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MUSCLE_BAND,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_PSYCHIC_FANGS,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_MEWTWO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYSTRIKE,
            MOVE_AURA_SPHERE,
            MOVE_FIRE_BLAST,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0151
    {
        .species = SPECIES_MEW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_AURA_SPHERE,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_MEW,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_TAUNT,
            MOVE_ROOST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_MEW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_EARTHQUAKE,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // ====================================
    // Generation II
    // ====================================

    // 0154
    {
        .species = SPECIES_MEGANIUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_AROMATHERAPY,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MIRACLE_SEED,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_HORN_LEECH,
            MOVE_PLAY_ROUGH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_TOXIC,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_TERRAIN_EXTENDER,
        .moves =
        {
            MOVE_GRASSY_GLIDE,
            MOVE_HELPING_HAND,
            MOVE_BODY_PRESS,
            MOVE_PROTECT
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0157
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_EARTH_POWER,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_ERUPTION,
            MOVE_FIRE_BLAST,
            MOVE_SOLAR_BEAM,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0157
    {
        .species = SPECIES_TYPHLOSION_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_INFERNAL_PARADE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TYPHLOSION_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_HEX,
            MOVE_FIRE_BLAST,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0160
    {
        .species = SPECIES_FERALIGATR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH,
            MOVE_CRUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_FERALIGATR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_FERALIGATR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0162
    {
        .species = SPECIES_FURRET,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_SUPER_FANG
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_FURRET,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_HELPING_HAND,
            MOVE_KNOCK_OFF,
            MOVE_PROTECT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0164
    {
        .species = SPECIES_NOCTOWL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_NOCTOWL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_NOCTOWL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHARP_BEAK,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HYPER_VOICE,
            MOVE_AIR_SLASH,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0166
    {
        .species = SPECIES_LEDIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_BUG_BUZZ,
            MOVE_AURA_SPHERE,
            MOVE_ROOST
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_LEDIAN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_TAILWIND,
            MOVE_BUG_BUZZ
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0168
    {
        .species = SPECIES_ARIADOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_TOXIC_SPIKES,
            MOVE_POISON_JAB,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_ARIADOS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_POISON_JAB,
            MOVE_SUCKER_PUNCH,
            MOVE_LEECH_LIFE
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_BUG,
    },

    // 0169
    {
        .species = SPECIES_CROBAT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_LEECH_LIFE,
            MOVE_POISON_FANG,
            MOVE_PROTECT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_CROBAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_TAUNT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CROBAT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_GUNK_SHOT,
            MOVE_U_TURN,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0171
    {
        .species = SPECIES_LANTURN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_VOLT_SWITCH,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_LANTURN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 132,
            .spd = 124
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_LANTURN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM,
            MOVE_THUNDER_WAVE,
            MOVE_PROTECT
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0178
    {
        .species = SPECIES_XATU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_ROOST,
            MOVE_THUNDER_WAVE,
            MOVE_PSYCHIC,
            MOVE_U_TURN
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_XATU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_TWISTED_SPOON,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_PSYCHIC,
            MOVE_AIR_SLASH,
            MOVE_U_TURN
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0181
    {
        .species = SPECIES_AMPHAROS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_AMPHAROS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_DRAGON_PULSE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_AMPHAROS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DRAGON_PULSE,
            MOVE_ENERGY_BALL,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0182
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_MOONBLAST,
            MOVE_WEATHER_BALL
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_SOLAR_BEAM,
            MOVE_WEATHER_BALL,
            MOVE_POLLEN_PUFF,
            MOVE_PROTECT
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0184
    {
        .species = SPECIES_AZUMARILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_PLAY_ROUGH,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 92,
            .atk = 252,
            .spe = 164
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_AZUMARILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_AQUA_JET,
            MOVE_PLAY_ROUGH,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 92,
            .atk = 252,
            .spe = 164
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_AZUMARILL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_AQUA_JET,
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0185
    {
        .species = SPECIES_SUDOWOODO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_WOOD_HAMMER,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_SUDOWOODO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0186
    {
        .species = SPECIES_POLITOED,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DAMP_ROCK,
        .moves =
        {
            MOVE_WEATHER_BALL,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_ENCORE
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_POLITOED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_ENCORE,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 200,
            .spd = 56
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_POLITOED,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0189
    {
        .species = SPECIES_JUMPLUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SLEEP_POWDER,
            MOVE_LEECH_SEED,
            MOVE_SUBSTITUTE,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_COTTON_DOWN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_JUMPLUFF,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_SLEEP_POWDER,
            MOVE_STRENGTH_SAP,
            MOVE_TAILWIND
        },
        .ability = ABILITY_COTTON_DOWN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0192
    {
        .species = SPECIES_SUNFLORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_EARTH_POWER,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_SOLAR_POWER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SUNFLORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_GIGA_DRAIN,
            MOVE_WEATHER_BALL,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SOLAR_POWER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0195
    {
        .species = SPECIES_QUAGSIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_QUAGSIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0196
    {
        .species = SPECIES_ESPEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_ESPEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_TRICK
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0197
    {
        .species = SPECIES_UMBREON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_TOXIC
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_UMBREON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_SNARL,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0199
    {
        .species = SPECIES_SLOWKING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FUTURE_SIGHT,
            MOVE_SLACK_OFF,
            MOVE_PSYCHIC_NOISE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 16,
            .spd = 240
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SLOWKING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_PSYSHOCK,
            MOVE_ICE_BEAM,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SLOWKING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TWISTED_SPOON,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_CHILLING_WATER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_SLOWKING,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_INSTRUCT,
            MOVE_HELPING_HAND,
            MOVE_CHILLING_WATER,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 132,
            .spd = 124
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0199
    {
        .species = SPECIES_SLOWKING_GALAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_HEX,
            MOVE_SLUDGE_BOMB,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_CURIOUS_MEDICINE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_SLOWKING_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SLUDGE_BOMB,
            MOVE_PSYCHIC,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_CURIOUS_MEDICINE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0201
    {
        .species = SPECIES_UNOWN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_ANCIENT_POWER,
            MOVE_PSYCHIC,
            MOVE_ENCORE,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_NEUTRALIZING_GAS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_UNOWN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_ALLY_SWITCH,
            MOVE_PSYCHIC,
            MOVE_MEMENTO
        },
        .ability = ABILITY_NEUTRALIZING_GAS,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_NORMAL,
    },

    // 0202
    {
        .species = SPECIES_WOBBUFFET,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_COUNTER,
            MOVE_MIRROR_COAT,
            MOVE_ENCORE,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_WOBBUFFET,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_ENCORE,
            MOVE_COUNTER,
            MOVE_MIRROR_COAT,
            MOVE_TICKLE
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0205
    {
        .species = SPECIES_FORRETRESS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_RAPID_SPIN,
            MOVE_GYRO_BALL
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_FORRETRESS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_VOLT_SWITCH,
            MOVE_RAPID_SPIN,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_STEEL,
    },

    // 0208
    {
        .species = SPECIES_STEELIX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_STEALTH_ROCK,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_STEELIX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_TOXIC
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_STEELIX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_STONE_EDGE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0210
    {
        .species = SPECIES_GRANBULL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_GRANBULL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_KNOCK_OFF,
            MOVE_ROAR,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0211
    {
        .species = SPECIES_QWILFISH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_TOXIC_SPIKES,
            MOVE_LIQUIDATION,
            MOVE_HAZE
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_QWILFISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_GUNK_SHOT,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0212
    {
        .species = SPECIES_SCIZOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_METAL_COAT,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BULLET_PUNCH,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_SCIZOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BULLET_PUNCH,
            MOVE_U_TURN,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_SCIZOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULLET_PUNCH,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0213
    {
        .species = SPECIES_SHUCKLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_ENCORE,
            MOVE_KNOCK_OFF,
            MOVE_TOXIC
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_SHUCKLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_POWER_TRICK,
            MOVE_SUPERPOWER,
            MOVE_GYRO_BALL,
            MOVE_REST
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0214
    {
        .species = SPECIES_HERACROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_PIN_MISSILE,
            MOVE_ROCK_BLAST,
            MOVE_BULLET_SEED,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_HERACROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_MEGAHORN,
            MOVE_KNOCK_OFF,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_HERACROSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_MEGAHORN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0217
    {
        .species = SPECIES_URSARING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_URSARING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0219
    {
        .species = SPECIES_MAGCARGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_FIRE_BLAST,
            MOVE_EARTH_POWER,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPA_UP, DEF_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_MAGCARGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_FIRE_BLAST,
            MOVE_EARTH_POWER,
            MOVE_RECOVER
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 132,
            .spa = 124
        ),
        .teraType = TYPE_FIRE,
    },

    // 0222
    {
        .species = SPECIES_CORSOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_RECOVER,
            MOVE_CHILLING_WATER,
            MOVE_POWER_GEM,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CORSOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_RECOVER,
            MOVE_CHILLING_WATER,
            MOVE_POWER_GEM,
            MOVE_TOXIC
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0224
    {
        .species = SPECIES_OCTILLERY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FIRE_BLAST,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_OCTILLERY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_MUDDY_WATER,
            MOVE_SLUDGE_BOMB,
            MOVE_ENERGY_BALL,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0225
    {
        .species = SPECIES_DELIBIRD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_ICE_SPINNER,
            MOVE_ICICLE_SPEAR,
            MOVE_DRILL_PECK,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_DELIBIRD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_ICY_WIND,
            MOVE_ICE_SHARD,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0226
    {
        .species = SPECIES_MANTINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MANTINE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_HURRICANE,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0227
    {
        .species = SPECIES_SKARMORY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_ROOST,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_SKARMORY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0229
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HEAT_WAVE,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0230
    {
        .species = SPECIES_KINGDRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_ICE_BEAM,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KINGDRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_OUTRAGE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KINGDRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0232
    {
        .species = SPECIES_DONPHAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DONPHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DONPHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_HEAVY_SLAM,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0233
    {
        .species = SPECIES_PORYGON2,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE,
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_ICE_BEAM,
            MOVE_RECOVER,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_PORYGON2,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EVIOLITE,
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_RECOVER
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0235
    {
        .species = SPECIES_SMEARGLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPORE,
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_SMEARGLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_SPORE,
            MOVE_FAKE_OUT,
            MOVE_FOLLOW_ME,
            MOVE_KINGS_SHIELD
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0237
    {
        .species = SPECIES_HITMONTOP,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_HITMONTOP,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_RAPID_SPIN,
            MOVE_CLOSE_COMBAT,
            MOVE_MACH_PUNCH,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0241
    {
        .species = SPECIES_MILTANK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_MILK_DRINK,
            MOVE_HEAL_BELL,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MILTANK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0242
    {
        .species = SPECIES_BLISSEY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_HEAL_BELL,
            MOVE_TOXIC
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .def = 252,
            .spd = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_BLISSEY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .def = 252,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0243
    {
        .species = SPECIES_RAIKOU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_THUNDERBOLT,
            MOVE_AURA_SPHERE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_RAIKOU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_RAIKOU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE,
            MOVE_SNARL
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0244
    {
        .species = SPECIES_ENTEI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ENTEI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_BULLDOZE
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ENTEI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0245
    {
        .species = SPECIES_SUICUNE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SUICUNE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SUICUNE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_DEFOG,
            MOVE_REST,
            MOVE_SLEEP_TALK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0248
    {
        .species = SPECIES_TYRANITAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SMOOTH_ROCK,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_CRUNCH,
            MOVE_FIRE_BLAST,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_ROCK,
    },

    // 0249
    {
        .species = SPECIES_LUGIA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_AEROBLAST,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_LUGIA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_AEROBLAST,
            MOVE_PSYCHIC,
            MOVE_ROOST
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 196,
            .spe = 60
        ),
        .teraType = TYPE_FLYING,
    },

    // 0250
    {
        .species = SPECIES_HO_OH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_BRAVE_BIRD,
            MOVE_EARTHQUAKE,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_HO_OH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_BRAVE_BIRD,
            MOVE_RECOVER,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0251
    {
        .species = SPECIES_CELEBI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_CELEBI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_RECOVER,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 240,
            .spd = 16
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CELEBI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_RECOVER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 196,
            .spe = 60
        ),
        .teraType = TYPE_FAIRY,
    },

    // ====================================
    // Generation III
    // ====================================

    // 0254
    {
        .species = SPECIES_SCEPTILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_PETAYA_BERRY,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_GIGA_DRAIN,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_SCEPTILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SCEPTILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0257
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_JUMP_KICK,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0260
    {
        .species = SPECIES_SWAMPERT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SWAMPERT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_CHILLING_WATER,
            MOVE_EARTHQUAKE,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 216,
            .spd = 40
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SWAMPERT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_FLIP_TURN,
            MOVE_EARTHQUAKE,
            MOVE_ICE_BEAM,
            MOVE_POWER_GEM
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 128,
            .spa = 128
        ),
        .teraType = TYPE_WATER,
    },

    // 0262
    {
        .species = SPECIES_MIGHTYENA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MIGHTYENA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_SUCKER_PUNCH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0264
    {
        .species = SPECIES_LINOONE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_EXTREME_SPEED,
            MOVE_SEED_BOMB,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_LINOONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_EXTREME_SPEED,
            MOVE_KNOCK_OFF,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0267
    {
        .species = SPECIES_BEAUTIFLY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_BEAUTIFLY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_POLLEN_PUFF,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0269
    {
        .species = SPECIES_DUSTOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_SLUDGE_BOMB,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_DUSTOX,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TOXIC_SPIKES,
            MOVE_STRUGGLE_BUG,
            MOVE_SLUDGE_BOMB,
            MOVE_ROOST
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0272
    {
        .species = SPECIES_LUDICOLO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_GIGA_DRAIN,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_LUDICOLO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_LUDICOLO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ENERGY_BALL,
            MOVE_ICE_BEAM,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0275
    {
        .species = SPECIES_SHIFTRY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_HEAT_WAVE,
            MOVE_DARK_PULSE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SHIFTRY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_DEFOG
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SHIFTRY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_LEAF_BLADE,
            MOVE_SUCKER_PUNCH,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0277
    {
        .species = SPECIES_SWELLOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_SWELLOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_U_TURN,
            MOVE_STEEL_WING
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0279
    {
        .species = SPECIES_PELIPPER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DAMP_ROCK,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HYDRO_PUMP,
            MOVE_U_TURN,
            MOVE_ROOST
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .spa = 252,
            .spe = 8
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_PELIPPER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_HURRICANE,
            MOVE_DEFOG,
            MOVE_ROOST
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 252,
            .spd = 8
        ),
        .teraType = TYPE_GROUND,
    },

    // 0282
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_PSYSHOCK,
            MOVE_MOONBLAST,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_TRICK
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_PSYSHOCK,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0284
    {
        .species = SPECIES_MASQUERAIN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_HYDRO_PUMP
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_MASQUERAIN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_TAILWIND,
            MOVE_ICY_WIND,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0286
    {
        .species = SPECIES_BRELOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_SPORE,
            MOVE_BULLET_SEED,
            MOVE_MACH_PUNCH,
            MOVE_ROCK_TOMB
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_BRELOOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_SPORE,
            MOVE_SUBSTITUTE,
            MOVE_FOCUS_PUNCH,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 236,
            .spd = 236,
            .spe = 36
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_BRELOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPORE,
            MOVE_BULLET_SEED,
            MOVE_MACH_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0289
    {
        .species = SPECIES_SLAKING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_RETURN,
            MOVE_EARTHQUAKE,
            MOVE_GIGA_IMPACT,
            MOVE_NIGHT_SLASH
        },
        .ability = ABILITY_TRUANT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_SLAKING,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_GUNK_SHOT,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_TRUANT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_SLAKING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_NIGHT_SLASH,
            MOVE_HAMMER_ARM
        },
        .ability = ABILITY_TRUANT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0291
    {
        .species = SPECIES_NINJASK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_X_SCISSOR,
            MOVE_AERIAL_ACE,
            MOVE_DIG
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_NINJASK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SUBSTITUTE,
            MOVE_X_SCISSOR,
            MOVE_PROTECT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0292
    {
        .species = SPECIES_SHEDINJA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_X_SCISSOR,
            MOVE_SHADOW_SNEAK,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_WONDER_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0295
    {
        .species = SPECIES_EXPLOUD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_EXPLOUD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_OVERHEAT,
            MOVE_SURF,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0297
    {
        .species = SPECIES_HARIYAMA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_FAKE_OUT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_HARIYAMA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_HEAVY_SLAM,
            MOVE_BULLET_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 168,
            .atk = 252,
            .spd = 88
        ),
        .teraType = TYPE_STEEL,
    },

    // 0301
    {
        .species = SPECIES_DELCATTY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_FAKE_OUT,
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_NORMALIZE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0302
    {
        .species = SPECIES_SABLEYE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DARK_PULSE,
            MOVE_RECOVER,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SABLEYE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_RECOVER,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0303
    {
        .species = SPECIES_MAWILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FAIRY_FEATHER,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_PLAY_ROUGH,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MAWILE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_PLAY_ROUGH,
            MOVE_IRON_HEAD,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0306
    {
        .species = SPECIES_AGGRON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_ROAR
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_AGGRON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_AVALANCHE
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_AGGRON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_AUTOTOMIZE,
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0308
    {
        .species = SPECIES_MEDICHAM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_BELT,
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_FAKE_OUT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_MEDICHAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_TRICK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0310
    {
        .species = SPECIES_MANECTRIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_FLAMETHROWER,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_MANECTRIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_OVERHEAT,
            MOVE_THUNDER
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0311
    {
        .species = SPECIES_PLUSLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_PLUS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0312
    {
        .species = SPECIES_MINUN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_MINUS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0313
    {
        .species = SPECIES_VOLBEAT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_DAMP_ROCK,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_RAIN_DANCE,
            MOVE_THUNDER_WAVE,
            MOVE_U_TURN
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .spd = 252,
            .spe = 8
        ),
        .teraType = TYPE_BUG,
    },

    // 0314
    {
        .species = SPECIES_ILLUMISE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_ENCORE,
            MOVE_HELPING_HAND,
            MOVE_BUG_BUZZ
        },
        .ability = ABILITY_LINGERING_AROMA,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 8,
            .spd = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0317
    {
        .species = SPECIES_SWALOT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_TOXIC,
            MOVE_PAIN_SPLIT,
            MOVE_ENCORE
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0319
    {
        .species = SPECIES_SHARPEDO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_PROTECT,
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SHARPEDO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SHARPEDO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_CLOSE_COMBAT,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0321
    {
        .species = SPECIES_WAILORD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_WATER_SPOUT,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_HYPER_VOICE
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0323
    {
        .species = SPECIES_CAMERUPT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_ERUPTION,
            MOVE_EARTH_POWER,
            MOVE_FIRE_BLAST,
            MOVE_ANCIENT_POWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CAMERUPT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LAVA_PLUME,
            MOVE_EARTH_POWER,
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0324
    {
        .species = SPECIES_TORKOAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_ERUPTION,
            MOVE_LAVA_PLUME,
            MOVE_SOLAR_BEAM,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TORKOAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LAVA_PLUME,
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_YAWN
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0326
    {
        .species = SPECIES_GRUMPIG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_REST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0327
    {
        .species = SPECIES_SPINDA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_SUPERPOWER,
            MOVE_BODY_SLAM,
            MOVE_FACADE,
            MOVE_ENCORE
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0330
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_U_TURN,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_U_TURN,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SOFT_SAND,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DEFOG,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN,
            MOVE_TOXIC
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 196,
            .spe = 60
        ),
        .teraType = TYPE_GROUND,
    },

    // 0332
    {
        .species = SPECIES_CACTURNE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SEED_BOMB,
            MOVE_SUCKER_PUNCH,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_CACTURNE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_SEED_BOMB,
            MOVE_SUCKER_PUNCH,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0334
    {
        .species = SPECIES_ALTARIA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FAIRY_FEATHER,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_RETURN,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ALTARIA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 8,
            .spd = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0335
    {
        .species = SPECIES_ZANGOOSE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_ZANGOOSE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0336
    {
        .species = SPECIES_SEVIPER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0337
    {
        .species = SPECIES_LUNATONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_LUNATONE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_COSMIC_POWER,
            MOVE_STORED_POWER,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0338
    {
        .species = SPECIES_SOLROCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_STONE_EDGE,
            MOVE_ZEN_HEADBUTT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_SOLROCK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_STEALTH_ROCK,
            MOVE_HELPING_HAND,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_ROCK,
    },

    // 0340
    {
        .species = SPECIES_WHISCASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0342
    {
        .species = SPECIES_CRAWDAUNT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_CRAWDAUNT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CRAWDAUNT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CRABHAMMER,
            MOVE_KNOCK_OFF,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0344
    {
        .species = SPECIES_CLAYDOL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_EARTH_POWER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0346
    {
        .species = SPECIES_CRADILY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CURSE,
            MOVE_SEED_BOMB,
            MOVE_STONE_EDGE,
            MOVE_RECOVER
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0348
    {
        .species = SPECIES_ARMALDO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE,
            MOVE_X_SCISSOR,
            MOVE_AQUA_TAIL
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_ARMALDO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0350
    {
        .species = SPECIES_MILOTIC,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_ICE_BEAM,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 252,
            .spd = 8
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_MILOTIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 252,
            .spd = 8
        ),
        .teraType = TYPE_WATER,
    },

    // 0351
    {
        .species = SPECIES_CASTFORM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_WEATHER_BALL,
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM,
            MOVE_SUNNY_DAY
        },
        .ability = ABILITY_FORECAST,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0352
    {
        .species = SPECIES_KECLEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_DRAIN_PUNCH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0354
    {
        .species = SPECIES_BANETTE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SPELL_TAG,
        .moves =
        {
            MOVE_SHADOW_CLAW,
            MOVE_KNOCK_OFF,
            MOVE_WILL_O_WISP,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_BANETTE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SHADOW_CLAW,
            MOVE_SUCKER_PUNCH,
            MOVE_DESTINY_BOND,
            MOVE_TAUNT
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0356
    {
        .species = SPECIES_DUSCLOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE,
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_NIGHT_SHADE,
            MOVE_PAIN_SPLIT,
            MOVE_HEX
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 168,
            .spd = 88
        ),
        .teraType = TYPE_GHOST,
    },

    // 0357
    {
        .species = SPECIES_TROPIUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_SUBSTITUTE,
            MOVE_AIR_SLASH,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SOLAR_POWER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0358
    {
        .species = SPECIES_CHIMECHO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_RECOVER
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0359
    {
        .species = SPECIES_ABSOL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_PLAY_ROUGH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_ABSOL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_PSYCHO_CUT
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_ABSOL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_NIGHT_SLASH,
            MOVE_PSYCHO_CUT,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0362
    {
        .species = SPECIES_GLALIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NEVER_MELT_ICE,
        .moves =
        {
            MOVE_RETURN,
            MOVE_ICICLE_CRASH,
            MOVE_EARTHQUAKE,
            MOVE_FREEZE_DRY
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_GLALIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICE_BEAM,
            MOVE_FREEZE_DRY,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0365
    {
        .species = SPECIES_WALREIN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_TOXIC,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_WALREIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_FREEZE_DRY,
            MOVE_BODY_SLAM
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0367
    {
        .species = SPECIES_HUNTAIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_WATERFALL,
            MOVE_CRUNCH,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0368
    {
        .species = SPECIES_GOREBYSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0369
    {
        .species = SPECIES_RELICANTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_HEAD_SMASH,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_RELICANTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_YAWN,
            MOVE_WATERFALL,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0370
    {
        .species = SPECIES_LUVDISC,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_SWEET_KISS,
            MOVE_SOAK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_LOVE,
    },

    // 0373
    {
        .species = SPECIES_SALAMENCE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_SALAMENCE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_SALAMENCE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0376
    {
        .species = SPECIES_METAGROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_METEOR_MASH,
            MOVE_ZEN_HEADBUTT,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_METAGROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_METEOR_MASH,
            MOVE_BULLET_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_METAGROSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_AGILITY,
            MOVE_METEOR_MASH,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0377
    {
        .species = SPECIES_REGIROCK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CURSE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 96,
            .def = 160
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_REGIROCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HAMMER_ARM
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0378
    {
        .species = SPECIES_REGICE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_THUNDER_WAVE,
            MOVE_REST
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_REGICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0379
    {
        .species = SPECIES_REGISTEEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CURSE,
            MOVE_IRON_HEAD,
            MOVE_BODY_PRESS,
            MOVE_REST
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_REGISTEEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_IRON_HEAD,
            MOVE_THUNDER_WAVE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0380
    {
        .species = SPECIES_LATIAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DRAGON_PULSE,
            MOVE_ROOST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 124,
            .spe = 132
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_LATIAS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_HEALING_WISH
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0381
    {
        .species = SPECIES_LATIOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DRAGON_FANG,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DRACO_METEOR,
            MOVE_ROOST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_LATIOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_PSYCHIC,
            MOVE_AURA_SPHERE,
            MOVE_TRICK
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0382
    {
        .species = SPECIES_KYOGRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLUE_ORB,
        .moves =
        {
            MOVE_WATER_SPOUT,
            MOVE_ORIGIN_PULSE,
            MOVE_ICE_BEAM,
            MOVE_THUNDER
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KYOGRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_WATER_SPOUT,
            MOVE_ORIGIN_PULSE,
            MOVE_ICE_BEAM,
            MOVE_THUNDER
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KYOGRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_ORIGIN_PULSE,
            MOVE_ICE_BEAM,
            MOVE_REST
        },
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0383
    {
        .species = SPECIES_GROUDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_RED_ORB,
        .moves =
        {
            MOVE_PRECIPICE_BLADES,
            MOVE_FIRE_PUNCH,
            MOVE_STONE_EDGE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GROUDON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_PRECIPICE_BLADES,
            MOVE_LAVA_PLUME,
            MOVE_ROAR
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0384
    {
        .species = SPECIES_RAYQUAZA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NONE,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_DRAGON_ASCENT,
            MOVE_EARTHQUAKE,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_AIR_LOCK,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_RAYQUAZA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_EARTHQUAKE,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_AIR_LOCK,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0385
    {
        .species = SPECIES_JIRACHI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_IRON_HEAD,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_JIRACHI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WISH,
            MOVE_IRON_HEAD,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_JIRACHI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0386
    {
        .species = SPECIES_DEOXYS_ATTACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHO_BOOST,
            MOVE_ICE_BEAM,
            MOVE_SUPERPOWER,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 128,
            .spa = 128,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_DEOXYS_ATTACK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_PSYCHO_BOOST,
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0386
    {
        .species = SPECIES_DEOXYS_SPEED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_TAUNT,
            MOVE_PSYCHO_BOOST
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_DEOXYS_SPEED,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHO_BOOST,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0386
    {
        .species = SPECIES_DEOXYS_DEFENSE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_TOXIC,
            MOVE_RECOVER
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // ====================================
    // Generation IV
    // ====================================

    // 0389
    {
        .species = SPECIES_TORTERRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_TORTERRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0392
    {
        .species = SPECIES_INFERNAPE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_GUNK_SHOT,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_INFERNAPE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_FAKE_OUT,
            MOVE_CLOSE_COMBAT,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_INFERNAPE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_OVERHEAT,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0395
    {
        .species = SPECIES_EMPOLEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FLASH_CANNON,
            MOVE_ROOST,
            MOVE_DEFOG
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_EMPOLEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_FLASH_CANNON,
            MOVE_ICE_BEAM,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0398
    {
        .species = SPECIES_STARAPTOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_STARAPTOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FLYING_GEM,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_QUICK_ATTACK,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0400
    {
        .species = SPECIES_BIBAREL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WATERFALL,
            MOVE_BODY_SLAM,
            MOVE_YAWN,
            MOVE_ROOST
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },

    // 0402
    {
        .species = SPECIES_KRICKETUNE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WIDE_LENS,
        .moves =
        {
            MOVE_FURY_CUTTER,
            MOVE_BUG_BITE,
            MOVE_POUNCE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
        .ball = BALL_NET,
    },

    // 0405
    {
        .species = SPECIES_LUXRAY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_CRUNCH,
            MOVE_SUPERPOWER,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_LUXRAY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_CRUNCH,
            MOVE_SUPERPOWER,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0407
    {
        .species = SPECIES_ROSERADE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_SLUDGE_BOMB,
            MOVE_SLEEP_POWDER,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_ROSERADE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB,
            MOVE_SPIKES,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0409
    {
        .species = SPECIES_RAMPARDOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_CLOSE_COMBAT,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_RAMPARDOS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCK_GEM,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0411
    {
        .species = SPECIES_BASTIODON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_IRON_HEAD,
            MOVE_ROAR,
            MOVE_TOXIC
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0413
    {
        .species = SPECIES_WORMADAM_PLANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_WORMADAM_PLANT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_RAGE_POWDER,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_WORMADAM_SANDY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SMOOTH_ROCK,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_WORMADAM_SANDY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SOFT_SAND,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_ROCK_SLIDE,
            MOVE_WIDE_GUARD
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_WORMADAM_TRASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_METAL_COAT,
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_WORMADAM_TRASH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_IRON_BALL,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_GYRO_BALL,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_WATER,
    },

    // 0414
    {
        .species = SPECIES_MOTHIM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SILVER_POWDER,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 128,
            .spa = 128,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MOTHIM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHARP_BEAK,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_LUNGE,
            MOVE_U_TURN,
            MOVE_POLLEN_PUFF
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0416
    {
        .species = SPECIES_VESPIQUEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 8,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0417
    {
        .species = SPECIES_PACHIRISU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_NUZZLE,
            MOVE_SUPER_FANG,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0419
    {
        .species = SPECIES_FLOATZEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_FLOATZEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0421
    {
        .species = SPECIES_CHERRIM,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_HEAT_ROCK,
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_WEATHER_BALL,
            MOVE_HELPING_HAND,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_FLOWER_GIFT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 128,
            .spd = 128
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CHERRIM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_GROWTH,
            MOVE_WEATHER_BALL,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_FLOWER_GIFT,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0423
    {
        .species = SPECIES_GASTRODON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_RECOVER
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GASTRODON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_CLEAR_SMOG
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0424
    {
        .species = SPECIES_AMBIPOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_DOUBLE_HIT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_AMBIPOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DOUBLE_HIT,
            MOVE_KNOCK_OFF,
            MOVE_LOW_KICK,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0426
    {
        .species = SPECIES_DRIFBLIM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_QUICK_CLAW,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SHADOW_BALL,
            MOVE_AIR_SLASH,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_DRIFBLIM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_AIR_SLASH,
            MOVE_HEX,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0428
    {
        .species = SPECIES_LOPUNNY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_HIGH_JUMP_KICK,
            MOVE_RETURN,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_LOPUNNY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_SWITCHEROO,
            MOVE_RETURN,
            MOVE_HIGH_JUMP_KICK
        },
        .ability = ABILITY_KLUTZ,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0429
    {
        .species = SPECIES_MISMAGIUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MISMAGIUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_COLBUR_BERRY,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_WILL_O_WISP,
            MOVE_TAUNT,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0430
    {
        .species = SPECIES_HONCHKROW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SUCKER_PUNCH,
            MOVE_BRAVE_BIRD,
            MOVE_HEAT_WAVE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_HONCHKROW,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_SUCKER_PUNCH,
            MOVE_NIGHT_SLASH,
            MOVE_PSYCHO_CUT
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0432
    {
        .species = SPECIES_PURUGLY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_PLAY_ROUGH
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0435
    {
        .species = SPECIES_SKUNTANK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_CRUNCH,
            MOVE_FIRE_BLAST,
            MOVE_PURSUIT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0437
    {
        .species = SPECIES_BRONZONG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_STEALTH_ROCK,
            MOVE_GYRO_BALL,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_BRONZONG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_PSYCHIC,
            MOVE_TOXIC
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0441
    {
        .species = SPECIES_CHATOT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_HEAT_WAVE,
            MOVE_HURRICANE,
            MOVE_U_TURN
        },
        .ability = ABILITY_AERILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0442
    {
        .species = SPECIES_SPIRITOMB,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DARK_PULSE,
            MOVE_SHADOW_BALL,
            MOVE_REST
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SPIRITOMB,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_SUCKER_PUNCH,
            MOVE_FOUL_PLAY,
            MOVE_WILL_O_WISP,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0445
    {
        .species = SPECIES_GARCHOMP,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SOFT_SAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_BLAST,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GARCHOMP,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GARCHOMP,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_TAIL,
            MOVE_SPIKES
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0448
    {
        .species = SPECIES_LUCARIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_METEOR_MASH,
            MOVE_CLOSE_COMBAT,
            MOVE_BULLET_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_LUCARIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_AURA_SPHERE,
            MOVE_FLASH_CANNON,
            MOVE_VACUUM_WAVE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_LUCARIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FIGHTING_GEM,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_BULLET_PUNCH,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0450
    {
        .species = SPECIES_HIPPOWDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_SLACK_OFF,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_HIPPOWDON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SMOOTH_ROCK,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_SLACK_OFF,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0452
    {
        .species = SPECIES_DRAPION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_EARTHQUAKE,
            MOVE_TAUNT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_DRAPION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_AQUA_TAIL
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0454
    {
        .species = SPECIES_TOXICROAK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_GUNK_SHOT,
            MOVE_DRAIN_PUNCH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_TOXICROAK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_DRAIN_PUNCH,
            MOVE_SUCKER_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0455
    {
        .species = SPECIES_CARNIVINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SEED_SOWER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0457
    {
        .species = SPECIES_LUMINEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_U_TURN,
            MOVE_DEFOG
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },

    // 0460
    {
        .species = SPECIES_ABOMASNOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ICY_ROCK,
        .moves =
        {
            MOVE_BLIZZARD,
            MOVE_GIGA_DRAIN,
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_ABOMASNOW,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ICY_ROCK,
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_BLIZZARD,
            MOVE_GIGA_DRAIN,
            MOVE_PROTECT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0461
    {
        .species = SPECIES_WEAVILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_KINGS_ROCK,
        .moves =
        {
            MOVE_TRIPLE_AXEL,
            MOVE_KNOCK_OFF,
            MOVE_ICE_SHARD,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_WEAVILE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_KNOCK_OFF,
            MOVE_ICICLE_CRASH,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0462
    {
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_METAL_COAT,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0463
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_BODY_SLAM,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_BODY_SLAM,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0464
    {
        .species = SPECIES_RHYPERIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_STEALTH_ROCK,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_RHYPERIOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_MEGAHORN
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0465
    {
        .species = SPECIES_TANGROWTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_LEECH_SEED,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_TANGROWTH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0466
    {
        .species = SPECIES_ELECTIVIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_CROSS_CHOP
        },
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ELECTIVIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 112,
            .atk = 252,
            .spe = 144
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0467
    {
        .species = SPECIES_MAGMORTAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_MAGMORTAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0468
    {
        .species = SPECIES_TOGEKISS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_AIR_SLASH,
            MOVE_DAZZLING_GLEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 80,
            .spd = 176
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_TOGEKISS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_TAILWIND,
            MOVE_DAZZLING_GLEAM,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_HALO,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0469
    {
        .species = SPECIES_YANMEGA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_GIGA_DRAIN,
            MOVE_PROTECT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_YANMEGA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_ANCIENT_POWER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0470
    {
        .species = SPECIES_LEAFEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_LEAFEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BIG_ROOT,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_SYNTHESIS,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0471
    {
        .species = SPECIES_GLACEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_BLIZZARD,
            MOVE_FREEZE_DRY,
            MOVE_WATER_PULSE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_GLACEON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_FREEZE_DRY,
            MOVE_ICE_BEAM,
            MOVE_PROTECT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0472
    {
        .species = SPECIES_GLISCOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_PROTECT,
            MOVE_TOXIC,
            MOVE_ROOST
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 244,
            .def = 248,
            .spe = 16
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GLISCOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_ROOST
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 244,
            .atk = 252,
            .spe = 12
        ),
        .teraType = TYPE_GROUND,
    },

    // 0473
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICICLE_CRASH,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICICLE_CRASH,
            MOVE_ICE_SHARD,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 16,
            .spd = 240
        ),
        .teraType = TYPE_ICE,
    },

    // 0474
    {
        .species = SPECIES_PORYGON_Z,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_TRI_ATTACK,
            MOVE_DARK_PULSE,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_PORYGON_Z,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_TRICK
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0475
    {
        .species = SPECIES_GALLADE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHO_CUT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_GALLADE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_SACRED_SWORD,
            MOVE_PSYCHO_CUT,
            MOVE_LEAF_BLADE,
            MOVE_NIGHT_SLASH
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0476
    {
        .species = SPECIES_PROBOPASS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_POWER_GEM,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0477
    {
        .species = SPECIES_DUSKNOIR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_WILL_O_WISP,
            MOVE_PAIN_SPLIT,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_DUSKNOIR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_POLTERGEIST,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GHOST,
    },

    // 0478
    {
        .species = SPECIES_FROSLASS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICE_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_FROSLASS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_THUNDERBOLT,
            MOVE_TAUNT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0479
    {
        .species = SPECIES_ROTOM_HEAT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_OVERHEAT,
            MOVE_VOLT_SWITCH,
            MOVE_THUNDERBOLT,
            MOVE_TRICK
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ROTOM_HEAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_OVERHEAT,
            MOVE_VOLT_SWITCH,
            MOVE_WILL_O_WISP,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .spd = 244,
            .spe = 16
        ),
        .teraType = TYPE_FIRE,
    },

    // 0479
    {
        .species = SPECIES_ROTOM_WASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_VOLT_SWITCH,
            MOVE_WILL_O_WISP,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 168,
            .spd = 92
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_ROTOM_WASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_VOLT_SWITCH,
            MOVE_THUNDERBOLT,
            MOVE_TRICK
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0479
    {
        .species = SPECIES_ROTOM_MOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_VOLT_SWITCH,
            MOVE_THUNDERBOLT,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0479
    {
        .species = SPECIES_ROTOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_SHADOW_BALL,
            MOVE_VOLT_SWITCH,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0479
    {
        .species = SPECIES_ROTOM_FROST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_BLIZZARD,
            MOVE_VOLT_SWITCH,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0479
    {
        .species = SPECIES_ROTOM_FAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_AIR_SLASH,
            MOVE_VOLT_SWITCH,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0480
    {
        .species = SPECIES_UXIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_PSYCHIC,
            MOVE_YAWN,
            MOVE_U_TURN
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0481
    {
        .species = SPECIES_MESPRIT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_ICE_BEAM,
            MOVE_U_TURN,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_MOODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0482
    {
        .species = SPECIES_AZELF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_TAUNT,
            MOVE_PSYCHIC,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_AZELF,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_FIRE_BLAST,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0483
    {
        .species = SPECIES_DIALGA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_DIALGA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_FIRE_BLAST,
            MOVE_ROAR_OF_TIME
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0483
    {
        .species = SPECIES_DIALGA_ORIGIN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ADAMANT_CRYSTAL,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0484
    {
        .species = SPECIES_PALKIA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_PALKIA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0484
    {
        .species = SPECIES_PALKIA_ORIGIN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LUSTROUS_GLOBE,
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0485
    {
        .species = SPECIES_HEATRAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_MAGMA_STORM,
            MOVE_EARTH_POWER,
            MOVE_STEALTH_ROCK,
            MOVE_TAUNT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 156,
            .spe = 100
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_HEATRAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_MAGMA_STORM,
            MOVE_EARTH_POWER,
            MOVE_FLASH_CANNON,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0486
    {
        .species = SPECIES_REGIGIGAS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_BODY_SLAM,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_SLOW_START,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0487
    {
        .species = SPECIES_GIRATINA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_TAIL,
            MOVE_WILL_O_WISP,
            MOVE_REST,
            MOVE_DEFOG
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0487
    {
        .species = SPECIES_GIRATINA_ORIGIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GRISEOUS_ORB,
        .moves =
        {
            MOVE_SHADOW_FORCE,
            MOVE_DRACO_METEOR,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0488
    {
        .species = SPECIES_CRESSELIA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_PSYSHOCK,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_CRESSELIA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HELPING_HAND,
            MOVE_ICY_WIND,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0489
    {
        .species = SPECIES_PHIONE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0490
    {
        .species = SPECIES_MANAPHY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_MANAPHY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_SURF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0491
    {
        .species = SPECIES_DARKRAI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0492
    {
        .species = SPECIES_SHAYMIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SEED_FLARE,
            MOVE_EARTH_POWER,
            MOVE_AIR_SLASH,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0492
    {
        .species = SPECIES_SHAYMIN_SKY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SEED_FLARE,
            MOVE_AIR_SLASH,
            MOVE_EARTH_POWER,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0493
    {
        .species = SPECIES_ARCEUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EXTREME_SPEED,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_MULTITYPE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_ARCEUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_JUDGMENT,
            MOVE_ICE_BEAM,
            MOVE_RECOVER
        },
        .ability = ABILITY_MULTITYPE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // ====================================
    // Generation V
    // ====================================

    // 0494
    {
        .species = SPECIES_VICTINI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_V_CREATE,
            MOVE_BOLT_STRIKE,
            MOVE_ZEN_HEADBUTT,
            MOVE_U_TURN
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_VICTINI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_BLUE_FLARE,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_VICTORY_STAR,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0497
    {
        .species = SPECIES_SERPERIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_DRAGON_PULSE,
            MOVE_GIGA_DRAIN,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SERPERIOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_GLARE,
            MOVE_LEECH_SEED,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0500
    {
        .species = SPECIES_EMBOAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_WILD_CHARGE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_EMBOAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_HEAT_WAVE,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0503
    {
        .species = SPECIES_SAMUROTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LIQUIDATION,
            MOVE_SACRED_SWORD,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SAMUROTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0503
    {
        .species = SPECIES_SAMUROTT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_CEASELESS_EDGE,
            MOVE_AQUA_JET,
            MOVE_SUCKER_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SAMUROTT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CEASELESS_EDGE,
            MOVE_LIQUIDATION,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0505
    {
        .species = SPECIES_WATCHOG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0508
    {
        .species = SPECIES_STOUTLAND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_RETURN,
            MOVE_SUPERPOWER,
            MOVE_CRUNCH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_STOUTLAND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_TOXIC,
            MOVE_ROAR,
            MOVE_REST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0510
    {
        .species = SPECIES_LIEPARD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_THUNDER_WAVE,
            MOVE_ENCORE,
            MOVE_FOUL_PLAY,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0512
    {
        .species = SPECIES_SIMISAGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_ENERGY_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0514
    {
        .species = SPECIES_SIMISEAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHARCOAL,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0516
    {
        .species = SPECIES_SIMIPOUR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0518
    {
        .species = SPECIES_MUSHARNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_MOONLIGHT,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_MUSHARNA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0521
    {
        .species = SPECIES_UNFEZANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_RETURN,
            MOVE_U_TURN,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0523
    {
        .species = SPECIES_ZEBSTRIKA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_HEAT_WAVE,
            MOVE_VOLT_SWITCH,
            MOVE_OVERHEAT
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, DEF_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0526
    {
        .species = SPECIES_GIGALITH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_GIGALITH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HARD_STONE,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ROCK,
    },

    // 0528
    {
        .species = SPECIES_SWOOBAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_STORED_POWER,
            MOVE_AIR_SLASH,
            MOVE_ROOST
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0530
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0531
    {
        .species = SPECIES_AUDINO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_TOXIC,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0534
    {
        .species = SPECIES_CONKELDURR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_MACH_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_CONKELDURR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRAIN_PUNCH,
            MOVE_MACH_PUNCH,
            MOVE_POISON_JAB,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0537
    {
        .species = SPECIES_SEISMITOAD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_WAVE,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SEISMITOAD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_CHILLING_WATER,
            MOVE_EARTH_POWER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0538
    {
        .species = SPECIES_THROH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF,
            MOVE_REST
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0539
    {
        .species = SPECIES_SAWK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_SAWK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0542
    {
        .species = SPECIES_LEAVANNY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_LEAVANNY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0545
    {
        .species = SPECIES_SCOLIPEDE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_MEGAHORN,
            MOVE_POISON_JAB,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_SCOLIPEDE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_PROTECT,
            MOVE_POISON_JAB,
            MOVE_MEGAHORN
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0547
    {
        .species = SPECIES_WHIMSICOTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_LEECH_SEED,
            MOVE_ENCORE,
            MOVE_U_TURN
        },
        .ability = ABILITY_COTTON_DOWN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_WHIMSICOTT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HELPING_HAND,
            MOVE_MOONBLAST,
            MOVE_ENCORE
        },
        .ability = ABILITY_COTTON_DOWN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0549
    {
        .species = SPECIES_LILLIGANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_LILLIGANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0549
    {
        .species = SPECIES_LILLIGANT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_VICTORY_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_LILLIGANT_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_VICTORY_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_ICE_SPINNER
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0550
    {
        .species = SPECIES_BASCULIN_RED_STRIPED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_HEAD_SMASH,
            MOVE_DOUBLE_EDGE,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BASCULIN_RED_STRIPED,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MUSCLE_BAND,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_CRUNCH,
            MOVE_FLIP_TURN,
            MOVE_PROTECT
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BASCULIN_BLUE_STRIPED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_WAVE_CRASH,
            MOVE_DOUBLE_EDGE,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_BASCULIN_BLUE_STRIPED,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_HEAD_SMASH,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0553
    {
        .species = SPECIES_KROOKODILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_KROOKODILE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_TAUNT
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0555
    {
        .species = SPECIES_DARMANITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_DARMANITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EARTHQUAKE,
            MOVE_SUPERPOWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_DARMANITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SHELL_BELL,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0555
    {
        .species = SPECIES_DARMANITAN_GALAR_STANDARD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_FLARE_BLITZ,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_GORILLA_TACTICS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_DARMANITAN_GALAR_STANDARD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_FLARE_BLITZ,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_GORILLA_TACTICS,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0556
    {
        .species = SPECIES_MARACTUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_GROWTH,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0558
    {
        .species = SPECIES_CRUSTLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_STONE_EDGE,
            MOVE_X_SCISSOR,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0560
    {
        .species = SPECIES_SCRAFTY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SCRAFTY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0561
    {
        .species = SPECIES_SIGILYPH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_COSMIC_POWER,
            MOVE_STORED_POWER,
            MOVE_ROOST,
            MOVE_PSYCHO_SHIFT
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_SIGILYPH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_PSYCHIC,
            MOVE_HEAT_WAVE,
            MOVE_ROOST
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0563
    {
        .species = SPECIES_COFAGRIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_WILL_O_WISP,
            MOVE_TOXIC_SPIKES,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_COFAGRIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_GHOST,
    },

    // 0565
    {
        .species = SPECIES_CARRACOSTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0567
    {
        .species = SPECIES_ARCHEOPS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLYING_GEM,
        .moves =
        {
            MOVE_ACROBATICS,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_DEFEATIST,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_ARCHEOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_ACROBATICS,
            MOVE_TAUNT
        },
        .ability = ABILITY_DEFEATIST,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0569
    {
        .species = SPECIES_GARBODOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_TOXIC_SPIKES,
            MOVE_SPIKES,
            MOVE_GUNK_SHOT,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_GARBODOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_SEED_BOMB,
            MOVE_DRAIN_PUNCH,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0571
    {
        .species = SPECIES_ZOROARK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_ILLUSION,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_ZOROARK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER,
            MOVE_FOCUS_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_ILLUSION,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0571
    {
        .species = SPECIES_ZOROARK_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_HYPER_VOICE,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_ILLUSION,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_ZOROARK_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_HYPER_VOICE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_ILLUSION,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0573
    {
        .species = SPECIES_CINCCINO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TAIL_SLAP,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_CINCCINO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_TAIL_SLAP,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0576
    {
        .species = SPECIES_GOTHITELLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_SHADOW_BALL,
            MOVE_REST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0579
    {
        .species = SPECIES_REUNICLUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_FOCUS_BLAST,
            MOVE_RECOVER
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_REUNICLUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0581
    {
        .species = SPECIES_SWANNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0584
    {
        .species = SPECIES_VANILLUXE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_AUTOTOMIZE,
            MOVE_BLIZZARD,
            MOVE_FREEZE_DRY,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0586
    {
        .species = SPECIES_SAWSBUCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HORN_LEECH,
            MOVE_DOUBLE_EDGE,
            MOVE_JUMP_KICK
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SAWSBUCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HORN_LEECH,
            MOVE_DOUBLE_EDGE,
            MOVE_MEGAHORN,
            MOVE_JUMP_KICK
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0587
    {
        .species = SPECIES_EMOLGA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_NUZZLE,
            MOVE_VOLT_SWITCH,
            MOVE_AIR_SLASH,
            MOVE_TAILWIND
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0589
    {
        .species = SPECIES_ESCAVALIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_IRON_HEAD,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_ESCAVALIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_IRON_HEAD,
            MOVE_DRAIN_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0591
    {
        .species = SPECIES_AMOONGUSS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_SPORE,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_AMOONGUSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SPORE,
            MOVE_GIGA_DRAIN,
            MOVE_CLEAR_SMOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0593
    {
        .species = SPECIES_JELLICENT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_WILL_O_WISP,
            MOVE_RECOVER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_JELLICENT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_TAUNT
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0594
    {
        .species = SPECIES_ALOMOMOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_CHILLING_WATER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0596
    {
        .species = SPECIES_GALVANTULA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_THUNDER,
            MOVE_BUG_BUZZ,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_GALVANTULA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDER,
            MOVE_BUG_BUZZ,
            MOVE_ENERGY_BALL,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0598
    {
        .species = SPECIES_FERROTHORN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_LEECH_SEED,
            MOVE_POWER_WHIP
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 88,
            .spd = 168
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_FERROTHORN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GRASS,
    },

    // 0601
    {
        .species = SPECIES_KLINKLANG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHIFT_GEAR,
            MOVE_GEAR_GRIND,
            MOVE_SUBSTITUTE,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0604
    {
        .species = SPECIES_EELEKTROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_EELEKTROSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_COIL,
            MOVE_WILD_CHARGE,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0606
    {
        .species = SPECIES_BEHEEYEM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0609
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL,
            MOVE_TRICK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_FIRE_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HEAT_WAVE,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_FIRE,
    },

    // 0612
    {
        .species = SPECIES_HAXORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_HAXORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_CLOSE_COMBAT,
            MOVE_FIRST_IMPRESSION
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0614
    {
        .species = SPECIES_BEARTIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0615
    {
        .species = SPECIES_CRYOGONAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_RAPID_SPIN,
            MOVE_RECOVER,
            MOVE_TOXIC
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0617
    {
        .species = SPECIES_ACCELGOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_FOCUS_BLAST,
            MOVE_ENERGY_BALL,
            MOVE_SPIKES
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
        .ball = BALL_NET,
    },
    {
        .species = SPECIES_ACCELGOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_YAWN,
            MOVE_ENCORE,
            MOVE_BUG_BUZZ,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_NET,
    },

    // 0618
    {
        .species = SPECIES_STUNFISK_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SNAP_TRAP,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_MIMICRY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_STUNFISK_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_YAWN,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_MIMICRY,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0618
    {
        .species = SPECIES_STUNFISK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_DISCHARGE,
            MOVE_EARTH_POWER,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0620
    {
        .species = SPECIES_MIENSHAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_MIENSHAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_U_TURN
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0621
    {
        .species = SPECIES_DRUDDIGON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_DRAGON_CLAW,
            MOVE_GLARE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_DRUDDIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_FIRE_PUNCH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0623
    {
        .species = SPECIES_GOLURK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_POLTERGEIST,
            MOVE_DYNAMIC_PUNCH,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GOLURK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_POLTERGEIST,
            MOVE_TOXIC
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0626
    {
        .species = SPECIES_BOUFFALANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAD_CHARGE,
            MOVE_EARTHQUAKE,
            MOVE_SUPERPOWER,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0628
    {
        .species = SPECIES_BRAVIARY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_BRAVIARY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0628
    {
        .species = SPECIES_BRAVIARY_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_HEAT_WAVE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_BRAVIARY_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0630
    {
        .species = SPECIES_MANDIBUZZ,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MANDIBUZZ,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_ROOST,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },

    // 0631
    {
        .species = SPECIES_HEATMOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_GIGA_DRAIN,
            MOVE_SUCKER_PUNCH,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0632
    {
        .species = SPECIES_DURANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_IRON_HEAD,
            MOVE_X_SCISSOR,
            MOVE_STONE_EDGE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_DURANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HONE_CLAWS,
            MOVE_IRON_HEAD,
            MOVE_X_SCISSOR,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0635
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_FIRE_BLAST,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_DARK_PULSE,
            MOVE_DEFOG,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 160,
            .spe = 96
        ),
        .teraType = TYPE_STEEL,
    },

    // 0637
    {
        .species = SPECIES_VOLCARONA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_FLAMETHROWER,
            MOVE_BUG_BUZZ,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_VOLCARONA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_FIRE_BLAST,
            MOVE_GIGA_DRAIN,
            MOVE_ROOST
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 156,
            .spe = 100
        ),
        .teraType = TYPE_FIRE,
    },

    // 0638
    {
        .species = SPECIES_COBALION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_COBALION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_IRON_HEAD,
            MOVE_STONE_EDGE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0639
    {
        .species = SPECIES_TERRAKION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_TERRAKION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0640
    {
        .species = SPECIES_VIRIZION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_VIRIZION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_FOCUS_BLAST,
            MOVE_AIR_SLASH,
            MOVE_VACUUM_WAVE
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0641
    {
        .species = SPECIES_TORNADUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_TORNADUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HURRICANE,
            MOVE_TAUNT,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0641
    {
        .species = SPECIES_TORNADUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0642
    {
        .species = SPECIES_THUNDURUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_KNOCK_OFF,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_THUNDURUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_THUNDER_WAVE,
            MOVE_THUNDERBOLT,
            MOVE_TAUNT,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0642
    {
        .species = SPECIES_THUNDURUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0643
    {
        .species = SPECIES_RESHIRAM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_BLUE_FLARE,
            MOVE_DRACO_METEOR,
            MOVE_FLAMETHROWER,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_RESHIRAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BLUE_FLARE,
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 200,
            .spd = 56
        ),
        .teraType = TYPE_FIRE,
    },

    // 0644
    {
        .species = SPECIES_ZEKROM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_BOLT_STRIKE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ZEKROM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BOLT_STRIKE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0645
    {
        .species = SPECIES_LANDORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0645
    {
        .species = SPECIES_LANDORUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_U_TURN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_LANDORUS_THERIAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 216,
            .spd = 40
        ),
        .teraType = TYPE_GROUND,
    },

    // 0646
    {
        .species = SPECIES_KYUREM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_DRACO_METEOR,
            MOVE_FREEZE_DRY,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_KYUREM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_ROOST,
            MOVE_ICE_BEAM,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 200,
            .spe = 56
        ),
        .teraType = TYPE_ICE,
    },

    // 0646
    {
        .species = SPECIES_KYUREM_BLACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_FUSION_BOLT,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_KYUREM_BLACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_FUSION_BOLT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0646
    {
        .species = SPECIES_KYUREM_WHITE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_FUSION_FLARE,
            MOVE_DRACO_METEOR,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0647
    {
        .species = SPECIES_KELDEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SECRET_SWORD,
            MOVE_VACUUM_WAVE,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_KELDEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SECRET_SWORD,
            MOVE_TERA_BLAST,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0648
    {
        .species = SPECIES_MELOETTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_HYPER_VOICE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_MELOETTA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0649
    {
        .species = SPECIES_GENESECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_U_TURN,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_GENESECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TECHNO_BLAST,
            MOVE_FLASH_CANNON,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // ====================================
    // Generation VI
    // ====================================

    // 0652
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_SPIKY_SHIELD,
            MOVE_LEECH_SEED,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_WOOD_HAMMER,
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_WOOD_HAMMER,
            MOVE_CLOSE_COMBAT,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0655
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_PSYCHIC,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_CALM_MIND,
            MOVE_FIRE_BLAST,
            MOVE_PSYSHOCK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0658
    {
        .species = SPECIES_GRENINJA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DARK_PULSE,
            MOVE_ICE_BEAM,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GRENINJA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_TOXIC_SPIKES,
            MOVE_HYDRO_PUMP,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GRENINJA_BATTLE_BOND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DARK_PULSE,
            MOVE_ICE_BEAM,
            MOVE_U_TURN
        },
        .ability = ABILITY_BATTLE_BOND,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0660
    {
        .species = SPECIES_DIGGERSBY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_RETURN,
            MOVE_QUICK_ATTACK,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DIGGERSBY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_RETURN,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0663
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_FLARE_BLITZ,
            MOVE_SWORDS_DANCE,
            MOVE_U_TURN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 240,
            .spd = 20
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHARP_BEAK,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_BRAVE_BIRD,
            MOVE_FLARE_BLITZ,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0666
    {
        .species = SPECIES_VIVILLON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WISE_GLASSES,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_HURRICANE,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_VIVILLON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_TAILWIND,
            MOVE_BUG_BUZZ,
            MOVE_POWDER
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0668
    {
        .species = SPECIES_PYROAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_HYPER_VOICE,
            MOVE_DARK_PULSE,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_PYROAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_HEAT_WAVE,
            MOVE_SNARL,
            MOVE_PROTECT
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0671
    {
        .species = SPECIES_FLORGES,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_SYNTHESIS,
            MOVE_AROMATHERAPY
        },
        .ability = ABILITY_SYMBIOSIS,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_FLORGES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_MOONBLAST,
            MOVE_PSYCHIC,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_SYMBIOSIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0673
    {
        .species = SPECIES_GOGOAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_HORN_LEECH,
            MOVE_EARTHQUAKE,
            MOVE_MILK_DRINK
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_GOGOAT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HORN_LEECH,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_BULK_UP
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0675
    {
        .species = SPECIES_PANGORO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT,
            MOVE_GUNK_SHOT,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_PANGORO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0676
    {
        .species = SPECIES_FURFROU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHESTO_BERRY,
        .moves =
        {
            MOVE_COTTON_GUARD,
            MOVE_BODY_PRESS,
            MOVE_CRUNCH,
            MOVE_REST
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_FURFROU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_HELPING_HAND,
            MOVE_BABY_DOLL_EYES,
            MOVE_BODY_SLAM
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0678
    {
        .species = SPECIES_MEOWSTIC,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_FAKE_OUT,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_MEOWSTIC,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_THUNDER_WAVE,
            MOVE_HELPING_HAND,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0678
    {
        .species = SPECIES_MEOWSTIC_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_MEOWSTIC_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0681
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_KINGS_SHIELD,
            MOVE_IRON_HEAD,
            MOVE_SHADOW_SNEAK,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_KINGS_SHIELD,
            MOVE_SHADOW_BALL,
            MOVE_FLASH_CANNON,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SPECTRAL_THIEF,
            MOVE_IRON_HEAD,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0683
    {
        .species = SPECIES_AROMATISSE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_MOONBLAST,
            MOVE_AROMATHERAPY,
            MOVE_WISH
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
        .iv = IVS(SPE, 0),
    },

    // 0685
    {
        .species = SPECIES_SLURPUFF,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_PLAY_ROUGH,
            MOVE_DRAIN_PUNCH,
            MOVE_FACADE
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SLURPUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DAZZLING_GLEAM,
            MOVE_FLAMETHROWER,
            MOVE_DRAINING_KISS
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0687
    {
        .species = SPECIES_MALAMAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SUPERPOWER,
            MOVE_KNOCK_OFF,
            MOVE_PSYCHO_CUT,
            MOVE_REST
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MALAMAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SUPERPOWER,
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0689
    {
        .species = SPECIES_BARBARACLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_CROSS_CHOP
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BARBARACLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0691
    {
        .species = SPECIES_DRAGALGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SLUDGE_WAVE,
            MOVE_FLIP_TURN,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_DRAGALGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TOXIC_SPIKES,
            MOVE_SLUDGE_BOMB,
            MOVE_DRAGON_PULSE,
            MOVE_TOXIC
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0693
    {
        .species = SPECIES_CLAWITZER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_WATER_PULSE,
            MOVE_AURA_SPHERE,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_CLAWITZER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DARK_PULSE,
            MOVE_ICE_BEAM,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0695
    {
        .species = SPECIES_HELIOLISK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_HYPER_VOICE,
            MOVE_VOLT_SWITCH,
            MOVE_SURF
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_HELIOLISK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_HYPER_VOICE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0697
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_OUTRAGE,
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_CRUNCH,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0699
    {
        .species = SPECIES_AURORUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_FREEZE_DRY,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_REFRIGERATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_AURORUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_FREEZE_DRY,
            MOVE_HYPER_VOICE,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0700
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_PSYSHOCK,
            MOVE_MYSTICAL_FIRE,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HYPER_VOICE,
            MOVE_WISH,
            MOVE_PROTECT
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_MYSTICAL_FIRE,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0701
    {
        .species = SPECIES_HAWLUCHA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_QUICK_CLAW,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ACROBATICS,
            MOVE_CLOSE_COMBAT,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_HAWLUCHA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_ACROBATICS,
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_PROTECT
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0702
    {
        .species = SPECIES_DEDENNE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_NUZZLE,
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_PLUS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0703
    {
        .species = SPECIES_CARBINK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_CARBINK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
        .iv = IVS(SPE, 0),
    },

    // 0706
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .spa = 252,
            .spd = 8
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_CHILLING_WATER,
            MOVE_TOXIC,
            MOVE_REST
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SLUDGE_WAVE,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0706
    {
        .species = SPECIES_GOODRA_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_FIRE_BLAST,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_GOODRA_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_DRACO_METEOR,
            MOVE_RECOVER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0707
    {
        .species = SPECIES_KLEFKI,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_THUNDER_WAVE,
            MOVE_SPIKES
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_KLEFKI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_THUNDER_WAVE,
            MOVE_FOUL_PLAY,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0709
    {
        .species = SPECIES_TREVENANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HORN_LEECH,
            MOVE_POLTERGEIST,
            MOVE_WILL_O_WISP,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_TREVENANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
        .iv = IVS(SPE, 0),
    },

    // 0711
    {
        .species = SPECIES_GOURGEIST_SUPER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_POLTERGEIST,
            MOVE_LEECH_SEED,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GOURGEIST_SUPER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_SEED_BOMB,
            MOVE_TRICK,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GHOST,
        .iv = IVS(SPE, 0),
    },

    // 0713
    {
        .species = SPECIES_AVALUGG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_RECOVER,
            MOVE_AVALANCHE,
            MOVE_BODY_PRESS,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_AVALUGG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_AVALANCHE,
            MOVE_BODY_PRESS,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
        .iv = IVS(SPE, 0),
    },

    // 0713
    {
        .species = SPECIES_AVALUGG_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_RECOVER,
            MOVE_ICICLE_CRASH,
            MOVE_BODY_PRESS,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_AVALUGG_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICE_FANG,
            MOVE_CRUNCH,
            MOVE_STONE_EDGE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0715
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_HURRICANE,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_U_TURN
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HURRICANE,
            MOVE_DRACO_METEOR,
            MOVE_HEAT_WAVE
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0716
    {
        .species = SPECIES_XERNEAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_POWER_HERB,
        .moves =
        {
            MOVE_GEOMANCY,
            MOVE_MOONBLAST,
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_FAIRY_AURA,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_XERNEAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_HORN_LEECH
        },
        .ability = ABILITY_FAIRY_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0717
    {
        .species = SPECIES_YVELTAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_HURRICANE,
            MOVE_FOCUS_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_YVELTAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 200,
            .spd = 56
        ),
        .teraType = TYPE_DARK,
    },

    // 0718
    {
        .species = SPECIES_ZYGARDE_50_POWER_CONSTRUCT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_THOUSAND_ARROWS,
            MOVE_OUTRAGE,
            MOVE_COIL
        },
        .ability = ABILITY_POWER_CONSTRUCT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_ZYGARDE_10_POWER_CONSTRUCT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_THOUSAND_ARROWS,
            MOVE_OUTRAGE,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_POWER_CONSTRUCT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spe = 252,
            .hp = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0719
    {
        .species = SPECIES_DIANCIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DIAMOND_STORM,
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_PROTECT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_DIANCIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_DIAMOND_STORM,
            MOVE_MOONBLAST,
            MOVE_REFLECT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ROCK,
    },

    // 0720
    {
        .species = SPECIES_HOOPA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0720
    {
        .species = SPECIES_HOOPA_UNBOUND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_HYPERSPACE_FURY,
            MOVE_PSYCHIC,
            MOVE_GUNK_SHOT,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_HOOPA_UNBOUND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DARK_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
        .iv = IVS(SPE, 0),
    },

    // 0721
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_STEAM_ERUPTION,
            MOVE_FLAMETHROWER,
            MOVE_SLUDGE_WAVE,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_STEAM_ERUPTION,
            MOVE_FLAMETHROWER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HEAT_WAVE,
            MOVE_STEAM_ERUPTION,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_WAVE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // ====================================
    // Generation VII
    // ====================================

    // 0724
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SPELL_TAG,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SPIRIT_SHACKLE,
            MOVE_LEAF_BLADE,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SPIRIT_SHACKLE,
            MOVE_LEAF_BLADE,
            MOVE_DEFOG,
            MOVE_ROOST
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_BLADE,
            MOVE_SPIRIT_SHACKLE,
            MOVE_SUCKER_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0724
    {
        .species = SPECIES_DECIDUEYE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_TRIPLE_ARROWS,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_DECIDUEYE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_TRIPLE_ARROWS,
            MOVE_LEAF_BLADE,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .atk = 16,
            .spd = 244
        ),
        .teraType = TYPE_GRASS,
    },

    // 0727
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_FLARE_BLITZ,
            MOVE_DARKEST_LARIAT,
            MOVE_PARTING_SHOT
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_DARKEST_LARIAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_FLARE_BLITZ,
            MOVE_WILL_O_WISP,
            MOVE_U_TURN
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0730
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SPARKLING_ARIA,
            MOVE_MOONBLAST,
            MOVE_PSYCHIC,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_LIQUID_VOICE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SPARKLING_ARIA,
            MOVE_MOONBLAST,
            MOVE_REST
        },
        .ability = ABILITY_LIQUID_VOICE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_MOONBLAST,
            MOVE_ENERGY_BALL,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_LIQUID_VOICE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0733
    {
        .species = SPECIES_TOUCANNON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BEAK_BLAST,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_TOUCANNON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_BRAVE_BIRD,
            MOVE_BEAK_BLAST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0735
    {
        .species = SPECIES_GUMSHOOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0738
    {
        .species = SPECIES_VIKAVOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_VIKAVOLT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .iv = IVS(ATK, 0, SPE, 0),
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_BUG,
    },

    // 0740
    {
        .species = SPECIES_CRABOMINABLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_ICE_HAMMER
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CRABOMINABLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .iv = IVS(SPE, 0),
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0741
    {
        .species = SPECIES_ORICORIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_REVELATION_DANCE,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ORICORIO_PAU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_REVELATION_DANCE,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0743
    {
        .species = SPECIES_RIBOMBEE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_MOONBLAST,
            MOVE_BUG_BUZZ,
            MOVE_STUN_SPORE
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_RIBOMBEE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_MOONBLAST,
            MOVE_BUG_BUZZ,
            MOVE_POLLEN_PUFF
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0745
    {
        .species = SPECIES_LYCANROC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_ACCELEROCK,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_LYCANROC_DUSK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE,
            MOVE_ACCELEROCK,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_LYCANROC_MIDNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_ACCELEROCK,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0746
    {
        .species = SPECIES_WISHIWASHI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SCHOOLING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_WISHIWASHI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_TOXIC,
            MOVE_PROTECT
        },
        .ability = ABILITY_SCHOOLING,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0748
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_BANEFUL_BUNKER,
            MOVE_TOXIC_SPIKES,
            MOVE_RECOVER,
            MOVE_CHILLING_WATER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_BANEFUL_BUNKER,
            MOVE_TOXIC,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0750
    {
        .species = SPECIES_MUDSDALE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HIGH_HORSEPOWER,
            MOVE_BODY_PRESS,
            MOVE_STEALTH_ROCK,
            MOVE_ROAR
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_MUDSDALE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_HIGH_HORSEPOWER,
            MOVE_HEAVY_SLAM,
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0752
    {
        .species = SPECIES_ARAQUANID,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_LEECH_LIFE,
            MOVE_MIRROR_COAT,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_ARAQUANID,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .iv = IVS(SPE, 0),
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_LEECH_LIFE,
            MOVE_ICE_PUNCH,
            MOVE_BUG_BITE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0754
    {
        .species = SPECIES_LURANTIS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_SUPERPOWER,
            MOVE_HIDDEN_POWER,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_LURANTIS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEAF_BLADE,
            MOVE_LEECH_SEED,
            MOVE_SYNTHESIS,
            MOVE_TOXIC
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0756
    {
        .species = SPECIES_SHIINOTIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SPORE,
            MOVE_STRENGTH_SAP,
            MOVE_MOONBLAST,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_MYCELIUM_MIGHT,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0758
    {
        .species = SPECIES_SALAZZLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_FIRE_BLAST,
            MOVE_PROTECT,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_SALAZZLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_SLUDGE_WAVE,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0760
    {
        .species = SPECIES_BEWEAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_BEWEAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0763
    {
        .species = SPECIES_TSAREENA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_HIGH_JUMP_KICK,
            MOVE_PLAY_ROUGH,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_TSAREENA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_RAPID_SPIN,
            MOVE_SYNTHESIS,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0764
    {
        .species = SPECIES_COMFEY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FLORAL_HEALING,
            MOVE_DRAINING_KISS,
            MOVE_GIGA_DRAIN,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_COMFEY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DRAINING_KISS,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0765
    {
        .species = SPECIES_ORANGURU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .iv = IVS(ATK, 0, SPE, 0),
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_INSTRUCT,
            MOVE_PSYCHIC,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_SYMBIOSIS,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_ORANGURU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SYMBIOSIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0766
    {
        .species = SPECIES_PASSIMIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_PASSIMIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_RIVALRY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0768
    {
        .species = SPECIES_GOLISOPOD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FIRST_IMPRESSION,
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_EMERGENCY_EXIT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_GOLISOPOD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FIRST_IMPRESSION,
            MOVE_LIQUIDATION,
            MOVE_SPIKES,
            MOVE_LEECH_LIFE
        },
        .ability = ABILITY_EMERGENCY_EXIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0770
    {
        .species = SPECIES_PALOSSAND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_TOXIC,
            MOVE_SHORE_UP
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_PALOSSAND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_GIGA_DRAIN,
            MOVE_SHORE_UP
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0771
    {
        .species = SPECIES_PYUKUMUKU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_COUNTER,
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_SOAK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0773
    {
        .species = SPECIES_SILVALLY_DRAGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DRAGON_MEMORY,
        .moves =
        {
            MOVE_MULTI_ATTACK,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_SILVALLY_FAIRY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FAIRY_MEMORY,
        .moves =
        {
            MOVE_MULTI_ATTACK,
            MOVE_PARTING_SHOT,
            MOVE_DEFOG,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SILVALLY_STEEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_STEEL_MEMORY,
        .moves =
        {
            MOVE_MULTI_ATTACK,
            MOVE_FLAMETHROWER,
            MOVE_ICE_BEAM,
            MOVE_U_TURN
        },
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0774
    {
        .species = SPECIES_MINIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_ACROBATICS,
            MOVE_POWER_GEM,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SHIELDS_DOWN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_MINIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_POWER_GEM,
            MOVE_DAZZLING_GLEAM,
            MOVE_ACROBATICS
        },
        .ability = ABILITY_SHIELDS_DOWN,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 128,
            .spa = 128,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0775
    {
        .species = SPECIES_KOMALA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_RETURN,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0776
    {
        .species = SPECIES_TURTONATOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHELL_TRAP,
            MOVE_FLAMETHROWER,
            MOVE_DRAGON_PULSE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TURTONATOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_DRACO_METEOR,
            MOVE_EARTH_POWER,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0777
    {
        .species = SPECIES_TOGEDEMARU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_ZING_ZAP,
            MOVE_IRON_HEAD,
            MOVE_U_TURN,
            MOVE_NUZZLE
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_TOGEDEMARU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_ZING_ZAP,
            MOVE_SPIKY_SHIELD,
            MOVE_ENCORE
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0778
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_PLAY_ROUGH,
            MOVE_SHADOW_CLAW,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_DISGUISE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_PLAY_ROUGH,
            MOVE_SHADOW_SNEAK,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_DISGUISE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_SHADOW_SNEAK,
            MOVE_WILL_O_WISP,
            MOVE_TAUNT
        },
        .ability = ABILITY_DISGUISE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0779
    {
        .species = SPECIES_BRUXISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_LIQUIDATION,
            MOVE_CRUNCH,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BRUXISH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_LIQUIDATION,
            MOVE_FLIP_TURN,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0780
    {
        .species = SPECIES_DRAMPA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_HYPER_VOICE,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_DRAMPA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_FLAMETHROWER,
            MOVE_ROOST,
            MOVE_GLARE
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0781
    {
        .species = SPECIES_DHELMISE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ANCHOR_SHOT,
            MOVE_POWER_WHIP,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 248,
            .atk = 252,
            .spd = 8
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_DHELMISE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .iv = IVS(SPE, 0),
        .moves =
        {
            MOVE_ANCHOR_SHOT,
            MOVE_POWER_WHIP,
            MOVE_SHADOW_CLAW,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0784
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DRAGON_GEM,
        .moves =
        {
            MOVE_CLANGING_SCALES,
            MOVE_CLOSE_COMBAT,
            MOVE_FLAMETHROWER,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_BODY_PRESS,
            MOVE_DRAIN_PUNCH,
            MOVE_IRON_DEFENSE
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_SCALE_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0785
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_VOLT_SWITCH,
            MOVE_NATURES_MADNESS
        },
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_VOLT_SWITCH,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DAZZLING_GLEAM,
            MOVE_U_TURN,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0786
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_MOONBLAST,
            MOVE_PSYSHOCK,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PSYCHIC_SEED,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_MOONBLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_PSYCHIC_GEM,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_MOONBLAST,
            MOVE_PSYSHOCK,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0787
    {
        .species = SPECIES_TAPU_BULU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_WOOD_HAMMER,
            MOVE_HORN_LEECH,
            MOVE_PLAY_ROUGH,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_TAPU_BULU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRASSY_SEED,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HORN_LEECH,
            MOVE_PLAY_ROUGH,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0788
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SURF,
            MOVE_MOONBLAST,
            MOVE_TAUNT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_MOONBLAST,
            MOVE_ICE_BEAM,
            MOVE_SURF
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MISTY_SEED,
        .moves =
        {
            MOVE_MUDDY_WATER,
            MOVE_MOONBLAST,
            MOVE_HAZE,
            MOVE_PROTECT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0791
    {
        .species = SPECIES_SOLGALEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SUNSTEEL_STRIKE,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_SOLGALEO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SUNSTEEL_STRIKE,
            MOVE_MORNING_SUN,
            MOVE_CALM_MIND,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_DROUGHT,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0792
    {
        .species = SPECIES_LUNALA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GHOST_GEM,
        .moves =
        {
            MOVE_MOONGEIST_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_MOONBLAST,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_LUNALA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONGEIST_BEAM,
            MOVE_PSYSHOCK,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0793
    {
        .species = SPECIES_NIHILEGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_POWER_GEM,
            MOVE_THUNDERBOLT,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_NIHILEGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_POWER_GEM,
            MOVE_TOXIC_SPIKES,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 252,
            .spe = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0794
    {
        .species = SPECIES_BUZZWOLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_LEECH_LIFE,
            MOVE_ICE_PUNCH,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_BUZZWOLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_LEECH_LIFE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0795
    {
        .species = SPECIES_PHEROMOSA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_BUG_BUZZ,
            MOVE_U_TURN
        },
        .ability = ABILITY_LINGERING_AROMA,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_PHEROMOSA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_LINGERING_AROMA,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0796
    {
        .species = SPECIES_XURKITREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM,
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_XURKITREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_DAZZLING_GLEAM,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0797
    {
        .species = SPECIES_CELESTEELA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_PROTECT,
            MOVE_FLAMETHROWER,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CELESTEELA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_AUTOTOMIZE,
            MOVE_HEAVY_SLAM,
            MOVE_FLAMETHROWER,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0798
    {
        .species = SPECIES_KARTANA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_LEAF_BLADE,
            MOVE_SACRED_SWORD,
            MOVE_SMART_STRIKE,
            MOVE_AERIAL_ACE
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0799
    {
        .species = SPECIES_GUZZLORD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_DRAGON_TAIL,
            MOVE_HEAVY_SLAM,
            MOVE_REST
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_GUZZLORD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0800
    {
        .species = SPECIES_NECROZMA_DUSK_MANE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SUNSTEEL_STRIKE,
            MOVE_EARTHQUAKE,
            MOVE_PHOTON_GEYSER
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_NECROZMA_DAWN_WINGS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONGEIST_BEAM,
            MOVE_PHOTON_GEYSER,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_NECROZMA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PSYCHIC_GEM,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PHOTON_GEYSER,
            MOVE_HEAT_WAVE,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0801
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FAIRY_GEM,
        .iv = IVS(ATK, 0, SPE, 0),
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0802
    {
        .species = SPECIES_MARSHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GHOST_GEM,
        .moves =
        {
            MOVE_SPECTRAL_THIEF,
            MOVE_CLOSE_COMBAT,
            MOVE_SHADOW_SNEAK,
            MOVE_BULK_UP
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_MARSHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_SPECTRAL_THIEF,
            MOVE_MACH_PUNCH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_TRACE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0804
    {
        .species = SPECIES_NAGANADEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SLUDGE_WAVE,
            MOVE_FIRE_BLAST,
            MOVE_DRACO_METEOR
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_NAGANADEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0805
    {
        .species = SPECIES_STAKATAKA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .iv = IVS(SPE, 0),
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_ROCK_SLIDE,
            MOVE_EARTHQUAKE,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_STAKATAKA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_GYRO_BALL,
            MOVE_BODY_PRESS,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0806
    {
        .species = SPECIES_BLACEPHALON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_FIRE_BLAST,
            MOVE_PSYCHIC,
            MOVE_TRICK
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_BLACEPHALON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FIRE_GEM,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_FIRE_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0807
    {
        .species = SPECIES_ZERAORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PLASMA_FISTS,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_PLAY_ROUGH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ZERAORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_PLASMA_FISTS,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0809
    {
        .species = SPECIES_MELMETAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_DOUBLE_IRON_BASH,
            MOVE_THUNDER_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_MELMETAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DOUBLE_IRON_BASH,
            MOVE_THUNDER_PUNCH,
            MOVE_SUPERPOWER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // ====================================
    // Generation VIII
    // ====================================

    // 0812
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_GRASSY_GLIDE,
            MOVE_WOOD_HAMMER,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_GRASSY_GLIDE,
            MOVE_HIGH_HORSEPOWER,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_GRASSY_GLIDE,
            MOVE_KNOCK_OFF,
            MOVE_SYNTHESIS,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 200,
            .spd = 56
        ),
        .teraType = TYPE_GRASS,
    },

    // 0815
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_PYRO_BALL,
            MOVE_HIGH_HORSEPOWER,
            MOVE_U_TURN,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_LIBERO,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_PYRO_BALL,
            MOVE_HIGH_HORSEPOWER,
            MOVE_ZEN_HEADBUTT,
            MOVE_U_TURN
        },
        .ability = ABILITY_LIBERO,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_PYRO_BALL,
            MOVE_COURT_CHANGE,
            MOVE_HIGH_HORSEPOWER,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_LIBERO,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0818
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_DARK_PULSE,
            MOVE_U_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS,
        .moves =
        {
            MOVE_SNIPE_SHOT,
            MOVE_ICE_BEAM,
            MOVE_DARK_PULSE,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_U_TURN,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0820
    {
        .species = SPECIES_GREEDENT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_SWORDS_DANCE,
            MOVE_BULLET_SEED
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0823
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BODY_PRESS,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_IRON_DEFENSE
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 168,
            .spd = 88
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_BODY_PRESS,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_BULK_UP,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0826
    {
        .species = SPECIES_ORBEETLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_PSYCHIC,
            MOVE_STICKY_WEB
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_NET,
    },
    {
        .species = SPECIES_ORBEETLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_BUG_BUZZ,
            MOVE_ROOST
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_BUG,
        .ball = BALL_NET,
    },

    // 0828
    {
        .species = SPECIES_THIEVUL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_PARTING_SHOT,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_RUN_AWAY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0830
    {
        .species = SPECIES_ELDEGOSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_COTTON_DOWN,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_GRASS,
    },

    // 0832
    {
        .species = SPECIES_DUBWOOL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_COTTON_GUARD,
            MOVE_BODY_PRESS,
            MOVE_BODY_SLAM,
            MOVE_ROOST
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0834
    {
        .species = SPECIES_DREDNAW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUM_BERRY,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_DREDNAW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HARD_STONE,
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
        .ball = BALL_DIVE,
    },

    // 0836
    {
        .species = SPECIES_BOLTUND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MAGNET,
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0839
    {
        .species = SPECIES_COALOSSAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_FLAMETHROWER,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_COALOSSAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_FLAMETHROWER,
            MOVE_STONE_EDGE,
            MOVE_EARTH_POWER,
            MOVE_HEAT_CRASH
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0841
    {
        .species = SPECIES_FLAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_GRAV_APPLE,
            MOVE_DRAGON_RUSH,
            MOVE_U_TURN,
            MOVE_OUTRAGE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_FLAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_APPLE_ACID,
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0842
    {
        .species = SPECIES_APPLETUN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_APPLE_ACID,
            MOVE_DRAGON_PULSE,
            MOVE_RECOVER,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 8,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_APPLETUN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_APPLE_ACID,
            MOVE_DRACO_METEOR,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0844
    {
        .species = SPECIES_SANDACONDA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_COIL,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_GLARE
        },
        .ability = ABILITY_SAND_SPIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_SANDACONDA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_GLARE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_SPIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0845
    {
        .species = SPECIES_CRAMORANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SURF,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_DEFOG
        },
        .ability = ABILITY_GULP_MISSILE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },

    // 0847
    {
        .species = SPECIES_BARRASKEWDA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHIC_FANGS,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BARRASKEWDA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0849
    {
        .species = SPECIES_TOXTRICITY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_TOXTRICITY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_TOXTRICITY_LOW_KEY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH,
            MOVE_BOOMBURST
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0851
    {
        .species = SPECIES_CENTISKORCH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FIERY_DANCE,
            MOVE_OVERHEAT,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CENTISKORCH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_COIL,
            MOVE_FLARE_BLITZ,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0853
    {
        .species = SPECIES_GRAPPLOCT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_OCTOLOCK,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_BULK_UP
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_DIVE,
    },
    {
        .species = SPECIES_GRAPPLOCT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_MACH_PUNCH,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 160,
            .atk = 252,
            .spd = 96
        ),
        .teraType = TYPE_ICE,
    },

    // 0855
    {
        .species = SPECIES_POLTEAGEIST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_STORED_POWER,
            MOVE_SHADOW_BALL,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_POLTEAGEIST,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_SHADOW_BALL,
            MOVE_GIGA_DRAIN,
            MOVE_STORED_POWER
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0858
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EXPANDING_FORCE,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_PROTECT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_POWER_WHIP
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0861
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_SPIRIT_BREAK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_SPIRIT_BREAK,
            MOVE_SUCKER_PUNCH,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_SPIRIT_BREAK,
            MOVE_THUNDER_WAVE,
            MOVE_TAUNT,
            MOVE_PARTING_SHOT
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 100,
            .spd = 156
        ),
        .teraType = TYPE_DARK,
    },

    // 0862
    {
        .species = SPECIES_OBSTAGOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT,
            MOVE_OBSTRUCT
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_OBSTAGOON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_OBSTRUCT
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0863
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_IRON_BALL,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_GYRO_BALL,
            MOVE_CLOSE_COMBAT,
            MOVE_PROTECT
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_WATER,
    },

    // 0864
    {
        .species = SPECIES_CURSOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SHADOW_BALL,
            MOVE_ICE_BEAM,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_PERISH_BODY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_CURSOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_ICE_BEAM,
            MOVE_EARTH_POWER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0865
    {
        .species = SPECIES_SIRFETCHD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEEK,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_FIRST_IMPRESSION
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_SIRFETCHD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_NIGHT_SLASH
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_SIRFETCHD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_COVERT_CLOAK,
        .moves =
        {
            MOVE_WIDE_GUARD,
            MOVE_COACHING,
            MOVE_CLOSE_COMBAT,
            MOVE_FIRST_IMPRESSION
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0866
    {
        .species = SPECIES_MR_RIME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_MR_RIME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FREEZE_DRY,
            MOVE_FOCUS_BLAST,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_SCREEN_CLEANER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0867
    {
        .species = SPECIES_RUNERIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_BODY_PRESS,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_RUNERIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_POLTERGEIST
        },
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0869
    {
        .species = SPECIES_ALCREMIE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_AGUAV_BERRY,
        .moves =
        {
            MOVE_DECORATE,
            MOVE_HYPER_VOICE,
            MOVE_RECOVER,
            MOVE_PROTECT
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_ALCREMIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HYPER_VOICE,
            MOVE_MYSTICAL_FIRE,
            MOVE_RECOVER
        },
        .ability = ABILITY_PIXILATE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0870
    {
        .species = SPECIES_FALINKS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NO_RETREAT,
            MOVE_CLOSE_COMBAT,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_FALINKS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB,
        .moves =
        {
            MOVE_NO_RETREAT,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0871
    {
        .species = SPECIES_PINCURCHIN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_RISING_VOLTAGE,
            MOVE_DISCHARGE,
            MOVE_RECOVER,
            MOVE_SPIKES
        },
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_DIVE,
    },

    // 0873
    {
        .species = SPECIES_FROSMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_ICE_BEAM,
            MOVE_BUG_BUZZ,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_FROSMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_ICE_BEAM,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 156,
            .spe = 100
        ),
        .teraType = TYPE_ICE,
    },

    // 0874
    {
        .species = SPECIES_STONJOURNER,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ROCK,
    },

    // 0875
    {
        .species = SPECIES_EISCUE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_LIQUIDATION,
            MOVE_ICICLE_CRASH,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_ICE_FACE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_EISCUE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_HEAD_SMASH,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_ICE_FACE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0876
    {
        .species = SPECIES_INDEEDEE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EXPANDING_FORCE,
            MOVE_DAZZLING_GLEAM,
            MOVE_PSYCHIC_TERRAIN,
            MOVE_PROTECT
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spd = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_INDEEDEE_F,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_HEAL_PULSE
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0877
    {
        .species = SPECIES_MORPEKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_AURA_WHEEL,
            MOVE_KNOCK_OFF,
            MOVE_PSYCHIC_FANGS,
            MOVE_PROTECT
        },
        .ability = ABILITY_HUNGER_SWITCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0879
    {
        .species = SPECIES_COPPERAJAH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_HEAVY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_PLAY_ROUGH,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_COPPERAJAH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_HEAVY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0880
    {
        .species = SPECIES_DRACOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_DRACOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0881
    {
        .species = SPECIES_ARCTOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_ICICLE_CRASH,
            MOVE_LOW_KICK,
            MOVE_BLIZZARD
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_ARCTOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_FREEZE_DRY,
            MOVE_THUNDERBOLT,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0882
    {
        .species = SPECIES_DRACOVISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_CRUNCH,
            MOVE_PSYCHIC_FANGS,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_DRACOVISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0883
    {
        .species = SPECIES_ARCTOVISH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_ICICLE_CRASH,
            MOVE_PSYCHIC_FANGS,
            MOVE_CRUNCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_ARCTOVISH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_FLIP_TURN,
            MOVE_BODY_PRESS,
            MOVE_RECOVER
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0887
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SHADOW_BALL,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DRAGON_DARTS,
            MOVE_PHANTOM_FORCE,
            MOVE_U_TURN,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_DRAGON_DARTS,
            MOVE_PHANTOM_FORCE,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0888
    {
        .species = SPECIES_ZACIAN_CROWNED,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_RUSTED_SWORD,
        .moves =
        {
            MOVE_BEHEMOTH_BLADE,
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ZACIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0889
    {
        .species = SPECIES_ZAMAZENTA_CROWNED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_RUSTED_SHIELD,
        .moves =
        {
            MOVE_BEHEMOTH_BASH,
            MOVE_BODY_PRESS,
            MOVE_IRON_DEFENSE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 144,
            .spd = 112
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_ZAMAZENTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH
        },
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0890
    {
        .species = SPECIES_ETERNATUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DYNAMAX_CANNON,
            MOVE_SLUDGE_WAVE,
            MOVE_FLAMETHROWER,
            MOVE_DRACO_METEOR
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_ETERNATUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves =
        {
            MOVE_DYNAMAX_CANNON,
            MOVE_FLAMETHROWER,
            MOVE_TOXIC_SPIKES,
            MOVE_RECOVER
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 156,
            .spe = 100
        ),
        .teraType = TYPE_POISON,
    },

    // 0892
    {
        .species = SPECIES_URSHIFU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_WICKED_BLOW,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_URSHIFU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_WICKED_BLOW,
            MOVE_CLOSE_COMBAT,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0892
    {
        .species = SPECIES_URSHIFU_RAPID_STRIKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_SURGING_STRIKES,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET,
            MOVE_U_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_URSHIFU_RAPID_STRIKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SURGING_STRIKES,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0893
    {
        .species = SPECIES_ZARUDE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_POWER_WHIP,
            MOVE_DARKEST_LARIAT,
            MOVE_JUNGLE_HEALING
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_ZARUDE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_DARKEST_LARIAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0894
    {
        .species = SPECIES_REGIELEKI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_RISING_VOLTAGE,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_REGIELEKI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY,
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_THUNDERBOLT,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0895
    {
        .species = SPECIES_REGIDRAGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_REGIDRAGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_DRAGON_PULSE,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0896
    {
        .species = SPECIES_GLASTRIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_CLOSE_COMBAT,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_GLASTRIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0897
    {
        .species = SPECIES_SPECTRIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_DARK_PULSE,
            MOVE_MYSTICAL_FIRE,
            MOVE_DRAINING_KISS
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_SPECTRIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_MYSTICAL_FIRE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0898
    {
        .species = SPECIES_CALYREX_ICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_GLACIAL_LANCE,
            MOVE_HIGH_HORSEPOWER,
            MOVE_TRICK_ROOM,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_AS_ONE_ICE_RIDER,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CALYREX_ICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_GLACIAL_LANCE,
            MOVE_HIGH_HORSEPOWER,
            MOVE_CLOSE_COMBAT,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_AS_ONE_ICE_RIDER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0898
    {
        .species = SPECIES_CALYREX_SHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_ASTRAL_BARRAGE,
            MOVE_PSYSHOCK,
            MOVE_GIGA_DRAIN,
            MOVE_DRACO_METEOR
        },
        .ability = ABILITY_AS_ONE_SHADOW_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_CALYREX_SHADOW,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_ASTRAL_BARRAGE,
            MOVE_GIGA_DRAIN,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_AS_ONE_SHADOW_RIDER,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0898
    {
        .species = SPECIES_CALYREX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_CALYREX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_POLLEN_PUFF,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0899
    {
        .species = SPECIES_WYRDEER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_MEGAHORN,
            MOVE_BODY_SLAM,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_WYRDEER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_HYPER_VOICE,
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0900
    {
        .species = SPECIES_KLEAVOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_STONE_AXE,
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_KLEAVOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STONE_AXE,
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_DEFOG
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0901
    {
        .species = SPECIES_URSALUNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_HEADLONG_RUSH,
            MOVE_CRUNCH,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_URSALUNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HEADLONG_RUSH,
            MOVE_CRUNCH,
            MOVE_PROTECT
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0901
    {
        .species = SPECIES_URSALUNA_BLOODMOON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BLOOD_MOON,
            MOVE_EARTH_POWER,
            MOVE_HYPER_VOICE,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_URSALUNA_BLOODMOON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_BLOOD_MOON,
            MOVE_EARTH_POWER,
            MOVE_HYPER_VOICE,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0902
    {
        .species = SPECIES_BASCULEGION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_PHANTOM_FORCE,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_BASCULEGION_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SHADOW_BALL,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0903
    {
        .species = SPECIES_SNEASLER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_KINGS_ROCK,
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_GUNK_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_SNEASLER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_QUICK_CLAW,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_ACROBATICS,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0904
    {
        .species = SPECIES_OVERQWIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BARB_BARRAGE,
            MOVE_KNOCK_OFF,
            MOVE_TOXIC_SPIKES,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_OVERQWIL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BARB_BARRAGE,
            MOVE_KNOCK_OFF,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0905
    {
        .species = SPECIES_ENAMORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ENAMORUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_CONTRARY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ENAMORUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_EARTHQUAKE,
            MOVE_SPRINGTIDE_STORM,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // ====================================
    // Generation IX
    // ====================================

    // 0908
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_FLOWER_TRICK,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_PLAY_ROUGH
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLOWER_TRICK,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_SPIKES,
            MOVE_FLOWER_TRICK,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_PROTEAN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0911
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_SHADOW_BALL,
            MOVE_SLACK_OFF,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_HEX,
            MOVE_SLACK_OFF,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_MUMMY,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0914
    {
        .species = SPECIES_QUAQUAVAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_AQUA_STEP,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_SPINNER,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_QUAQUAVAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_AQUA_STEP,
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_U_TURN
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0916
    {
        .species = SPECIES_OINKOLOGNE_M,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FIGY_BERRY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_PLAY_ROUGH
        },
        .ability = ABILITY_LINGERING_AROMA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_OINKOLOGNE_M,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_LINGERING_AROMA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_OINKOLOGNE_F,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MISTY_SEED,
        .moves =
        {
            MOVE_HELPING_HAND,
            MOVE_BODY_SLAM,
            MOVE_DEFOG,
            MOVE_PROTECT
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_OINKOLOGNE_F,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_CURSE,
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_GHOST,
    },

    // 0918
    {
        .species = SPECIES_SPIDOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_SPIKES,
            MOVE_KNOCK_OFF,
            MOVE_CIRCLE_THROW
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_BUG,
    },

    // 0920
    {
        .species = SPECIES_LOKIX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_FIRST_IMPRESSION,
            MOVE_SUCKER_PUNCH,
            MOVE_LEECH_LIFE,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_LOKIX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SILK_SCARF,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FIRST_IMPRESSION,
            MOVE_SUCKER_PUNCH,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0923
    {
        .species = SPECIES_PAWMOT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_DOUBLE_SHOCK,
            MOVE_CLOSE_COMBAT,
            MOVE_MACH_PUNCH,
            MOVE_NUZZLE
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_PAWMOT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT,
        .moves =
        {
            MOVE_VOLT_SWITCH,
            MOVE_CLOSE_COMBAT,
            MOVE_NUZZLE,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0925
    {
        .species = SPECIES_MAUSHOLD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_POPULATION_BOMB,
            MOVE_BULLET_SEED,
            MOVE_TIDY_UP,
            MOVE_ENCORE
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_MAUSHOLD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_BEAT_UP,
            MOVE_HELPING_HAND,
            MOVE_PROTECT
        },
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0927
    {
        .species = SPECIES_DACHSBUN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_BODY_PRESS,
            MOVE_WISH,
            MOVE_PROTECT
        },
        .ability = ABILITY_WELL_BAKED_BODY,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0930
    {
        .species = SPECIES_ARBOLIVA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_ENERGY_BALL,
            MOVE_EARTH_POWER,
            MOVE_STRENGTH_SAP,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_SEED_SOWER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 100,
            .spd = 156
        ),
        .teraType = TYPE_GRASS,
        .ball = BALL_NEST,
    },

    // 0931
    {
        .species = SPECIES_SQUAWKABILLY_GREEN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB,
        .moves =
        {
            MOVE_FACADE,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_HUSTLE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0934
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SALT_CURE,
            MOVE_RECOVER,
            MOVE_STEALTH_ROCK,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_SALT_CURE,
            MOVE_RECOVER
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_SALT_CURE,
            MOVE_WIDE_GUARD,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0936
    {
        .species = SPECIES_ARMAROUGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_ARMOR_CANNON,
            MOVE_PSYSHOCK,
            MOVE_AURA_SPHERE,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_ARMAROUGE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_EXPANDING_FORCE,
            MOVE_ARMOR_CANNON,
            MOVE_TRICK_ROOM,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0937
    {
        .species = SPECIES_CERULEDGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BITTER_BLADE,
            MOVE_SHADOW_SNEAK,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_CERULEDGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_BITTER_BLADE,
            MOVE_SHADOW_SNEAK,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0939
    {
        .species = SPECIES_BELLIBOLT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_VOLT_SWITCH,
            MOVE_MUDDY_WATER,
            MOVE_SLACK_OFF,
            MOVE_TOXIC
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 200,
            .spd = 56
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },
    {
        .species = SPECIES_BELLIBOLT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_PARABOLIC_CHARGE,
            MOVE_DISCHARGE,
            MOVE_MUDDY_WATER,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_DIVE,
    },

    // 0941
    {
        .species = SPECIES_KILOWATTREL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_HURRICANE,
            MOVE_VOLT_SWITCH,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0943
    {
        .species = SPECIES_MABOSSTIFF,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_PSYCHIC_FANGS,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MABOSSTIFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_COMEUPPANCE,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0945
    {
        .species = SPECIES_GRAFAIAI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_ENCORE,
            MOVE_GUNK_SHOT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0947
    {
        .species = SPECIES_BRAMBLEGHAST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_SHADOW_BALL,
            MOVE_RAPID_SPIN,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_BRAMBLEGHAST,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_POLTERGEIST,
            MOVE_RAPID_SPIN,
            MOVE_INFERNAL_PARADE
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0949
    {
        .species = SPECIES_TOEDSCRUEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SPORE,
            MOVE_RAPID_SPIN,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_MYCELIUM_MIGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_TOEDSCRUEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_ENERGY_BALL,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_MYCELIUM_MIGHT,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0950
    {
        .species = SPECIES_KLAWF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_KLAWF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_CRABHAMMER
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0952
    {
        .species = SPECIES_SCOVILLAIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_GROWTH,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0954
    {
        .species = SPECIES_RABSCA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_BUG_BUZZ,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(SPE, 0),
        .teraType = TYPE_PSYCHIC,
    },

    // 0956
    {
        .species = SPECIES_ESPATHRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_STORED_POWER,
            MOVE_DAZZLING_GLEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ESPATHRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0959
    {
        .species = SPECIES_TINKATON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_AIR_BALLOON,
        .moves =
        {
            MOVE_GIGATON_HAMMER,
            MOVE_PLAY_ROUGH,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_TINKATON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_GIGATON_HAMMER,
            MOVE_PLAY_ROUGH,
            MOVE_KNOCK_OFF,
            MOVE_ICE_HAMMER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0961
    {
        .species = SPECIES_WUGTRIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },

    // 0962
    {
        .species = SPECIES_BOMBIRDIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_KNOCK_OFF,
            MOVE_BRAVE_BIRD,
            MOVE_ROOST
        },
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0964
    {
        .species = SPECIES_PALAFIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_JET_PUNCH,
            MOVE_WAVE_CRASH,
            MOVE_CLOSE_COMBAT,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_ZERO_TO_HERO,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_PALAFIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_JET_PUNCH,
            MOVE_WAVE_CRASH,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_ZERO_TO_HERO,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0966
    {
        .species = SPECIES_REVAVROOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_SHIFT_GEAR,
            MOVE_GUNK_SHOT,
            MOVE_IRON_HEAD,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_REVAVROOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_SPIKES,
            MOVE_PARTING_SHOT,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0967
    {
        .species = SPECIES_CYCLIZAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SHED_TAIL,
            MOVE_DRAGON_PULSE,
            MOVE_OVERHEAT,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CYCLIZAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_DRAGON_CLAW,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0968
    {
        .species = SPECIES_ORTHWORM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_SHED_TAIL,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_ORTHWORM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_SHED_TAIL,
            MOVE_IRON_HEAD,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GHOST,
    },

    // 0970
    {
        .species = SPECIES_GLIMMORA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_POWER_GEM,
            MOVE_MORTAL_SPIN
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_GLIMMORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_POWER_GEM,
            MOVE_SLUDGE_WAVE,
            MOVE_EARTH_POWER,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0972
    {
        .species = SPECIES_HOUNDSTONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_LAST_RESPECTS,
            MOVE_BODY_PRESS,
            MOVE_PLAY_ROUGH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_FLUFFY,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0973
    {
        .species = SPECIES_FLAMIGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_THROAT_CHOP,
            MOVE_U_TURN
        },
        .ability = ABILITY_COSTAR,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_FLAMIGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_COSTAR,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0975
    {
        .species = SPECIES_CETITAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_NEVER_MELT_ICE,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
        .ball = BALL_DIVE,
    },
    {
        .species = SPECIES_CETITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
        .ball = BALL_DIVE,
    },

    // 0976
    {
        .species = SPECIES_VELUZA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MYSTIC_WATER,
        .moves =
        {
            MOVE_AQUA_CUTTER,
            MOVE_PSYCHO_CUT,
            MOVE_NIGHT_SLASH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_DIVE,
    },
    {
        .species = SPECIES_VELUZA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves =
        {
            MOVE_FILLET_AWAY,
            MOVE_AQUA_CUTTER,
            MOVE_PSYCHO_CUT,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_DIVE,
    },

    // 0977
    {
        .species = SPECIES_DONDOZO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CURSE,
            MOVE_WAVE_CRASH,
            MOVE_REST,
            MOVE_SLEEP_TALK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_DONDOZO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0978
    {
        .species = SPECIES_TATSUGIRI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THROAT_SPRAY,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DRACO_METEOR,
            MOVE_MUDDY_WATER,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0979
    {
        .species = SPECIES_ANNIHILAPE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_RAGE_FIST,
            MOVE_DRAIN_PUNCH,
            MOVE_TAUNT
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ANNIHILAPE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves =
        {
            MOVE_RAGE_FIST,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_ANGER_SHELL,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0980
    {
        .species = SPECIES_CLODSIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_CLODSIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET,
        .moves =
        {
            MOVE_CURSE,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0981
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC_NOISE,
            MOVE_HYPER_VOICE,
            MOVE_REST
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB,
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_FIRE,
    },

    // 0982
    {
        .species = SPECIES_DUDUNSPARCE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_COIL,
            MOVE_BODY_SLAM,
            MOVE_ROOST,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_DUDUNSPARCE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_BOOMBURST,
            MOVE_EARTH_POWER,
            MOVE_ROOST
        },
        .ability = ABILITY_SIMPLE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0983
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 112,
            .atk = 252,
            .spd = 144
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES,
        .moves =
        {
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 112,
            .atk = 252,
            .spd = 144
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0984
    {
        .species = SPECIES_GREAT_TUSK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_HEADLONG_RUSH,
            MOVE_CLOSE_COMBAT,
            MOVE_RAPID_SPIN,
            MOVE_ICE_SPINNER
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GREAT_TUSK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_HEADLONG_RUSH,
            MOVE_BODY_PRESS,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0985
    {
        .species = SPECIES_SCREAM_TAIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_DAZZLING_GLEAM,
            MOVE_ENCORE
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_SCREAM_TAIL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_PSYCHIC_FANGS,
            MOVE_THUNDER_WAVE,
            MOVE_WISH
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0986
    {
        .species = SPECIES_BRUTE_BONNET,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_SUCKER_PUNCH,
            MOVE_SEED_BOMB,
            MOVE_SPORE,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0987
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_SHADOW_BALL,
            MOVE_MYSTICAL_FIRE,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_SHADOW_BALL,
            MOVE_POWER_GEM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_BAND,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_SHADOW_BALL,
            MOVE_DAZZLING_GLEAM,
            MOVE_PROTECT
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0988
    {
        .species = SPECIES_SLITHER_WING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_FIRST_IMPRESSION,
            MOVE_U_TURN,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0989
    {
        .species = SPECIES_SANDY_SHOCKS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER,
            MOVE_VOLT_SWITCH,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_SANDY_SHOCKS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0990
    {
        .species = SPECIES_IRON_TREADS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_RAPID_SPIN,
            MOVE_ICE_SPINNER
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_IRON_TREADS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_RAPID_SPIN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0991
    {
        .species = SPECIES_IRON_BUNDLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_FREEZE_DRY,
            MOVE_FLIP_TURN,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_IRON_BUNDLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN,
            MOVE_FREEZE_DRY
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0992
    {
        .species = SPECIES_IRON_HANDS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_DRAIN_PUNCH,
            MOVE_THUNDER_PUNCH,
            MOVE_FAKE_OUT,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_IRON_HANDS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_FIRE,
    },

    // 0993
    {
        .species = SPECIES_IRON_JUGULIS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_HURRICANE,
            MOVE_EARTH_POWER,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0994
    {
        .species = SPECIES_IRON_MOTH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_FIERY_DANCE,
            MOVE_SLUDGE_WAVE,
            MOVE_ENERGY_BALL,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_IRON_MOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_FLAMETHROWER,
            MOVE_SLUDGE_WAVE,
            MOVE_TOXIC_SPIKES,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0995
    {
        .species = SPECIES_IRON_THORNS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 0998
    {
        .species = SPECIES_BAXCALIBUR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_SPEAR,
            MOVE_GLAIVE_RUSH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_BAXCALIBUR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_GLAIVE_RUSH,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 1000
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_RECOVER
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_RECOVER,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_TRICK
        },
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 1001
    {
        .species = SPECIES_WO_CHIEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_GIGA_DRAIN,
            MOVE_KNOCK_OFF,
            MOVE_PROTECT
        },
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_WO_CHIEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_GIGA_DRAIN,
            MOVE_FOUL_PLAY,
            MOVE_STUN_SPORE
        },
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 1002
    {
        .species = SPECIES_CHIEN_PAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_SUCKER_PUNCH,
            MOVE_SACRED_SWORD
        },
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_CHIEN_PAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_ICE_SPINNER,
            MOVE_CRUNCH,
            MOVE_SACRED_SWORD,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 1003
    {
        .species = SPECIES_TING_LU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_EARTHQUAKE,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_VESSEL_OF_RUIN,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_TING_LU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_RUINATION,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_REST
        },
        .ability = ABILITY_VESSEL_OF_RUIN,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 1004
    {
        .species = SPECIES_CHI_YU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_OVERHEAT,
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_BEADS_OF_RUIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CHI_YU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_DARK_PULSE,
            MOVE_PSYCHIC,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_BEADS_OF_RUIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 1005
    {
        .species = SPECIES_ROARING_MOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_OUTRAGE,
            MOVE_ROOST
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_ROARING_MOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 1006
    {
        .species = SPECIES_IRON_VALIANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_SPIRIT_BREAK
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_IRON_VALIANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_AURA_SPHERE,
            MOVE_PSYSHOCK,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 1007
    {
        .species = SPECIES_KORAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_COLLISION_COURSE,
            MOVE_FLARE_BLITZ,
            MOVE_DRAGON_CLAW,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_ORICHALCUM_PULSE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_KORAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_COLLISION_COURSE,
            MOVE_OUTRAGE,
            MOVE_FLARE_BLITZ,
            MOVE_U_TURN
        },
        .ability = ABILITY_ORICHALCUM_PULSE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 1008
    {
        .species = SPECIES_MIRAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_ELECTRO_DRIFT,
            MOVE_DRACO_METEOR,
            MOVE_VOLT_SWITCH,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_HADRON_ENGINE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_MIRAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_ELECTRO_DRIFT,
            MOVE_DRACO_METEOR,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_HADRON_ENGINE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 1009
    {
        .species = SPECIES_WALKING_WAKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_HYDRO_STEAM,
            MOVE_DRACO_METEOR,
            MOVE_FLAMETHROWER,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_WALKING_WAKE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_HYDRO_PUMP,
            MOVE_FLAMETHROWER,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 1010
    {
        .species = SPECIES_IRON_LEAVES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_PSYBLADE,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 1013
    {
        .species = SPECIES_SINISTCHA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MATCHA_GOTCHA,
            MOVE_SHADOW_BALL,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_SINISTCHA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_MATCHA_GOTCHA,
            MOVE_SHADOW_BALL,
            MOVE_GIGA_DRAIN,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = IVS(ATK, 0, SPE, 0),
        .teraType = TYPE_FAIRY,
    },

    // 1014
    {
        .species = SPECIES_OKIDOGI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_GUNK_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_OKIDOGI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_POISON_JAB,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 1015
    {
        .species = SPECIES_MUNKIDORI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SLUDGE_WAVE,
            MOVE_PSYCHIC,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },
    {
        .species = SPECIES_MUNKIDORI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_PSYCHIC,
            MOVE_U_TURN,
            MOVE_FUTURE_SIGHT
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 1016
    {
        .species = SPECIES_FEZANDIPITI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_ROOST,
            MOVE_TOXIC,
            MOVE_FOUL_PLAY,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_FEZANDIPITI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_SLUDGE_BOMB,
            MOVE_ROOST
        },
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },

    // 1017
    {
        .species = SPECIES_OGERPON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SEED_SOWER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 1017
    {
        .species = SPECIES_OGERPON_WELLSPRING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WELLSPRING_MASK,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_PLAY_ROUGH
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 1017
    {
        .species = SPECIES_OGERPON_HEARTHFLAME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEARTHFLAME_MASK,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_HORN_LEECH
        },
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 1017
    {
        .species = SPECIES_OGERPON_CORNERSTONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CORNERSTONE_MASK,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 1017
    {
        .species = SPECIES_OGERPON_TEAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SEED_SOWER,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 1018
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_ELECTRO_SHOT,
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_DRAGON_TAIL,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_ELECTRO_SHOT,
            MOVE_FLASH_CANNON,
            MOVE_DRACO_METEOR,
            MOVE_PROTECT
        },
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 1019
    {
        .species = SPECIES_HYDRAPPLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_FICKLE_BEAM,
            MOVE_GIGA_DRAIN,
            MOVE_NASTY_PLOT,
            MOVE_RECOVER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_HYDRAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_FICKLE_BEAM,
            MOVE_DRACO_METEOR,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 1020
    {
        .species = SPECIES_GOUGING_FIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_HEAT_CRASH,
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_GOUGING_FIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_DRAGON_CLAW,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 1021
    {
        .species = SPECIES_RAGING_BOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_THUNDERCLAP,
            MOVE_DRACO_METEOR,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_RAGING_BOLT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_THUNDERBOLT,
            MOVE_DRAGON_PULSE,
            MOVE_THUNDERCLAP
        },
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 1022
    {
        .species = SPECIES_IRON_BOULDER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_MIGHTY_CLEAVE,
            MOVE_EARTHQUAKE,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 1023
    {
        .species = SPECIES_IRON_CROWN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_TACHYON_CUTTER,
            MOVE_PSYCHIC_NOISE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 1024
    {
        .species = SPECIES_TERAPAGOS_TERASTAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_TERA_STARSTORM,
            MOVE_EARTH_POWER,
            MOVE_RECOVER
        },
        .ability = ABILITY_TERA_SHELL,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_TERAPAGOS_TERASTAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves =
        {
            MOVE_TERA_STARSTORM,
            MOVE_EARTH_POWER,
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_TERA_SHELL,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STELLAR,
    },

    // 1025
    {
        .species = SPECIES_PECHARUNT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_HEX,
            MOVE_SLUDGE_BOMB,
            MOVE_RECOVER
        },
        .ability = ABILITY_POISON_PUPPETEER,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_PECHARUNT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_MALIGNANT_CHAIN,
            MOVE_HEX,
            MOVE_PARTING_SHOT,
            MOVE_RECOVER
        },
        .ability = ABILITY_POISON_PUPPETEER,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
    },
};

const u16 gFrontierExtendedMonsCount = ARRAY_COUNT(gFrontierExtendedMons);

// FORK: draw one random roster index whose set is valid for the current battle
// format (singles/doubles). Uniform across the whole list (the roster has no
// weak->strong tier ordering), so there is no difficulty scaling by streak.
// Lives here, with the roster data, rather than in the upstream battle_factory.c,
// and is shared so any facility (Battle Factory, Battle Tower) can field opponents
// from the competitive list; callers handle their own species/held-item dedup.
u16 GetRandomFrontierExtendedMonId(void)
{
    u32 formatTag = (VarGet(VAR_FRONTIER_BATTLE_MODE) == FRONTIER_MODE_DOUBLES)
                  ? FORMAT_DOUBLES : FORMAT_SINGLES;
    u16 id;

    do
        id = Random() % gFrontierExtendedMonsCount;
    while (!(gFrontierExtendedMons[id].tags & formatTag));
    return id;
}
