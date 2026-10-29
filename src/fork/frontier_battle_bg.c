#include "global.h"
#include "fork/frontier_battle_bg.h"
#include "fork/battle_tower_trainers.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "frontier_util.h"
#include "recorded_battle.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/trainers.h"

// Entries are stored as environment + 1 so that a class left out of the table reads 0
// ("no theme") rather than BATTLE_ENVIRONMENT_GRASS, which is also 0.
#define BG(env) (BATTLE_ENVIRONMENT_##env + 1)

// Only the classes that appear in gBattleFrontierTrainers are listed; the rest (grunts,
// leaders, ...) never reach this table and fall back to the Frontier backdrop.
static const u8 sFacilityClassEnvironments[FACILITY_CLASSES_COUNT] =
{
    // Grass and forest
    [FACILITY_CLASS_BUG_CATCHER]          = BG(LONG_GRASS),
    [FACILITY_CLASS_BUG_MANIAC]           = BG(LONG_GRASS),
    [FACILITY_CLASS_NINJA_BOY]            = BG(LONG_GRASS),
    [FACILITY_CLASS_PKMN_RANGER_M]        = BG(LONG_GRASS),
    [FACILITY_CLASS_PKMN_RANGER_F]        = BG(LONG_GRASS),
    [FACILITY_CLASS_YOUNGSTER]            = BG(GRASS),
    [FACILITY_CLASS_LASS]                 = BG(GRASS),
    [FACILITY_CLASS_CAMPER]               = BG(GRASS),
    [FACILITY_CLASS_PICNICKER]            = BG(POND),     // lakeside picnic
    [FACILITY_CLASS_AROMA_LADY]           = BG(GRASS),
    [FACILITY_CLASS_PKMN_BREEDER_M]       = BG(GRASS),
    [FACILITY_CLASS_PKMN_BREEDER_F]       = BG(GRASS),

    // Open routes
    [FACILITY_CLASS_BIRD_KEEPER]          = BG(RAYQUAZA), // Sky Pillar rooftop
    [FACILITY_CLASS_POKEFAN_M]            = BG(PLAIN),
    [FACILITY_CLASS_POKEFAN_F]            = BG(PLAIN),
    [FACILITY_CLASS_BEAUTY]               = BG(PLAIN),
    [FACILITY_CLASS_RUNNING_TRIATHLETE_M] = BG(PLAIN),
    [FACILITY_CLASS_RUNNING_TRIATHLETE_F] = BG(PLAIN),
    [FACILITY_CLASS_CYCLING_TRIATHLETE_M] = BG(PLAIN),
    [FACILITY_CLASS_CYCLING_TRIATHLETE_F] = BG(PLAIN),

    // Beach and water
    [FACILITY_CLASS_PARASOL_LADY]         = BG(SAND),
    [FACILITY_CLASS_TUBER_M]              = BG(SAND),
    [FACILITY_CLASS_TUBER_F]              = BG(SAND),
    [FACILITY_CLASS_SWIMMER_M]            = BG(WATER),
    [FACILITY_CLASS_SWIMMER_F]            = BG(WATER),
    [FACILITY_CLASS_SAILOR]               = BG(AQUA),
    [FACILITY_CLASS_SWIMMING_TRIATHLETE_M] = BG(UNDERWATER),
    [FACILITY_CLASS_SWIMMING_TRIATHLETE_F] = BG(UNDERWATER),
    [FACILITY_CLASS_FISHERMAN]            = BG(KYOGRE),   // stormy water

    // Mountains and caves
    [FACILITY_CLASS_HIKER]                = BG(MOUNTAIN),
    [FACILITY_CLASS_EXPERT_M]             = BG(MAGMA),
    [FACILITY_CLASS_EXPERT_F]             = BG(MAGMA),
    [FACILITY_CLASS_RUIN_MANIAC]          = BG(CAVE),
    [FACILITY_CLASS_POKEMANIAC]           = BG(CAVE),
    [FACILITY_CLASS_KINDLER]              = BG(GROUDON),  // volcanic cave

    // Gyms
    [FACILITY_CLASS_COOLTRAINER_M]        = BG(GYM),
    [FACILITY_CLASS_COOLTRAINER_F]        = BG(GYM),
    [FACILITY_CLASS_BLACK_BELT]           = BG(GYM),
    [FACILITY_CLASS_BATTLE_GIRL]          = BG(GYM),

    // Indoors
    [FACILITY_CLASS_COLLECTOR]            = BG(BUILDING),
    [FACILITY_CLASS_GENTLEMAN]            = BG(BUILDING),
    [FACILITY_CLASS_LADY]                 = BG(BUILDING),
    [FACILITY_CLASS_RICH_BOY]             = BG(BUILDING),
    [FACILITY_CLASS_SCHOOL_KID_M]         = BG(BUILDING),
    [FACILITY_CLASS_SCHOOL_KID_F]         = BG(BUILDING),
    [FACILITY_CLASS_GUITARIST]            = BG(BUILDING),

    // Elite Four arenas
    [FACILITY_CLASS_HEX_MANIAC]           = BG(PHOEBE),
    [FACILITY_CLASS_PSYCHIC_M]            = BG(PHOEBE),
    [FACILITY_CLASS_PSYCHIC_F]            = BG(PHOEBE),
    [FACILITY_CLASS_DRAGON_TAMER]         = BG(DRAKE),
};

static const u8 sFrontierBrainEnvironments[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER]   = BATTLE_ENVIRONMENT_CHAMPION, // Anabel
    [FRONTIER_FACILITY_DOME]    = BATTLE_ENVIRONMENT_SIDNEY,   // Tucker
    [FRONTIER_FACILITY_PALACE]  = BATTLE_ENVIRONMENT_PHOEBE,   // Spenser
    [FRONTIER_FACILITY_ARENA]   = BATTLE_ENVIRONMENT_DRAKE,    // Greta
    [FRONTIER_FACILITY_FACTORY] = BATTLE_ENVIRONMENT_CHAMPION, // Noland
    [FRONTIER_FACILITY_PIKE]    = BATTLE_ENVIRONMENT_SIDNEY,   // Lucy
    [FRONTIER_FACILITY_PYRAMID] = BATTLE_ENVIRONMENT_GLACIA,   // Brandon
};

#undef BG

u32 GetFrontierEnvironmentForFacilityClass(u32 facilityClass)
{
    if (facilityClass >= FACILITY_CLASSES_COUNT || sFacilityClassEnvironments[facilityClass] == 0)
        return BATTLE_ENVIRONMENT_FRONTIER;
    return sFacilityClassEnvironments[facilityClass] - 1;
}

u32 GetFrontierBrainEnvironment(u32 facility)
{
    if (facility >= NUM_FRONTIER_FACILITIES)
        return BATTLE_ENVIRONMENT_FRONTIER;
    return sFrontierBrainEnvironments[facility];
}

u32 GetFrontierBattleEnvironment(void)
{
    u32 trainerId = TRAINER_BATTLE_PARAM.opponentA;

    // Pyramid wild encounters have no trainer to theme.
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return BATTLE_ENVIRONMENT_FRONTIER;

    if (trainerId == TRAINER_FRONTIER_BRAIN)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
            return GetFrontierBrainEnvironment(GetRecordedBattleFrontierFacility());
        return GetFrontierBrainEnvironment(VarGet(VAR_FRONTIER_FACILITY));
    }
#if B_FRONTIER_ENDLESS
    if (IsTowerBossTrainerId(trainerId))
        return BATTLE_ENVIRONMENT_LEADER;
#endif
    if (trainerId < FRONTIER_TRAINERS_COUNT)
        return GetFrontierEnvironmentForFacilityClass(GetFrontierTrainerFacilityClass(trainerId));

    return BATTLE_ENVIRONMENT_FRONTIER;
}
