#include "global.h"
#include "test/battle.h"
#include "innate_abilities.h"

// FORK: coverage for FEATURE_INNATE_ABILITIES (config/feature.h). Feature flags
// default off in the test baseline (see TestInitConfigData), so each test that
// wants innates opts in with WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE). The
// supported innate set is currently just LEVITATE (see src/innate_abilities.c).

// Flavor-floater coverage: a species with no native Levitate that floats by design
// (Magnemite hovers magnetically; primary is Magnet Pull/Sturdy/Analytic, and it's
// Electric/Steel so Ground is super effective) gains Ground immunity from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: a flavor floater (Magnemite) gets innate Levitate")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGNEMITE, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_MAGNEMITE].abilities[0] != ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEMITE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_LEVITATE);
        MESSAGE("It doesn't affect Magnemite…");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Levitate (stock behavior)")
{
    GIVEN {
        // Feature is off by default in the test baseline; assert stock behavior.
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEVITATE); }
        HP_BAR(player); // the super effective Ground hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Mold Breaker pierces an innate Levitate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_TINKATON) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEVITATE); }
        HP_BAR(player); // grounded through Mold Breaker, the hit connects
    }
}

// Comprehensive-table coverage: every species that natively carries Levitate is also
// listed as an innate (forward-looking — a later flag re-homes their primary). Today
// that's redundant, but it becomes observable the moment the primary is overwritten:
// the innate must keep the mon airborne. Worry Seed strips Gastly's primary Levitate
// (-> Insomnia); the feature-off leg's HP_BAR doubles as proof the primary really was
// removed (otherwise it would still be immune), so the feature-on leg's immunity can
// only come from the innate.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Levitate persists after the primary Levitate is overwritten")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        ASSUME(SpeciesHasInnate(SPECIES_GASTLY, ABILITY_LEVITATE));
        ASSUME(gSpeciesInfo[SPECIES_GASTLY].abilities[0] == ABILITY_LEVITATE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_GASTLY); // primary Levitate AND (now) innate Levitate
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_WORRY_SEED, MOVE_MUD_SLAP); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_WORRY_SEED); } // primary Levitate -> Insomnia (pops the old Levitate)
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        // Worry Seed pops the *old* ability (Levitate) as it overwrites, so we can't key
        // on the pop-up; the turn-2 outcome is the discriminator. Both legs run the same
        // Worry Seed, so the feature-off HP_BAR proves the primary Levitate really was
        // removed — making the feature-on immunity attributable only to the innate.
        if (enabled)
            MESSAGE("It doesn't affect Gastly…"); // innate Levitate still blocks the Ground hit
        else
            HP_BAR(player); // grounded: Insomnia replaced Levitate and there's no innate
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Levitate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_MAGNEZONE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        MESSAGE("Magnezone's Ability was suppressed!"); // player side: no "opposing" prefix
        HP_BAR(player); // suppressed, so the Ground hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies only the primary ability, never an innate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
        OPPONENT(SPECIES_MAGNEZONE); // primary Magnet Pull, innate Levitate
    } WHEN {
        TURN {}
    } SCENE {
        // Trace resolves deterministically to the single primary ability slot,
        // never an innate.
        ABILITY_POPUP(player, ABILITY_TRACE);
        MESSAGE("It traced the opposing Magnezones's Magnet Pull!");
    }
}

AI_DOUBLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI will spread-Earthquake when its ally's innate Levitate makes it immune")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        // Earthquake hits the ally too (TARGET_FOES_AND_ALLY). The AI normally avoids it
        // when it would damage its own ally for no benefit (the Magnezone ally is
        // Steel/Electric, so Ground is super effective). With the feature on, the ally's
        // innate Levitate makes it immune, so the AI is willing to use Earthquake — proving
        // the partner-ability scoring honors the innate (battle_ai_main.c).
        ASSUME(GetMoveTarget(MOVE_EARTHQUAKE) == TARGET_FOES_AND_ALLY);
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_PHANPY) { Moves(MOVE_EARTHQUAKE, MOVE_SCRATCH); }
        OPPONENT(SPECIES_MAGNEZONE) { Moves(MOVE_CELEBRATE); } // innate Levitate when the feature is on
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponentLeft, MOVE_EARTHQUAKE); }                  // ally immune via innate Levitate
        else
            TURN { EXPECT_MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft); } // would hit the ally, so avoided
    }
}

AI_SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: AI treats an innate Levitate as Ground immunity")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        // The AI's damage prediction runs through the shared type calc, where the
        // innate immunity is applied off the battler's real species — so it sees the
        // immunity with no AI-specific plumbing. With the feature off, Earthquake is
        // 4x super effective on Steel/Electric Magnezone and the AI prefers it.
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_MAGNEZONE); // innate Levitate when the feature is on
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE, MOVE_SURF); }
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_SURF); }       // Earthquake would do nothing
        else
            TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); } // 4x super effective on Steel/Electric
    }
}
