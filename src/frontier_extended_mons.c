#include "global.h"
#include "frontier_extended_mons.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// FORK: fork-owned Battle Factory roster overhaul (B_FRONTIER_EXTENDED_MONS).
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
//  - .tags  is REQUIRED here: FORMAT_SINGLES / FORMAT_DOUBLES / FORMAT_BOTH
//          marks which battle format(s) the set is suited for, so a singles-only
//          set never shows up in a doubles challenge (and vice versa). A set that
//          works in either mode is FORMAT_BOTH. (See frontier_extended_mons.h.)
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
// STATUS: Generations I-IX are all built out (~2-4 builds per reasonable species,
// mega & non-mega, with a deliberate mix of offensive and defensive/support sets).
// B_FRONTIER_EXTENDED_MONS is TRUE — the list is large enough to draft a 6-mon
// team (plus distinct opponents) in both singles and doubles from any subset.
// ORDER: entries are sorted by National Pokédex number (see the Generation
// banners, which delimit the dex ranges). All builds for one species are kept
// contiguous, and alternate formes (megas share the base entry; Rotom/Therian/
// Hisuian/Paradox/etc. are their own entries) sort with their base species' dex
// number. Cross-gen evolutions therefore live at their own dex slot — e.g.
// Magnezone/Electivire/Magmortar/Rhyperior/Tangrowth sit in the Generation IV
// range (#462/466/467/464/465), NOT next to their Gen I pre-evolutions. Insert a
// new species at its correct dex position. NOTE: saved rentals reference entries
// by array INDEX, so any mid-list insertion/removal invalidates an in-progress
// rented team in an existing save (appending past the end is the only save-safe
// edit) — acceptable while iterating, but be aware when editing a shipped save.

const struct TrainerMon gFrontierExtendedMons[] =
{

    // ============================================================
    //                       Generation I
    // ============================================================

    // ---- Venusaur ----
    {
        .species = SPECIES_VENUSAUR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // no-mega hurricane spam
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_U_TURN, MOVE_HYPER_VOICE},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Raichu ----
    {
        .species = SPECIES_RAICHU,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // doubles Intimidate support
        .moves = {MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_SNARL, MOVE_PROTECT},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Arcanine-Hisui ----
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Rock Head Head Smash breaker
        .moves = {MOVE_HEAD_SMASH, MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCANINE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Intimidate bulky pivot
        .moves = {MOVE_FLARE_BLITZ, MOVE_ROCK_SLIDE, MOVE_EXTREME_SPEED, MOVE_MORNING_SUN},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Poliwrath ----
    {
        .species = SPECIES_POLIWRATH,
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker (0 Spe IV)
        .moves = {MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_SCALD, MOVE_SLACK_OFF},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Dodrio ----
    {
        .species = SPECIES_DODRIO,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST},
        .ability = ABILITY_INSOMNIA,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Exeggutor ----
    {
        .species = SPECIES_EXEGGUTOR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_LEAF_STORM, MOVE_PSYCHIC, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_HARVEST,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Marowak ----
    {
        .species = SPECIES_MAROWAK,
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FIRE_BLAST, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Chansey ----
    {
        .species = SPECIES_CHANSEY,
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LUCKY_PUNCH, // Chansey-only crit item; guaranteed first crit
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Kangaskhan ----
    {
        .species = SPECIES_KANGASKHAN,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SILK_SCARF, // no-mega Scrappy attacker
        .moves = {MOVE_FAKE_OUT, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Starmie ----
    {
        .species = SPECIES_STARMIE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ICIUM_Z, // Subzero Slammer
        .moves = {MOVE_NASTY_PLOT, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_FOCUS_BLAST},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Pinsir ----
    {
        .species = SPECIES_PINSIR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAT_ROCK, // sun setter for the team
        .moves = {MOVE_SUNNY_DAY, MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_EARTH_POWER},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Typhlosion-Hisui ----
    {
        .species = SPECIES_TYPHLOSION_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Frisk Ghost/Fire breaker
        .moves = {MOVE_SHADOW_BALL, MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_INFERNAL_PARADE},
        .ability = ABILITY_FRISK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYPHLOSION_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Nasty Plot Hex sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_HEX, MOVE_FIRE_BLAST, MOVE_WILL_O_WISP},
        .ability = ABILITY_FRISK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Feraligatr ----
    {
        .species = SPECIES_FERALIGATR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // stallbreaker support with hazards
        .moves = {MOVE_SEISMIC_TOSS, MOVE_SOFT_BOILED, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(4, 0, 252, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Raikou ----
    {
        .species = SPECIES_RAIKOU,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // bulky defogger / wall
        .moves = {MOVE_SCALD, MOVE_DEFOG, MOVE_REST, MOVE_SLEEP_TALK},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Tyranitar ----
    {
        .species = SPECIES_TYRANITAR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank in sand
        .moves = {MOVE_STONE_EDGE, MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_EARTHQUAKE},
        .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Lugia ----
    {
        .species = SPECIES_LUGIA,
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot / cleric
        .moves = {MOVE_SACRED_FIRE, MOVE_BRAVE_BIRD, MOVE_RECOVER, MOVE_WHIRLWIND},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation III
    // ============================================================

    // ---- Sceptile ----
    {
        .species = SPECIES_SCEPTILE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SOFT_SAND, // Dragon Dance setup sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW, MOVE_FIRE_PUNCH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLYGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // defensive Defog pivot (innate/native Levitate)
        .moves = {MOVE_DEFOG, MOVE_EARTHQUAKE, MOVE_U_TURN, MOVE_TOXIC},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 196, 60, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Cacturne ----
    {
        .species = SPECIES_CACTURNE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance physical attacker
        .moves = {MOVE_SWORDS_DANCE, MOVE_STONE_EDGE, MOVE_X_SCISSOR, MOVE_AQUA_TAIL},
        .ability = ABILITY_BATTLE_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARMALDO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // defensive Rapid Spin + Stealth Rock
        .moves = {MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE},
        .ability = ABILITY_BATTLE_ARMOR,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Milotic ----
    {
        .species = SPECIES_MILOTIC,
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Rock Head Rock Polish setup tank
        .moves = {MOVE_ROCK_POLISH, MOVE_HEAD_SMASH, MOVE_WATERFALL, MOVE_EARTHQUAKE},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RELICANTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy defensive rocker + Yawn
        .moves = {MOVE_STEALTH_ROCK, MOVE_YAWN, MOVE_WATERFALL, MOVE_TOXIC},
        .ability = ABILITY_STURDY,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Salamence ---- (Bagon line)
    {
        .species = SPECIES_SALAMENCE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Drizzle revenge killer (no primal)
        .moves = {MOVE_WATER_SPOUT, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_THUNDER},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(4, 0, 0, 252, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KYOGRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind + Rest (rain, no primal)
        .moves = {MOVE_CALM_MIND, MOVE_ORIGIN_PULSE, MOVE_ICE_BEAM, MOVE_REST},
        .ability = ABILITY_DRIZZLE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Groudon ----
    {
        .species = SPECIES_GROUDON,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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

    // ---- Torterra ----
    {
        .species = SPECIES_TORTERRA,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Swords Dance + Pickpocket
        .moves = {MOVE_SWORDS_DANCE, MOVE_ICICLE_CRASH, MOVE_KNOCK_OFF, MOVE_ICE_SHARD},
        .ability = ABILITY_PICKPOCKET,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Magnezone (Gen IV evolution of Magneton; innate Levitate = Ground-immune) ----
    {
        .species = SPECIES_MAGNEZONE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Analytic special tank
        .moves = {MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Lickilicky ----
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Own Tempo Trick Room support
        .moves = {MOVE_TRICK_ROOM, MOVE_BODY_SLAM, MOVE_EXPLOSION, MOVE_KNOCK_OFF},
        .ability = ABILITY_OWN_TEMPO,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31), // min Speed for Trick Room
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LICKILICKY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Wish + Protect cleric wall
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_BODY_SLAM, MOVE_KNOCK_OFF},
        .ability = ABILITY_CLOUD_NINE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Rhyperior (Gen IV evolution of Rhydon) ----
    {
        .species = SPECIES_RHYPERIOR,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Solid Rock + WP sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_MEGAHORN},
        .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Tangrowth (Gen IV evolution of Tangela) ----
    {
        .species = SPECIES_TANGROWTH,
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Regenerator physical tank
        .moves = {MOVE_POWER_WHIP, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Electivire (Gen IV evolution of Electabuzz) ----
    {
        .species = SPECIES_ELECTIVIRE,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Vital Spirit revenge killer
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
        .ability = ABILITY_VITAL_SPIRIT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Togekiss ----
    {
        .species = SPECIES_TOGEKISS,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_EARTHQUAKE, MOVE_ICICLE_CRASH, MOVE_ICE_SHARD, MOVE_SUPERPOWER},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAMOSWINE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Thick Fat tank rocker
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_ICE_SHARD, MOVE_KNOCK_OFF},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 16, 0, 0, 0, 240),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Porygon-Z (innate Levitate) ----
    {
        .species = SPECIES_PORYGON_Z,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_DOUBLES,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_JUDGMENT, MOVE_ICE_BEAM, MOVE_RECOVER},
        .ability = ABILITY_MULTITYPE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation V
    // ============================================================

    // ---- Serperior ----
    {
        .species = SPECIES_SERPERIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Contrary Leaf Storm sweeper
        .moves = {MOVE_LEAF_STORM, MOVE_DRAGON_PULSE, MOVE_GIGA_DRAIN, MOVE_SUBSTITUTE},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SERPERIOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Sub + Glare pivot
        .moves = {MOVE_LEAF_STORM, MOVE_GLARE, MOVE_LEECH_SEED, MOVE_SUBSTITUTE},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Emboar ----
    {
        .species = SPECIES_EMBOAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Reckless wallbreaker
        .moves = {MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT, MOVE_WILD_CHARGE, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EMBOAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed attacker
        .moves = {MOVE_FLARE_BLITZ, MOVE_CLOSE_COMBAT, MOVE_HEAT_WAVE, MOVE_GRASS_KNOT},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_NAUGHTY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Samurott ----
    {
        .species = SPECIES_SAMUROTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed Swords Dance attacker
        .moves = {MOVE_SWORDS_DANCE, MOVE_LIQUIDATION, MOVE_SACRED_SWORD, MOVE_AQUA_JET},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SAMUROTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FLIP_TURN, MOVE_GRASS_KNOT},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Samurott-Hisui ----
    {
        .species = SPECIES_SAMUROTT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // Sharpness Ceaseless Edge lead
        .moves = {MOVE_CEASELESS_EDGE, MOVE_AQUA_JET, MOVE_SUCKER_PUNCH, MOVE_KNOCK_OFF},
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SAMUROTT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sharpness Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_CEASELESS_EDGE, MOVE_LIQUIDATION, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Stoutland ----
    {
        .species = SPECIES_STOUTLAND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sand Rush band
        .moves = {MOVE_RETURN, MOVE_SUPERPOWER, MOVE_CRUNCH, MOVE_WILD_CHARGE},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STOUTLAND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Intimidate physical wall
        .moves = {MOVE_BODY_SLAM, MOVE_TOXIC, MOVE_ROAR, MOVE_REST},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Musharna (innate Levitate per roster rule) ----
    {
        .species = SPECIES_MUSHARNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_MOONLIGHT, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_SYNCHRONIZE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MUSHARNA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB, // Telepathy Trick Room setter
        .moves = {MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_HELPING_HAND},
        .ability = ABILITY_TELEPATHY,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Excadrill ----
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sand Rush sweeper
        .moves = {MOVE_EARTHQUAKE, MOVE_IRON_HEAD, MOVE_ROCK_SLIDE, MOVE_RAPID_SPIN},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Mold Breaker hazard lead + spinner
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_IRON_HEAD, MOVE_RAPID_SPIN},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EXCADRILL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_EARTHQUAKE, MOVE_IRON_HEAD, MOVE_ROCK_SLIDE, MOVE_HIGH_HORSEPOWER},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Audino ----
    {
        .species = SPECIES_AUDINO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator cleric wall
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_TOXIC, MOVE_HEAL_BELL},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 128, 0, 0, 128),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Conkeldurr ----
    {
        .species = SPECIES_CONKELDURR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts bulk-up tank
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_MACH_PUNCH, MOVE_KNOCK_OFF},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CONKELDURR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Sheer Force mixed tank
        .moves = {MOVE_DRAIN_PUNCH, MOVE_MACH_PUNCH, MOVE_POISON_JAB, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Seismitoad ----
    {
        .species = SPECIES_SEISMITOAD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim rain sweeper
        .moves = {MOVE_HYDRO_PUMP, MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE, MOVE_ICE_BEAM},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SEISMITOAD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky pivot
        .moves = {MOVE_STEALTH_ROCK, MOVE_SCALD, MOVE_EARTH_POWER, MOVE_TOXIC},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Throh ----
    {
        .species = SPECIES_THROH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Guts Bulk Up tank
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_KNOCK_OFF, MOVE_REST},
        .ability = ABILITY_GUTS,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Sawk ----
    {
        .species = SPECIES_SAWK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Mold Breaker revenge killer
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_STONE_EDGE, MOVE_EARTHQUAKE},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SAWK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sturdy breaker band
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_ICE_PUNCH, MOVE_POISON_JAB},
        .ability = ABILITY_STURDY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Leavanny ----
    {
        .species = SPECIES_LEAVANNY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance attacker
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_X_SCISSOR, MOVE_KNOCK_OFF},
        .ability = ABILITY_SWARM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LEAVANNY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Sticky Web lead
        .moves = {MOVE_STICKY_WEB, MOVE_LEAF_BLADE, MOVE_KNOCK_OFF, MOVE_X_SCISSOR},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Scolipede ----
    {
        .species = SPECIES_SCOLIPEDE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Speed Boost sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_MEGAHORN, MOVE_POISON_JAB, MOVE_EARTHQUAKE},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCOLIPEDE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Speed Boost screens / Tailwind support
        .moves = {MOVE_TAILWIND, MOVE_PROTECT, MOVE_POISON_JAB, MOVE_MEGAHORN},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Whimsicott (innate Levitate not applicable; Prankster pivot) ----
    {
        .species = SPECIES_WHIMSICOTT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Prankster utility pivot
        .moves = {MOVE_MOONBLAST, MOVE_LEECH_SEED, MOVE_ENCORE, MOVE_U_TURN},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WHIMSICOTT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Prankster Tailwind + redirect support
        .moves = {MOVE_TAILWIND, MOVE_HELPING_HAND, MOVE_MOONBLAST, MOVE_ENCORE},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Lilligant ----
    {
        .species = SPECIES_LILLIGANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Quiver Dance sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_OWN_TEMPO,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LILLIGANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Chlorophyll sun sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_GIGA_DRAIN, MOVE_HURRICANE, MOVE_SUBSTITUTE},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Lilligant-Hisui ----
    {
        .species = SPECIES_LILLIGANT_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll Victory Dance sweeper
        .moves = {MOVE_VICTORY_DANCE, MOVE_CLOSE_COMBAT, MOVE_LEAF_BLADE, MOVE_TRIPLE_AXEL},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LILLIGANT_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // sash setup sweeper
        .moves = {MOVE_VICTORY_DANCE, MOVE_CLOSE_COMBAT, MOVE_LEAF_BLADE, MOVE_ICE_SPINNER},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Krookodile ----
    {
        .species = SPECIES_KROOKODILE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Moxie band
        .moves = {MOVE_EARTHQUAKE, MOVE_KNOCK_OFF, MOVE_STONE_EDGE, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KROOKODILE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Intimidate bulky hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE, MOVE_TAUNT},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Darmanitan ----
    {
        .species = SPECIES_DARMANITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Sheer Force revenge killer
        .moves = {MOVE_FLARE_BLITZ, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_U_TURN},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DARMANITAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band wallbreaker
        .moves = {MOVE_FLARE_BLITZ, MOVE_EARTHQUAKE, MOVE_SUPERPOWER, MOVE_U_TURN},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Maractus ----
    {
        .species = SPECIES_MARACTUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll sun sweeper
        .moves = {MOVE_GROWTH, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER, MOVE_LEECH_SEED},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Crustle ----
    {
        .species = SPECIES_CRUSTLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Sturdy Shell Smash + hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_SHELL_SMASH, MOVE_X_SCISSOR, MOVE_STONE_EDGE},
        .ability = ABILITY_STURDY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CRUSTLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_STONE_EDGE, MOVE_X_SCISSOR, MOVE_EARTHQUAKE},
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Scrafty ----
    {
        .species = SPECIES_SCRAFTY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Moxie sweeper
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_KNOCK_OFF, MOVE_ICE_PUNCH},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCRAFTY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate Fake Out support tank
        .moves = {MOVE_FAKE_OUT, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH, MOVE_ICE_PUNCH},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Sigilyph ----
    {
        .species = SPECIES_SIGILYPH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Magic Guard Cosmic Power stallbreaker
        .moves = {MOVE_COSMIC_POWER, MOVE_STORED_POWER, MOVE_ROOST, MOVE_PSYCHO_SHIFT},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SIGILYPH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // special attacker
        .moves = {MOVE_AIR_SLASH, MOVE_PSYCHIC, MOVE_HEAT_WAVE, MOVE_ROOST},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Cofagrigus (innate Levitate) ----
    {
        .species = SPECIES_COFAGRIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Mummy bulky special wall
        .moves = {MOVE_SHADOW_BALL, MOVE_WILL_O_WISP, MOVE_TOXIC_SPIKES, MOVE_PAIN_SPLIT},
        .ability = ABILITY_MUMMY,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_COFAGRIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_NASTY_PLOT, MOVE_SHADOW_BALL, MOVE_PSYCHIC},
        .ability = ABILITY_MUMMY,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Carracosta ----
    {
        .species = SPECIES_CARRACOSTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Solid Rock Shell Smash sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_LIQUIDATION, MOVE_STONE_EDGE, MOVE_AQUA_JET},
        .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CARRACOSTA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Sturdy Shell Smash lead
        .moves = {MOVE_SHELL_SMASH, MOVE_LIQUIDATION, MOVE_STONE_EDGE, MOVE_EARTHQUAKE},
        .ability = ABILITY_STURDY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Archeops ----
    {
        .species = SPECIES_ARCHEOPS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // glass cannon (Defeatist drawback)
        .moves = {MOVE_ACROBATICS, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_U_TURN},
        .ability = ABILITY_DEFEATIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCHEOPS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_STONE_EDGE, MOVE_ACROBATICS, MOVE_TAUNT},
        .ability = ABILITY_DEFEATIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Garbodor ----
    {
        .species = SPECIES_GARBODOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Aftermath hazard setter
        .moves = {MOVE_TOXIC_SPIKES, MOVE_SPIKES, MOVE_GUNK_SHOT, MOVE_PAIN_SPLIT},
        .ability = ABILITY_AFTERMATH,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARBODOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Weak Armor attacker
        .moves = {MOVE_GUNK_SHOT, MOVE_SEED_BOMB, MOVE_DRAIN_PUNCH, MOVE_EXPLOSION},
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Zoroark ----
    {
        .species = SPECIES_ZOROARK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Illusion mixed attacker
        .moves = {MOVE_NASTY_PLOT, MOVE_DARK_PULSE, MOVE_FLAMETHROWER, MOVE_FOCUS_BLAST},
        .ability = ABILITY_ILLUSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZOROARK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Illusion special breaker
        .moves = {MOVE_DARK_PULSE, MOVE_FLAMETHROWER, MOVE_FOCUS_BLAST, MOVE_U_TURN},
        .ability = ABILITY_ILLUSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Zoroark-Hisui ----
    {
        .species = SPECIES_ZOROARK_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Illusion Normal/Ghost breaker
        .moves = {MOVE_SHADOW_BALL, MOVE_HYPER_VOICE, MOVE_FLAMETHROWER, MOVE_U_TURN},
        .ability = ABILITY_ILLUSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZOROARK_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_SHADOW_BALL, MOVE_HYPER_VOICE, MOVE_FOCUS_BLAST},
        .ability = ABILITY_ILLUSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Cinccino ----
    {
        .species = SPECIES_CINCCINO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Skill Link multi-hit sweeper
        .moves = {MOVE_TAIL_SLAP, MOVE_BULLET_SEED, MOVE_ROCK_BLAST, MOVE_KNOCK_OFF},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CINCCINO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Skill Link band
        .moves = {MOVE_TAIL_SLAP, MOVE_BULLET_SEED, MOVE_ROCK_BLAST, MOVE_U_TURN},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Gothitelle ----
    {
        .species = SPECIES_GOTHITELLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shadow Tag Calm Mind trapper
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_SHADOW_BALL, MOVE_REST},
        .ability = ABILITY_SHADOW_TAG,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Reuniclus (innate Levitate per roster rule) ----
    {
        .species = SPECIES_REUNICLUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Magic Guard Calm Mind tank
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_FOCUS_BLAST, MOVE_RECOVER},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REUNICLUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST},
        .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Swanna ----
    {
        .species = SPECIES_SWANNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // hydration / offensive pivot
        .moves = {MOVE_HURRICANE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_ROOST},
        .ability = ABILITY_HYDRATION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Vanilluxe (innate Levitate per roster rule) ----
    {
        .species = SPECIES_VANILLUXE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Snow Warning special attacker
        .moves = {MOVE_AUTOTOMIZE, MOVE_BLIZZARD, MOVE_FREEZE_DRY, MOVE_FLASH_CANNON},
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Sawsbuck ----
    {
        .species = SPECIES_SAWSBUCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_HORN_LEECH, MOVE_DOUBLE_EDGE, MOVE_JUMP_KICK},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SAWSBUCK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sap Sipper band
        .moves = {MOVE_HORN_LEECH, MOVE_DOUBLE_EDGE, MOVE_MEGAHORN, MOVE_JUMP_KICK},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Escavalier ----
    {
        .species = SPECIES_ESCAVALIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Swarm band breaker
        .moves = {MOVE_MEGAHORN, MOVE_IRON_HEAD, MOVE_KNOCK_OFF, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_SWARM,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ESCAVALIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Overcoat bulky tank
        .moves = {MOVE_MEGAHORN, MOVE_IRON_HEAD, MOVE_DRAIN_PUNCH, MOVE_SWORDS_DANCE},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Amoonguss ----
    {
        .species = SPECIES_AMOONGUSS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET, // Regenerator Rage Powder redirect
        .moves = {MOVE_RAGE_POWDER, MOVE_SPORE, MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AMOONGUSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Regenerator status wall
        .moves = {MOVE_SPORE, MOVE_GIGA_DRAIN, MOVE_CLEAR_SMOG, MOVE_TOXIC},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Jellicent (innate Levitate per roster rule) ----
    {
        .species = SPECIES_JELLICENT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky spinblocker
        .moves = {MOVE_SCALD, MOVE_WILL_O_WISP, MOVE_RECOVER, MOVE_TOXIC},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_JELLICENT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Cursed Body special wall
        .moves = {MOVE_SHADOW_BALL, MOVE_SCALD, MOVE_RECOVER, MOVE_TAUNT},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Alomomola ----
    {
        .species = SPECIES_ALOMOMOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator Wish wall
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_SCALD, MOVE_TOXIC},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Galvantula ----
    {
        .species = SPECIES_GALVANTULA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Compound Eyes Sticky Web lead
        .moves = {MOVE_STICKY_WEB, MOVE_THUNDER, MOVE_BUG_BUZZ, MOVE_VOLT_SWITCH},
        .ability = ABILITY_COMPOUND_EYES,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GALVANTULA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // special attacker
        .moves = {MOVE_THUNDER, MOVE_BUG_BUZZ, MOVE_ENERGY_BALL, MOVE_VOLT_SWITCH},
        .ability = ABILITY_COMPOUND_EYES,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Ferroseed (NFE Eviolite niche) ----
    {
        .species = SPECIES_FERROSEED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_EVIOLITE, // Iron Barbs hazard wall (Eviolite niche)
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_LEECH_SEED, MOVE_GYRO_BALL},
        .ability = ABILITY_IRON_BARBS,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 88, 0, 0, 168),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Ferrothorn ----
    {
        .species = SPECIES_FERROTHORN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Iron Barbs hazard wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_LEECH_SEED, MOVE_POWER_WHIP},
        .ability = ABILITY_IRON_BARBS,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 88, 0, 0, 168),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FERROTHORN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Iron Barbs + Helmet contact punisher
        .moves = {MOVE_GYRO_BALL, MOVE_POWER_WHIP, MOVE_KNOCK_OFF, MOVE_LEECH_SEED},
        .ability = ABILITY_IRON_BARBS,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Klinklang (innate Levitate per roster rule) ----
    {
        .species = SPECIES_KLINKLANG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Clear Body Shift Gear sweeper
        .moves = {MOVE_SHIFT_GEAR, MOVE_GEAR_GRIND, MOVE_SUBSTITUTE, MOVE_WILD_CHARGE},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Eelektross (native Levitate) ----
    {
        .species = SPECIES_EELEKTROSS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Levitate mixed tank
        .moves = {MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN, MOVE_FLIP_TURN},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EELEKTROSS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Coil physical attacker
        .moves = {MOVE_COIL, MOVE_WILD_CHARGE, MOVE_DRAIN_PUNCH, MOVE_KNOCK_OFF},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Beheeyem (innate Levitate per roster rule) ----
    {
        .species = SPECIES_BEHEEYEM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Trick Room attacker
        .moves = {MOVE_TRICK_ROOM, MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_THUNDERBOLT},
        .ability = ABILITY_ANALYTIC,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Chandelure (innate Levitate per roster rule) ----
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Infiltrator revenge killer
        .moves = {MOVE_FIRE_BLAST, MOVE_SHADOW_BALL, MOVE_ENERGY_BALL, MOVE_TRICK},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Flash Fire Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_FIRE_BLAST, MOVE_SHADOW_BALL, MOVE_SUBSTITUTE},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHANDELURE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Trick Room wallbreaker
        .moves = {MOVE_TRICK_ROOM, MOVE_HEAT_WAVE, MOVE_SHADOW_BALL, MOVE_ENERGY_BALL},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Haxorus ----
    {
        .species = SPECIES_HAXORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Mold Breaker Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_POISON_JAB},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HAXORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Mold Breaker band breaker
        .moves = {MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_CLOSE_COMBAT, MOVE_FIRST_IMPRESSION},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Beartic ----
    {
        .species = SPECIES_BEARTIC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swift Swim Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_ICICLE_CRASH, MOVE_LIQUIDATION, MOVE_AQUA_JET},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Cryogonal (innate Levitate per roster rule) ----
    {
        .species = SPECIES_CRYOGONAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Levitate special wall + Rapid Spin
        .moves = {MOVE_FREEZE_DRY, MOVE_RAPID_SPIN, MOVE_RECOVER, MOVE_TOXIC},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Mienshao ----
    {
        .species = SPECIES_MIENSHAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Regenerator revenge killer / pivot
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_STONE_EDGE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MIENSHAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Reckless wallbreaker
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_KNOCK_OFF, MOVE_POISON_JAB, MOVE_U_TURN},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Druddigon ----
    {
        .species = SPECIES_DRUDDIGON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Rough Skin bulky pivot
        .moves = {MOVE_STEALTH_ROCK, MOVE_DRAGON_CLAW, MOVE_GLARE, MOVE_FIRE_PUNCH},
        .ability = ABILITY_ROUGH_SKIN,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRUDDIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force band
        .moves = {MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_FIRE_PUNCH, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Golurk ----
    {
        .species = SPECIES_GOLURK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // No Guard band breaker
        .moves = {MOVE_EARTHQUAKE, MOVE_POLTERGEIST, MOVE_DYNAMIC_PUNCH, MOVE_ICE_PUNCH},
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOLURK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Iron Fist bulky hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_POLTERGEIST, MOVE_TOXIC},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Bisharp ----
    {
        .species = SPECIES_BISHARP,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Defiant Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_KNOCK_OFF, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BISHARP,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Defiant band breaker
        .moves = {MOVE_KNOCK_OFF, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Bouffalant ----
    {
        .species = SPECIES_BOUFFALANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Reckless head-charge band
        .moves = {MOVE_HEAD_CHARGE, MOVE_EARTHQUAKE, MOVE_SUPERPOWER, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_RECKLESS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Braviary ----
    {
        .species = SPECIES_BRAVIARY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Defiant band
        .moves = {MOVE_BRAVE_BIRD, MOVE_CLOSE_COMBAT, MOVE_U_TURN, MOVE_ROCK_SLIDE},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRAVIARY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Sheer Force Bulk Up sweeper
        .moves = {MOVE_BULK_UP, MOVE_BRAVE_BIRD, MOVE_CLOSE_COMBAT, MOVE_ROOST},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Braviary-Hisui ----
    {
        .species = SPECIES_BRAVIARY_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Tinted Lens special attacker
        .moves = {MOVE_HURRICANE, MOVE_PSYCHIC, MOVE_HEAT_WAVE, MOVE_U_TURN},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRAVIARY_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_HURRICANE, MOVE_PSYCHIC, MOVE_SUBSTITUTE},
        .ability = ABILITY_TINTED_LENS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Mandibuzz ----
    {
        .species = SPECIES_MANDIBUZZ,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Overcoat defensive Defog pivot
        .moves = {MOVE_FOUL_PLAY, MOVE_ROOST, MOVE_DEFOG, MOVE_TOXIC},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MANDIBUZZ,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Weak Armor physical wall
        .moves = {MOVE_FOUL_PLAY, MOVE_ROOST, MOVE_KNOCK_OFF, MOVE_TAUNT},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Heatmor ----
    {
        .species = SPECIES_HEATMOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // White Smoke mixed attacker
        .moves = {MOVE_FIRE_BLAST, MOVE_GIGA_DRAIN, MOVE_SUCKER_PUNCH, MOVE_FOCUS_BLAST},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_RASH,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Durant ----
    {
        .species = SPECIES_DURANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Hustle band breaker
        .moves = {MOVE_IRON_HEAD, MOVE_X_SCISSOR, MOVE_STONE_EDGE, MOVE_SUPERPOWER},
        .ability = ABILITY_HUSTLE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DURANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swarm Hone Claws sweeper
        .moves = {MOVE_HONE_CLAWS, MOVE_IRON_HEAD, MOVE_X_SCISSOR, MOVE_ROCK_SLIDE},
        .ability = ABILITY_SWARM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Hydreigon (native Levitate) ----
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Levitate revenge killer
        .moves = {MOVE_DARK_PULSE, MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_U_TURN},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_DARK_PULSE, MOVE_FIRE_BLAST, MOVE_FLASH_CANNON},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HYDREIGON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Defog pivot
        .moves = {MOVE_DRACO_METEOR, MOVE_DARK_PULSE, MOVE_DEFOG, MOVE_ROOST},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 96, 0, 160),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Volcarona ----
    {
        .species = SPECIES_VOLCARONA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Quiver Dance sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_FLAMETHROWER, MOVE_BUG_BUZZ, MOVE_GIGA_DRAIN},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VOLCARONA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Quiver Dance + Roost
        .moves = {MOVE_QUIVER_DANCE, MOVE_FIRE_BLAST, MOVE_GIGA_DRAIN, MOVE_ROOST},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 100, 156, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Cobalion ----
    {
        .species = SPECIES_COBALION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Justified Swords Dance setup
        .moves = {MOVE_SWORDS_DANCE, MOVE_IRON_HEAD, MOVE_CLOSE_COMBAT, MOVE_STEALTH_ROCK},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_COBALION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer / pivot
        .moves = {MOVE_CLOSE_COMBAT, MOVE_IRON_HEAD, MOVE_STONE_EDGE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Terrakion ----
    {
        .species = SPECIES_TERRAKION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Justified band breaker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_QUICK_ATTACK},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TERRAKION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Swords Dance / hazard lead
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_STEALTH_ROCK},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Virizion ----
    {
        .species = SPECIES_VIRIZION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_LEAF_BLADE, MOVE_STONE_EDGE},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VIRIZION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Calm Mind special tank
        .moves = {MOVE_GIGA_DRAIN, MOVE_FOCUS_BLAST, MOVE_AIR_SLASH, MOVE_VACUUM_WAVE},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Tornadus ----
    {
        .species = SPECIES_TORNADUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Prankster offensive pivot
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_FOCUS_BLAST, MOVE_U_TURN},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TORNADUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Prankster Tailwind support
        .moves = {MOVE_TAILWIND, MOVE_HURRICANE, MOVE_TAUNT, MOVE_RAIN_DANCE},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Tornadus-Therian ----
    {
        .species = SPECIES_TORNADUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Regenerator special pivot
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_KNOCK_OFF, MOVE_U_TURN},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Thundurus ----
    {
        .species = SPECIES_THUNDURUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Prankster mixed attacker
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_KNOCK_OFF, MOVE_NASTY_PLOT},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_THUNDURUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Prankster Thunder Wave support
        .moves = {MOVE_THUNDER_WAVE, MOVE_THUNDERBOLT, MOVE_TAUNT, MOVE_VOLT_SWITCH},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Thundurus-Therian ----
    {
        .species = SPECIES_THUNDURUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Volt Absorb revenge killer
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST, MOVE_SLUDGE_WAVE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Reshiram ----
    {
        .species = SPECIES_RESHIRAM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Turboblaze special breaker
        .moves = {MOVE_BLUE_FLARE, MOVE_DRACO_METEOR, MOVE_FLAMETHROWER, MOVE_EARTH_POWER},
        .ability = ABILITY_TURBOBLAZE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RESHIRAM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Roost attacker
        .moves = {MOVE_BLUE_FLARE, MOVE_DRAGON_PULSE, MOVE_ROOST, MOVE_WILL_O_WISP},
        .ability = ABILITY_TURBOBLAZE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 200, 56),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Zekrom ----
    {
        .species = SPECIES_ZEKROM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Teravolt Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_BOLT_STRIKE, MOVE_OUTRAGE, MOVE_EARTHQUAKE},
        .ability = ABILITY_TERAVOLT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZEKROM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Teravolt band breaker
        .moves = {MOVE_BOLT_STRIKE, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_TERAVOLT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Landorus ----
    {
        .species = SPECIES_LANDORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sheer Force special nuke
        .moves = {MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE, MOVE_FOCUS_BLAST, MOVE_PSYCHIC},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Landorus-Therian ----
    {
        .species = SPECIES_LANDORUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Intimidate revenge killer
        .moves = {MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_U_TURN, MOVE_KNOCK_OFF},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LANDORUS_THERIAN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Intimidate bulky hazard pivot
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_U_TURN, MOVE_TOXIC},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 216, 0, 0, 40),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Kyurem ----
    {
        .species = SPECIES_KYUREM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Pressure special breaker
        .moves = {MOVE_ICE_BEAM, MOVE_DRACO_METEOR, MOVE_FREEZE_DRY, MOVE_EARTH_POWER},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KYUREM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sub-Roost stallbreaker
        .moves = {MOVE_SUBSTITUTE, MOVE_ROOST, MOVE_ICE_BEAM, MOVE_EARTH_POWER},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 56, 200, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Kyurem-Black ----
    {
        .species = SPECIES_KYUREM_BLACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Teravolt physical breaker
        .moves = {MOVE_ICICLE_CRASH, MOVE_FUSION_BOLT, MOVE_OUTRAGE, MOVE_EARTHQUAKE},
        .ability = ABILITY_TERAVOLT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KYUREM_BLACK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_ICICLE_CRASH, MOVE_FUSION_BOLT, MOVE_EARTHQUAKE},
        .ability = ABILITY_TERAVOLT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Kyurem-White ----
    {
        .species = SPECIES_KYUREM_WHITE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Turboblaze special nuke
        .moves = {MOVE_ICE_BEAM, MOVE_FUSION_FLARE, MOVE_DRACO_METEOR, MOVE_EARTH_POWER},
        .ability = ABILITY_TURBOBLAZE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Keldeo ----
    {
        .species = SPECIES_KELDEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Justified special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_SECRET_SWORD, MOVE_SCALD, MOVE_ICY_WIND},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KELDEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_HYDRO_PUMP, MOVE_SECRET_SWORD, MOVE_HYDRO_PUMP, MOVE_ICY_WIND},
        .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Meloetta ----
    {
        .species = SPECIES_MELOETTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Serene Grace Calm Mind attacker
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_HYPER_VOICE, MOVE_FOCUS_BLAST},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MELOETTA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // bulky special pivot
        .moves = {MOVE_HYPER_VOICE, MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Genesect ----
    {
        .species = SPECIES_GENESECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Download revenge killer / pivot
        .moves = {MOVE_U_TURN, MOVE_ICE_BEAM, MOVE_FLAMETHROWER, MOVE_THUNDERBOLT},
        .ability = ABILITY_DOWNLOAD,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GENESECT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Download Techno Blast attacker
        .moves = {MOVE_TECHNO_BLAST, MOVE_FLASH_CANNON, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .ability = ABILITY_DOWNLOAD,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation VI
    // ============================================================

    // ---- Chesnaught ----
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Spikes / pivot wall
        .moves = {MOVE_SPIKES, MOVE_SPIKY_SHIELD, MOVE_LEECH_SEED, MOVE_BODY_PRESS},
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // mixed bulky attacker
        .moves = {MOVE_WOOD_HAMMER, MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_GUNK_SHOT},
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHESNAUGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance breaker
        .moves = {MOVE_SWORDS_DANCE, MOVE_WOOD_HAMMER, MOVE_CLOSE_COMBAT, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Delphox ----
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_PSYSHOCK, MOVE_DAZZLING_GLEAM, MOVE_GRASS_KNOT},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_FIRE_BLAST, MOVE_PSYCHIC, MOVE_MYSTICAL_FIRE},
        .ability = ABILITY_MAGICIAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DELPHOX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sub stallbreaker
        .moves = {MOVE_SUBSTITUTE, MOVE_CALM_MIND, MOVE_FIRE_BLAST, MOVE_PSYSHOCK},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Greninja ----
    {
        .species = SPECIES_GRENINJA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Protean wallbreaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_DARK_PULSE, MOVE_ICE_BEAM, MOVE_GUNK_SHOT},
        .ability = ABILITY_PROTEAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GRENINJA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // suicide hazard lead
        .moves = {MOVE_SPIKES, MOVE_TOXIC_SPIKES, MOVE_HYDRO_PUMP, MOVE_DARK_PULSE},
        .ability = ABILITY_PROTEAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GRENINJA_BATTLE_BOND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Battle Bond revenge killer
        .moves = {MOVE_HYDRO_PUMP, MOVE_DARK_PULSE, MOVE_ICE_BEAM, MOVE_U_TURN},
        .ability = ABILITY_BATTLE_BOND,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Diggersby ----
    {
        .species = SPECIES_DIGGERSBY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Huge Power band
        .moves = {MOVE_EARTHQUAKE, MOVE_RETURN, MOVE_QUICK_ATTACK, MOVE_WILD_CHARGE},
        .ability = ABILITY_HUGE_POWER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DIGGERSBY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_RETURN, MOVE_QUICK_ATTACK},
        .ability = ABILITY_HUGE_POWER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Talonflame ----
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Gale Wings priority sweeper
        .moves = {MOVE_BRAVE_BIRD, MOVE_FLARE_BLITZ, MOVE_SWORDS_DANCE, MOVE_U_TURN},
        .ability = ABILITY_GALE_WINGS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Roost / hazard control
        .moves = {MOVE_BRAVE_BIRD, MOVE_ROOST, MOVE_DEFOG, MOVE_WILL_O_WISP},
        .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(248, 0, 240, 0, 0, 20),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TALONFLAME,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SHARP_BEAK, // Tailwind setter + priority
        .moves = {MOVE_TAILWIND, MOVE_BRAVE_BIRD, MOVE_FLARE_BLITZ, MOVE_WILL_O_WISP},
        .ability = ABILITY_GALE_WINGS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Pyroar ----
    {
        .species = SPECIES_PYROAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_FIRE_BLAST, MOVE_HYPER_VOICE, MOVE_DARK_PULSE, MOVE_SOLAR_BEAM},
        .ability = ABILITY_UNNERVE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PYROAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THROAT_SPRAY, // Hyper Voice spread attacker
        .moves = {MOVE_HYPER_VOICE, MOVE_HEAT_WAVE, MOVE_SNARL, MOVE_PROTECT},
        .ability = ABILITY_UNNERVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Florges ----
    {
        .species = SPECIES_FLORGES,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind cleric wall
        .moves = {MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_SYNTHESIS, MOVE_AROMATHERAPY},
        .ability = ABILITY_FLOWER_VEIL,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLORGES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Pixilate breaker
        .moves = {MOVE_HYPER_VOICE, MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_ENERGY_BALL},
        .ability = ABILITY_SYMBIOSIS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Gogoat ----
    {
        .species = SPECIES_GOGOAT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Sap Sipper tank
        .moves = {MOVE_BULK_UP, MOVE_HORN_LEECH, MOVE_EARTHQUAKE, MOVE_MILK_DRINK},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOGOAT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // physical attacker
        .moves = {MOVE_HORN_LEECH, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_BULK_UP},
        .ability = ABILITY_GRASS_PELT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Pangoro ----
    {
        .species = SPECIES_PANGORO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Iron Fist / Scrappy band
        .moves = {MOVE_KNOCK_OFF, MOVE_CLOSE_COMBAT, MOVE_GUNK_SHOT, MOVE_ICE_PUNCH},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PANGORO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance breaker
        .moves = {MOVE_SWORDS_DANCE, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Meowstic (Male) ----
    {
        .species = SPECIES_MEOWSTIC,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Prankster screens/support
        .moves = {MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_FAKE_OUT, MOVE_THUNDER_WAVE},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEOWSTIC,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB, // Prankster disruption support
        .moves = {MOVE_FOLLOW_ME, MOVE_THUNDER_WAVE, MOVE_HELPING_HAND, MOVE_PSYCHIC},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Meowstic (Female) ----
    {
        .species = SPECIES_MEOWSTIC_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Competitive special attacker
        .moves = {MOVE_NASTY_PLOT, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_ENERGY_BALL},
        .ability = ABILITY_COMPETITIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEOWSTIC_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Competitive breaker
        .moves = {MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_SHADOW_BALL, MOVE_ENERGY_BALL},
        .ability = ABILITY_COMPETITIVE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Aegislash (innate Stance Change; no Air Balloon) ----
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // King's Shield stance tank
        .moves = {MOVE_KINGS_SHIELD, MOVE_IRON_HEAD, MOVE_SHADOW_SNEAK, MOVE_SHADOW_CLAW},
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Weakness Policy sweeper
        .moves = {MOVE_KINGS_SHIELD, MOVE_SHADOW_BALL, MOVE_FLASH_CANNON, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AEGISLASH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance / Spectral Thief
        .moves = {MOVE_SWORDS_DANCE, MOVE_SPECTRAL_THIEF, MOVE_IRON_HEAD, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_STANCE_CHANGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Aromatisse ----
    {
        .species = SPECIES_AROMATISSE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Aroma Veil Trick Room cleric
        .moves = {MOVE_TRICK_ROOM, MOVE_MOONBLAST, MOVE_AROMATHERAPY, MOVE_WISH},
        .ability = ABILITY_AROMA_VEIL,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Slurpuff ----
    {
        .species = SPECIES_SLURPUFF,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_SITRUS_BERRY, // Unburden Belly Drum sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_PLAY_ROUGH, MOVE_DRAIN_PUNCH, MOVE_FACADE},
        .ability = ABILITY_UNBURDEN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SLURPUFF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind special wall
        .moves = {MOVE_CALM_MIND, MOVE_DAZZLING_GLEAM, MOVE_FLAMETHROWER, MOVE_DRAINING_KISS},
        .ability = ABILITY_SWEET_VEIL,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Malamar ----
    {
        .species = SPECIES_MALAMAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Contrary Superpower sweeper
        .moves = {MOVE_SUPERPOWER, MOVE_KNOCK_OFF, MOVE_PSYCHO_CUT, MOVE_REST},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MALAMAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Superpower + priority
        .moves = {MOVE_SUPERPOWER, MOVE_KNOCK_OFF, MOVE_SUCKER_PUNCH, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Barbaracle ----
    {
        .species = SPECIES_BARBARACLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_LIQUIDATION, MOVE_STONE_EDGE, MOVE_CROSS_CHOP},
        .ability = ABILITY_TOUGH_CLAWS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BARBARACLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // physical Tough Claws attacker
        .moves = {MOVE_LIQUIDATION, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_SHADOW_CLAW},
        .ability = ABILITY_TOUGH_CLAWS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Dragalge ----
    {
        .species = SPECIES_DRAGALGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Adaptability special tank
        .moves = {MOVE_DRACO_METEOR, MOVE_SLUDGE_WAVE, MOVE_FLIP_TURN, MOVE_FOCUS_BLAST},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGALGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky Toxic Spikes pivot
        .moves = {MOVE_TOXIC_SPIKES, MOVE_SLUDGE_BOMB, MOVE_DRAGON_PULSE, MOVE_TOXIC},
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Clawitzer ----
    {
        .species = SPECIES_CLAWITZER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Mega Launcher breaker
        .moves = {MOVE_DARK_PULSE, MOVE_WATER_PULSE, MOVE_AURA_SPHERE, MOVE_DRAGON_PULSE},
        .ability = ABILITY_MEGA_LAUNCHER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLAWITZER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // bulky special pivot
        .moves = {MOVE_HYDRO_PUMP, MOVE_DARK_PULSE, MOVE_ICE_BEAM, MOVE_AURA_SPHERE},
        .ability = ABILITY_MEGA_LAUNCHER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Heliolisk ----
    {
        .species = SPECIES_HELIOLISK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dry Skin fast special attacker
        .moves = {MOVE_THUNDERBOLT, MOVE_HYPER_VOICE, MOVE_VOLT_SWITCH, MOVE_SURF},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HELIOLISK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_HYPER_VOICE, MOVE_FOCUS_BLAST},
        .ability = ABILITY_DRY_SKIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Tyrantrum ----
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Strong Jaw band breaker
        .moves = {MOVE_HEAD_SMASH, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_CRUNCH},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_OUTRAGE, MOVE_HEAD_SMASH, MOVE_EARTHQUAKE},
        .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TYRANTRUM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SCARF, // Strong Jaw revenge killer
        .moves = {MOVE_PSYCHIC_FANGS, MOVE_CRUNCH, MOVE_DRAGON_CLAW, MOVE_FIRE_FANG},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Aurorus ----
    {
        .species = SPECIES_AURORUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Refrigerate bulky attacker / Aurora Veil
        .moves = {MOVE_HYPER_VOICE, MOVE_FREEZE_DRY, MOVE_THUNDERBOLT, MOVE_EARTH_POWER},
        .ability = ABILITY_REFRIGERATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AURORUS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Snow Warning Aurora Veil setter
        .moves = {MOVE_AURORA_VEIL, MOVE_FREEZE_DRY, MOVE_HYPER_VOICE, MOVE_THUNDER_WAVE},
        .ability = ABILITY_SNOW_WARNING,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Sylveon ----
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Pixilate Hyper Voice breaker
        .moves = {MOVE_HYPER_VOICE, MOVE_PSYSHOCK, MOVE_MYSTICAL_FIRE, MOVE_QUICK_ATTACK},
        .ability = ABILITY_PIXILATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind cleric wall
        .moves = {MOVE_CALM_MIND, MOVE_HYPER_VOICE, MOVE_WISH, MOVE_PROTECT},
        .ability = ABILITY_PIXILATE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SYLVEON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_THROAT_SPRAY, // Hyper Voice spread support
        .moves = {MOVE_HYPER_VOICE, MOVE_MYSTICAL_FIRE, MOVE_HELPING_HAND, MOVE_PROTECT},
        .ability = ABILITY_PIXILATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Hawlucha ----
    {
        .species = SPECIES_HAWLUCHA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GRASSY_SEED, // Unburden Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_ACROBATICS, MOVE_CLOSE_COMBAT, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_UNBURDEN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HAWLUCHA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Mold Breaker physical attacker
        .moves = {MOVE_ACROBATICS, MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_PROTECT},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Dedenne ----
    {
        .species = SPECIES_DEDENNE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Cheek Pouch / screens support
        .moves = {MOVE_NUZZLE, MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_CHEEK_POUCH,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Carbink (innate; no Air Balloon) ----
    {
        .species = SPECIES_CARBINK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy dual-screens / hazards wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_LIGHT_SCREEN, MOVE_REFLECT, MOVE_BODY_PRESS},
        .ability = ABILITY_STURDY,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CARBINK,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Trick Room screens setter
        .moves = {MOVE_TRICK_ROOM, MOVE_LIGHT_SCREEN, MOVE_REFLECT, MOVE_MOONBLAST},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Goodra ----
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Sap Sipper special tank
        .moves = {MOVE_DRACO_METEOR, MOVE_FIRE_BLAST, MOVE_THUNDERBOLT, MOVE_FLIP_TURN},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(248, 0, 0, 0, 252, 8),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Gooey bulky pivot
        .moves = {MOVE_DRAGON_PULSE, MOVE_SCALD, MOVE_TOXIC, MOVE_REST},
        .ability = ABILITY_GOOEY,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOODRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Hydration breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_SLUDGE_WAVE, MOVE_FIRE_BLAST, MOVE_THUNDERBOLT},
        .ability = ABILITY_HYDRATION,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Goodra-Hisui ----
    {
        .species = SPECIES_GOODRA_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // Sap Sipper special tank
        .moves = {MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_FIRE_BLAST, MOVE_EARTH_POWER},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOODRA_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shell Armor bulky setup
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_DRACO_METEOR, MOVE_RECOVER},
        .ability = ABILITY_SHELL_ARMOR,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Klefki (innate Levitate; no Air Balloon) ----
    {
        .species = SPECIES_KLEFKI,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Prankster screens + Spikes
        .moves = {MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_THUNDER_WAVE, MOVE_SPIKES},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KLEFKI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Prankster annoyer wall
        .moves = {MOVE_SPIKES, MOVE_THUNDER_WAVE, MOVE_FOUL_PLAY, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Trevenant ----
    {
        .species = SPECIES_TREVENANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Harvest Sitrus stall
        .moves = {MOVE_HORN_LEECH, MOVE_POLTERGEIST, MOVE_WILL_O_WISP, MOVE_LEECH_SEED},
        .ability = ABILITY_HARVEST,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TREVENANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Natural Cure trick-room-friendly breaker
        .moves = {MOVE_POLTERGEIST, MOVE_WOOD_HAMMER, MOVE_EARTHQUAKE, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Gourgeist-Super (innate; no Air Balloon) ----
    {
        .species = SPECIES_GOURGEIST_SUPER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // physically defensive WoW wall
        .moves = {MOVE_WILL_O_WISP, MOVE_POLTERGEIST, MOVE_LEECH_SEED, MOVE_SYNTHESIS},
        .ability = ABILITY_FRISK,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOURGEIST_SUPER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Trick Room band breaker
        .moves = {MOVE_POLTERGEIST, MOVE_SEED_BOMB, MOVE_TRICK, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_INSOMNIA,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Avalugg ----
    {
        .species = SPECIES_AVALUGG,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sturdy physical wall / Rapid Spin
        .moves = {MOVE_RECOVER, MOVE_AVALANCHE, MOVE_BODY_PRESS, MOVE_RAPID_SPIN},
        .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AVALUGG,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Ice Body chip wall
        .moves = {MOVE_AVALANCHE, MOVE_BODY_PRESS, MOVE_RECOVER, MOVE_EARTHQUAKE},
        .ability = ABILITY_ICE_BODY,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Avalugg-Hisui ----
    {
        .species = SPECIES_AVALUGG_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Strong Jaw bulky setup
        .moves = {MOVE_RECOVER, MOVE_ICICLE_CRASH, MOVE_BODY_PRESS, MOVE_RAPID_SPIN},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_AVALUGG_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Strong Jaw band attacker
        .moves = {MOVE_ICE_FANG, MOVE_CRUNCH, MOVE_STONE_EDGE, MOVE_BODY_PRESS},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Noivern ----
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Infiltrator special breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_HURRICANE, MOVE_FLAMETHROWER, MOVE_U_TURN},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // defensive Defog pivot
        .moves = {MOVE_HURRICANE, MOVE_ROOST, MOVE_DEFOG, MOVE_U_TURN},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NOIVERN,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Tailwind setter
        .moves = {MOVE_TAILWIND, MOVE_HURRICANE, MOVE_DRACO_METEOR, MOVE_HEAT_WAVE},
        .ability = ABILITY_FRISK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },

    // ---- Xerneas ----
    {
        .species = SPECIES_XERNEAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_POWER_HERB, // Geomancy sweeper
        .moves = {MOVE_GEOMANCY, MOVE_MOONBLAST, MOVE_THUNDERBOLT, MOVE_FOCUS_BLAST},
        .ability = ABILITY_FAIRY_AURA,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_XERNEAS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // physical Swords Dance variant
        .moves = {MOVE_SWORDS_DANCE, MOVE_PLAY_ROUGH, MOVE_CLOSE_COMBAT, MOVE_HORN_LEECH},
        .ability = ABILITY_FAIRY_AURA,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Yveltal ----
    {
        .species = SPECIES_YVELTAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dark Aura mixed attacker
        .moves = {MOVE_DARK_PULSE, MOVE_HURRICANE, MOVE_FOCUS_BLAST, MOVE_U_TURN},
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_YVELTAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Roost pivot
        .moves = {MOVE_FOUL_PLAY, MOVE_HURRICANE, MOVE_ROOST, MOVE_TOXIC},
        .ability = ABILITY_DARK_AURA,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 200, 0, 0, 56),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Zygarde ----
    {
        .species = SPECIES_ZYGARDE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Dragon Dance bulky sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_THOUSAND_ARROWS, MOVE_OUTRAGE, MOVE_COIL},
        .ability = ABILITY_AURA_BREAK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZYGARDE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Coil bulky setup wall
        .moves = {MOVE_COIL, MOVE_THOUSAND_ARROWS, MOVE_DRAGON_TAIL, MOVE_GLARE},
        .ability = ABILITY_POWER_CONSTRUCT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Diancie (innate Levitate-class; no Air Balloon) ----
    {
        .species = SPECIES_DIANCIE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DIANCITE, // Mega Diancie (Magic Bounce) glass cannon
        .moves = {MOVE_DIAMOND_STORM, MOVE_MOONBLAST, MOVE_EARTH_POWER, MOVE_PROTECT},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DIANCIE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Clear Body hazards / dual-screens wall
        .moves = {MOVE_STEALTH_ROCK, MOVE_DIAMOND_STORM, MOVE_MOONBLAST, MOVE_REFLECT},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Hoopa (Confined; innate Levitate-class; no Air Balloon) ----
    {
        .species = SPECIES_HOOPA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Magician special breaker
        .moves = {MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST, MOVE_NASTY_PLOT},
        .ability = ABILITY_MAGICIAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Hoopa-Unbound (innate Levitate-class; no Air Balloon) ----
    {
        .species = SPECIES_HOOPA_UNBOUND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed Magician wallbreaker
        .moves = {MOVE_HYPERSPACE_FURY, MOVE_PSYCHIC, MOVE_GUNK_SHOT, MOVE_FIRE_PUNCH},
        .ability = ABILITY_MAGICIAN,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HOOPA_UNBOUND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // special tank breaker
        .moves = {MOVE_PSYCHIC, MOVE_DARK_PULSE, MOVE_FOCUS_BLAST, MOVE_GUNK_SHOT},
        .ability = ABILITY_MAGICIAN,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
    },

    // ---- Volcanion ----
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Water Absorb special breaker
        .moves = {MOVE_STEAM_ERUPTION, MOVE_FLAMETHROWER, MOVE_SLUDGE_WAVE, MOVE_EARTH_POWER},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Substitute pivot
        .moves = {MOVE_SUBSTITUTE, MOVE_STEAM_ERUPTION, MOVE_FLAMETHROWER, MOVE_TOXIC},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VOLCANION,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // spread special tank
        .moves = {MOVE_HEAT_WAVE, MOVE_STEAM_ERUPTION, MOVE_EARTH_POWER, MOVE_SLUDGE_WAVE},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation VII
    // ============================================================

    // ---- Decidueye ----
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DECIDIUM_Z, // Sinister Arrow Raid nuke
        .moves = {MOVE_SWORDS_DANCE, MOVE_SPIRIT_SHACKLE, MOVE_LEAF_BLADE, MOVE_BRAVE_BIRD},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Long Reach trap-shooter
        .moves = {MOVE_SPIRIT_SHACKLE, MOVE_LEAF_BLADE, MOVE_DEFOG, MOVE_ROOST},
        .ability = ABILITY_LONG_REACH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DECIDUEYE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed attacker
        .moves = {MOVE_LEAF_BLADE, MOVE_SPIRIT_SHACKLE, MOVE_SUCKER_PUNCH, MOVE_U_TURN},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Decidueye-Hisui ----
    {
        .species = SPECIES_DECIDUEYE_HISUI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Scrappy Triple Arrows attacker
        .moves = {MOVE_TRIPLE_ARROWS, MOVE_CLOSE_COMBAT, MOVE_LEAF_BLADE, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DECIDUEYE_HISUI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Swords Dance bulky setup
        .moves = {MOVE_SWORDS_DANCE, MOVE_TRIPLE_ARROWS, MOVE_LEAF_BLADE, MOVE_ROOST},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(248, 16, 0, 0, 0, 244),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Incineroar ----
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_SITRUS_BERRY, // Intimidate pivot, Fake Out support
        .moves = {MOVE_FAKE_OUT, MOVE_FLARE_BLITZ, MOVE_DARKEST_LARIAT, MOVE_PARTING_SHOT},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_INCINIUM_Z, // Malicious Moonsault nuke
        .moves = {MOVE_SWORDS_DANCE, MOVE_FLARE_BLITZ, MOVE_DARKEST_LARIAT, MOVE_EARTHQUAKE},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INCINEROAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Intimidate pivot
        .moves = {MOVE_KNOCK_OFF, MOVE_FLARE_BLITZ, MOVE_WILL_O_WISP, MOVE_U_TURN},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Primarina ----
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Liquid Voice special breaker
        .moves = {MOVE_SPARKLING_ARIA, MOVE_MOONBLAST, MOVE_PSYCHIC, MOVE_FLIP_TURN},
        .ability = ABILITY_LIQUID_VOICE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind bulky setup
        .moves = {MOVE_CALM_MIND, MOVE_SPARKLING_ARIA, MOVE_MOONBLAST, MOVE_REST},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PRIMARINA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_PRIMARIUM_Z, // Oceanic Operetta nuke
        .moves = {MOVE_HYDRO_PUMP, MOVE_MOONBLAST, MOVE_ENERGY_BALL, MOVE_PSYCHIC},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Toucannon ----
    {
        .species = SPECIES_TOUCANNON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Skill Link Bullet Seed band
        .moves = {MOVE_BEAK_BLAST, MOVE_BULLET_SEED, MOVE_ROCK_BLAST, MOVE_BRAVE_BIRD},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOUCANNON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE, // guaranteed multi-hit
        .moves = {MOVE_BULLET_SEED, MOVE_ROCK_BLAST, MOVE_BRAVE_BIRD, MOVE_BEAK_BLAST},
        .ability = ABILITY_SKILL_LINK,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Vikavolt ----
    {
        .species = SPECIES_VIKAVOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_BUG_BUZZ, MOVE_THUNDERBOLT, MOVE_ENERGY_BALL, MOVE_VOLT_SWITCH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_VIKAVOLT,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room special attacker
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves = {MOVE_BUG_BUZZ, MOVE_THUNDERBOLT, MOVE_ENERGY_BALL, MOVE_AIR_SLASH},
        .ability = ABILITY_LEVITATE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Crabominable ----
    {
        .species = SPECIES_CRABOMINABLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Iron Fist punch band
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ICE_PUNCH, MOVE_EARTHQUAKE, MOVE_ICE_HAMMER},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CRABOMINABLE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room bruiser
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ICE_PUNCH, MOVE_EARTHQUAKE, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_IRON_FIST,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Oricorio (Baile) ----
    {
        .species = SPECIES_ORICORIO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Dancer Fire/Flying special attacker
        .moves = {MOVE_REVELATION_DANCE, MOVE_HURRICANE, MOVE_ROOST, MOVE_CALM_MIND},
        .ability = ABILITY_DANCER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ORICORIO_PAU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Psychic/Flying pivot
        .moves = {MOVE_REVELATION_DANCE, MOVE_HURRICANE, MOVE_ROOST, MOVE_U_TURN},
        .ability = ABILITY_DANCER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Ribombee ----
    {
        .species = SPECIES_RIBOMBEE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast Sticky Web lead
        .moves = {MOVE_STICKY_WEB, MOVE_MOONBLAST, MOVE_BUG_BUZZ, MOVE_STUN_SPORE},
        .ability = ABILITY_SWEET_VEIL,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RIBOMBEE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Quiver Dance sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_MOONBLAST, MOVE_BUG_BUZZ, MOVE_POLLEN_PUFF},
        .ability = ABILITY_SHIELD_DUST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Lycanroc (Midday) ----
    {
        .species = SPECIES_LYCANROC,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sand Rush attacker
        .moves = {MOVE_STONE_EDGE, MOVE_ACCELEROCK, MOVE_CLOSE_COMBAT, MOVE_PSYCHIC_FANGS},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LYCANROC_DUSK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LYCANIUM_Z, // Splintered Stormshards nuke, Tough Claws
        .moves = {MOVE_SWORDS_DANCE, MOVE_STONE_EDGE, MOVE_ACCELEROCK, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_TOUGH_CLAWS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LYCANROC_MIDNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // No Guard band
        .moves = {MOVE_STONE_EDGE, MOVE_CLOSE_COMBAT, MOVE_ACCELEROCK, MOVE_EARTHQUAKE},
        .ability = ABILITY_NO_GUARD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Wishiwashi (School) ----
    {
        .species = SPECIES_WISHIWASHI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Schooling special tank
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FLIP_TURN, MOVE_EARTH_POWER},
        .ability = ABILITY_SCHOOLING,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WISHIWASHI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
        .moves = {MOVE_SCALD, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_PROTECT},
        .ability = ABILITY_SCHOOLING,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Toxapex ----
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Regenerator wall
        .moves = {MOVE_SCALD, MOVE_TOXIC, MOVE_RECOVER, MOVE_HAZE},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // physically defensive Banded Bunker stall
        .moves = {MOVE_BANEFUL_BUNKER, MOVE_TOXIC_SPIKES, MOVE_RECOVER, MOVE_SCALD},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOXAPEX,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Merciless redirect support
        .moves = {MOVE_SCALD, MOVE_BANEFUL_BUNKER, MOVE_TOXIC, MOVE_HAZE},
        .ability = ABILITY_MERCILESS,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Mudsdale ----
    {
        .species = SPECIES_MUDSDALE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Stamina physical wall
        .moves = {MOVE_HIGH_HORSEPOWER, MOVE_BODY_PRESS, MOVE_STEALTH_ROCK, MOVE_ROAR},
        .ability = ABILITY_STAMINA,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MUDSDALE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Stamina hazard tank
        .moves = {MOVE_HIGH_HORSEPOWER, MOVE_HEAVY_SLAM, MOVE_STEALTH_ROCK, MOVE_TOXIC},
        .ability = ABILITY_STAMINA,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Araquanid ----
    {
        .species = SPECIES_ARAQUANID,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Water Bubble bulky attacker
        .moves = {MOVE_LIQUIDATION, MOVE_LEECH_LIFE, MOVE_MIRROR_COAT, MOVE_TOXIC},
        .ability = ABILITY_WATER_BUBBLE,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARAQUANID,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room Water Bubble nuke
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves = {MOVE_LIQUIDATION, MOVE_LEECH_LIFE, MOVE_ICE_PUNCH, MOVE_BUG_BITE},
        .ability = ABILITY_WATER_BUBBLE,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Lurantis ----
    {
        .species = SPECIES_LURANTIS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Contrary Leaf Storm attacker
        .moves = {MOVE_LEAF_STORM, MOVE_SUPERPOWER, MOVE_HIDDEN_POWER, MOVE_SYNTHESIS},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LURANTIS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky support
        .moves = {MOVE_LEAF_BLADE, MOVE_LEECH_SEED, MOVE_SYNTHESIS, MOVE_TOXIC},
        .ability = ABILITY_LEAF_GUARD,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Salazzle ----
    {
        .species = SPECIES_SALAZZLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Corrosion toxic staller
        .moves = {MOVE_TOXIC, MOVE_FIRE_BLAST, MOVE_PROTECT, MOVE_SUBSTITUTE},
        .ability = ABILITY_CORROSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SALAZZLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_FIRE_BLAST, MOVE_SLUDGE_WAVE, MOVE_DRAGON_PULSE},
        .ability = ABILITY_CORROSION,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Bewear ----
    {
        .species = SPECIES_BEWEAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Fluffy / Force band
        .moves = {MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH},
        .ability = ABILITY_FLUFFY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BEWEAR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Fluffy bulky setup
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE},
        .ability = ABILITY_FLUFFY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Tsareena ----
    {
        .species = SPECIES_TSAREENA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Queenly Majesty attacker
        .moves = {MOVE_POWER_WHIP, MOVE_HIGH_JUMP_KICK, MOVE_PLAY_ROUGH, MOVE_U_TURN},
        .ability = ABILITY_QUEENLY_MAJESTY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TSAREENA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky priority-block pivot
        .moves = {MOVE_POWER_WHIP, MOVE_RAPID_SPIN, MOVE_SYNTHESIS, MOVE_KNOCK_OFF},
        .ability = ABILITY_QUEENLY_MAJESTY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Comfey ---- (innate Levitate)
    {
        .species = SPECIES_COMFEY,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Triage priority healer
        .moves = {MOVE_FLORAL_HEALING, MOVE_DRAINING_KISS, MOVE_GIGA_DRAIN, MOVE_CALM_MIND},
        .ability = ABILITY_TRIAGE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_COMFEY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Triage Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_DRAINING_KISS, MOVE_GIGA_DRAIN, MOVE_PSYCHIC},
        .ability = ABILITY_TRIAGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Oranguru ----
    {
        .species = SPECIES_ORANGURU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Trick Room setter / Instruct support
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves = {MOVE_TRICK_ROOM, MOVE_INSTRUCT, MOVE_PSYCHIC, MOVE_FOUL_PLAY},
        .ability = ABILITY_INNER_FOCUS,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ORANGURU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_NASTY_PLOT},
        .ability = ABILITY_TELEPATHY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Passimian ----
    {
        .species = SPECIES_PASSIMIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Defiant revenge killer
        .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_GUNK_SHOT},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PASSIMIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // physical breaker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_KNOCK_OFF},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Golisopod ----
    {
        .species = SPECIES_GOLISOPOD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Emergency Exit First Impression band
        .moves = {MOVE_FIRST_IMPRESSION, MOVE_LIQUIDATION, MOVE_CLOSE_COMBAT, MOVE_AQUA_JET},
        .ability = ABILITY_EMERGENCY_EXIT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOLISOPOD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky pivot
        .moves = {MOVE_FIRST_IMPRESSION, MOVE_LIQUIDATION, MOVE_SPIKES, MOVE_LEECH_LIFE},
        .ability = ABILITY_EMERGENCY_EXIT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Palossand ----
    {
        .species = SPECIES_PALOSSAND,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Water Compaction defensive trapper
        .moves = {MOVE_SHADOW_BALL, MOVE_EARTH_POWER, MOVE_TOXIC, MOVE_SHORE_UP},
        .ability = ABILITY_WATER_COMPACTION,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PALOSSAND,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GHOSTIUM_Z, // Never-Ending Nightmare setup
        .moves = {MOVE_SHADOW_BALL, MOVE_EARTH_POWER, MOVE_GIGA_DRAIN, MOVE_SHORE_UP},
        .ability = ABILITY_WATER_COMPACTION,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Pyukumuku ----
    {
        .species = SPECIES_PYUKUMUKU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Unaware Counter/Toxic staller
        .moves = {MOVE_COUNTER, MOVE_TOXIC, MOVE_RECOVER, MOVE_SOAK},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Silvally ----
    {
        .species = SPECIES_SILVALLY_DRAGON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_DRAGON_MEMORY, // RKS System Dragon pivot
        .moves = {MOVE_MULTI_ATTACK, MOVE_FLAMETHROWER, MOVE_U_TURN, MOVE_SWORDS_DANCE},
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SILVALLY_FAIRY,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FAIRY_MEMORY, // defensive pivot
        .moves = {MOVE_MULTI_ATTACK, MOVE_PARTING_SHOT, MOVE_DEFOG, MOVE_THUNDER_WAVE},
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SILVALLY_STEEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_STEEL_MEMORY, // Steel attacker
        .moves = {MOVE_MULTI_ATTACK, MOVE_FLAMETHROWER, MOVE_ICE_BEAM, MOVE_U_TURN},
        .ability = ABILITY_RKS_SYSTEM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Minior (Core) ----
    {
        .species = SPECIES_MINIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // Shields Down Shell Smash sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_ACROBATICS, MOVE_POWER_GEM, MOVE_EARTHQUAKE},
        .ability = ABILITY_SHIELDS_DOWN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MINIOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WHITE_HERB, // Shell Smash, White Herb restores drops
        .moves = {MOVE_SHELL_SMASH, MOVE_POWER_GEM, MOVE_DAZZLING_GLEAM, MOVE_ACROBATICS},
        .ability = ABILITY_SHIELDS_DOWN,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 128, 0, 252, 128, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Komala ----
    {
        .species = SPECIES_KOMALA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Comatose status-immune attacker
        .moves = {MOVE_RETURN, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE, MOVE_SUPERPOWER},
        .ability = ABILITY_COMATOSE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },

    // ---- Turtonator ----
    {
        .species = SPECIES_TURTONATOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shell Trap defensive attacker
        .moves = {MOVE_SHELL_TRAP, MOVE_FLAMETHROWER, MOVE_DRAGON_PULSE, MOVE_BODY_PRESS},
        .ability = ABILITY_SHELL_ARMOR,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TURTONATOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Fire/Dragon special attacker
        .moves = {MOVE_FIRE_BLAST, MOVE_DRACO_METEOR, MOVE_EARTH_POWER, MOVE_FLASH_CANNON},
        .ability = ABILITY_SHELL_ARMOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Togedemaru ----
    {
        .species = SPECIES_TOGEDEMARU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Iron Barbs / Lightning Rod attacker
        .moves = {MOVE_ZING_ZAP, MOVE_IRON_HEAD, MOVE_U_TURN, MOVE_NUZZLE},
        .ability = ABILITY_IRON_BARBS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOGEDEMARU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Lightning Rod redirect support
        .moves = {MOVE_FAKE_OUT, MOVE_ZING_ZAP, MOVE_SPIKY_SHIELD, MOVE_ENCORE},
        .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Mimikyu ----
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MIMIKIUM_Z, // Let's Snuggle Forever nuke, Disguise
        .moves = {MOVE_SWORDS_DANCE, MOVE_PLAY_ROUGH, MOVE_SHADOW_CLAW, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_DISGUISE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Disguise sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_PLAY_ROUGH, MOVE_SHADOW_SNEAK, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_DISGUISE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MIMIKYU,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LUM_BERRY, // Disguise disruptor
        .moves = {MOVE_PLAY_ROUGH, MOVE_SHADOW_SNEAK, MOVE_WILL_O_WISP, MOVE_TAUNT},
        .ability = ABILITY_DISGUISE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Bruxish ----
    {
        .species = SPECIES_BRUXISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Strong Jaw attacker
        .moves = {MOVE_PSYCHIC_FANGS, MOVE_LIQUIDATION, MOVE_CRUNCH, MOVE_ICE_FANG},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRUXISH,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_CHOICE_SCARF, // Wonder Skin / disruption pivot
        .moves = {MOVE_PSYCHIC_FANGS, MOVE_LIQUIDATION, MOVE_FLIP_TURN, MOVE_ICE_FANG},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Drampa ----
    {
        .species = SPECIES_DRAMPA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Berserk special breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_HYPER_VOICE, MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN},
        .ability = ABILITY_BERSERK,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAMPA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Berserk Roost staller
        .moves = {MOVE_HYPER_VOICE, MOVE_FLAMETHROWER, MOVE_ROOST, MOVE_GLARE},
        .ability = ABILITY_BERSERK,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Dhelmise ----
    {
        .species = SPECIES_DHELMISE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Steelworker Anchor Shot band
        .moves = {MOVE_ANCHOR_SHOT, MOVE_POWER_WHIP, MOVE_EARTHQUAKE, MOVE_SHADOW_CLAW},
        .ability = ABILITY_STEELWORKER,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(248, 252, 0, 0, 0, 8),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DHELMISE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room trapper
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves = {MOVE_ANCHOR_SHOT, MOVE_POWER_WHIP, MOVE_SHADOW_CLAW, MOVE_EARTHQUAKE},
        .ability = ABILITY_STEELWORKER,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Kommo-o ----
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_KOMMONIUM_Z, // Clangorous Soulblaze setup
        .moves = {MOVE_CLANGING_SCALES, MOVE_CLOSE_COMBAT, MOVE_FLAMETHROWER, MOVE_FLASH_CANNON},
        .ability = ABILITY_SOUNDPROOF,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 4, 0, 252, 252, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up / Body Press setup
        .moves = {MOVE_BULK_UP, MOVE_BODY_PRESS, MOVE_DRAIN_PUNCH, MOVE_IRON_DEFENSE},
        .ability = ABILITY_BULLETPROOF,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KOMMO_O,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE, // Dragon Dance physical sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_SCALE_SHOT, MOVE_CLOSE_COMBAT, MOVE_POISON_JAB},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Tapu Koko ---- (innate Levitate)
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TAPUNIUM_Z, // Guardian of Alola / Electric Surge
        .moves = {MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_NATURES_MADNESS},
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Electric Surge breaker
        .moves = {MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH, MOVE_GRASS_KNOT},
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_KOKO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Electric Terrain sweeper
        .moves = {MOVE_THUNDERBOLT, MOVE_DAZZLING_GLEAM, MOVE_U_TURN, MOVE_CALM_MIND},
        .ability = ABILITY_ELECTRIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Tapu Lele ---- (innate Levitate)
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Psychic Surge breaker
        .moves = {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_PSYSHOCK, MOVE_FOCUS_BLAST},
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PSYCHIC_SEED, // Calm Mind sweeper, terrain-boosted SpD
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_THUNDERBOLT},
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_LELE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_PSYCHIUM_Z, // Shattered Psyche nuke
        .moves = {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_PSYSHOCK, MOVE_THUNDERBOLT},
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Tapu Bulu ---- (innate Levitate)
    {
        .species = SPECIES_TAPU_BULU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Grassy Surge band
        .moves = {MOVE_WOOD_HAMMER, MOVE_HORN_LEECH, MOVE_PLAY_ROUGH, MOVE_SUPERPOWER},
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_BULU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_GRASSY_SEED, // Swords Dance bulky setup
        .moves = {MOVE_SWORDS_DANCE, MOVE_HORN_LEECH, MOVE_PLAY_ROUGH, MOVE_SYNTHESIS},
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Tapu Fini ---- (innate Levitate)
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Misty Surge Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_SURF, MOVE_MOONBLAST, MOVE_TAUNT},
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_MOONBLAST, MOVE_ICE_BEAM, MOVE_SURF},
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TAPU_FINI,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MISTY_SEED, // bulky support, terrain-boosted SpD
        .moves = {MOVE_MUDDY_WATER, MOVE_MOONBLAST, MOVE_HAZE, MOVE_PROTECT},
        .ability = ABILITY_MISTY_SURGE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Solgaleo ----
    {
        .species = SPECIES_SOLGALEO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Full Metal Body physical attacker
        .moves = {MOVE_SUNSTEEL_STRIKE, MOVE_CLOSE_COMBAT, MOVE_EARTHQUAKE, MOVE_FLARE_BLITZ},
        .ability = ABILITY_FULL_METAL_BODY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SOLGALEO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky setup pivot
        .moves = {MOVE_SUNSTEEL_STRIKE, MOVE_MORNING_SUN, MOVE_CALM_MIND, MOVE_FLAMETHROWER},
        .ability = ABILITY_FULL_METAL_BODY,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Lunala ----
    {
        .species = SPECIES_LUNALA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LUNALIUM_Z, // Menacing Moonraze Maelstrom nuke
        .moves = {MOVE_MOONGEIST_BEAM, MOVE_SHADOW_BALL, MOVE_MOONBLAST, MOVE_CALM_MIND},
        .ability = ABILITY_SHADOW_SHIELD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_LUNALA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shadow Shield Calm Mind tank
        .moves = {MOVE_CALM_MIND, MOVE_MOONGEIST_BEAM, MOVE_PSYSHOCK, MOVE_MOONLIGHT},
        .ability = ABILITY_SHADOW_SHIELD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Nihilego ---- (innate Levitate)
    {
        .species = SPECIES_NIHILEGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Beast Boost special breaker
        .moves = {MOVE_SLUDGE_WAVE, MOVE_POWER_GEM, MOVE_THUNDERBOLT, MOVE_GRASS_KNOT},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NIHILEGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // special wall / status spreader
        .moves = {MOVE_SLUDGE_WAVE, MOVE_POWER_GEM, MOVE_TOXIC_SPIKES, MOVE_STEALTH_ROCK},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 0, 252),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Buzzwole ----
    {
        .species = SPECIES_BUZZWOLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Beast Boost physical breaker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_LEECH_LIFE, MOVE_ICE_PUNCH, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BUZZWOLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up bulky setup
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_LEECH_LIFE, MOVE_ICE_PUNCH},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Pheromosa ----
    {
        .species = SPECIES_PHEROMOSA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Beast Boost glass cannon
        .moves = {MOVE_CLOSE_COMBAT, MOVE_TRIPLE_AXEL, MOVE_BUG_BUZZ, MOVE_U_TURN},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PHEROMOSA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // fast lead
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_RAPID_SPIN},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Xurkitree ---- (innate Levitate)
    {
        .species = SPECIES_XURKITREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIUM_Z, // Gigavolt Havoc nuke
        .moves = {MOVE_TAIL_GLOW, MOVE_THUNDERBOLT, MOVE_ENERGY_BALL, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_XURKITREE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Beast Boost revenge killer
        .moves = {MOVE_THUNDERBOLT, MOVE_ENERGY_BALL, MOVE_DAZZLING_GLEAM, MOVE_VOLT_SWITCH},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Celesteela ----
    {
        .species = SPECIES_CELESTEELA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Beast Boost defensive wall
        .moves = {MOVE_LEECH_SEED, MOVE_PROTECT, MOVE_FLAMETHROWER, MOVE_HEAVY_SLAM},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CELESTEELA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Autotomize sweeper
        .moves = {MOVE_AUTOTOMIZE, MOVE_HEAVY_SLAM, MOVE_FLAMETHROWER, MOVE_AIR_SLASH},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 4, 252, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Kartana ---- (innate Levitate)
    {
        .species = SPECIES_KARTANA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Beast Boost physical breaker
        .moves = {MOVE_LEAF_BLADE, MOVE_SACRED_SWORD, MOVE_KNOCK_OFF, MOVE_SMART_STRIKE},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KARTANA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_GRASSIUM_Z, // Bloom Doom Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_SACRED_SWORD, MOVE_SMART_STRIKE},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Guzzlord ----
    {
        .species = SPECIES_GUZZLORD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // massive HP mixed tank
        .moves = {MOVE_KNOCK_OFF, MOVE_DRAGON_TAIL, MOVE_HEAVY_SLAM, MOVE_REST},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GUZZLORD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank attacker
        .moves = {MOVE_DRACO_METEOR, MOVE_DARK_PULSE, MOVE_FLAMETHROWER, MOVE_SLUDGE_BOMB},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Necrozma ---- (innate Levitate)
    {
        .species = SPECIES_NECROZMA_DUSK_MANE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance physical sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_SUNSTEEL_STRIKE, MOVE_EARTHQUAKE, MOVE_PHOTON_GEYSER},
        .ability = ABILITY_PRISM_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NECROZMA_DAWN_WINGS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Calm Mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_MOONGEIST_BEAM, MOVE_PHOTON_GEYSER, MOVE_AURA_SPHERE},
        .ability = ABILITY_PRISM_ARMOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NECROZMA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_PSYCHIUM_Z, // Calm Mind Shattered Psyche
        .moves = {MOVE_CALM_MIND, MOVE_PHOTON_GEYSER, MOVE_HEAT_WAVE, MOVE_MOONLIGHT},
        .ability = ABILITY_PRISM_ARMOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Magearna ---- (innate Levitate)
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Soul-Heart Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_FLEUR_CANNON, MOVE_FLASH_CANNON, MOVE_AURA_SPHERE},
        .ability = ABILITY_SOUL_HEART,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves = {MOVE_FLEUR_CANNON, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH, MOVE_AURA_SPHERE},
        .ability = ABILITY_SOUL_HEART,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAGEARNA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FAIRIUM_Z, // Twinkle Tackle / Trick Room nuke
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .moves = {MOVE_TRICK_ROOM, MOVE_FLEUR_CANNON, MOVE_FLASH_CANNON, MOVE_THUNDERBOLT},
        .ability = ABILITY_SOUL_HEART,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Marshadow ----
    {
        .species = SPECIES_MARSHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_MARSHADIUM_Z, // Soul-Stealing 7-Star Strike nuke
        .moves = {MOVE_SPECTRAL_THIEF, MOVE_CLOSE_COMBAT, MOVE_SHADOW_SNEAK, MOVE_BULK_UP},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MARSHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Technician priority sweeper
        .moves = {MOVE_BULK_UP, MOVE_SPECTRAL_THIEF, MOVE_MACH_PUNCH, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Naganadel ---- (innate Levitate)
    {
        .species = SPECIES_NAGANADEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Beast Boost Nasty Plot sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_SLUDGE_WAVE, MOVE_FIRE_BLAST, MOVE_DRACO_METEOR},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_NAGANADEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_SLUDGE_WAVE, MOVE_DRACO_METEOR, MOVE_FIRE_BLAST, MOVE_U_TURN},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Stakataka ----
    {
        .species = SPECIES_STAKATAKA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_WEAKNESS_POLICY, // Trick Room Beast Boost wallbreaker
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .moves = {MOVE_GYRO_BALL, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_TRICK_ROOM},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_STAKATAKA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // physical wall / hazard setter
        .moves = {MOVE_STEALTH_ROCK, MOVE_GYRO_BALL, MOVE_BODY_PRESS, MOVE_TRICK_ROOM},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Blacephalon ---- (innate Levitate)
    {
        .species = SPECIES_BLACEPHALON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Beast Boost revenge killer
        .moves = {MOVE_SHADOW_BALL, MOVE_FIRE_BLAST, MOVE_PSYCHIC, MOVE_TRICK},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BLACEPHALON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FIRIUM_Z, // Inferno Overdrive nuke
        .moves = {MOVE_CALM_MIND, MOVE_FIRE_BLAST, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST},
        .ability = ABILITY_BEAST_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Zeraora ----
    {
        .species = SPECIES_ZERAORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Volt Absorb fast physical attacker
        .moves = {MOVE_PLASMA_FISTS, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_PLAY_ROUGH},
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZERAORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ELECTRIUM_Z, // Gigavolt Havoc + Bulk Up
        .moves = {MOVE_BULK_UP, MOVE_PLASMA_FISTS, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF},
        .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation VIII
    // ============================================================

    // ---- Rillaboom ----
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Grassy Surge band breaker
        .moves = {MOVE_GRASSY_GLIDE, MOVE_WOOD_HAMMER, MOVE_KNOCK_OFF, MOVE_U_TURN},
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance terrain sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_GRASSY_GLIDE, MOVE_HIGH_HORSEPOWER, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RILLABOOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Grassy Terrain pivot
        .moves = {MOVE_GRASSY_GLIDE, MOVE_KNOCK_OFF, MOVE_SYNTHESIS, MOVE_U_TURN},
        .ability = ABILITY_GRASSY_SURGE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 200, 0, 0, 56),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Cinderace ----
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Libero offensive pivot
        .moves = {MOVE_PYRO_BALL, MOVE_HIGH_HORSEPOWER, MOVE_U_TURN, MOVE_GUNK_SHOT},
        .ability = ABILITY_LIBERO,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Libero band breaker
        .moves = {MOVE_PYRO_BALL, MOVE_HIGH_HORSEPOWER, MOVE_ZEN_HEADBUTT, MOVE_U_TURN},
        .ability = ABILITY_LIBERO,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CINDERACE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Court Change utility sweeper
        .moves = {MOVE_PYRO_BALL, MOVE_COURT_CHANGE, MOVE_HIGH_HORSEPOWER, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_LIBERO,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Inteleon ----
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Sniper special breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_DARK_PULSE, MOVE_U_TURN},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_SCOPE_LENS, // Sniper guaranteed-crit Snipe Shot
        .moves = {MOVE_SNIPE_SHOT, MOVE_ICE_BEAM, MOVE_DARK_PULSE, MOVE_AIR_SLASH},
        .ability = ABILITY_SNIPER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INTELEON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // revenge killer
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_U_TURN, MOVE_DARK_PULSE},
        .ability = ABILITY_TORRENT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Corviknight ----
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Pressure Defog wall
        .moves = {MOVE_BODY_PRESS, MOVE_ROOST, MOVE_DEFOG, MOVE_IRON_DEFENSE},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 168, 0, 0, 88),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Mirror Armor physical wall
        .moves = {MOVE_BRAVE_BIRD, MOVE_BODY_PRESS, MOVE_ROOST, MOVE_U_TURN},
        .ability = ABILITY_MIRROR_ARMOR,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CORVIKNIGHT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky offensive pivot
        .moves = {MOVE_BRAVE_BIRD, MOVE_BULK_UP, MOVE_ROOST, MOVE_U_TURN},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Coalossal ----
    {
        .species = SPECIES_COALOSSAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Steam Engine bulky hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_RAPID_SPIN, MOVE_FLAMETHROWER, MOVE_STONE_EDGE},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_COALOSSAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Steam Engine sweeper
        .moves = {MOVE_FLAMETHROWER, MOVE_STONE_EDGE, MOVE_EARTH_POWER, MOVE_HEAT_CRASH},
        .ability = ABILITY_STEAM_ENGINE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Flapple ----
    {
        .species = SPECIES_FLAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Hustle physical attacker
        .moves = {MOVE_GRAV_APPLE, MOVE_DRAGON_RUSH, MOVE_U_TURN, MOVE_OUTRAGE},
        .ability = ABILITY_HUSTLE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_APPLE_ACID, MOVE_DRACO_METEOR, MOVE_FIRE_BLAST, MOVE_U_TURN},
        .ability = ABILITY_HUSTLE,
        .nature = NATURE_NAIVE,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Appletun ----
    {
        .species = SPECIES_APPLETUN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Thick Fat bulky special wall
        .moves = {MOVE_APPLE_ACID, MOVE_DRAGON_PULSE, MOVE_RECOVER, MOVE_LEECH_SEED},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(248, 0, 8, 0, 0, 252),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_APPLETUN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_APPLE_ACID, MOVE_DRACO_METEOR, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER},
        .ability = ABILITY_THICK_FAT,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Sandaconda ----
    {
        .species = SPECIES_SANDACONDA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Sand Spit Coil wall
        .moves = {MOVE_COIL, MOVE_EARTHQUAKE, MOVE_STONE_EDGE, MOVE_GLARE},
        .ability = ABILITY_SHED_SKIN,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SANDACONDA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // bulky hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_GLARE, MOVE_RAPID_SPIN},
        .ability = ABILITY_SAND_SPIT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Barraskewda ----
    {
        .species = SPECIES_BARRASKEWDA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Swift Swim band breaker
        .moves = {MOVE_LIQUIDATION, MOVE_CLOSE_COMBAT, MOVE_PSYCHIC_FANGS, MOVE_FLIP_TURN},
        .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BARRASKEWDA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // fast revenge killer
        .moves = {MOVE_LIQUIDATION, MOVE_CLOSE_COMBAT, MOVE_AQUA_JET, MOVE_FLIP_TURN},
        .ability = ABILITY_PROPELLER_TAIL,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Toxtricity ----
    {
        .species = SPECIES_TOXTRICITY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY, // Punk Rock Boomburst nuke (Amped)
        .moves = {MOVE_BOOMBURST, MOVE_OVERDRIVE, MOVE_SLUDGE_WAVE, MOVE_VOLT_SWITCH},
        .ability = ABILITY_PUNK_ROCK,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOXTRICITY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Punk Rock special breaker
        .moves = {MOVE_OVERDRIVE, MOVE_SLUDGE_WAVE, MOVE_VOLT_SWITCH, MOVE_FOCUS_BLAST},
        .ability = ABILITY_PUNK_ROCK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOXTRICITY_LOW_KEY,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Low Key revenge killer
        .moves = {MOVE_OVERDRIVE, MOVE_SLUDGE_WAVE, MOVE_VOLT_SWITCH, MOVE_BOOMBURST},
        .ability = ABILITY_PUNK_ROCK,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Centiskorch ----
    {
        .species = SPECIES_CENTISKORCH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Flash Fire bulky attacker
        .moves = {MOVE_FIERY_DANCE, MOVE_OVERHEAT, MOVE_POWER_WHIP, MOVE_KNOCK_OFF},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CENTISKORCH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY, // Fiery Dance / Coil setup
        .moves = {MOVE_COIL, MOVE_FLARE_BLITZ, MOVE_POWER_WHIP, MOVE_KNOCK_OFF},
        .ability = ABILITY_WHITE_SMOKE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Polteageist ---- (innate Levitate)
    {
        .species = SPECIES_POLTEAGEIST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Shell Smash Stored Power sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_STORED_POWER, MOVE_SHADOW_BALL, MOVE_GIGA_DRAIN},
        .ability = ABILITY_CURSED_BODY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 4, 252, 252, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_POLTEAGEIST,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FOCUS_SASH, // Shell Smash sash sweeper
        .moves = {MOVE_SHELL_SMASH, MOVE_SHADOW_BALL, MOVE_GIGA_DRAIN, MOVE_STORED_POWER},
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Hatterene ---- (innate Magic Bounce)
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Magic Bounce Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_PSYSHOCK, MOVE_DAZZLING_GLEAM, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Psychic Terrain breaker
        .moves = {MOVE_EXPANDING_FORCE, MOVE_DAZZLING_GLEAM, MOVE_MYSTICAL_FIRE, MOVE_PROTECT},
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HATTERENE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_MYSTICAL_FIRE, MOVE_POWER_WHIP},
        .ability = ABILITY_MAGIC_BOUNCE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Grimmsnarl ----
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIGHT_CLAY, // Prankster dual screens lead
        .moves = {MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_SPIRIT_BREAK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Bulk Up physical sweeper
        .moves = {MOVE_BULK_UP, MOVE_SPIRIT_BREAK, MOVE_SUCKER_PUNCH, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GRIMMSNARL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_MENTAL_HERB, // Prankster support / Taunt-proof
        .moves = {MOVE_SPIRIT_BREAK, MOVE_THUNDER_WAVE, MOVE_TAUNT, MOVE_PARTING_SHOT},
        .ability = ABILITY_PRANKSTER,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 100, 0, 0, 156),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Obstagoon ----
    {
        .species = SPECIES_OBSTAGOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts Facade breaker
        .moves = {MOVE_FACADE, MOVE_KNOCK_OFF, MOVE_CLOSE_COMBAT, MOVE_OBSTRUCT},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_OBSTAGOON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up Guts wall-breaker
        .moves = {MOVE_BULK_UP, MOVE_FACADE, MOVE_KNOCK_OFF, MOVE_OBSTRUCT},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Perrserker ----
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Steely Spirit band breaker
        .moves = {MOVE_IRON_HEAD, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_U_TURN},
        .ability = ABILITY_STEELY_SPIRIT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PERRSERKER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Swords Dance
        .moves = {MOVE_SWORDS_DANCE, MOVE_IRON_HEAD, MOVE_CLOSE_COMBAT, MOVE_STEALTH_ROCK},
        .ability = ABILITY_TOUGH_CLAWS,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Cursola ---- (innate Perish Body)
    {
        .species = SPECIES_CURSOLA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Perish Body bulky special attacker
        .moves = {MOVE_CALM_MIND, MOVE_SHADOW_BALL, MOVE_ICE_BEAM, MOVE_STRENGTH_SAP},
        .ability = ABILITY_PERISH_BODY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CURSOLA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // glass cannon
        .moves = {MOVE_SHADOW_BALL, MOVE_ICE_BEAM, MOVE_EARTH_POWER, MOVE_GIGA_DRAIN},
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Sirfetch'd ----
    {
        .species = SPECIES_SIRFETCHD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEEK, // guaranteed-crit Leaf Blade Scrappy fighter
        .moves = {MOVE_CLOSE_COMBAT, MOVE_LEAF_BLADE, MOVE_KNOCK_OFF, MOVE_FIRST_IMPRESSION},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SIRFETCHD,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_BRAVE_BIRD},
        .ability = ABILITY_SCRAPPY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Runerigus ---- (innate Levitate)
    {
        .species = SPECIES_RUNERIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // Wandering Spirit bulky hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_BODY_PRESS, MOVE_WILL_O_WISP},
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RUNERIGUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Iron Defense Body Press wall
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_EARTHQUAKE, MOVE_POLTERGEIST},
        .ability = ABILITY_WANDERING_SPIRIT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Falinks ----
    {
        .species = SPECIES_FALINKS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // No Retreat setup sweeper
        .moves = {MOVE_NO_RETREAT, MOVE_CLOSE_COMBAT, MOVE_IRON_HEAD, MOVE_ROCK_SLIDE},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FALINKS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_WHITE_HERB, // No Retreat sweeper with herb reset
        .moves = {MOVE_NO_RETREAT, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_THROAT_CHOP},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Frosmoth ----
    {
        .species = SPECIES_FROSMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Quiver Dance sweeper
        .moves = {MOVE_QUIVER_DANCE, MOVE_ICE_BEAM, MOVE_BUG_BUZZ, MOVE_GIGA_DRAIN},
        .ability = ABILITY_ICE_SCALES,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FROSMOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Ice Scales special wall
        .moves = {MOVE_QUIVER_DANCE, MOVE_ICE_BEAM, MOVE_HURRICANE, MOVE_SUBSTITUTE},
        .ability = ABILITY_ICE_SCALES,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 100, 156, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Eiscue ----
    {
        .species = SPECIES_EISCUE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Ice Face Belly Drum sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_LIQUIDATION, MOVE_ICICLE_CRASH, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_ICE_FACE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_EISCUE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Ice Face band attacker
        .moves = {MOVE_ICICLE_CRASH, MOVE_LIQUIDATION, MOVE_HEAD_SMASH, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_ICE_FACE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Indeedee ----
    {
        .species = SPECIES_INDEEDEE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Psychic Surge attacker
        .moves = {MOVE_EXPANDING_FORCE, MOVE_DAZZLING_GLEAM, MOVE_PSYCHIC_TERRAIN, MOVE_PROTECT},
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 4, 0, 252, 252),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_INDEEDEE_F,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LEFTOVERS, // Healer redirect support (Indeedee-F)
        .moves = {MOVE_FOLLOW_ME, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM, MOVE_HEAL_PULSE},
        .ability = ABILITY_PSYCHIC_SURGE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Morpeko ----
    {
        .species = SPECIES_MORPEKO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Hunger Switch Aura Wheel attacker
        .moves = {MOVE_AURA_WHEEL, MOVE_KNOCK_OFF, MOVE_PSYCHIC_FANGS, MOVE_PROTECT},
        .ability = ABILITY_HUNGER_SWITCH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Copperajah ----
    {
        .species = SPECIES_COPPERAJAH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sheer Force Heavy Slam breaker
        .moves = {MOVE_HEAVY_SLAM, MOVE_HIGH_HORSEPOWER, MOVE_PLAY_ROUGH, MOVE_ROCK_SLIDE},
        .ability = ABILITY_SHEER_FORCE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_COPPERAJAH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky hazard setter
        .moves = {MOVE_STEALTH_ROCK, MOVE_HEAVY_SLAM, MOVE_HIGH_HORSEPOWER, MOVE_WHIRLWIND},
        .ability = ABILITY_HEAVY_METAL,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Dracozolt ----
    {
        .species = SPECIES_DRACOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Hustle Bolt Beak nuke
        .moves = {MOVE_BOLT_BEAK, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_FIRE_PUNCH},
        .ability = ABILITY_HUSTLE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRACOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Sand Rush sweeper
        .moves = {MOVE_BOLT_BEAK, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Arctozolt ----
    {
        .species = SPECIES_ARCTOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Slush Rush Bolt Beak nuke
        .moves = {MOVE_BOLT_BEAK, MOVE_ICICLE_CRASH, MOVE_LOW_KICK, MOVE_BLIZZARD},
        .ability = ABILITY_SLUSH_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCTOZOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Static special breaker
        .moves = {MOVE_BOLT_BEAK, MOVE_FREEZE_DRY, MOVE_THUNDERBOLT, MOVE_FLASH_CANNON},
        .ability = ABILITY_STATIC,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Dracovish ----
    {
        .species = SPECIES_DRACOVISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Strong Jaw Fishious Rend nuke
        .moves = {MOVE_FISHIOUS_REND, MOVE_CRUNCH, MOVE_PSYCHIC_FANGS, MOVE_ICE_FANG},
        .ability = ABILITY_STRONG_JAW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRACOVISH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sand Rush band breaker
        .moves = {MOVE_FISHIOUS_REND, MOVE_CRUNCH, MOVE_EARTHQUAKE, MOVE_ICE_FANG},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Arctovish ----
    {
        .species = SPECIES_ARCTOVISH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_BAND, // Slush Rush Fishious Rend
        .moves = {MOVE_FISHIOUS_REND, MOVE_ICICLE_CRASH, MOVE_PSYCHIC_FANGS, MOVE_CRUNCH},
        .ability = ABILITY_SLUSH_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCTOVISH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Water Absorb bulky pivot
        .moves = {MOVE_FREEZE_DRY, MOVE_FLIP_TURN, MOVE_BODY_PRESS, MOVE_RECOVER},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_RELAXED,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Duraludon ----
    {
        .species = SPECIES_DURALUDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Light Metal special breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_THUNDERBOLT, MOVE_BODY_PRESS},
        .ability = ABILITY_LIGHT_METAL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DURALUDON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Stealth Rock bulky setup
        .moves = {MOVE_STEALTH_ROCK, MOVE_FLASH_CANNON, MOVE_DRACO_METEOR, MOVE_BODY_PRESS},
        .ability = ABILITY_STALWART,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Dragapult ---- (innate Levitate)
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Draco/Shadow Ball breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_SHADOW_BALL, MOVE_FLAMETHROWER, MOVE_U_TURN},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Dragon Darts band breaker
        .moves = {MOVE_DRAGON_DARTS, MOVE_PHANTOM_FORCE, MOVE_U_TURN, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DRAGAPULT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Dragon Dance physical sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_DRAGON_DARTS, MOVE_PHANTOM_FORCE, MOVE_FIRE_BLAST},
        .ability = ABILITY_INFILTRATOR,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Zacian (Crowned) ----
    {
        .species = SPECIES_ZACIAN_CROWNED,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_RUSTED_SWORD, // Intrepid Sword Behemoth Blade
        .moves = {MOVE_BEHEMOTH_BLADE, MOVE_PLAY_ROUGH, MOVE_CLOSE_COMBAT, MOVE_SWORDS_DANCE},
        .ability = ABILITY_INTREPID_SWORD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZACIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // base Zacian band breaker
        .moves = {MOVE_PLAY_ROUGH, MOVE_CLOSE_COMBAT, MOVE_CRUNCH, MOVE_WILD_CHARGE},
        .ability = ABILITY_INTREPID_SWORD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Zamazenta (Crowned) ----
    {
        .species = SPECIES_ZAMAZENTA_CROWNED,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_RUSTED_SHIELD, // Dauntless Shield Body Press wall
        .moves = {MOVE_BEHEMOTH_BASH, MOVE_BODY_PRESS, MOVE_IRON_DEFENSE, MOVE_CRUNCH},
        .ability = ABILITY_DAUNTLESS_SHIELD,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 144, 0, 0, 112),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ZAMAZENTA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // base Zamazenta bulky setup
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_CLOSE_COMBAT, MOVE_CRUNCH},
        .ability = ABILITY_DAUNTLESS_SHIELD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Eternatus ----
    {
        .species = SPECIES_ETERNATUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Pressure special breaker
        .moves = {MOVE_DYNAMAX_CANNON, MOVE_SLUDGE_WAVE, MOVE_FLAMETHROWER, MOVE_DRACO_METEOR},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ETERNATUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // bulky Toxic Spikes / Cosmic Power
        .moves = {MOVE_DYNAMAX_CANNON, MOVE_FLAMETHROWER, MOVE_TOXIC_SPIKES, MOVE_RECOVER},
        .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 100, 156, 0),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Urshifu (Single Strike) ----
    {
        .species = SPECIES_URSHIFU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Unseen Fist Wicked Blow breaker
        .moves = {MOVE_WICKED_BLOW, MOVE_CLOSE_COMBAT, MOVE_U_TURN, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_UNSEEN_FIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_URSHIFU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_WICKED_BLOW, MOVE_CLOSE_COMBAT, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_UNSEEN_FIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Urshifu (Rapid Strike) ----
    {
        .species = SPECIES_URSHIFU_RAPID_STRIKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Surging Strikes always-crit breaker
        .moves = {MOVE_SURGING_STRIKES, MOVE_CLOSE_COMBAT, MOVE_AQUA_JET, MOVE_U_TURN},
        .ability = ABILITY_UNSEEN_FIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_URSHIFU_RAPID_STRIKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_SURGING_STRIKES, MOVE_CLOSE_COMBAT, MOVE_AQUA_JET},
        .ability = ABILITY_UNSEEN_FIST,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Regieleki ---- (innate Transistor / Levitate-tier speed)
    {
        .species = SPECIES_REGIELEKI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Transistor electric nuke
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_RISING_VOLTAGE, MOVE_TERA_BLAST},
        .ability = ABILITY_TRANSISTOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REGIELEKI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIGHT_CLAY, // screens lead
        .moves = {MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_THUNDERBOLT, MOVE_EXPLOSION},
        .ability = ABILITY_TRANSISTOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },

    // ---- Regidrago ----
    {
        .species = SPECIES_REGIDRAGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Dragon's Maw Dragon Energy breaker
        .moves = {MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_OUTRAGE, MOVE_FIRE_FANG},
        .ability = ABILITY_DRAGONS_MAW,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REGIDRAGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special Dragon Energy breaker
        .moves = {MOVE_DRACO_METEOR, MOVE_DRAGON_PULSE, MOVE_THUNDERBOLT, MOVE_EARTH_POWER},
        .ability = ABILITY_DRAGONS_MAW,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Glastrier ----
    {
        .species = SPECIES_GLASTRIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Chilling Neigh band breaker
        .moves = {MOVE_ICICLE_CRASH, MOVE_HIGH_HORSEPOWER, MOVE_CLOSE_COMBAT, MOVE_HEAVY_SLAM},
        .ability = ABILITY_CHILLING_NEIGH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GLASTRIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Swords Dance
        .moves = {MOVE_SWORDS_DANCE, MOVE_ICICLE_CRASH, MOVE_HIGH_HORSEPOWER, MOVE_BODY_PRESS},
        .ability = ABILITY_CHILLING_NEIGH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Spectrier ---- (innate Levitate-tier)
    {
        .species = SPECIES_SPECTRIER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Grim Neigh special breaker
        .moves = {MOVE_SHADOW_BALL, MOVE_DARK_PULSE, MOVE_MYSTICAL_FIRE, MOVE_DRAINING_KISS},
        .ability = ABILITY_GRIM_NEIGH,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SPECTRIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Nasty Plot snowball sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_SHADOW_BALL, MOVE_MYSTICAL_FIRE, MOVE_SUBSTITUTE},
        .ability = ABILITY_GRIM_NEIGH,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Calyrex (Ice Rider) ----
    {
        .species = SPECIES_CALYREX_ICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // As One Glacial Lance breaker
        .moves = {MOVE_GLACIAL_LANCE, MOVE_HIGH_HORSEPOWER, MOVE_TRICK_ROOM, MOVE_LEECH_SEED},
        .ability = ABILITY_AS_ONE_ICE_RIDER,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CALYREX_ICE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Chilling Neigh band breaker
        .moves = {MOVE_GLACIAL_LANCE, MOVE_HIGH_HORSEPOWER, MOVE_CLOSE_COMBAT, MOVE_SEED_BOMB},
        .ability = ABILITY_AS_ONE_ICE_RIDER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Calyrex (Shadow Rider) ----
    {
        .species = SPECIES_CALYREX_SHADOW,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Grim Neigh Astral Barrage nuke
        .moves = {MOVE_ASTRAL_BARRAGE, MOVE_PSYSHOCK, MOVE_GIGA_DRAIN, MOVE_DRACO_METEOR},
        .ability = ABILITY_AS_ONE_SHADOW_RIDER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CALYREX_SHADOW,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot Substitute sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_ASTRAL_BARRAGE, MOVE_GIGA_DRAIN, MOVE_SUBSTITUTE},
        .ability = ABILITY_AS_ONE_SHADOW_RIDER,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Wyrdeer ----
    {
        .species = SPECIES_WYRDEER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sap Sipper band attacker
        .moves = {MOVE_PSYCHIC_FANGS, MOVE_MEGAHORN, MOVE_BODY_SLAM, MOVE_THROAT_CHOP},
        .ability = ABILITY_SAP_SIPPER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WYRDEER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // Intimidate special tank
        .moves = {MOVE_PSYCHIC, MOVE_HYPER_VOICE, MOVE_SHADOW_BALL, MOVE_EARTH_POWER},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Kleavor ----
    {
        .species = SPECIES_KLEAVOR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sharpness Stone Axe breaker
        .moves = {MOVE_STONE_AXE, MOVE_X_SCISSOR, MOVE_CLOSE_COMBAT, MOVE_U_TURN},
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KLEAVOR,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Stone Axe hazard lead
        .moves = {MOVE_STONE_AXE, MOVE_X_SCISSOR, MOVE_CLOSE_COMBAT, MOVE_DEFOG},
        .ability = ABILITY_SHARPNESS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_BUG,
        .ball = BALL_POKE,
    },

    // ---- Ursaluna ----
    {
        .species = SPECIES_URSALUNA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_FLAME_ORB, // Guts Facade wallbreaker
        .moves = {MOVE_FACADE, MOVE_HEADLONG_RUSH, MOVE_CRUNCH, MOVE_FIRE_PUNCH},
        .ability = ABILITY_GUTS,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 4, 0, 0, 0),
        .iv = TRAINER_PARTY_IVS(31, 31, 31, 0, 31, 31),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_URSALUNA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Swords Dance Bulk Up bruiser
        .moves = {MOVE_SWORDS_DANCE, MOVE_HEADLONG_RUSH, MOVE_CRUNCH, MOVE_PROTECT},
        .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Basculegion ----
    {
        .species = SPECIES_BASCULEGION,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Adaptability Wave Crash breaker
        .moves = {MOVE_WAVE_CRASH, MOVE_PHANTOM_FORCE, MOVE_AQUA_JET, MOVE_FLIP_TURN},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BASCULEGION_F,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special Basculegion-F breaker
        .moves = {MOVE_HYDRO_PUMP, MOVE_SHADOW_BALL, MOVE_ICE_BEAM, MOVE_FLIP_TURN},
        .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Sneasler ----
    {
        .species = SPECIES_SNEASLER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_TOXIC_ORB, // Poison Touch Dire Claw attacker
        .moves = {MOVE_DIRE_CLAW, MOVE_CLOSE_COMBAT, MOVE_FAKE_OUT, MOVE_THROAT_CHOP},
        .ability = ABILITY_POISON_TOUCH,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SNEASLER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance Unburden-style sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_DIRE_CLAW, MOVE_ACROBATICS},
        .ability = ABILITY_UNBURDEN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Overqwil ----
    {
        .species = SPECIES_OVERQWIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Intimidate Toxic Spikes pivot
        .moves = {MOVE_BARB_BARRAGE, MOVE_KNOCK_OFF, MOVE_TOXIC_SPIKES, MOVE_DESTINY_BOND},
        .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_OVERQWIL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_BARB_BARRAGE, MOVE_KNOCK_OFF, MOVE_AQUA_JET},
        .ability = ABILITY_POISON_POINT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 4, 252, 0, 0),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Enamorus ----
    {
        .species = SPECIES_ENAMORUS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Cute Charm special attacker (Incarnate)
        .moves = {MOVE_MOONBLAST, MOVE_EARTH_POWER, MOVE_SLUDGE_BOMB, MOVE_MYSTICAL_FIRE},
        .ability = ABILITY_CUTE_CHARM,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ENAMORUS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Calm Mind setup
        .moves = {MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_EARTH_POWER, MOVE_SUBSTITUTE},
        .ability = ABILITY_CONTRARY,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ENAMORUS_THERIAN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Overcoat Therian physical breaker
        .moves = {MOVE_PLAY_ROUGH, MOVE_EARTHQUAKE, MOVE_SPRINGTIDE_STORM, MOVE_U_TURN},
        .ability = ABILITY_OVERCOAT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ============================================================
    //                       Generation IX
    // ============================================================

    // ---- Meowscarada ----
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Protean physical attacker
        .moves = {MOVE_FLOWER_TRICK, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_PLAY_ROUGH},
        .ability = ABILITY_PROTEAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // hit-and-run wallbreaker
        .moves = {MOVE_FLOWER_TRICK, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_TRIPLE_AXEL},
        .ability = ABILITY_PROTEAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MEOWSCARADA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // fast lead / spike support
        .moves = {MOVE_SPIKES, MOVE_FLOWER_TRICK, MOVE_KNOCK_OFF, MOVE_TAUNT},
        .ability = ABILITY_OVERGROW,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Skeledirge ----
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Torch Song bulky setup pivot
        .moves = {MOVE_TORCH_SONG, MOVE_SHADOW_BALL, MOVE_SLACK_OFF, MOVE_WILL_O_WISP},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Unaware special wall
        .moves = {MOVE_TORCH_SONG, MOVE_HEX, MOVE_SLACK_OFF, MOVE_WILL_O_WISP},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SKELEDIRGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_THROAT_SPRAY, // Torch Song snowball sweeper
        .moves = {MOVE_TORCH_SONG, MOVE_SHADOW_BALL, MOVE_EARTH_POWER, MOVE_SLACK_OFF},
        .ability = ABILITY_BLAZE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Quaquaval ----
    {
        .species = SPECIES_QUAQUAVAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Aqua Step setup sweeper
        .moves = {MOVE_AQUA_STEP, MOVE_CLOSE_COMBAT, MOVE_ICE_SPINNER, MOVE_AQUA_JET},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_QUAQUAVAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Moxie band breaker
        .moves = {MOVE_AQUA_STEP, MOVE_CLOSE_COMBAT, MOVE_TRIPLE_AXEL, MOVE_U_TURN},
        .ability = ABILITY_MOXIE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Maushold ----
    {
        .species = SPECIES_MAUSHOLD,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WIDE_LENS, // Technician Population Bomb sweeper
        .moves = {MOVE_POPULATION_BOMB, MOVE_BULLET_SEED, MOVE_TIDY_UP, MOVE_ENCORE},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MAUSHOLD,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // Friend Guard support lead
        .moves = {MOVE_FOLLOW_ME, MOVE_BEAT_UP, MOVE_HELPING_HAND, MOVE_PROTECT},
        .ability = ABILITY_FRIEND_GUARD,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Garganacl ----
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Purifying Salt physical wall
        .moves = {MOVE_SALT_CURE, MOVE_RECOVER, MOVE_STEALTH_ROCK, MOVE_BODY_PRESS},
        .ability = ABILITY_PURIFYING_SALT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Iron Defense + Body Press sweeper
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_SALT_CURE, MOVE_RECOVER},
        .ability = ABILITY_PURIFYING_SALT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GARGANACL,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ROCKY_HELMET, // doubles Salt Cure chip + Wide Guard
        .moves = {MOVE_SALT_CURE, MOVE_WIDE_GUARD, MOVE_RECOVER, MOVE_EARTHQUAKE},
        .ability = ABILITY_PURIFYING_SALT,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Armarouge ----
    {
        .species = SPECIES_ARMAROUGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WEAKNESS_POLICY, // Armor Cannon special sweeper
        .moves = {MOVE_ARMOR_CANNON, MOVE_PSYSHOCK, MOVE_AURA_SPHERE, MOVE_CALM_MIND},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARMAROUGE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // Expanding Force Trick Room attacker
        .moves = {MOVE_EXPANDING_FORCE, MOVE_ARMOR_CANNON, MOVE_TRICK_ROOM, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_PSYCHIC,
        .ball = BALL_POKE,
    },

    // ---- Ceruledge ----
    {
        .species = SPECIES_CERULEDGE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Bitter Blade Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_BITTER_BLADE, MOVE_SHADOW_SNEAK, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_WEAK_ARMOR,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CERULEDGE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Bulk Up bulky setup
        .moves = {MOVE_BULK_UP, MOVE_BITTER_BLADE, MOVE_SHADOW_SNEAK, MOVE_WILL_O_WISP},
        .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Brambleghast ----
    {
        .species = SPECIES_BRAMBLEGHAST,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Wind Rider spin / hazard control
        .moves = {MOVE_POWER_WHIP, MOVE_SHADOW_BALL, MOVE_RAPID_SPIN, MOVE_LEECH_SEED},
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BRAMBLEGHAST,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // physical breaker
        .moves = {MOVE_POWER_WHIP, MOVE_POLTERGEIST, MOVE_RAPID_SPIN, MOVE_INFERNAL_PARADE},
        .ability = ABILITY_WIND_RIDER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Toedscruel ----
    {
        .species = SPECIES_TOEDSCRUEL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Mycelium Might hazard control
        .moves = {MOVE_SPORE, MOVE_RAPID_SPIN, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER},
        .ability = ABILITY_MYCELIUM_MIGHT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TOEDSCRUEL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // fast special attacker
        .moves = {MOVE_ENERGY_BALL, MOVE_EARTH_POWER, MOVE_SLUDGE_BOMB, MOVE_RAPID_SPIN},
        .ability = ABILITY_MYCELIUM_MIGHT,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Scovillain ----
    {
        .species = SPECIES_SCOVILLAIN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Chlorophyll mixed sun attacker
        .moves = {MOVE_GROWTH, MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER},
        .ability = ABILITY_CHLOROPHYLL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Espathra ----
    {
        .species = SPECIES_ESPATHRA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Opportunist Calm Mind sweeper
        .moves = {MOVE_CALM_MIND, MOVE_STORED_POWER, MOVE_DAZZLING_GLEAM, MOVE_ROOST},
        .ability = ABILITY_OPPORTUNIST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ESPATHRA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // fast special breaker
        .moves = {MOVE_PSYSHOCK, MOVE_DAZZLING_GLEAM, MOVE_SHADOW_BALL, MOVE_TERA_BLAST},
        .ability = ABILITY_SPEED_BOOST,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Tinkaton ----
    {
        .species = SPECIES_TINKATON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_AIR_BALLOON, // Gigaton Hammer + hazards utility
        .moves = {MOVE_GIGATON_HAMMER, MOVE_PLAY_ROUGH, MOVE_STEALTH_ROCK, MOVE_THUNDER_WAVE},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TINKATON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Gigaton Hammer band breaker
        .moves = {MOVE_GIGATON_HAMMER, MOVE_PLAY_ROUGH, MOVE_KNOCK_OFF, MOVE_ICE_HAMMER},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Bombirdier ----
    {
        .species = SPECIES_BOMBIRDIER,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Big Pecks hazard / utility pivot
        .moves = {MOVE_STEALTH_ROCK, MOVE_KNOCK_OFF, MOVE_BRAVE_BIRD, MOVE_ROOST},
        .ability = ABILITY_BIG_PECKS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Palafin (Hero) ----
    {
        .species = SPECIES_PALAFIN_HERO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Zero to Hero band breaker
        .moves = {MOVE_JET_PUNCH, MOVE_WAVE_CRASH, MOVE_CLOSE_COMBAT, MOVE_FLIP_TURN},
        .ability = ABILITY_ZERO_TO_HERO,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PALAFIN_HERO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Bulk Up Hero sweeper
        .moves = {MOVE_BULK_UP, MOVE_JET_PUNCH, MOVE_WAVE_CRASH, MOVE_DRAIN_PUNCH},
        .ability = ABILITY_ZERO_TO_HERO,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Revavroom ----
    {
        .species = SPECIES_REVAVROOM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Filter Shift Gear sweeper
        .moves = {MOVE_SHIFT_GEAR, MOVE_GUNK_SHOT, MOVE_IRON_HEAD, MOVE_HIGH_HORSEPOWER},
        .ability = ABILITY_FILTER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_REVAVROOM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ROCKY_HELMET, // bulky pivot / hazard support
        .moves = {MOVE_GUNK_SHOT, MOVE_SPIKES, MOVE_PARTING_SHOT, MOVE_HIGH_HORSEPOWER},
        .ability = ABILITY_FILTER,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Cyclizar ----
    {
        .species = SPECIES_CYCLIZAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Regenerator Shed Tail pivot
        .moves = {MOVE_SHED_TAIL, MOVE_DRAGON_PULSE, MOVE_OVERHEAT, MOVE_RAPID_SPIN},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CYCLIZAR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // fast U-turn pivot
        .moves = {MOVE_DRAGON_CLAW, MOVE_KNOCK_OFF, MOVE_U_TURN, MOVE_RAPID_SPIN},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Orthworm ----
    {
        .species = SPECIES_ORTHWORM,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Earth Eater Iron Defense + Body Press wall
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_SHED_TAIL, MOVE_STEALTH_ROCK},
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ORTHWORM,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ROCKY_HELMET, // Shed Tail pivot
        .moves = {MOVE_SHED_TAIL, MOVE_IRON_HEAD, MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK},
        .ability = ABILITY_EARTH_EATER,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Glimmora ----
    {
        .species = SPECIES_GLIMMORA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_FOCUS_SASH, // Toxic Debris hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_POWER_GEM, MOVE_MORTAL_SPIN},
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GLIMMORA,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // special attacker
        .moves = {MOVE_POWER_GEM, MOVE_SLUDGE_WAVE, MOVE_EARTH_POWER, MOVE_ENERGY_BALL},
        .ability = ABILITY_TOXIC_DEBRIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Houndstone ----
    {
        .species = SPECIES_HOUNDSTONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sand Rush Last Respects sweeper
        .moves = {MOVE_LAST_RESPECTS, MOVE_BODY_PRESS, MOVE_PLAY_ROUGH, MOVE_SHADOW_SNEAK},
        .ability = ABILITY_SAND_RUSH,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Dondozo ----
    {
        .species = SPECIES_DONDOZO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Unaware Curse physical wall
        .moves = {MOVE_CURSE, MOVE_WAVE_CRASH, MOVE_REST, MOVE_SLEEP_TALK},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 4, 252, 0, 0, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DONDOZO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Order Up / bulky pivot
        .moves = {MOVE_WAVE_CRASH, MOVE_BODY_PRESS, MOVE_EARTHQUAKE, MOVE_REST},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Annihilape ----
    {
        .species = SPECIES_ANNIHILAPE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Bulk Up + Rage Fist snowball
        .moves = {MOVE_BULK_UP, MOVE_RAGE_FIST, MOVE_DRAIN_PUNCH, MOVE_TAUNT},
        .ability = ABILITY_VITAL_SPIRIT,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ANNIHILAPE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SCARF, // Defiant revenge killer
        .moves = {MOVE_RAGE_FIST, MOVE_CLOSE_COMBAT, MOVE_ICE_PUNCH, MOVE_U_TURN},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Clodsire ----
    {
        .species = SPECIES_CLODSIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Unaware special wall / status spreader
        .moves = {MOVE_TOXIC, MOVE_RECOVER, MOVE_EARTHQUAKE, MOVE_TOXIC_SPIKES},
        .ability = ABILITY_UNAWARE,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CLODSIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Water Absorb stall pivot
        .moves = {MOVE_RECOVER, MOVE_EARTHQUAKE, MOVE_TOXIC, MOVE_STEALTH_ROCK},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Farigiraf ----
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Armor Tail Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC_NOISE, MOVE_HYPER_VOICE, MOVE_REST},
        .ability = ABILITY_ARMOR_TAIL,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FARIGIRAF,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Trick Room support tank
        .moves = {MOVE_TRICK_ROOM, MOVE_HYPER_VOICE, MOVE_PSYCHIC, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_ARMOR_TAIL,
        .nature = NATURE_SASSY,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Dudunsparce ----
    {
        .species = SPECIES_DUDUNSPARCE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Serene Grace Coil + flinch / status
        .moves = {MOVE_COIL, MOVE_BODY_SLAM, MOVE_ROOST, MOVE_EARTHQUAKE},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_DUDUNSPARCE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind + Boomburst special
        .moves = {MOVE_CALM_MIND, MOVE_BOOMBURST, MOVE_EARTH_POWER, MOVE_ROOST},
        .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Kingambit ----
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Supreme Overlord Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_KOWTOW_CLEAVE, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH},
        .ability = ABILITY_SUPREME_OVERLORD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(112, 252, 0, 0, 0, 144),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_GLASSES, // Defiant pivot punisher
        .moves = {MOVE_KOWTOW_CLEAVE, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH, MOVE_LOW_KICK},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(112, 252, 0, 0, 0, 144),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KINGAMBIT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_ASSAULT_VEST, // bulky special tank
        .moves = {MOVE_KOWTOW_CLEAVE, MOVE_IRON_HEAD, MOVE_SUCKER_PUNCH, MOVE_LOW_KICK},
        .ability = ABILITY_SUPREME_OVERLORD,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Great Tusk ----
    {
        .species = SPECIES_GREAT_TUSK,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis hazard control sweeper
        .moves = {MOVE_HEADLONG_RUSH, MOVE_CLOSE_COMBAT, MOVE_RAPID_SPIN, MOVE_ICE_SPINNER},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GREAT_TUSK,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Bulk Up physical wall / spinner
        .moves = {MOVE_BULK_UP, MOVE_HEADLONG_RUSH, MOVE_BODY_PRESS, MOVE_RAPID_SPIN},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Scream Tail ----
    {
        .species = SPECIES_SCREAM_TAIL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Protosynthesis utility wall
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_DAZZLING_GLEAM, MOVE_ENCORE},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 252, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SCREAM_TAIL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // fast support pivot
        .moves = {MOVE_PLAY_ROUGH, MOVE_PSYCHIC_FANGS, MOVE_THUNDER_WAVE, MOVE_WISH},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Brute Bonnet ----
    {
        .species = SPECIES_BRUTE_BONNET,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis bulky breaker
        .moves = {MOVE_SUCKER_PUNCH, MOVE_SEED_BOMB, MOVE_SPORE, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_BRAVE,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Flutter Mane ----
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special sweeper (innate Levitate)
        .moves = {MOVE_MOONBLAST, MOVE_SHADOW_BALL, MOVE_MYSTICAL_FIRE, MOVE_CALM_MIND},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS, // special nuke (innate Levitate)
        .moves = {MOVE_MOONBLAST, MOVE_SHADOW_BALL, MOVE_POWER_GEM, MOVE_THUNDERBOLT},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FLUTTER_MANE,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_FOCUS_SASH, // fast Perish Trap / utility (innate Levitate)
        .moves = {MOVE_MOONBLAST, MOVE_SHADOW_BALL, MOVE_DAZZLING_GLEAM, MOVE_PROTECT},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Slither Wing ----
    {
        .species = SPECIES_SLITHER_WING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis bulky attacker
        .moves = {MOVE_CLOSE_COMBAT, MOVE_FIRST_IMPRESSION, MOVE_U_TURN, MOVE_FLARE_BLITZ},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Sandy Shocks ----
    {
        .species = SPECIES_SANDY_SHOCKS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special attacker (innate Levitate)
        .moves = {MOVE_THUNDERBOLT, MOVE_EARTH_POWER, MOVE_VOLT_SWITCH, MOVE_FLASH_CANNON},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SANDY_SHOCKS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // hazard lead (innate Levitate)
        .moves = {MOVE_STEALTH_ROCK, MOVE_THUNDERBOLT, MOVE_EARTH_POWER, MOVE_VOLT_SWITCH},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Iron Treads ----
    {
        .species = SPECIES_IRON_TREADS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive hazard control sweeper
        .moves = {MOVE_EARTHQUAKE, MOVE_IRON_HEAD, MOVE_RAPID_SPIN, MOVE_ICE_SPINNER},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_IRON_TREADS,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky hazard lead
        .moves = {MOVE_STEALTH_ROCK, MOVE_EARTHQUAKE, MOVE_RAPID_SPIN, MOVE_KNOCK_OFF},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Iron Bundle ----
    {
        .species = SPECIES_IRON_BUNDLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive fast special attacker
        .moves = {MOVE_HYDRO_PUMP, MOVE_FREEZE_DRY, MOVE_FLIP_TURN, MOVE_ICE_BEAM},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_IRON_BUNDLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS, // hydro specs nuke
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_FLIP_TURN, MOVE_FREEZE_DRY},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Iron Hands ----
    {
        .species = SPECIES_IRON_HANDS,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Quark Drive bulky attacker
        .moves = {MOVE_DRAIN_PUNCH, MOVE_THUNDER_PUNCH, MOVE_FAKE_OUT, MOVE_WILD_CHARGE},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_IRON_HANDS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Belly Drum + Drain Punch sweeper
        .moves = {MOVE_BELLY_DRUM, MOVE_DRAIN_PUNCH, MOVE_ICE_PUNCH, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Iron Jugulis ----
    {
        .species = SPECIES_IRON_JUGULIS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive fast special attacker
        .moves = {MOVE_DARK_PULSE, MOVE_HURRICANE, MOVE_EARTH_POWER, MOVE_FLAMETHROWER},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Iron Moth ----
    {
        .species = SPECIES_IRON_MOTH,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive special sweeper (innate Levitate)
        .moves = {MOVE_FIERY_DANCE, MOVE_SLUDGE_WAVE, MOVE_ENERGY_BALL, MOVE_TERA_BLAST},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_IRON_MOTH,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Toxic Spikes / special pivot (innate Levitate)
        .moves = {MOVE_FLAMETHROWER, MOVE_SLUDGE_WAVE, MOVE_TOXIC_SPIKES, MOVE_MORNING_SUN},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Iron Thorns ----
    {
        .species = SPECIES_IRON_THORNS,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_THUNDER_PUNCH},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Baxcalibur ----
    {
        .species = SPECIES_BAXCALIBUR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LOADED_DICE, // Dragon Dance + Icicle Spear sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_ICICLE_SPEAR, MOVE_GLAIVE_RUSH, MOVE_EARTHQUAKE},
        .ability = ABILITY_THERMAL_EXCHANGE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_BAXCALIBUR,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // mixed Dragon Dance breaker
        .moves = {MOVE_DRAGON_DANCE, MOVE_ICICLE_CRASH, MOVE_GLAIVE_RUSH, MOVE_ICE_SHARD},
        .ability = ABILITY_THERMAL_EXCHANGE,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Gholdengo ----
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Nasty Plot + Make It Rain sweeper (innate Levitate)
        .moves = {MOVE_NASTY_PLOT, MOVE_MAKE_IT_RAIN, MOVE_SHADOW_BALL, MOVE_RECOVER},
        .ability = ABILITY_GOOD_AS_GOLD,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Good as Gold status blocker / pivot (innate Levitate)
        .moves = {MOVE_MAKE_IT_RAIN, MOVE_SHADOW_BALL, MOVE_RECOVER, MOVE_THUNDER_WAVE},
        .ability = ABILITY_GOOD_AS_GOLD,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GHOLDENGO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // special nuke (innate Levitate)
        .moves = {MOVE_MAKE_IT_RAIN, MOVE_SHADOW_BALL, MOVE_FOCUS_BLAST, MOVE_TRICK},
        .ability = ABILITY_GOOD_AS_GOLD,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Wo-Chien ----
    {
        .species = SPECIES_WO_CHIEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Tablets of Ruin defensive status spreader
        .moves = {MOVE_LEECH_SEED, MOVE_GIGA_DRAIN, MOVE_KNOCK_OFF, MOVE_PROTECT},
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WO_CHIEN,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_BLACK_SLUDGE, // Tablets stall wall
        .moves = {MOVE_LEECH_SEED, MOVE_GIGA_DRAIN, MOVE_FOUL_PLAY, MOVE_STUN_SPORE},
        .ability = ABILITY_TABLETS_OF_RUIN,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Chien-Pao ----
    {
        .species = SPECIES_CHIEN_PAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Sword of Ruin Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_ICICLE_CRASH, MOVE_SUCKER_PUNCH, MOVE_SACRED_SWORD},
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHIEN_PAO,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Sword of Ruin band breaker
        .moves = {MOVE_ICE_SPINNER, MOVE_CRUNCH, MOVE_SACRED_SWORD, MOVE_ICE_SHARD},
        .ability = ABILITY_SWORD_OF_RUIN,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ICE,
        .ball = BALL_POKE,
    },

    // ---- Ting-Lu ----
    {
        .species = SPECIES_TING_LU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Vessel of Ruin physical wall / hazards
        .moves = {MOVE_STEALTH_ROCK, MOVE_SPIKES, MOVE_EARTHQUAKE, MOVE_WHIRLWIND},
        .ability = ABILITY_VESSEL_OF_RUIN,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TING_LU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Ruination staller
        .moves = {MOVE_RUINATION, MOVE_EARTHQUAKE, MOVE_STEALTH_ROCK, MOVE_REST},
        .ability = ABILITY_VESSEL_OF_RUIN,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Chi-Yu ----
    {
        .species = SPECIES_CHI_YU,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Beads of Ruin special sweeper
        .moves = {MOVE_NASTY_PLOT, MOVE_OVERHEAT, MOVE_DARK_PULSE, MOVE_FLAMETHROWER},
        .ability = ABILITY_BEADS_OF_RUIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_CHI_YU,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS, // Beads of Ruin nuke
        .moves = {MOVE_FIRE_BLAST, MOVE_DARK_PULSE, MOVE_PSYCHIC, MOVE_FLAMETHROWER},
        .ability = ABILITY_BEADS_OF_RUIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },

    // ---- Roaring Moon ----
    {
        .species = SPECIES_ROARING_MOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_KNOCK_OFF, MOVE_OUTRAGE, MOVE_ROOST},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FLYING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ROARING_MOON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // Dragon's Maw band breaker
        .moves = {MOVE_OUTRAGE, MOVE_KNOCK_OFF, MOVE_EARTHQUAKE, MOVE_U_TURN},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_DARK,
        .ball = BALL_POKE,
    },

    // ---- Iron Valiant ----
    {
        .species = SPECIES_IRON_VALIANT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive mixed Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_SPIRIT_BREAK},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_IRON_VALIANT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS, // special breaker
        .moves = {MOVE_MOONBLAST, MOVE_AURA_SPHERE, MOVE_PSYSHOCK, MOVE_THUNDERBOLT},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Koraidon ----
    {
        .species = SPECIES_KORAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Orichalcum Pulse sun sweeper
        .moves = {MOVE_COLLISION_COURSE, MOVE_FLARE_BLITZ, MOVE_DRAGON_CLAW, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_ORICHALCUM_PULSE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_KORAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_BAND, // band breaker
        .moves = {MOVE_COLLISION_COURSE, MOVE_OUTRAGE, MOVE_FLARE_BLITZ, MOVE_U_TURN},
        .ability = ABILITY_ORICHALCUM_PULSE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIGHTING,
        .ball = BALL_POKE,
    },

    // ---- Miraidon ----
    {
        .species = SPECIES_MIRAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CHOICE_SPECS, // Hadron Engine special nuke (innate Levitate)
        .moves = {MOVE_ELECTRO_DRIFT, MOVE_DRACO_METEOR, MOVE_VOLT_SWITCH, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_HADRON_ENGINE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MIRAIDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Calm Mind sweeper (innate Levitate)
        .moves = {MOVE_CALM_MIND, MOVE_ELECTRO_DRIFT, MOVE_DRACO_METEOR, MOVE_DAZZLING_GLEAM},
        .ability = ABILITY_HADRON_ENGINE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Walking Wake ----
    {
        .species = SPECIES_WALKING_WAKE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis special attacker
        .moves = {MOVE_HYDRO_STEAM, MOVE_DRACO_METEOR, MOVE_FLAMETHROWER, MOVE_FLIP_TURN},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_WALKING_WAKE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_CHOICE_SPECS, // Draco specs nuke
        .moves = {MOVE_DRACO_METEOR, MOVE_HYDRO_PUMP, MOVE_FLAMETHROWER, MOVE_FLIP_TURN},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Iron Leaves ----
    {
        .species = SPECIES_IRON_LEAVES,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_PSYBLADE, MOVE_CLOSE_COMBAT},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Sinistcha ----
    {
        .species = SPECIES_SINISTCHA,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Calm Mind Matcha Gotcha wall (innate Levitate)
        .moves = {MOVE_CALM_MIND, MOVE_MATCHA_GOTCHA, MOVE_SHADOW_BALL, MOVE_STRENGTH_SAP},
        .ability = ABILITY_HEATPROOF,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_SINISTCHA,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_ASSAULT_VEST, // Hospitality support tank (innate Levitate)
        .moves = {MOVE_MATCHA_GOTCHA, MOVE_SHADOW_BALL, MOVE_GIGA_DRAIN, MOVE_TRICK_ROOM},
        .ability = ABILITY_HOSPITALITY,
        .nature = NATURE_QUIET,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .iv = TRAINER_PARTY_IVS(31, 0, 31, 0, 31, 31),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Okidogi ----
    {
        .species = SPECIES_OKIDOGI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Toxic Chain Bulk Up sweeper
        .moves = {MOVE_BULK_UP, MOVE_GUNK_SHOT, MOVE_CLOSE_COMBAT, MOVE_CRUNCH},
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_OKIDOGI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Guard Dog Bulk Up wall breaker
        .moves = {MOVE_BULK_UP, MOVE_DRAIN_PUNCH, MOVE_POISON_JAB, MOVE_PSYCHIC_FANGS},
        .ability = ABILITY_GUARD_DOG,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Munkidori ----
    {
        .species = SPECIES_MUNKIDORI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LIFE_ORB, // Toxic Chain special attacker
        .moves = {MOVE_NASTY_PLOT, MOVE_SLUDGE_WAVE, MOVE_PSYCHIC, MOVE_DARK_PULSE},
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_MUNKIDORI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator special pivot
        .moves = {MOVE_SLUDGE_BOMB, MOVE_PSYCHIC, MOVE_U_TURN, MOVE_FUTURE_SIGHT},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Fezandipiti ----
    {
        .species = SPECIES_FEZANDIPITI,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Toxic Chain utility pivot
        .moves = {MOVE_ROOST, MOVE_TOXIC, MOVE_FOUL_PLAY, MOVE_U_TURN},
        .ability = ABILITY_TOXIC_CHAIN,
        .nature = NATURE_CAREFUL,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 0, 252),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_FEZANDIPITI,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // Calm Mind special pivot
        .moves = {MOVE_CALM_MIND, MOVE_MOONBLAST, MOVE_SLUDGE_BOMB, MOVE_ROOST},
        .ability = ABILITY_TECHNICIAN,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 252, 4, 0),
        .teraType = TYPE_GROUND,
        .ball = BALL_POKE,
    },

    // ---- Ogerpon (Teal) ----
    {
        .species = SPECIES_OGERPON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_LEFTOVERS, // Defiant Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_IVY_CUDGEL, MOVE_POWER_WHIP, MOVE_KNOCK_OFF},
        .ability = ABILITY_DEFIANT,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_GRASS,
        .ball = BALL_POKE,
    },

    // ---- Ogerpon (Wellspring) ----
    {
        .species = SPECIES_OGERPON_WELLSPRING,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_WELLSPRING_MASK, // Water Absorb Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_IVY_CUDGEL, MOVE_POWER_WHIP, MOVE_PLAY_ROUGH},
        .ability = ABILITY_WATER_ABSORB,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_WATER,
        .ball = BALL_POKE,
    },

    // ---- Ogerpon (Hearthflame) ----
    {
        .species = SPECIES_OGERPON_HEARTHFLAME,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEARTHFLAME_MASK, // Mold Breaker Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_IVY_CUDGEL, MOVE_POWER_WHIP, MOVE_HORN_LEECH},
        .ability = ABILITY_MOLD_BREAKER,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },

    // ---- Ogerpon (Cornerstone) ----
    {
        .species = SPECIES_OGERPON_CORNERSTONE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_CORNERSTONE_MASK, // Sturdy Swords Dance sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_IVY_CUDGEL, MOVE_POWER_WHIP, MOVE_STONE_EDGE},
        .ability = ABILITY_STURDY,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Archaludon ----
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // Electro Shot special tank
        .moves = {MOVE_ELECTRO_SHOT, MOVE_DRACO_METEOR, MOVE_FLASH_CANNON, MOVE_BODY_PRESS},
        .ability = ABILITY_STAMINA,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Stamina Body Press wall
        .moves = {MOVE_IRON_DEFENSE, MOVE_BODY_PRESS, MOVE_DRAGON_TAIL, MOVE_STEALTH_ROCK},
        .ability = ABILITY_STAMINA,
        .nature = NATURE_IMPISH,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .teraType = TYPE_GHOST,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_ARCHALUDON,
        .tags = FORMAT_DOUBLES,
        .heldItem = ITEM_LIFE_ORB, // rain Electro Shot nuke
        .moves = {MOVE_ELECTRO_SHOT, MOVE_FLASH_CANNON, MOVE_DRACO_METEOR, MOVE_PROTECT},
        .ability = ABILITY_STAMINA,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Hydrapple ----
    {
        .species = SPECIES_HYDRAPPLE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Regenerator bulky special tank
        .moves = {MOVE_FICKLE_BEAM, MOVE_GIGA_DRAIN, MOVE_NASTY_PLOT, MOVE_RECOVER},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_HYDRAPPLE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank pivot
        .moves = {MOVE_FICKLE_BEAM, MOVE_DRACO_METEOR, MOVE_GIGA_DRAIN, MOVE_EARTH_POWER},
        .ability = ABILITY_REGENERATOR,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_POISON,
        .ball = BALL_POKE,
    },

    // ---- Gouging Fire ----
    {
        .species = SPECIES_GOUGING_FIRE,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Dragon Dance sweeper
        .moves = {MOVE_DRAGON_DANCE, MOVE_HEAT_CRASH, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_FIRE,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_GOUGING_FIRE,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // bulky Morning Sun setup
        .moves = {MOVE_DRAGON_DANCE, MOVE_FLARE_BLITZ, MOVE_DRAGON_CLAW, MOVE_MORNING_SUN},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_ADAMANT,
        .ev = TRAINER_PARTY_EVS(252, 4, 0, 252, 0, 0),
        .teraType = TYPE_DRAGON,
        .ball = BALL_POKE,
    },

    // ---- Raging Bolt ----
    {
        .species = SPECIES_RAGING_BOLT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Protosynthesis Calm Mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_THUNDERCLAP, MOVE_DRACO_METEOR, MOVE_THUNDERBOLT},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_ELECTRIC,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_RAGING_BOLT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // bulky Calm Mind wall
        .moves = {MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_DRAGON_PULSE, MOVE_THUNDERCLAP},
        .ability = ABILITY_PROTOSYNTHESIS,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Iron Boulder ----
    {
        .species = SPECIES_IRON_BOULDER,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Mighty Cleave sweeper
        .moves = {MOVE_SWORDS_DANCE, MOVE_MIGHTY_CLEAVE, MOVE_EARTHQUAKE, MOVE_ZEN_HEADBUTT},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_JOLLY,
        .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .teraType = TYPE_ROCK,
        .ball = BALL_POKE,
    },

    // ---- Iron Crown ----
    {
        .species = SPECIES_IRON_CROWN,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BOOSTER_ENERGY, // Quark Drive Calm Mind special sweeper
        .moves = {MOVE_CALM_MIND, MOVE_TACHYON_CUTTER, MOVE_PSYCHIC_NOISE, MOVE_FOCUS_BLAST},
        .ability = ABILITY_QUARK_DRIVE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },

    // ---- Terapagos ----
    {
        .species = SPECIES_TERAPAGOS_TERASTAL,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Tera Shell Calm Mind tank
        .moves = {MOVE_CALM_MIND, MOVE_TERA_STARSTORM, MOVE_EARTH_POWER, MOVE_RECOVER},
        .ability = ABILITY_TERA_SHELL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 4, 0, 252, 0),
        .teraType = TYPE_NORMAL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_TERAPAGOS_TERASTAL,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_ASSAULT_VEST, // special tank
        .moves = {MOVE_TERA_STARSTORM, MOVE_EARTH_POWER, MOVE_DARK_PULSE, MOVE_FLAMETHROWER},
        .ability = ABILITY_TERA_SHELL,
        .nature = NATURE_MODEST,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .teraType = TYPE_STELLAR,
        .ball = BALL_POKE,
    },

    // ---- Pecharunt ----
    {
        .species = SPECIES_PECHARUNT,
        .tags = FORMAT_SINGLES,
        .heldItem = ITEM_LEFTOVERS, // Poison Puppeteer Nasty Plot tank (innate Levitate)
        .moves = {MOVE_NASTY_PLOT, MOVE_HEX, MOVE_SLUDGE_BOMB, MOVE_RECOVER},
        .ability = ABILITY_POISON_PUPPETEER,
        .nature = NATURE_BOLD,
        .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .teraType = TYPE_STEEL,
        .ball = BALL_POKE,
    },
    {
        .species = SPECIES_PECHARUNT,
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_BLACK_SLUDGE, // Malignant Chain status tank (innate Levitate)
        .moves = {MOVE_MALIGNANT_CHAIN, MOVE_HEX, MOVE_PARTING_SHOT, MOVE_RECOVER},
        .ability = ABILITY_POISON_PUPPETEER,
        .nature = NATURE_CALM,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

    // ---- Celebi ---- (innate Levitate — Ground immune, never give an Air Balloon)
    {
        .species = SPECIES_CELEBI,
        .tags = FORMAT_BOTH,
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
        .tags = FORMAT_SINGLES,
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
        .tags = FORMAT_BOTH,
        .heldItem = ITEM_HEAVY_DUTY_BOOTS, // calm mind bulky sweeper
        .moves = {MOVE_CALM_MIND, MOVE_GIGA_DRAIN, MOVE_PSYCHIC, MOVE_RECOVER},
        .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID,
        .ev = TRAINER_PARTY_EVS(252, 0, 0, 60, 196, 0),
        .teraType = TYPE_FAIRY,
        .ball = BALL_POKE,
    },

};

const u16 gFrontierExtendedMonsCount = ARRAY_COUNT(gFrontierExtendedMons);
