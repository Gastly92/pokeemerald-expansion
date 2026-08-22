#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "fork/battle_ai_gimmick.h"

// FORK: the AI scores a gimmick move's type matchup off the move the engine will really
// execute, not off the move that was chosen.
//
// A Z-Move or Max Move inherits its base move's *type* but none of its matchup quirks:
// Subzero Slammer and Max Hailstorm are plain Ice moves, so Freeze-Dry's bonus against
// Water-types is gone the moment the move is upgraded. Reading the base move's matchup
// made the AI predict 2x where the real hit lands for 0.5x - a 4x error - so it happily
// spent its Z-Move on, and Dynamaxed into, moves the target resists.
//
// Freeze-Dry (EFFECT_SUPER_EFFECTIVE_ON_ARG) is the clearest case; the same swap also
// covers Flying Press's second Flying pass, Thousand Arrows' grounding of Flying-types
// and Synchronoise's same-type-only rule, which converted moves likewise do not keep.

AI_SINGLE_BATTLE_TEST("AI GIMMICK MATCHUP: does not spend the Z-Move on Freeze-Dry against a Water-type")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT);
        ASSUME(GetMoveEffect(MOVE_FREEZE_DRY) == EFFECT_SUPER_EFFECTIVE_ON_ARG);
        ASSUME(GetMoveArgType(MOVE_FREEZE_DRY) == TYPE_WATER);
        ASSUME(GetMoveEffect(MOVE_SUBZERO_SLAMMER) != EFFECT_SUPER_EFFECTIVE_ON_ARG);
        ASSUME(gSpeciesInfo[SPECIES_VAPOREON].types[0] == TYPE_WATER);
        ASSUME(gSpeciesInfo[SPECIES_VAPOREON].types[1] == TYPE_WATER);
        // Bulky enough that neither the plain move nor the Z-Move is anywhere near a KO,
        // so the only thing deciding the upgrade is the matchup.
        PLAYER(SPECIES_VAPOREON) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_ICIUM_Z); Moves(MOVE_FREEZE_DRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_FREEZE_DRY, gimmick: GIMMICK_NONE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK MATCHUP: still spends the Z-Move on Freeze-Dry against a target that really is weak to Ice")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_BASIC_TRAINER | AI_FLAG_OMNISCIENT);
        ASSUME(gSpeciesInfo[SPECIES_ALTARIA].types[1] == TYPE_FLYING);
        // Control for the test above: Subzero Slammer is super effective here on its own
        // Ice typing, so declining it would be the wrong call.
        PLAYER(SPECIES_ALTARIA) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_ICIUM_Z); Moves(MOVE_FREEZE_DRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_FREEZE_DRY, gimmick: GIMMICK_Z_MOVE); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK MATCHUP: a Dynamaxed AI picks the Max Move that is really super effective")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // Freeze-Dry is the stronger Max Move on paper - 70 BP lands in a higher power
        // tier than Shock Wave's 60, so Max Hailstorm out-powers Max Lightning. It is only
        // the matchup that separates them, and only if the AI reads Max Hailstorm as the
        // plain Ice move it is (0.5x) rather than as Freeze-Dry (2x).
        ASSUME(GetMovePower(MOVE_FREEZE_DRY) > GetMovePower(MOVE_SHOCK_WAVE));
        ASSUME(GetMoveType(MOVE_SHOCK_WAVE) == TYPE_ELECTRIC);
        ASSUME(gSpeciesInfo[SPECIES_VAPOREON].types[0] == TYPE_WATER);
        PLAYER(SPECIES_VAPOREON) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FREEZE_DRY, MOVE_SHOCK_WAVE); DynamaxLevel(10); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_SHOCK_WAVE, gimmick: GIMMICK_DYNAMAX); }
    }
}

AI_SINGLE_BATTLE_TEST("AI GIMMICK MATCHUP: an un-gimmicked AI still credits Freeze-Dry against a Water-type")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        // Control for the Dynamax test: with no gimmick converting it, Freeze-Dry keeps its
        // bonus against Water and outclasses the resisted-nothing Shock Wave.
        PLAYER(SPECIES_VAPOREON) { MaxHP(500); HP(500); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_FREEZE_DRY, MOVE_SHOCK_WAVE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_FREEZE_DRY); }
    }
}
