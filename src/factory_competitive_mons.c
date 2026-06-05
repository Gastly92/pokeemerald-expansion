#include "global.h"
#include "factory_competitive_mons.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// FORK: fork-owned Battle Factory roster overhaul (B_FRONTIER_COMPETITIVE_MONS).
// A from-scratch roster of modern competitive sets that replaces the vanilla
// gBattleFrontierMons on the Battle Factory's code paths. Kept in this new file
// (not gBattleFrontierMons) so upstream syncs never touch it and the vanilla
// roster stays intact for the other facilities (Tower/Dome) until they adopt this
// list too. Same struct TrainerMon format as gBattleFrontierMons, so every field
// CreateFacilityMon understands is available: moves, heldItem, ability, nature,
// ev, iv, teraType, gender, isShiny, ball, dynamax/gigantamax.
//
// Field notes:
//  - .ev   uses TRAINER_PARTY_EVS(hp, atk, def, speed, spatk, spdef)  <- speed is 4th.
//  - .iv   is OPTIONAL. Leave it out and the mon keeps the Factory's fixed IVs
//          (31 in every stat under B_FRONTIER_MAX_IVS). Only set .iv =
//          TRAINER_PARTY_IVS(...) for an intentional non-max spread (e.g. 0 Atk).
//  - .ball is cosmetic; default BALL_POKE.
//  - .teraType TYPE_NORMAL (0) reads as "unset" in CreateFacilityMon, so a Tera
//          Normal set would not apply — fine, those are rare.
//  - .tags  is REQUIRED here: FACTORY_SINGLES / FACTORY_DOUBLES / FACTORY_BOTH
//          marks which battle format(s) the set is suited for, so a singles-only
//          set never shows up in a doubles challenge (and vice versa). A set that
//          works in either mode is FACTORY_BOTH. (See factory_competitive_mons.h.)
//
// Design intent — these sets are tuned for THIS fork, not the live competitive
// metagame. Account for the DETERMINISTIC_* changes (see FORK.md):
//  - Accuracy never misses (low-accuracy moves instead cost extra PP), so
//    Hydro Pump / Focus Blast / Fire Blast are "reliable" — pick power freely.
//  - Crits only land when guaranteed; crit items (Scope Lens, etc.) get a
//    guaranteed first-hit crit via DETERMINISTIC_HOLD_EFFECTS instead of a chance.
//  - Focus Sash behaves like a one-shot entry Focus Band (survives one lethal hit
//    from any HP on the entry turn, then is consumed).
//  - Sub-100% additional effects (burn/para/flinch) are gated on super effective
//    or STAB rather than rolled — value secondary effects accordingly.
//  - Sleep from sub-100% moves becomes drowsiness (Yawn-like); 100% Spore sleeps.
//
// Roster rules enforced at draft time (src/battle_factory.c): unique species,
// unique held item, and at most ONE Mega Stone + at most ONE Z-Crystal per team.
//
// STATUS: work in progress. The roster is being built generation by generation;
// B_FRONTIER_COMPETITIVE_MONS stays FALSE until it is large enough to draft a
// 6-mon team (plus distinct opponents) without the random selector stalling. The
// batch below is the first pass at Generation I and is expected to grow/change.

const struct TrainerMon gFactoryCompetitiveMons[] =
{
    // ---------------- Generation I ----------------
    {
        .species = SPECIES_CHARIZARD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHARIZARDITE_Y, // Mega Charizard Y (Drought)
        .moves = {MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_AIR_SLASH, MOVE_ROOST},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_VENUSAURITE, // Mega Venusaur (Thick Fat)
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_LEECH_SEED, MOVE_SYNTHESIS},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 220, 0, 0, 36),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_NASTY_PLOT},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FOCUS_SASH,
        .moves = {MOVE_SHADOW_BALL, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST, MOVE_DESTINY_BOND},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SNORLAX,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_BODY_SLAM, MOVE_CURSE, MOVE_EARTHQUAKE, MOVE_REST},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JOLTEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ELECTRIUM_Z, // Gigavolt Havoc
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_SHADOW_BALL, MOVE_HYPER_VOICE},
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAUROS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT, MOVE_IRON_HEAD},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STARMIE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS,
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGONITE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves = {MOVE_DRAGON_DANCE, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_ROOST},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GYARADOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GYARADOSITE, // Mega Gyarados (Mold Breaker)
        .moves = {MOVE_DRAGON_DANCE, MOVE_WATERFALL, MOVE_CRUNCH, MOVE_EARTHQUAKE},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AERODACTYL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_AERODACTYLITE, // Mega Aerodactyl (Tough Claws)
        .moves = {MOVE_ROCK_SLIDE, MOVE_DUAL_WINGBEAT, MOVE_EARTHQUAKE, MOVE_AQUA_TAIL},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLOWBRO,
        .tags = FACTORY_SINGLES, // slow defensive Calm Mind wall; poor doubles fit
        .heldItem = ITEM_SLOWBRONITE, // Mega Slowbro (Shell Armor)
        .moves = {MOVE_SCALD, MOVE_PSYSHOCK, MOVE_CALM_MIND, MOVE_SLACK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FACTORY_SINGLES, // Shell Smash sweeper (debatable — could be BOTH)
        .heldItem = ITEM_WHITE_HERB, // restores the Shell Smash defense drops
        .moves = {MOVE_SHELL_SMASH, MOVE_ICICLE_SPEAR, MOVE_ROCK_BLAST, MOVE_HYDRO_PUMP},
        .ability = ABILITY_SKILL_LINK, // guarantees max multi-hit under DETERMINISTIC_MOVE_RESULTS
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHANSEY,
        .tags = FACTORY_SINGLES, // stall wall
        .heldItem = ITEM_EVIOLITE,
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_TOXIC, MOVE_HEAL_BELL},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZAPDOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_ROOST},
        .ability = ABILITY_STATIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
};

const u16 gFactoryCompetitiveMonsCount = ARRAY_COUNT(gFactoryCompetitiveMons);
