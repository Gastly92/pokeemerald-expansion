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
//          TRAINER_PARTY_IVS(...) for an intentional non-max spread (e.g. 0 Speed
//          for a Trick Room attacker).
//  - .ball is cosmetic; default BALL_POKE.
//  - .teraType TYPE_NORMAL (0) reads as "unset" in CreateFacilityMon, so a Tera
//          Normal set would not apply — fine, Tera is disabled in this fork for now
//          (B_FLAG_TERA_ORB_* = 0), so teraType is recorded as future-proofing only.
//  - .tags  is REQUIRED here: FACTORY_SINGLES / FACTORY_DOUBLES / FACTORY_BOTH
//          marks which battle format(s) the set is suited for, so a singles-only
//          set never shows up in a doubles challenge (and vice versa). A set that
//          works in either mode is FACTORY_BOTH. (See factory_competitive_mons.h.)
//
// Design intent — these sets are tuned for THIS fork, not the live competitive
// metagame. Account for the DETERMINISTIC_* changes (see FORK.md):
//  - Accuracy never misses (low-accuracy moves instead cost extra PP), so
//    Hydro Pump / Focus Blast / Fire Blast / Hurricane are "reliable" — pick power.
//  - Crits only land when guaranteed; crit items (Scope Lens / Razor Claw / Lucky
//    Punch / Leek) give a guaranteed first-hit crit via DETERMINISTIC_HOLD_EFFECTS.
//  - Focus Sash behaves like a one-shot entry Focus Band (survives one lethal hit
//    from any HP on the entry turn, then is consumed).
//  - Skill Link / Loaded Dice guarantee max multi-hit; 2-5 hit moves always hit 3.
//  - King's Rock / Razor Fang flinch on the holder's first attack; Quick Claw
//    guarantees a first-turn first-strike once.
//  - Paralysis is a PP/priority tax (no full-para, full Speed kept), so Thunder
//    Wave still has value as a priority-bracket demotion.
//  - Sub-100% additional effects (burn/para/flinch) are gated on super effective
//    or STAB; sub-100% sleep moves only drowse (Yawn-like); 100% Spore sleeps.
//
// Roster rules enforced at draft time (src/battle_factory.c): unique species,
// unique held item, and at most ONE Mega Stone + at most ONE Z-Crystal per team.
// The roster intentionally carries several distinct builds per species so the
// opponent's exact set can't be read off the species alone.
//
// STATUS: work in progress. Generation I is built out (~4-5 builds per reasonable
// species); later generations follow. B_FRONTIER_COMPETITIVE_MONS may be flipped
// to TRUE for a Gen I-only roster — the list is already large enough to draft a
// 6-mon team (plus distinct opponents) in both singles and doubles.

const struct TrainerMon gFactoryCompetitiveMons[] =
{
    // ============================================================
    //                       Generation I
    // ============================================================

    // ---- Venusaur ----
    {
        .species = SPECIES_VENUSAUR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_VENUSAURITE, // Mega Venusaur (Thick Fat) — bulky pivot
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_LEECH_SEED, MOVE_SYNTHESIS},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 220, 0, 0, 36),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper (no mega)
        .moves = {MOVE_GROWTH, MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // defensive status spreader
        .moves = {MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VENUSAUR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GRASSIUM_Z, // Bloom Doom nuke
        .moves = {MOVE_GROWTH, MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_WEATHER_BALL},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Charizard ----
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
        .species = SPECIES_CHARIZARD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHARIZARDITE_X, // Mega Charizard X (Tough Claws, Fire/Dragon)
        .moves = {MOVE_DRAGON_DANCE, MOVE_FLARE_BLITZ, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHARIZARD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // no-mega special breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_AIR_SLASH, MOVE_FOCUS_BLAST, MOVE_OVERHEAT},
        .ability = ABILITY_SOLAR_POWER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHARIZARD,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sub-Roost stallbreaker
        .moves = {MOVE_SUBSTITUTE, MOVE_ROOST, MOVE_FIRE_BLAST, MOVE_DRAGON_PULSE},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Blastoise ----
    {
        .species = SPECIES_BLASTOISE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLASTOISINITE, // Mega Blastoise (Mega Launcher)
        .moves = {MOVE_HYDRO_PUMP, MOVE_AURA_SPHERE, MOVE_DARK_PULSE, MOVE_ICE_BEAM},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLASTOISE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky spinner
        .moves = {MOVE_SCALD, MOVE_RAPID_SPIN, MOVE_ICE_BEAM, MOVE_REST},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLASTOISE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FLIP_TURN, MOVE_EARTHQUAKE},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Butterfree ----
    {
        .species = SPECIES_BUTTERFREE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Quiver Dance sweeper, sash = one-shot entry guard
        .moves = {MOVE_QUIVER_DANCE, MOVE_BUG_BUZZ, MOVE_AIR_SLASH, MOVE_SLEEP_POWDER},
        .ability = ABILITY_COMPOUND_EYES,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BUTTERFREE,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // doubles spread + support
        .moves = {MOVE_BUG_BUZZ, MOVE_AIR_SLASH, MOVE_SLEEP_POWDER, MOVE_TAILWIND},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Beedrill ----
    {
        .species = SPECIES_BEEDRILL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BEEDRILLITE, // Mega Beedrill (Adaptability)
        .moves = {MOVE_X_SCISSOR, MOVE_POISON_JAB, MOVE_DRILL_RUN, MOVE_KNOCK_OFF},
        .ability = ABILITY_SWARM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BEEDRILL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast pivot lead
        .moves = {MOVE_POISON_JAB, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_SWORDS_DANCE},
        .ability = ABILITY_SWARM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Pidgeot ----
    {
        .species = SPECIES_PIDGEOT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_PIDGEOTITE, // Mega Pidgeot (No Guard)
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_U_TURN, MOVE_ROOST},
        .ability = ABILITY_KEEN_EYE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PIDGEOT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // no-mega hurricane spam
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_U_TURN, MOVE_HYPER_VOICE},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Pikachu ----
    {
        .species = SPECIES_PIKACHU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIGHT_BALL, // doubles the attack stats
        .moves = {MOVE_VOLT_TACKLE, MOVE_PLAY_ROUGH, MOVE_KNOCK_OFF, MOVE_FAKE_OUT},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PIKACHU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_PIKANIUM_Z, // Catastropika
        .moves = {MOVE_VOLT_TACKLE, MOVE_PLAY_ROUGH, MOVE_IRON_TAIL, MOVE_NASTY_PLOT},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Raichu ----
    {
        .species = SPECIES_RAICHU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_SURF, MOVE_NASTY_PLOT},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAICHU,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // fast Fake Out support
        .moves = {MOVE_THUNDERBOLT, MOVE_FAKE_OUT, MOVE_VOLT_SWITCH, MOVE_GRASS_KNOT},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Sandslash ----
    {
        .species = SPECIES_SANDSLASH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_EARTHQUAKE, MOVE_KNOCK_OFF, MOVE_RAPID_SPIN, MOVE_SWORDS_DANCE},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SANDSLASH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_GROUNDIUM_Z, // SD + Tectonic Rage
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_KNOCK_OFF},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Nidoqueen ----
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sheer Force nuke
        .moves = {MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE, MOVE_ICE_BEAM, MOVE_FLAMETHROWER},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NIDOQUEEN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_TOXIC_SPIKES, MOVE_EARTH_POWER, MOVE_ICE_BEAM},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 200, 0, 0, 56),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Nidoking ----
    {
        .species = SPECIES_NIDOKING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sheer Force nuke
        .moves = {MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NIDOKING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE, MOVE_ICE_BEAM, MOVE_FLAMETHROWER},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Clefable ----
    {
        .species = SPECIES_CLEFABLE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Magic Guard CM wall
        .moves = {MOVE_MOONBLAST, MOVE_CALM_MIND, MOVE_SOFT_BOILED, MOVE_FLAMETHROWER},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLEFABLE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Unaware / offensive
        .moves = {MOVE_MOONBLAST, MOVE_NASTY_PLOT, MOVE_FLAMETHROWER, MOVE_THUNDERBOLT},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLEFABLE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_STICKY_BARB, // Unaware staller (toggle-friendly)
        .moves = {MOVE_MOONBLAST, MOVE_SOFT_BOILED, MOVE_TOXIC, MOVE_THUNDER_WAVE},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Ninetales ----
    {
        .species = SPECIES_NINETALES,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAT_ROCK, // Drought setter
        .moves = {MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_NASTY_PLOT, MOVE_WILL_O_WISP},
        .ability = ABILITY_DROUGHT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NINETALES,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FIRIUM_Z, // sun-boosted Inferno Overdrive
        .moves = {MOVE_NASTY_PLOT, MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_SCORCHING_SANDS},
        .ability = ABILITY_DROUGHT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Venomoth ----
    {
        .species = SPECIES_VENOMOTH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Quiver Dance + Sleep Powder sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_BUG_BUZZ, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VENOMOTH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLACK_SLUDGE,
        .moves = {MOVE_BUG_BUZZ, MOVE_SLUDGE_BOMB, MOVE_QUIVER_DANCE, MOVE_ROOST},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Dugtrio ----
    {
        .species = SPECIES_DUGTRIO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Arena Trap revenge / trapper
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_SUCKER_PUNCH, MOVE_SWORDS_DANCE},
        .ability = ABILITY_ARENA_TRAP,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DUGTRIO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sand Force band
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_SUCKER_PUNCH, MOVE_AERIAL_ACE},
        .ability = ABILITY_SAND_FORCE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Golduck ----
    {
        .species = SPECIES_GOLDUCK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_CALM_MIND, MOVE_FOCUS_BLAST},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOLDUCK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_SCALD, MOVE_CALM_MIND, MOVE_RECOVER, MOVE_ICE_BEAM},
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Arcanine ----
    {
        .species = SPECIES_ARCANINE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky pivot
        .moves = {MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_MORNING_SUN, MOVE_WILL_O_WISP},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCANINE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band wallbreaker
        .moves = {MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_WILD_CHARGE, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCANINE,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // doubles Intimidate support
        .moves = {MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Poliwrath ----
    {
        .species = SPECIES_POLIWRATH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Bulk Up
        .moves = {MOVE_BULK_UP, MOVE_LIQUIDATION, MOVE_DRAIN_PUNCH, MOVE_REST},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_POLIWRATH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim attacker
        .moves = {MOVE_LIQUIDATION, MOVE_CLOSE_COMBAT, MOVE_ICE_PUNCH, MOVE_DARKEST_LARIAT},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Alakazam ----
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Magic Guard nuke
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_NASTY_PLOT},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ALAKAZITE, // Mega Alakazam (Trace)
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_ENERGY_BALL},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ALAKAZAM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast lead, sash = one-shot guard
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_ENCORE},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Machamp ----
    {
        .species = SPECIES_MACHAMP,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // No Guard band breaker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_ICE_PUNCH, MOVE_STONE_EDGE},
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLAME_ORB, // Guts staller-breaker
        .moves = {MOVE_FACADE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_BULLET_PUNCH},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MACHAMP,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // doubles bulk
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ROCK_SLIDE, MOVE_KNOCK_OFF, MOVE_ICE_PUNCH},
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Victreebel ----
    {
        .species = SPECIES_VICTREEBEL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves = {MOVE_SOLAR_BLADE, MOVE_KNOCK_OFF, MOVE_SUCKER_PUNCH, MOVE_SWORDS_DANCE},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VICTREEBEL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH,
        .moves = {MOVE_SLEEP_POWDER, MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_WEATHER_BALL},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Tentacruel ----
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky spinner / hazards
        .moves = {MOVE_SCALD, MOVE_RAPID_SPIN, MOVE_TOXIC_SPIKES, MOVE_HAZE},
        .ability = ABILITY_LIQUID_OOZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 132, 0, 124),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TENTACRUEL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Rain Dish / offensive
        .moves = {MOVE_HYDRO_PUMP, MOVE_SLUDGE_WAVE, MOVE_ICE_BEAM, MOVE_FLIP_TURN},
        .ability = ABILITY_RAIN_DISH,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Golem ----
    {
        .species = SPECIES_GOLEM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sturdy band breaker
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_EXPLOSION, MOVE_SUPERPOWER},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOLEM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CUSTAP_BERRY, // Sturdy + Custap lead w/ rocks
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_EXPLOSION},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Rapidash ----
    {
        .species = SPECIES_RAPIDASH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_FLARE_BLITZ, MOVE_HIGH_HORSEPOWER, MOVE_WILD_CHARGE, MOVE_MORNING_SUN},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAPIDASH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND,
        .moves = {MOVE_FLARE_BLITZ, MOVE_HIGH_HORSEPOWER, MOVE_WILD_CHARGE, MOVE_MEGAHORN},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Slowbro ----
    {
        .species = SPECIES_SLOWBRO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SLOWBRONITE, // Mega Slowbro (Shell Armor) CM wall
        .moves = {MOVE_SCALD, MOVE_PSYSHOCK, MOVE_CALM_MIND, MOVE_SLACK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLOWBRO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator pivot wall
        .moves = {MOVE_SCALD, MOVE_FUTURE_SIGHT, MOVE_SLACK_OFF, MOVE_THUNDER_WAVE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLOWBRO,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker (0 Spe IV)
        .moves = {MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_SCALD, MOVE_SLACK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Magneton ----
    {
        .species = SPECIES_MAGNETON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Magnet Pull trapper
        .moves = {MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH, MOVE_TRI_ATTACK},
        .ability = ABILITY_MAGNET_PULL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAGNETON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_AIR_BALLOON, // Analytic, dodges Ground
        .moves = {MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_DAZZLING_GLEAM, MOVE_THUNDER_WAVE},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Dodrio ----
    {
        .species = SPECIES_DODRIO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // fast band
        .moves = {MOVE_BRAVE_BIRD, MOVE_DOUBLE_EDGE, MOVE_KNOCK_OFF, MOVE_QUICK_ATTACK},
        .ability = ABILITY_EARLY_BIRD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DODRIO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLYINIUM_Z, // SD + Supersonic Skystrike
        .moves = {MOVE_SWORDS_DANCE, MOVE_BRAVE_BIRD, MOVE_DOUBLE_EDGE, MOVE_KNOCK_OFF},
        .ability = ABILITY_EARLY_BIRD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Muk ----
    {
        .species = SPECIES_MUK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky special tank
        .moves = {MOVE_GUNK_SHOT, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH, MOVE_TOXIC},
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 84, 0, 0, 0, 172),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MUK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves = {MOVE_GUNK_SHOT, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Cloyster ----
    {
        .species = SPECIES_CLOYSTER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper, Skill Link max multi-hit
        .moves = {MOVE_SHELL_SMASH, MOVE_ICICLE_SPEAR, MOVE_ROCK_BLAST, MOVE_HYDRO_PUMP},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // guaranteed Shell Smash via one-shot entry guard
        .moves = {MOVE_SHELL_SMASH, MOVE_ICICLE_SPEAR, MOVE_ROCK_BLAST, MOVE_ICE_SHARD},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLOYSTER,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive spiker
        .moves = {MOVE_SPIKES, MOVE_ICICLE_SPEAR, MOVE_RAPID_SPIN, MOVE_ICE_SHARD},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Gengar ----
    {
        .species = SPECIES_GENGAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GENGARITE, // Mega Gengar (Shadow Tag) trapper
        .moves = {MOVE_SHADOW_BALL, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST, MOVE_NASTY_PLOT},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast lead w/ Destiny Bond
        .moves = {MOVE_SHADOW_BALL, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST, MOVE_DESTINY_BOND},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_SHADOW_BALL, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST, MOVE_TRICK},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GENGAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GHOSTIUM_Z, // Never-Ending Nightmare nuke
        .moves = {MOVE_NASTY_PLOT, MOVE_SHADOW_BALL, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Hypno ----
    {
        .species = SPECIES_HYPNO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // CM wall
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_FOUL_PLAY, MOVE_WISH},
        .ability = ABILITY_INSOMNIA,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HYPNO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST},
        .ability = ABILITY_INSOMNIA,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Marowak ----
    {
        .species = SPECIES_MAROWAK,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_THICK_CLUB, // doubles up Attack; Trick Room sweeper
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_KNOCK_OFF, MOVE_BONEMERANG},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAROWAK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_THICK_CLUB, // SD physical sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_FIRE_PUNCH},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Hitmonlee ----
    {
        .species = SPECIES_HITMONLEE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Reckless / Unburden sweeper
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_KNOCK_OFF, MOVE_MACH_PUNCH, MOVE_STONE_EDGE},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HITMONLEE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FIGHTINIUM_Z, // All-Out Pummeling nuke
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_STONE_EDGE, MOVE_BLAZE_KICK},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Hitmonchan ----
    {
        .species = SPECIES_HITMONCHAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Iron Fist bulky attacker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ICE_PUNCH, MOVE_THUNDER_PUNCH, MOVE_MACH_PUNCH},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HITMONCHAN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_ICE_PUNCH, MOVE_MACH_PUNCH},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Weezing ----
    {
        .species = SPECIES_WEEZING,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Levitate phys wall
        .moves = {MOVE_SLUDGE_BOMB, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT, MOVE_TOXIC_SPIKES},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WEEZING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FIRE_BLAST, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Rhydon ----
    {
        .species = SPECIES_RHYDON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_EVIOLITE, // bulky rocker (NFE)
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_STEALTH_ROCK, MOVE_MEGAHORN},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RHYDON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Solid Rock + WP
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_MEGAHORN},
        .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Chansey ----
    {
        .species = SPECIES_CHANSEY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_EVIOLITE, // special wall
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_TOXIC, MOVE_HEAL_BELL},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHANSEY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LUCKY_PUNCH, // Chansey-only crit item; guaranteed first crit
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Tangela ----
    {
        .species = SPECIES_TANGELA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_EVIOLITE, // bulky NFE wall
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_KNOCK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TANGELA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun
        .moves = {MOVE_GROWTH, MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_EARTH_POWER},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Kangaskhan ----
    {
        .species = SPECIES_KANGASKHAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_KANGASKHANITE, // Mega Kangaskhan (Parental Bond)
        .moves = {MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_SUCKER_PUNCH, MOVE_POWER_UP_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KANGASKHAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SILK_SCARF, // no-mega Scrappy attacker
        .moves = {MOVE_FAKE_OUT, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Exeggutor ----
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun nuke
        .moves = {MOVE_LEAF_STORM, MOVE_PSYCHIC, MOVE_SLEEP_POWDER, MOVE_GIGA_DRAIN},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_LEAF_STORM, MOVE_PSYCHIC, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_HARVEST,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Starmie ----
    {
        .species = SPECIES_STARMIE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast special pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYSHOCK, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STARMIE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Natural Cure spinner
        .moves = {MOVE_SCALD, MOVE_RAPID_SPIN, MOVE_RECOVER, MOVE_ICE_BEAM},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STARMIE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Jynx ----
    {
        .species = SPECIES_JYNX,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Lovely Kiss lead
        .moves = {MOVE_LOVELY_KISS, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_NASTY_PLOT},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JYNX,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ICIUM_Z, // Subzero Slammer
        .moves = {MOVE_NASTY_PLOT, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_FOCUS_BLAST},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Electabuzz ----
    {
        .species = SPECIES_ELECTABUZZ,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_EVIOLITE, // bulky NFE pivot
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_ICE_PUNCH, MOVE_VOLT_SWITCH},
        .ability = ABILITY_VITAL_SPIRIT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 4, 252, 252, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ELECTABUZZ,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_ICE_PUNCH, MOVE_THUNDER_WAVE},
        .ability = ABILITY_STATIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Magmar ----
    {
        .species = SPECIES_MAGMAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_EVIOLITE, // bulky NFE
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 4, 252, 252, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAGMAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF,
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_OVERHEAT},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Pinsir ----
    {
        .species = SPECIES_PINSIR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_PINSIRITE, // Mega Pinsir (Aerilate)
        .moves = {MOVE_SWORDS_DANCE, MOVE_RETURN, MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE},
        .ability = ABILITY_HYPER_CUTTER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PINSIR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF, // Moxie revenge (no mega)
        .moves = {MOVE_X_SCISSOR, MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE, MOVE_STONE_EDGE},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Tauros ----
    {
        .species = SPECIES_TAUROS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band
        .moves = {MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT, MOVE_IRON_HEAD},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAUROS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Intimidate lure
        .moves = {MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_THROAT_CHOP},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Gyarados ----
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
        .species = SPECIES_GYARADOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // no-mega DD sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_WATERFALL, MOVE_POWER_WHIP, MOVE_EARTHQUAKE},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GYARADOS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLYINIUM_Z, // DD + Supersonic Skystrike
        .moves = {MOVE_DRAGON_DANCE, MOVE_BOUNCE, MOVE_WATERFALL, MOVE_EARTHQUAKE},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Lapras ----
    {
        .species = SPECIES_LAPRAS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Water Absorb tank
        .moves = {MOVE_FREEZE_DRY, MOVE_SURF, MOVE_THUNDERBOLT, MOVE_ICE_SHARD},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LAPRAS,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_ICIUM_Z, // Snow Warning + Aurora Veil setter
        .moves = {MOVE_AURORA_VEIL, MOVE_FREEZE_DRY, MOVE_SURF, MOVE_PROTECT},
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Vaporeon ----
    {
        .species = SPECIES_VAPOREON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Wish pivot wall
        .moves = {MOVE_SCALD, MOVE_WISH, MOVE_PROTECT, MOVE_FLIP_TURN},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VAPOREON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FLIP_TURN, MOVE_SHADOW_BALL},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Jolteon ----
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
        .species = SPECIES_JOLTEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // fast breaker
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_SHADOW_BALL, MOVE_ALLURING_VOICE},
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Flareon ----
    {
        .species = SPECIES_FLAREON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Flash Fire / Guts band
        .moves = {MOVE_FLARE_BLITZ, MOVE_DOUBLE_EDGE, MOVE_SUPERPOWER, MOVE_QUICK_ATTACK},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLAREON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLAME_ORB, // Guts breaker
        .moves = {MOVE_FACADE, MOVE_FLARE_BLITZ, MOVE_SUPERPOWER, MOVE_QUICK_ATTACK},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Aerodactyl ----
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
        .species = SPECIES_AERODACTYL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // suicide lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_TAUNT, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE},
        .ability = ABILITY_UNNERVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Snorlax ----
    {
        .species = SPECIES_SNORLAX,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Curse setup
        .moves = {MOVE_BODY_SLAM, MOVE_CURSE, MOVE_EARTHQUAKE, MOVE_REST},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SNORLAX,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_BODY_SLAM, MOVE_HIGH_HORSEPOWER, MOVE_CRUNCH, MOVE_SELF_DESTRUCT},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(132, 252, 0, 0, 0, 124),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SNORLAX,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SNORLIUM_Z, // Pulverizing Pancake (Gluttony recovery elsewhere)
        .moves = {MOVE_BELLY_DRUM, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_CRUNCH},
        .ability = ABILITY_GLUTTONY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Articuno ----
    {
        .species = SPECIES_ARTICUNO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky special wall
        .moves = {MOVE_FREEZE_DRY, MOVE_HURRICANE, MOVE_ROOST, MOVE_HAZE},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARTICUNO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_BLIZZARD, MOVE_HURRICANE, MOVE_FREEZE_DRY, MOVE_ROOST},
        .ability = ABILITY_SNOW_CLOAK,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Zapdos ----
    {
        .species = SPECIES_ZAPDOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // pivot
        .moves = {MOVE_THUNDERBOLT, MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_ROOST},
        .ability = ABILITY_STATIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZAPDOS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive sub-roost
        .moves = {MOVE_THUNDERBOLT, MOVE_HURRICANE, MOVE_SUBSTITUTE, MOVE_ROOST},
        .ability = ABILITY_STATIC,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(248, 0, 0, 84, 0, 176),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Moltres ----
    {
        .species = SPECIES_MOLTRES,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky pivot
        .moves = {MOVE_FIRE_BLAST, MOVE_HURRICANE, MOVE_ROOST, MOVE_WILL_O_WISP},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MOLTRES,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_HURRICANE, MOVE_SCORCHING_SANDS, MOVE_U_TURN},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Dragonite ----
    {
        .species = SPECIES_DRAGONITE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Multiscale DD sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_ROOST},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGONITE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Extreme Speed band
        .moves = {MOVE_EXTREME_SPEED, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_FIRE_PUNCH},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGONITE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLYINIUM_Z, // DD + Supersonic Skystrike
        .moves = {MOVE_DRAGON_DANCE, MOVE_FLY, MOVE_EARTHQUAKE, MOVE_OUTRAGE},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Mewtwo ----
    {
        .species = SPECIES_MEWTWO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MEWTWONITE_Y, // Mega Mewtwo Y (Insomnia) special nuke
        .moves = {MOVE_PSYSTRIKE, MOVE_AURA_SPHERE, MOVE_ICE_BEAM, MOVE_NASTY_PLOT},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEWTWO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MEWTWONITE_X, // Mega Mewtwo X (Steadfast) physical
        .moves = {MOVE_BULK_UP, MOVE_PSYCHIC_FANGS, MOVE_DRAIN_PUNCH, MOVE_ICE_PUNCH},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEWTWO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // no-mega all-out attacker
        .moves = {MOVE_PSYSTRIKE, MOVE_AURA_SPHERE, MOVE_FIRE_BLAST, MOVE_ICE_BEAM},
        .ability = ABILITY_UNNERVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Mew ----
    {
        .species = SPECIES_MEW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed nasty plot
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHIC, MOVE_AURA_SPHERE, MOVE_FIRE_BLAST},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEW,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // utility lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_TAUNT, MOVE_ROOST},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // physical pivot
        .moves = {MOVE_CLOSE_COMBAT, MOVE_U_TURN, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
};

const u16 gFactoryCompetitiveMonsCount = ARRAY_COUNT(gFactoryCompetitiveMons);
