#include "global.h"
#include "test/battle.h"
#include "innate_abilities.h"

// FORK: coverage for FEATURE_INNATE_ABILITIES (config/innate.h). Feature flags
// default off in the test baseline (see TestInitConfigData), so each test that
// wants innates opts in with WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE). The
// seed table (src/innate_abilities.c) gives the Beldum line an innate Levitate on
// top of its native Clear Body, so a Ground move is the natural probe: Metagross
// is Steel/Psychic (Ground is super effective), so the difference between "immune
// via innate Levitate" and "takes a super effective hit" is unambiguous.

ASSUMPTIONS
{
    ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
    // The seed data these tests rely on. If the table changes, update the tests.
    ASSUME(SpeciesHasInnate(SPECIES_METAGROSS, ABILITY_LEVITATE));
    ASSUME(SpeciesHasInnate(SPECIES_METAGROSS, ABILITY_SPEED_BOOST)); // active end-turn innate (slot 1)
    ASSUME(gSpeciesInfo[SPECIES_METAGROSS].abilities[0] == ABILITY_CLEAR_BODY);
    ASSUME(SpeciesHasInnate(SPECIES_AGGRON, ABILITY_INTIMIDATE));
    ASSUME(gSpeciesInfo[SPECIES_AGGRON].abilities[0] != ABILITY_INTIMIDATE);
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Levitate grants Ground immunity")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_METAGROSS); // native Clear Body, innate Levitate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        // The pop-up is forced to show Levitate (the innate that blocked), not the
        // primary Clear Body, via abilityPopupOverwrite.
        ABILITY_POPUP(player, ABILITY_LEVITATE);
        MESSAGE("It doesn't affect Metagross…");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: with the feature off, no innate Levitate (stock behavior)")
{
    GIVEN {
        // Feature is off by default in the test baseline; assert stock behavior.
        PLAYER(SPECIES_METAGROSS);
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
        PLAYER(SPECIES_METAGROSS);
        OPPONENT(SPECIES_TINKATON) { Ability(ABILITY_MOLD_BREAKER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_LEVITATE); }
        HP_BAR(player); // grounded through Mold Breaker, the hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Gastro Acid suppresses an innate Levitate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_METAGROSS);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GASTRO_ACID); }
        TURN { MOVE(opponent, MOVE_MUD_SLAP); }
    } SCENE {
        MESSAGE("Metagross's Ability was suppressed!"); // player side: no "opposing" prefix
        HP_BAR(player); // suppressed, so the Ground hit connects
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: Trace copies only the primary ability, never an innate")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_GARDEVOIR) { Ability(ABILITY_TRACE); }
        OPPONENT(SPECIES_METAGROSS); // primary Clear Body, innate Levitate
    } WHEN {
        TURN {}
    } SCENE {
        // Trace resolves deterministically to the single primary ability slot,
        // never an innate.
        ABILITY_POPUP(player, ABILITY_TRACE);
        MESSAGE("It traced the opposing Metagross's Clear Body!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an active innate (Intimidate) fires its switch-in effect")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_AGGRON); // native Sturdy, innate Intimidate
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        // Innate Intimidate activates on entry, with the pop-up showing Intimidate.
        ABILITY_POPUP(player, ABILITY_INTIMIDATE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponent);
        MESSAGE("The opposing Wobbuffet's Attack fell!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an active innate (Speed Boost) fires its end-turn effect")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_METAGROSS); // native Clear Body, innate Levitate + Speed Boost
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        // The end-turn innate fires with the pop-up forced to Speed Boost (not the
        // primary Clear Body), raising Metagross's Speed at the end of the turn.
        ABILITY_POPUP(player, ABILITY_SPEED_BOOST);
        MESSAGE("Metagross's Speed rose!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an end-turn innate does not fire when the feature is off")
{
    GIVEN {
        // Feature off in the test baseline → Metagross's native ability only.
        PLAYER(SPECIES_METAGROSS);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_SPEED_BOOST); }
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: an active innate does not fire when the feature is off")
{
    GIVEN {
        // Feature off in the test baseline → Aggron's native ability only.
        PLAYER(SPECIES_AGGRON);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_INTIMIDATE); }
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
        // 2x super effective on Steel/Psychic Metagross and the AI prefers it.
        ASSUME(GetMoveType(MOVE_EARTHQUAKE) == TYPE_GROUND);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY);
        PLAYER(SPECIES_METAGROSS); // innate Levitate when the feature is on
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE, MOVE_SURF); }
    } WHEN {
        if (enabled)
            TURN { EXPECT_MOVE(opponent, MOVE_SURF); }       // Earthquake would do nothing
        else
            TURN { EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); } // 2x super effective on Steel
    }
}
