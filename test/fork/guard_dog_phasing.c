#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Coverage for FEATURE_ANNOUNCE_GUARD_DOG (config/feature.h, src/fork/guard_dog_phasing.c).
// Feature flags default off in the test baseline, so each test opts in with WITH_CONFIG; the
// stock "But it failed!" path stays covered by test/battle/move_effect/roar.c.

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_GUARD_DOG: Guard Dog announces blocking Whirlwind")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_GUARD_DOG, announce);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_GUARD_DOG); }
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_WHIRLWIND); }
    } SCENE {
        MESSAGE("Wobbuffet used Whirlwind!");
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
            MESSAGE("The opposing Okidogi refuses to be forced out!");
        }
        else
        {
            NOT ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
            MESSAGE("But it failed!");
        }
        NONE_OF {
            MESSAGE("But it failed!");
            MESSAGE("The opposing Charmander was dragged out!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_GUARD_DOG: Guard Dog announces blocking Roar")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_GUARD_DOG, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_GUARD_DOG); }
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_ROAR); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
        MESSAGE("The opposing Okidogi refuses to be forced out!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_GUARD_DOG: innate Guard Dog announces blocking Whirlwind")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_OKIDOGI, ABILITY_GUARD_DOG));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_ANNOUNCE_GUARD_DOG, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_TOXIC_CHAIN); } // chosen differs from the innate
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_WHIRLWIND); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
        MESSAGE("The opposing Okidogi refuses to be forced out!");
        NOT MESSAGE("The opposing Charmander was dragged out!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_GUARD_DOG: Guard Dog announces blocking Dragon Tail's switch")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_GUARD_DOG, announce);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_OKIDOGI) { Ability(ABILITY_GUARD_DOG); }
        OPPONENT(SPECIES_CHARMANDER);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_TAIL); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_TAIL, player);
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
            MESSAGE("The opposing Okidogi refuses to be forced out!");
            NOT MESSAGE("The opposing Charmander was dragged out!");
        }
        else
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_GUARD_DOG);
                MESSAGE("The opposing Okidogi refuses to be forced out!");
                MESSAGE("The opposing Charmander was dragged out!");
            }
        }
    }
}
