#include "global.h"
#include "test/battle.h"
#include "fork/damage_preview.h"
#include "fork/deterministic_moves.h"
#include "fork/multi_hit_pp.h"

// NERF_MULTI_HIT_PP_PER_HIT: every strike of a multi-hit move costs PP, paid per strike
// thrown, and a strike the move cannot pay for is not thrown. See src/fork/multi_hit_pp.c.

ASSUMPTIONS
{
    ASSUME(IsMultiHitMove(MOVE_BULLET_SEED));
    ASSUME(GetMovePP(MOVE_BULLET_SEED) == 30);
    ASSUME(GetMoveStrikeCount(MOVE_DOUBLE_KICK) == 2);
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: a 5-hit move costs 5 PP")
{
    bool32 feature;
    PARAMETRIZE { feature = FALSE; }
    PARAMETRIZE { feature = TRUE; }
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, feature);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Moves(MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); }
    } SCENE {
        MESSAGE("The Pokémon was hit 5 times!");
    } THEN {
        EXPECT_EQ(player->pp[0], feature ? 25 : 29);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: with 2 PP left a 5-hit move hits twice")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); MovesWithPP({MOVE_BULLET_SEED, 2}); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); }
    } SCENE {
        MESSAGE("The Pokémon was hit 2 times!");
    } THEN {
        EXPECT_EQ(player->pp[0], 0);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: a fixed two-strike move with 1 PP left strikes once")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_HITMONLEE) { MovesWithPP({MOVE_DOUBLE_KICK, 1}); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        MESSAGE("The Pokémon was hit 1 time!");
    } THEN {
        EXPECT_EQ(player->pp[0], 0);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: a KO ends the move and only the strikes thrown are paid for")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Moves(MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The Pokémon was hit 1 time!");
    } THEN {
        EXPECT_EQ(player->pp[0], 29);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: a strike Protect blocks costs only the first PP")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Speed(10); Moves(MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_PROTECT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_PROTECT); MOVE(player, MOVE_BULLET_SEED); }
    } THEN {
        EXPECT_EQ(player->pp[0], 29);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: Pressure taxes every strike")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Moves(MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_ABSOL) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); }
    } SCENE {
        MESSAGE("The Pokémon was hit 5 times!");
    } THEN {
        EXPECT_EQ(player->pp[0], 20); // 5 strikes x (1 + Pressure)
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: a move called by Sleep Talk spends only Sleep Talk's PP")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        ASSUME(GetMovePP(MOVE_SLEEP_TALK) == 10);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Status1(STATUS1_SLEEP); Moves(MOVE_SLEEP_TALK, MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SLEEP_TALK); }
    } SCENE {
        MESSAGE("The Pokémon was hit 5 times!");
    } THEN {
        EXPECT_EQ(player->pp[0], 9);
        EXPECT_EQ(player->pp[1], 30);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: Parental Bond's second hit is not a multi-hit move and stays free")
{
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        ASSUME(GetMovePP(MOVE_TACKLE) == 35);
        PLAYER(SPECIES_KANGASKHAN_MEGA) { Ability(ABILITY_PARENTAL_BOND); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("The Pokémon was hit 2 times!");
    } THEN {
        EXPECT_EQ(player->pp[0], 34);
    }
}

SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: the move menu and damage preview project only the strikes it can pay for")
{
    u32 loFull = 0, hiFull = 0, loLow = 0, hiLow = 0;
    enum DamagePreviewKO ko;
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        PLAYER(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); Moves(MOVE_BULLET_SEED, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetProjectedMultiHitCount(B_BATTLER_0, B_BATTLER_1, MOVE_BULLET_SEED), 5);
        EXPECT_EQ(GetProjectedMovePPCost(B_BATTLER_0, MOVE_BULLET_SEED, 5), 5);
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_BULLET_SEED, GIMMICK_NONE, &loFull, &hiFull, &ko));

        gBattleMons[B_BATTLER_0].pp[0] = 2;
        EXPECT_EQ(GetProjectedMultiHitCount(B_BATTLER_0, B_BATTLER_1, MOVE_BULLET_SEED), 2);
        EXPECT_EQ(GetProjectedMovePPCost(B_BATTLER_0, MOVE_BULLET_SEED, 2), 2);
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_BULLET_SEED, GIMMICK_NONE, &loLow, &hiLow, &ko));

        // Two strikes out of five: the preview shrinks to about two fifths.
        EXPECT_LT(hiLow, hiFull);
        EXPECT_LE(hiLow * 5, hiFull * 2 + 5);
        EXPECT_GE(hiLow * 5 + 5, hiFull * 2);
    }
}

AI_SINGLE_BATTLE_TEST("NERF_MULTI_HIT_PP_PER_HIT: the AI prices a multi-hit move at the strikes it can pay for")
{
    u32 pp;
    PARAMETRIZE { pp = 30; }
    PARAMETRIZE { pp = 1; }
    GIVEN {
        WITH_CONFIG(NERF_MULTI_HIT_PP_PER_HIT, TRUE);
        ASSUME(GetMovePower(MOVE_BULLET_SEED) * 5 > GetMovePower(MOVE_SEED_BOMB));
        ASSUME(GetMovePower(MOVE_BULLET_SEED) < GetMovePower(MOVE_SEED_BOMB));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CINCCINO) { Ability(ABILITY_SKILL_LINK); MovesWithPP({MOVE_BULLET_SEED, pp}, {MOVE_SEED_BOMB, 15}); }
    } WHEN {
        TURN { EXPECT_MOVE(opponent, pp == 30 ? MOVE_BULLET_SEED : MOVE_SEED_BOMB); }
    }
}
