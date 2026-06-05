#include "global.h"
#include "test/battle.h"
#include "new_types.h"

// FORK: coverage for FEATURE_NEW_TYPES (config/feature.h). Feature flags default
// off in the test baseline (see TestInitConfigData), so each test that wants the
// re-typing opts in with WITH_CONFIG(FEATURE_NEW_TYPES, TRUE). The first override
// re-types Galarian Ponyta (stock pure Psychic) and Galarian Rapidash (stock
// Psychic/Fairy) to Fire/Fairy. Galarian Ponyta is the cleanest probe: as pure
// Psychic it takes a neutral Dragon hit and a neutral Water hit, while as Fire/
// Fairy it is immune to Dragon (Fairy) and weak to Water (Fire) — two unambiguous
// matchup flips. The override is data-only (the lookup ignores the flag), so the
// table itself is asserted directly.

ASSUMPTIONS
{
    ASSUME(GetMoveType(MOVE_DRAGON_BREATH) == TYPE_DRAGON);
    ASSUME(GetMoveType(MOVE_WATER_GUN) == TYPE_WATER);
    // The override data these tests rely on. If the table changes, update these.
    enum Type type;
    ASSUME(GetSpeciesTypeOverride(SPECIES_PONYTA_GALAR, 0, &type) && type == TYPE_FIRE);
    ASSUME(GetSpeciesTypeOverride(SPECIES_PONYTA_GALAR, 1, &type) && type == TYPE_FAIRY);
    // Galarian Ponyta is pure Psychic in stock data (both slots Psychic).
    ASSUME(gSpeciesInfo[SPECIES_PONYTA_GALAR].types[0] == TYPE_PSYCHIC);
    ASSUME(gSpeciesInfo[SPECIES_PONYTA_GALAR].types[1] == TYPE_PSYCHIC);
}

SINGLE_BATTLE_TEST("FEATURE_NEW_TYPES: Galarian Ponyta gains the Fairy half (immune to Dragon)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_NEW_TYPES, TRUE);
        PLAYER(SPECIES_PONYTA_GALAR); // re-typed Fire/Fairy
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_BREATH); }
    } SCENE {
        MESSAGE("It doesn't affect Ponyta…");
    }
}

SINGLE_BATTLE_TEST("FEATURE_NEW_TYPES: Galarian Ponyta gains the Fire half (weak to Water)")
{
    GIVEN {
        WITH_CONFIG(FEATURE_NEW_TYPES, TRUE);
        PLAYER(SPECIES_PONYTA_GALAR); // re-typed Fire/Fairy
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_WATER_GUN); }
    } SCENE {
        MESSAGE("It's super effective!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_NEW_TYPES: with the feature off, Galarian Ponyta keeps its stock Psychic typing")
{
    GIVEN {
        // Feature is off by default in the test baseline; assert stock behavior:
        // pure Psychic is neither immune to Dragon nor weak to Water.
        PLAYER(SPECIES_PONYTA_GALAR);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_BREATH); }
        TURN { MOVE(opponent, MOVE_WATER_GUN); }
    } SCENE {
        NONE_OF {
            MESSAGE("It doesn't affect Ponyta…");
            MESSAGE("It's super effective!");
        }
    }
}
