#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "fork/battle_ai_gimmick.h"

// FORK: with FEATURE_FREE_GIMMICKS an AI Mega Evolution or Dynamax is not spent on the first
// turn it becomes available. Each turn it commits on a roll (RNG_AI_COMMIT_MEGA /
// RNG_AI_COMMIT_DYNAMAX), unless the gimmick secures a KO, the mon is threatened with a KO,
// or no teammate is left to use it later. See ShouldCommitGimmickNow.
//
// The player's side only ever Splashes and the AI's attacks are weak, so neither the KO
// nor the threat check fires unless a test sets it up on purpose.

AI_SINGLE_BATTLE_TEST("AI GIMMICK TIMING: Dynamax is held more often with teammates in reserve")
{
    u32 reserves, commitChance;
    PARAMETRIZE { reserves = 1; }
    PARAMETRIZE { reserves = 3; }
    PARAMETRIZE { reserves = 5; }
    commitChance = 100 - reserves * AI_FREE_DYNAMAX_HOLD_CHANCE_PER_MON;

    PASSES_RANDOMLY(commitChance, 100, RNG_AI_COMMIT_DYNAMAX);
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        for (u32 i = 0; i < reserves; i++)
            OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK TIMING: Mega Evolution hesitates a little before firing")
{
    PASSES_RANDOMLY(AI_FREE_MEGA_COMMIT_CHANCE, 100, RNG_AI_COMMIT_MEGA);
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_LOPUNNY) { Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_POUND, gimmick: GIMMICK_MEGA); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK TIMING: the last mon standing Dynamaxes at once")
{
    PASSES_RANDOMLY(100, 100, RNG_AI_COMMIT_DYNAMAX);
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(0); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK TIMING: a mon threatened with a KO Dynamaxes at once")
{
    PASSES_RANDOMLY(100, 100, RNG_AI_COMMIT_DYNAMAX);
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(500); HP(500); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(5); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); EXPECT_MOVE(opponent, MOVE_TACKLE, gimmick: GIMMICK_DYNAMAX); }
    }
}
