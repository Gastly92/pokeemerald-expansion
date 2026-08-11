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
//          under B_FRONTIER_MAX_IVS); set .iv = TRAINER_PARTY_IVS(...) only for an
//          intentional spread (e.g. 0 Speed for Trick Room). Omit .ball for
//          BALL_POKE; set it only for a non-Poke look.
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
// the innate (e.g. Slowbro OWN_TEMPO + innate Regenerator, Rotom LIGHTNING_ROD +
// innate Levitate). Role comments naming an innate describe that innate-backed
// playstyle, not the .ability field. For the full picture — which species are
// freed vs. left "redundant-but-correct", and the per-species override picks — see
// fork-docs/INNATE_ABILITIES.md, fork-docs/INNATE_ABILITIES_PROGRESS.md, and the
// override rows in src/fork/species_ability_overrides.c. The current set of still-
// redundant sets is CI-guarded by sKnownRedundant in
// test/fork/frontier_extended_roster.c — that table, not a comment, is the record.
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
            MOVE_GIGA_DRAIN,
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
        .teraType = TYPE_STEEL,
    },

    // 0020
    {
        .species = SPECIES_RATICATE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_BODY_SLAM,
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
        .species = SPECIES_RATICATE_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
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
        .teraType = TYPE_STEEL,
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
        .species = SPECIES_SANDSLASH_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
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
        .heldItem = ITEM_LIFE_ORB,
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
        .heldItem = ITEM_LIFE_ORB,
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
        .ability = ABILITY_MAGIC_BOUNCE,
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
        .ability = ABILITY_MAGIC_BOUNCE,
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
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_WATER,
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
        .heldItem = ITEM_LEFTOVERS, // Bulky fairy
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
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Doubles support
        .moves =
        {
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND,
            MOVE_THUNDER_WAVE,
            MOVE_PROTECT
        },
        .ability = ABILITY_HALO,
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
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

    // 0045
    {
        .species = SPECIES_VILEPLUME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES, // boosts Sludge Bomb chip
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_SOLAR_POWER, // moved off Effect Spore (redundant deterministic sleep vs Sleep Powder); sun Sp.Atk w/ innate Chlorophyll
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // hazard-proof Chlorophyll
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB,
            MOVE_MOONBLAST,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_SOLAR_POWER, // moved off Effect Spore (redundant deterministic sleep vs Sleep Powder); sun Sp.Atk w/ innate Chlorophyll
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0047
    {
        .species = SPECIES_PARASECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Spore disabler
        .moves =
        {
            MOVE_SPORE,
            MOVE_SEED_BOMB,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_DRY_SKIN, // moved off Effect Spore (redundant deterministic sleep vs Spore); real slot, fungus flavor
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
        .heldItem = ITEM_FIGY_BERRY, // Dry Skin sweeper
        .moves =
        {
            MOVE_SPORE,
            MOVE_GIGA_DRAIN,
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

    // 0049
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // Quiver Dance + Sleep Powder sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_SLUDGE_BOMB,
            MOVE_SLEEP_POWDER
        },
        .ability = ABILITY_SHEER_FORCE, // moved off Effect Spore (deterministic sleep collides w/ Sleep Powder); powers Sludge Bomb/Bug Buzz
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },
    {
        .species = SPECIES_VENOMOTH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_SLUDGE_BOMB,
            MOVE_QUIVER_DANCE,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE, // moved off Effect Spore; powers Sludge Bomb/Bug Buzz
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0051
    {
        .species = SPECIES_DUGTRIO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // Arena Trap revenge / trapper
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SAND_STREAM, // Arena Trap/Sand Veil/Sand Force now innate; chosen Sand Stream (override) sets the sand that powers them
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
        .heldItem = ITEM_CHOICE_BAND, // Sand Force band
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH,
            MOVE_AERIAL_ACE
        },
        .ability = ABILITY_SAND_STREAM, // Arena Trap/Sand Veil/Sand Force now innate; chosen Sand Stream (override) sets the sand that powers them
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
        .heldItem = ITEM_FOCUS_BAND, // Sand Force trapper/revenge killer
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_STONE_EDGE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DUGTRIO_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SOFT_SAND, // Tangling Hair contact-punisher
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
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
        .heldItem = ITEM_BLACK_GLASSES, // Technician fast attacker
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_FAKE_OUT,
            MOVE_PLAY_ROUGH,
            MOVE_U_TURN
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_PERSIAN_ALOLA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET, // Fur Coat physical wall + support
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_FAKE_OUT,
            MOVE_PARTING_SHOT,
            MOVE_TAUNT
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 8,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0053
    {
        .species = SPECIES_PERSIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF, // Technician Fake Out
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_NASTY_PLOT,
            MOVE_HYPER_VOICE,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_FUR_COAT, // all real abilities innate; chosen Fur Coat (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Fast support
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_FAKE_OUT,
            MOVE_U_TURN,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_FUR_COAT, // all real abilities innate; chosen Fur Coat (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0055
    {
        .species = SPECIES_GOLDUCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_CALM_MIND,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_DAMP, // Swift Swim now innate; chosen Damp
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
        .heldItem = ITEM_LEFTOVERS,
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
            .spa = 4
        ),
        .teraType = TYPE_WATER,
    },

    // 0059
    {
        .species = SPECIES_ARCANINE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky pivot
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_MORNING_SUN,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE, // Intimidate now innate; chosen Flash Fire (real slot 1)
        .nature = NATURE(DEF_UP, SPA_DOWN),
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
        .heldItem = ITEM_CHOICE_BAND, // band wallbreaker
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
        .heldItem = ITEM_SITRUS_BERRY, // doubles Intimidate support
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_SNARL,
            MOVE_PROTECT
        },
        .ability = ABILITY_FLASH_FIRE, // Intimidate now innate; chosen Flash Fire (real slot 1)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_GRASS,
    },

    // 0059
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Rock Head Head Smash breaker
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_FLARE_BLITZ,
            MOVE_EXTREME_SPEED,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_FLASH_FIRE, // Rock Head now innate; chosen Flash Fire grants a Fire immunity
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Intimidate bulky pivot
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_ROCK_SLIDE,
            MOVE_EXTREME_SPEED,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_FLASH_FIRE, // Intimidate now innate; chosen Flash Fire (real slot 1)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0062
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Bulk Up
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
        .heldItem = ITEM_LIFE_ORB, // Swift Swim attacker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_DARKEST_LARIAT
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim now innate; chosen Water Absorb
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0065
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Magic Guard nuke
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Guard now innate (Tier 5.4); freed chosen slot to its real, non-innate Synchronize
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TWISTED_SPOON, // Mega Alakazam (Trace); Psychic STAB boost
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Guard now innate (Tier 5.4); freed chosen slot to its real, non-innate Synchronize
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // fast lead, sash = one-shot guard
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_ENCORE
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Guard now innate (Tier 5.4); freed chosen slot to its real, non-innate Synchronize
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0068
    {
        .species = SPECIES_MACHAMP,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // No Guard band breaker
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH,
            MOVE_STONE_EDGE
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
        .heldItem = ITEM_FLAME_ORB, // Guts staller-breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_BULLET_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // Guts now innate; No Guard lands its STABs reliably
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
        .heldItem = ITEM_ASSAULT_VEST, // doubles bulk
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
        .teraType = TYPE_FIGHTING,
    },

    // 0071
    {
        .species = SPECIES_VICTREEBEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves =
        {
            MOVE_SOLAR_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE, // moved off Effect Spore (shared override); safe on this physical set
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
            MOVE_WEATHER_BALL
        },
        .ability = ABILITY_SHEER_FORCE, // moved off Effect Spore (deterministic sleep collides w/ Sleep Powder); powers Sludge Bomb
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
        .heldItem = ITEM_LEFTOVERS, // bulky spinner / hazards
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RAPID_SPIN,
            MOVE_TOXIC_SPIKES,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB, // Clear Body / Liquid Ooze / Rain Dish all innate; chosen Water Absorb (override)
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
        .heldItem = ITEM_LIFE_ORB, // Rain Dish / offensive
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SLUDGE_WAVE,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Clear Body / Liquid Ooze / Rain Dish all innate; chosen Water Absorb (override)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0076
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sturdy band breaker
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_EXPLOSION,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_SAND_STREAM, // Rock Head + Sturdy + Sand Veil now innate; chosen Sand Stream turns on innate Sand Veil (override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GOLEM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CUSTAP_BERRY, // Sturdy + Custap lead w/ rocks
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_SAND_STREAM, // Rock Head + Sturdy + Sand Veil now innate; chosen Sand Stream turns on innate Sand Veil (override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0076
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY, // Galvanize Explosion/STAB attacker
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
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },
    {
        .species = SPECIES_GOLEM_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HARD_STONE, // Sturdy bulky tank
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_THUNDER_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_CURSE
        },
        .ability = ABILITY_GALVANIZE, // Magnet Pull & Sturdy now innate; chosen Galvanize (real slot) makes its Normal moves Electric for this slow tank
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_ROCK,
    },

    // 0078
    {
        .species = SPECIES_RAPIDASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
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
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_RAPIDASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_HORSEPOWER,
            MOVE_WILD_CHARGE,
            MOVE_MEGAHORN
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

    // 0078
    {
        .species = SPECIES_RAPIDASH_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_CALM_MIND
        },
        // FORK: Pastel Veil is now an innate (Step 3.5) — freed to its real Hidden Ability slot.
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // physical sweeper
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_FLARE_BLITZ,
            MOVE_AGILITY
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0080
    {
        .species = SPECIES_SLOWBRO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Mega Slowbro (Shell Armor); recovery for the Calm Mind wall
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_PSYSHOCK,
            MOVE_CALM_MIND,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_ICE_SCALES, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator pivot wall
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FUTURE_SIGHT,
            MOVE_SLACK_OFF,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_ICE_SCALES, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker (0 Spe IV)
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_CHILLING_WATER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_ICE_SCALES, // chosen via fork override (species_ability_overrides.c)
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_FAIRY,
    },

    // 0080
    {
        .species = SPECIES_SLOWBRO_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed tank
        .moves =
        {
            MOVE_SHELL_SIDE_ARM,
            MOVE_PSYCHIC,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_POISON_TOUCH, // Quick Draw/Own Tempo/Regenerator ALL now innate; chosen Poison Touch (fork override, slot 2) stays observable
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
        .moves =
        {
            MOVE_SHELL_SIDE_ARM,
            MOVE_SLUDGE_BOMB,
            MOVE_CALM_MIND,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_POISON_TOUCH, // Quick Draw/Own Tempo/Regenerator ALL now innate; chosen Poison Touch (fork override, slot 2) stays observable
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0083
    {
        .species = SPECIES_FARFETCHD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEEK, // Guaranteed crit
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BRAVE_BIRD,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SUPER_LUCK, // all real abilities innate; chosen Super Luck (non-redundant)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0085
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // fast band
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
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
        .heldItem = ITEM_FLYING_GEM, // one-shot Flying burst after Swords Dance
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0087
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Thick Fat tank
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_SURF,
            MOVE_ICY_WIND,
            MOVE_PROTECT
        },
        // Thick Fat + Hydration + Ice Body all now innate; chosen Snow Warning (override) sets the snow its
        // innate Ice Body heals in.
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_DEWGONG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_NEVER_MELT_ICE, // Freeze-Dry tech
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_SURF,
            MOVE_AURORA_VEIL,
            MOVE_PROTECT
        },
        // Snow Warning (override) auto-sets snow, enabling Aurora Veil without a lead and feeding innate Ice Body.
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0089
    {
        .species = SPECIES_MUK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky special tank
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_TOXIC
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

    // 0089
    {
        .species = SPECIES_MUK_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Power of Alchemy special wall
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
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_MUK_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Poison Touch bulky attacker
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
        .teraType = TYPE_POISON,
    },

    // 0091
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper, Skill Link max multi-hit
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_ICICLE_SPEAR,
            MOVE_ROCK_BLAST,
            MOVE_HYDRO_PUMP
        },
        .ability = ABILITY_SNIPER, // Shell Armor + Skill Link + Overcoat now innate; chosen Sniper crits Icicle Spear (override)
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND, // guaranteed Shell Smash via one-shot entry guard
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_ICICLE_SPEAR,
            MOVE_ROCK_BLAST,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SNIPER, // Shell Armor + Skill Link + Overcoat now innate; chosen Sniper crits Icicle Spear (override)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive spiker
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICICLE_SPEAR,
            MOVE_RAPID_SPIN,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SNIPER, // Shell Armor + Skill Link + Overcoat now innate; chosen Sniper crits Icicle Spear (override)
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0094
    {
        .species = SPECIES_GENGAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mega Gengar (Shadow Tag); power for the trapping nuke
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_CURSED_BODY,
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
        .heldItem = ITEM_FOCUS_BAND, // fast lead w/ Destiny Bond
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_CURSED_BODY,
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_TRICK
        },
        .ability = ABILITY_CURSED_BODY,
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
        .heldItem = ITEM_GHOST_GEM, // one-shot Ghost burst nuke
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_SLUDGE_WAVE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0097
    {
        .species = SPECIES_HYPNO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // CM wall
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_CALM_MIND,
            MOVE_FOUL_PLAY,
            MOVE_WISH
        },
        .ability = ABILITY_BAD_DREAMS, // all real abilities innate; chosen Bad Dreams (non-redundant)
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_BAD_DREAMS, // all real abilities innate; chosen Bad Dreams (non-redundant)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0099
    {
        .species = SPECIES_KINGLER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MYSTIC_WATER, // Sheer Force pincer
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CRABHAMMER,
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
        .heldItem = ITEM_SCOPE_LENS, // Guaranteed crit claw
        .moves =
        {
            MOVE_CRABHAMMER,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE, // Hyper Cutter now innate (Shell Armor too); chosen Sheer Force powers its coverage
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0101
    {
        .species = SPECIES_ELECTRODE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MAGNET, // boosts STAB Electric
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
        .heldItem = ITEM_SITRUS_BERRY, // fast Thunder Wave support
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
        .heldItem = ITEM_MAGNET, // Blazing speed
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_THUNDER_WAVE,
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
        .species = SPECIES_ELECTRODE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Screens setter
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun nuke
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_PSYCHIC,
            MOVE_SLEEP_POWDER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SAP_SIPPER, // Chlorophyll + Harvest now innate; chosen Sap Sipper (override, empty slot 1)
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
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_LEAF_STORM,
            MOVE_PSYCHIC,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_SAP_SIPPER, // Chlorophyll + Harvest now innate; chosen Sap Sipper (override, empty slot 1)
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GRASS,
    },

    // 0103
    {
        .species = SPECIES_EXEGGUTOR_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Trick Room sweeper (0 Speed IVs)
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_LEAF_STORM,
            MOVE_DRAGON_HAMMER,
            MOVE_WOOD_HAMMER
        },
        .ability = ABILITY_SAP_SIPPER, // Frisk + Harvest now innate; chosen Sap Sipper (override, empty slot 1); innate Harvest still recycles the berry
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_DRAGON,
    },
    {
        .species = SPECIES_EXEGGUTOR_ALOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY, // Harvest bulky mixed attacker
        .moves =
        {
            MOVE_DRAGON_HAMMER,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_SAP_SIPPER, // Frisk + Harvest now innate; chosen Sap Sipper (override, empty slot 1); innate Harvest still recycles the berry
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 128,
            .spa = 128
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_GRASS,
    },

    // 0105
    {
        .species = SPECIES_MAROWAK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THICK_CLUB, // doubles up Attack; Trick Room sweeper
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_BONEMERANG
        },
        .ability = ABILITY_LIGHTNING_ROD, // Rock Head now innate; chosen Lightning Rod draws in Electric moves
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_MAROWAK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_THICK_CLUB, // SD physical sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_LIGHTNING_ROD,
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
        .heldItem = ITEM_THICK_CLUB, // iconic Thick Club + Rock Head Flare Blitz
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_SHADOW_BONE,
            MOVE_BONEMERANG,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_LIGHTNING_ROD, // Rock Head now innate; chosen Lightning Rod is its signature
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
        .heldItem = ITEM_THICK_CLUB, // Lightning Rod redirection bruiser
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
        .heldItem = ITEM_QUICK_CLAW, // Reckless / Unburden sweeper
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_MACH_PUNCH,
            MOVE_STONE_EDGE
        },
        // Limber/Reckless/Unburden all now innate; chosen No Guard makes its kicks never miss (fork override).
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
        .heldItem = ITEM_FIGHTING_GEM, // one-shot Fighting burst nuke
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_BLAZE_KICK
        },
        .ability = ABILITY_NO_GUARD, // Limber/Reckless/Unburden now innate; chosen No Guard (kicks never miss)
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
        .heldItem = ITEM_ASSAULT_VEST, // Iron Fist bulky attacker
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_THUNDER_PUNCH,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // Keen Eye/Iron Fist/Inner Focus ALL now innate; chosen No Guard (fork override, slot 2) makes its punches never miss
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_FIGHTING,
    },
    {
        .species = SPECIES_HITMONCHAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // Keen Eye/Iron Fist/Inner Focus ALL now innate; chosen No Guard (fork override, slot 2) makes its punches never miss
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
        .heldItem = ITEM_BLACK_SLUDGE, // Levitate phys wall
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_WILL_O_WISP,
            MOVE_PAIN_SPLIT,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_NEUTRALIZING_GAS,
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
        .heldItem = ITEM_LEFTOVERS, // defensive wall
        .moves =
        {
            MOVE_STRANGE_STEAM,
            MOVE_SLUDGE_BOMB,
            MOVE_WILL_O_WISP,
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
        .heldItem = ITEM_ROCKY_HELMET, // hazard control & Neutralizing Gas
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_DEFOG,
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
        .heldItem = ITEM_LEFTOVERS, // Misty Surge support
        .moves =
        {
            MOVE_STRANGE_STEAM,
            MOVE_SLUDGE_BOMB,
            MOVE_WILL_O_WISP,
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
        .heldItem = ITEM_EVIOLITE, // special wall
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_TOXIC,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_SILK_SCARF, // Mega Kangaskhan (Parental Bond); boosts its Normal double-hits
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SUCKER_PUNCH,
            MOVE_POWER_UP_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_SILK_SCARF, // no-mega Scrappy attacker
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_MYSTIC_WATER, // Lightning Rod draw
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
        .heldItem = ITEM_LUM_BERRY, // Swift Swim sweep
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_MEGAHORN,
            MOVE_AGILITY,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_LIGHTNING_ROD, // Swift Swim now innate; chosen Lightning Rod
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
        .heldItem = ITEM_LIFE_ORB, // fast special pivot
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_PSYSHOCK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Natural Cure spinner
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RAPID_SPIN,
            MOVE_RECOVER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_SPECS, // breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_PSYCHIC,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_LIGHT_CLAY, // Screens support
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_DAZZLING_GLEAM,
            MOVE_FOLLOW_ME
        },
        .ability = ABILITY_SOUNDPROOF, // Filter now innate; chosen Soundproof shrugs off spread sound moves (doubles support)
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
        .heldItem = ITEM_TWISTED_SPOON, // Technician special
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_ICY_WIND,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SOUNDPROOF, // Filter + Technician now innate; chosen Soundproof blanks sound moves on the special pivot
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
        .heldItem = ITEM_FOCUS_BAND, // Lovely Kiss lead
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
        .heldItem = ITEM_ICE_GEM, // one-shot Ice burst nuke
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
        .heldItem = ITEM_SHARP_BEAK, // Mega Pinsir (Aerilate); boosts its -ate Flying moves
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_RETURN,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_TOUGH_CLAWS, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_SCARF, // Moxie revenge (no mega)
        .moves =
        {
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_TOUGH_CLAWS, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band
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
        .heldItem = ITEM_LIFE_ORB, // Intimidate lure
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (real slot 2)
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
        .heldItem = ITEM_ROCKY_HELMET, // chip + Intimidate pivot
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up sweeper
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // hazard-immune attacker
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_FLARE_BLITZ,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (slot 1)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0128
    {
        .species = SPECIES_TAUROS_PALDEA_AQUA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_MYSTIC_WATER, // boosts Raging Bull (Water)
        .moves =
        {
            MOVE_RAGING_BULL,
            MOVE_CLOSE_COMBAT,
            MOVE_WAVE_CRASH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (slot 1)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0130
    {
        .species = SPECIES_GYARADOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUM_BERRY, // Mega Gyarados (Mold Breaker); status insurance for the Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Intimidate now innate; chosen Motor Drive (slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // no-mega DD sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_POWER_WHIP,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Intimidate now innate; chosen Motor Drive (slot 1)
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
        .heldItem = ITEM_FLYING_GEM, // one-shot Flying burst after Dragon Dance
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_BOUNCE,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // all real abilities innate; chosen Motor Drive (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Water Absorb tank
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
        .heldItem = ITEM_LIGHT_CLAY, // extends the screens for the Aurora Veil setter
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_FREEZE_DRY,
            MOVE_SURF,
            MOVE_PROTECT
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
        .heldItem = ITEM_CHOICE_SCARF, // outspeed copied foe
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
        .heldItem = ITEM_LEFTOVERS, // Wish pivot wall
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
        .heldItem = ITEM_ELECTRIC_GEM, // one-shot Electric burst nuke
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
        .heldItem = ITEM_CHOICE_SPECS, // fast breaker
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
        .heldItem = ITEM_CHOICE_BAND, // Flash Fire / Guts band
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
        .heldItem = ITEM_FLAME_ORB, // Guts breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_FLARE_BLITZ,
            MOVE_SUPERPOWER,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_FLASH_FIRE, // Guts now innate; Flash Fire adds a Fire immunity
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Shell Smash sweeper
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
        .heldItem = ITEM_HARD_STONE, // Rain abuser
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_POWER_GEM,
            MOVE_EARTH_POWER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WEAK_ARMOR, // Swift Swim & Shell Armor now innate; chosen Weak Armor banks Speed when hit
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
        .heldItem = ITEM_LUM_BERRY, // Battle Armor blade
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WEAK_ARMOR, // Battle Armor & Swift Swim now innate; chosen Weak Armor banks Speed when hit
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Weak Armor sweep
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
        .heldItem = ITEM_KINGS_ROCK, // outspeeds almost everything; its first Thunderbolt (no innate flinch) flinches via King's Rock, stealing a turn
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
        .ability = ABILITY_TOUGH_CLAWS, // Rock Head + Pressure now innate; chosen Tough Claws (its Mega's ability) powers contact STAB (override)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0143
    // EXCLUSION: Snorlax's three real abilities (Immunity, Thick Fat, Gluttony) are ALL now innate, but
    // ALL THREE slots are test-pinned -- Immunity (immunity.c / synchronize.c / corrosion.c / check_bad_move.c /
    // deterministic_abilities.c / hit_switch_target.c), Thick Fat (thick_fat.c), Gluttony (the innate test's
    // Ability(ABILITY_GLUTTONY) chosen-differs exemplar) -- so it keeps chosen Gluttony (redundant-but-correct),
    // like Clefable (W2) / Excadrill & Tinkaton (W3). Do NOT free it.
    {
        .species = SPECIES_SNORLAX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Curse setup
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_CURSE,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        // Thick Fat (and Immunity) now innate; chosen Gluttony fills the slot.
        .ability = ABILITY_SAP_SIPPER, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_CRUNCH,
            MOVE_SELF_DESTRUCT
        },
        // Thick Fat (and Immunity) now innate; chosen Gluttony fills the slot.
        .ability = ABILITY_SAP_SIPPER, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_NORMAL_GEM, // one-shot Normal burst for the Giga Impact set
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_SAP_SIPPER, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky special wall
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_HAZE
        },
        .ability = ABILITY_SNOW_WARNING, // Pressure + Snow Cloak now innate; chosen Snow Warning (override) sets snow for Snow Cloak
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
        .ability = ABILITY_SNOW_WARNING, // Pressure + Snow Cloak now innate; chosen Snow Warning (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Calm Mind setup
        .moves =
        {
            MOVE_FREEZING_GLARE,
            MOVE_HURRICANE,
            MOVE_CALM_MIND,
            MOVE_ROOST
        },
        .ability = ABILITY_COMPETITIVE,
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
        .heldItem = ITEM_TWISTED_SPOON, // immediate special attacker
        .moves =
        {
            MOVE_FREEZING_GLARE,
            MOVE_PSYCHIC,
            MOVE_HURRICANE,
            MOVE_RECOVER
        },
        .ability = ABILITY_COMPETITIVE,
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // pivot
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
        .heldItem = ITEM_LEFTOVERS, // defensive sub-roost
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE,
            MOVE_ROOST
        },
        .ability = ABILITY_STATIC,
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // physical breaker
        .moves =
        {
            MOVE_THUNDEROUS_KICK,
            MOVE_CLOSE_COMBAT,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN
        },
        .ability = ABILITY_DEFIANT,
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up sweeper
        .moves =
        {
            MOVE_THUNDEROUS_KICK,
            MOVE_BRAVE_BIRD,
            MOVE_BULK_UP,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_DEFIANT,
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky pivot
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
        .heldItem = ITEM_CHOICE_SPECS, // breaker
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Nasty Plot sweeper
        .moves =
        {
            MOVE_FIERY_WRATH,
            MOVE_HURRICANE,
            MOVE_NASTY_PLOT,
            MOVE_AGILITY
        },
        .ability = ABILITY_BERSERK,
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Berserk + WP snowball
        .moves =
        {
            MOVE_FIERY_WRATH,
            MOVE_AIR_SLASH,
            MOVE_NASTY_PLOT,
            MOVE_HURRICANE
        },
        .ability = ABILITY_BERSERK,
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
        .ability = ABILITY_RECKLESS,
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0150
    {
        .species = SPECIES_MEWTWO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mega Mewtwo Y (Insomnia); power for the special nuke
        .moves =
        {
            MOVE_PSYSTRIKE,
            MOVE_AURA_SPHERE,
            MOVE_ICE_BEAM,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SYNCHRONIZE, // all real abilities innate; chosen Synchronize (non-redundant)
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
        .heldItem = ITEM_MUSCLE_BAND, // Mega Mewtwo X (Steadfast); physical boost
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_PSYCHIC_FANGS,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SYNCHRONIZE, // all real abilities innate; chosen Synchronize (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // no-mega all-out attacker
        .moves =
        {
            MOVE_PSYSTRIKE,
            MOVE_AURA_SPHERE,
            MOVE_FIRE_BLAST,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SYNCHRONIZE, // all real abilities innate; chosen Synchronize (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // mixed nasty plot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // utility lead
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
        .heldItem = ITEM_CHOICE_BAND, // physical pivot
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
        .heldItem = ITEM_LEFTOVERS, // bulky defensive pivot / cleric
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_AROMATHERAPY,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_GRASSY_SURGE, // Leaf Guard now innate; chosen Grassy Surge (override) sets healing terrain
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
        .heldItem = ITEM_MIRACLE_SEED, // dragon dance physical attacker
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_HORN_LEECH,
            MOVE_PLAY_ROUGH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_GRASSY_SURGE, // Leaf Guard now innate; chosen Grassy Surge (override) sets healing terrain
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // defensive spreader with hazards
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_TOXIC,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_GRASSY_SURGE, // Leaf Guard now innate; chosen Grassy Surge (override) sets healing terrain
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0157
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_EARTH_POWER,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze now innate (latched); chosen Flash Fire
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
        .heldItem = ITEM_LIFE_ORB, // nasty plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze now innate (latched); chosen Flash Fire
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
        .heldItem = ITEM_HEAT_ROCK, // sun setter for the team
        .moves =
        {
            MOVE_SUNNY_DAY,
            MOVE_FIRE_BLAST,
            MOVE_SOLAR_BEAM,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze now innate (latched); chosen Flash Fire
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
        .heldItem = ITEM_CHOICE_SPECS, // Frisk Ghost/Fire breaker
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_INFERNAL_PARADE
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Nasty Plot Hex sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_HEX,
            MOVE_FIRE_BLAST,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // dragon dance physical sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH,
            MOVE_CRUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Torrent now innate (latched); chosen Sheer Force
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
        .heldItem = ITEM_CHOICE_BAND, // band wallbreaker with priority
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SHEER_FORCE, // Torrent now innate (latched); chosen Sheer Force
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
        .heldItem = ITEM_LEFTOVERS, // bulky belly-drum sweeper
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Torrent now innate (latched); chosen Sheer Force
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
        .heldItem = ITEM_SHELL_BELL, // scrappy U-turn pivot
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_SUPER_FANG
        },
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 4,
            .atk = 252,
            .spe = 252
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0164
    {
        .species = SPECIES_NOCTOWL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Tinted Lens special
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_ROOST
        },
        .ability = ABILITY_FRISK, // all real abilities innate; chosen Frisk (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Insomnia staller
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_FRISK, // all real abilities innate; chosen Frisk (non-redundant)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0166
    {
        .species = SPECIES_LEDIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EXPERT_BELT, // Iron Fist mixed
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_BUG_BUZZ,
            MOVE_AURA_SPHERE,
            MOVE_ROOST
        },
        .ability = ABILITY_TINTED_LENS, // all real abilities innate; chosen Tinted Lens (non-redundant)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0168
    {
        .species = SPECIES_ARIADOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Web setter
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_TOXIC_SPIKES,
            MOVE_POISON_JAB,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_POISON_POINT, // all real abilities innate; chosen Poison Point (non-redundant)
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
        .heldItem = ITEM_SCOPE_LENS, // Sniper jabs
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_POISON_JAB,
            MOVE_SUCKER_PUNCH,
            MOVE_LEECH_LIFE
        },
        .ability = ABILITY_POISON_POINT, // all real abilities innate; chosen Poison Point (non-redundant)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0169
    {
        .species = SPECIES_CROBAT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast physical attacker / pivot
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_GUNK_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN
        },
        .ability = ABILITY_RECKLESS, // Inner Focus + Infiltrator now innate; chosen Reckless (fork override, slot 1) powers Brave Bird
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },
    {
        .species = SPECIES_CROBAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive defogger / status
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_TAUNT
        },
        .ability = ABILITY_RECKLESS, // Inner Focus + Infiltrator now innate; chosen Reckless powers Brave Bird (override)
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
        .heldItem = ITEM_CHOICE_BAND, // band pivot wallbreaker
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_GUNK_SHOT,
            MOVE_U_TURN,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_RECKLESS, // Inner Focus + Infiltrator now innate; chosen Reckless (fork override, slot 1) powers Brave Bird
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
        .heldItem = ITEM_LEFTOVERS, // bulky Volt Absorb pivot
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
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

    // 0178
    {
        .species = SPECIES_XATU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Magic Bounce support
        .moves =
        {
            MOVE_ROOST,
            MOVE_THUNDER_WAVE,
            MOVE_PSYCHIC,
            MOVE_U_TURN
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Bounce now innate (Tier 5.8); chosen Synchronize (:x: stable, its real slot 0) is the freed complementary ability
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },
    {
        .species = SPECIES_XATU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_TWISTED_SPOON, // Tailwind setter
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
        .heldItem = ITEM_MAGNET, // Mega Ampharos (Mold Breaker); Electric STAB boost
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_VOLT_SWITCH
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker (no mega)
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
        .heldItem = ITEM_ASSAULT_VEST, // bulky special tank
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_DRAGON_PULSE,
            MOVE_POWER_WHIP,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC,
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
        .heldItem = ITEM_LIFE_ORB, // chlorophyll sun sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_MOONBLAST,
            MOVE_WEATHER_BALL
        },
        .ability = ABILITY_SOLAR_POWER, // moved off Effect Spore (shared override); sun Sp.Atk w/ innate Chlorophyll
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive quiver dance + sleep
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_SOLAR_POWER, // moved off Effect Spore (redundant deterministic sleep vs Sleep Powder); sun Sp.Atk w/ innate Chlorophyll
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0184
    {
        .species = SPECIES_AZUMARILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Huge Power band breaker w/ priority
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_PLAY_ROUGH,
            MOVE_AQUA_JET,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAP_SIPPER, // Huge Power now innate; chosen Sap Sipper adds a Grass immunity + Atk boost
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
        .heldItem = ITEM_SITRUS_BERRY, // belly drum + aqua jet sweeper
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_AQUA_JET,
            MOVE_PLAY_ROUGH,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_SAP_SIPPER, // Huge Power now innate; chosen Sap Sipper adds a Grass immunity + Atk boost
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with utility
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_AQUA_JET,
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION
        },
        .ability = ABILITY_SAP_SIPPER, // Huge Power now innate; chosen Sap Sipper adds a Grass immunity + Atk boost
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
        .heldItem = ITEM_CHOICE_BAND, // Rock Head reckless band attacker
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_WOOD_HAMMER,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SOLID_ROCK, // Sturdy + Rock Head now innate; chosen Solid Rock blunts its weaknesses (override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_ROCK,
    },
    {
        .species = SPECIES_SUDOWOODO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // sturdy physical wall w/ rocks
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SOLID_ROCK, // Sturdy + Rock Head now innate; chosen Solid Rock blunts its weaknesses (override)
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
        .heldItem = ITEM_DAMP_ROCK, // Drizzle rain setter
        .moves =
        {
            MOVE_RAIN_DANCE,
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
        .heldItem = ITEM_LEFTOVERS, // bulky water with utility
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
        .heldItem = ITEM_CHOICE_SPECS, // rain-boosted special breaker
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
        .heldItem = ITEM_LEFTOVERS, // fast sleep + leech seed staller
        .moves =
        {
            MOVE_SLEEP_POWDER,
            MOVE_LEECH_SEED,
            MOVE_SUBSTITUTE,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_PRANKSTER, // Chlorophyll + Leaf Guard + Infiltrator now innate; chosen Prankster for its status kit (override)
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLYING_GEM, // chlorophyll offensive utility
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_ACROBATICS,
            MOVE_SLEEP_POWDER,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_PRANKSTER, // Chlorophyll + Leaf Guard + Infiltrator now innate; chosen Prankster for its status kit (override)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0192
    {
        .species = SPECIES_SUNFLORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Chlorophyll sun nuke
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_EARTH_POWER,
            MOVE_WEATHER_BALL,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_SOLAR_POWER, // Chlorophyll now innate; chosen Solar Power (sun synergy)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_SUNFLORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // growth sun sweeper
        .moves =
        {
            MOVE_GROWTH,
            MOVE_GIGA_DRAIN,
            MOVE_WEATHER_BALL,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SOLAR_POWER, // Chlorophyll now innate; chosen Solar Power (sun synergy)
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
        .heldItem = ITEM_LEFTOVERS, // Unaware physical wall
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB, // Unaware now innate; chosen Water Absorb adds a Water immunity + heal
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
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulk water absorber
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
        .heldItem = ITEM_LIFE_ORB, // Magic Bounce special sweeper
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Bounce now innate (Tier 5.8); chosen Synchronize (:x: stable, its real slot 0) is the freed complementary ability
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
        .heldItem = ITEM_CHOICE_SPECS, // hazard-bouncing special breaker
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_TRICK
        },
        .ability = ABILITY_SYNCHRONIZE, // Magic Bounce now innate (Tier 5.8); chosen Synchronize (:x: stable, its real slot 0) is the freed complementary ability
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
        .heldItem = ITEM_LEFTOVERS, // wish-passing special wall
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // physically defensive cleric
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_HEAL_BELL,
            MOVE_WISH,
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator special wall / pivot
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FUTURE_SIGHT,
            MOVE_SLACK_OFF,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_WATER_ABSORB, // Oblivious/Own Tempo/Regenerator ALL now innate; chosen Water Absorb (fork override, slot 2) stays observable
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
        .heldItem = ITEM_ASSAULT_VEST, // bulky special attacker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_PSYSHOCK,
            MOVE_ICE_BEAM,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_WATER_ABSORB, // Oblivious/Own Tempo/Regenerator ALL now innate; chosen Water Absorb (fork override, slot 2) stays observable
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
        .heldItem = ITEM_TWISTED_SPOON, // calm mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_CHILLING_WATER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_WATER_ABSORB, // Oblivious/Own Tempo/Regenerator ALL now innate; chosen Water Absorb (fork override, slot 2) stays observable
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0199
    {
        .species = SPECIES_SLOWKING_GALAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky special pivot
        .moves =
        {
            MOVE_FUTURE_SIGHT,
            MOVE_SLUDGE_BOMB,
            MOVE_PSYCHIC,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_CURIOUS_MEDICINE, // Own Tempo now innate; chosen Curious Medicine (real slot) for doubles support
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
        .heldItem = ITEM_BLACK_SLUDGE, // Nasty Plot wallbreaker
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SLUDGE_BOMB,
            MOVE_PSYCHIC,
            MOVE_CHILLY_RECEPTION
        },
        .ability = ABILITY_CURIOUS_MEDICINE, // Own Tempo now innate; chosen Curious Medicine (real slot) for doubles support
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0202
    {
        .species = SPECIES_WOBBUFFET,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shadow Tag trapper
        .moves =
        {
            MOVE_COUNTER,
            MOVE_MIRROR_COAT,
            MOVE_ENCORE,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_SYNCHRONIZE, // all real abilities innate; chosen Synchronize (non-redundant)
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_NORMAL,
    },

    // 0205
    {
        .species = SPECIES_FORRETRESS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // hazard setter / spinner
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_RAPID_SPIN,
            MOVE_GYRO_BALL
        },
        .ability = ABILITY_FILTER, // Overcoat now innate; chosen Filter (override) blunts its Fire weakness
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_FORRETRESS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // physical wall with Volt Switch pivot
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_VOLT_SWITCH,
            MOVE_RAPID_SPIN,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_FILTER, // Overcoat now innate; chosen Filter (override) blunts its Fire weakness
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
    },

    // 0208
    {
        .species = SPECIES_STEELIX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Mega Steelix (Sand Force); chips the physical attackers it walls
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_STEALTH_ROCK,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SHEER_FORCE, // Rock Head + Sturdy now innate; chosen Sheer Force
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
        .heldItem = ITEM_LEFTOVERS, // Sturdy hazard tank (no mega)
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE, // Rock Head + Sturdy now innate; chosen Sheer Force
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
        .heldItem = ITEM_CHOICE_BAND, // band trapper-style breaker
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_STONE_EDGE,
            MOVE_ICE_PUNCH
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

    // 0210
    {
        .species = SPECIES_GRANBULL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Intimidate band breaker
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_STATIC, // Intimidate now innate; chosen Static (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with status
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_KNOCK_OFF,
            MOVE_THUNDER_WAVE,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_STATIC, // Intimidate now innate; chosen Static (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Intimidate hazard setter
        .moves =
        {
            MOVE_SPIKES,
            MOVE_TOXIC_SPIKES,
            MOVE_LIQUIDATION,
            MOVE_HAZE
        },
        .ability = ABILITY_POISON_POINT, // Intimidate now innate; chosen Poison Point (real slot 0)
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
        .heldItem = ITEM_LIFE_ORB, // swift swim rain attacker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_GUNK_SHOT,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_POISON_POINT, // Swift Swim + Intimidate now innate; chosen Poison Point (real slot 0)
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
        .heldItem = ITEM_METAL_COAT, // Mega Scizor (Technician); Steel STAB boost for Bullet Punch
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BULLET_PUNCH,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_TOUGH_CLAWS, // Swarm + Technician + Light Metal now innate; chosen Tough Claws powers Bullet Punch / U-turn (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Technician band breaker w/ priority
        .moves =
        {
            MOVE_BULLET_PUNCH,
            MOVE_U_TURN,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_TOUGH_CLAWS, // Swarm + Technician + Light Metal now innate; chosen Tough Claws powers Bullet Punch / U-turn (override)
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
        .heldItem = ITEM_LEFTOVERS, // defensive defogger / pivot
        .moves =
        {
            MOVE_BULLET_PUNCH,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Swarm + Technician + Light Metal now innate; chosen Tough Claws powers Bullet Punch / U-turn (override)
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
        .heldItem = ITEM_MENTAL_HERB, // anti-Taunt support
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_ENCORE,
            MOVE_KNOCK_OFF,
            MOVE_TOXIC
        },
        .ability = ABILITY_CONTRARY, // Sturdy now innate; chosen Contrary turns stat drops into boosts on this staller
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 128,
            .spd = 128
        ),
        .teraType = TYPE_BUG,
    },

    // 0214
    {
        .species = SPECIES_HERACROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_BELT, // Mega Heracross (Skill Link); Fighting STAB boost for the multi-hit breaker
        .moves =
        {
            MOVE_PIN_MISSILE,
            MOVE_ROCK_BLAST,
            MOVE_BULLET_SEED,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Guts band breaker (no mega)
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_MEGAHORN,
            MOVE_KNOCK_OFF,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_FLAME_ORB, // Guts self-status breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_MEGAHORN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_FLAME_ORB, // Guts Facade wallbreaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker with coverage
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Shell Smash + WP payoff
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
        .heldItem = ITEM_ROCKY_HELMET, // hazard setter
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator wall
        .moves =
        {
            MOVE_RECOVER,
            MOVE_CHILLING_WATER,
            MOVE_POWER_GEM,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_HUSTLE, // Regen + Natural Cure both innate; Hustle is the only other real slot (inert on this special set)
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
        .heldItem = ITEM_ROCKY_HELMET, // Natural Cure pivot
        .moves =
        {
            MOVE_RECOVER,
            MOVE_CHILLING_WATER,
            MOVE_POWER_GEM,
            MOVE_TOXIC
        },
        .ability = ABILITY_HUSTLE, // Regen + Natural Cure both innate; Hustle is the only other real slot (inert on this special set)
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
        .heldItem = ITEM_CHOICE_SPECS, // Sniper / special breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FIRE_BLAST,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_MOODY, // Sniper now innate; chosen Moody (real slot 2)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },
    {
        .species = SPECIES_OCTILLERY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sniper Life Orb attacker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_GUNK_SHOT,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_MOODY, // Sniper now innate; chosen Moody (real slot 2)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0225
    {
        .species = SPECIES_DELIBIRD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_BAND, // Pure flavor
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

    // 0226
    {
        .species = SPECIES_MANTINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky defogger / special wall
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
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
        .heldItem = ITEM_ROCKY_HELMET, // hazard setter physical wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_ROOST,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF, // Sturdy + Keen Eye now both innate; chosen Bulletproof (fork override) blocks ball/bomb moves on this wall
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
        .heldItem = ITEM_LEFTOVERS, // defogger pivot
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DEFOG,
            MOVE_ROOST,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_BULLETPROOF, // Sturdy + Keen Eye now both innate; chosen Bulletproof (fork override) shields this defogger from Focus Blast/Sludge Bomb
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0229
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHARCOAL, // Mega Houndoom (Solar Power); Fire STAB boost
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
        .heldItem = ITEM_CHOICE_SPECS, // Flash Fire special breaker (no mega)
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
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB,
            MOVE_OVERHEAT
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
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_DAMP, // Swift Swim & Sniper now innate; chosen Damp (real slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // dragon dance physical sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_OUTRAGE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_DAMP, // Swift Swim & Sniper now innate; chosen Damp (real slot 2)
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
        .heldItem = ITEM_CHOICE_SPECS, // specs special breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_DAMP, // Swift Swim & Sniper now innate; chosen Damp (real slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Sturdy hazard setter / spinner
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SAND_STREAM, // Sturdy + Sand Veil now innate; chosen Sand Stream (override) sets sand for Sand Veil
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker with priority
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM, // Sturdy + Sand Veil now innate; chosen Sand Stream (override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_DONPHAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulk pivot
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_HEAVY_SLAM,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SAND_STREAM, // Sturdy + Sand Veil now innate; chosen Sand Stream (override) sets sand for Sand Veil
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0233 (Eviolite NFE niche: Porygon-Z is a glass cannon, Porygon2 the bulky tank)
    {
        .species = SPECIES_PORYGON2,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE, // Trace bulky tank / recovery (Porygon2 NFE niche)
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
        .teraType = TYPE_FAIRY,
    },
    {
        .species = SPECIES_PORYGON2,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_EVIOLITE, // Download offensive Eviolite pivot (Download now innate)
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_RECOVER
        },
        .ability = ABILITY_TRACE, // Download & Analytic now innate; chosen Trace copies a foe ability
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
        .heldItem = ITEM_FOCUS_BAND, // suicide lead, hazards + sleep (sash = one-shot guard)
        .moves =
        {
            MOVE_SPORE,
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_MOODY, // Own Tempo + Technician now innate; chosen Moody (real slot 2, :x: never-innate) stays observable
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
        .heldItem = ITEM_FOCUS_SASH, // doubles support lead (Spore + Fake Out)
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
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate doubles support attacker
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // Intimidate now innate; chosen No Guard (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Technician spinner / pivot
        .moves =
        {
            MOVE_RAPID_SPIN,
            MOVE_CLOSE_COMBAT,
            MOVE_MACH_PUNCH,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_NO_GUARD, // Technician + Intimidate now innate; chosen No Guard (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // bulky physical wall / cleric
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_MILK_DRINK,
            MOVE_HEAL_BELL,
            MOVE_STEALTH_ROCK
        },
        // Thick Fat now innate; chosen Sap Sipper (HA) absorbs Grass moves for an Attack boost.
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
        .heldItem = ITEM_CHOICE_BAND, // Scrappy band breaker
        .moves =
        {
            MOVE_DOUBLE_EDGE,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SAP_SIPPER, // Thick Fat & Scrappy now innate; chosen Sap Sipper (its real HA) turns Grass hits into Attack boosts
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
        .heldItem = ITEM_LEFTOVERS, // special wall / cleric
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_HEAL_BELL,
            MOVE_TOXIC
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_ROCKY_HELMET, // stallbreaker support with hazards
        .moves =
        {
            MOVE_SEISMIC_TOSS,
            MOVE_SOFT_BOILED,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 4,
            .def = 252,
            .spd = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0243
    {
        .species = SPECIES_RAIKOU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // calm mind special sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_THUNDERBOLT,
            MOVE_AURA_SPHERE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD, // Inner Focus now innate; chosen Lightning Rod (override) draws Electric + SpA
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
        .heldItem = ITEM_CHOICE_SPECS, // specs special breaker / pivot
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD, // Inner Focus now innate; chosen Lightning Rod (override) draws Electric + SpA
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
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE,
            MOVE_WEATHER_BALL
        },
        .ability = ABILITY_LIGHTNING_ROD, // Inner Focus now innate; chosen Lightning Rod (override) draws Electric + SpA
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
        .heldItem = ITEM_CHOICE_BAND, // Sacred Fire band breaker w/ priority
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_FLAME_BODY, // Inner Focus now innate; chosen Flame Body (override) burns contact attackers
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
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_BULLDOZE
        },
        .ability = ABILITY_FLAME_BODY, // Inner Focus now innate; chosen Flame Body (override) burns contact attackers
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_EXTREME_SPEED,
            MOVE_STONE_EDGE,
            MOVE_MORNING_SUN
        },
        .ability = ABILITY_FLAME_BODY, // Inner Focus now innate; chosen Flame Body (override) burns contact attackers
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
        .heldItem = ITEM_LEFTOVERS, // calm mind bulky sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB, // Inner Focus now innate; chosen Water Absorb (override) heals on Water hits
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
        .heldItem = ITEM_LIFE_ORB, // offensive calm mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_WATER_ABSORB, // Inner Focus now innate; chosen Water Absorb (override) heals on Water hits
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
        .heldItem = ITEM_ROCKY_HELMET, // bulky defogger / wall
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_DEFOG,
            MOVE_REST,
            MOVE_SLEEP_TALK
        },
        .ability = ABILITY_WATER_ABSORB, // Inner Focus now innate; chosen Water Absorb (override) heals on Water hits
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
        .heldItem = ITEM_SMOOTH_ROCK, // Mega Tyranitar (Sand Stream); extends its own sandstorm
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
        .heldItem = ITEM_CHOICE_BAND, // Sand Stream band breaker
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
        .heldItem = ITEM_LEFTOVERS, // sand tank with hazards
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank in sand
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_CRUNCH,
            MOVE_FIRE_BLAST,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_ROCK,
    },

    // 0249
    {
        .species = SPECIES_LUGIA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Multiscale physical wall / defogger
        .moves =
        {
            MOVE_AEROBLAST,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_STORM_DRAIN, // Multiscale now innate; chosen Storm Drain (override) gives the sea guardian a Water immunity + SpA boost
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // calm mind bulky sweeper
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
        .heldItem = ITEM_CHOICE_BAND, // Sacred Fire band breaker
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_BRAVE_BIRD,
            MOVE_EARTHQUAKE,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_FLAME_BODY, // Pressure + Regenerator both now innate; chosen Flame Body burns on contact (slot-1 override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot / cleric
        .moves =
        {
            MOVE_SACRED_FIRE,
            MOVE_BRAVE_BIRD,
            MOVE_RECOVER,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_FLAME_BODY, // Pressure + Regenerator both now innate; chosen Flame Body burns on contact (slot-1 override)
        .nature = NATURE(SPD_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spd = 252
        ),
        .teraType = TYPE_FIRE,
    },

    // 0251 (innate Levitate — Ground immune, never give an Air Balloon)
    {
        .species = SPECIES_CELEBI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // nasty plot special sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_GRASSY_SURGE, // Natural Cure now innate; chosen Grassy Surge via override (forest guardian: heals + boosts Grass)
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with utility
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_RECOVER,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE, // Natural Cure now innate; chosen Grassy Surge via override (forest guardian: heals + boosts Grass)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // calm mind bulky sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_RECOVER
        },
        .ability = ABILITY_GRASSY_SURGE, // Natural Cure now innate; chosen Grassy Surge via override (forest guardian: heals + boosts Grass)
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
        .heldItem = ITEM_MIRACLE_SEED, // Mega Sceptile (Lightning Rod); Grass STAB boost
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_LIGHTNING_ROD, // Overgrow now innate (latched); chosen Lightning Rod via override (matches its Mega's ability)
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
        .heldItem = ITEM_LIFE_ORB, // physical Swords Dance sweeper (no mega)
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW
        },
        .ability = ABILITY_LIGHTNING_ROD, // Overgrow now innate (latched); chosen Lightning Rod via override (its Mega's ability)
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
        .heldItem = ITEM_CHOICE_SPECS, // Overgrow special revenge killer
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_DRAGON_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_LIGHTNING_ROD, // Overgrow now innate (latched); chosen Lightning Rod via override (its Mega's ability)
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
        .heldItem = ITEM_LIFE_ORB, // Speed Boost (now innate) snowballing sweeper; chosen Sheer Force adds power
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_JUMP_KICK,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Blaze + Speed Boost both now innate; chosen Sheer Force (override)
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
        .heldItem = ITEM_LIFE_ORB, // Speed Boost (now innate) mixed wallbreaker
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SHEER_FORCE, // Blaze + Speed Boost both now innate; chosen Sheer Force (override)
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
        .heldItem = ITEM_CHOICE_BAND, // immediate banded breaker
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Blaze + Speed Boost both now innate; chosen Sheer Force (override)
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
        .heldItem = ITEM_MYSTIC_WATER, // Mega Swampert (Swift Swim); Water STAB boost for the rain sweeper
        .moves =
        {
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_DAMP, // Torrent now innate (latched); chosen Damp
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
        .heldItem = ITEM_LEFTOVERS, // bulky hazard setter
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_CHILLING_WATER,
            MOVE_EARTHQUAKE,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_DAMP, // Torrent now innate (latched); chosen Damp
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
        .heldItem = ITEM_ASSAULT_VEST, // mixed-tank pivot
        .moves =
        {
            MOVE_FLIP_TURN,
            MOVE_EARTHQUAKE,
            MOVE_ICE_BEAM,
            MOVE_POWER_GEM
        },
        .ability = ABILITY_DAMP, // Torrent now innate (latched); chosen Damp
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
        .heldItem = ITEM_FLAME_ORB, // Quick Feet status-fueled attacker
        .moves =
        {
            MOVE_FACADE,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE, // Quick Feet + Moxie now innate; chosen Sheer Force (slot 2)
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
        .heldItem = ITEM_CHOICE_BAND, // Intimidate hit-and-run band
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_SUCKER_PUNCH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (slot 2)
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
        .heldItem = ITEM_SITRUS_BERRY, // Gluttony Belly Drum + Extreme Speed sweeper
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_EXTREME_SPEED,
            MOVE_SEED_BOMB,
            MOVE_KNOCK_OFF
        },
        // Pickup/Gluttony/Quick Feet all now innate; chosen Scrappy lets its Normal STAB hit Ghosts (fork override).
        .ability = ABILITY_SCRAPPY,
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
        .heldItem = ITEM_FLAME_ORB, // Quick Feet Flame Orb priority breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_EXTREME_SPEED,
            MOVE_KNOCK_OFF,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_SCRAPPY, // Pickup/Gluttony/Quick Feet now innate; chosen Scrappy (Facade hits Ghosts)
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
        .heldItem = ITEM_SITRUS_BERRY, // Quiver Dance flavor sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY, // Swarm now innate (latched); chosen Rivalry
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .def = 4,
            .spa = 252,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0269
    {
        .species = SPECIES_DUSTOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // bulky flavor wall
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_SLUDGE_BOMB,
            MOVE_ROOST,
            MOVE_TOXIC
        },
        .ability = ABILITY_POISON_POINT, // Shield Dust & Compound Eyes now innate; chosen Poison Point (empty-slot override) poisons the contact Rocky Helmet punishes
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0272
    {
        .species = SPECIES_LUDICOLO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_GIGA_DRAIN,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_STORM_DRAIN, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LEFTOVERS, // Rain Dish bulky pivot
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_GIGA_DRAIN,
            MOVE_LEECH_SEED,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_STORM_DRAIN, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_MYSTIC_WATER, // Water STAB boost for the rain sweeper
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ENERGY_BALL,
            MOVE_ICE_BEAM,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_STORM_DRAIN, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_HEAT_WAVE
        },
        .ability = ABILITY_WIND_RIDER, // Chlorophyll now innate; chosen Wind Rider
        .nature = NATURE(ATK_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_SHIFTRY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // Sticky Web lead, sash = one-shot entry guard
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_DEFOG
        },
        .ability = ABILITY_WIND_RIDER, // Chlorophyll now innate; chosen Wind Rider
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
        .heldItem = ITEM_CHOICE_BAND, // Swords-less band breaker
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_LEAF_BLADE,
            MOVE_SUCKER_PUNCH,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_WIND_RIDER, // Chlorophyll now innate; chosen Wind Rider
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
        .heldItem = ITEM_TOXIC_ORB, // Guts Facade sweeper
        .moves =
        {
            MOVE_FACADE,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_QUICK_FEET, // Guts & Scrappy now innate; chosen Quick Feet (empty-slot override) — the Toxic Orb also buys +50% Speed
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
        .heldItem = ITEM_CHOICE_BAND, // Scrappy-less band hit-and-run
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_FACADE,
            MOVE_U_TURN,
            MOVE_STEEL_WING
        },
        .ability = ABILITY_QUICK_FEET, // Guts & Scrappy now innate; chosen Quick Feet (empty-slot override)
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
        .heldItem = ITEM_DAMP_ROCK, // Drizzle rain setter
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Defog pivot
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
        .heldItem = ITEM_THROAT_SPRAY, // Mega Gardevoir (Pixilate); Hyper Voice is a sound move -> +SpAtk
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
        .heldItem = ITEM_CHOICE_SCARF, // Trace revenge killer (no mega)
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
        .heldItem = ITEM_LIFE_ORB, // Calm Mind setup sweeper
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
        .heldItem = ITEM_FOCUS_BAND, // Quiver Dance sweeper, sash = one-shot entry guard
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_HYDRO_PUMP
        },
        .ability = ABILITY_STORM_DRAIN, // Intimidate now innate; chosen Storm Drain (slot 1)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0286
    {
        .species = SPECIES_BRELOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Technician-less Spore + priority breaker
        .moves =
        {
            MOVE_SPORE,
            MOVE_BULLET_SEED,
            MOVE_MACH_PUNCH,
            MOVE_ROCK_TOMB
        },
        .ability = ABILITY_HUSTLE, // moved off Effect Spore (redundant deterministic sleep vs Spore); +50% Atk, Poison Heal/Technician still innate
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
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal Sub-Punch staller
        .moves =
        {
            MOVE_SPORE,
            MOVE_SUBSTITUTE,
            MOVE_FOCUS_PUNCH,
            MOVE_SEED_BOMB
        },
        .ability = ABILITY_HUSTLE, // moved off Effect Spore (redundant deterministic sleep vs Spore); +50% Atk, Poison Heal still innate (Toxic Orb)
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
        .heldItem = ITEM_FOCUS_BAND, // Spore lead, sash = one-shot entry guard
        .moves =
        {
            MOVE_SPORE,
            MOVE_BULLET_SEED,
            MOVE_MACH_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_HUSTLE, // moved off Effect Spore (redundant deterministic sleep vs Spore); +50% Atk, Poison Heal/Technician still innate
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
        .heldItem = ITEM_CHOICE_BAND, // Truant banded wallbreaker
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
        .heldItem = ITEM_LIFE_ORB, // coverage breaker
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
        .heldItem = ITEM_SHELL_BELL, // monstrous Attack -> each hit banks a big 1/4 heal, and offsets Double-Edge recoil on the Truant loaf turn
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
        .heldItem = ITEM_LIFE_ORB, // Speed Boost (now innate) fast sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_X_SCISSOR,
            MOVE_AERIAL_ACE,
            MOVE_DIG
        },
        .ability = ABILITY_TOUGH_CLAWS, // Speed Boost + Infiltrator now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_FOCUS_BAND, // baton-pass-style speed control lead (Speed Boost now innate)
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SUBSTITUTE,
            MOVE_X_SCISSOR,
            MOVE_PROTECT
        },
        .ability = ABILITY_TOUGH_CLAWS, // Speed Boost + Infiltrator now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Wonder Guard sweeper (boots dodge hazards)
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
        .heldItem = ITEM_CHOICE_SPECS, // Boomburst spam breaker
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SOUNDPROOF, // Scrappy now innate; chosen Soundproof (its real slot 0) blocks opposing sound moves
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
        .heldItem = ITEM_LIFE_ORB, // mixed sound attacker
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_OVERHEAT,
            MOVE_SURF,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_SOUNDPROOF, // Scrappy now innate; chosen Soundproof
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
        .heldItem = ITEM_FLAME_ORB, // Guts status-fueled bruiser
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_FAKE_OUT
        },
        .ability = ABILITY_SHEER_FORCE, // Guts + Thick Fat now innate; Sheer Force powers its moves
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
        .heldItem = ITEM_ASSAULT_VEST, // Thick Fat special tank
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_HEAVY_SLAM,
            MOVE_BULLET_PUNCH
        },
        // Thick Fat now innate; chosen Sheer Force (HA) powers up this Assault Vest attacker.
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
        .heldItem = ITEM_SILK_SCARF, // Normalized everything
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
        .heldItem = ITEM_LEFTOVERS, // Mega Sableye (Magic Bounce); recovery for the defensive pivot
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DARK_PULSE,
            MOVE_RECOVER,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MAGIC_BOUNCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LEFTOVERS, // Prankster utility staller (no mega)
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_RECOVER,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_MAGIC_BOUNCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_FAIRY_FEATHER, // Mega Mawile (Huge Power); Fairy STAB boost for Play Rough
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_PLAY_ROUGH,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (real slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Intimidate utility setter (no mega)
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_PLAY_ROUGH,
            MOVE_IRON_HEAD,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (real slot 2)
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
        .heldItem = ITEM_ROCKY_HELMET, // Mega Aggron (Filter); chips the physical attackers it walls
        .moves =
        {
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK,
            MOVE_ROAR
        },
        .ability = ABILITY_FILTER, // Sturdy + Rock Head + Heavy Metal now innate; chosen Filter (its Mega's ability) blunts weaknesses (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Rock Head band breaker (no mega)
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_AVALANCHE
        },
        .ability = ABILITY_FILTER, // Sturdy + Rock Head + Heavy Metal now innate; chosen Filter (its Mega's ability) blunts weaknesses (override)
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Sturdy bait setup tank (no mega)
        .moves =
        {
            MOVE_AUTOTOMIZE,
            MOVE_HEAVY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_FILTER, // Sturdy + Rock Head + Heavy Metal now innate; chosen Filter (its Mega's ability) blunts weaknesses (override)
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
        .heldItem = ITEM_BLACK_BELT, // Mega Medicham (Pure Power); Fighting STAB boost
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_FAKE_OUT
        },
        .ability = ABILITY_RECKLESS, // Pure Power now innate; chosen Reckless (override slot 1) boosts High Jump Kick
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
        .heldItem = ITEM_CHOICE_SCARF, // Pure Power revenge killer (no mega)
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_TRICK
        },
        .ability = ABILITY_RECKLESS, // Pure Power now innate; chosen Reckless (override slot 1) boosts High Jump Kick
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
        .heldItem = ITEM_MAGNET, // Mega Manectric (Intimidate); Electric STAB boost
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
        .heldItem = ITEM_CHOICE_SPECS, // Lightning Rod special breaker (no mega)
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

    // 0311 (doubles support)
    {
        .species = SPECIES_PLUSLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Plus partner special attacker
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

    // 0312 (doubles support)
    {
        .species = SPECIES_MINUN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Minus support pivot
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

    // 0313 (doubles support)
    {
        .species = SPECIES_VOLBEAT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_DAMP_ROCK, // Prankster Tailwind/Rain support
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_RAIN_DANCE,
            MOVE_THUNDER_WAVE,
            MOVE_U_TURN
        },
        .ability = ABILITY_VICTORY_STAR, // Illuminate + Prankster + Swarm now all innate; chosen Victory Star (fork override) boosts ally accuracy for this doubles supporter
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 248,
            .spd = 252,
            .spe = 8
        ),
        .teraType = TYPE_BUG,
    },

    // 0314 (doubles support)
    {
        .species = SPECIES_ILLUMISE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB, // Prankster utility setter
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_ENCORE,
            MOVE_HELPING_HAND,
            MOVE_BUG_BUZZ
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_BLACK_SLUDGE, // Sticky Hold status tank
        .moves =
        {
            MOVE_SLUDGE_BOMB,
            MOVE_TOXIC,
            MOVE_PAIN_SPLIT,
            MOVE_ENCORE
        },
        .ability = ABILITY_POISON_TOUCH, // Liquid Ooze / Sticky Hold / Gluttony all innate; chosen Poison Touch (override)
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
        .heldItem = ITEM_BLACK_GLASSES, // Mega Sharpedo (Strong Jaw); Dark STAB boost (Speed Boost now innate)
        .moves =
        {
            MOVE_PROTECT,
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_STRONG_JAW, // Rough Skin & Speed Boost now innate; chosen Strong Jaw (slot-1 override) powers the biting STAB
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
        .heldItem = ITEM_LIFE_ORB, // Speed Boost (now innate) wallbreaker (no mega)
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_STRONG_JAW, // Rough Skin & Speed Boost now innate; chosen Strong Jaw (slot-1 override) powers the biting STAB
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
        .heldItem = ITEM_CHOICE_SCARF, // rough-skin revenge killer
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_WATERFALL,
            MOVE_CLOSE_COMBAT,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_STRONG_JAW, // Rough Skin & Speed Boost now innate; chosen Strong Jaw (slot-1 override)
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
        .heldItem = ITEM_CHOICE_SPECS, // Water Spout cannon
        .moves =
        {
            MOVE_WATER_SPOUT,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_HYPER_VOICE
        },
        .ability = ABILITY_DRIZZLE, // all real abilities innate; chosen Drizzle (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Mega Camerupt (Sheer Force); Sheer Force negates the Life Orb recoil
        .moves =
        {
            MOVE_ERUPTION,
            MOVE_EARTH_POWER,
            MOVE_FIRE_BLAST,
            MOVE_ANCIENT_POWER
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_CAMERUPT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Solid Rock special wall (no mega)
        .moves =
        {
            MOVE_LAVA_PLUME,
            MOVE_EARTH_POWER,
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_HEAT_ROCK, // Drought sun setter
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
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
    },
    {
        .species = SPECIES_TORKOAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // White Smoke defensive setter
        .moves =
        {
            MOVE_LAVA_PLUME,
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_YAWN
        },
        .ability = ABILITY_DROUGHT, // White Smoke now innate (Shell Armor too); chosen Drought sets the sun
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
        .heldItem = ITEM_LEFTOVERS, // Thick Fat Calm Mind tank
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_REST
        },
        // Thick Fat/Own Tempo/Gluttony ALL now innate; chosen Synchronize (fork override, slot 1) stays observable.
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
        .heldItem = ITEM_SITRUS_BERRY, // Contrary Superpower flavor
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

    // 0330 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Levitate banded pivot
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_U_TURN,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM, // Levitate now innate; chosen Sand Stream (desert spirit; Ground-type takes no sand chip)
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
        .heldItem = ITEM_CHOICE_SCARF, // fast revenge killer
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_U_TURN,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM, // Levitate now innate; chosen Sand Stream (desert spirit; Ground-type takes no sand chip)
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
        .heldItem = ITEM_SOFT_SAND, // Dragon Dance setup sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SAND_STREAM, // Levitate now innate; chosen Sand Stream (desert spirit; Ground-type takes no sand chip)
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
        .heldItem = ITEM_LEFTOVERS, // defensive Defog pivot (innate/native Levitate)
        .moves =
        {
            MOVE_DEFOG,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN,
            MOVE_TOXIC
        },
        .ability = ABILITY_SAND_STREAM, // Levitate now innate; chosen Sand Stream (desert spirit; Ground-type takes no sand chip)
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance priority breaker
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
        .heldItem = ITEM_FOCUS_BAND, // Spikes + Destiny Bond lead, sash = one-shot guard
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

    // 0334 (Swablu line)
    {
        .species = SPECIES_ALTARIA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FAIRY_FEATHER, // Mega Altaria (Pixilate); boosts its -ate Fairy moves
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_RETURN,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_CLOUD_NINE, // Natural Cure now innate; chosen Cloud Nine (real slot 2; becomes Pixilate on Mega)
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
        .heldItem = ITEM_LEFTOVERS, // Natural Cure defensive pivot (no mega)
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_CLOUD_NINE, // Natural Cure now innate; chosen Cloud Nine (real slot 2) negates weather
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
        .heldItem = ITEM_TOXIC_ORB, // Toxic Boost Facade breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_SHEER_FORCE, // Toxic Boost now innate; chosen Sheer Force (override) powers its coverage
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE, // Toxic Boost now innate; chosen Sheer Force (override) skips Life Orb recoil
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
        .heldItem = ITEM_CHOICE_SPECS, // mixed special breaker
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_POISON_POINT, // Infiltrator now innate; chosen Poison Point punishes contact (override)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_POISON,
    },

    // 0337 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LUNATONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Levitate Cosmic Power / Trick Room attacker
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
        .heldItem = ITEM_LEFTOVERS, // Trick Room setter / Cosmic Power tank
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
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0338 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_SOLROCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Levitate physical setup attacker
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_STONE_EDGE,
            MOVE_ZEN_HEADBUTT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_DROUGHT, // Levitate now innate; chosen Drought (sun meteorite)
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
        .heldItem = ITEM_SITRUS_BERRY, // doubles support / Trick Room
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_STEALTH_ROCK,
            MOVE_HELPING_HAND,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_DROUGHT, // Levitate now innate; chosen Drought (sun meteorite)
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_ROCK,
    },

    // 0340
    {
        .species = SPECIES_WHISCASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Dragon Dance bulky sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_STORM_DRAIN, // Oblivious + Anticipation + Hydration all innate; chosen Storm Drain (override, slot 2)
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
        .heldItem = ITEM_LIFE_ORB, // Adaptability wallbreaker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SNIPER, // Hyper Cutter now innate (Shell Armor/Adaptability too); chosen Sniper via override pays off Crabhammer crits
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
        .heldItem = ITEM_CHOICE_BAND, // Adaptability band breaker
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SNIPER, // Hyper Cutter now innate (Shell Armor/Adaptability too); chosen Sniper via override pays off Crabhammer crits
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
        .heldItem = ITEM_SHELL_BELL, // Adaptability Crabhammer hits enormously hard; the 1/4 heal patches its glassy bulk between swings
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CRABHAMMER,
            MOVE_KNOCK_OFF,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SNIPER, // Hyper Cutter now innate (Shell Armor/Adaptability too); chosen Sniper via override pays off Crabhammer crits
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0344 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CLAYDOL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Levitate hazard setter / spinner
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_EARTH_POWER,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SAND_STREAM, // Levitate now innate; chosen Sand Stream (ancient desert clay; Ground-type takes no sand chip)
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
        .heldItem = ITEM_LEFTOVERS, // Storm Drain Curse setup tank
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance physical attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE,
            MOVE_X_SCISSOR,
            MOVE_AQUA_TAIL
        },
        .ability = ABILITY_WATER_ABSORB, // Battle Armor & Swift Swim now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_LEFTOVERS, // defensive Rapid Spin + Stealth Rock
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_RAPID_SPIN,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB, // Battle Armor & Swift Swim now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_LEFTOVERS, // Marvel Scale defensive pivot
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_ICE_BEAM,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_FLAME_ORB, // Marvel Scale flame-orb tank
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 248,
            .def = 252,
            .spd = 8
        ),
        .teraType = TYPE_WATER,
    },

    // 0351 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CASTFORM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Forecast Weather Ball attacker
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
        .heldItem = ITEM_ASSAULT_VEST, // Color Change special tank
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

    // 0354 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_BANETTE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SPELL_TAG, // Mega Banette (Prankster); Ghost STAB boost for the disruptor
        .moves =
        {
            MOVE_SHADOW_CLAW,
            MOVE_KNOCK_OFF,
            MOVE_WILL_O_WISP,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_WANDERING_SPIRIT, // Insomnia / Frisk / Cursed Body all innate; chosen Wandering Spirit (override)
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
        .heldItem = ITEM_FOCUS_BAND, // Destiny Bond trapper (no mega)
        .moves =
        {
            MOVE_SHADOW_CLAW,
            MOVE_SUCKER_PUNCH,
            MOVE_DESTINY_BOND,
            MOVE_TAUNT
        },
        .ability = ABILITY_WANDERING_SPIRIT, // Insomnia / Frisk / Cursed Body all innate; chosen Wandering Spirit (override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },

    // 0356 (Eviolite NFE wall — INNATE LEVITATE; Dusknoir intentionally omitted)
    {
        .species = SPECIES_DUSCLOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE, // Eviolite physical/special wall
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_NIGHT_SHADE,
            MOVE_PAIN_SPLIT,
            MOVE_HEX
        },
        .ability = ABILITY_MUMMY, // all real abilities innate; chosen Mummy (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Harvest Sitrus staller
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_SUBSTITUTE,
            MOVE_AIR_SLASH,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SOLAR_POWER, // Chlorophyll + Harvest now innate; chosen Solar Power (its real slot-1 HA, :x:)
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0358 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CHIMECHO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Levitate Calm Mind tank
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
        .heldItem = ITEM_LIFE_ORB, // Mega Absol (Magic Bounce); power for the Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_PLAY_ROUGH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_MAGIC_BOUNCE, // all real abilities innate; chosen Magic Bounce (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Super Luck band breaker (no mega)
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_SUCKER_PUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_PSYCHO_CUT
        },
        .ability = ABILITY_MAGIC_BOUNCE, // all real abilities innate; chosen Magic Bounce (non-redundant)
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
        .heldItem = ITEM_SCOPE_LENS, // Super Luck guaranteed-crit attacker
        .moves =
        {
            MOVE_NIGHT_SLASH,
            MOVE_PSYCHO_CUT,
            MOVE_SUCKER_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_MAGIC_BOUNCE, // all real abilities innate; chosen Magic Bounce (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0362 (Gen III mega OK — INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_GLALIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NEVER_MELT_ICE, // Mega Glalie (Refrigerate); boosts its -ate Ice Return
        .moves =
        {
            MOVE_RETURN,
            MOVE_ICICLE_CRASH,
            MOVE_EARTHQUAKE,
            MOVE_FREEZE_DRY
        },
        .ability = ABILITY_MOODY, // Inner Focus + Ice Body now innate; chosen Moody (real slot 2, :x: never-innate) stays observable
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
        .heldItem = ITEM_FOCUS_BAND, // Spikes + Explosion lead (no mega)
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICE_BEAM,
            MOVE_FREEZE_DRY,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_MOODY, // Inner Focus + Ice Body now innate; chosen Moody (real slot 2, :x: never-innate) stays observable
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
        .heldItem = ITEM_LEFTOVERS, // Thick Fat defensive staller
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_TOXIC,
            MOVE_PROTECT
        },
        // Thick Fat (and Oblivious) now innate; chosen Ice Body heals 1/16 HP each turn in snow.
        .ability = ABILITY_WATER_ABSORB, // Thick Fat + Ice Body + Oblivious all innate; chosen Water Absorb (override, slot 1)
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
        .heldItem = ITEM_ASSAULT_VEST, // Thick Fat special tank
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_FREEZE_DRY,
            MOVE_BODY_SLAM
        },
        // Thick Fat (and Oblivious) now innate; chosen Ice Body heals 1/16 HP each turn in snow.
        .ability = ABILITY_WATER_ABSORB, // Thick Fat + Ice Body + Oblivious all innate; chosen Water Absorb (override, slot 1)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0367 (Clamperl line)
    {
        .species = SPECIES_HUNTAIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash physical sweeper
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_WATERFALL,
            MOVE_CRUNCH,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB, // Water Veil now innate; chosen Water Absorb (override) heals on Water hits
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0368 (Clamperl line)
    {
        .species = SPECIES_GOREBYSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash special sweeper
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Hydration now innate; chosen Water Absorb (override, empty slot 1)
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Rock Head Rock Polish setup tank
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_HEAD_SMASH,
            MOVE_WATERFALL,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Rock Head + Sturdy now innate; chosen Water Absorb for bulk (override)
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
        .heldItem = ITEM_LEFTOVERS, // Sturdy defensive rocker + Yawn
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_YAWN,
            MOVE_WATERFALL,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Rock Head + Sturdy now innate; chosen Water Absorb for bulk (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // pure flavor pivot
        .moves =
        {
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_SWEET_KISS,
            MOVE_SOAK
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Hydration now innate; chosen Water Absorb (override, empty slot 1)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
        .ball = BALL_LOVE,
    },

    // 0373 (Bagon line)
    {
        .species = SPECIES_SALAMENCE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mega Salamence (Aerilate); power for the Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_DOUBLE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY, // Intimidate now innate; chosen Rivalry (slot 1)
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
        .heldItem = ITEM_CHOICE_SCARF, // Intimidate revenge killer (no mega)
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_RIVALRY, // Intimidate now innate; chosen Rivalry (slot 1)
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
        .heldItem = ITEM_LIFE_ORB, // mixed Draco breaker (no mega)
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_EARTHQUAKE,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY, // Intimidate now innate; chosen Rivalry (slot 1)
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0376 (Beldum line — INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_METAGROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mega Metagross (Tough Claws); power for the fast physical breaker
        .moves =
        {
            MOVE_METEOR_MASH,
            MOVE_ZEN_HEADBUTT,
            MOVE_EARTHQUAKE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_TOUGH_CLAWS, // Clear Body now innate; chosen Tough Claws via override powers its contact STAB
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
        .heldItem = ITEM_CHOICE_BAND, // Clear Body band breaker (no mega)
        .moves =
        {
            MOVE_METEOR_MASH,
            MOVE_BULLET_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_TOUGH_CLAWS, // Clear Body now innate; chosen Tough Claws via override powers its contact STAB
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
        .heldItem = ITEM_LEFTOVERS, // Agility setup / utility tank
        .moves =
        {
            MOVE_AGILITY,
            MOVE_METEOR_MASH,
            MOVE_EARTHQUAKE,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_TOUGH_CLAWS, // Clear Body now innate; chosen Tough Claws via override powers its contact STAB
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
        .heldItem = ITEM_LEFTOVERS, // Clear Body Curse setup wall
        .moves =
        {
            MOVE_CURSE,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate (Sturdy too); chosen Solid Rock via override blunts SE hits
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Sturdy bait setup tank
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HAMMER_ARM
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate (Sturdy too); chosen Solid Rock via override blunts SE hits
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
        .heldItem = ITEM_LEFTOVERS, // Clear Body special wall
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_THUNDER_WAVE,
            MOVE_REST
        },
        .ability = ABILITY_ICE_SCALES, // Clear Body now innate; chosen Ice Scales via override halves special damage
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_ICE_SCALES, // Clear Body now innate; chosen Ice Scales via override halves special damage
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
        .heldItem = ITEM_LEFTOVERS, // Clear Body Curse setup wall
        .moves =
        {
            MOVE_CURSE,
            MOVE_IRON_HEAD,
            MOVE_BODY_PRESS,
            MOVE_REST
        },
        .ability = ABILITY_BULLETPROOF, // Clear Body now innate; chosen Bulletproof via override deflects ball/bomb moves
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
        .heldItem = ITEM_LEFTOVERS, // defensive setter / pivot
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_IRON_HEAD,
            MOVE_THUNDER_WAVE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF, // Clear Body now innate; chosen Bulletproof via override deflects ball/bomb moves
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0380 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LATIAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Mega Latias; recovery for the bulky Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DRAGON_PULSE,
            MOVE_ROOST
        },
        .ability = ABILITY_ILLUSION, // Levitate now innate; chosen Illusion (the Eon refracts light to vanish)
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
        .heldItem = ITEM_LEFTOVERS, // defensive Levitate pivot (no mega)
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_HEALING_WISH
        },
        .ability = ABILITY_ILLUSION, // Levitate now innate; chosen Illusion (the Eon refracts light to vanish)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0381 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LATIOS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DRAGON_FANG, // Mega Latios; Dragon STAB boost for the fast special sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DRACO_METEOR,
            MOVE_ROOST
        },
        .ability = ABILITY_ILLUSION, // Levitate now innate; chosen Illusion (the Eon refracts light to vanish)
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
        .heldItem = ITEM_CHOICE_SPECS, // Levitate special breaker (no mega)
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_PSYCHIC,
            MOVE_AURA_SPHERE,
            MOVE_TRICK
        },
        .ability = ABILITY_ILLUSION, // Levitate now innate; chosen Illusion (the Eon refracts light to vanish)
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
        .heldItem = ITEM_BLUE_ORB, // Primal Kyogre (Primordial Sea) — rain nuke
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
        .heldItem = ITEM_CHOICE_SCARF, // Drizzle revenge killer (no primal)
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
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind + Rest (rain, no primal)
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
        .heldItem = ITEM_RED_ORB, // Primal Groudon (Desolate Land) — sun breaker
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
        .heldItem = ITEM_LEFTOVERS, // Drought bulky setter (no primal)
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

    // 0384 (Mega via Dragon Ascent + no item)
    {
        .species = SPECIES_RAYQUAZA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_NONE, // Mega Rayquaza (Delta Stream, via Dragon Ascent) — Dragon Dance sweeper
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
        .heldItem = ITEM_LIFE_ORB, // mixed Draco breaker (no Dragon Ascent / no mega)
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

    // 0385 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_JIRACHI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Serene Grace flinch revenge killer
        .moves =
        {
            MOVE_IRON_HEAD,
            MOVE_ZEN_HEADBUTT,
            MOVE_ICE_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_VICTORY_STAR, // Serene Grace now innate; chosen Victory Star (override, real slot empty)
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
        .heldItem = ITEM_LEFTOVERS, // Serene Grace Wish support / Iron Head flinch
        .moves =
        {
            MOVE_WISH,
            MOVE_IRON_HEAD,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_VICTORY_STAR, // Serene Grace now innate; chosen Victory Star (override, real slot empty)
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
        .heldItem = ITEM_LIFE_ORB, // Calm Mind special setup sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_VICTORY_STAR, // Serene Grace now innate; chosen Victory Star (override, real slot empty)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0386 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_ATTACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // glass-cannon mixed attacker
        .moves =
        {
            MOVE_PSYCHO_BOOST,
            MOVE_ICE_BEAM,
            MOVE_SUPERPOWER,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_TRACE, // Pressure now innate; chosen Trace copies the foe's ability (slot-1 override)
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
        .heldItem = ITEM_FOCUS_BAND, // sash glass cannon, one-shot entry guard
        .moves =
        {
            MOVE_PSYCHO_BOOST,
            MOVE_THUNDERBOLT,
            MOVE_ICE_BEAM,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_TRACE, // Pressure now innate; chosen Trace copies the foe's ability (slot-1 override)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0386 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_SPEED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // hyper-offense hazard lead, one-shot entry guard
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_TAUNT,
            MOVE_PSYCHO_BOOST
        },
        .ability = ABILITY_TRACE, // Pressure now innate; chosen Trace copies the foe's ability (slot-1 override)
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
        .heldItem = ITEM_LIFE_ORB, // fast Nasty Plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHO_BOOST,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_TRACE, // Pressure now innate; chosen Trace copies the foe's ability (slot-1 override)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0386 (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_DEFENSE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky hazard / status wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_TOXIC,
            MOVE_RECOVER
        },
        .ability = ABILITY_TRACE, // Pressure now innate; chosen Trace copies the foe's ability (slot-1 override)
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
        .heldItem = ITEM_LEFTOVERS, // bulky Rock Polish / Shell Smash setup
        .moves =
        {
            MOVE_ROCK_POLISH,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM, // Overgrow & Shell Armor now innate; chosen Sand Stream (override)
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
        .heldItem = ITEM_SITRUS_BERRY, // bulky hazards + Synthesis
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_SAND_STREAM, // Overgrow & Shell Armor now innate; chosen Sand Stream (override)
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
        .heldItem = ITEM_LIFE_ORB, // mixed wallbreaker
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_GUNK_SHOT,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_FOCUS_SASH, // fast suicide lead
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_FAKE_OUT,
            MOVE_CLOSE_COMBAT,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_OVERHEAT,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // bulky special pivot
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_FLASH_CANNON,
            MOVE_ROOST,
            MOVE_DEFOG
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_SPECS, // Torrent special breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_FLASH_CANNON,
            MOVE_ICE_BEAM,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Reckless band
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_DOUBLE_EDGE,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN
        },
        .ability = ABILITY_HUSTLE, // Reckless + Intimidate now innate; chosen Hustle (slot 1)
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
        .heldItem = ITEM_FLYING_GEM, // one-shot Flying burst nuke
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_QUICK_ATTACK,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_HUSTLE, // Reckless + Intimidate now innate; chosen Hustle (slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Unaware wall
        .moves =
        {
            MOVE_WATERFALL,
            MOVE_BODY_SLAM,
            MOVE_YAWN,
            MOVE_ROOST
        },
        .ability = ABILITY_MOODY, // Unaware now innate; chosen Moody slowly snowballs the bulky wall
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
        .heldItem = ITEM_WIDE_LENS, // Technician Fury Cutter ramp
        .moves =
        {
            MOVE_FURY_CUTTER,
            MOVE_BUG_BITE,
            MOVE_POUNCE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE, // Swarm + Technician now innate; chosen Sheer Force (override) powers Pounce
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
        .heldItem = ITEM_LIFE_ORB, // Intimidate physical attacker
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_CRUNCH,
            MOVE_SUPERPOWER,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_RIVALRY, // Intimidate now innate; chosen Rivalry (real slot 0)
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
        .heldItem = ITEM_CHOICE_BAND, // Guts band
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_CRUNCH,
            MOVE_SUPERPOWER,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_RIVALRY, // Guts + Intimidate now innate; chosen Rivalry (real slot 0)
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
        .heldItem = ITEM_LIFE_ORB, // special attacker
        .moves =
        {
            MOVE_LEAF_STORM,
            MOVE_SLUDGE_BOMB,
            MOVE_SLEEP_POWDER,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_POISON_POINT, // Natural Cure now innate; chosen Poison Point (real slot 1) chips contact attackers
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
        .heldItem = ITEM_LEFTOVERS, // Technician status spreader
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB,
            MOVE_SPIKES,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_POISON_POINT, // Natural Cure + Technician now innate; chosen Poison Point punishes contact on the status spreader
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
        .heldItem = ITEM_CHOICE_SCARF, // Mold Breaker glass cannon
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE,
            MOVE_CLOSE_COMBAT,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_SHEER_FORCE, // Mold Breaker now innate; chosen Sheer Force (its real HA, :x: stable) powers this glass cannon's coverage
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
        .heldItem = ITEM_ROCK_GEM, // Sheer Force Swords Dance nuke
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
        .heldItem = ITEM_LEFTOVERS, // physically defensive wall
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

    // 0416
    {
        .species = SPECIES_VESPIQUEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Pressure stall / defensive pivot
        .moves =
        {
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_WATER_ABSORB, // moved off Effect Spore (deterministic sleep pre-empted its Toxic); Water immunity + recovery for the staller
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
        .heldItem = ITEM_SITRUS_BERRY, // Follow Me redirection support
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
        .heldItem = ITEM_LIFE_ORB, // Swift Swim / fast attacker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_WATER_ABSORB, // Water Veil now innate; chosen Water Absorb (override) heals on Water hits
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
        .heldItem = ITEM_CHOICE_BAND, // Water Veil band breaker
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_ICE_PUNCH,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Water Veil now innate; chosen Water Absorb (override) heals on Water hits
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_WATER,
    },

    // 0423
    {
        .species = SPECIES_GASTRODON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Storm Drain special tank
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
        .heldItem = ITEM_ASSAULT_VEST, // Sticky Hold special tank
        .moves =
        {
            MOVE_EARTH_POWER,
            MOVE_CHILLING_WATER,
            MOVE_ICE_BEAM,
            MOVE_CLEAR_SMOG
        },
        .ability = ABILITY_STORM_DRAIN, // Sticky Hold + Sand Force now innate; chosen Storm Drain is its signature
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
        .heldItem = ITEM_SILK_SCARF, // Technician Fake Out pivot
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_DOUBLE_HIT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Technician + Skill Link + Prankster now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves =
        {
            MOVE_DOUBLE_HIT,
            MOVE_KNOCK_OFF,
            MOVE_LOW_KICK,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_TOUGH_CLAWS, // Technician + Skill Link + Prankster now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_QUICK_CLAW, // Unburden sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SHADOW_BALL,
            MOVE_AIR_SLASH,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_UNAWARE, // Aftermath/Unburden/Flare Boost all now innate; chosen Unaware (slot-0 override) walls the Calm Mind set
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
        .heldItem = ITEM_FLAME_ORB, // Flare Boost special attacker
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_AIR_SLASH,
            MOVE_HEX,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_UNAWARE, // Aftermath/Flare Boost/Unburden all now innate; chosen Unaware (slot-0 override) ignores the foe's boosts
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
        .heldItem = ITEM_LIFE_ORB, // Mega Lopunny (Scrappy); power for the fast breaker
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_HIGH_JUMP_KICK,
            MOVE_RETURN,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Cute Charm + Limber now both innate; chosen Sheer Force (override) powers Fake Out / Ice Punch
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
        .heldItem = ITEM_TOXIC_ORB, // Klutz Switcheroo / status spreader
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot special sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_WANDERING_SPIRIT, // Levitate now innate; chosen Wandering Spirit (roaming ghost swaps abilities on contact)
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GHOST,
    },
    {
        .species = SPECIES_MISMAGIUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_COLBUR_BERRY, // utility / Taunt + Will-O-Wisp
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_WILL_O_WISP,
            MOVE_TAUNT,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_WANDERING_SPIRIT, // Levitate now innate; chosen Wandering Spirit (roaming ghost swaps abilities on contact)
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
        .heldItem = ITEM_LIFE_ORB, // Moxie sweeper
        .moves =
        {
            MOVE_SUCKER_PUNCH,
            MOVE_BRAVE_BIRD,
            MOVE_HEAT_WAVE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_SNIPER, // all real abilities innate; chosen Sniper (non-redundant)
        .nature = NATURE(ATK_UP, SPA_DOWN),
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
        .heldItem = ITEM_CHOICE_BAND, // Super Luck band (guaranteed crits on crit moves)
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_SUCKER_PUNCH,
            MOVE_NIGHT_SLASH,
            MOVE_PSYCHO_CUT
        },
        .ability = ABILITY_SNIPER, // all real abilities innate; chosen Sniper (non-redundant)
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
        .heldItem = ITEM_SILK_SCARF, // Fake Out + Facade attacker
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_PLAY_ROUGH
        },
        // Thick Fat now innate; chosen Defiant (HA) punishes stat drops with a +2 Attack boost.
        .ability = ABILITY_HUSTLE, // all real abilities innate; chosen Hustle (non-redundant)
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
        .heldItem = ITEM_BLACK_GLASSES, // Aftermath pivot
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_CRUNCH,
            MOVE_FIRE_BLAST,
            MOVE_PURSUIT
        },
        .ability = ABILITY_POISON_TOUCH, // Aftermath/Stench/Keen Eye all now innate; chosen Poison Touch (slot-1 override) poisons via Gunk Shot
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
        .heldItem = ITEM_LEFTOVERS, // dual-screen / Trick Room setter
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_STEALTH_ROCK,
            MOVE_GYRO_BALL,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SOUNDPROOF, // Heatproof (+ Levitate) now innate; chosen Soundproof (override) makes the bell immune to sound moves
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room / Gyro Ball
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_BRONZONG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY, // Heatproof screens wall
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
        .heldItem = ITEM_THROAT_SPRAY, // Boomburst spam
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_HEAT_WAVE,
            MOVE_HURRICANE,
            MOVE_U_TURN
        },
        .ability = ABILITY_PUNK_ROCK, // Big Pecks now innate (Keen Eye/Tangled Feet too); chosen Punk Rock via override powers Boomburst
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
        .heldItem = ITEM_LEFTOVERS, // Infiltrator Calm Mind wall
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DARK_PULSE,
            MOVE_SHADOW_BALL,
            MOVE_REST
        },
        .ability = ABILITY_UNAWARE, // Pressure + Infiltrator now innate; chosen Unaware ignores the foe's boosts (override)
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
        .heldItem = ITEM_BLACK_GLASSES, // Pressure / Pursuit trapper
        .moves =
        {
            MOVE_SUCKER_PUNCH,
            MOVE_FOUL_PLAY,
            MOVE_WILL_O_WISP,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_UNAWARE, // Pressure + Infiltrator now innate; chosen Unaware ignores the foe's boosts (override)
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
        .heldItem = ITEM_SOFT_SAND, // Mega Garchomp (Sand Force); Ground STAB boost
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_BLAST,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_SAND_STREAM, // Rough Skin & Sand Veil now innate; chosen Sand Stream (slot-1 override) turns on innate Sand Veil
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_SAND_STREAM, // Rough Skin & Sand Veil now innate; chosen Sand Stream (slot-1 override) turns on innate Sand Veil
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
        .heldItem = ITEM_ROCKY_HELMET, // Rough Skin hazards lead
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_TAIL,
            MOVE_SPIKES
        },
        .ability = ABILITY_SAND_STREAM, // Rough Skin & Sand Veil now innate; chosen Sand Stream (slot-1 override) turns on innate Sand Veil
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
        .heldItem = ITEM_LIFE_ORB, // Mega Lucario (Adaptability); power for the mixed breaker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_METEOR_MASH,
            MOVE_CLOSE_COMBAT,
            MOVE_BULLET_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot special
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_AURA_SPHERE,
            MOVE_FLASH_CANNON,
            MOVE_VACUUM_WAVE
        },
        .ability = ABILITY_STEADFAST, // Inner Focus + Justified now innate; chosen Steadfast (real slot 0, not carried innately) stays observable
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
        .heldItem = ITEM_FIGHTING_GEM, // one-shot Fighting burst after Swords Dance
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_BULLET_PUNCH,
            MOVE_EXTREME_SPEED
        },
        .ability = ABILITY_STEADFAST,
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
        .heldItem = ITEM_LEFTOVERS, // Sand Stream physical wall
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
        .heldItem = ITEM_SMOOTH_ROCK, // sand setter + Slack Off
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
        .heldItem = ITEM_LEFTOVERS, // Sniper / Knock Off pivot
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_EARTHQUAKE,
            MOVE_TAUNT
        },
        .ability = ABILITY_BATTLE_ARMOR, // all real abilities innate; chosen Battle Armor (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_AQUA_TAIL
        },
        .ability = ABILITY_BATTLE_ARMOR, // all real abilities innate; chosen Battle Armor (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Dry Skin rain sweeper
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
        .heldItem = ITEM_BLACK_SLUDGE, // Poison Touch bulk
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance grass attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_CHLOROPHYLL,
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Defog/U-turn utility
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
        .heldItem = ITEM_ICY_ROCK, // Mega Abomasnow (Snow Warning); extends its own snow
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
        .heldItem = ITEM_ICY_ROCK, // extends its own snow for Aurora Veil + Blizzard
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
        .ability = ABILITY_TOUGH_CLAWS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0462
    {
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Magnet Pull trapper
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_LIGHTNING_ROD, // Magnet Pull/Levitate/Sturdy/Analytic now innate; chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FAIRY,
    },
    {
        // Innate Levitate already dodges Ground, so no Air Balloon needed — run a
        // bulky Analytic special tank instead.
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Analytic special tank
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_LIGHTNING_ROD, // Magnet Pull/Levitate/Sturdy/Analytic now innate; chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0463
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Own Tempo Trick Room support
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_BODY_SLAM,
            MOVE_EXPLOSION,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_CLOUD_NINE, // Own Tempo now innate; chosen Cloud Nine (real slot) negates weather
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_NORMAL,
    },
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Wish + Protect cleric wall
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
        .heldItem = ITEM_LEFTOVERS, // bulky Lightning Rod rocker
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Solid Rock + WP sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_MEGAHORN
        },
        .ability = ABILITY_LIGHTNING_ROD, // Solid Rock now innate; chosen Lightning Rod redirects Electric + SpA boost
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator wall
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_LEECH_SEED,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SAP_SIPPER, // Regenerator now innate; chosen Sap Sipper (vine tangle drinks Grass energy for +Atk)
        .nature = NATURE(DEF_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_GRASS,
    },
    {
        .species = SPECIES_TANGROWTH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Regenerator physical tank
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SAP_SIPPER, // Regenerator now innate; chosen Sap Sipper (vine tangle drinks Grass energy for +Atk)
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
        .heldItem = ITEM_LIFE_ORB, // Motor Drive mixed sweeper
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_CROSS_CHOP
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
        .species = SPECIES_ELECTIVIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // bulky Vital Spirit pivot
        .moves =
        {
            MOVE_WILD_CHARGE,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE, // Vital Spirit now innate; chosen Motor Drive
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
        .heldItem = ITEM_CHOICE_SPECS, // Vital Spirit special breaker
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_OVERHEAT
        },
        .ability = ABILITY_FLAME_BODY, // Vital Spirit now innate; chosen Flame Body
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
        .heldItem = ITEM_CHOICE_SCARF, // Vital Spirit revenge killer
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_FLAME_BODY, // Vital Spirit now innate; chosen Flame Body
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
        .heldItem = ITEM_LEFTOVERS, // Serene Grace flinch / Nasty Plot
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_AIR_SLASH,
            MOVE_DAZZLING_GLEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_SITRUS_BERRY, // doubles support (Follow Me + Tailwind)
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_TAILWIND,
            MOVE_DAZZLING_GLEAM,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Tinted Lens breaker
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_GIGA_DRAIN,
            MOVE_PROTECT
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_FOCUS_BAND, // Speed Boost (now innate) sweeper
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_AIR_SLASH,
            MOVE_ANCIENT_POWER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_BUG,
    },

    // 0470
    {
        .species = SPECIES_LEAFEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll / Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_SAP_SIPPER, // Leaf Guard (x2) + Chlorophyll ALL now innate; chosen Sap Sipper (fork override, dup slot 1) stays observable
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0471
    {
        .species = SPECIES_GLACEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves =
        {
            MOVE_BLIZZARD,
            MOVE_FREEZE_DRY,
            MOVE_WATER_PULSE,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_SNOW_WARNING, // Snow Cloak + Ice Body all innate; chosen Snow Warning (override, slot 2) feeds innate Snow Cloak/Ice Body
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0472
    {
        .species = SPECIES_GLISCOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal stall
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_PROTECT,
            MOVE_TOXIC,
            MOVE_ROOST
        },
        .ability = ABILITY_UNAWARE, // chosen via fork override (species_ability_overrides.c)
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 244,
            .def = 248,
            .spe = 16
        ),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_GLISCOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_ROOST
        },
        .ability = ABILITY_UNAWARE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LIFE_ORB, // Thick Fat physical attacker
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICICLE_CRASH,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF
        },
        // All three real abilities (Oblivious/Snow Cloak/Thick Fat) now innate; chosen Snow Warning
        // (fork override) sets the snow that turns on its own innate Snow Cloak evasion.
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_ICICLE_CRASH,
            MOVE_ICE_SHARD,
            MOVE_SUPERPOWER
        },
        // All three real abilities (Oblivious/Snow Cloak/Thick Fat) now innate; chosen Snow Warning
        // (fork override) sets the snow that turns on its own innate Snow Cloak evasion.
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
        .heldItem = ITEM_LEFTOVERS, // Thick Fat tank rocker
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_ICE_SHARD,
            MOVE_KNOCK_OFF
        },
        // All three real abilities (Oblivious/Snow Cloak/Thick Fat) now innate; chosen Snow Warning
        // (fork override) sets the snow that turns on its own innate Snow Cloak evasion.
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
        .heldItem = ITEM_LIFE_ORB, // Adaptability Nasty Plot nuke
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_TRI_ATTACK,
            MOVE_DARK_PULSE,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SIMPLE, // Adaptability/Download/Analytic all now innate; slot-2 Analytic repurposed to chosen Simple (Nasty Plot synergy), innate Download intact
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
        .heldItem = ITEM_CHOICE_SPECS, // Download breaker
        .moves =
        {
            MOVE_TRI_ATTACK,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_TRICK
        },
        .ability = ABILITY_SIMPLE, // Adaptability/Download/Analytic all now innate; slot-2 Analytic repurposed to chosen Simple (Nasty Plot synergy), innate Download intact
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
        .heldItem = ITEM_LIFE_ORB, // Mega Gallade (Inner Focus); power for the Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHO_CUT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SUPER_LUCK, // all real abilities innate; chosen Super Luck (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Sharpness slicer
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SACRED_SWORD,
            MOVE_PSYCHO_CUT,
            MOVE_LEAF_BLADE
        },
        .ability = ABILITY_SUPER_LUCK, // all real abilities innate; chosen Super Luck (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Magnet Pull steel trapper / hazards
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_POWER_GEM,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_LIGHTNING_ROD, // Magnet Pull/Sturdy/Sand Force now innate; chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk on this special wall
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
        .heldItem = ITEM_LEFTOVERS, // bulky utility / Pain Split
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_WILL_O_WISP,
            MOVE_PAIN_SPLIT,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_MUMMY, // all real abilities innate; chosen Mummy (non-redundant)
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
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_POLTERGEIST,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_MUMMY, // all real abilities innate; chosen Mummy (non-redundant)
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GHOST,
    },

    // 0478
    {
        .species = SPECIES_FROSLASS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_BAND, // fast spikes / Destiny Bond lead
        .moves =
        {
            MOVE_SPIKES,
            MOVE_ICE_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_SNOW_WARNING, // Snow Cloak / Cursed Body both innate; chosen Snow Warning (override)
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
        .heldItem = ITEM_LIFE_ORB, // Snow Cloak offensive
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_THUNDERBOLT,
            MOVE_TAUNT
        },
        .ability = ABILITY_SNOW_WARNING, // Snow Cloak / Cursed Body both innate; chosen Snow Warning (override)
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
        .heldItem = ITEM_CHOICE_SPECS, // fire/electric breaker
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
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
        .heldItem = ITEM_LEFTOVERS, // bulky water/electric pivot
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge pivot
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
        .heldItem = ITEM_LIFE_ORB, // grass/electric attacker
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
        .heldItem = ITEM_SITRUS_BERRY, // Levitate, no Air Balloon
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
        .heldItem = ITEM_LEFTOVERS, // Levitate, no Air Balloon
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
        .heldItem = ITEM_SITRUS_BERRY, // Levitate, no Air Balloon
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
        .heldItem = ITEM_LEFTOVERS, // dual-screen / hazards wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_PSYCHIC,
            MOVE_YAWN,
            MOVE_U_TURN
        },
        .ability = ABILITY_TRACE, // Levitate now innate; chosen Trace (Being of Knowledge reads/copies the foe)
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
        .heldItem = ITEM_LIFE_ORB, // mixed pivot
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_ICE_BEAM,
            MOVE_U_TURN,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_MOODY, // Levitate now innate; chosen Moody (Being of Emotion's volatile moods)
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
        .heldItem = ITEM_FOCUS_BAND, // fast suicide lead
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_TAUNT,
            MOVE_PSYCHIC,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_VICTORY_STAR, // Levitate now innate; chosen Victory Star (Being of Willpower: will to win)
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_FIRE_BLAST,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_VICTORY_STAR, // Levitate now innate; chosen Victory Star (Being of Willpower: will to win)
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
        .heldItem = ITEM_LEFTOVERS, // bulky special legend
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF, // Pressure + Telepathy both now innate; chosen Bulletproof (fork override) deflects Focus Blast at its Fighting weakness
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
        .heldItem = ITEM_CHOICE_SPECS, // Roar of Time breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_FIRE_BLAST,
            MOVE_ROAR_OF_TIME
        },
        .ability = ABILITY_BULLETPROOF, // Pressure + Telepathy both now innate; chosen Bulletproof (fork override) for the armored Steel legend
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
        .heldItem = ITEM_ADAMANT_CRYSTAL, // forme-locked item
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_BULLETPROOF, // Pressure + Telepathy both now innate; chosen Bulletproof (fork override) for the armored Steel legend
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
        .heldItem = ITEM_LIFE_ORB, // fast special legend
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB, // Pressure + Telepathy both now innate; chosen Water Absorb (fork override) heals off Water for the space legend
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge legend
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_WATER_ABSORB, // Pressure + Telepathy both now innate; chosen Water Absorb (fork override) for the space legend
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
        .heldItem = ITEM_LUSTROUS_GLOBE, // forme-locked item
        .moves =
        {
            MOVE_SPACIAL_REND,
            MOVE_HYDRO_PUMP,
            MOVE_DRACO_METEOR,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_WATER_ABSORB, // Pressure + Telepathy both now innate; chosen Water Absorb (fork override) for the space legend
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
        .heldItem = ITEM_LEFTOVERS, // Flash Fire pivot / hazards
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
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
        .heldItem = ITEM_LEFTOVERS, // Slow Start sit-out with Substitute
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
        .heldItem = ITEM_LEFTOVERS, // bulky Will-O / Defog wall
        .moves =
        {
            MOVE_DRAGON_TAIL,
            MOVE_WILL_O_WISP,
            MOVE_REST,
            MOVE_DEFOG
        },
        .ability = ABILITY_UNAWARE, // Pressure + Telepathy both now innate; chosen Unaware (fork override) ignores setup on the bulky Renegade wall
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
        .heldItem = ITEM_GRISEOUS_ORB, // Origin forme nuke
        .moves =
        {
            MOVE_SHADOW_FORCE,
            MOVE_DRACO_METEOR,
            MOVE_EARTHQUAKE,
            MOVE_DRAGON_CLAW
        },
        .ability = ABILITY_DRAGONS_MAW, // Levitate now innate (Origin forme); chosen Dragon's Maw (the Renegade's draconic might)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind bulky sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_PSYSHOCK,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_CLOUD_NINE, // Levitate now innate; chosen Cloud Nine (serene lunar presence stills the weather)
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
        .heldItem = ITEM_SITRUS_BERRY, // doubles support
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HELPING_HAND,
            MOVE_ICY_WIND,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_CLOUD_NINE, // Levitate now innate; chosen Cloud Nine (serene lunar presence stills the weather)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind staller
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB, // Hydration now innate; chosen Water Absorb (override, empty slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Tail Glow sweeper
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_SURF,
            MOVE_ICE_BEAM,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_WATER_ABSORB, // Hydration now innate; chosen Water Absorb (override, empty slot 1)
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
        .heldItem = ITEM_MYSTIC_WATER, // hard-hitting setup
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_SURF
        },
        .ability = ABILITY_WATER_ABSORB, // Hydration now innate; chosen Water Absorb (override, empty slot 1)
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot / Dark Void sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_SLUDGE_BOMB,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_SHEER_FORCE, // Bad Dreams now innate; chosen Sheer Force (override, empty slot 1)
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
        .heldItem = ITEM_LIFE_ORB, // Serene Grace Air Slash flincher
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
        .heldItem = ITEM_LIFE_ORB, // Extreme Speed Swords Dance sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind special sweeper
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
        .heldItem = ITEM_CHARCOAL, // V-create physical wallbreaker
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
        .heldItem = ITEM_EXPERT_BELT, // special mixed coverage
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
        .heldItem = ITEM_LIFE_ORB, // Contrary Leaf Storm sweeper
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
        .heldItem = ITEM_LEFTOVERS, // bulky Sub + Glare pivot
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
        .heldItem = ITEM_CHOICE_BAND, // Reckless wallbreaker
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_WILD_CHARGE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // mixed attacker
        .moves =
        {
            MOVE_FLARE_BLITZ,
            MOVE_CLOSE_COMBAT,
            MOVE_HEAT_WAVE,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE, // all real abilities innate; chosen Flash Fire (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // mixed Swords Dance attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LIQUIDATION,
            MOVE_SACRED_SWORD,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB, // Torrent & Shell Armor now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_WATER_ABSORB, // Torrent & Shell Armor now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_FOCUS_BAND, // Sharpness Ceaseless Edge lead
        .moves =
        {
            MOVE_CEASELESS_EDGE,
            MOVE_AQUA_JET,
            MOVE_SUCKER_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Sharpness Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CEASELESS_EDGE,
            MOVE_LIQUIDATION,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_EXPERT_BELT, // Analytic coverage
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_FRISK, // all real abilities innate; chosen Frisk (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Sand Rush band
        .moves =
        {
            MOVE_RETURN,
            MOVE_SUPERPOWER,
            MOVE_CRUNCH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_SHEER_FORCE, // Sand Rush + Intimidate now innate; chosen Sheer Force (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Intimidate physical wall
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_TOXIC,
            MOVE_ROAR,
            MOVE_REST
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate; chosen Sheer Force (slot 2)
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
        .heldItem = ITEM_SITRUS_BERRY, // prankster support
        .moves =
        {
            MOVE_THUNDER_WAVE,
            MOVE_ENCORE,
            MOVE_FOUL_PLAY,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_INFILTRATOR, // Limber/Unburden/Prankster now innate; chosen Infiltrator (Foul Play/Encore ignore subs & screens)
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
        .heldItem = ITEM_SITRUS_BERRY, // Nasty Plot mixed
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_ENERGY_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_CHLOROPHYLL, // Overgrow + Gluttony now innate; chosen Chlorophyll (override, empty slot 1)
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
        .heldItem = ITEM_CHARCOAL, // Nasty Plot fire
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_FOCUS_BLAST,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze + Gluttony now innate; chosen Flash Fire (override, empty slot 1)
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
        .heldItem = ITEM_MYSTIC_WATER, // Nasty Plot water
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_WATER_ABSORB, // Torrent + Gluttony now innate; chosen Water Absorb (override, empty slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind wall
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
        .heldItem = ITEM_MENTAL_HERB, // Telepathy Trick Room setter
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_HELPING_HAND
        },
        .ability = ABILITY_SYNCHRONIZE, // Telepathy now innate; chosen Synchronize (:x:, never an innate -> stable) shares status back
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0521
    {
        .species = SPECIES_UNFEZANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS, // Super Luck guaranteed crits
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_RETURN,
            MOVE_U_TURN,
            MOVE_ROOST
        },
        .ability = ABILITY_RIVALRY, // all real abilities innate; chosen Rivalry (non-redundant)
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
        .heldItem = ITEM_EXPERT_BELT, // fast Motor Drive attacker
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
        .heldItem = ITEM_ROCKY_HELMET, // sand setter
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
        .heldItem = ITEM_HARD_STONE, // Sand Force hitter
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SAND_STREAM, // Sturdy & Sand Force now innate; chosen Sand Stream sets the sand they thrive in
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
        .heldItem = ITEM_SITRUS_BERRY, // Simple Calm Mind + Stored Power
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
        .heldItem = ITEM_LIFE_ORB, // Sand Rush sweeper
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_STREAM, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LEFTOVERS, // Mold Breaker hazard lead + spinner
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_SAND_STREAM, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SAND_STREAM, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator cleric wall
        .moves =
        {
            MOVE_WISH,
            MOVE_PROTECT,
            MOVE_TOXIC,
            MOVE_HEAL_BELL
        },
        .ability = ABILITY_CUTE_CHARM, // Regenerator now innate; chosen Cute Charm (gentle, affectionate nurse)
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
        .heldItem = ITEM_FLAME_ORB, // Guts bulk-up tank
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_MACH_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE, // Guts + Iron Fist now innate; Sheer Force powers its moves
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
        .heldItem = ITEM_ASSAULT_VEST, // Sheer Force mixed tank
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
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_WAVE,
            MOVE_ICE_BEAM
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim now innate; chosen Water Absorb
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
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky pivot
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
        .heldItem = ITEM_LEFTOVERS, // Guts Bulk Up tank
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF,
            MOVE_REST
        },
        .ability = ABILITY_SIMPLE, // Guts/Inner Focus/Mold Breaker ALL now innate; chosen Simple (fork override, slot 1) doubles Bulk Up on this tank
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
        .heldItem = ITEM_CHOICE_SCARF, // Mold Breaker revenge killer
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SHEER_FORCE, // Sturdy/Inner Focus/Mold Breaker all now innate; fork override frees slot 1 to a chosen Sheer Force
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
        .heldItem = ITEM_CHOICE_BAND, // Sturdy breaker band
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_SHEER_FORCE, // Sturdy/Inner Focus/Mold Breaker all now innate; fork override frees slot 1 to a chosen Sheer Force
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_LEAF_BLADE,
            MOVE_X_SCISSOR,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHARPNESS, // Swarm/Chlorophyll/Overcoat ALL now innate; chosen Sharpness (fork override, slot 2) boosts its slicing STAB
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
        .heldItem = ITEM_FOCUS_BAND, // Sticky Web lead
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_X_SCISSOR
        },
        .ability = ABILITY_SHARPNESS, // Swarm/Chlorophyll/Overcoat ALL now innate; chosen Sharpness (fork override, slot 2) boosts its slicing STAB
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
        .heldItem = ITEM_LIFE_ORB, // Speed Boost (now innate) sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_MEGAHORN,
            MOVE_POISON_JAB,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_POISON_POINT, // Swarm + Speed Boost both now innate; chosen Poison Point
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
        .heldItem = ITEM_LEFTOVERS, // Tailwind support (Speed Boost now innate)
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_PROTECT,
            MOVE_POISON_JAB,
            MOVE_MEGAHORN
        },
        .ability = ABILITY_POISON_POINT, // Swarm + Speed Boost both now innate; chosen Poison Point
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
        .heldItem = ITEM_LEFTOVERS, // Prankster utility pivot
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_LEECH_SEED,
            MOVE_ENCORE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SWEET_VEIL, // Prankster + Infiltrator + Chlorophyll now innate; chosen Sweet Veil keeps the team awake (override)
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
        .heldItem = ITEM_FOCUS_BAND, // Prankster Tailwind + redirect support
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HELPING_HAND,
            MOVE_MOONBLAST,
            MOVE_ENCORE
        },
        .ability = ABILITY_SWEET_VEIL, // Prankster + Infiltrator + Chlorophyll now innate; chosen Sweet Veil keeps the team awake (override)
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
        .heldItem = ITEM_LIFE_ORB, // Quiver Dance sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_SLEEP_POWDER,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_GRASSY_SURGE, // Chlorophyll/Own Tempo/Leaf Guard ALL now innate; chosen Grassy Surge (fork override, slot 2) stays observable
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Chlorophyll sun sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_GIGA_DRAIN,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_GRASSY_SURGE, // Chlorophyll/Own Tempo/Leaf Guard ALL now innate; chosen Grassy Surge (fork override, slot 2) stays observable
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll Victory Dance sweeper
        .moves =
        {
            MOVE_VICTORY_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_TRIPLE_AXEL
        },
        .ability = ABILITY_HUSTLE, // Chlorophyll + Leaf Guard now innate; chosen Hustle (real slot 1, :x: never-innate) powers this physical sweeper
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
        .heldItem = ITEM_FOCUS_BAND, // sash setup sweeper
        .moves =
        {
            MOVE_VICTORY_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_ICE_SPINNER
        },
        .ability = ABILITY_HUSTLE, // Chlorophyll + Leaf Guard now innate; chosen Hustle (real slot 1, :x: never-innate) powers this physical sweeper
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0553
    {
        .species = SPECIES_KROOKODILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Moxie band
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_KNOCK_OFF,
            MOVE_STONE_EDGE,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SAND_STREAM, // Moxie now innate; chosen Sand Stream (slot 2)
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
        .heldItem = ITEM_LEFTOVERS, // Intimidate bulky hazard lead
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_TAUNT
        },
        .ability = ABILITY_SAND_STREAM, // Intimidate now innate; chosen Sand Stream (slot 2)
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
        .heldItem = ITEM_CHOICE_SCARF, // Sheer Force revenge killer
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
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band wallbreaker
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
        .heldItem = ITEM_SHELL_BELL, // Sheer Force Flare Blitz hits huge but Sheer Force does not strip its recoil; the 1/4 heal pays that recoil back
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
        .heldItem = ITEM_CHOICE_SCARF, // Gorilla Tactics revenge killer
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
        .heldItem = ITEM_CHOICE_BAND, // wallbreaker
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves =
        {
            MOVE_GROWTH,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_STORM_DRAIN, // Chlorophyll now innate; chosen Storm Drain
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
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Moxie sweeper
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_KNOCK_OFF,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_RIVALRY, // Moxie now innate; chosen Rivalry (slot 1)
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
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate Fake Out support tank
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_RIVALRY, // Intimidate now innate; chosen Rivalry (slot 1)
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
        .heldItem = ITEM_LIFE_ORB, // Simple Cosmic Power stallbreaker (innate Magic Guard voids Life Orb recoil)
        .moves =
        {
            MOVE_COSMIC_POWER,
            MOVE_STORED_POWER,
            MOVE_ROOST,
            MOVE_PSYCHO_SHIFT
        },
        .ability = ABILITY_SIMPLE, // Wonder Skin/Magic Guard/Tinted Lens all innate now (Tier 5.4); chosen Simple doubles Cosmic Power
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // special attacker
        .moves =
        {
            MOVE_AIR_SLASH,
            MOVE_PSYCHIC,
            MOVE_HEAT_WAVE,
            MOVE_ROOST
        },
        .ability = ABILITY_SIMPLE, // Wonder Skin/Tinted Lens/Magic Guard all innate now (Tier 5.4); chosen Simple observable atop them
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
        .heldItem = ITEM_LEFTOVERS, // Mummy bulky special wall
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
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot Trick Room attacker
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
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_GHOST,
    },

    // 0565
    {
        .species = SPECIES_CARRACOSTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Solid Rock Shell Smash sweeper
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB, // Solid Rock (+ Sturdy + Swift Swim) now innate; chosen Water Absorb (override) heals the shell turtle
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
        .heldItem = ITEM_FLYING_GEM, // glass cannon (Defeatist drawback)
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
        .heldItem = ITEM_FOCUS_BAND, // fast hazard lead
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
        .heldItem = ITEM_ROCKY_HELMET, // Aftermath hazard setter
        .moves =
        {
            MOVE_TOXIC_SPIKES,
            MOVE_SPIKES,
            MOVE_GUNK_SHOT,
            MOVE_PAIN_SPLIT
        },
        .ability = ABILITY_POISON_TOUCH, // Aftermath now innate (Weak Armor is a wall drawback); chosen Poison Touch (slot-2 override) poisons via Gunk Shot
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
        .heldItem = ITEM_CHOICE_BAND, // Weak Armor attacker
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
        .heldItem = ITEM_LIFE_ORB, // Illusion mixed attacker
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
        .heldItem = ITEM_CHOICE_SPECS, // Illusion special breaker
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
        .heldItem = ITEM_CHOICE_SPECS, // Illusion Normal/Ghost breaker
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Skill Link multi-hit sweeper
        .moves =
        {
            MOVE_TAIL_SLAP,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_TOUGH_CLAWS, // Cute Charm + Technician + Skill Link now innate; chosen Tough Claws powers Tail Slap contact (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Skill Link band
        .moves =
        {
            MOVE_TAIL_SLAP,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Cute Charm + Technician + Skill Link now innate; chosen Tough Claws powers Tail Slap contact (override)
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
        .heldItem = ITEM_LEFTOVERS, // Shadow Tag Calm Mind trapper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_SHADOW_BALL,
            MOVE_REST
        },
        .ability = ABILITY_UNAWARE, // Shadow Tag now innate; chosen Unaware (override) lets this Calm Mind sweeper ignore the foe's stat boosts (real Frisk/Competitive slots kept for future innates)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind tank (innate Magic Guard); chosen No Guard makes Focus Blast reliable
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_FOCUS_BLAST,
            MOVE_RECOVER
        },
        .ability = ABILITY_NO_GUARD, // Overcoat/Magic Guard/Regenerator all innate now (Tier 5.4); chosen No Guard = sure-hit Focus Blast
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
        .heldItem = ITEM_LIFE_ORB, // Trick Room attacker (innate Magic Guard voids Life Orb recoil); No Guard = sure-hit Focus Blast
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_NO_GUARD, // Overcoat/Magic Guard/Regenerator all innate now (Tier 5.4); chosen No Guard
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0581
    {
        .species = SPECIES_SWANNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // hydration / offensive pivot
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_STORM_DRAIN, // Keen Eye + Big Pecks + Hydration all innate; chosen Storm Drain (override, slot 2)
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
        .heldItem = ITEM_LIFE_ORB, // Snow Warning special attacker
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HORN_LEECH,
            MOVE_DOUBLE_EDGE,
            MOVE_JUMP_KICK
        },
        .ability = ABILITY_SAP_SIPPER, // Chlorophyll now innate; chosen Sap Sipper
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
        .heldItem = ITEM_CHOICE_BAND, // Sap Sipper band
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
        .heldItem = ITEM_FOCUS_BAND, // doubles support glider
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
        .heldItem = ITEM_CHOICE_BAND, // Swarm band breaker
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_IRON_HEAD,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SHEER_FORCE, // Swarm/Shell Armor/Overcoat ALL now innate; chosen Sheer Force (fork override, slot 2) boosts Iron Head
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
        .heldItem = ITEM_LEFTOVERS, // Overcoat bulky tank
        .moves =
        {
            MOVE_MEGAHORN,
            MOVE_IRON_HEAD,
            MOVE_DRAIN_PUNCH,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_SHEER_FORCE, // Swarm/Shell Armor/Overcoat ALL now innate; chosen Sheer Force (fork override, slot 2) boosts Iron Head
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
        .heldItem = ITEM_ROCKY_HELMET, // Regenerator Rage Powder redirect
        .moves =
        {
            MOVE_RAGE_POWDER,
            MOVE_SPORE,
            MOVE_GIGA_DRAIN,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_WATER_ABSORB, // moved off Effect Spore (redundant deterministic sleep vs Spore); defensive pivot
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator status wall
        .moves =
        {
            MOVE_SPORE,
            MOVE_GIGA_DRAIN,
            MOVE_CLEAR_SMOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB, // moved off Effect Spore (deterministic sleep pre-empted its Toxic); defensive pivot
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
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky spinblocker
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
        .heldItem = ITEM_LEFTOVERS, // Cursed Body special wall
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_CHILLING_WATER,
            MOVE_RECOVER,
            MOVE_TAUNT
        },
        .ability = ABILITY_WATER_ABSORB, // Cursed Body now innate; chosen Water Absorb soaks Water hits + heals
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator Wish wall
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
        .heldItem = ITEM_FOCUS_BAND, // Compound Eyes Sticky Web lead
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_THUNDER,
            MOVE_BUG_BUZZ,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC, // all real abilities innate; chosen Static (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // special attacker
        .moves =
        {
            MOVE_THUNDER,
            MOVE_BUG_BUZZ,
            MOVE_ENERGY_BALL,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_STATIC, // all real abilities innate; chosen Static (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Iron Barbs hazard wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_SPIKES,
            MOVE_LEECH_SEED,
            MOVE_POWER_WHIP
        },
        .ability = ABILITY_FILTER, // Iron Barbs now innate; chosen Filter (slot-1 override) blunts the supereffective Fire hit
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 88,
            .spd = 168
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_FERROTHORN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Iron Barbs + Helmet contact punisher
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_FILTER, // Iron Barbs now innate; chosen Filter (slot-1 override) blunts the supereffective Fire hit
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_GRASS,
    },

    // 0601
    {
        .species = SPECIES_KLINKLANG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Clear Body Shift Gear sweeper
        .moves =
        {
            MOVE_SHIFT_GEAR,
            MOVE_GEAR_GRIND,
            MOVE_SUBSTITUTE,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Clear Body now innate; chosen Motor Drive via override (Electric immunity + Speed)
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
        .heldItem = ITEM_ASSAULT_VEST, // Levitate mixed tank
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
        .heldItem = ITEM_LIFE_ORB, // Coil physical attacker
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
        .heldItem = ITEM_LIFE_ORB, // Trick Room attacker
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SYNCHRONIZE, // Analytic now innate; chosen Synchronize
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0609
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Infiltrator revenge killer
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL,
            MOVE_TRICK
        },
        .ability = ABILITY_FLASH_FIRE, // Infiltrator now innate; chosen Flash Fire grants a Fire immunity
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
        .heldItem = ITEM_LIFE_ORB, // Flash Fire Calm Mind sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Trick Room wallbreaker
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
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
    },

    // 0612
    {
        .species = SPECIES_HAXORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mold Breaker Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_RIVALRY, // Mold Breaker + Unnerve now innate; repointed to its real slot-0 Rivalry (:x:, no override needed)
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
        .heldItem = ITEM_CHOICE_BAND, // Mold Breaker band breaker
        .moves =
        {
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_CLOSE_COMBAT,
            MOVE_FIRST_IMPRESSION
        },
        .ability = ABILITY_RIVALRY, // Mold Breaker + Unnerve now innate; repointed to its real slot-0 Rivalry (:x:, no override needed)
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
        .heldItem = ITEM_LIFE_ORB, // Swift Swim Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SNOW_WARNING, // Snow Cloak + Slush Rush + Swift Swim all now innate; chosen Snow Warning (override) sets snow
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
        .heldItem = ITEM_LEFTOVERS, // Levitate special wall + Rapid Spin
        .moves =
        {
            MOVE_FREEZE_DRY,
            MOVE_RAPID_SPIN,
            MOVE_RECOVER,
            MOVE_TOXIC
        },
        .ability = ABILITY_SNOW_WARNING, // Levitate now innate; chosen Snow Warning (ice-crystal being; Ice-type gets +Def in snow, no chip)
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
        .heldItem = ITEM_FOCUS_BAND, // fast lead, survives one hit
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_FOCUS_BLAST,
            MOVE_ENERGY_BALL,
            MOVE_SPIKES
        },
        .ability = ABILITY_TINTED_LENS, // Hydration/Sticky Hold/Unburden now all innate; chosen Tinted Lens (override, slot 1)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // disruptive Yawn/Encore lead
        .moves =
        {
            MOVE_YAWN,
            MOVE_ENCORE,
            MOVE_BUG_BUZZ,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_TINTED_LENS, // Sticky Hold now innate; chosen Tinted Lens lets Bug Buzz / Focus Blast hit resists full (override)
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
        .heldItem = ITEM_LEFTOVERS, // defensive trapper
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
        .heldItem = ITEM_ROCKY_HELMET, // bulky utility
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
        .heldItem = ITEM_LEFTOVERS, // bulky Ground/Electric pivot
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
        .heldItem = ITEM_CHOICE_SCARF, // Regenerator revenge killer / pivot
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_NO_GUARD, // Inner Focus/Regenerator/Reckless ALL now innate; chosen No Guard (fork override, slot 2) makes High Jump Kick never miss
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
        .heldItem = ITEM_LIFE_ORB, // Reckless wallbreaker
        .moves =
        {
            MOVE_HIGH_JUMP_KICK,
            MOVE_KNOCK_OFF,
            MOVE_POISON_JAB,
            MOVE_U_TURN
        },
        .ability = ABILITY_NO_GUARD, // Inner Focus/Regenerator/Reckless ALL now innate; chosen No Guard (fork override, slot 2) makes High Jump Kick never miss
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
        .heldItem = ITEM_ROCKY_HELMET, // Rough Skin bulky pivot
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_DRAGON_CLAW,
            MOVE_GLARE,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // Rough Skin now innate; chosen Sheer Force (real slot 1, :x: stable) powers Fire Punch / Dragon Claw
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
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band
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
        .heldItem = ITEM_CHOICE_BAND, // No Guard band breaker
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
        .heldItem = ITEM_LEFTOVERS, // Iron Fist bulky hazard lead
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_POLTERGEIST,
            MOVE_TOXIC
        },
        .ability = ABILITY_NO_GUARD, // Iron Fist now innate; chosen No Guard
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
        .heldItem = ITEM_CHOICE_BAND, // Reckless head-charge band
        .moves =
        {
            MOVE_HEAD_CHARGE,
            MOVE_EARTHQUAKE,
            MOVE_SUPERPOWER,
            MOVE_ZEN_HEADBUTT
        },
        .ability = ABILITY_SAP_SIPPER, // Reckless now innate; chosen Sap Sipper
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
        .heldItem = ITEM_CHOICE_BAND, // Defiant band
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Sheer Force Bulk Up sweeper
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Tinted Lens special attacker
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_HEAT_WAVE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE, // Keen Eye & Tinted Lens now innate; chosen Sheer Force powers up Hurricane / Psychic / Heat Wave
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
        .heldItem = ITEM_LIFE_ORB, // Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_HURRICANE,
            MOVE_PSYCHIC,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_SHEER_FORCE, // Keen Eye & Tinted Lens now innate; chosen Sheer Force (also skips Life Orb recoil on boosted moves)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Overcoat defensive Defog pivot
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_TOXIC
        },
        .ability = ABILITY_UNAWARE, // Big Pecks + Overcoat now innate; chosen Unaware (fork override, slot 2, ignores foe boosts) suits this Foul Play wall
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
        .heldItem = ITEM_ROCKY_HELMET, // Weak Armor physical wall
        .moves =
        {
            MOVE_FOUL_PLAY,
            MOVE_ROOST,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_UNAWARE, // Big Pecks + Overcoat now innate; chosen Unaware (fork override, slot 2, ignores foe boosts) suits this Foul Play wall
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
        .heldItem = ITEM_LIFE_ORB, // White Smoke mixed attacker
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
        .heldItem = ITEM_CHOICE_BAND, // Hustle band breaker
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
        .heldItem = ITEM_LIFE_ORB, // Swarm Hone Claws sweeper
        .moves =
        {
            MOVE_HONE_CLAWS,
            MOVE_IRON_HEAD,
            MOVE_X_SCISSOR,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_HUSTLE, // Swarm now innate (latched); chosen Hustle
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
        .heldItem = ITEM_CHOICE_SCARF, // Levitate revenge killer
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Defog pivot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Quiver Dance sweeper
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
        .heldItem = ITEM_LEFTOVERS, // bulky Quiver Dance + Roost
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
        .heldItem = ITEM_LEFTOVERS, // Justified Swords Dance setup
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer / pivot
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_IRON_HEAD,
            MOVE_STONE_EDGE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_CHOICE_BAND, // Justified band breaker
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_FOCUS_BAND, // Swords Dance / hazard lead
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_ASSAULT_VEST, // Calm Mind special tank
        .moves =
        {
            MOVE_GIGA_DRAIN,
            MOVE_FOCUS_BLAST,
            MOVE_AIR_SLASH,
            MOVE_VACUUM_WAVE
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_LIFE_ORB, // Prankster offensive pivot
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_FOCUS_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_CLOUD_NINE, // all real abilities innate; chosen Cloud Nine (non-redundant)
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
        .heldItem = ITEM_FOCUS_BAND, // Prankster Tailwind support
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HURRICANE,
            MOVE_TAUNT,
            MOVE_RAIN_DANCE
        },
        .ability = ABILITY_CLOUD_NINE, // all real abilities innate; chosen Cloud Nine (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Regenerator special pivot
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_HEAT_WAVE,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_PRANKSTER,
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
        .heldItem = ITEM_LIFE_ORB, // Prankster mixed attacker
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_FOCUS_BLAST,
            MOVE_KNOCK_OFF,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_VOLT_ABSORB, // all real abilities innate; chosen Volt Absorb (non-redundant)
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
        .heldItem = ITEM_FOCUS_BAND, // Prankster Thunder Wave support
        .moves =
        {
            MOVE_THUNDER_WAVE,
            MOVE_THUNDERBOLT,
            MOVE_TAUNT,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB, // all real abilities innate; chosen Volt Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_SCARF, // Volt Absorb revenge killer
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
        .heldItem = ITEM_CHOICE_SPECS, // Turboblaze special breaker
        .moves =
        {
            MOVE_BLUE_FLARE,
            MOVE_DRACO_METEOR,
            MOVE_FLAMETHROWER,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_FLASH_FIRE, // Turboblaze now innate (fork override, Flash Fire slot 1); chosen Flash Fire shrugs off Fire
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Roost attacker
        .moves =
        {
            MOVE_BLUE_FLARE,
            MOVE_DRAGON_PULSE,
            MOVE_ROOST,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLASH_FIRE, // Turboblaze now innate (fork override, Flash Fire slot 1); chosen Flash Fire shrugs off Fire
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
        .heldItem = ITEM_LIFE_ORB, // Teravolt Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_BOLT_STRIKE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Teravolt now innate (fork override, Motor Drive slot 1); chosen Motor Drive draws Electric + Speed
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
        .heldItem = ITEM_CHOICE_BAND, // Teravolt band breaker
        .moves =
        {
            MOVE_BOLT_STRIKE,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE, // Teravolt now innate (fork override, Motor Drive slot 1); chosen Motor Drive draws Electric + Speed
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
        .heldItem = ITEM_LIFE_ORB, // Sheer Force special nuke
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
        .heldItem = ITEM_CHOICE_SCARF, // Intimidate revenge killer
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_U_TURN,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate (fork override, Sheer Force slot 1); chosen Sheer Force powers Stone Edge/EQ
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
        .heldItem = ITEM_ROCKY_HELMET, // Intimidate bulky hazard pivot
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN,
            MOVE_TOXIC
        },
        .ability = ABILITY_SHEER_FORCE, // Intimidate now innate (fork override, Sheer Force slot 1); chosen Sheer Force is dead on this set but keeps the slot valid
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
        .heldItem = ITEM_CHOICE_SPECS, // Pressure special breaker
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_DRACO_METEOR,
            MOVE_FREEZE_DRY,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SNOW_WARNING, // Pressure now innate; chosen Snow Warning summons snow (slot-1 override)
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
        .heldItem = ITEM_LEFTOVERS, // Sub-Roost stallbreaker
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_ROOST,
            MOVE_ICE_BEAM,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SNOW_WARNING, // Pressure now innate; chosen Snow Warning summons snow (slot-1 override)
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
        .heldItem = ITEM_CHOICE_BAND, // Teravolt physical breaker
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_FUSION_BOLT,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Teravolt now innate (fork override, Motor Drive slot 1); chosen Motor Drive draws Electric + Speed
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
        .heldItem = ITEM_LIFE_ORB, // Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_FUSION_BOLT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_MOTOR_DRIVE, // Teravolt now innate (fork override, Motor Drive slot 1); chosen Motor Drive draws Electric + Speed
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
        .heldItem = ITEM_CHOICE_SPECS, // Turboblaze special nuke
        .moves =
        {
            MOVE_ICE_BEAM,
            MOVE_FUSION_FLARE,
            MOVE_DRACO_METEOR,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_FLASH_FIRE, // Turboblaze now innate (fork override, Flash Fire slot 1); chosen Flash Fire shrugs off Fire
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
        .heldItem = ITEM_CHOICE_SPECS, // Justified special breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SECRET_SWORD,
            MOVE_VACUUM_WAVE,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SECRET_SWORD,
            MOVE_TERA_BLAST,
            MOVE_ICY_WIND
        },
        .ability = ABILITY_JUSTIFIED,
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
        .heldItem = ITEM_LIFE_ORB, // Serene Grace Calm Mind attacker
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_HYPER_VOICE,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_PUNK_ROCK, // Serene Grace now innate; chosen Punk Rock (override, real slot empty)
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
        .heldItem = ITEM_ASSAULT_VEST, // bulky special pivot
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_SHADOW_BALL
        },
        .ability = ABILITY_PUNK_ROCK, // Serene Grace now innate; chosen Punk Rock (override, real slot empty)
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
        .heldItem = ITEM_CHOICE_SCARF, // Download revenge killer / pivot
        .moves =
        {
            MOVE_U_TURN,
            MOVE_ICE_BEAM,
            MOVE_FLAMETHROWER,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SHEER_FORCE, // sole real ability Download now innate; empty slot 1 filled with chosen Sheer Force, innate Download intact
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
        .heldItem = ITEM_LIFE_ORB, // Download Techno Blast attacker
        .moves =
        {
            MOVE_TECHNO_BLAST,
            MOVE_FLASH_CANNON,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SHEER_FORCE, // sole real ability Download now innate; empty slot 1 filled with chosen Sheer Force, innate Download intact
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
        .heldItem = ITEM_LEFTOVERS, // bulky Spikes / pivot wall
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
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulky attacker
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance breaker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_WOOD_HAMMER,
            MOVE_CLOSE_COMBAT,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_BULLETPROOF, // Overgrow now innate (latched); chosen Bulletproof
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze / Magician both innate; chosen Flash Fire (override)
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_PSYCHIC,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze / Magician both innate; chosen Flash Fire (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sub stallbreaker
        .moves =
        {
            MOVE_SUBSTITUTE,
            MOVE_CALM_MIND,
            MOVE_FIRE_BLAST,
            MOVE_PSYSHOCK
        },
        .ability = ABILITY_FLASH_FIRE, // Blaze / Magician both innate; chosen Flash Fire (override)
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
        .heldItem = ITEM_LIFE_ORB, // Protean wallbreaker
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
        .heldItem = ITEM_FOCUS_BAND, // suicide hazard lead
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
        .heldItem = ITEM_CHOICE_SCARF, // Battle Bond revenge killer
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
        .heldItem = ITEM_CHOICE_BAND, // Huge Power band
        .moves =
        {
            MOVE_EARTHQUAKE,
            MOVE_RETURN,
            MOVE_QUICK_ATTACK,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_SCRAPPY, // Huge Power now innate; chosen Scrappy (override slot 2) lets Return/Quick Attack hit Ghosts
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_EARTHQUAKE,
            MOVE_RETURN,
            MOVE_QUICK_ATTACK
        },
        .ability = ABILITY_SCRAPPY, // Huge Power now innate; chosen Scrappy (override slot 2) lets Return/Quick Attack hit Ghosts
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
        .heldItem = ITEM_LIFE_ORB, // Gale Wings priority sweeper
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_FLARE_BLITZ,
            MOVE_SWORDS_DANCE,
            MOVE_U_TURN
        },
        .ability = ABILITY_FLAME_BODY, // Gale Wings now innate; chosen Flame Body burns contact attackers
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Roost / hazard control
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
        .heldItem = ITEM_SHARP_BEAK, // Tailwind setter + priority
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_BRAVE_BIRD,
            MOVE_FLARE_BLITZ,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_FLAME_BODY, // Gale Wings now innate; chosen Flame Body burns contact attackers
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FLYING,
    },

    // 0668
    {
        .species = SPECIES_PYROAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_HYPER_VOICE,
            MOVE_DARK_PULSE,
            MOVE_SOLAR_BEAM
        },
        .ability = ABILITY_RIVALRY, // all real abilities innate; chosen Rivalry (non-redundant)
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
        .heldItem = ITEM_THROAT_SPRAY, // Hyper Voice spread attacker
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_HEAT_WAVE,
            MOVE_SNARL,
            MOVE_PROTECT
        },
        .ability = ABILITY_RIVALRY, // all real abilities innate; chosen Rivalry (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind cleric wall
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_SYNTHESIS,
            MOVE_AROMATHERAPY
        },
        .ability = ABILITY_SYMBIOSIS, // Flower Veil now innate; chosen Symbiosis (:x:, never an innate -> stable) passes its item to the ally
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
        .heldItem = ITEM_CHOICE_SPECS, // Pixilate breaker
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Sap Sipper tank
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
        .heldItem = ITEM_LIFE_ORB, // physical attacker
        .moves =
        {
            MOVE_HORN_LEECH,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_BULK_UP
        },
        .ability = ABILITY_SAP_SIPPER, // Grass Pelt now innate; chosen Sap Sipper (real slot 0) grants Grass immunity + Atk boost
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
        .heldItem = ITEM_CHOICE_BAND, // Iron Fist / Scrappy band
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT,
            MOVE_GUNK_SHOT,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_TOUGH_CLAWS, // Iron Fist/Mold Breaker/Scrappy all now innate; fork override frees slot 0 to a chosen Tough Claws (all its moves make contact)
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance breaker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KNOCK_OFF,
            MOVE_DRAIN_PUNCH,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_TOUGH_CLAWS, // Iron Fist/Mold Breaker/Scrappy all now innate; fork override frees slot 0 to a chosen Tough Claws (all its moves make contact)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0678
    {
        .species = SPECIES_MEOWSTIC,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Prankster screens/support
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_FAKE_OUT,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_OWN_TEMPO, // Keen Eye + Infiltrator + Prankster now innate; chosen Own Tempo (confusion immunity) (override)
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
        .heldItem = ITEM_MENTAL_HERB, // Prankster disruption support
        .moves =
        {
            MOVE_FOLLOW_ME,
            MOVE_THUNDER_WAVE,
            MOVE_HELPING_HAND,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_OWN_TEMPO, // Keen Eye + Infiltrator + Prankster now innate; chosen Own Tempo (confusion immunity) (override)
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
        .heldItem = ITEM_LIFE_ORB, // Competitive special attacker
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_PRANKSTER, // all real abilities innate; chosen Prankster (non-redundant)
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
        .heldItem = ITEM_CHOICE_SPECS, // Competitive breaker
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_ENERGY_BALL
        },
        .ability = ABILITY_PRANKSTER, // all real abilities innate; chosen Prankster (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // King's Shield stance tank
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Weakness Policy sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance / Spectral Thief
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
        .heldItem = ITEM_LEFTOVERS, // Trick Room cleric (innate Aroma Veil / Healer)
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_MOONBLAST,
            MOVE_AROMATHERAPY,
            MOVE_WISH
        },
        .ability = ABILITY_MISTY_SURGE, // Healer + Aroma Veil both now innate; chosen Misty Surge (fork override) sets terrain support for the Fairy cleric
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0685
    {
        .species = SPECIES_SLURPUFF,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY, // Unburden Belly Drum sweeper
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_PLAY_ROUGH,
            MOVE_DRAIN_PUNCH,
            MOVE_FACADE
        },
        .ability = ABILITY_UNAWARE, // Sweet Veil/Unburden now innate (Unburden still doubles Speed once Sitrus is eaten); chosen Unaware (override, slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind special wall
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DAZZLING_GLEAM,
            MOVE_FLAMETHROWER,
            MOVE_DRAINING_KISS
        },
        .ability = ABILITY_UNAWARE, // Sweet Veil now innate; chosen Unaware (fork override)
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
        .heldItem = ITEM_LEFTOVERS, // Contrary Superpower sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Superpower + priority
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
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_CROSS_CHOP
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // physical Tough Claws attacker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_ASSAULT_VEST, // Adaptability special tank
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SLUDGE_WAVE,
            MOVE_FLIP_TURN,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_POISON_TOUCH, // Adaptability now innate; chosen Poison Touch
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
        .heldItem = ITEM_BLACK_SLUDGE, // bulky Toxic Spikes pivot
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
        .heldItem = ITEM_CHOICE_SPECS, // Mega Launcher breaker
        .moves =
        {
            MOVE_DARK_PULSE,
            MOVE_WATER_PULSE,
            MOVE_AURA_SPHERE,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_WATER_ABSORB, // Mega Launcher now innate; chosen Water Absorb (override)
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
        .heldItem = ITEM_ASSAULT_VEST, // bulky special pivot
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_DARK_PULSE,
            MOVE_ICE_BEAM,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_WATER_ABSORB, // Mega Launcher now innate; chosen Water Absorb (override)
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
        .heldItem = ITEM_LIFE_ORB, // Dry Skin fast special attacker
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
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
        .heldItem = ITEM_CHOICE_BAND, // Strong Jaw band breaker
        .moves =
        {
            MOVE_HEAD_SMASH,
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_RECKLESS, // Strong Jaw + Rock Head now innate; chosen Reckless powers Head Smash (recoil voided by innate Rock Head) (override)
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
        .heldItem = ITEM_LIFE_ORB, // Dragon Dance sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_OUTRAGE,
            MOVE_HEAD_SMASH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_RECKLESS, // Strong Jaw + Rock Head now innate; chosen Reckless powers Head Smash (recoil voided by innate Rock Head) (override)
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
        .heldItem = ITEM_CHOICE_SCARF, // Strong Jaw revenge killer
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_CRUNCH,
            MOVE_DRAGON_CLAW,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_RECKLESS, // Strong Jaw + Rock Head now innate; chosen Reckless powers Head Smash (recoil voided by innate Rock Head) (override)
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
        .heldItem = ITEM_LEFTOVERS, // Refrigerate bulky attacker / Aurora Veil
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
        .heldItem = ITEM_LIGHT_CLAY, // Snow Warning Aurora Veil setter
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
        .heldItem = ITEM_LIFE_ORB, // Pixilate Hyper Voice breaker
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind cleric wall
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
        .heldItem = ITEM_THROAT_SPRAY, // Hyper Voice spread support
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
        .heldItem = ITEM_QUICK_CLAW, // Unburden Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ACROBATICS,
            MOVE_CLOSE_COMBAT,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_TOUGH_CLAWS, // Limber/Unburden/Mold Breaker all now innate; fork override frees slot 1 to a chosen Tough Claws (all its STAB makes contact)
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
        .heldItem = ITEM_FOCUS_BAND, // Mold Breaker physical attacker
        .moves =
        {
            MOVE_ACROBATICS,
            MOVE_CLOSE_COMBAT,
            MOVE_STONE_EDGE,
            MOVE_PROTECT
        },
        .ability = ABILITY_TOUGH_CLAWS, // Limber/Unburden/Mold Breaker all now innate; fork override frees slot 1 to a chosen Tough Claws (all its STAB makes contact)
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
        .heldItem = ITEM_LIGHT_CLAY, // Cheek Pouch / screens support
        .moves =
        {
            MOVE_NUZZLE,
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_DAZZLING_GLEAM
        },
        // Cheek Pouch + Pickup now innate; chosen Plus (real slot 2) is the Electric mouse's doubles Sp. Atk gimmick.
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
        .heldItem = ITEM_LIGHT_CLAY, // Sturdy dual-screens / hazards wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate (Sturdy too); chosen Solid Rock via override blunts SE hits
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
        .heldItem = ITEM_LIGHT_CLAY, // Trick Room screens setter
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_MOONBLAST
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate (Sturdy too); chosen Solid Rock via override blunts SE hits
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_FAIRY,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0706
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Sap Sipper special tank
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
        .heldItem = ITEM_LEFTOVERS, // Gooey bulky pivot
        .moves =
        {
            MOVE_DRAGON_PULSE,
            MOVE_CHILLING_WATER,
            MOVE_TOXIC,
            MOVE_REST
        },
        .ability = ABILITY_SAP_SIPPER, // Gooey now innate; chosen Sap Sipper (real slot 0, :x: stable) grants a Grass immunity + Attack boost
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
        .heldItem = ITEM_CHOICE_SPECS, // Hydration breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SLUDGE_WAVE,
            MOVE_FIRE_BLAST,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SAP_SIPPER, // Hydration now innate (rain-cures); chosen Sap Sipper adds a Grass immunity
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
        .heldItem = ITEM_ASSAULT_VEST, // Sap Sipper special tank
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
        .heldItem = ITEM_LEFTOVERS, // Shell Armor bulky setup
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_DRACO_METEOR,
            MOVE_RECOVER
        },
        .ability = ABILITY_SAP_SIPPER, // Shell Armor now innate; chosen Sap Sipper walls Grass
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
        .heldItem = ITEM_LIGHT_CLAY, // Prankster screens + Spikes
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_THUNDER_WAVE,
            MOVE_SPIKES
        },
        .ability = ABILITY_BULLETPROOF, // Prankster / Magician both innate; chosen Bulletproof (override)
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
        .heldItem = ITEM_LEFTOVERS, // Prankster annoyer wall
        .moves =
        {
            MOVE_SPIKES,
            MOVE_THUNDER_WAVE,
            MOVE_FOUL_PLAY,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_BULLETPROOF, // Prankster / Magician both innate; chosen Bulletproof (override)
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
        .heldItem = ITEM_LEFTOVERS, // Harvest Sitrus stall
        .moves =
        {
            MOVE_HORN_LEECH,
            MOVE_POLTERGEIST,
            MOVE_WILL_O_WISP,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_SAP_SIPPER, // Natural Cure + Frisk + Harvest all innate; chosen Sap Sipper (override, slot 2)
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
        .heldItem = ITEM_CHOICE_BAND, // Natural Cure trick-room-friendly breaker
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_WOOD_HAMMER,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_SAP_SIPPER, // Natural Cure + Frisk + Harvest all innate; chosen Sap Sipper (override, slot 2)
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spd = 4
        ),
        .teraType = TYPE_GHOST,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0711
    {
        .species = SPECIES_GOURGEIST_SUPER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // physically defensive WoW wall
        .moves =
        {
            MOVE_WILL_O_WISP,
            MOVE_POLTERGEIST,
            MOVE_LEECH_SEED,
            MOVE_SYNTHESIS
        },
        .ability = ABILITY_HARVEST, // all real abilities innate; chosen Harvest (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Trick Room band breaker
        .moves =
        {
            MOVE_POLTERGEIST,
            MOVE_SEED_BOMB,
            MOVE_TRICK,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_HARVEST, // all real abilities innate; chosen Harvest (non-redundant)
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .teraType = TYPE_GHOST,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0713
    {
        .species = SPECIES_AVALUGG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy physical wall / Rapid Spin
        .moves =
        {
            MOVE_RECOVER,
            MOVE_AVALANCHE,
            MOVE_BODY_PRESS,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_ICE_SCALES, // Own Tempo/Ice Body/Sturdy ALL now innate; chosen Ice Scales (fork override, slot 2) halves special damage on this physical wall
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
        .heldItem = ITEM_ROCKY_HELMET, // Ice Body chip wall
        .moves =
        {
            MOVE_AVALANCHE,
            MOVE_BODY_PRESS,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_ICE_SCALES, // Own Tempo/Ice Body/Sturdy ALL now innate; chosen Ice Scales (fork override, slot 2) halves special damage on this physical wall
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spd = 4
        ),
        .teraType = TYPE_ICE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0713
    {
        .species = SPECIES_AVALUGG_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Strong Jaw bulky setup
        .moves =
        {
            MOVE_RECOVER,
            MOVE_ICICLE_CRASH,
            MOVE_BODY_PRESS,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_ICE_SCALES, // Strong Jaw + Ice Body + Sturdy all innate; chosen Ice Scales (override, slot 1)
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
        .heldItem = ITEM_CHOICE_BAND, // Strong Jaw band attacker
        .moves =
        {
            MOVE_ICE_FANG,
            MOVE_CRUNCH,
            MOVE_STONE_EDGE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_ICE_SCALES, // Strong Jaw + Ice Body + Sturdy all innate; chosen Ice Scales (override, slot 1)
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
        .heldItem = ITEM_CHOICE_SPECS, // Infiltrator special breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_HURRICANE,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_PUNK_ROCK, // Infiltrator now innate; chosen Punk Rock powers Boomburst / Hyper Voice (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Defog pivot
        .moves =
        {
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_U_TURN
        },
        .ability = ABILITY_PUNK_ROCK, // Infiltrator now innate; chosen Punk Rock powers Boomburst / Hyper Voice (override)
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
        .heldItem = ITEM_LIFE_ORB, // Tailwind setter
        .moves =
        {
            MOVE_TAILWIND,
            MOVE_HURRICANE,
            MOVE_DRACO_METEOR,
            MOVE_HEAT_WAVE
        },
        .ability = ABILITY_PUNK_ROCK, // all real abilities innate; chosen Punk Rock (non-redundant)
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
        .heldItem = ITEM_POWER_HERB, // Geomancy sweeper
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
        .heldItem = ITEM_LIFE_ORB, // physical Swords Dance variant
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
        .heldItem = ITEM_LIFE_ORB, // Dark Aura mixed attacker
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Roost pivot
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
        .heldItem = ITEM_LEFTOVERS, // 50% Dragon Dance bulky sweeper; Power Construct -> Complete at <=50% HP
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
        .heldItem = ITEM_LIFE_ORB, // 10% fast Dragon Dance sweeper; Power Construct -> Complete at <=50% HP
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
        .heldItem = ITEM_LIFE_ORB, // Mega Diancie (Magic Bounce); power for the glass cannon
        .moves =
        {
            MOVE_DIAMOND_STORM,
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_PROTECT
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate; chosen Solid Rock via override blunts SE hits
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
        .heldItem = ITEM_LEFTOVERS, // Clear Body hazards / dual-screens wall
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_DIAMOND_STORM,
            MOVE_MOONBLAST,
            MOVE_REFLECT
        },
        .ability = ABILITY_SOLID_ROCK, // Clear Body now innate; chosen Solid Rock via override blunts SE hits
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
        .heldItem = ITEM_CHOICE_SPECS, // Magician special breaker
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_TINTED_LENS, // Magician innate; chosen Tinted Lens (override)
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
        .heldItem = ITEM_LIFE_ORB, // mixed Magician wallbreaker
        .moves =
        {
            MOVE_HYPERSPACE_FURY,
            MOVE_PSYCHIC,
            MOVE_GUNK_SHOT,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_TOUGH_CLAWS, // Magician innate; chosen Tough Claws (override)
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank breaker
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DARK_PULSE,
            MOVE_FOCUS_BLAST,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_TOUGH_CLAWS, // Magician innate; chosen Tough Claws (override)
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_PSYCHIC,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // 0721
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Water Absorb special breaker
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
        .heldItem = ITEM_LEFTOVERS, // bulky Substitute pivot
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
        .heldItem = ITEM_ASSAULT_VEST, // spread special tank
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
        .heldItem = ITEM_SPELL_TAG, // Ghost STAB boost for the Spirit Shackle attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SPIRIT_SHACKLE,
            MOVE_LEAF_BLADE,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_SNIPER, // Overgrow + Long Reach now innate; chosen Sniper pays off Triple Arrows crit (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Long Reach trap-shooter
        .moves =
        {
            MOVE_SPIRIT_SHACKLE,
            MOVE_LEAF_BLADE,
            MOVE_DEFOG,
            MOVE_ROOST
        },
        .ability = ABILITY_SNIPER, // Overgrow + Long Reach now innate; chosen Sniper pays off Triple Arrows crit (override)
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
        .heldItem = ITEM_LIFE_ORB, // mixed attacker
        .moves =
        {
            MOVE_LEAF_BLADE,
            MOVE_SPIRIT_SHACKLE,
            MOVE_SUCKER_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_SNIPER, // Overgrow + Long Reach now innate; chosen Sniper pays off Triple Arrows crit (override)
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
        .heldItem = ITEM_LIFE_ORB, // Scrappy Triple Arrows attacker
        .moves =
        {
            MOVE_TRIPLE_ARROWS,
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SNIPER, // Overgrow (latched) & Scrappy now innate; chosen Sniper (empty-slot override) pays off Triple Arrows' high crit rate
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
        .heldItem = ITEM_LEFTOVERS, // Swords Dance bulky setup
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_TRIPLE_ARROWS,
            MOVE_LEAF_BLADE,
            MOVE_ROOST
        },
        .ability = ABILITY_SNIPER, // Overgrow (latched) & Scrappy now innate; chosen Sniper (empty-slot override)
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
        .heldItem = ITEM_SITRUS_BERRY, // Intimidate pivot, Fake Out support
        .moves =
        {
            MOVE_FAKE_OUT,
            MOVE_FLARE_BLITZ,
            MOVE_DARKEST_LARIAT,
            MOVE_PARTING_SHOT
        },
        .ability = ABILITY_TOUGH_CLAWS, // Blaze + Intimidate now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_BLACK_GLASSES, // Dark STAB boost for the Darkest Lariat attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FLARE_BLITZ,
            MOVE_DARKEST_LARIAT,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_TOUGH_CLAWS, // Blaze + Intimidate now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_LEFTOVERS, // bulky Intimidate pivot
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_FLARE_BLITZ,
            MOVE_WILL_O_WISP,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Blaze + Intimidate now innate; chosen Tough Claws powers its contact kit (override)
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
        .heldItem = ITEM_CHOICE_SPECS, // Liquid Voice special breaker
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind bulky setup
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_SPARKLING_ARIA,
            MOVE_MOONBLAST,
            MOVE_REST
        },
        .ability = ABILITY_LIQUID_VOICE, // Torrent now innate (latched); chosen Liquid Voice
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
        .heldItem = ITEM_THROAT_SPRAY, // Sparkling Aria is a sound move -> +SpAtk
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_MOONBLAST,
            MOVE_ENERGY_BALL,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_LIQUID_VOICE, // Torrent now innate (latched); chosen Liquid Voice
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
        .heldItem = ITEM_CHOICE_BAND, // Skill Link Bullet Seed band
        .moves =
        {
            MOVE_BEAK_BLAST,
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_SHEER_FORCE, // Keen Eye + Skill Link now innate; chosen Sheer Force
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
        .heldItem = ITEM_LIFE_ORB, // Skill Link already maxes multi-hit; Life Orb adds power
        .moves =
        {
            MOVE_BULLET_SEED,
            MOVE_ROCK_BLAST,
            MOVE_BRAVE_BIRD,
            MOVE_BEAK_BLAST
        },
        .ability = ABILITY_SHEER_FORCE, // Keen Eye + Skill Link now innate; chosen Sheer Force
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
        .heldItem = ITEM_SITRUS_BERRY, // Stakeout switch punisher
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE,
            MOVE_U_TURN
        },
        .ability = ABILITY_SUPER_LUCK, // all real abilities innate; chosen Super Luck (non-redundant)
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_MOTOR_DRIVE, // Levitate now innate; chosen Motor Drive (electromagnetic beetle banks electricity into Speed)
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
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room special attacker
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves =
        {
            MOVE_BUG_BUZZ,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_MOTOR_DRIVE, // Levitate now innate; chosen Motor Drive (electromagnetic beetle banks electricity into Speed)
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
        .heldItem = ITEM_CHOICE_BAND, // Iron Fist punch band
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_ICE_HAMMER
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room bruiser
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Dancer Fire/Flying special attacker
        .moves =
        {
            MOVE_REVELATION_DANCE,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_TINTED_LENS, // Dancer now innate (Tier 5.9); chosen Tinted Lens (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Psychic/Flying pivot
        .moves =
        {
            MOVE_REVELATION_DANCE,
            MOVE_HURRICANE,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_TINTED_LENS, // Dancer now innate (Tier 5.9); chosen Tinted Lens (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_FOCUS_BAND, // fast Sticky Web lead
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_MOONBLAST,
            MOVE_BUG_BUZZ,
            MOVE_STUN_SPORE
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Quiver Dance sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_MOONBLAST,
            MOVE_BUG_BUZZ,
            MOVE_POLLEN_PUFF
        },
        .ability = ABILITY_CUTE_CHARM, // all real abilities innate; chosen Cute Charm (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // Sand Rush attacker
        .moves =
        {
            MOVE_STONE_EDGE,
            MOVE_ACCELEROCK,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_SAND_STREAM, // all real abilities innate; chosen Sand Stream (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // power for the Tough Claws Stone Edge attacker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_STONE_EDGE,
            MOVE_ACCELEROCK,
            MOVE_CLOSE_COMBAT
        },
        .ability = ABILITY_SAND_RUSH, // Tough Claws now innate; chosen Sand Rush (override)
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
        .heldItem = ITEM_CHOICE_BAND, // No Guard band
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
        .heldItem = ITEM_ASSAULT_VEST, // Schooling special tank
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
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
        .heldItem = ITEM_BLACK_SLUDGE, // Regenerator wall
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_ROCKY_HELMET, // physically defensive Banded Bunker stall
        .moves =
        {
            MOVE_BANEFUL_BUNKER,
            MOVE_TOXIC_SPIKES,
            MOVE_RECOVER,
            MOVE_CHILLING_WATER
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_BLACK_SLUDGE, // Merciless redirect support
        .moves =
        {
            MOVE_CHILLING_WATER,
            MOVE_BANEFUL_BUNKER,
            MOVE_TOXIC,
            MOVE_HAZE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Stamina physical wall
        .moves =
        {
            MOVE_HIGH_HORSEPOWER,
            MOVE_BODY_PRESS,
            MOVE_STEALTH_ROCK,
            MOVE_ROAR
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
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
        .heldItem = ITEM_ROCKY_HELMET, // Stamina hazard tank
        .moves =
        {
            MOVE_HIGH_HORSEPOWER,
            MOVE_HEAVY_SLAM,
            MOVE_STEALTH_ROCK,
            MOVE_TOXIC
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Water Bubble bulky attacker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_LEECH_LIFE,
            MOVE_MIRROR_COAT,
            MOVE_TOXIC
        },
        .ability = ABILITY_WATER_ABSORB, // Water Bubble now innate; chosen Water Absorb heals the water spider on Water hits
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
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room Water Bubble nuke
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
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
        .heldItem = ITEM_LIFE_ORB, // Contrary Leaf Storm attacker
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
        .heldItem = ITEM_LEFTOVERS, // bulky support
        .moves =
        {
            MOVE_LEAF_BLADE,
            MOVE_LEECH_SEED,
            MOVE_SYNTHESIS,
            MOVE_TOXIC
        },
        .ability = ABILITY_CONTRARY, // Leaf Guard now innate; chosen Contrary (real slot) inverts stat drops
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
        .heldItem = ITEM_LEFTOVERS, // Spore + Strength Sap support
        .moves =
        {
            MOVE_SPORE,
            MOVE_STRENGTH_SAP,
            MOVE_MOONBLAST,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_ILLUMINATE, // moved off Effect Spore (redundant deterministic sleep vs Spore); real slot, glowing-mushroom flavor
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
        .heldItem = ITEM_BLACK_SLUDGE, // Corrosion toxic staller
        .moves =
        {
            MOVE_TOXIC,
            MOVE_FIRE_BLAST,
            MOVE_PROTECT,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_FLAME_BODY, // Corrosion + Oblivious now innate; chosen Flame Body punishes contact (override)
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
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FIRE_BLAST,
            MOVE_SLUDGE_WAVE,
            MOVE_DRAGON_PULSE
        },
        .ability = ABILITY_FLAME_BODY, // Corrosion + Oblivious now innate; chosen Flame Body punishes contact (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Fluffy / Force band
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
        .heldItem = ITEM_LEFTOVERS, // Fluffy bulky setup
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
        .heldItem = ITEM_LIFE_ORB, // Queenly Majesty attacker
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_HIGH_JUMP_KICK,
            MOVE_PLAY_ROUGH,
            MOVE_U_TURN
        },
        .ability = ABILITY_GRASSY_SURGE, // Leaf Guard/Queenly Majesty/Sweet Veil ALL now innate; chosen Grassy Surge (fork override, slot 2) powers its Grass STAB
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky priority-block pivot
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_RAPID_SPIN,
            MOVE_SYNTHESIS,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_GRASSY_SURGE, // Leaf Guard/Queenly Majesty/Sweet Veil ALL now innate; chosen Grassy Surge (fork override, slot 2) powers its Grass STAB
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0764 (innate Levitate + Triage)
    {
        .species = SPECIES_COMFEY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Triage priority healer
        .moves =
        {
            MOVE_FLORAL_HEALING,
            MOVE_DRAINING_KISS,
            MOVE_GIGA_DRAIN,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_SWEET_VEIL, // Triage now innate; chosen Sweet Veil keeps the doubles team awake
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
        .heldItem = ITEM_LIFE_ORB, // Triage Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_DRAINING_KISS,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC
        },
        .ability = ABILITY_SWEET_VEIL, // Triage now innate; chosen Sweet Veil keeps the doubles team awake
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
        .heldItem = ITEM_LEFTOVERS, // Trick Room setter / Instruct support
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_INSTRUCT,
            MOVE_PSYCHIC,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_SYMBIOSIS, // Inner Focus now innate; chosen Symbiosis (real slot) passes items in doubles
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_FOCUS_BLAST,
            MOVE_THUNDERBOLT,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SYMBIOSIS, // Telepathy now innate; chosen Symbiosis (:x:, never an innate -> stable) passes its item to the ally
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
        .heldItem = ITEM_CHOICE_SCARF, // Defiant revenge killer
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_GUNK_SHOT
        },
        .ability = ABILITY_RIVALRY, // all real abilities innate; chosen Rivalry (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // physical breaker
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_RIVALRY, // all real abilities innate; chosen Rivalry (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Emergency Exit First Impression band
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
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
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
        .heldItem = ITEM_LEFTOVERS, // Water Compaction defensive trapper
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_TOXIC,
            MOVE_SHORE_UP
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // recovery for the defensive Ghost/Ground trapper
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_GIGA_DRAIN,
            MOVE_SHORE_UP
        },
        .ability = ABILITY_EARTH_EATER, // all real abilities innate; chosen Earth Eater (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Unaware Counter/Toxic staller
        .moves =
        {
            MOVE_COUNTER,
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_SOAK
        },
        .ability = ABILITY_WATER_ABSORB, // Innards Out/Unaware both now innate; chosen Water Absorb (slot-1 override) heals the Counter staller off Water hits
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
        .heldItem = ITEM_DRAGON_MEMORY, // RKS System Dragon pivot
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
        .heldItem = ITEM_FAIRY_MEMORY, // defensive pivot
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
        .heldItem = ITEM_STEEL_MEMORY, // Steel attacker
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
        .heldItem = ITEM_FOCUS_BAND, // Shields Down Shell Smash sweeper
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
        .heldItem = ITEM_WHITE_HERB, // Shell Smash, White Herb restores drops
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
        .heldItem = ITEM_CHOICE_BAND, // innate Comatose keeps it status-immune; chosen Sticky Hold keeps its Choice Band
        .moves =
        {
            MOVE_RETURN,
            MOVE_KNOCK_OFF,
            MOVE_EARTHQUAKE,
            MOVE_SUPERPOWER
        },
        .ability = ABILITY_STICKY_HOLD,
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
        .heldItem = ITEM_LEFTOVERS, // Shell Trap defensive attacker
        .moves =
        {
            MOVE_SHELL_TRAP,
            MOVE_FLAMETHROWER,
            MOVE_DRAGON_PULSE,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_FLAME_BODY, // Shell Armor now innate; chosen Flame Body (override) burns on contact
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Fire/Dragon special attacker
        .moves =
        {
            MOVE_FIRE_BLAST,
            MOVE_DRACO_METEOR,
            MOVE_EARTH_POWER,
            MOVE_FLASH_CANNON
        },
        .ability = ABILITY_FLAME_BODY, // Shell Armor now innate; chosen Flame Body (override) burns on contact
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
        .heldItem = ITEM_LIFE_ORB, // Iron Barbs / Lightning Rod attacker
        .moves =
        {
            MOVE_ZING_ZAP,
            MOVE_IRON_HEAD,
            MOVE_U_TURN,
            MOVE_NUZZLE
        },
        .ability = ABILITY_LIGHTNING_ROD, // Iron Barbs now innate; chosen Lightning Rod (real slot 1, :x: stable) draws in Electric moves
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
        .heldItem = ITEM_FOCUS_SASH, // Lightning Rod redirect support
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
        .heldItem = ITEM_LIFE_ORB, // power for the Disguise attacker
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
        .heldItem = ITEM_LIFE_ORB, // Disguise sweeper
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
        .heldItem = ITEM_LUM_BERRY, // Disguise disruptor
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
        .heldItem = ITEM_LIFE_ORB, // Strong Jaw attacker
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_LIQUIDATION,
            MOVE_CRUNCH,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SHEER_FORCE, // Strong Jaw + Dazzling now innate; chosen Sheer Force (slot-1 override) powers up the fangs
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
        .heldItem = ITEM_CHOICE_SCARF, // Wonder Skin / disruption pivot
        .moves =
        {
            MOVE_PSYCHIC_FANGS,
            MOVE_LIQUIDATION,
            MOVE_FLIP_TURN,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_SHEER_FORCE, // Strong Jaw + Dazzling now innate; chosen Sheer Force (slot-1 override) powers up the fangs
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
        .heldItem = ITEM_CHOICE_SPECS, // Berserk special breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_HYPER_VOICE,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SAP_SIPPER, // all real abilities innate; chosen Sap Sipper (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Berserk Roost staller
        .moves =
        {
            MOVE_HYPER_VOICE,
            MOVE_FLAMETHROWER,
            MOVE_ROOST,
            MOVE_GLARE
        },
        .ability = ABILITY_SAP_SIPPER, // all real abilities innate; chosen Sap Sipper (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Steelworker Anchor Shot band
        .moves =
        {
            MOVE_ANCHOR_SHOT,
            MOVE_POWER_WHIP,
            MOVE_EARTHQUAKE,
            MOVE_SHADOW_CLAW
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room trapper
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves =
        {
            MOVE_ANCHOR_SHOT,
            MOVE_POWER_WHIP,
            MOVE_SHADOW_CLAW,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_DRAGON_GEM, // one-shot Dragon burst after Clangorous setup
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up / Body Press setup
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
        .heldItem = ITEM_LOADED_DICE, // Dragon Dance physical sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_SCALE_SHOT,
            MOVE_CLOSE_COMBAT,
            MOVE_POISON_JAB
        },
        .ability = ABILITY_BULLETPROOF, // Overcoat now innate; chosen Bulletproof (real slot) deflects ball/bomb moves
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0785 (innate Levitate)
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM, // one-shot Electric burst nuke
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
        .heldItem = ITEM_CHOICE_SPECS, // Electric Surge breaker
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
        .heldItem = ITEM_LIFE_ORB, // Electric Terrain sweeper
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

    // 0786 (innate Levitate)
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Psychic Surge breaker
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
        .heldItem = ITEM_PSYCHIC_SEED, // Calm Mind sweeper, terrain-boosted SpD
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
        .heldItem = ITEM_PSYCHIC_GEM, // one-shot Psychic burst nuke
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

    // 0787 (innate Levitate)
    {
        .species = SPECIES_TAPU_BULU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Grassy Surge band
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
        .heldItem = ITEM_GRASSY_SEED, // Swords Dance bulky setup
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

    // 0788 (innate Levitate)
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Misty Surge Calm Mind wall
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
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
        .heldItem = ITEM_MISTY_SEED, // bulky support, terrain-boosted SpD
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
        .heldItem = ITEM_LIFE_ORB, // Full Metal Body physical attacker
        .moves =
        {
            MOVE_SUNSTEEL_STRIKE,
            MOVE_CLOSE_COMBAT,
            MOVE_EARTHQUAKE,
            MOVE_FLARE_BLITZ
        },
        .ability = ABILITY_TOUGH_CLAWS, // Full Metal Body now innate; chosen Tough Claws powers its contact STAB
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
        .heldItem = ITEM_LEFTOVERS, // bulky setup pivot
        .moves =
        {
            MOVE_SUNSTEEL_STRIKE,
            MOVE_MORNING_SUN,
            MOVE_CALM_MIND,
            MOVE_FLAMETHROWER
        },
        .ability = ABILITY_TOUGH_CLAWS, // Full Metal Body now innate; chosen Tough Claws powers its Sunsteel Strike contact STAB
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
        .heldItem = ITEM_GHOST_GEM, // one-shot Ghost burst nuke
        .moves =
        {
            MOVE_MOONGEIST_BEAM,
            MOVE_SHADOW_BALL,
            MOVE_MOONBLAST,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_ADAPTABILITY, // Shadow Shield now innate; chosen Adaptability (override) doubles STAB on top of innate full-HP bulk
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
        .heldItem = ITEM_LEFTOVERS, // Shadow Shield Calm Mind tank
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONGEIST_BEAM,
            MOVE_PSYSHOCK,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_ADAPTABILITY, // Shadow Shield now innate; chosen Adaptability (override) doubles STAB on top of innate full-HP bulk
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0793 (innate Beast Boost + Levitate)
    {
        .species = SPECIES_NIHILEGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Beast Boost special breaker
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_POWER_GEM,
            MOVE_THUNDERBOLT,
            MOVE_GRASS_KNOT
        },
        .ability = ABILITY_MERCILESS, // Beast Boost now innate (Y7); chosen Merciless (override) auto-crits its poisoned targets
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
        .heldItem = ITEM_BLACK_SLUDGE, // special wall / status spreader
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_POWER_GEM,
            MOVE_TOXIC_SPIKES,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_MERCILESS, // Beast Boost now innate (Y7); chosen Merciless (override) auto-crits its Toxic Spikes targets
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 252,
            .spe = 4
        ),
        .teraType = TYPE_POISON,
    },

    // 0794 (innate Beast Boost)
    {
        .species = SPECIES_BUZZWOLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Beast Boost physical breaker
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_LEECH_LIFE,
            MOVE_ICE_PUNCH,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_IRON_FIST, // Beast Boost now innate (Y7); chosen Iron Fist (override) powers its punch kit
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up bulky setup
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_LEECH_LIFE,
            MOVE_ICE_PUNCH
        },
        .ability = ABILITY_IRON_FIST, // Beast Boost now innate (Y7); chosen Iron Fist (override) powers Drain/Ice Punch
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0795 (innate Beast Boost)
    {
        .species = SPECIES_PHEROMOSA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Beast Boost glass cannon
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_BUG_BUZZ,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Beast Boost now innate (Y7); chosen Tough Claws (override) powers its contact STAB
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
        .heldItem = ITEM_FOCUS_BAND, // fast lead
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_BEAM,
            MOVE_THUNDERBOLT,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Beast Boost now innate (Y7); chosen Tough Claws (override) powers Close Combat/Rapid Spin
        .nature = NATURE(SPE_UP, SPD_DOWN),
        .ev = EVS(
            .atk = 252,
            .spa = 4,
            .spe = 252
        ),
        .teraType = TYPE_ICE,
    },

    // 0796 (innate Beast Boost + Levitate)
    {
        .species = SPECIES_XURKITREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIC_GEM, // one-shot Electric burst nuke
        .moves =
        {
            MOVE_TAIL_GLOW,
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_LIGHTNING_ROD, // Beast Boost now innate (Y7); chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
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
        .heldItem = ITEM_CHOICE_SCARF, // Beast Boost revenge killer
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_ENERGY_BALL,
            MOVE_DAZZLING_GLEAM,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_LIGHTNING_ROD, // Beast Boost now innate (Y7); chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0797 (innate Beast Boost)
    {
        .species = SPECIES_CELESTEELA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Beast Boost defensive wall
        .moves =
        {
            MOVE_LEECH_SEED,
            MOVE_PROTECT,
            MOVE_FLAMETHROWER,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_FILTER, // Beast Boost now innate (Y7); chosen Filter (override) blunts supereffective Fire/Electric
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Autotomize sweeper
        .moves =
        {
            MOVE_AUTOTOMIZE,
            MOVE_HEAVY_SLAM,
            MOVE_FLAMETHROWER,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_FILTER, // Beast Boost now innate (Y7); chosen Filter (override) blunts supereffective Fire/Electric
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
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_GRASS,
    },

    // 0799 (innate Beast Boost)
    {
        .species = SPECIES_GUZZLORD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // massive HP mixed tank
        .moves =
        {
            MOVE_KNOCK_OFF,
            MOVE_DRAGON_TAIL,
            MOVE_HEAVY_SLAM,
            MOVE_REST
        },
        .ability = ABILITY_FILTER, // Beast Boost now innate (Y7); chosen Filter (override) blunts its 4x Fairy weakness
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank attacker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_DARK_PULSE,
            MOVE_FLAMETHROWER,
            MOVE_SLUDGE_BOMB
        },
        .ability = ABILITY_FILTER, // Beast Boost now innate (Y7); chosen Filter (override) blunts its 4x Fairy weakness
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_DARK,
    },

    // 0800 (innate Levitate)
    {
        .species = SPECIES_NECROZMA_DUSK_MANE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance physical sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SUNSTEEL_STRIKE,
            MOVE_EARTHQUAKE,
            MOVE_PHOTON_GEYSER
        },
        .ability = ABILITY_ADAPTABILITY, // Prism Armor now innate; chosen Adaptability (override) doubles STAB on top of innate SE-damage cut
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
        .heldItem = ITEM_LIFE_ORB, // Calm Mind special sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONGEIST_BEAM,
            MOVE_PHOTON_GEYSER,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_ADAPTABILITY, // Prism Armor now innate; chosen Adaptability (override) doubles STAB on top of innate SE-damage cut
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
        .heldItem = ITEM_PSYCHIC_GEM, // Psychic burst for the Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PHOTON_GEYSER,
            MOVE_HEAT_WAVE,
            MOVE_MOONLIGHT
        },
        .ability = ABILITY_ADAPTABILITY, // Prism Armor now innate; chosen Adaptability (override) doubles STAB on top of innate SE-damage cut
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0801 (innate Levitate)
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Soul-Heart Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_SOUL_HEART,
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves =
        {
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_VOLT_SWITCH,
            MOVE_AURA_SPHERE
        },
        .ability = ABILITY_SOUL_HEART,
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
        .heldItem = ITEM_FAIRY_GEM, // one-shot Fairy burst for the Trick Room nuke
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_FLEUR_CANNON,
            MOVE_FLASH_CANNON,
            MOVE_THUNDERBOLT
        },
        .ability = ABILITY_SOUL_HEART,
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
        .heldItem = ITEM_GHOST_GEM, // one-shot Ghost burst for the Spectral Thief nuke
        .moves =
        {
            MOVE_SPECTRAL_THIEF,
            MOVE_CLOSE_COMBAT,
            MOVE_SHADOW_SNEAK,
            MOVE_BULK_UP
        },
        .ability = ABILITY_ILLUSION, // Technician now innate; chosen Illusion (override) disguises the shadow
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
        .heldItem = ITEM_LIFE_ORB, // Technician priority sweeper
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_SPECTRAL_THIEF,
            MOVE_MACH_PUNCH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_ILLUSION, // Technician now innate; chosen Illusion (override) disguises the shadow
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0804 (innate Beast Boost)
    {
        .species = SPECIES_NAGANADEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Beast Boost Nasty Plot sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SLUDGE_WAVE,
            MOVE_FIRE_BLAST,
            MOVE_DRACO_METEOR
        },
        .ability = ABILITY_SHEER_FORCE, // Beast Boost now innate (Y7); chosen Sheer Force (override) powers its Nasty Plot sweeper
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_SLUDGE_WAVE,
            MOVE_DRACO_METEOR,
            MOVE_FIRE_BLAST,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE, // Beast Boost now innate (Y7); chosen Sheer Force (override) powers Sludge Wave/Fire Blast/Draco Meteor
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_DRAGON,
    },

    // 0805 (innate Beast Boost)
    {
        .species = SPECIES_STAKATAKA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Trick Room Beast Boost wallbreaker
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves =
        {
            MOVE_GYRO_BALL,
            MOVE_ROCK_SLIDE,
            MOVE_EARTHQUAKE,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_SOLID_ROCK, // Beast Boost now innate (Y7); chosen Solid Rock (override) blunts supereffective hits
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
        .heldItem = ITEM_LEFTOVERS, // physical wall / hazard setter
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_GYRO_BALL,
            MOVE_BODY_PRESS,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_SOLID_ROCK, // Beast Boost now innate (Y7); chosen Solid Rock (override) blunts supereffective hits
        .nature = NATURE(DEF_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_STEEL,
    },

    // 0806 (innate Beast Boost + Levitate)
    {
        .species = SPECIES_BLACEPHALON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Beast Boost revenge killer
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_FIRE_BLAST,
            MOVE_PSYCHIC,
            MOVE_TRICK
        },
        .ability = ABILITY_INFILTRATOR, // Beast Boost now innate (Y7); chosen Infiltrator (override) ignores screens/Substitute
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
        .heldItem = ITEM_FIRE_GEM, // one-shot Fire burst nuke
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_FIRE_BLAST,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_INFILTRATOR, // Beast Boost now innate (Y7); chosen Infiltrator (override) ignores screens/Substitute
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
        .heldItem = ITEM_LIFE_ORB, // Volt Absorb fast physical attacker
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
        .heldItem = ITEM_ELECTRIC_GEM, // one-shot Electric burst after Bulk Up
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
        .heldItem = ITEM_LEFTOVERS, // bulky Iron Fist breaker
        .moves =
        {
            MOVE_DOUBLE_IRON_BASH,
            MOVE_THUNDER_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_FILTER, // Iron Fist now innate; chosen Filter (override)
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
        .heldItem = ITEM_ASSAULT_VEST, // special bulk tank
        .moves =
        {
            MOVE_DOUBLE_IRON_BASH,
            MOVE_THUNDER_PUNCH,
            MOVE_SUPERPOWER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_FILTER, // Iron Fist now innate; chosen Filter (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Grassy Surge band breaker
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance terrain sweeper
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
        .heldItem = ITEM_LEFTOVERS, // bulky Grassy Terrain pivot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Libero offensive pivot
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
        .heldItem = ITEM_CHOICE_BAND, // Libero band breaker
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
        .heldItem = ITEM_LIFE_ORB, // Court Change utility sweeper
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
        .heldItem = ITEM_CHOICE_SPECS, // Sniper special breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_DARK_PULSE,
            MOVE_U_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_SCOPE_LENS, // Sniper guaranteed-crit Snipe Shot
        .moves =
        {
            MOVE_SNIPE_SHOT,
            MOVE_ICE_BEAM,
            MOVE_DARK_PULSE,
            MOVE_AIR_SLASH
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_ICE_BEAM,
            MOVE_U_TURN,
            MOVE_DARK_PULSE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_SITRUS_BERRY, // Cheek Pouch heal loop
        .moves =
        {
            MOVE_BODY_SLAM,
            MOVE_EARTHQUAKE,
            MOVE_SWORDS_DANCE,
            MOVE_BULLET_SEED
        },
        .ability = ABILITY_PICKUP, // Cheek Pouch + Gluttony now innate (Cheek Pouch still runs the heal loop); chosen Pickup (override, empty slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Pressure Defog wall
        .moves =
        {
            MOVE_BODY_PRESS,
            MOVE_ROOST,
            MOVE_DEFOG,
            MOVE_IRON_DEFENSE
        },
        .ability = ABILITY_BULLETPROOF, // Pressure / Unnerve / Mirror Armor all now innate (Tier 5.7); chosen Bulletproof (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_ROCKY_HELMET, // Mirror Armor physical wall
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_BODY_PRESS,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF, // Pressure / Unnerve / Mirror Armor all now innate (Tier 5.7); chosen Bulletproof (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_BULK_UP,
            MOVE_ROOST,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF, // Pressure / Unnerve / Mirror Armor all now innate (Tier 5.7); chosen Bulletproof (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_LIGHT_CLAY, // dual screens support
        .moves =
        {
            MOVE_LIGHT_SCREEN,
            MOVE_REFLECT,
            MOVE_PSYCHIC,
            MOVE_STICKY_WEB
        },
        .ability = ABILITY_UNAWARE, // Swarm/Frisk/Telepathy all now innate; chosen Unaware (fork override) on the dual-screens pivot
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
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC,
            MOVE_BUG_BUZZ,
            MOVE_ROOST
        },
        .ability = ABILITY_UNAWARE, // Swarm/Frisk/Telepathy all now innate; chosen Unaware (fork override) ignores setup on the bulky Calm Mind sweeper
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
        .heldItem = ITEM_BLACK_GLASSES, // Nasty Plot special sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_DARK_PULSE,
            MOVE_PARTING_SHOT,
            MOVE_FOUL_PLAY
        },
        .ability = ABILITY_RUN_AWAY, // Unburden + Stakeout now innate; Run Away is its only stable non-innate real slot
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator support pivot
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
        .heldItem = ITEM_LEFTOVERS, // Cotton Guard Body Press wall
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
        .heldItem = ITEM_LUM_BERRY, // status insurance for setup
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_HARD_STONE, // jaw-boosted Crunch bite set
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_LIQUIDATION,
            MOVE_STONE_EDGE,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_MAGNET, // Strong Jaw Bolt Beak
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_LIGHTNING_ROD, // all real abilities innate; chosen Lightning Rod (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Steam Engine bulky hazards
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Steam Engine sweeper
        .moves =
        {
            MOVE_FLAMETHROWER,
            MOVE_STONE_EDGE,
            MOVE_EARTH_POWER,
            MOVE_HEAT_CRASH
        },
        .ability = ABILITY_FLASH_FIRE, // Steam Engine now innate; chosen Flash Fire adds Fire immunity + a Fire-power boost
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
        .heldItem = ITEM_LIFE_ORB, // Hustle physical attacker
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
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
        .heldItem = ITEM_LEFTOVERS, // Thick Fat bulky special wall
        .moves =
        {
            MOVE_APPLE_ACID,
            MOVE_DRAGON_PULSE,
            MOVE_RECOVER,
            MOVE_LEECH_SEED
        },
        // Ripen/Gluttony/Thick Fat all now innate; chosen Filter blunts its 4x Ice weakness (fork override).
        .ability = ABILITY_FILTER,
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves =
        {
            MOVE_APPLE_ACID,
            MOVE_DRACO_METEOR,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        // Ripen/Gluttony/Thick Fat all now innate; chosen Filter blunts its 4x Ice weakness (fork override).
        .ability = ABILITY_FILTER,
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
        .heldItem = ITEM_LEFTOVERS, // Sand Spit Coil wall
        .moves =
        {
            MOVE_COIL,
            MOVE_EARTHQUAKE,
            MOVE_STONE_EDGE,
            MOVE_GLARE
        },
        .ability = ABILITY_SAND_SPIT, // Shed Skin now innate (30% self-cure); chosen Sand Spit summons sand (feeds innate Sand Veil)
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
        .heldItem = ITEM_ROCKY_HELMET, // bulky hazard lead
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Gulp Missile spam
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
        .heldItem = ITEM_CHOICE_BAND, // Swift Swim band breaker
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_PSYCHIC_FANGS,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Propeller Tail now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_CHOICE_SCARF, // fast revenge killer
        .moves =
        {
            MOVE_LIQUIDATION,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim + Propeller Tail now innate; chosen Water Absorb (override) heals off Water
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
        .heldItem = ITEM_THROAT_SPRAY, // Punk Rock Boomburst nuke (Amped)
        .moves =
        {
            MOVE_BOOMBURST,
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH
        },
        .ability = ABILITY_VOLT_ABSORB, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_SPECS, // Punk Rock special breaker
        .moves =
        {
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH,
            MOVE_FOCUS_BLAST
        },
        .ability = ABILITY_VOLT_ABSORB, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_SCARF, // Low Key revenge killer
        .moves =
        {
            MOVE_OVERDRIVE,
            MOVE_SLUDGE_WAVE,
            MOVE_VOLT_SWITCH,
            MOVE_BOOMBURST
        },
        .ability = ABILITY_VOLT_ABSORB, // all real abilities innate; chosen Volt Absorb (non-redundant)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Flash Fire bulky attacker
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
        .heldItem = ITEM_THROAT_SPRAY, // Fiery Dance / Coil setup
        .moves =
        {
            MOVE_COIL,
            MOVE_FLARE_BLITZ,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_FLASH_FIRE, // White Smoke now innate; chosen Flash Fire powers its Fire STAB
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
        .heldItem = ITEM_SITRUS_BERRY, // Technician punches + Bulk Up
        .moves =
        {
            MOVE_OCTOLOCK,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_BULK_UP
        },
        .ability = ABILITY_WATER_ABSORB, // Limber + Technician now innate; chosen Water Absorb (override) heals off Water moves
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
        .heldItem = ITEM_ASSAULT_VEST, // priority Mach Punch attacker
        .moves =
        {
            MOVE_MACH_PUNCH,
            MOVE_DRAIN_PUNCH,
            MOVE_ICE_PUNCH,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_WATER_ABSORB, // Limber + Technician now innate; chosen Water Absorb (override) heals off Water moves
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 160,
            .atk = 252,
            .spd = 96
        ),
        .teraType = TYPE_ICE,
    },

    // 0855 (innate Levitate)
    {
        .species = SPECIES_POLTEAGEIST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shell Smash Stored Power sweeper
        .moves =
        {
            MOVE_SHELL_SMASH,
            MOVE_STORED_POWER,
            MOVE_SHADOW_BALL,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_WEAK_ARMOR, // Cursed Body now innate; chosen Weak Armor adds Speed on a physical hit
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
        .heldItem = ITEM_FOCUS_BAND, // Shell Smash sash sweeper
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

    // 0858 (innate Magic Bounce)
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Magic Bounce Calm Mind wall
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_UNAWARE, // Healer / Anticipation / Magic Bounce all now innate (Tier 5.8); chosen Unaware (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_LIFE_ORB, // Psychic Terrain breaker
        .moves =
        {
            MOVE_EXPANDING_FORCE,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_PROTECT
        },
        .ability = ABILITY_UNAWARE, // Healer / Anticipation / Magic Bounce all now innate (Tier 5.8); chosen Unaware (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM,
            MOVE_MYSTICAL_FIRE,
            MOVE_POWER_WHIP
        },
        .ability = ABILITY_UNAWARE, // Healer / Anticipation / Magic Bounce all now innate (Tier 5.8); chosen Unaware (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_LIGHT_CLAY, // Prankster dual screens lead
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_SPIRIT_BREAK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_INFILTRATOR, // Prankster / Frisk / Pickpocket all innate; chosen Infiltrator (override)
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
        .heldItem = ITEM_LIFE_ORB, // Bulk Up physical sweeper
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_SPIRIT_BREAK,
            MOVE_SUCKER_PUNCH,
            MOVE_DRAIN_PUNCH
        },
        .ability = ABILITY_INFILTRATOR, // Prankster / Frisk / Pickpocket all innate; chosen Infiltrator (override)
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
        .heldItem = ITEM_MENTAL_HERB, // Prankster support / Taunt-proof
        .moves =
        {
            MOVE_SPIRIT_BREAK,
            MOVE_THUNDER_WAVE,
            MOVE_TAUNT,
            MOVE_PARTING_SHOT
        },
        .ability = ABILITY_INFILTRATOR, // Prankster / Frisk / Pickpocket all innate; chosen Infiltrator (override)
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
        .heldItem = ITEM_FLAME_ORB, // Guts Facade breaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_CLOSE_COMBAT,
            MOVE_OBSTRUCT
        },
        .ability = ABILITY_QUICK_FEET, // all real abilities innate; chosen Quick Feet (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Guts wall-breaker
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_FACADE,
            MOVE_KNOCK_OFF,
            MOVE_OBSTRUCT
        },
        .ability = ABILITY_QUICK_FEET, // all real abilities innate; chosen Quick Feet (non-redundant)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0863
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Steely Spirit band breaker
        .moves =
        {
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_BULLETPROOF, // all real abilities innate; chosen Bulletproof (non-redundant)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Swords Dance
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IRON_HEAD,
            MOVE_CLOSE_COMBAT,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF, // all real abilities innate; chosen Bulletproof (non-redundant)
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .def = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0864 (innate Perish Body)
    {
        .species = SPECIES_CURSOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Perish Body bulky special attacker
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
        .heldItem = ITEM_LIFE_ORB, // glass cannon
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
        .heldItem = ITEM_LEEK, // guaranteed-crit Leaf Blade Scrappy fighter
        .moves =
        {
            MOVE_CLOSE_COMBAT,
            MOVE_LEAF_BLADE,
            MOVE_KNOCK_OFF,
            MOVE_FIRST_IMPRESSION
        },
        .ability = ABILITY_SUPER_LUCK, // Scrappy now innate; chosen Super Luck (empty-slot override) stacks with the Leek for guaranteed crits
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_BRAVE_BIRD
        },
        .ability = ABILITY_SUPER_LUCK, // Scrappy now innate; chosen Super Luck (empty-slot override)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0866
    {
        .species = SPECIES_MR_RIME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIGHT_CLAY, // Aurora Veil support
        .moves =
        {
            MOVE_AURORA_VEIL,
            MOVE_ICE_BEAM,
            MOVE_PSYCHIC,
            MOVE_NASTY_PLOT
        },
        .ability = ABILITY_SCREEN_CLEANER,
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
        .heldItem = ITEM_LEFTOVERS, // bulky Nasty Plot
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_FREEZE_DRY,
            MOVE_FOCUS_BLAST,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_SCREEN_CLEANER, // Ice Body now innate (snow heal); chosen Screen Cleaner wipes foe screens on entry
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spe = 4
        ),
        .teraType = TYPE_PSYCHIC,
    },

    // 0867 (innate Levitate)
    {
        .species = SPECIES_RUNERIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Wandering Spirit bulky hazards
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
        .heldItem = ITEM_LEFTOVERS, // Iron Defense Body Press wall
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

    // 0870
    {
        .species = SPECIES_FALINKS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // No Retreat setup sweeper
        .moves =
        {
            MOVE_NO_RETREAT,
            MOVE_CLOSE_COMBAT,
            MOVE_IRON_HEAD,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_WHITE_HERB, // No Retreat sweeper with herb reset
        .moves =
        {
            MOVE_NO_RETREAT,
            MOVE_CLOSE_COMBAT,
            MOVE_KNOCK_OFF,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_NO_GUARD, // all real abilities innate; chosen No Guard (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // terrain setter
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
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_DIVE,
    },

    // 0873
    {
        .species = SPECIES_FROSMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Quiver Dance sweeper
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_ICE_BEAM,
            MOVE_BUG_BUZZ,
            MOVE_GIGA_DRAIN
        },
        .ability = ABILITY_SNOW_WARNING, // Shield Dust & Ice Scales now innate; chosen Snow Warning (empty-slot override) — the frost moth heralds snow (Ice-type Def boost)
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
        .heldItem = ITEM_LEFTOVERS, // Ice Scales special wall
        .moves =
        {
            MOVE_QUIVER_DANCE,
            MOVE_ICE_BEAM,
            MOVE_HURRICANE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_SNOW_WARNING, // Shield Dust & Ice Scales now innate; chosen Snow Warning (empty-slot override)
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
        .heldItem = ITEM_SITRUS_BERRY, // Power Spot ally booster
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_STONE_EDGE,
            MOVE_EARTHQUAKE,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SOLID_ROCK, // Power Spot (doubles-only) now innate; chosen Solid Rock (fork override, species_ability_overrides.c) blunts its many weaknesses
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
        .heldItem = ITEM_LEFTOVERS, // Ice Face Belly Drum sweeper
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
        .heldItem = ITEM_CHOICE_BAND, // Ice Face band attacker
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
        .heldItem = ITEM_LIFE_ORB, // Psychic Surge attacker
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
        .heldItem = ITEM_LEFTOVERS, // Healer redirect support (Indeedee-F)
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
        .heldItem = ITEM_LIFE_ORB, // Hunger Switch Aura Wheel attacker
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
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force Heavy Slam breaker
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
        .heldItem = ITEM_LEFTOVERS, // bulky hazard setter
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_HEAVY_SLAM,
            MOVE_HIGH_HORSEPOWER,
            MOVE_WHIRLWIND
        },
        .ability = ABILITY_SHEER_FORCE, // Heavy Metal now innate; chosen Sheer Force
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
        .heldItem = ITEM_CHOICE_BAND, // Hustle Bolt Beak nuke
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
        .heldItem = ITEM_LIFE_ORB, // Sand Rush sweeper
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_OUTRAGE,
            MOVE_EARTHQUAKE,
            MOVE_ROCK_SLIDE
        },
        .ability = ABILITY_VOLT_ABSORB, // Sand Rush now innate; chosen Volt Absorb
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
        .heldItem = ITEM_LIFE_ORB, // Slush Rush Bolt Beak nuke
        .moves =
        {
            MOVE_BOLT_BEAK,
            MOVE_ICICLE_CRASH,
            MOVE_LOW_KICK,
            MOVE_BLIZZARD
        },
        .ability = ABILITY_VOLT_ABSORB, // Slush Rush now innate; chosen Volt Absorb
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
        .heldItem = ITEM_CHOICE_SPECS, // Static special breaker
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
        .heldItem = ITEM_CHOICE_SCARF, // Strong Jaw Fishious Rend nuke
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_CRUNCH,
            MOVE_PSYCHIC_FANGS,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB, // Sand Rush & Strong Jaw now innate; chosen Water Absorb
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
        .heldItem = ITEM_CHOICE_BAND, // Sand Rush band breaker
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_CRUNCH,
            MOVE_EARTHQUAKE,
            MOVE_ICE_FANG
        },
        .ability = ABILITY_WATER_ABSORB, // Sand Rush & Strong Jaw now innate; chosen Water Absorb
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
        .heldItem = ITEM_CHOICE_BAND, // Slush Rush Fishious Rend
        .moves =
        {
            MOVE_FISHIOUS_REND,
            MOVE_ICICLE_CRASH,
            MOVE_PSYCHIC_FANGS,
            MOVE_CRUNCH
        },
        .ability = ABILITY_WATER_ABSORB, // Slush Rush & Ice Body now innate; chosen Water Absorb heals off Water moves
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
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky pivot
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

    // 0887 (innate Levitate)
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Draco/Shadow Ball breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_SHADOW_BALL,
            MOVE_FLAMETHROWER,
            MOVE_U_TURN
        },
        .ability = ABILITY_DRAGONS_MAW, // Clear Body + Infiltrator now innate; chosen Dragon's Maw powers Draco Meteor / Dragon Darts (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Dragon Darts band breaker
        .moves =
        {
            MOVE_DRAGON_DARTS,
            MOVE_PHANTOM_FORCE,
            MOVE_U_TURN,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_DRAGONS_MAW, // Clear Body + Infiltrator now innate; chosen Dragon's Maw powers Draco Meteor / Dragon Darts (override)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Dragon Dance physical sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_DRAGON_DARTS,
            MOVE_PHANTOM_FORCE,
            MOVE_FIRE_BLAST
        },
        .ability = ABILITY_DRAGONS_MAW, // Clear Body + Infiltrator now innate; chosen Dragon's Maw powers Draco Meteor / Dragon Darts (override)
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
        .heldItem = ITEM_RUSTED_SWORD, // Intrepid Sword Behemoth Blade
        .moves =
        {
            MOVE_BEHEMOTH_BLADE,
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_TOUGH_CLAWS, // FORK: innate Intrepid Sword; chosen slot freed to Tough Claws
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
        .heldItem = ITEM_CHOICE_BAND, // base Zacian band breaker
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_TOUGH_CLAWS, // FORK: innate Intrepid Sword; chosen slot freed to Tough Claws
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
        .heldItem = ITEM_RUSTED_SHIELD, // Dauntless Shield Body Press wall
        .moves =
        {
            MOVE_BEHEMOTH_BASH,
            MOVE_BODY_PRESS,
            MOVE_IRON_DEFENSE,
            MOVE_CRUNCH
        },
        .ability = ABILITY_FILTER, // FORK: innate Dauntless Shield; chosen slot freed to Filter
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
        .heldItem = ITEM_LEFTOVERS, // base Zamazenta bulky setup
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_CLOSE_COMBAT,
            MOVE_CRUNCH
        },
        .ability = ABILITY_FILTER, // FORK: innate Dauntless Shield; chosen slot freed to Filter
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
        .heldItem = ITEM_CHOICE_SPECS, // Pressure special breaker
        .moves =
        {
            MOVE_DYNAMAX_CANNON,
            MOVE_SLUDGE_WAVE,
            MOVE_FLAMETHROWER,
            MOVE_DRACO_METEOR
        },
        .ability = ABILITY_POISON_TOUCH, // Pressure now innate; chosen Poison Touch poisons on contact (slot-1 override)
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
        .heldItem = ITEM_BLACK_SLUDGE, // bulky Toxic Spikes / Cosmic Power
        .moves =
        {
            MOVE_DYNAMAX_CANNON,
            MOVE_FLAMETHROWER,
            MOVE_TOXIC_SPIKES,
            MOVE_RECOVER
        },
        .ability = ABILITY_POISON_TOUCH, // Pressure now innate; chosen Poison Touch poisons on contact (slot-1 override)
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
        .heldItem = ITEM_CHOICE_BAND, // Unseen Fist Wicked Blow breaker
        .moves =
        {
            MOVE_WICKED_BLOW,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SNIPER, // Unseen Fist now innate; chosen Sniper pays off always-crit Wicked Blow (override)
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_WICKED_BLOW,
            MOVE_CLOSE_COMBAT,
            MOVE_THUNDER_PUNCH
        },
        .ability = ABILITY_SNIPER, // Unseen Fist now innate; chosen Sniper pays off always-crit Wicked Blow (override)
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
        .heldItem = ITEM_CHOICE_BAND, // Surging Strikes always-crit breaker
        .moves =
        {
            MOVE_SURGING_STRIKES,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET,
            MOVE_U_TURN
        },
        .ability = ABILITY_SNIPER, // Unseen Fist now innate; chosen Sniper pays off always-crit Surging Strikes (override)
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
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_SURGING_STRIKES,
            MOVE_CLOSE_COMBAT,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_SNIPER, // Unseen Fist now innate; chosen Sniper pays off always-crit Surging Strikes (override)
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
        .heldItem = ITEM_LEFTOVERS, // Swords Dance + recovery
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_POWER_WHIP,
            MOVE_DARKEST_LARIAT,
            MOVE_JUNGLE_HEALING
        },
        .ability = ABILITY_TOUGH_CLAWS, // Leaf Guard now innate; chosen Tough Claws (override) powers its contact kit
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
        .heldItem = ITEM_BLACK_GLASSES, // Dark-boosted pivot
        .moves =
        {
            MOVE_POWER_WHIP,
            MOVE_DARKEST_LARIAT,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN
        },
        .ability = ABILITY_TOUGH_CLAWS, // Leaf Guard now innate; chosen Tough Claws (override) powers its contact kit
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .def = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },

    // 0894 (innate Transistor + Levitate; chosen Lightning Rod override)
    {
        .species = SPECIES_REGIELEKI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // innate-Transistor electric nuke
        .moves =
        {
            MOVE_THUNDERBOLT,
            MOVE_VOLT_SWITCH,
            MOVE_RISING_VOLTAGE,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_LIGHTNING_ROD, // Transistor now innate; chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
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
        .heldItem = ITEM_LIGHT_CLAY, // screens lead
        .moves =
        {
            MOVE_REFLECT,
            MOVE_LIGHT_SCREEN,
            MOVE_THUNDERBOLT,
            MOVE_EXPLOSION
        },
        .ability = ABILITY_LIGHTNING_ROD, // Transistor now innate; chosen Lightning Rod (override) draws Electric for immunity + Sp. Atk
        .nature = NATURE(SPE_UP, ATK_DOWN),
        .ev = EVS(
            .spa = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_ELECTRIC,
    },

    // 0895 (innate Dragon's Maw; chosen Adaptability override)
    {
        .species = SPECIES_REGIDRAGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // innate-Dragon's-Maw + Adaptability Dragon Energy breaker
        .moves =
        {
            MOVE_DRAGON_CLAW,
            MOVE_EARTHQUAKE,
            MOVE_OUTRAGE,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_ADAPTABILITY, // Dragon's Maw now innate; chosen Adaptability (override) stacks 2x STAB on its 1.5x Dragon boost
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
        .heldItem = ITEM_CHOICE_SPECS, // special Dragon Energy breaker
        .moves =
        {
            MOVE_DRACO_METEOR,
            MOVE_DRAGON_PULSE,
            MOVE_THUNDERBOLT,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_ADAPTABILITY, // Dragon's Maw now innate; chosen Adaptability (override) stacks 2x STAB on its 1.5x Dragon boost
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
        .heldItem = ITEM_CHOICE_BAND, // Chilling Neigh band breaker
        .moves =
        {
            MOVE_ICICLE_CRASH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_CLOSE_COMBAT,
            MOVE_HEAVY_SLAM
        },
        .ability = ABILITY_SNOW_WARNING, // Chilling Neigh now innate; chosen Snow Warning sets snow (Ice-type Def)
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
        .heldItem = ITEM_LEFTOVERS, // bulky Swords Dance
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_HIGH_HORSEPOWER,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SNOW_WARNING, // Chilling Neigh now innate; chosen Snow Warning sets snow (Ice-type Def)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .spe = 4
        ),
        .teraType = TYPE_ICE,
    },

    // 0897 (innate Levitate-tier)
    {
        .species = SPECIES_SPECTRIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Grim Neigh special breaker
        .moves =
        {
            MOVE_SHADOW_BALL,
            MOVE_DARK_PULSE,
            MOVE_MYSTICAL_FIRE,
            MOVE_DRAINING_KISS
        },
        .ability = ABILITY_INFILTRATOR, // Grim Neigh now innate; chosen Infiltrator ignores screens/Substitute
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Nasty Plot snowball sweeper
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_SHADOW_BALL,
            MOVE_MYSTICAL_FIRE,
            MOVE_SUBSTITUTE
        },
        .ability = ABILITY_INFILTRATOR, // Grim Neigh now innate; chosen Infiltrator ignores screens/Substitute
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
        .heldItem = ITEM_LEFTOVERS, // As One Glacial Lance breaker
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
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_ICE,
    },
    {
        .species = SPECIES_CALYREX_ICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Chilling Neigh band breaker
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
        .heldItem = ITEM_CHOICE_SPECS, // Grim Neigh Astral Barrage nuke
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
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot Substitute sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind bulky setup
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_GIGA_DRAIN,
            MOVE_PSYCHIC,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_UNNERVE,
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
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room utility
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_POLLEN_PUFF,
            MOVE_LEECH_SEED
        },
        .ability = ABILITY_UNNERVE,
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0899
    {
        .species = SPECIES_WYRDEER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sap Sipper band attacker
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
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate special tank
        .moves =
        {
            MOVE_PSYCHIC,
            MOVE_HYPER_VOICE,
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SAP_SIPPER, // Intimidate + Frisk now innate; chosen Sap Sipper (real slot 2)
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
        .heldItem = ITEM_CHOICE_BAND, // Sharpness Stone Axe breaker
        .moves =
        {
            MOVE_STONE_AXE,
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE, // Swarm & Sharpness now innate; chosen Sheer Force
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
        .heldItem = ITEM_FOCUS_BAND, // Stone Axe hazard lead
        .moves =
        {
            MOVE_STONE_AXE,
            MOVE_X_SCISSOR,
            MOVE_CLOSE_COMBAT,
            MOVE_DEFOG
        },
        .ability = ABILITY_SHEER_FORCE, // Swarm & Sharpness now innate; chosen Sheer Force
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
        .heldItem = ITEM_FLAME_ORB, // Guts Facade wallbreaker
        .moves =
        {
            MOVE_FACADE,
            MOVE_HEADLONG_RUSH,
            MOVE_CRUNCH,
            MOVE_FIRE_PUNCH
        },
        .ability = ABILITY_BULLETPROOF, // Guts now innate; Bulletproof blocks ball/bomb moves
        .nature = NATURE(ATK_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 252,
            .def = 4
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_GROUND,
    },
    {
        .species = SPECIES_URSALUNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Swords Dance Bulk Up bruiser
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_HEADLONG_RUSH,
            MOVE_CRUNCH,
            MOVE_PROTECT
        },
        .ability = ABILITY_BULLETPROOF, // Guts now innate; Bulletproof blocks ball/bomb moves
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
        .heldItem = ITEM_LEFTOVERS, // special tank
        .moves =
        {
            MOVE_BLOOD_MOON,
            MOVE_EARTH_POWER,
            MOVE_HYPER_VOICE,
            MOVE_CALM_MIND
        },
        .ability = ABILITY_UNAWARE, // Mind's Eye now innate; chosen Unaware ignores foe boosts on its Calm Mind tank
        .nature = NATURE(SPA_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 252,
            .spd = 4
        ),
        .teraType = TYPE_GROUND,
    },

    // 0902
    {
        .species = SPECIES_BASCULEGION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Adaptability Wave Crash breaker
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_PHANTOM_FORCE,
            MOVE_AQUA_JET,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim/Adaptability/Mold Breaker all now innate; fork override frees slot 1 to a chosen Water Absorb
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
        .heldItem = ITEM_CHOICE_SPECS, // special Basculegion-F breaker
        .moves =
        {
            MOVE_HYDRO_PUMP,
            MOVE_SHADOW_BALL,
            MOVE_ICE_BEAM,
            MOVE_FLIP_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // Swift Swim/Adaptability/Mold Breaker all now innate; fork override frees slot 1 to a chosen Water Absorb
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
        .heldItem = ITEM_LEFTOVERS, // Intimidate Toxic Spikes pivot
        .moves =
        {
            MOVE_BARB_BARRAGE,
            MOVE_KNOCK_OFF,
            MOVE_TOXIC_SPIKES,
            MOVE_DESTINY_BOND
        },
        .ability = ABILITY_POISON_POINT, // Intimidate + Swift Swim now innate; chosen Poison Point (real slot 0)
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .atk = 4,
            .spe = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_OVERQWIL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Cute Charm special attacker (Incarnate)
        .moves =
        {
            MOVE_MOONBLAST,
            MOVE_EARTH_POWER,
            MOVE_SLUDGE_BOMB,
            MOVE_MYSTICAL_FIRE
        },
        .ability = ABILITY_CONTRARY, // Cute Charm now innate; chosen Contrary (its real HA)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Calm Mind setup
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
        .heldItem = ITEM_CHOICE_BAND, // Overcoat Therian physical breaker
        .moves =
        {
            MOVE_PLAY_ROUGH,
            MOVE_EARTHQUAKE,
            MOVE_SPRINGTIDE_STORM,
            MOVE_U_TURN
        },
        .ability = ABILITY_SHEER_FORCE, // Overcoat now innate; chosen Sheer Force (override) powers Play Rough/Springtide
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
        .heldItem = ITEM_LIFE_ORB, // Protean physical attacker
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
        .heldItem = ITEM_CHOICE_BAND, // hit-and-run wallbreaker
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
        .heldItem = ITEM_FOCUS_BAND, // fast lead / spike support
        .moves =
        {
            MOVE_SPIKES,
            MOVE_FLOWER_TRICK,
            MOVE_KNOCK_OFF,
            MOVE_TAUNT
        },
        .ability = ABILITY_PROTEAN, // Overgrow now innate (latched); chosen Protean
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Torch Song bulky setup pivot
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_SHADOW_BALL,
            MOVE_SLACK_OFF,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MUMMY, // Blaze+Unaware both innate; chosen Mummy via species_ability_overrides (curse spreads on contact)
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
        .heldItem = ITEM_LEFTOVERS, // Unaware special wall
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_HEX,
            MOVE_SLACK_OFF,
            MOVE_WILL_O_WISP
        },
        .ability = ABILITY_MUMMY, // Blaze+Unaware both innate; chosen Mummy via species_ability_overrides (curse spreads on contact)
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
        .heldItem = ITEM_THROAT_SPRAY, // Torch Song snowball sweeper
        .moves =
        {
            MOVE_TORCH_SONG,
            MOVE_SHADOW_BALL,
            MOVE_EARTH_POWER,
            MOVE_SLACK_OFF
        },
        .ability = ABILITY_MUMMY, // Blaze+Unaware both innate; chosen Mummy via species_ability_overrides (curse spreads on contact)
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
        .heldItem = ITEM_LIFE_ORB, // Aqua Step setup sweeper
        .moves =
        {
            MOVE_AQUA_STEP,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_SPINNER,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
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
        .heldItem = ITEM_CHOICE_BAND, // Moxie band breaker
        .moves =
        {
            MOVE_AQUA_STEP,
            MOVE_CLOSE_COMBAT,
            MOVE_TRIPLE_AXEL,
            MOVE_U_TURN
        },
        .ability = ABILITY_WATER_ABSORB, // all real abilities innate; chosen Water Absorb (non-redundant)
        .nature = NATURE(SPE_UP, SPA_DOWN),
        .ev = EVS(
            .atk = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_FIGHTING,
    },

    // 0918
    {
        .species = SPECIES_SPIDOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // hazard setter
        .moves =
        {
            MOVE_STICKY_WEB,
            MOVE_SPIKES,
            MOVE_KNOCK_OFF,
            MOVE_CIRCLE_THROW
        },
        .ability = ABILITY_STICKY_HOLD, // all real abilities innate; chosen Sticky Hold (non-redundant)
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
        .heldItem = ITEM_FOCUS_BAND, // priority bug
        .moves =
        {
            MOVE_FIRST_IMPRESSION,
            MOVE_SUCKER_PUNCH,
            MOVE_LEECH_LIFE,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_TOUGH_CLAWS, // Swarm & Tinted Lens now innate; chosen Tough Claws (empty-slot override) boosts its all-contact kit
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
        .heldItem = ITEM_SILK_SCARF, // SD wallbreaker
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_FIRST_IMPRESSION,
            MOVE_SUCKER_PUNCH,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_TOUGH_CLAWS, // Swarm & Tinted Lens now innate; chosen Tough Claws (empty-slot override)
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
        .heldItem = ITEM_SITRUS_BERRY, // pivot/punch
        .moves =
        {
            MOVE_DOUBLE_SHOCK,
            MOVE_CLOSE_COMBAT,
            MOVE_MACH_PUNCH,
            MOVE_NUZZLE
        },
        .ability = ABILITY_VOLT_ABSORB, // Natural Cure & Iron Fist now innate; chosen Volt Absorb
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
        .heldItem = ITEM_EXPERT_BELT, // coverage pivot
        .moves =
        {
            MOVE_VOLT_SWITCH,
            MOVE_CLOSE_COMBAT,
            MOVE_NUZZLE,
            MOVE_MACH_PUNCH
        },
        .ability = ABILITY_VOLT_ABSORB, // Natural Cure & Iron Fist now innate; chosen Volt Absorb
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
        .heldItem = ITEM_LOADED_DICE, // Technician sweeper; Loaded Dice guarantees Population Bomb's full 10 hits (and Bullet Seed's 5)
        .moves =
        {
            MOVE_POPULATION_BOMB,
            MOVE_BULLET_SEED,
            MOVE_TIDY_UP,
            MOVE_ENCORE
        },
        .ability = ABILITY_NO_GUARD, // Friend Guard (+ Technician) now innate; chosen No Guard (override) lands Population Bomb / Beat Up reliably
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
        .heldItem = ITEM_FOCUS_BAND, // Friend Guard support lead
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
        .heldItem = ITEM_LEFTOVERS, // baked bread wall
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
        .heldItem = ITEM_LEFTOVERS, // bulky grass
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
        .heldItem = ITEM_FLAME_ORB, // guts attacker
        .moves =
        {
            MOVE_FACADE,
            MOVE_BRAVE_BIRD,
            MOVE_U_TURN,
            MOVE_DOUBLE_EDGE
        },
        .ability = ABILITY_HUSTLE, // Intimidate + Guts now innate; chosen Hustle (real slot 1) powers its Facade attacker
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
        .heldItem = ITEM_LEFTOVERS, // Purifying Salt physical wall
        .moves =
        {
            MOVE_SALT_CURE,
            MOVE_RECOVER,
            MOVE_STEALTH_ROCK,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_SOLID_ROCK, // Purifying Salt / Sturdy / Clear Body all now innate; chosen Solid Rock (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_LEFTOVERS, // Iron Defense + Body Press sweeper
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_SALT_CURE,
            MOVE_RECOVER
        },
        .ability = ABILITY_SOLID_ROCK, // Purifying Salt / Sturdy / Clear Body all now innate; chosen Solid Rock (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_ROCKY_HELMET, // doubles Salt Cure chip + Wide Guard
        .moves =
        {
            MOVE_SALT_CURE,
            MOVE_WIDE_GUARD,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SOLID_ROCK, // Purifying Salt / Sturdy / Clear Body all now innate; chosen Solid Rock (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_WEAKNESS_POLICY, // Armor Cannon special sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Expanding Force Trick Room attacker
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
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0937
    {
        .species = SPECIES_CERULEDGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Bitter Blade Swords Dance sweeper
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Bulk Up bulky setup
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
        .heldItem = ITEM_LEFTOVERS, // bulky frog
        .moves =
        {
            MOVE_VOLT_SWITCH,
            MOVE_MUDDY_WATER,
            MOVE_SLACK_OFF,
            MOVE_TOXIC
        },
        .ability = ABILITY_STATIC, // Electromorphosis now innate; chosen Static paralyzes contact attackers (its doubles set already runs Static)
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
        .heldItem = ITEM_ASSAULT_VEST, // special wall
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // fast special
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
        .heldItem = ITEM_SITRUS_BERRY, // guard dog mauler
        .moves =
        {
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_PSYCHIC_FANGS,
            MOVE_FIRE_FANG
        },
        .ability = ABILITY_STRONG_JAW, // Guard Dog + Stakeout now innate; chosen Strong Jaw powers its bites (override)
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
        .heldItem = ITEM_ROCKY_HELMET, // intimidate punish
        .moves =
        {
            MOVE_COMEUPPANCE,
            MOVE_CRUNCH,
            MOVE_PLAY_ROUGH,
            MOVE_WILD_CHARGE
        },
        .ability = ABILITY_STRONG_JAW, // Intimidate + Guard Dog + Stakeout now innate; chosen Strong Jaw powers its bites (override)
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
        .heldItem = ITEM_ROCKY_HELMET, // prankster status
        .moves =
        {
            MOVE_TOXIC,
            MOVE_ENCORE,
            MOVE_GUNK_SHOT,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_POISON_TOUCH, // Prankster now innate; chosen Poison Touch poisons on contact (Gunk Shot/Knock Off)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Wind Rider spin / hazard control
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
        .heldItem = ITEM_CHOICE_BAND, // physical breaker
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Mycelium Might hazard control
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
        .heldItem = ITEM_LIFE_ORB, // fast special attacker
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
        .heldItem = ITEM_ROCKY_HELMET, // rock setter
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
        .heldItem = ITEM_WEAKNESS_POLICY, // anger shell sweep
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
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll mixed sun attacker
        .moves =
        {
            MOVE_GROWTH,
            MOVE_FLAMETHROWER,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SHEER_FORCE, // Chlorophyll & Insomnia now innate; chosen Sheer Force (fork override)
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
        .heldItem = ITEM_MENTAL_HERB, // trick room setter
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_PSYCHIC,
            MOVE_BUG_BUZZ,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_SYNCHRONIZE, // Telepathy now innate; chosen Synchronize (:x:, never an innate -> stable) shares status back
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
    },

    // 0956
    {
        .species = SPECIES_ESPATHRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Opportunist Calm Mind sweeper
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_STORED_POWER,
            MOVE_DAZZLING_GLEAM,
            MOVE_ROOST
        },
        .ability = ABILITY_COMPETITIVE, // Opportunist / Frisk / Speed Boost all now innate; chosen Competitive (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_CHOICE_SPECS, // fast special breaker (Speed Boost now innate)
        .moves =
        {
            MOVE_PSYSHOCK,
            MOVE_DAZZLING_GLEAM,
            MOVE_SHADOW_BALL,
            MOVE_TERA_BLAST
        },
        .ability = ABILITY_COMPETITIVE, // Opportunist / Frisk / Speed Boost all now innate; chosen Competitive (fork override) observable + frees the redundant slot
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
        .heldItem = ITEM_AIR_BALLOON, // Gigaton Hammer + hazards utility
        .moves =
        {
            MOVE_GIGATON_HAMMER,
            MOVE_PLAY_ROUGH,
            MOVE_STEALTH_ROCK,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_CHOICE_BAND, // Gigaton Hammer band breaker
        .moves =
        {
            MOVE_GIGATON_HAMMER,
            MOVE_PLAY_ROUGH,
            MOVE_KNOCK_OFF,
            MOVE_ICE_HAMMER
        },
        .ability = ABILITY_SHEER_FORCE, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_MYSTIC_WATER, // priority eel
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_AQUA_JET,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_WATER_ABSORB, // Gooey & Sand Veil now innate; chosen Water Absorb (slot-0 override, :x: stable) heals on Water hits
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Big Pecks hazard / utility pivot
        .moves =
        {
            MOVE_STEALTH_ROCK,
            MOVE_KNOCK_OFF,
            MOVE_BRAVE_BIRD,
            MOVE_ROOST
        },
        .ability = ABILITY_RECKLESS, // Big Pecks now innate (Keen Eye/Rocky Payload too); chosen Reckless via override powers Brave Bird
        .nature = NATURE(ATK_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .spd = 4,
            .spe = 252
        ),
        .teraType = TYPE_STEEL,
    },

    // 0964
    // Enter in Zero form; Zero to Hero transforms it to Hero after its first switch-out
    // (the form table has no FORM_CHANGE_BEGIN_BATTLE, so naming the Hero form here would
    //  wrongly start it transformed).
    {
        .species = SPECIES_PALAFIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Zero to Hero band breaker
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
        .heldItem = ITEM_LIFE_ORB, // Bulk Up Hero sweeper
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
        .heldItem = ITEM_LIFE_ORB, // Filter Shift Gear sweeper
        .moves =
        {
            MOVE_SHIFT_GEAR,
            MOVE_GUNK_SHOT,
            MOVE_IRON_HEAD,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SHEER_FORCE, // Overcoat now innate; chosen Sheer Force (override) powers Gunk Shot/Iron Head
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
        .heldItem = ITEM_ROCKY_HELMET, // bulky pivot / hazard support
        .moves =
        {
            MOVE_GUNK_SHOT,
            MOVE_SPIKES,
            MOVE_PARTING_SHOT,
            MOVE_HIGH_HORSEPOWER
        },
        .ability = ABILITY_SHEER_FORCE, // Overcoat now innate; chosen Sheer Force (override) powers Gunk Shot/Iron Head
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Regenerator Shed Tail pivot
        .moves =
        {
            MOVE_SHED_TAIL,
            MOVE_DRAGON_PULSE,
            MOVE_OVERHEAT,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_MOTOR_DRIVE, // Shed Skin + Regenerator now innate; chosen Motor Drive (override, empty slot 1)
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
        .heldItem = ITEM_CHOICE_SCARF, // fast U-turn pivot
        .moves =
        {
            MOVE_DRAGON_CLAW,
            MOVE_KNOCK_OFF,
            MOVE_U_TURN,
            MOVE_RAPID_SPIN
        },
        .ability = ABILITY_MOTOR_DRIVE, // Shed Skin + Regenerator now innate; chosen Motor Drive (override, empty slot 1)
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
        .heldItem = ITEM_LEFTOVERS, // Earth Eater Iron Defense + Body Press wall
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
        .heldItem = ITEM_ROCKY_HELMET, // Shed Tail pivot
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
        .heldItem = ITEM_FOCUS_BAND, // Toxic Debris hazard lead
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
        .heldItem = ITEM_LIFE_ORB, // special attacker
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sand Rush Last Respects sweeper
        .moves =
        {
            MOVE_LAST_RESPECTS,
            MOVE_BODY_PRESS,
            MOVE_PLAY_ROUGH,
            MOVE_SHADOW_SNEAK
        },
        .ability = ABILITY_FLUFFY, // Sand Rush now innate; chosen Fluffy
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // scrappy bird
        .moves =
        {
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_THROAT_CHOP,
            MOVE_U_TURN
        },
        .ability = ABILITY_COSTAR, // Scrappy & Tangled Feet now innate; chosen Costar (its real HA) copies the ally's stat changes in doubles
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
        .heldItem = ITEM_SITRUS_BERRY, // SD sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_BRAVE_BIRD,
            MOVE_CLOSE_COMBAT,
            MOVE_THROAT_CHOP
        },
        .ability = ABILITY_COSTAR, // Scrappy & Tangled Feet now innate; chosen Costar
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
        .heldItem = ITEM_NEVER_MELT_ICE, // belly drum whale
        .moves =
        {
            MOVE_BELLY_DRUM,
            MOVE_ICICLE_CRASH,
            MOVE_LIQUIDATION,
            MOVE_ICE_SHARD
        },
        // Thick Fat (and Slush Rush) now innate; chosen Sheer Force (HA) powers up this Ice attacker.
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
        .heldItem = ITEM_ASSAULT_VEST, // sheer force tank
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
        .heldItem = ITEM_MYSTIC_WATER, // sharpness slicer
        .moves =
        {
            MOVE_AQUA_CUTTER,
            MOVE_PSYCHO_CUT,
            MOVE_NIGHT_SLASH,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB, // Mold Breaker + Sharpness now innate; fork override fills empty slot 1 with a chosen Water Absorb
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
        .heldItem = ITEM_SITRUS_BERRY, // fillet away sweep
        .moves =
        {
            MOVE_FILLET_AWAY,
            MOVE_AQUA_CUTTER,
            MOVE_PSYCHO_CUT,
            MOVE_AQUA_JET
        },
        .ability = ABILITY_WATER_ABSORB, // Mold Breaker + Sharpness now innate; fork override fills empty slot 1 with a chosen Water Absorb
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
        .heldItem = ITEM_LEFTOVERS, // Unaware Curse physical wall
        .moves =
        {
            MOVE_CURSE,
            MOVE_WAVE_CRASH,
            MOVE_REST,
            MOVE_SLEEP_TALK
        },
        .ability = ABILITY_WATER_ABSORB, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Order Up / bulky pivot
        .moves =
        {
            MOVE_WAVE_CRASH,
            MOVE_BODY_PRESS,
            MOVE_EARTHQUAKE,
            MOVE_REST
        },
        .ability = ABILITY_WATER_ABSORB, // chosen via fork override (species_ability_overrides.c)
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
        .heldItem = ITEM_THROAT_SPRAY, // SpA boost off spread move
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
        .heldItem = ITEM_LEFTOVERS, // Bulk Up + Rage Fist snowball
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_RAGE_FIST,
            MOVE_DRAIN_PUNCH,
            MOVE_TAUNT
        },
        .ability = ABILITY_VITAL_SPIRIT, // all real abilities innate; chosen Vital Spirit (non-redundant)
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
        .heldItem = ITEM_CHOICE_SCARF, // Defiant revenge killer
        .moves =
        {
            MOVE_RAGE_FIST,
            MOVE_CLOSE_COMBAT,
            MOVE_ICE_PUNCH,
            MOVE_U_TURN
        },
        .ability = ABILITY_VITAL_SPIRIT, // all real abilities innate; chosen Vital Spirit (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Unaware special wall / status spreader
        .moves =
        {
            MOVE_TOXIC,
            MOVE_RECOVER,
            MOVE_EARTHQUAKE,
            MOVE_TOXIC_SPIKES
        },
        .ability = ABILITY_WATER_ABSORB, // Unaware now innate; chosen Water Absorb adds a Water immunity to the special wall
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
        .heldItem = ITEM_LEFTOVERS, // Water Absorb stall pivot
        .moves =
        {
            MOVE_RECOVER,
            MOVE_EARTHQUAKE,
            MOVE_TOXIC,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE(DEF_UP, SPA_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 252,
            .spa = 4
        ),
        .teraType = TYPE_FAIRY,
    },

    // 0981
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Armor Tail Calm Mind wall
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_PSYCHIC_NOISE,
            MOVE_HYPER_VOICE,
            MOVE_REST
        },
        .ability = ABILITY_SAP_SIPPER, // Armor Tail now innate; chosen Sap Sipper (real HA) adds a Grass immunity + Attack boost
        .nature = NATURE(SPD_UP, ATK_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spd = 252
        ),
        .teraType = TYPE_DARK,
    },
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room support tank
        .moves =
        {
            MOVE_TRICK_ROOM,
            MOVE_HYPER_VOICE,
            MOVE_PSYCHIC,
            MOVE_DAZZLING_GLEAM
        },
        .ability = ABILITY_SAP_SIPPER, // Armor Tail now innate; chosen Sap Sipper (real HA) adds a Grass immunity + Attack boost
        .nature = NATURE(SPD_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .spa = 4,
            .spd = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
    },

    // 0982
    {
        .species = SPECIES_DUDUNSPARCE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Serene Grace Coil + flinch / status
        .moves =
        {
            MOVE_COIL,
            MOVE_BODY_SLAM,
            MOVE_ROOST,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SIMPLE, // all real abilities innate; chosen Simple (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind + Boomburst special
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_BOOMBURST,
            MOVE_EARTH_POWER,
            MOVE_ROOST
        },
        .ability = ABILITY_SIMPLE, // all real abilities innate; chosen Simple (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Supreme Overlord Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_BLACK_GLASSES, // Defiant pivot punisher
        .moves =
        {
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_ASSAULT_VEST, // bulky special tank
        .moves =
        {
            MOVE_KOWTOW_CLEAVE,
            MOVE_IRON_HEAD,
            MOVE_SUCKER_PUNCH,
            MOVE_LOW_KICK
        },
        .ability = ABILITY_SHEER_FORCE, // all real abilities innate; chosen Sheer Force (non-redundant)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis hazard control sweeper
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Bulk Up physical wall / spinner
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
        .heldItem = ITEM_LEFTOVERS, // Protosynthesis utility wall
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
        .heldItem = ITEM_BOOSTER_ENERGY, // fast support pivot
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis bulky breaker
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special sweeper (innate Levitate)
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
        .heldItem = ITEM_CHOICE_SPECS, // special nuke (innate Levitate)
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
        .heldItem = ITEM_FOCUS_BAND, // fast Perish Trap / utility (innate Levitate)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis bulky attacker
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special attacker (innate Levitate)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // hazard lead (innate Levitate)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive hazard control sweeper
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
        .heldItem = ITEM_LEFTOVERS, // bulky hazard lead
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive fast special attacker
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
        .heldItem = ITEM_CHOICE_SPECS, // hydro specs nuke
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
        .heldItem = ITEM_ASSAULT_VEST, // Quark Drive bulky attacker
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Belly Drum + Drain Punch sweeper
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive fast special attacker
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive special sweeper (innate Levitate)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Toxic Spikes / special pivot (innate Levitate)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Dragon Dance sweeper
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
        .heldItem = ITEM_LOADED_DICE, // Dragon Dance + Icicle Spear sweeper
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_SPEAR,
            MOVE_GLAIVE_RUSH,
            MOVE_EARTHQUAKE
        },
        .ability = ABILITY_SNOW_WARNING, // Thermal Exchange (+ Ice Body) now innate; chosen Snow Warning (fork override, empty slot 1) sets snow -> innate Ice Body heals
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
        .heldItem = ITEM_LIFE_ORB, // mixed Dragon Dance breaker
        .moves =
        {
            MOVE_DRAGON_DANCE,
            MOVE_ICICLE_CRASH,
            MOVE_GLAIVE_RUSH,
            MOVE_ICE_SHARD
        },
        .ability = ABILITY_SNOW_WARNING, // Thermal Exchange (+ Ice Body) now innate; chosen Snow Warning (fork override, empty slot 1) sets snow -> innate Ice Body heals
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
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot + Make It Rain sweeper (innate Levitate)
        .moves =
        {
            MOVE_NASTY_PLOT,
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_RECOVER
        },
        .ability = ABILITY_STICKY_HOLD, // Good as Gold now innate (still blocks status moves); chosen Sticky Hold (override) keeps its coin hoard
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Good as Gold status blocker / pivot (innate Levitate)
        .moves =
        {
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_RECOVER,
            MOVE_THUNDER_WAVE
        },
        .ability = ABILITY_STICKY_HOLD, // Good as Gold now innate (still blocks status moves); chosen Sticky Hold (override) keeps its coin hoard
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
        .heldItem = ITEM_CHOICE_SPECS, // special nuke (innate Levitate)
        .moves =
        {
            MOVE_MAKE_IT_RAIN,
            MOVE_SHADOW_BALL,
            MOVE_FOCUS_BLAST,
            MOVE_TRICK
        },
        .ability = ABILITY_STICKY_HOLD, // Good as Gold now innate (still blocks status moves); chosen Sticky Hold (override) keeps its coin hoard
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
        .heldItem = ITEM_LEFTOVERS, // Tablets of Ruin defensive status spreader
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
        .heldItem = ITEM_LEFTOVERS, // Tablets stall wall
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sword of Ruin Swords Dance sweeper
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
        .heldItem = ITEM_CHOICE_BAND, // Sword of Ruin band breaker
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
        .heldItem = ITEM_LEFTOVERS, // Vessel of Ruin physical wall / hazards
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Ruination staller
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Beads of Ruin special sweeper
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
        .heldItem = ITEM_CHOICE_SPECS, // Beads of Ruin nuke
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Dragon Dance sweeper
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
        .heldItem = ITEM_CHOICE_BAND, // Dragon's Maw band breaker
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive mixed Swords Dance sweeper
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
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
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
        .heldItem = ITEM_LIFE_ORB, // Orichalcum Pulse sun sweeper
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
        .heldItem = ITEM_CHOICE_BAND, // band breaker
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
        .heldItem = ITEM_CHOICE_SPECS, // Hadron Engine special nuke (innate Levitate)
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
        .heldItem = ITEM_LIFE_ORB, // Calm Mind sweeper (innate Levitate)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special attacker
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
        .heldItem = ITEM_CHOICE_SPECS, // Draco specs nuke
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Swords Dance sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Calm Mind Matcha Gotcha wall (innate Levitate)
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MATCHA_GOTCHA,
            MOVE_SHADOW_BALL,
            MOVE_STRENGTH_SAP
        },
        .ability = ABILITY_FLASH_FIRE, // Heatproof now innate; chosen Flash Fire (override) turns the hot tea's Fire weakness into an immunity
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
        .heldItem = ITEM_ASSAULT_VEST, // Hospitality support tank (innate Hospitality / Heatproof / Levitate)
        .moves =
        {
            MOVE_MATCHA_GOTCHA,
            MOVE_SHADOW_BALL,
            MOVE_GIGA_DRAIN,
            MOVE_TRICK_ROOM
        },
        .ability = ABILITY_FLASH_FIRE, // Hospitality + Heatproof now innate; chosen Flash Fire (override) turns the hot tea's Fire weakness into an immunity
        .nature = NATURE(SPA_UP, SPE_DOWN),
        .ev = EVS(
            .hp = 252,
            .def = 4,
            .spa = 252
        ),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FAIRY,
    },

    // 1014
    {
        .species = SPECIES_OKIDOGI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Toxic Chain Bulk Up sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Guard Dog Bulk Up wall breaker
        .moves =
        {
            MOVE_BULK_UP,
            MOVE_DRAIN_PUNCH,
            MOVE_POISON_JAB,
            MOVE_PSYCHIC_FANGS
        },
        .ability = ABILITY_TOXIC_CHAIN, // Guard Dog now innate; chosen Toxic Chain is its signature
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
        .heldItem = ITEM_LIFE_ORB, // Toxic Chain special attacker
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
        .heldItem = ITEM_LEFTOVERS, // Toxic Chain defensive pivot
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
        .heldItem = ITEM_LEFTOVERS, // Toxic Chain utility pivot
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Calm Mind special pivot
        .moves =
        {
            MOVE_CALM_MIND,
            MOVE_MOONBLAST,
            MOVE_SLUDGE_BOMB,
            MOVE_ROOST
        },
        .ability = ABILITY_TOXIC_CHAIN, // Technician now innate; chosen Toxic Chain may badly poison on hit
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
        .heldItem = ITEM_LEFTOVERS, // Defiant Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF
        },
        .ability = ABILITY_DEFIANT,
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
        .heldItem = ITEM_WELLSPRING_MASK, // Water Absorb Swords Dance sweeper
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
        .heldItem = ITEM_HEARTHFLAME_MASK, // Mold Breaker Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_HORN_LEECH
        },
        .ability = ABILITY_FLASH_FIRE, // Mold Breaker now innate (sole ability); fork override fills empty slot 1 with a chosen Flash Fire (Fire immunity + Fire-power boost)
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
        .heldItem = ITEM_CORNERSTONE_MASK, // Sturdy Swords Dance sweeper
        .moves =
        {
            MOVE_SWORDS_DANCE,
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_STONE_EDGE
        },
        .ability = ABILITY_EARTH_EATER, // Sturdy now innate (sole ability); fork override gives a chosen Earth Eater (Ground immunity+heal covers its Rock weakness)
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // hazard-immune Defiant pivot
        .moves =
        {
            MOVE_IVY_CUDGEL,
            MOVE_POWER_WHIP,
            MOVE_KNOCK_OFF,
            MOVE_SWORDS_DANCE
        },
        .ability = ABILITY_DEFIANT,
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
        .heldItem = ITEM_ASSAULT_VEST, // Electro Shot special tank
        .moves =
        {
            MOVE_ELECTRO_SHOT,
            MOVE_DRACO_METEOR,
            MOVE_FLASH_CANNON,
            MOVE_BODY_PRESS
        },
        .ability = ABILITY_BULLETPROOF, // all real abilities innate; chosen Bulletproof (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Stamina Body Press wall
        .moves =
        {
            MOVE_IRON_DEFENSE,
            MOVE_BODY_PRESS,
            MOVE_DRAGON_TAIL,
            MOVE_STEALTH_ROCK
        },
        .ability = ABILITY_BULLETPROOF, // all real abilities innate; chosen Bulletproof (non-redundant)
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
        .heldItem = ITEM_LIFE_ORB, // rain Electro Shot nuke
        .moves =
        {
            MOVE_ELECTRO_SHOT,
            MOVE_FLASH_CANNON,
            MOVE_DRACO_METEOR,
            MOVE_PROTECT
        },
        .ability = ABILITY_BULLETPROOF, // all real abilities innate; chosen Bulletproof (non-redundant)
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
        .heldItem = ITEM_LEFTOVERS, // Regenerator bulky special tank
        .moves =
        {
            MOVE_FICKLE_BEAM,
            MOVE_GIGA_DRAIN,
            MOVE_NASTY_PLOT,
            MOVE_RECOVER
        },
        .ability = ABILITY_GRASSY_SURGE, // Regenerator + Sticky Hold now innate; chosen Grassy Surge powers Grass STAB and heals (override)
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves =
        {
            MOVE_FICKLE_BEAM,
            MOVE_DRACO_METEOR,
            MOVE_GIGA_DRAIN,
            MOVE_EARTH_POWER
        },
        .ability = ABILITY_GRASSY_SURGE, // Regenerator + Sticky Hold now innate; chosen Grassy Surge powers Grass STAB and heals (override)
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Dragon Dance sweeper
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
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Morning Sun setup
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Calm Mind special sweeper
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
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind wall
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Mighty Cleave sweeper
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
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Calm Mind special sweeper
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
        .heldItem = ITEM_LEFTOVERS, // Tera Shell Calm Mind tank
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
        .heldItem = ITEM_ASSAULT_VEST, // special tank
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
        .heldItem = ITEM_LEFTOVERS, // Poison Puppeteer Nasty Plot tank (innate Levitate)
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
        .heldItem = ITEM_LEFTOVERS, // Malignant Chain status tank (innate Levitate)
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
