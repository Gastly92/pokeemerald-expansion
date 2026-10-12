#include "global.h"
#include "test/battle.h"

// FEATURE_FREE_GIMMICKS: a mon with every move blocked still reaches the move menu when a
// Z-Move or Dynamax would unlock one (src/fork/free_gimmick_struggle.c). The setup is the
// reported case: choice-locked into Tackle, then Tackle is disabled.

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a choice-locked mon whose move is disabled can still Z-Move it")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_Z_MOVE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BREAKNECK_BLITZ, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: a choice-locked mon whose move is disabled can still Dynamax")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MAX_STRIKE, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: picking a blocked move without arming a gimmick still Struggles")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
    }
}

SINGLE_BATTLE_TEST("FREE_GIMMICKS: with the flag off a choice-locked mon whose move is disabled goes straight to Struggle")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_CHOICE_BAND); Moves(MOVE_TACKLE, MOVE_CELEBRATE); Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DISABLE, MOVE_CELEBRATE); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_DISABLE); }
        TURN { MOVE(player, MOVE_TACKLE, allowed: FALSE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DISABLE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STRUGGLE, player);
    }
}
