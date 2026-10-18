#include "global.h"
#include "test/battle.h"

// Field moves don't target a battler, so a foe's Substitute must not make them fail.
// Guards the UPSTREAM: early return in CancelerSubstitute (src/battle_move_resolution.c).

SINGLE_BATTLE_TEST("Stealth Rock is not blocked by the target's Substitute")
{
    GIVEN {
        PLAYER(SPECIES_REGISTEEL);
        OPPONENT(SPECIES_JOLTEON);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_STEALTH_ROCK); }
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STEALTH_ROCK, player);
        MESSAGE("Pointed stones float in the air on the opposing side!");
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("Field moves are not blocked by the target's Substitute")
{
    u32 move;
    PARAMETRIZE { move = MOVE_SPIKES; }
    PARAMETRIZE { move = MOVE_TOXIC_SPIKES; }
    PARAMETRIZE { move = MOVE_STICKY_WEB; }
    PARAMETRIZE { move = MOVE_RAIN_DANCE; }
    PARAMETRIZE { move = MOVE_ELECTRIC_TERRAIN; }
    PARAMETRIZE { move = MOVE_TRICK_ROOM; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_JOLTEON) { Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, move); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, move, player);
    }
}

SINGLE_BATTLE_TEST("Status moves aimed at the target are still blocked by its Substitute")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_JOLTEON) { Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_TOXIC); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        NONE_OF { ANIMATION(ANIM_TYPE_MOVE, MOVE_TOXIC, player); }
        MESSAGE("But it failed!");
    }
}
