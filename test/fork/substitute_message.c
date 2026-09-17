#include "global.h"
#include "test/battle.h"

// FORK: regression coverage for the Substitute-damaged line naming a battler that has no
// Substitute. CancelerHealthBarUpdate() in src/battle_move_resolution.c used to test
// DoesSubstituteBlockMove() before ShouldSkipBattlerForDamage(), so every battler on the
// field was asked whether its Substitute blocked the move -- including the attacker itself
// and untargeted allies. STRINGID_SUBSTITUTEDAMAGED reads {B_DEF_NAME_WITH_PREFIX2}
// (gBattlerTarget), not the battler passed to PrepareStringBattle(), so a stray hit printed
// the line against whatever the move was actually aimed at.

SINGLE_BATTLE_TEST("Substitute: a user behind its own Substitute doesn't report one on the target")
{
    GIVEN {
        PLAYER(SPECIES_KELDEO);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SUBSTITUTE); }
        TURN { MOVE(player, MOVE_HYDRO_PUMP); }
    } SCENE {
        MESSAGE("Keldeo put in a substitute!");
        MESSAGE("Keldeo used Hydro Pump!");
        HP_BAR(opponent); // the foe is damaged normally
        NONE_OF { MESSAGE("The substitute took damage for the opposing Wobbuffet!"); }
    }
}

DOUBLE_BATTLE_TEST("Substitute: an untargeted ally's Substitute doesn't report one on the target")
{
    GIVEN {
        PLAYER(SPECIES_KELDEO);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_SNORLAX);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_SUBSTITUTE); MOVE(playerLeft, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_HYDRO_PUMP, target: opponentLeft); }
    } SCENE {
        MESSAGE("Wynaut put in a substitute!");
        MESSAGE("Keldeo used Hydro Pump!");
        HP_BAR(opponentLeft);
        NONE_OF { MESSAGE("The substitute took damage for the opposing Snorlax!"); }
    }
}

// The control: a target that really is behind a Substitute must still say so.
SINGLE_BATTLE_TEST("Substitute: a target behind a Substitute still reports it")
{
    GIVEN {
        PLAYER(SPECIES_KELDEO);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_HYDRO_PUMP); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet put in a substitute!");
        MESSAGE("Keldeo used Hydro Pump!");
        MESSAGE("The substitute took damage for the opposing Wobbuffet!");
    }
}
