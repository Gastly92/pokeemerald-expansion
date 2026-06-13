#include "global.h"
#include "battle_tower_trainers.h"
#include "battle.h"
#include "battle_frontier.h"
#include "frontier_util.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "data.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/event_objects.h"
#include "constants/frontier_util.h"

// FORK: fork-owned static competitive teams for the Battle Tower's special
// opponents — the Salon Maiden (Frontier Brain) and the gym-leader bosses.
// Kept out of the upstream frontier files so this data carries no merge-conflict
// surface; battle_frontier.c / frontier_util.c / battle_tower.c hold only tiny
// hooks that route the tower brain / boss ids to the functions below. The teams
// are Lv100 with max IVs (built through CreateFacilityMon, like the roster), tuned
// for this fork's DETERMINISTIC_* rules (accuracy never misses, etc.). All sets
// are authored to bring exactly one Mega Stone; the bosses additionally each carry
// one legendary-tier mon. See FORK.md / battle_tower_trainers.h.
//
// EVs use TRAINER_PARTY_EVS(hp, atk, def, speed, spatk, spdef) — speed is 4th.
//
// Teams are authored at the full 6v6 size. The arrays are a fixed 6 (not
// FRONTIER_PARTY_SIZE, which is 3 when B_FRONTIER_PARTY_SIZE_6V6 is off) so the
// initializers always fit; the builders only create the first FRONTIER_PARTY_SIZE.
#define TOWER_SPECIAL_TEAM_SIZE 6

// ===========================================================================
// Frontier Brain (Anabel) — static teams, Silver (50th win) and Gold (100th).
// Modelled on her vanilla Tower teams (a psychic specialist backed by the
// legendary beasts and the Lati twins), expanded to a full six and made
// competitive; the Gold team is the tougher rematch.
// ===========================================================================

static const struct TrainerMon sTowerBrainTeam[2][TOWER_SPECIAL_TEAM_SIZE] =
{
    // Silver Symbol (50th win).
    {
        {
            .species = SPECIES_ALAKAZAM, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_MAGIC_GUARD,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
            .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_NASTY_PLOT},
        },
        {
            .species = SPECIES_METAGROSS, .heldItem = ITEM_METAGROSSITE, .ability = ABILITY_CLEAR_BODY,
            .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
            .moves = {MOVE_METEOR_MASH, MOVE_ZEN_HEADBUTT, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE},
        },
        {
            .species = SPECIES_ENTEI, .heldItem = ITEM_CHOICE_BAND, .ability = ABILITY_INNER_FOCUS,
            .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
            .moves = {MOVE_SACRED_FIRE, MOVE_EXTREME_SPEED, MOVE_STONE_EDGE, MOVE_FLARE_BLITZ},
        },
        {
            .species = SPECIES_STARMIE, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_NATURAL_CURE,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
            .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        },
        {
            .species = SPECIES_SNORLAX, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_THICK_FAT,
            .nature = NATURE_CAREFUL, .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
            .moves = {MOVE_BODY_SLAM, MOVE_CURSE, MOVE_REST, MOVE_EARTHQUAKE},
        },
        {
            .species = SPECIES_SALAMENCE, .heldItem = ITEM_LUM_BERRY, .ability = ABILITY_INTIMIDATE,
            .nature = NATURE_NAIVE, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
            .moves = {MOVE_DRAGON_DANCE, MOVE_DRAGON_CLAW, MOVE_EARTHQUAKE, MOVE_FIRE_BLAST},
        },
    },
    // Gold Symbol (100th win).
    {
        {
            .species = SPECIES_RAIKOU, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_PRESSURE,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
            .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_AURA_SPHERE, MOVE_CALM_MIND},
        },
        {
            .species = SPECIES_LATIAS, .heldItem = ITEM_LATIASITE, .ability = ABILITY_LEVITATE,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(252, 0, 0, 200, 0, 56),
            .moves = {MOVE_DRAGON_PULSE, MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_RECOVER},
        },
        {
            .species = SPECIES_LATIOS, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_LEVITATE,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
            .moves = {MOVE_DRACO_METEOR, MOVE_PSYCHIC, MOVE_AURA_SPHERE, MOVE_CALM_MIND},
        },
        {
            .species = SPECIES_METAGROSS, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_CLEAR_BODY,
            .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
            .moves = {MOVE_METEOR_MASH, MOVE_ZEN_HEADBUTT, MOVE_BULLET_PUNCH, MOVE_EARTHQUAKE},
        },
        {
            .species = SPECIES_SNORLAX, .heldItem = ITEM_CHESTO_BERRY, .ability = ABILITY_THICK_FAT,
            .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
            .moves = {MOVE_CURSE, MOVE_BODY_SLAM, MOVE_REST, MOVE_HIGH_HORSEPOWER},
        },
        {
            .species = SPECIES_ALAKAZAM, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_MAGIC_GUARD,
            .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
            .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_NASTY_PLOT},
        },
    },
};

void FillTowerBrainParty(u32 symbol)
{
    u32 i;
    u8 level = SetFacilityPtrsGetLevel();
    u32 otId = READ_OTID_FROM_SAVE;

    if (symbol > 1)
        symbol = 1;
    ZeroEnemyPartyMons();
    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        CreateFacilityMon(&sTowerBrainTeam[symbol][i], level, MAX_PER_STAT_IVS, otId, 0,
                          &gParties[B_TRAINER_OPPONENT_A][i]);
}

// ===========================================================================
// Gym-leader bosses. Each is a real Hoenn gym leader (its gTrainers[] id gives
// the displayed name/class, its overworld sprite the boss object) fielding a
// type-themed competitive team that carries one legendary-tier mon and one Mega.
// Extend gTowerBosses[] (and TOWER_BOSS_TRAINER_FIRST's range follows
// automatically) to add Elite Four / Champions later — the order is irrelevant,
// bosses are chosen at random.
// ===========================================================================

// --- Roxanne (Rock) — legendary: Regirock ---
static const struct TrainerMon sBossRoxanne[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_TYRANITAR, .heldItem = ITEM_TYRANITARITE, .ability = ABILITY_SAND_STREAM,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_STONE_EDGE, MOVE_CRUNCH, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE},
    },
    {
        .species = SPECIES_REGIROCK, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_CLEAR_BODY,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .moves = {MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_CURSE, MOVE_REST},
    },
    {
        .species = SPECIES_AERODACTYL, .heldItem = ITEM_FOCUS_SASH, .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_TAILWIND},
    },
    {
        .species = SPECIES_AGGRON, .heldItem = ITEM_ASSAULT_VEST, .ability = ABILITY_ROCK_HEAD,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .moves = {MOVE_HEAD_SMASH, MOVE_HEAVY_SLAM, MOVE_EARTHQUAKE, MOVE_FIRE_PUNCH},
    },
    {
        .species = SPECIES_RHYPERIOR, .heldItem = ITEM_WEAKNESS_POLICY, .ability = ABILITY_SOLID_ROCK,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .moves = {MOVE_ROCK_WRECKER, MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_ICE_PUNCH},
    },
    {
        .species = SPECIES_CRADILY, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_STORM_DRAIN,
        .nature = NATURE_CAREFUL, .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .moves = {MOVE_POWER_WHIP, MOVE_ROCK_SLIDE, MOVE_RECOVER, MOVE_TOXIC},
    },
};

// --- Brawly (Fighting) — legendary: Terrakion ---
static const struct TrainerMon sBossBrawly[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_MEDICHAM, .heldItem = ITEM_MEDICHAMITE, .ability = ABILITY_PURE_POWER,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_ZEN_HEADBUTT, MOVE_ICE_PUNCH, MOVE_BULLET_PUNCH},
    },
    {
        .species = SPECIES_TERRAKION, .heldItem = ITEM_CHOICE_SCARF, .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_CLOSE_COMBAT, MOVE_STONE_EDGE, MOVE_EARTHQUAKE, MOVE_X_SCISSOR},
    },
    {
        .species = SPECIES_CONKELDURR, .heldItem = ITEM_FLAME_ORB, .ability = ABILITY_GUTS,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 0, 0, 4),
        .moves = {MOVE_DRAIN_PUNCH, MOVE_MACH_PUNCH, MOVE_KNOCK_OFF, MOVE_FACADE},
    },
    {
        .species = SPECIES_HAWLUCHA, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_UNBURDEN,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_CLOSE_COMBAT, MOVE_ACROBATICS, MOVE_SWORDS_DANCE, MOVE_STONE_EDGE},
    },
    {
        .species = SPECIES_BRELOOM, .heldItem = ITEM_TOXIC_ORB, .ability = ABILITY_POISON_HEAL,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .moves = {MOVE_BULLET_SEED, MOVE_MACH_PUNCH, MOVE_SPORE, MOVE_SWORDS_DANCE},
    },
    {
        .species = SPECIES_LUCARIO, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_JUSTIFIED,
        .nature = NATURE_NAIVE, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .moves = {MOVE_AURA_SPHERE, MOVE_FLASH_CANNON, MOVE_EXTREME_SPEED, MOVE_NASTY_PLOT},
    },
};

// --- Wattson (Electric) — legendary: Raikou ---
static const struct TrainerMon sBossWattson[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_MANECTRIC, .heldItem = ITEM_MANECTITE, .ability = ABILITY_LIGHTNING_ROD,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_THUNDERBOLT, MOVE_VOLT_SWITCH, MOVE_FLAMETHROWER, MOVE_OVERHEAT},
    },
    {
        .species = SPECIES_RAIKOU, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_PRESSURE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_THUNDERBOLT, MOVE_AURA_SPHERE, MOVE_CALM_MIND, MOVE_SUBSTITUTE},
    },
    {
        .species = SPECIES_MAGNEZONE, .heldItem = ITEM_ASSAULT_VEST, .ability = ABILITY_MAGNET_PULL,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .moves = {MOVE_THUNDERBOLT, MOVE_FLASH_CANNON, MOVE_VOLT_SWITCH, MOVE_BODY_PRESS},
    },
    {
        .species = SPECIES_ROTOM_WASH, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_LEVITATE,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .moves = {MOVE_HYDRO_PUMP, MOVE_VOLT_SWITCH, MOVE_WILL_O_WISP, MOVE_THUNDERBOLT},
    },
    {
        .species = SPECIES_LANTURN, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_VOLT_ABSORB,
        .nature = NATURE_CALM, .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 4, 252),
        .moves = {MOVE_SCALD, MOVE_VOLT_SWITCH, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
    },
    {
        .species = SPECIES_ELECTIVIRE, .heldItem = ITEM_EXPERT_BELT, .ability = ABILITY_MOTOR_DRIVE,
        .nature = NATURE_NAIVE, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .moves = {MOVE_WILD_CHARGE, MOVE_ICE_PUNCH, MOVE_EARTHQUAKE, MOVE_CROSS_CHOP},
    },
};

// --- Flannery (Fire) — legendary: Heatran ---
static const struct TrainerMon sBossFlannery[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_CHARIZARD, .heldItem = ITEM_CHARIZARDITE_Y, .ability = ABILITY_BLAZE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_FIRE_BLAST, MOVE_AIR_SLASH, MOVE_SOLAR_BEAM, MOVE_ROOST},
    },
    {
        .species = SPECIES_HEATRAN, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_FLASH_FIRE,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .moves = {MOVE_MAGMA_STORM, MOVE_EARTH_POWER, MOVE_FLASH_CANNON, MOVE_TAUNT},
    },
    {
        .species = SPECIES_ARCANINE, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_FLARE_BLITZ, MOVE_EXTREME_SPEED, MOVE_WILD_CHARGE, MOVE_MORNING_SUN},
    },
    {
        .species = SPECIES_TORKOAL, .heldItem = ITEM_HEAT_ROCK, .ability = ABILITY_DROUGHT,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .moves = {MOVE_ERUPTION, MOVE_FIRE_BLAST, MOVE_EARTH_POWER, MOVE_RAPID_SPIN},
    },
    {
        .species = SPECIES_NINETALES, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_DROUGHT,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_NASTY_PLOT, MOVE_SCORCHING_SANDS},
    },
    {
        .species = SPECIES_MAGMORTAR, .heldItem = ITEM_CHOICE_SPECS, .ability = ABILITY_FLAME_BODY,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_OVERHEAT, MOVE_FOCUS_BLAST, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
    },
};

// --- Norman (Normal) — legendary: Regigigas ---
static const struct TrainerMon sBossNorman[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_LOPUNNY, .heldItem = ITEM_LOPUNNITE, .ability = ABILITY_LIMBER,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_HIGH_JUMP_KICK, MOVE_RETURN, MOVE_FAKE_OUT, MOVE_ICE_PUNCH},
    },
    {
        .species = SPECIES_REGIGIGAS, .heldItem = ITEM_LUM_BERRY, .ability = ABILITY_SLOW_START,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .moves = {MOVE_BODY_SLAM, MOVE_KNOCK_OFF, MOVE_DRAIN_PUNCH, MOVE_SUBSTITUTE},
    },
    {
        .species = SPECIES_SLAKING, .heldItem = ITEM_CHOICE_BAND, .ability = ABILITY_TRUANT,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_GIGA_IMPACT, MOVE_NIGHT_SLASH},
    },
    {
        .species = SPECIES_PORYGON_Z, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_ADAPTABILITY,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_TRI_ATTACK, MOVE_DARK_PULSE, MOVE_THUNDERBOLT, MOVE_NASTY_PLOT},
    },
    {
        .species = SPECIES_STARAPTOR, .heldItem = ITEM_CHOICE_SCARF, .ability = ABILITY_RECKLESS,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_BRAVE_BIRD, MOVE_DOUBLE_EDGE, MOVE_CLOSE_COMBAT, MOVE_U_TURN},
    },
    {
        .species = SPECIES_SNORLAX, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_THICK_FAT,
        .nature = NATURE_CAREFUL, .ev = TRAINER_PARTY_EVS(252, 4, 0, 0, 0, 252),
        .moves = {MOVE_BODY_SLAM, MOVE_CURSE, MOVE_REST, MOVE_HIGH_HORSEPOWER},
    },
};

// --- Winona (Flying) — legendary: Tornadus ---
static const struct TrainerMon sBossWinona[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_PIDGEOT, .heldItem = ITEM_PIDGEOTITE, .ability = ABILITY_KEEN_EYE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_HURRICANE, MOVE_HEAT_WAVE, MOVE_ROOST, MOVE_U_TURN},
    },
    {
        .species = SPECIES_TORNADUS, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_PRANKSTER,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_HURRICANE, MOVE_FOCUS_BLAST, MOVE_NASTY_PLOT, MOVE_TAILWIND},
    },
    {
        .species = SPECIES_SKARMORY, .heldItem = ITEM_ROCKY_HELMET, .ability = ABILITY_STURDY,
        .nature = NATURE_IMPISH, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 0, 4),
        .moves = {MOVE_BRAVE_BIRD, MOVE_BODY_PRESS, MOVE_SPIKES, MOVE_ROOST},
    },
    {
        .species = SPECIES_GYARADOS, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_JOLLY, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 0, 4),
        .moves = {MOVE_WATERFALL, MOVE_BOUNCE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE},
    },
    {
        .species = SPECIES_TOGEKISS, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_SERENE_GRACE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(252, 0, 0, 200, 0, 56),
        .moves = {MOVE_AIR_SLASH, MOVE_DAZZLING_GLEAM, MOVE_NASTY_PLOT, MOVE_ROOST},
    },
    {
        .species = SPECIES_SALAMENCE, .heldItem = ITEM_CHOICE_SCARF, .ability = ABILITY_INTIMIDATE,
        .nature = NATURE_NAIVE, .ev = TRAINER_PARTY_EVS(0, 252, 0, 252, 4, 0),
        .moves = {MOVE_DRAGON_CLAW, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_FIRE_BLAST},
    },
};

// --- Tate & Liza (Psychic) — legendary: Latios ---
static const struct TrainerMon sBossTateLiza[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_GARDEVOIR, .heldItem = ITEM_GARDEVOIRITE, .ability = ABILITY_TRACE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_PSYCHIC, MOVE_MOONBLAST, MOVE_FOCUS_BLAST, MOVE_CALM_MIND},
    },
    {
        .species = SPECIES_LATIOS, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_LEVITATE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_DRACO_METEOR, MOVE_PSYCHIC, MOVE_AURA_SPHERE, MOVE_CALM_MIND},
    },
    {
        .species = SPECIES_ALAKAZAM, .heldItem = ITEM_FOCUS_SASH, .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_BLAST, MOVE_SHADOW_BALL, MOVE_ENCORE},
    },
    {
        .species = SPECIES_REUNICLUS, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_MAGIC_GUARD,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .moves = {MOVE_PSYSHOCK, MOVE_FOCUS_BLAST, MOVE_CALM_MIND, MOVE_RECOVER},
    },
    {
        .species = SPECIES_SLOWBRO, .heldItem = ITEM_HEAVY_DUTY_BOOTS, .ability = ABILITY_REGENERATOR,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .moves = {MOVE_SCALD, MOVE_PSYCHIC, MOVE_SLACK_OFF, MOVE_THUNDER_WAVE},
    },
    {
        .species = SPECIES_BRONZONG, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_LEVITATE,
        .nature = NATURE_SASSY, .ev = TRAINER_PARTY_EVS(252, 0, 128, 0, 0, 128),
        .moves = {MOVE_GYRO_BALL, MOVE_PSYCHIC, MOVE_STEALTH_ROCK, MOVE_TRICK_ROOM},
    },
};

// --- Juan (Water) — legendary: Suicune ---
static const struct TrainerMon sBossJuan[TOWER_SPECIAL_TEAM_SIZE] =
{
    {
        .species = SPECIES_SWAMPERT, .heldItem = ITEM_SWAMPERTITE, .ability = ABILITY_TORRENT,
        .nature = NATURE_ADAMANT, .ev = TRAINER_PARTY_EVS(252, 252, 0, 4, 0, 0),
        .moves = {MOVE_WATERFALL, MOVE_EARTHQUAKE, MOVE_ICE_PUNCH, MOVE_POWER_UP_PUNCH},
    },
    {
        .species = SPECIES_SUICUNE, .heldItem = ITEM_LEFTOVERS, .ability = ABILITY_PRESSURE,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .moves = {MOVE_SCALD, MOVE_CALM_MIND, MOVE_REST, MOVE_SLEEP_TALK},
    },
    {
        .species = SPECIES_STARMIE, .heldItem = ITEM_LIFE_ORB, .ability = ABILITY_NATURAL_CURE,
        .nature = NATURE_TIMID, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
    },
    {
        .species = SPECIES_MILOTIC, .heldItem = ITEM_FLAME_ORB, .ability = ABILITY_MARVEL_SCALE,
        .nature = NATURE_BOLD, .ev = TRAINER_PARTY_EVS(252, 0, 252, 0, 4, 0),
        .moves = {MOVE_SCALD, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_HAZE},
    },
    {
        .species = SPECIES_KINGDRA, .heldItem = ITEM_DRAGON_FANG, .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(0, 0, 0, 252, 252, 4),
        .moves = {MOVE_HYDRO_PUMP, MOVE_DRACO_METEOR, MOVE_ICE_BEAM, MOVE_RAIN_DANCE},
    },
    {
        .species = SPECIES_LUDICOLO, .heldItem = ITEM_ASSAULT_VEST, .ability = ABILITY_SWIFT_SWIM,
        .nature = NATURE_MODEST, .ev = TRAINER_PARTY_EVS(252, 0, 0, 0, 252, 4),
        .moves = {MOVE_HYDRO_PUMP, MOVE_GIGA_DRAIN, MOVE_ICE_BEAM, MOVE_SCALD},
    },
};

struct TowerBoss
{
    u16 linkedTrainerId; // gTrainers[] id used for the displayed name / class
    u8 objEventGfx;      // overworld sprite shown for the boss object
    const struct TrainerMon *party;
};

static const struct TowerBoss sTowerBosses[] =
{
    { TRAINER_ROXANNE_1,        OBJ_EVENT_GFX_ROXANNE,  sBossRoxanne  },
    { TRAINER_BRAWLY_1,         OBJ_EVENT_GFX_BRAWLY,   sBossBrawly   },
    { TRAINER_WATTSON_1,        OBJ_EVENT_GFX_WATTSON,  sBossWattson  },
    { TRAINER_FLANNERY_1,       OBJ_EVENT_GFX_FLANNERY, sBossFlannery },
    { TRAINER_NORMAN_1,         OBJ_EVENT_GFX_NORMAN,   sBossNorman   },
    { TRAINER_WINONA_1,         OBJ_EVENT_GFX_WINONA,   sBossWinona   },
    { TRAINER_TATE_AND_LIZA_1,  OBJ_EVENT_GFX_LIZA,     sBossTateLiza },
    { TRAINER_JUAN_1,           OBJ_EVENT_GFX_JUAN,     sBossJuan     },
};

u32 GetTowerBossCount(void)
{
    return ARRAY_COUNT(sTowerBosses);
}

bool32 IsTowerBossTrainerId(u16 trainerId)
{
    return trainerId >= TOWER_BOSS_TRAINER_FIRST
        && trainerId < TOWER_BOSS_TRAINER_FIRST + ARRAY_COUNT(sTowerBosses);
}

u16 ChooseTowerBossTrainerId(void)
{
    // Pure random pick. (No "avoid immediate repeat" tracking: a mutable static
    // would land in .data, which the ld script doesn't place for fork-added
    // files, and the fork avoids editing the upstream ld script. A back-to-back
    // repeat is a rare, minor polish item.)
    return TOWER_BOSS_TRAINER_FIRST + (Random() % ARRAY_COUNT(sTowerBosses));
}

u8 GetTowerBossObjEventGfx(u16 trainerId)
{
    return sTowerBosses[trainerId - TOWER_BOSS_TRAINER_FIRST].objEventGfx;
}

u8 GetTowerBossTrainerPic(u16 trainerId)
{
    return GetTrainerPicFromId(sTowerBosses[trainerId - TOWER_BOSS_TRAINER_FIRST].linkedTrainerId);
}

enum TrainerClassID GetTowerBossTrainerClass(u16 trainerId)
{
    return GetTrainerClassFromId(sTowerBosses[trainerId - TOWER_BOSS_TRAINER_FIRST].linkedTrainerId);
}

void GetTowerBossTrainerName(u8 *dst, u16 trainerId)
{
    StringCopy(dst, GetTrainerNameFromId(sTowerBosses[trainerId - TOWER_BOSS_TRAINER_FIRST].linkedTrainerId));
}

static const u8 sTowerBossIntroText[] = _("{STR_VAR_1}: So you've made it this\nfar. Let's see your real strength!");
static const u8 sTowerBossWonText[]   = _("{STR_VAR_1}: Impressive… you've truly\nearned this victory!");
static const u8 sTowerBossLostText[]  = _("{STR_VAR_1}: Not bad… but not quite\nenough to beat me!");

// Buffer the boss's pre-battle / post-battle line into gStringVar4 (whichText is
// FRONTIER_BEFORE_TEXT / FRONTIER_PLAYER_WON_TEXT / FRONTIER_PLAYER_LOST_TEXT).
void BufferTowerBossBattleText(u8 whichText, u16 trainerId)
{
    GetTowerBossTrainerName(gStringVar1, trainerId);
    if (whichText == FRONTIER_PLAYER_WON_TEXT)
        StringExpandPlaceholders(gStringVar4, sTowerBossWonText);
    else if (whichText == FRONTIER_PLAYER_LOST_TEXT)
        StringExpandPlaceholders(gStringVar4, sTowerBossLostText);
    else
        StringExpandPlaceholders(gStringVar4, sTowerBossIntroText);
}

void FillTowerBossParty(u16 trainerId, u8 monCount)
{
    const struct TrainerMon *party = sTowerBosses[trainerId - TOWER_BOSS_TRAINER_FIRST].party;
    u8 level = SetFacilityPtrsGetLevel();
    u32 otId = READ_OTID_FROM_SAVE;
    u32 i;

    ZeroEnemyPartyMons();
    for (i = 0; i < monCount && i < FRONTIER_PARTY_SIZE; i++)
        CreateFacilityMon(&party[i], level, MAX_PER_STAT_IVS, otId, 0, &gParties[B_TRAINER_OPPONENT_A][i]);
}
