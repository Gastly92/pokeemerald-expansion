#include "global.h"
#include "test/battle.h"

#define AI_FRAME_CEILING_SINGLES_NO_FLAGS                       3
#define AI_FRAME_CEILING_SINGLES_SMART_TRAINER                  10
#define AI_FRAME_CEILING_DOUBLES_NO_FLAGS                       25 // +1: Batch Y6 innate switch-in stat sims (Intrepid Sword/Dauntless Shield) in SetBattlerStatStagesForSwitchin; +2: Batch Y8 Teravolt/Turboblaze add two IsInnateActive checks to IsMoldBreakerTypeAbility, which the AI runs
#define AI_FRAME_CEILING_DOUBLES_SMART_TRAINER                  42 // +1: Tier 5.1 innate Mega Sol adds an IsInnateActive check to GetAttackerWeather, which the AI's shared damage calc runs; +1: Tier 5.5 innate Mold Breaker adds an IsInnateActive check to IsMoldBreakerTypeAbility, which the AI runs; +1: Batch Y8 Teravolt/Turboblaze add two more IsInnateActive checks there; +1: upstream sync through 1.16.3, chiefly #10542 replacing the BATTLE_PARTNER XOR macro with the GetPartnerBattler() call chain (see note below)
#define AI_FRAME_CEILING_STEVEN_MULTI                           30 // +1: Tier 5.5 innate Mold Breaker adds an IsInnateActive check to IsMoldBreakerTypeAbility, which the AI runs; +1: Batch Y8 Teravolt/Turboblaze add two more IsInnateActive checks there; +1: upstream sync through 1.16.3 (see note below)
#define AI_FRAME_CEILING_STEVEN_MULTI_SMART_TRAINER             34 // +1: Batch S innate AI reads (Infiltrator/Skill Link/etc.); +1: Batch U side-wide AI reads (Aroma Veil/Flower Veil/Telepathy); +1: Tier 5.5 innate Mold Breaker adds an IsInnateActive check to IsMoldBreakerTypeAbility, which the AI runs; +1: upstream sync through 1.16.3 (see note below). (Batch W2 gave GetSpeciesAbility's species_ability_overrides lookup an O(1) bitmap fast-path, so the growing override table no longer taxes this scenario -- it stays flat despite W2's added rows, and future Batch W growth is free.)
// FORK note on the 1.16.3 sync +1s: upstream #10542 removed the BATTLE_PARTNER/BATTLE_OPPOSITE XOR macros in
// favour of GetPartnerBattler()/GetOppositeBattler(), which resolve through
// GetBattlerAtPosition(GetPartnerPosition(GetBattlerPosition(b))) -- three calls where there used to be one XOR.
// The AI walks these constantly, and the fork's innate checks add more call sites on top, so the three doubles/
// multi scenarios each crossed one frame. Redundant lookups in the fork's own AI-hot helpers (AI_IsInnateOnSide,
// IsInnateOnSide, the innate partner-ability scoring block in battle_ai_main.c) were hoisted to locals first;
// that trims real work but does not recover a whole frame, hence the +1. Singles and doubles-no-flags are
// unaffected, so this is the refactor's fixed cost, not an AI-logic explosion.
#define AI_FRAME_CEILING_CHECK                                  FALSE // If TRUE, forces all thinking time tests to fail. Useful for printing all actual frame times to console by running the tests

AI_SINGLE_BATTLE_TEST("AI thinking time doesn't explode (singles, no flags)")
{
    GIVEN {
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_SINGLES_NO_FLAGS);
    }
}

AI_SINGLE_BATTLE_TEST("AI thinking time doesn't explode (singles, smart)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_SMART_TRAINER);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_SINGLES_SMART_TRAINER);
    }
}

AI_DOUBLE_BATTLE_TEST("AI thinking time doesn't explode (doubles, no flags)")
{
    GIVEN {
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_DOUBLES_NO_FLAGS);
    }
}

AI_DOUBLE_BATTLE_TEST("AI thinking time doesn't explode (doubles, smart)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_SMART_TRAINER);
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        PLAYER(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_CELEBRATE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
        OPPONENT(SPECIES_ZIGZAGOON) { Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_FLING); }
    } WHEN {
        TURN { }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_DOUBLES_SMART_TRAINER);
    }
}

AI_MULTI_BATTLE_TEST("AI thinking time doesn't explode (Steven multi)")
{
    GIVEN {
        BATTLER_AI_FLAGS(playerRight, AI_FLAG_BASIC_TRAINER);
        BATTLER_AI_FLAGS(opponentLeft, AI_FLAG_BASIC_TRAINER);
        BATTLER_AI_FLAGS(opponentRight, AI_FLAG_CHECK_BAD_MOVE);
        PLAYER(SPECIES_ZIGZAGOON) { Level(100); Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_CELEBRATE); }
        PARTNER(SPECIES_METANG) { Level(42); Moves(MOVE_CELEBRATE, MOVE_PSYCHIC, MOVE_REFLECT, MOVE_METAL_CLAW); }
        PARTNER(SPECIES_SKARMORY) { Level(43); Moves(MOVE_TOXIC, MOVE_AERIAL_ACE, MOVE_PROTECT, MOVE_STEEL_WING); }
        PARTNER(SPECIES_AGGRON) { Level(44); Moves(MOVE_THUNDER, MOVE_PROTECT, MOVE_SOLARBEAM, MOVE_DRAGON_CLAW); }
        OPPONENT_A(SPECIES_MIGHTYENA) { Level(42); Moves(MOVE_CELEBRATE, MOVE_SCARY_FACE, MOVE_ASSURANCE, MOVE_SWAGGER); }
        OPPONENT_A(SPECIES_CROBAT) { Level(43); Moves(MOVE_HAZE, MOVE_BITE, MOVE_AIR_CUTTER, MOVE_QUICK_GUARD); }
        OPPONENT_A(SPECIES_CAMERUPT) { Level(44); Moves(MOVE_YAWN, MOVE_TAKE_DOWN, MOVE_CURSE, MOVE_EARTH_POWER); }
        OPPONENT_B(SPECIES_CAMERUPT) { Level(36); Moves(MOVE_TAKE_DOWN, MOVE_CELEBRATE, MOVE_EARTH_POWER, MOVE_LAVA_PLUME); }
        OPPONENT_B(SPECIES_MIGHTYENA) { Level(38); Moves(MOVE_TAUNT, MOVE_SCARY_FACE, MOVE_ASSURANCE, MOVE_SWAGGER); }
        OPPONENT_B(SPECIES_GOLBAT) { Level(40); Moves(MOVE_BITE, MOVE_AIR_CUTTER, MOVE_QUICK_GUARD, MOVE_POISON_FANG); }
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_HI, 0);
    } WHEN {
        TURN { EXPECT_MOVE(playerRight, MOVE_REFLECT); }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_STEVEN_MULTI);
    }
}

AI_MULTI_BATTLE_TEST("AI thinking time doesn't explode (Steven multi, smart)")
{
    GIVEN {
        BATTLER_AI_FLAGS(playerRight, AI_FLAG_SMART_TRAINER);
        BATTLER_AI_FLAGS(opponentLeft, AI_FLAG_SMART_TRAINER);
        BATTLER_AI_FLAGS(opponentRight, AI_FLAG_SMART_TRAINER);
        PLAYER(SPECIES_ZIGZAGOON) { Level(100); Moves(MOVE_DOUBLE_EDGE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_CELEBRATE); }
        PARTNER(SPECIES_METANG) { Level(42); Moves(MOVE_CELEBRATE, MOVE_PSYCHIC, MOVE_REFLECT, MOVE_METAL_CLAW); }
        PARTNER(SPECIES_SKARMORY) { Level(43); Moves(MOVE_TOXIC, MOVE_AERIAL_ACE, MOVE_PROTECT, MOVE_STEEL_WING); }
        PARTNER(SPECIES_AGGRON) { Level(44); Moves(MOVE_THUNDER, MOVE_PROTECT, MOVE_SOLARBEAM, MOVE_DRAGON_CLAW); }
        OPPONENT_A(SPECIES_MIGHTYENA) { Level(42); Moves(MOVE_CELEBRATE, MOVE_SCARY_FACE, MOVE_ASSURANCE, MOVE_SWAGGER); }
        OPPONENT_A(SPECIES_CROBAT) { Level(43); Moves(MOVE_HAZE, MOVE_BITE, MOVE_AIR_CUTTER, MOVE_QUICK_GUARD); }
        OPPONENT_A(SPECIES_CAMERUPT) { Level(44); Moves(MOVE_YAWN, MOVE_TAKE_DOWN, MOVE_CURSE, MOVE_EARTH_POWER); }
        OPPONENT_B(SPECIES_CAMERUPT) { Level(36); Moves(MOVE_TAKE_DOWN, MOVE_CELEBRATE, MOVE_EARTH_POWER, MOVE_LAVA_PLUME); }
        OPPONENT_B(SPECIES_MIGHTYENA) { Level(38); Moves(MOVE_TAUNT, MOVE_SCARY_FACE, MOVE_ASSURANCE, MOVE_SWAGGER); }
        OPPONENT_B(SPECIES_GOLBAT) { Level(40); Moves(MOVE_BITE, MOVE_AIR_CUTTER, MOVE_QUICK_GUARD, MOVE_POISON_FANG); }
        TIE_BREAK_SCORE(RNG_AI_SCORE_TIE_DOUBLES_MOVE, SCORE_TIE_HI, 0);
    } WHEN {
        TURN { EXPECT_MOVE(playerRight, MOVE_REFLECT); }
    } THEN {
        EXPECT_LE(gBattleStruct->aiDelayFrames, AI_FRAME_CEILING_CHECK ? 0 : AI_FRAME_CEILING_STEVEN_MULTI_SMART_TRAINER);
    }
}
