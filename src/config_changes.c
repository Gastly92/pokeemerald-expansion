#include "global.h"
#include "config_changes.h"
#include "malloc.h"
#include "constants/config_changes.h"
#include "config/pokerus.h"
#include "config/ai.h"

#define UNPACK_BATTLE_CONFIG_GEN_CHANGES(_name, _field, ...) ._field = _name,
#define UNPACK_POKEMON_CONFIG_GEN_CHANGES(_name, _field, ...) ._field = P_##_name,
#define UNPACK_AI_CONFIG_CHANGES(_name, _field, ...) ._field = _name,
#define UNPACK_DETERMINISTIC_CONFIG_CHANGES(_name, _field, ...) ._field = _name, // FORK
#define UNPACK_BUFF_CONFIG_CHANGES(_name, _field, ...) ._field = _name, // FORK

const struct ConfigChanges sConfigChanges =
{
    BATTLE_CONFIG_DEFINITIONS(UNPACK_BATTLE_CONFIG_GEN_CHANGES)
    POKEMON_CONFIG_DEFINITIONS(UNPACK_POKEMON_CONFIG_GEN_CHANGES)
    AI_CONFIG_DEFINITIONS(UNPACK_AI_CONFIG_CHANGES)
    DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_DETERMINISTIC_CONFIG_CHANGES) // FORK
    BUFF_CONFIG_DEFINITIONS(UNPACK_BUFF_CONFIG_CHANGES) // FORK
    /* Expands to:
    .critChance     = B_CRIT_CHANCE,
    .critMultiplier = B_CRIT_MULTIPLIER,
    */
};

#if TESTING
EWRAM_DATA struct ConfigChanges *gConfigChangesTestOverride = NULL;
#define UNPACK_CONFIG_OVERRIDE_GETTERS(_name, _field, ...)   case CONFIG_##_name: return gConfigChangesTestOverride->_field;
#define UNPACK_CONFIG_GETTERS(_name, _field, ...) case CONFIG_##_name: return sConfigChanges._field;
#define UNPACK_CONFIG_CLAMPER(_name, _field, _typeMaxValue, ...) case CONFIG_##_name: clampedValue = min(GET_CONFIG_MAXIMUM(_typeMaxValue), newValue); break;
#define UNPACK_CONFIG_SETTERS(_name, _field, _typeMaxValue, ...) case CONFIG_##_name: gConfigChangesTestOverride->_field = clampedValue; break;

#else

#define UNPACK_CONFIG_OVERRIDE_GETTERS(_name, _field, ...) case CONFIG_##_name: return sConfigChanges._field;
#define UNPACK_CONFIG_GETTERS(_name, _field, ...) case CONFIG_##_name: return sConfigChanges._field;
#define UNPACK_CONFIG_CLAMPER(_name, _field, ...) case CONFIG_##_name: return 0;
#define UNPACK_CONFIG_SETTERS(_name, _field, ...) case CONFIG_##_name: return;
#endif

// Gets the value of a volatile status flag for a certain battler
// Primarily used for the debug menu and scripts. Outside of it explicit references are preferred
u32 GetConfigInternal(enum ConfigTag _config)
{
#if TESTING
    if (gConfigChangesTestOverride == NULL)
    {
        switch (_config)
        {
        BATTLE_CONFIG_DEFINITIONS(UNPACK_CONFIG_GETTERS)
        POKEMON_CONFIG_DEFINITIONS(UNPACK_CONFIG_GETTERS)
        AI_CONFIG_DEFINITIONS(UNPACK_CONFIG_GETTERS)
        DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_CONFIG_GETTERS) // FORK
        BUFF_CONFIG_DEFINITIONS(UNPACK_CONFIG_GETTERS) // FORK
        /* Expands to:
        case CONFIG_CRIT_CHANCE:
            return gConfigChangesTestOverride->critChance;
        */
        default:
            return 0;
        }
    }
    else
#endif
    {
        switch (_config)
        {
        BATTLE_CONFIG_DEFINITIONS(UNPACK_CONFIG_OVERRIDE_GETTERS)
        POKEMON_CONFIG_DEFINITIONS(UNPACK_CONFIG_OVERRIDE_GETTERS)
        AI_CONFIG_DEFINITIONS(UNPACK_CONFIG_OVERRIDE_GETTERS)
        DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_CONFIG_OVERRIDE_GETTERS) // FORK
        BUFF_CONFIG_DEFINITIONS(UNPACK_CONFIG_OVERRIDE_GETTERS) // FORK
        /* Expands to:
        case CONFIG_CRIT_CHANCE:
            return sConfigChanges.critChance;
        */
        default: // Invalid config tag
            return 0;
        }
     }
}

#if TESTING
u32 GetClampedValue(enum ConfigTag _config, u32 newValue)
{
    u32 clampedValue = 0;
    switch (_config)
    {
    BATTLE_CONFIG_DEFINITIONS(UNPACK_CONFIG_CLAMPER)
    POKEMON_CONFIG_DEFINITIONS(UNPACK_CONFIG_CLAMPER)
    AI_CONFIG_DEFINITIONS(UNPACK_CONFIG_CLAMPER)
    DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_CONFIG_CLAMPER) // FORK
    BUFF_CONFIG_DEFINITIONS(UNPACK_CONFIG_CLAMPER) // FORK
    default:
        return 0;
    }
    return clampedValue;
}
#endif

void SetConfig(enum ConfigTag _config, u32 _value)
{
#if TESTING
    // Clamping is done here instead of the switch due to an internal compiler error!
    u32 clampedValue = GetClampedValue(_config, _value);
    switch (_config)
    {
    BATTLE_CONFIG_DEFINITIONS(UNPACK_CONFIG_SETTERS)
    POKEMON_CONFIG_DEFINITIONS(UNPACK_CONFIG_SETTERS)
    AI_CONFIG_DEFINITIONS(UNPACK_CONFIG_SETTERS)
    DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_CONFIG_SETTERS) // FORK
    BUFF_CONFIG_DEFINITIONS(UNPACK_CONFIG_SETTERS) // FORK
    /* Expands to:
    #if TESTING
    case CONFIG_CRIT_CHANCE:
        gConfigChangesTestOverride->critChance = clampedValue;
        break;
    #else
    case CONFIG_CRIT_CHANCE:
        return;
    #endif
    */
    default: // Invalid config tag
        return;
    }
#endif
}

#if TESTING
// FORK: force every fork flag (DETERMINISTIC_*, BUFF_*) off in the per-test
// baseline. Their production default is on (seeded from sConfigChanges above),
// but the inherited test suite was written against stock behavior, so tests run
// with these features off unless they opt in with WITH_CONFIG(FLAG, TRUE).
#define UNPACK_FORK_FLAG_FORCE_OFF(_name, _field, ...) gConfigChangesTestOverride->_field = FALSE;

void TestInitConfigData(void)
{
    gConfigChangesTestOverride = Alloc(sizeof(sConfigChanges));
    memcpy(gConfigChangesTestOverride, &sConfigChanges, sizeof(sConfigChanges));
    DETERMINISTIC_CONFIG_DEFINITIONS(UNPACK_FORK_FLAG_FORCE_OFF) // FORK
    BUFF_CONFIG_DEFINITIONS(UNPACK_FORK_FLAG_FORCE_OFF) // FORK
}

void TestFreeConfigData(void)
{
    TRY_FREE_AND_SET_NULL(gConfigChangesTestOverride)
}
#endif
