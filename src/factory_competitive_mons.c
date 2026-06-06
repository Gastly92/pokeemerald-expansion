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

    // ---- Magnezone (Gen IV evolution of Magneton; innate Levitate = Ground-immune) ----
    {
        .species = SPECIES_MAGNEZONE,
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
        // Innate Levitate already dodges Ground, so no Air Balloon needed — run a
        // bulky Analytic special tank instead.
        .species = SPECIES_MAGNEZONE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Analytic special tank
        .moves = {MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
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

    // ---- Rhyperior (Gen IV evolution of Rhydon) ----
    {
        .species = SPECIES_RHYPERIOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky Lightning Rod rocker
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_STEALTH_ROCK, MOVE_ICE_PUNCH},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RHYPERIOR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Solid Rock + WP sweeper
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

    // ---- Tangrowth (Gen IV evolution of Tangela) ----
    {
        .species = SPECIES_TANGROWTH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator wall
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_KNOCK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TANGROWTH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Regenerator physical tank
        .moves = {MOVE_POWER_WHIP, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
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

    // ---- Electivire (Gen IV evolution of Electabuzz) ----
    {
        .species = SPECIES_ELECTIVIRE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Motor Drive mixed sweeper
        .moves = {MOVE_WILD_CHARGE, MOVE_ICE_PUNCH, MOVE_EARTHQUAKE, MOVE_CROSS_CHOP},
        .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ELECTIVIRE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // bulky Vital Spirit pivot
        .moves = {MOVE_WILD_CHARGE, MOVE_ICE_PUNCH, MOVE_EARTHQUAKE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_VITAL_SPIRIT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(112, 252, 0, 144, 0, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Magmortar (Gen IV evolution of Magmar) ----
    {
        .species = SPECIES_MAGMORTAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Flash Fire special breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_OVERHEAT},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAGMORTAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Vital Spirit revenge killer
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
        .ability = ABILITY_VITAL_SPIRIT,
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

    // ============================================================
    //                       Generation II
    // ============================================================

    // ---- Meganium ----
    {
        .species = SPECIES_MEGANIUM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky defensive pivot / cleric
        .moves = {MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_AROMATHERAPY, MOVE_BODY_PRESS},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MIRACLE_SEED, // dragon dance physical attacker
        .moves = {MOVE_DRAGON_DANCE, MOVE_HORN_LEECH, MOVE_PLAY_ROUGH, MOVE_EARTHQUAKE},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEGANIUM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // defensive spreader with hazards
        .moves = {MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_TOXIC, MOVE_SYNTHESIS},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Typhlosion ----
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_EARTH_POWER, MOVE_SOLAR_BEAM},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // nasty plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYPHLOSION,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAT_ROCK, // sun setter for the team
        .moves = {MOVE_SUNNY_DAY, MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_EARTH_POWER},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Feraligatr ----
    {
        .species = SPECIES_FERALIGATR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // dragon dance physical sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_LIQUIDATION, MOVE_ICE_PUNCH, MOVE_CRUNCH},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FERALIGATR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band wallbreaker with priority
        .moves = {MOVE_LIQUIDATION, MOVE_AQUA_JET, MOVE_ICE_PUNCH, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FERALIGATR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky belly-drum sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_LIQUIDATION, MOVE_AQUA_JET, MOVE_ICE_PUNCH},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Crobat ----
    {
        .species = SPECIES_CROBAT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast physical attacker / pivot
        .moves = {MOVE_BRAVE_BIRD, MOVE_GUNK_SHOT, MOVE_CLOSE_COMBAT, MOVE_U_TURN},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CROBAT,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // defensive defogger / status
        .moves = {MOVE_BRAVE_BIRD, MOVE_DEFOG, MOVE_ROOST, MOVE_TAUNT},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CROBAT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band pivot wallbreaker
        .moves = {MOVE_BRAVE_BIRD, MOVE_GUNK_SHOT, MOVE_U_TURN, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Ampharos ----
    {
        .species = SPECIES_AMPHAROS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_AMPHAROSITE, // Mega Ampharos (Mold Breaker, Electric/Dragon)
        .moves = {MOVE_THUNDERBOLT, MOVE_DRAGON_PULSE, MOVE_FOCUS_BLAST, MOVE_VOLT_SWITCH},
        .ability = ABILITY_STATIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AMPHAROS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker (no mega)
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_DRAGON_PULSE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_STATIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AMPHAROS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // bulky special tank
        .moves = {MOVE_THUNDERBOLT, MOVE_DRAGON_PULSE, MOVE_POWER_WHIP, MOVE_VOLT_SWITCH},
        .ability = ABILITY_STATIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Bellossom ----
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // chlorophyll sun sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_GIGA_DRAIN, MOVE_MOONBLAST, MOVE_WEATHER_BALL},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BELLOSSOM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive quiver dance + sleep
        .moves = {MOVE_QUIVER_DANCE, MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_MOONLIGHT},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Azumarill ----
    {
        .species = SPECIES_AZUMARILL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Huge Power band breaker w/ priority
        .moves = {MOVE_LIQUIDATION, MOVE_PLAY_ROUGH, MOVE_AQUA_JET, MOVE_ICE_PUNCH},
        .ability = ABILITY_HUGE_POWER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(92, 252, 0, 164, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AZUMARILL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SITRUS_BERRY, // belly drum + aqua jet sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_AQUA_JET, MOVE_PLAY_ROUGH, MOVE_LIQUIDATION},
        .ability = ABILITY_HUGE_POWER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(92, 252, 0, 164, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AZUMARILL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with utility
        .moves = {MOVE_PLAY_ROUGH, MOVE_AQUA_JET, MOVE_KNOCK_OFF, MOVE_LIQUIDATION},
        .ability = ABILITY_HUGE_POWER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Sudowoodo ----
    {
        .species = SPECIES_SUDOWOODO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Rock Head reckless band attacker
        .moves = {MOVE_HEAD_SMASH, MOVE_EARTHQUAKE, MOVE_WOOD_HAMMER, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SUDOWOODO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // sturdy physical wall w/ rocks
        .moves = {MOVE_STEALTH_ROCK, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_BODY_PRESS},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Politoed ----
    {
        .species = SPECIES_POLITOED,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_DAMP_ROCK, // Drizzle rain setter
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_ENCORE},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_POLITOED,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky water with utility
        .moves = {MOVE_SCALD, MOVE_ICE_BEAM, MOVE_ENCORE, MOVE_PROTECT},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 200, 0, 0, 56),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_POLITOED,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // rain-boosted special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FOCUS_BLAST, MOVE_FLIP_TURN},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Jumpluff ----
    {
        .species = SPECIES_JUMPLUFF,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // fast sleep + leech seed staller
        .moves = {MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_SUBSTITUTE, MOVE_GIGA_DRAIN},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JUMPLUFF,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // chlorophyll offensive utility
        .moves = {MOVE_GIGA_DRAIN, MOVE_ACROBATICS, MOVE_SLEEP_POWDER, MOVE_STRENGTH_SAP},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Sunflora ----
    {
        .species = SPECIES_SUNFLORA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Chlorophyll sun nuke
        .moves = {MOVE_LEAF_STORM, MOVE_EARTH_POWER, MOVE_WEATHER_BALL, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SUNFLORA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // growth sun sweeper
        .moves = {MOVE_GROWTH, MOVE_GIGA_DRAIN, MOVE_WEATHER_BALL, MOVE_EARTH_POWER},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Quagsire ----
    {
        .species = SPECIES_QUAGSIRE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Unaware physical wall
        .moves = {MOVE_EARTHQUAKE, MOVE_SCALD, MOVE_RECOVER, MOVE_TOXIC},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_QUAGSIRE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulk water absorber
        .moves = {MOVE_EARTHQUAKE, MOVE_LIQUIDATION, MOVE_ICE_PUNCH, MOVE_KNOCK_OFF},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Espeon ----
    {
        .species = SPECIES_ESPEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Magic Bounce special sweeper
        .moves = {MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_SHADOW_BALL, MOVE_CALM_MIND},
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ESPEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // hazard-bouncing special breaker
        .moves = {MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_SHADOW_BALL, MOVE_TRICK},
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Umbreon ----
    {
        .species = SPECIES_UMBREON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // wish-passing special wall
        .moves = {MOVE_FOUL_PLAY, MOVE_WISH, MOVE_PROTECT, MOVE_TOXIC},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_UMBREON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // physically defensive cleric
        .moves = {MOVE_FOUL_PLAY, MOVE_HEAL_BELL, MOVE_WISH, MOVE_PROTECT},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Slowking ----
    {
        .species = SPECIES_SLOWKING,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator special wall / pivot
        .moves = {MOVE_SCALD, MOVE_FUTURE_SIGHT, MOVE_SLACK_OFF, MOVE_THUNDER_WAVE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 16, 0, 0, 240),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLOWKING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // bulky special attacker
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYSHOCK, MOVE_ICE_BEAM, MOVE_FIRE_BLAST},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLOWKING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_TWISTED_SPOON, // calm mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_SCALD, MOVE_SLACK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Girafarig ----
    {
        .species = SPECIES_GIRAFARIG,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // nasty plot special sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYSHOCK, MOVE_HYPER_VOICE, MOVE_THUNDERBOLT},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GIRAFARIG,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // calm mind + dual screens pivot
        .moves = {MOVE_CALM_MIND, MOVE_STORED_POWER, MOVE_SHADOW_BALL, MOVE_THUNDERBOLT},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Forretress ----
    {
        .species = SPECIES_FORRETRESS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // hazard setter / spinner
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_RAPID_SPIN, MOVE_GYRO_BALL},
        .ability = ABILITY_STURDY,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FORRETRESS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // physical wall with Volt Switch pivot
        .moves = {MOVE_GYRO_BALL, MOVE_VOLT_SWITCH, MOVE_RAPID_SPIN, MOVE_BODY_PRESS},
        .ability = ABILITY_STURDY,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Steelix ----
    {
        .species = SPECIES_STEELIX,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_STEELIXITE, // Mega Steelix (Sand Force) physical wall
        .moves = {MOVE_EARTHQUAKE, MOVE_HEAVY_SLAM, MOVE_STEALTH_ROCK, MOVE_BODY_PRESS},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STEELIX,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy hazard tank (no mega)
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_HEAVY_SLAM, MOVE_TOXIC},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STEELIX,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band trapper-style breaker
        .moves = {MOVE_EARTHQUAKE, MOVE_HEAVY_SLAM, MOVE_STONE_EDGE, MOVE_ICE_PUNCH},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Granbull ----
    {
        .species = SPECIES_GRANBULL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Intimidate band breaker
        .moves = {MOVE_PLAY_ROUGH, MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GRANBULL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with status
        .moves = {MOVE_PLAY_ROUGH, MOVE_KNOCK_OFF, MOVE_THUNDER_WAVE, MOVE_HEAL_BELL},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Qwilfish ----
    {
        .species = SPECIES_QWILFISH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Intimidate hazard setter
        .moves = {MOVE_SPIKES, MOVE_TOXIC_SPIKES, MOVE_LIQUIDATION, MOVE_HAZE},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_QWILFISH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // swift swim rain attacker
        .moves = {MOVE_LIQUIDATION, MOVE_GUNK_SHOT, MOVE_ICE_PUNCH, MOVE_AQUA_JET},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Scizor ----
    {
        .species = SPECIES_SCIZOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SCIZORITE, // Mega Scizor (Technician) swords dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_BULLET_PUNCH, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCIZOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Technician band breaker w/ priority
        .moves = {MOVE_BULLET_PUNCH, MOVE_U_TURN, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCIZOR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive defogger / pivot
        .moves = {MOVE_BULLET_PUNCH, MOVE_DEFOG, MOVE_ROOST, MOVE_U_TURN},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Heracross ----
    {
        .species = SPECIES_HERACROSS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HERACRONITE, // Mega Heracross (Skill Link) multi-hit breaker
        .moves = {MOVE_PIN_MISSILE, MOVE_ROCK_BLAST, MOVE_BULLET_SEED, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HERACROSS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Guts band breaker (no mega)
        .moves = {MOVE_CLOSE_COMBAT, MOVE_MEGAHORN, MOVE_KNOCK_OFF, MOVE_ROCK_SLIDE},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HERACROSS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLAME_ORB, // Guts self-status breaker
        .moves = {MOVE_FACADE, MOVE_CLOSE_COMBAT, MOVE_MEGAHORN, MOVE_KNOCK_OFF},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Ursaring ----
    {
        .species = SPECIES_URSARING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts Facade wallbreaker
        .moves = {MOVE_FACADE, MOVE_CLOSE_COMBAT, MOVE_CRUNCH, MOVE_EARTHQUAKE},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_URSARING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band breaker with coverage
        .moves = {MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_CRUNCH, MOVE_EARTHQUAKE},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Octillery ----
    {
        .species = SPECIES_OCTILLERY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Sniper / special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FIRE_BLAST, MOVE_ENERGY_BALL},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_OCTILLERY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // swift swim rain attacker
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_GUNK_SHOT, MOVE_ENERGY_BALL},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Mantine ----
    {
        .species = SPECIES_MANTINE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky defogger / special wall
        .moves = {MOVE_SCALD, MOVE_DEFOG, MOVE_ROOST, MOVE_TOXIC},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MANTINE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HURRICANE, MOVE_FLIP_TURN},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Skarmory ----
    {
        .species = SPECIES_SKARMORY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // hazard setter physical wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_ROOST, MOVE_BODY_PRESS},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SKARMORY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // defogger pivot
        .moves = {MOVE_BRAVE_BIRD, MOVE_DEFOG, MOVE_ROOST, MOVE_WHIRLWIND},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Houndoom ----
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HOUNDOOMINITE, // Mega Houndoom (Solar Power) nasty plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_FIRE_BLAST, MOVE_DARK_PULSE, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Flash Fire special breaker (no mega)
        .moves = {MOVE_FIRE_BLAST, MOVE_DARK_PULSE, MOVE_SLUDGE_BOMB, MOVE_FOCUS_BLAST},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HOUNDOOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves = {MOVE_FIRE_BLAST, MOVE_DARK_PULSE, MOVE_SLUDGE_BOMB, MOVE_OVERHEAT},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Kingdra ----
    {
        .species = SPECIES_KINGDRA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves = {MOVE_HYDRO_PUMP, MOVE_DRACO_METEOR, MOVE_ICE_BEAM, MOVE_FLIP_TURN},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KINGDRA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // dragon dance physical sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_WATERFALL, MOVE_OUTRAGE, MOVE_ICE_PUNCH},
        .ability = ABILITY_SNIPER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KINGDRA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // specs special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_DRACO_METEOR, MOVE_ICE_BEAM, MOVE_FLIP_TURN},
        .ability = ABILITY_SNIPER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Donphan ----
    {
        .species = SPECIES_DONPHAN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy hazard setter / spinner
        .moves = {MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_ICE_SHARD},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DONPHAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band breaker with priority
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_SHARD, MOVE_KNOCK_OFF, MOVE_STONE_EDGE},
        .ability = ABILITY_SAND_VEIL,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DONPHAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulk pivot
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_SHARD, MOVE_HEAVY_SLAM, MOVE_KNOCK_OFF},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Porygon2 ---- (Eviolite NFE niche: Porygon-Z is a glass cannon, Porygon2 the bulky tank)
    {
        .species = SPECIES_PORYGON2,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_EVIOLITE, // Trace bulky tank / recovery (Porygon2 NFE niche)
        .moves = {MOVE_TRI_ATTACK, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_THUNDER_WAVE},
        .ability = ABILITY_TRACE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PORYGON2,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_EVIOLITE, // Download offensive Eviolite pivot
        .moves = {MOVE_TRI_ATTACK, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_RECOVER},
        .ability = ABILITY_DOWNLOAD,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Stantler ----
    {
        .species = SPECIES_STANTLER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Intimidate mixed attacker
        .moves = {MOVE_DOUBLE_EDGE, MOVE_PSYCHIC, MOVE_EARTHQUAKE, MOVE_JUMP_KICK},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STANTLER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_DOUBLE_EDGE, MOVE_MEGAHORN, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Smeargle ----
    {
        .species = SPECIES_SMEARGLE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // suicide lead, hazards + sleep (sash = one-shot guard)
        .moves = {MOVE_SPORE, MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_WHIRLWIND},
        .ability = ABILITY_OWN_TEMPO,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SMEARGLE,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // doubles support lead (Spore + Fake Out)
        .moves = {MOVE_SPORE, MOVE_FAKE_OUT, MOVE_FOLLOW_ME, MOVE_KINGS_SHIELD},
        .ability = ABILITY_MOODY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Hitmontop ----
    {
        .species = SPECIES_HITMONTOP,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate doubles support attacker
        .moves = {MOVE_FAKE_OUT, MOVE_CLOSE_COMBAT, MOVE_TRIPLE_AXEL, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HITMONTOP,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Technician spinner / pivot
        .moves = {MOVE_RAPID_SPIN, MOVE_CLOSE_COMBAT, MOVE_MACH_PUNCH, MOVE_TRIPLE_AXEL},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Miltank ----
    {
        .species = SPECIES_MILTANK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky physical wall / cleric
        .moves = {MOVE_BODY_SLAM, MOVE_MILK_DRINK, MOVE_HEAL_BELL, MOVE_STEALTH_ROCK},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MILTANK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Scrappy band breaker
        .moves = {MOVE_DOUBLE_EDGE, MOVE_BODY_PRESS, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Blissey ----
    {
        .species = SPECIES_BLISSEY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // special wall / cleric
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_HEAL_BELL, MOVE_TOXIC},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(4, 0, 252, 0, 0, 252),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLISSEY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // stallbreaker support with hazards
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(4, 0, 252, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Tyranitar ----
    {
        .species = SPECIES_TYRANITAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_TYRANITARITE, // Mega Tyranitar (Sand Stream) dragon dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sand Stream band breaker
        .moves = {MOVE_STONE_EDGE, MOVE_CRUNCH, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // sand tank with hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_THUNDER_WAVE},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYRANITAR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank in sand
        .moves = {MOVE_STONE_EDGE, MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_EARTHQUAKE},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Raikou ----
    {
        .species = SPECIES_RAIKOU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // calm mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_AURA_SPHERE, MOVE_SHADOW_BALL},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAIKOU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // specs special breaker / pivot
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_AURA_SPHERE, MOVE_SHADOW_BALL},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAIKOU,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_AURA_SPHERE, MOVE_WEATHER_BALL},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Entei ----
    {
        .species = SPECIES_ENTEI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sacred Fire band breaker w/ priority
        .moves = {MOVE_SACRED_FIRE, MOVE_EXTREME_SPEED, MOVE_STONE_EDGE, MOVE_FLARE_BLITZ},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ENTEI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // scarf revenge killer
        .moves = {MOVE_SACRED_FIRE, MOVE_EXTREME_SPEED, MOVE_STONE_EDGE, MOVE_BULLDOZE},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ENTEI,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot
        .moves = {MOVE_SACRED_FIRE, MOVE_EXTREME_SPEED, MOVE_STONE_EDGE, MOVE_MORNING_SUN},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Suicune ----
    {
        .species = SPECIES_SUICUNE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // calm mind bulky sweeper
        .moves = {MOVE_CALM_MIND, MOVE_SCALD, MOVE_ICE_BEAM, MOVE_REST},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SUICUNE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // offensive calm mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_TERA_BLAST},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SUICUNE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // bulky defogger / wall
        .moves = {MOVE_SCALD, MOVE_DEFOG, MOVE_REST, MOVE_SLEEP_TALK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Lugia ----
    {
        .species = SPECIES_LUGIA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Multiscale physical wall / defogger
        .moves = {MOVE_AEROBLAST, MOVE_ROOST, MOVE_DEFOG, MOVE_TOXIC},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUGIA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // calm mind bulky sweeper
        .moves = {MOVE_CALM_MIND, MOVE_AEROBLAST, MOVE_PSYCHIC, MOVE_ROOST},
        .ability = ABILITY_MULTISCALE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 60, 196, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Ho-Oh ----
    {
        .species = SPECIES_HO_OH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sacred Fire band breaker
        .moves = {MOVE_SACRED_FIRE, MOVE_BRAVE_BIRD, MOVE_EARTHQUAKE, MOVE_EXTREME_SPEED},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HO_OH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot / cleric
        .moves = {MOVE_SACRED_FIRE, MOVE_BRAVE_BIRD, MOVE_RECOVER, MOVE_WHIRLWIND},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Celebi ---- (innate Levitate — Ground immune, never give an Air Balloon)
    {
        .species = SPECIES_CELEBI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // nasty plot special sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_GIGA_DRAIN, MOVE_PSYCHIC, MOVE_EARTH_POWER},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CELEBI,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot with utility
        .moves = {MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_RECOVER, MOVE_U_TURN},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 240, 0, 0, 16),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CELEBI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // calm mind bulky sweeper
        .moves = {MOVE_CALM_MIND, MOVE_GIGA_DRAIN, MOVE_PSYCHIC, MOVE_RECOVER},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 60, 196, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation III
    // ============================================================

    // ---- Sceptile ----
    {
        .species = SPECIES_SCEPTILE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SCEPTILITE, // Mega Sceptile (Lightning Rod, Grass/Dragon) — fast special breaker
        .moves = {MOVE_LEAF_STORM, MOVE_DRAGON_PULSE, MOVE_FOCUS_BLAST, MOVE_GIGA_DRAIN},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCEPTILE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // physical Swords Dance sweeper (no mega)
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCEPTILE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // sun special revenge killer
        .moves = {MOVE_LEAF_STORM, MOVE_DRAGON_PULSE, MOVE_FOCUS_BLAST, MOVE_GIGA_DRAIN},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Blaziken ----
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLAZIKENITE, // Mega Blaziken (Speed Boost) — snowballing sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_FLARE_BLITZ, MOVE_HIGH_JUMP_KICK, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Speed Boost mixed wallbreaker (no mega)
        .moves = {MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_STONE_EDGE},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLAZIKEN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // immediate banded breaker
        .moves = {MOVE_FLARE_BLITZ, MOVE_HIGH_JUMP_KICK, MOVE_KNOCK_OFF, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Swampert ----
    {
        .species = SPECIES_SWAMPERT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SWAMPERTITE, // Mega Swampert (Swift Swim) — rain sweeper
        .moves = {MOVE_WATERFALL, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH, MOVE_SUPERPOWER},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SWAMPERT,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky hazard setter
        .moves = {MOVE_STEALTH_ROCK, MOVE_SCALD, MOVE_EARTHQUAKE, MOVE_ICE_BEAM},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 216, 0, 0, 40),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SWAMPERT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed-tank pivot
        .moves = {MOVE_FLIP_TURN, MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_POWER_GEM},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 128, 0, 0, 128, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Mightyena ----
    {
        .species = SPECIES_MIGHTYENA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLAME_ORB, // Quick Feet status-fueled attacker
        .moves = {MOVE_FACADE, MOVE_CRUNCH, MOVE_PLAY_ROUGH, MOVE_FIRE_FANG},
        .ability = ABILITY_QUICK_FEET,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MIGHTYENA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Intimidate hit-and-run band
        .moves = {MOVE_CRUNCH, MOVE_PLAY_ROUGH, MOVE_SUCKER_PUNCH, MOVE_FIRE_FANG},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Linoone ----
    {
        .species = SPECIES_LINOONE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY, // Belly Drum + Extreme Speed sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_EXTREME_SPEED, MOVE_SEED_BOMB, MOVE_KNOCK_OFF},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LINOONE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts priority breaker
        .moves = {MOVE_FACADE, MOVE_EXTREME_SPEED, MOVE_KNOCK_OFF, MOVE_SEED_BOMB},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Ludicolo ----
    {
        .species = SPECIES_LUDICOLO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves = {MOVE_HYDRO_PUMP, MOVE_GIGA_DRAIN, MOVE_ICE_BEAM, MOVE_FOCUS_BLAST},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUDICOLO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Rain Dish bulky pivot
        .moves = {MOVE_SCALD, MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_RAIN_DANCE},
        .ability = ABILITY_RAIN_DISH,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUDICOLO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_WATERIUM_Z, // Hydro Vortex nuke under rain
        .moves = {MOVE_HYDRO_PUMP, MOVE_ENERGY_BALL, MOVE_ICE_BEAM, MOVE_RAIN_DANCE},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Shiftry ----
    {
        .species = SPECIES_SHIFTRY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves = {MOVE_LEAF_STORM, MOVE_KNOCK_OFF, MOVE_SUCKER_PUNCH, MOVE_HEAT_WAVE},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SHIFTRY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Sticky Web lead, sash = one-shot entry guard
        .moves = {MOVE_STICKY_WEB, MOVE_LEAF_BLADE, MOVE_KNOCK_OFF, MOVE_DEFOG},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SHIFTRY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Swords-less band breaker
        .moves = {MOVE_KNOCK_OFF, MOVE_LEAF_BLADE, MOVE_SUCKER_PUNCH, MOVE_X_SCISSOR},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Swellow ----
    {
        .species = SPECIES_SWELLOW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_TOXIC_ORB, // Guts Facade sweeper
        .moves = {MOVE_FACADE, MOVE_BRAVE_BIRD, MOVE_U_TURN, MOVE_QUICK_ATTACK},
        .ability = ABILITY_GUTS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SWELLOW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Scrappy-less band hit-and-run
        .moves = {MOVE_BRAVE_BIRD, MOVE_FACADE, MOVE_U_TURN, MOVE_STEEL_WING},
        .ability = ABILITY_GUTS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Pelipper ----
    {
        .species = SPECIES_PELIPPER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_DAMP_ROCK, // Drizzle rain setter
        .moves = {MOVE_HURRICANE, MOVE_HYDRO_PUMP, MOVE_U_TURN, MOVE_ROOST},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(248, 0, 0, 8, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PELIPPER,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Defog pivot
        .moves = {MOVE_SCALD, MOVE_HURRICANE, MOVE_DEFOG, MOVE_ROOST},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(248, 0, 252, 0, 0, 8),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Gardevoir ----
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GARDEVOIRITE, // Mega Gardevoir (Pixilate) — Hyper Voice nuke
        .moves = {MOVE_HYPER_VOICE, MOVE_PSYSHOCK, MOVE_MOONBLAST, MOVE_FOCUS_BLAST},
        .ability = ABILITY_TRACE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Trace revenge killer (no mega)
        .moves = {MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_TRICK},
        .ability = ABILITY_TRACE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARDEVOIR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Calm Mind setup sweeper
        .moves = {MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_PSYSHOCK, MOVE_FOCUS_BLAST},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Masquerain ----
    {
        .species = SPECIES_MASQUERAIN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Quiver Dance sweeper, sash = one-shot entry guard
        .moves = {MOVE_QUIVER_DANCE, MOVE_BUG_BUZZ, MOVE_AIR_SLASH, MOVE_HYDRO_PUMP},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Breloom ----
    {
        .species = SPECIES_BRELOOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Technician-less Spore + priority breaker
        .moves = {MOVE_SPORE, MOVE_BULLET_SEED, MOVE_MACH_PUNCH, MOVE_ROCK_TOMB},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRELOOM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal Sub-Punch staller
        .moves = {MOVE_SPORE, MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_SEED_BOMB},
        .ability = ABILITY_POISON_HEAL,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(236, 0, 0, 36, 0, 236),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRELOOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // Spore lead, sash = one-shot entry guard
        .moves = {MOVE_SPORE, MOVE_BULLET_SEED, MOVE_MACH_PUNCH, MOVE_SWORDS_DANCE},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Slaking ----
    {
        .species = SPECIES_SLAKING,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Truant banded wallbreaker
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_GIGA_IMPACT, MOVE_NIGHT_SLASH},
        .ability = ABILITY_TRUANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLAKING,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // coverage breaker
        .moves = {MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_GUNK_SHOT, MOVE_ICE_PUNCH},
        .ability = ABILITY_TRUANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Ninjask ----
    {
        .species = SPECIES_NINJASK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Speed Boost fast sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_X_SCISSOR, MOVE_AERIAL_ACE, MOVE_DIG},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NINJASK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // baton-pass-style speed control lead
        .moves = {MOVE_SWORDS_DANCE, MOVE_SUBSTITUTE, MOVE_X_SCISSOR, MOVE_PROTECT},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Shedinja ----
    {
        .species = SPECIES_SHEDINJA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Wonder Guard sweeper (boots dodge hazards)
        .moves = {MOVE_SWORDS_DANCE, MOVE_X_SCISSOR, MOVE_SHADOW_SNEAK, MOVE_WILL_O_WISP},
        .ability = ABILITY_WONDER_GUARD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Exploud ----
    {
        .species = SPECIES_EXPLOUD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Boomburst spam breaker
        .moves = {MOVE_BOOMBURST, MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_ICE_BEAM},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EXPLOUD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed sound attacker
        .moves = {MOVE_BOOMBURST, MOVE_OVERHEAT, MOVE_SURF, MOVE_FOCUS_BLAST},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Hariyama ----
    {
        .species = SPECIES_HARIYAMA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts status-fueled bruiser
        .moves = {MOVE_FACADE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_FAKE_OUT},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HARIYAMA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // Thick Fat special tank
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_HEAVY_SLAM, MOVE_BULLET_PUNCH},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(168, 252, 0, 0, 0, 88),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Sableye ----
    {
        .species = SPECIES_SABLEYE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SABLENITE, // Mega Sableye (Magic Bounce) — defensive pivot
        .moves = {MOVE_CALM_MIND, MOVE_DARK_PULSE, MOVE_RECOVER, MOVE_WILL_O_WISP},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SABLEYE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Prankster utility staller (no mega)
        .moves = {MOVE_WILL_O_WISP, MOVE_RECOVER, MOVE_KNOCK_OFF, MOVE_TAUNT},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Mawile ----
    {
        .species = SPECIES_MAWILE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MAWILITE, // Mega Mawile (Huge Power) — Swords Dance wallbreaker
        .moves = {MOVE_SWORDS_DANCE, MOVE_PLAY_ROUGH, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAWILE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Intimidate utility setter (no mega)
        .moves = {MOVE_STEALTH_ROCK, MOVE_PLAY_ROUGH, MOVE_IRON_HEAD, MOVE_THUNDER_WAVE},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Aggron ----
    {
        .species = SPECIES_AGGRON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_AGGRONITE, // Mega Aggron (Filter, pure Steel) — physical wall
        .moves = {MOVE_HEAVY_SLAM, MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK, MOVE_ROAR},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AGGRON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Rock Head band breaker (no mega)
        .moves = {MOVE_HEAD_SMASH, MOVE_HEAVY_SLAM, MOVE_EARTHQUAKE, MOVE_AVALANCHE},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AGGRON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Sturdy bait setup tank (no mega)
        .moves = {MOVE_AUTOTOMIZE, MOVE_HEAVY_SLAM, MOVE_EARTHQUAKE, MOVE_STONE_EDGE},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Medicham ----
    {
        .species = SPECIES_MEDICHAM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MEDICHAMITE, // Mega Medicham (Pure Power) — fast breaker
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_ZEN_HEADBUTT, MOVE_ICE_PUNCH, MOVE_FAKE_OUT},
        .ability = ABILITY_PURE_POWER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEDICHAM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF, // Pure Power revenge killer (no mega)
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_ZEN_HEADBUTT, MOVE_ICE_PUNCH, MOVE_TRICK},
        .ability = ABILITY_PURE_POWER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Manectric ----
    {
        .species = SPECIES_MANECTRIC,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_MANECTITE, // Mega Manectric (Intimidate) — fast special pivot
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_FLAMETHROWER, MOVE_ENERGY_BALL},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MANECTRIC,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Lightning Rod special breaker (no mega)
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_OVERHEAT, MOVE_THUNDER},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Plusle ---- (doubles support)
    {
        .species = SPECIES_PLUSLE,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Plus partner special attacker
        .moves = {MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_HELPING_HAND, MOVE_NASTY_PLOT},
        .ability = ABILITY_PLUS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Minun ---- (doubles support)
    {
        .species = SPECIES_MINUN,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Minus support pivot
        .moves = {MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_HELPING_HAND, MOVE_NASTY_PLOT},
        .ability = ABILITY_MINUS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Volbeat ---- (doubles support)
    {
        .species = SPECIES_VOLBEAT,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_DAMP_ROCK, // Prankster Tailwind/Rain support
        .moves = {MOVE_TAILWIND, MOVE_RAIN_DANCE, MOVE_THUNDER_WAVE, MOVE_U_TURN},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(248, 0, 0, 8, 0, 252),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Illumise ---- (doubles support)
    {
        .species = SPECIES_ILLUMISE,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB, // Prankster utility setter
        .moves = {MOVE_TAILWIND, MOVE_ENCORE, MOVE_HELPING_HAND, MOVE_BUG_BUZZ},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(248, 0, 8, 0, 0, 252),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Swalot ----
    {
        .species = SPECIES_SWALOT,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Sticky Hold status tank
        .moves = {MOVE_SLUDGE_BOMB, MOVE_TOXIC, MOVE_PAIN_SPLIT, MOVE_ENCORE},
        .ability = ABILITY_LIQUID_OOZE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Sharpedo ----
    {
        .species = SPECIES_SHARPEDO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SHARPEDONITE, // Mega Sharpedo (Strong Jaw) — Speed Boost sweeper
        .moves = {MOVE_PROTECT, MOVE_CRUNCH, MOVE_WATERFALL, MOVE_PSYCHIC_FANGS},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SHARPEDO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Speed Boost wallbreaker (no mega)
        .moves = {MOVE_CRUNCH, MOVE_WATERFALL, MOVE_CLOSE_COMBAT, MOVE_ICE_FANG},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SHARPEDO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF, // rough-skin revenge killer
        .moves = {MOVE_CRUNCH, MOVE_WATERFALL, MOVE_CLOSE_COMBAT, MOVE_DESTINY_BOND},
        .ability = ABILITY_ROUGH_SKIN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Wailord ----
    {
        .species = SPECIES_WAILORD,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Water Spout cannon
        .moves = {MOVE_WATER_SPOUT, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HYPER_VOICE},
        .ability = ABILITY_WATER_VEIL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Camerupt ----
    {
        .species = SPECIES_CAMERUPT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CAMERUPTITE, // Mega Camerupt (Sheer Force) — Trick Room breaker
        .moves = {MOVE_ERUPTION, MOVE_EARTH_POWER, MOVE_FIRE_BLAST, MOVE_ANCIENT_POWER},
        .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CAMERUPT,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Solid Rock special wall (no mega)
        .moves = {MOVE_LAVA_PLUME, MOVE_EARTH_POWER, MOVE_STEALTH_ROCK, MOVE_TOXIC},
        .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Torkoal ----
    {
        .species = SPECIES_TORKOAL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_HEAT_ROCK, // Drought sun setter
        .moves = {MOVE_ERUPTION, MOVE_LAVA_PLUME, MOVE_SOLAR_BEAM, MOVE_EARTH_POWER},
        .ability = ABILITY_DROUGHT,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TORKOAL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // White Smoke defensive setter
        .moves = {MOVE_LAVA_PLUME, MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_YAWN},
        .ability = ABILITY_WHITE_SMOKE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Grumpig ----
    {
        .species = SPECIES_GRUMPIG,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Thick Fat Calm Mind tank
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_REST},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Flygon ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_FLYGON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Levitate banded pivot
        .moves = {MOVE_EARTHQUAKE, MOVE_OUTRAGE, MOVE_U_TURN, MOVE_STONE_EDGE},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // fast revenge killer
        .moves = {MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_U_TURN, MOVE_FIRE_PUNCH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SOFT_SAND, // Dragon Dance setup sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_FIRE_PUNCH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Cacturne ----
    {
        .species = SPECIES_CACTURNE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance priority breaker
        .moves = {MOVE_SWORDS_DANCE, MOVE_SEED_BOMB, MOVE_SUCKER_PUNCH, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CACTURNE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Spikes + Destiny Bond lead, sash = one-shot guard
        .moves = {MOVE_SPIKES, MOVE_SEED_BOMB, MOVE_SUCKER_PUNCH, MOVE_DESTINY_BOND},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Altaria ---- (Swablu line)
    {
        .species = SPECIES_ALTARIA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ALTARIANITE, // Mega Altaria (Pixilate, Dragon/Fairy) — Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_ROOST},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ALTARIA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Natural Cure defensive pivot (no mega)
        .moves = {MOVE_DRAGON_PULSE, MOVE_ROOST, MOVE_DEFOG, MOVE_HEAL_BELL},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(248, 0, 8, 0, 0, 252),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Zangoose ----
    {
        .species = SPECIES_ZANGOOSE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_TOXIC_ORB, // Toxic Boost Facade breaker
        .moves = {MOVE_FACADE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_QUICK_ATTACK},
        .ability = ABILITY_TOXIC_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZANGOOSE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF},
        .ability = ABILITY_TOXIC_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Seviper ----
    {
        .species = SPECIES_SEVIPER,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // mixed special breaker
        .moves = {MOVE_SLUDGE_WAVE, MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN, MOVE_DARK_PULSE},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Lunatone ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LUNATONE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Levitate Cosmic Power / Trick Room attacker
        .moves = {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_EARTH_POWER, MOVE_ICE_BEAM},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUNATONE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Trick Room setter / Cosmic Power tank
        .moves = {MOVE_TRICK_ROOM, MOVE_COSMIC_POWER, MOVE_STORED_POWER, MOVE_MOONLIGHT},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Solrock ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_SOLROCK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Levitate physical setup attacker
        .moves = {MOVE_ROCK_POLISH, MOVE_STONE_EDGE, MOVE_ZEN_HEADBUTT, MOVE_EARTHQUAKE},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SOLROCK,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // doubles support / Trick Room
        .moves = {MOVE_TRICK_ROOM, MOVE_STEALTH_ROCK, MOVE_HELPING_HAND, MOVE_EXPLOSION},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Whiscash ----
    {
        .species = SPECIES_WHISCASH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Dragon Dance bulky sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_WATERFALL, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_OBLIVIOUS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Crawdaunt ----
    {
        .species = SPECIES_CRAWDAUNT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Adaptability wallbreaker
        .moves = {MOVE_SWORDS_DANCE, MOVE_KNOCK_OFF, MOVE_LIQUIDATION, MOVE_AQUA_JET},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CRAWDAUNT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Adaptability band breaker
        .moves = {MOVE_KNOCK_OFF, MOVE_LIQUIDATION, MOVE_AQUA_JET, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Claydol ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CLAYDOL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Levitate hazard setter / spinner
        .moves = {MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_EARTH_POWER, MOVE_ICE_BEAM},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Cradily ----
    {
        .species = SPECIES_CRADILY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Storm Drain Curse setup tank
        .moves = {MOVE_CURSE, MOVE_SEED_BOMB, MOVE_STONE_EDGE, MOVE_RECOVER},
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Armaldo ----
    {
        .species = SPECIES_ARMALDO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance physical attacker
        .moves = {MOVE_SWORDS_DANCE, MOVE_STONE_EDGE, MOVE_X_SCISSOR, MOVE_AQUA_TAIL},
        .ability = ABILITY_BATTLE_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Milotic ----
    {
        .species = SPECIES_MILOTIC,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Marvel Scale defensive pivot
        .moves = {MOVE_SCALD, MOVE_RECOVER, MOVE_ICE_BEAM, MOVE_HAZE},
        .ability = ABILITY_MARVEL_SCALE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(248, 0, 252, 0, 0, 8),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MILOTIC,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Marvel Scale flame-orb tank
        .moves = {MOVE_SCALD, MOVE_RECOVER, MOVE_ICE_BEAM, MOVE_FLIP_TURN},
        .ability = ABILITY_MARVEL_SCALE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(248, 0, 252, 0, 0, 8),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Castform ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CASTFORM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Forecast Weather Ball attacker
        .moves = {MOVE_WEATHER_BALL, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_SUNNY_DAY},
        .ability = ABILITY_FORECAST,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Kecleon ----
    {
        .species = SPECIES_KECLEON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // Color Change special tank
        .moves = {MOVE_KNOCK_OFF, MOVE_SUCKER_PUNCH, MOVE_DRAIN_PUNCH, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_PROTEAN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Banette ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_BANETTE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BANETTITE, // Mega Banette (Prankster) — fast disruptor
        .moves = {MOVE_SHADOW_CLAW, MOVE_KNOCK_OFF, MOVE_WILL_O_WISP, MOVE_DESTINY_BOND},
        .ability = ABILITY_FRISK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BANETTE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Destiny Bond trapper (no mega)
        .moves = {MOVE_SHADOW_CLAW, MOVE_SUCKER_PUNCH, MOVE_DESTINY_BOND, MOVE_TAUNT},
        .ability = ABILITY_INSOMNIA,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Dusclops ---- (Eviolite NFE wall — INNATE LEVITATE; Dusknoir intentionally omitted)
    {
        .species = SPECIES_DUSCLOPS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_EVIOLITE, // Eviolite physical/special wall
        .moves = {MOVE_WILL_O_WISP, MOVE_NIGHT_SHADE, MOVE_PAIN_SPLIT, MOVE_HEX},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 168, 0, 0, 88),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Tropius ----
    {
        .species = SPECIES_TROPIUS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Harvest Sitrus staller
        .moves = {MOVE_LEECH_SEED, MOVE_SUBSTITUTE, MOVE_AIR_SLASH, MOVE_GIGA_DRAIN},
        .ability = ABILITY_HARVEST,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Chimecho ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_CHIMECHO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Levitate Calm Mind tank
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_RECOVER},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Absol ----
    {
        .species = SPECIES_ABSOL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ABSOLITE, // Mega Absol (Magic Bounce) — Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_KNOCK_OFF, MOVE_PLAY_ROUGH, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SUPER_LUCK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ABSOL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Super Luck band breaker (no mega)
        .moves = {MOVE_KNOCK_OFF, MOVE_SUCKER_PUNCH, MOVE_PLAY_ROUGH, MOVE_PSYCHO_CUT},
        .ability = ABILITY_SUPER_LUCK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ABSOL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SCOPE_LENS, // Super Luck guaranteed-crit attacker
        .moves = {MOVE_NIGHT_SLASH, MOVE_PSYCHO_CUT, MOVE_SUCKER_PUNCH, MOVE_SWORDS_DANCE},
        .ability = ABILITY_SUPER_LUCK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Glalie ---- (Gen III mega OK — INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_GLALIE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GLALITITE, // Mega Glalie (Refrigerate) — Return/Explosion nuke
        .moves = {MOVE_RETURN, MOVE_ICICLE_CRASH, MOVE_EARTHQUAKE, MOVE_FREEZE_DRY},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GLALIE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Spikes + Explosion lead (no mega)
        .moves = {MOVE_SPIKES, MOVE_ICE_BEAM, MOVE_FREEZE_DRY, MOVE_EXPLOSION},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Walrein ----
    {
        .species = SPECIES_WALREIN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Thick Fat defensive staller
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_PROTECT},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WALREIN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Thick Fat special tank
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_FREEZE_DRY, MOVE_BODY_SLAM},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Huntail ---- (Clamperl line)
    {
        .species = SPECIES_HUNTAIL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash physical sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_WATERFALL, MOVE_CRUNCH, MOVE_ICE_FANG},
        .ability = ABILITY_WATER_VEIL,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Gorebyss ---- (Clamperl line)
    {
        .species = SPECIES_GOREBYSS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash special sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_PSYCHIC},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Relicanth ----
    {
        .species = SPECIES_RELICANTH,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Rock Head Rock Polish setup tank
        .moves = {MOVE_ROCK_POLISH, MOVE_HEAD_SMASH, MOVE_WATERFALL, MOVE_EARTHQUAKE},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Salamence ---- (Bagon line)
    {
        .species = SPECIES_SALAMENCE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SALAMENCITE, // Mega Salamence (Aerilate) — Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_ROOST},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SALAMENCE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Intimidate revenge killer (no mega)
        .moves = {MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_FIRE_BLAST},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SALAMENCE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed Draco breaker (no mega)
        .moves = {MOVE_DRACO_METEOR, MOVE_FIRE_BLAST, MOVE_EARTHQUAKE, MOVE_ROOST},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Metagross ---- (Beldum line — INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_METAGROSS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_METAGROSSITE, // Mega Metagross (Tough Claws) — fast physical breaker
        .moves = {MOVE_METEOR_MASH, MOVE_ZEN_HEADBUTT, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_METAGROSS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Clear Body band breaker (no mega)
        .moves = {MOVE_METEOR_MASH, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE, MOVE_EXPLOSION},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_METAGROSS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Agility setup / utility tank
        .moves = {MOVE_AGILITY, MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Regirock ----
    {
        .species = SPECIES_REGIROCK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Clear Body Curse setup wall
        .moves = {MOVE_CURSE, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_REST},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 96, 160, 0, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REGIROCK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Sturdy bait setup tank
        .moves = {MOVE_ROCK_POLISH, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_HAMMER_ARM},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Regice ----
    {
        .species = SPECIES_REGICE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Clear Body special wall
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_REST},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REGICE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_FLASH_CANNON},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Registeel ----
    {
        .species = SPECIES_REGISTEEL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Clear Body Curse setup wall
        .moves = {MOVE_CURSE, MOVE_IRON_HEAD, MOVE_BODY_PRESS, MOVE_REST},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REGISTEEL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // defensive setter / pivot
        .moves = {MOVE_STEALTH_ROCK, MOVE_IRON_HEAD, MOVE_THUNDER_WAVE, MOVE_BODY_PRESS},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Latias ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LATIAS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LATIASITE, // Mega Latias — bulky Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_DRAGON_PULSE, MOVE_ROOST},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 132, 0, 124),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LATIAS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive Levitate pivot (no mega)
        .moves = {MOVE_DRAGON_PULSE, MOVE_ROOST, MOVE_DEFOG, MOVE_HEALING_WISH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Latios ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_LATIOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LATIOSITE, // Mega Latios — fast special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_DRACO_METEOR, MOVE_ROOST},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LATIOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Levitate special breaker (no mega)
        .moves = {MOVE_DRACO_METEOR, MOVE_PSYCHIC, MOVE_AURA_SPHERE, MOVE_TRICK},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Kyogre ----
    {
        .species = SPECIES_KYOGRE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLUE_ORB, // Primal Kyogre (Primordial Sea) — rain nuke
        .moves = {MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_THUNDER},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KYOGRE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Drizzle revenge killer (no primal)
        .moves = {MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_THUNDER},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Groudon ----
    {
        .species = SPECIES_GROUDON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_RED_ORB, // Primal Groudon (Desolate Land) — sun breaker
        .moves = {MOVE_PRECIPICE_BLADES, MOVE_FIRE_PUNCH, MOVE_STONE_EDGE, MOVE_SWORDS_DANCE},
        .ability = ABILITY_DROUGHT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GROUDON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Drought bulky setter (no primal)
        .moves = {MOVE_STEALTH_ROCK, MOVE_PRECIPICE_BLADES, MOVE_LAVA_PLUME, MOVE_ROAR},
        .ability = ABILITY_DROUGHT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Rayquaza ---- (Mega via Dragon Ascent + no item)
    {
        .species = SPECIES_RAYQUAZA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_NONE, // Mega Rayquaza (Delta Stream, via Dragon Ascent) — Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_DRAGON_ASCENT, MOVE_EARTHQUAKE, MOVE_EXTREME_SPEED},
        .ability = ABILITY_AIR_LOCK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAYQUAZA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed Draco breaker (no Dragon Ascent / no mega)
        .moves = {MOVE_DRACO_METEOR, MOVE_FIRE_BLAST, MOVE_EARTHQUAKE, MOVE_EXTREME_SPEED},
        .ability = ABILITY_AIR_LOCK,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Jirachi ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_JIRACHI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Serene Grace flinch revenge killer
        .moves = {MOVE_IRON_HEAD, MOVE_ZEN_HEADBUTT, MOVE_ICE_PUNCH, MOVE_U_TURN},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JIRACHI,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Serene Grace Wish support / Iron Head flinch
        .moves = {MOVE_WISH, MOVE_IRON_HEAD, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JIRACHI,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Calm Mind special setup sweeper
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_FLASH_CANNON, MOVE_THUNDERBOLT},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Deoxys-Attack ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_ATTACK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // glass-cannon mixed attacker
        .moves = {MOVE_PSYCHO_BOOST, MOVE_ICE_BEAM, MOVE_SUPERPOWER, MOVE_KNOCK_OFF},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 128, 0, 252, 128, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DEOXYS_ATTACK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // sash glass cannon, one-shot entry guard
        .moves = {MOVE_PSYCHO_BOOST, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_SUPERPOWER},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Deoxys-Speed ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_SPEED,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // hyper-offense hazard lead, one-shot entry guard
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_TAUNT, MOVE_PSYCHO_BOOST},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DEOXYS_SPEED,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHO_BOOST, MOVE_ICE_BEAM, MOVE_FOCUS_BLAST},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Deoxys-Defense ---- (INNATE LEVITATE — no Air Balloon)
    {
        .species = SPECIES_DEOXYS_DEFENSE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky hazard / status wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_TOXIC, MOVE_RECOVER},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation IV
    // ============================================================
    // NOTE: Magnezone, Electivire, Magmortar, Rhyperior and Tangrowth are the
    // Gen IV evolutions of Gen I mons and live in the Generation I section above
    // (in place of their pre-evolutions), so they are intentionally not repeated
    // here.

    // ---- Torterra ----
    {
        .species = SPECIES_TORTERRA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky Rock Polish / Shell Smash setup
        .moves = {MOVE_ROCK_POLISH, MOVE_WOOD_HAMMER, MOVE_EARTHQUAKE, MOVE_STONE_EDGE},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(4, 252, 0, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TORTERRA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY, // bulky hazards + Synthesis
        .moves = {MOVE_STEALTH_ROCK, MOVE_WOOD_HAMMER, MOVE_EARTHQUAKE, MOVE_SYNTHESIS},
        .ability = ABILITY_SHELL_ARMOR,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Infernape ----
    {
        .species = SPECIES_INFERNAPE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed wallbreaker
        .moves = {MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT, MOVE_GUNK_SHOT, MOVE_GRASS_KNOT},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INFERNAPE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast suicide lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_FAKE_OUT, MOVE_CLOSE_COMBAT, MOVE_FIRE_BLAST},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INFERNAPE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_OVERHEAT, MOVE_CLOSE_COMBAT, MOVE_U_TURN, MOVE_GRASS_KNOT},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Empoleon ----
    {
        .species = SPECIES_EMPOLEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky special pivot
        .moves = {MOVE_SCALD, MOVE_FLASH_CANNON, MOVE_ROOST, MOVE_DEFOG},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EMPOLEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Torrent special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_FLASH_CANNON, MOVE_ICE_BEAM, MOVE_GRASS_KNOT},
        .ability = ABILITY_COMPETITIVE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Staraptor ----
    {
        .species = SPECIES_STARAPTOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Reckless band
        .moves = {MOVE_BRAVE_BIRD, MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_U_TURN},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STARAPTOR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLYINIUM_Z, // Supersonic Skystrike nuke
        .moves = {MOVE_BRAVE_BIRD, MOVE_CLOSE_COMBAT, MOVE_QUICK_ATTACK, MOVE_DOUBLE_EDGE},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Luxray ----
    {
        .species = SPECIES_LUXRAY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Intimidate physical attacker
        .moves = {MOVE_WILD_CHARGE, MOVE_CRUNCH, MOVE_SUPERPOWER, MOVE_ICE_FANG},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUXRAY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Guts band
        .moves = {MOVE_WILD_CHARGE, MOVE_CRUNCH, MOVE_SUPERPOWER, MOVE_VOLT_SWITCH},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Roserade ----
    {
        .species = SPECIES_ROSERADE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // special attacker
        .moves = {MOVE_LEAF_STORM, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER, MOVE_FLAMETHROWER},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ROSERADE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Technician status spreader
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_SPIKES, MOVE_TOXIC_SPIKES},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Rampardos ----
    {
        .species = SPECIES_RAMPARDOS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Mold Breaker glass cannon
        .moves = {MOVE_HEAD_SMASH, MOVE_EARTHQUAKE, MOVE_CLOSE_COMBAT, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAMPARDOS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCK_GEM, // Rock Head nuke (no recoil Head Smash)
        .moves = {MOVE_SWORDS_DANCE, MOVE_HEAD_SMASH, MOVE_EARTHQUAKE, MOVE_FIRE_PUNCH},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Bastiodon ----
    {
        .species = SPECIES_BASTIODON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // physically defensive wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_IRON_HEAD, MOVE_ROAR, MOVE_TOXIC},
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Vespiquen ----
    {
        .species = SPECIES_VESPIQUEN,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Pressure stall / defensive pivot
        .moves = {MOVE_ROOST, MOVE_DEFOG, MOVE_TOXIC, MOVE_AIR_SLASH},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(248, 0, 8, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Floatzel ----
    {
        .species = SPECIES_FLOATZEL,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim / fast attacker
        .moves = {MOVE_LIQUIDATION, MOVE_ICE_PUNCH, MOVE_AQUA_JET, MOVE_LOW_KICK},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLOATZEL,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // Water Veil band breaker
        .moves = {MOVE_WAVE_CRASH, MOVE_ICE_PUNCH, MOVE_AQUA_JET, MOVE_FLIP_TURN},
        .ability = ABILITY_WATER_VEIL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Gastrodon ----
    {
        .species = SPECIES_GASTRODON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Storm Drain special tank
        .moves = {MOVE_EARTH_POWER, MOVE_SCALD, MOVE_ICE_BEAM, MOVE_RECOVER},
        .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GASTRODON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // Sticky Hold special tank
        .moves = {MOVE_EARTH_POWER, MOVE_SCALD, MOVE_ICE_BEAM, MOVE_CLEAR_SMOG},
        .ability = ABILITY_STICKY_HOLD,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Ambipom ----
    {
        .species = SPECIES_AMBIPOM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_SILK_SCARF, // Technician Fake Out pivot
        .moves = {MOVE_FAKE_OUT, MOVE_DOUBLE_HIT, MOVE_KNOCK_OFF, MOVE_U_TURN},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AMBIPOM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_DOUBLE_HIT, MOVE_KNOCK_OFF, MOVE_LOW_KICK, MOVE_TRIPLE_AXEL},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Drifblim ----
    {
        .species = SPECIES_DRIFBLIM,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SITRUS_BERRY, // Unburden sweeper
        .moves = {MOVE_CALM_MIND, MOVE_SHADOW_BALL, MOVE_AIR_SLASH, MOVE_STRENGTH_SAP},
        .ability = ABILITY_UNBURDEN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRIFBLIM,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Flare Boost special attacker
        .moves = {MOVE_SHADOW_BALL, MOVE_AIR_SLASH, MOVE_HEX, MOVE_WILL_O_WISP},
        .ability = ABILITY_FLARE_BOOST,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Lopunny ----
    {
        .species = SPECIES_LOPUNNY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LOPUNNITE, // Mega Lopunny (Scrappy)
        .moves = {MOVE_FAKE_OUT, MOVE_HIGH_JUMP_KICK, MOVE_RETURN, MOVE_ICE_PUNCH},
        .ability = ABILITY_LIMBER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LOPUNNY,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_TOXIC_ORB, // Klutz Switcheroo / status spreader
        .moves = {MOVE_FAKE_OUT, MOVE_SWITCHEROO, MOVE_RETURN, MOVE_HIGH_JUMP_KICK},
        .ability = ABILITY_KLUTZ,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Mismagius (Gen IV evolution of Misdreavus; innate Levitate) ----
    {
        .species = SPECIES_MISMAGIUS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot special sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_SHADOW_BALL, MOVE_DAZZLING_GLEAM, MOVE_MYSTICAL_FIRE},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MISMAGIUS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_COLBUR_BERRY, // utility / Taunt + Will-O-Wisp
        .moves = {MOVE_SHADOW_BALL, MOVE_WILL_O_WISP, MOVE_TAUNT, MOVE_PAIN_SPLIT},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Honchkrow (Gen IV evolution of Murkrow) ----
    {
        .species = SPECIES_HONCHKROW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Moxie sweeper
        .moves = {MOVE_SUCKER_PUNCH, MOVE_BRAVE_BIRD, MOVE_HEAT_WAVE, MOVE_SUPERPOWER},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HONCHKROW,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // Super Luck band (guaranteed crits on crit moves)
        .moves = {MOVE_BRAVE_BIRD, MOVE_SUCKER_PUNCH, MOVE_NIGHT_SLASH, MOVE_PSYCHO_CUT},
        .ability = ABILITY_SUPER_LUCK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Skuntank ----
    {
        .species = SPECIES_SKUNTANK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLACK_GLASSES, // Aftermath pivot
        .moves = {MOVE_GUNK_SHOT, MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_PURSUIT},
        .ability = ABILITY_AFTERMATH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Bronzong (innate Levitate) ----
    {
        .species = SPECIES_BRONZONG,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // dual-screen / Trick Room setter
        .moves = {MOVE_TRICK_ROOM, MOVE_STEALTH_ROCK, MOVE_GYRO_BALL, MOVE_BODY_PRESS},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room / Gyro Ball
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRONZONG,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY, // Heatproof screens wall
        .moves = {MOVE_LIGHT_SCREEN, MOVE_REFLECT, MOVE_PSYCHIC, MOVE_TOXIC},
        .ability = ABILITY_HEATPROOF,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Spiritomb (innate Levitate) ----
    {
        .species = SPECIES_SPIRITOMB,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Infiltrator Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_DARK_PULSE, MOVE_SHADOW_BALL, MOVE_REST},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SPIRITOMB,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_BLACK_GLASSES, // Pressure / Pursuit trapper
        .moves = {MOVE_SUCKER_PUNCH, MOVE_FOUL_PLAY, MOVE_WILL_O_WISP, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Garchomp ----
    {
        .species = SPECIES_GARCHOMP,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GARCHOMPITE, // Mega Garchomp (Sand Force)
        .moves = {MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_FIRE_BLAST, MOVE_STONE_EDGE},
        .ability = ABILITY_ROUGH_SKIN,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARCHOMP,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_OUTRAGE, MOVE_FIRE_FANG},
        .ability = ABILITY_ROUGH_SKIN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARCHOMP,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Rough Skin hazards lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_DRAGON_TAIL, MOVE_SPIKES},
        .ability = ABILITY_ROUGH_SKIN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Lucario ----
    {
        .species = SPECIES_LUCARIO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LUCARIONITE, // Mega Lucario (Adaptability)
        .moves = {MOVE_SWORDS_DANCE, MOVE_METEOR_MASH, MOVE_CLOSE_COMBAT, MOVE_BULLET_PUNCH},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUCARIO,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot special
        .moves = {MOVE_NASTY_PLOT, MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_VACUUM_WAVE},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUCARIO,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FIGHTINIUM_Z, // SD + All-Out Pummeling
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_BULLET_PUNCH, MOVE_EXTREME_SPEED},
        .ability = ABILITY_STEADFAST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Hippowdon ----
    {
        .species = SPECIES_HIPPOWDON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Sand Stream physical wall
        .moves = {MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK, MOVE_SLACK_OFF, MOVE_WHIRLWIND},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HIPPOWDON,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_SMOOTH_ROCK, // sand setter + Slack Off
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_SLACK_OFF, MOVE_ICE_FANG},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Drapion ----
    {
        .species = SPECIES_DRAPION,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Sniper / Knock Off pivot
        .moves = {MOVE_KNOCK_OFF, MOVE_POISON_JAB, MOVE_EARTHQUAKE, MOVE_TAUNT},
        .ability = ABILITY_BATTLE_ARMOR,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAPION,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_KNOCK_OFF, MOVE_POISON_JAB, MOVE_AQUA_TAIL},
        .ability = ABILITY_SNIPER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Toxicroak ----
    {
        .species = SPECIES_TOXICROAK,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dry Skin rain sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_GUNK_SHOT, MOVE_DRAIN_PUNCH, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOXICROAK,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Poison Touch bulk
        .moves = {MOVE_GUNK_SHOT, MOVE_DRAIN_PUNCH, MOVE_SUCKER_PUNCH, MOVE_KNOCK_OFF},
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Carnivine (innate Levitate) ----
    {
        .species = SPECIES_CARNIVINE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance grass attacker
        .moves = {MOVE_SWORDS_DANCE, MOVE_POWER_WHIP, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Abomasnow ----
    {
        .species = SPECIES_ABOMASNOW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_ABOMASITE, // Mega Abomasnow (Snow Warning)
        .moves = {MOVE_BLIZZARD, MOVE_GIGA_DRAIN, MOVE_EARTHQUAKE, MOVE_ICE_SHARD},
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 0, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ABOMASNOW,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_ICIUM_Z, // Aurora Veil + Blizzard support
        .moves = {MOVE_AURORA_VEIL, MOVE_BLIZZARD, MOVE_GIGA_DRAIN, MOVE_PROTECT},
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Weavile (Gen IV evolution of Sneasel) ----
    {
        .species = SPECIES_WEAVILE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast physical attacker
        .moves = {MOVE_TRIPLE_AXEL, MOVE_KNOCK_OFF, MOVE_ICE_SHARD, MOVE_LOW_KICK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WEAVILE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Swords Dance + Pickpocket
        .moves = {MOVE_SWORDS_DANCE, MOVE_ICICLE_CRASH, MOVE_KNOCK_OFF, MOVE_ICE_SHARD},
        .ability = ABILITY_PICKPOCKET,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Lickilicky ----
    {
        .species = SPECIES_LICKILICKY,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Own Tempo Trick Room support
        .moves = {MOVE_TRICK_ROOM, MOVE_BODY_SLAM, MOVE_EXPLOSION, MOVE_KNOCK_OFF},
        .ability = ABILITY_OWN_TEMPO,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Togekiss ----
    {
        .species = SPECIES_TOGEKISS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Serene Grace flinch / Nasty Plot
        .moves = {MOVE_NASTY_PLOT, MOVE_AIR_SLASH, MOVE_DAZZLING_GLEAM, MOVE_ROOST},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 80, 176),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOGEKISS,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // doubles support (Follow Me + Tailwind)
        .moves = {MOVE_FOLLOW_ME, MOVE_TAILWIND, MOVE_DAZZLING_GLEAM, MOVE_AIR_SLASH},
        .ability = ABILITY_SUPER_LUCK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Yanmega (Gen IV evolution of Yanma) ----
    {
        .species = SPECIES_YANMEGA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Tinted Lens breaker
        .moves = {MOVE_BUG_BUZZ, MOVE_AIR_SLASH, MOVE_GIGA_DRAIN, MOVE_PROTECT},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_YANMEGA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Speed Boost sweeper
        .moves = {MOVE_BUG_BUZZ, MOVE_AIR_SLASH, MOVE_ANCIENT_POWER, MOVE_GIGA_DRAIN},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Leafeon ----
    {
        .species = SPECIES_LEAFEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll / Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_KNOCK_OFF, MOVE_X_SCISSOR},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Glaceon ----
    {
        .species = SPECIES_GLACEON,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_BLIZZARD, MOVE_FREEZE_DRY, MOVE_WATER_PULSE, MOVE_SHADOW_BALL},
        .ability = ABILITY_SNOW_CLOAK,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Gliscor (Gen IV evolution of Gligar) ----
    {
        .species = SPECIES_GLISCOR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal stall
        .moves = {MOVE_EARTHQUAKE, MOVE_PROTECT, MOVE_TOXIC, MOVE_ROOST},
        .ability = ABILITY_POISON_HEAL,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(244, 0, 248, 16, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GLISCOR,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_TOXIC_ORB, // Poison Heal Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_KNOCK_OFF, MOVE_ROOST},
        .ability = ABILITY_POISON_HEAL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(244, 252, 0, 12, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Mamoswine (Gen IV evolution of Piloswine) ----
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Thick Fat physical attacker
        .moves = {MOVE_EARTHQUAKE, MOVE_ICICLE_CRASH, MOVE_ICE_SHARD, MOVE_KNOCK_OFF},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_EARTHQUAKE, MOVE_ICICLE_CRASH, MOVE_ICE_SHARD, MOVE_SUPERPOWER},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Porygon-Z (innate Levitate) ----
    {
        .species = SPECIES_PORYGON_Z,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Adaptability Nasty Plot nuke
        .moves = {MOVE_NASTY_PLOT, MOVE_TRI_ATTACK, MOVE_DARK_PULSE, MOVE_THUNDERBOLT},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PORYGON_Z,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Download breaker
        .moves = {MOVE_TRI_ATTACK, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_TRICK},
        .ability = ABILITY_DOWNLOAD,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Gallade (Gen IV evolution of Kirlia) ----
    {
        .species = SPECIES_GALLADE,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GALLADITE, // Mega Gallade (Inner Focus)
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_PSYCHO_CUT, MOVE_KNOCK_OFF},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GALLADE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Sharpness slicer
        .moves = {MOVE_SWORDS_DANCE, MOVE_SACRED_SWORD, MOVE_PSYCHO_CUT, MOVE_LEAF_BLADE},
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Probopass ----
    {
        .species = SPECIES_PROBOPASS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Magnet Pull steel trapper / hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_POWER_GEM, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH},
        .ability = ABILITY_MAGNET_PULL,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Dusknoir (Gen IV evolution of Dusclops) ----
    {
        .species = SPECIES_DUSKNOIR,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky utility / Pain Split
        .moves = {MOVE_POLTERGEIST, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DUSKNOIR,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_POLTERGEIST, MOVE_EARTHQUAKE, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_FRISK,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Froslass (innate Levitate) ----
    {
        .species = SPECIES_FROSLASS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast spikes / Destiny Bond lead
        .moves = {MOVE_SPIKES, MOVE_ICE_BEAM, MOVE_SHADOW_BALL, MOVE_DESTINY_BOND},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FROSLASS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Snow Cloak offensive
        .moves = {MOVE_ICE_BEAM, MOVE_SHADOW_BALL, MOVE_THUNDERBOLT, MOVE_TAUNT},
        .ability = ABILITY_SNOW_CLOAK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Rotom-Heat (innate Levitate) ----
    {
        .species = SPECIES_ROTOM_HEAT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // fire/electric breaker
        .moves = {MOVE_OVERHEAT, MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_TRICK},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ROTOM_HEAT,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
        .moves = {MOVE_OVERHEAT, MOVE_VOLT_SWITCH, MOVE_WILL_O_WISP, MOVE_NASTY_PLOT},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(248, 0, 0, 16, 0, 244),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Rotom-Wash (innate Levitate) ----
    {
        .species = SPECIES_ROTOM_WASH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky water/electric pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_VOLT_SWITCH, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(248, 0, 168, 0, 0, 92),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ROTOM_WASH,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_TRICK},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Rotom-Mow (innate Levitate) ----
    {
        .species = SPECIES_ROTOM_MOW,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // grass/electric attacker
        .moves = {MOVE_LEAF_STORM, MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Uxie (innate Levitate) ----
    {
        .species = SPECIES_UXIE,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // dual-screen / hazards wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_PSYCHIC, MOVE_YAWN, MOVE_U_TURN},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Mesprit (innate Levitate) ----
    {
        .species = SPECIES_MESPRIT,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed pivot
        .moves = {MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_U_TURN, MOVE_STEALTH_ROCK},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Azelf (innate Levitate) ----
    {
        .species = SPECIES_AZELF,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast suicide lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_TAUNT, MOVE_PSYCHIC, MOVE_EXPLOSION},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AZELF,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHIC, MOVE_FIRE_BLAST, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Dialga ----
    {
        .species = SPECIES_DIALGA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // bulky special legend
        .moves = {MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_THUNDERBOLT, MOVE_STEALTH_ROCK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DIALGA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Roar of Time breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_FIRE_BLAST, MOVE_ROAR_OF_TIME},
        .ability = ABILITY_TELEPATHY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Palkia ----
    {
        .species = SPECIES_PALKIA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast special legend
        .moves = {MOVE_SPACIAL_REND, MOVE_HYDRO_PUMP, MOVE_FIRE_BLAST, MOVE_THUNDERBOLT},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PALKIA,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge legend
        .moves = {MOVE_SPACIAL_REND, MOVE_HYDRO_PUMP, MOVE_DRACO_METEOR, MOVE_FIRE_BLAST},
        .ability = ABILITY_TELEPATHY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Heatran ----
    {
        .species = SPECIES_HEATRAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Flash Fire pivot / hazards
        .moves = {MOVE_MAGMA_STORM, MOVE_EARTH_POWER, MOVE_STEALTH_ROCK, MOVE_TAUNT},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 100, 0, 156),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HEATRAN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_MAGMA_STORM, MOVE_EARTH_POWER, MOVE_FLASH_CANNON, MOVE_DRAGON_PULSE},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Regigigas ----
    {
        .species = SPECIES_REGIGIGAS,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_FLAME_ORB, // Slow Start sit-out with Substitute
        .moves = {MOVE_SUBSTITUTE, MOVE_BODY_SLAM, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_SLOW_START,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Giratina (Altered, innate Levitate) ----
    {
        .species = SPECIES_GIRATINA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Will-O / Defog wall
        .moves = {MOVE_DRAGON_TAIL, MOVE_WILL_O_WISP, MOVE_REST, MOVE_DEFOG},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Giratina-Origin (innate Levitate) ----
    {
        .species = SPECIES_GIRATINA_ORIGIN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_GRISEOUS_ORB, // Origin forme nuke
        .moves = {MOVE_SHADOW_FORCE, MOVE_DRACO_METEOR, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Cresselia (innate Levitate) ----
    {
        .species = SPECIES_CRESSELIA,
        .tags = FACTORY_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind bulky sweeper
        .moves = {MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_PSYSHOCK, MOVE_MOONLIGHT},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CRESSELIA,
        .tags = FACTORY_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // doubles support
        .moves = {MOVE_TRICK_ROOM, MOVE_HELPING_HAND, MOVE_ICY_WIND, MOVE_MOONBLAST},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Darkrai (innate Levitate) ----
    {
        .species = SPECIES_DARKRAI,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot / Dark Void sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_DARK_PULSE, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
        .ability = ABILITY_BAD_DREAMS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Shaymin (Land) ----
    {
        .species = SPECIES_SHAYMIN,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Natural Cure Seed Flare / Synthesis
        .moves = {MOVE_SEED_FLARE, MOVE_EARTH_POWER, MOVE_AIR_SLASH, MOVE_SYNTHESIS},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Shaymin-Sky ----
    {
        .species = SPECIES_SHAYMIN_SKY,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Serene Grace Air Slash flincher
        .moves = {MOVE_SEED_FLARE, MOVE_AIR_SLASH, MOVE_EARTH_POWER, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Arceus ----
    {
        .species = SPECIES_ARCEUS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Extreme Speed Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EXTREME_SPEED, MOVE_EARTHQUAKE, MOVE_SHADOW_CLAW},
        .ability = ABILITY_MULTITYPE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCEUS,
        .tags = FACTORY_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_JUDGMENT, MOVE_ICE_BEAM, MOVE_RECOVER},
        .ability = ABILITY_MULTITYPE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
};

const u16 gFactoryCompetitiveMonsCount = ARRAY_COUNT(gFactoryCompetitiveMons);
