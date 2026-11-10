#include "global.h"
#include "test/battle.h"
#include "fork/frontier_battle_info.h" // ApplyAccuracyItemReveals (BUFF_ACCURACY_ITEMS_REVEAL)

// FEATURE_FOREWARN_MATCHUP (config/feature.h, src/fork/forewarn.c). FEATURE_* flags are off in
// the test baseline, so each test opts in. None of these use PASSES_RANDOMLY: the pick draws no
// RNG, so a random tie-break would show up as a flaky failure here.

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: Forewarn reveals the move that hurts the user most, not the highest base power")
{
    u32 config;
    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, config);
        ASSUME(GetMovePower(MOVE_HYDRO_PUMP) > GetMovePower(MOVE_CRUNCH));
        ASSUME(GetMoveType(MOVE_CRUNCH) == TYPE_DARK);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_HYDRO_PUMP, MOVE_CRUNCH); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(player, ABILITY_FOREWARN);
        if (config)
            MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Crunch!");
        else
            MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Hydro Pump!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: the foe's real Attack and Sp. Atk do not change the pick")
{
    // Wobbuffet's Attack and Sp. Atk bases are equal, so bounded at its strongest both moves hit
    // as hard as Musharna's own Defense / Sp. Def allow, and its lower Defense makes Crunch the
    // pick. Reading the foe's real, special-leaning stats would have tipped it to Dark Pulse.
    u32 atk, spAtk;
    PARAMETRIZE { atk = 50;  spAtk = 400; }
    PARAMETRIZE { atk = 400; spAtk = 50; }
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        ASSUME(GetMovePower(MOVE_DARK_PULSE) == GetMovePower(MOVE_CRUNCH));
        ASSUME(GetSpeciesBaseAttack(SPECIES_WOBBUFFET) == GetSpeciesBaseSpAttack(SPECIES_WOBBUFFET));
        ASSUME(GetSpeciesBaseDefense(SPECIES_MUSHARNA) < GetSpeciesBaseSpDefense(SPECIES_MUSHARNA));
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DARK_PULSE, MOVE_CRUNCH); Attack(atk); SpAttack(spAtk); }
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Crunch!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: an all-status foe reveals its first move")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_PROTECT, MOVE_TOXIC, MOVE_SWORDS_DANCE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Protect!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: when no move can damage the user, the first move is revealed")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        ASSUME(GetMoveType(MOVE_BODY_SLAM) == TYPE_NORMAL);
        ASSUME(GetMoveType(MOVE_CLOSE_COMBAT) == TYPE_FIGHTING);
        PLAYER(SPECIES_GENGAR) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_BODY_SLAM, MOVE_CLOSE_COMBAT); }
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Forewarn alerted Gengar to the opposing Wobbuffet's Celebrate!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: equal threats go to the earlier move slot")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        ASSUME(GetMovePower(MOVE_TACKLE) == GetMovePower(MOVE_POUND));
        ASSUME(GetMovePower(MOVE_TACKLE) == GetMovePower(MOVE_SCRATCH));
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_ZUBAT) { Moves(MOVE_POUND, MOVE_TACKLE, MOVE_SCRATCH, MOVE_CELEBRATE); }
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Forewarn alerted Musharna to the opposing Zubat's Pound!");
    }
}

DOUBLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: equal threats across foes go to the first foe")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
    } WHEN {
        TURN {}
    } SCENE {
        ABILITY_POPUP(playerLeft, ABILITY_FOREWARN);
        MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Tackle!");
    } THEN {
        EXPECT_EQ(gBattleStruct->infoForewarnedMoves[B_SIDE_OPPONENT][0], 1u << 1);
        EXPECT_EQ(gBattleStruct->infoForewarnedMoves[B_SIDE_OPPONENT][1], 0);
    }
}

SINGLE_BATTLE_TEST("FEATURE_FOREWARN_MATCHUP: Counter-style moves count as a real threat")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        ASSUME(GetMoveEffect(MOVE_COUNTER) == EFFECT_REFLECT_DAMAGE);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_COUNTER); }
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Forewarn alerted Musharna to the opposing Wobbuffet's Counter!");
    }
}

SINGLE_BATTLE_TEST("Forewarn's reveal shows in the INFO viewer's move list, with or without FEATURE_FOREWARN_MATCHUP")
{
    u32 config;
    PARAMETRIZE { config = TRUE; }
    PARAMETRIZE { config = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, config);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_HYDRO_PUMP); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } THEN {
        EXPECT_EQ(gBattleStruct->infoForewarnedMoves[B_SIDE_OPPONENT][0], 1u << 1);
        EXPECT_EQ(gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][0], 0);
    }
}

SINGLE_BATTLE_TEST("Forewarn's reveal does not count as the foe having acted for the Zoom Lens")
{
    GIVEN {
        WITH_CONFIG(FEATURE_FOREWARN_MATCHUP, TRUE);
        WITH_CONFIG(BUFF_ACCURACY_ITEMS_REVEAL, TRUE);
        PLAYER(SPECIES_MUSHARNA) { Ability(ABILITY_FOREWARN); Item(ITEM_ZOOM_LENS); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_HYDRO_PUMP); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } THEN {
        ApplyAccuracyItemReveals();
        EXPECT_EQ(gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][0], 0);
        EXPECT_EQ(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u, 0);
        EXPECT_EQ(gBattleStruct->infoForewarnedMoves[B_SIDE_OPPONENT][0], 1u << 1);
    }
}
