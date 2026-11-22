#include "global.h"
#include "test/battle.h"

// "became nimble!" names gEffectBattler, which Autotomize never set, so the message named
// whichever battler a previous effect had left there (e.g. the foe hit by an earlier
// Nuzzle). Regression test for the UPSTREAM: fix in SetAdditionalEffectsOnStatChange.
SINGLE_BATTLE_TEST("Autotomize's message names the user, not a stale effect battler")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_AUTOTOMIZE) == EFFECT_AUTOTOMIZE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
        TURN { MOVE(player, MOVE_AUTOTOMIZE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AUTOTOMIZE, player);
        MESSAGE("Wobbuffet's Speed rose sharply!");
        MESSAGE("Wobbuffet became nimble!");
    }
}

SINGLE_BATTLE_TEST("Autotomize's message names the opposing user")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_AUTOTOMIZE) == EFFECT_AUTOTOMIZE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_NUZZLE); }
        TURN { MOVE(opponent, MOVE_AUTOTOMIZE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_NUZZLE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AUTOTOMIZE, opponent);
        MESSAGE("The opposing Wynaut's Speed rose sharply!");
        MESSAGE("The opposing Wynaut became nimble!");
    }
}
