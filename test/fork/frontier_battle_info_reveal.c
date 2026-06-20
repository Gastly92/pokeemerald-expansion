#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "battle_ai_record.h"
#include "fork/innate_abilities.h"

// FORK: B_FRONTIER_BATTLE_INFO. The in-battle INFO viewer must only treat a foe's
// ability/item as "revealed" once the player has actually witnessed it. The AI's
// *speculative* move evaluation calls RecordAbilityBattle through GetBattleMovePriority
// (the Prankster check on every status move it scores), so a foe whose ability the AI
// merely evaluated — but never used in front of the player — must NOT be marked revealed.
// Regression for the foe's ability wrongly displaying at battle start / on switch-in.
AI_SINGLE_BATTLE_TEST("Frontier INFO: AI scoring a Prankster status move does not reveal the foe")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        // Whimsicott has Prankster + a status move (Tailwind) the AI will score during its
        // calc, but it picks Moonblast to KO, so the status move never executes in view.
        OPPONENT(SPECIES_WHIMSICOTT) { Ability(ABILITY_PRANKSTER); Moves(MOVE_TAILWIND, MOVE_MOONBLAST); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); EXPECT_MOVE(opponent, MOVE_MOONBLAST); }
    } THEN {
        // Value is known (omniscient), but it must not be flagged as seen by the player.
        EXPECT(gAiPartyData->mons[B_SIDE_OPPONENT][0].ability == ABILITY_PRANKSTER);
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
    }
}

// FORK: B_FRONTIER_BATTLE_INFO. Once a foe's ability is genuinely witnessed, the viewer must
// keep showing *that* ability even though the AI's speculative move/switch evaluation later
// overwrites gAiPartyData->mons[].ability (its live, mutable knowledge model). The viewer reads
// a reveal-time snapshot (gBattleStruct->infoRevealedAbility) instead, so a speculative record of
// a different ability — e.g. a benched Prankster mon simulated in the active slot whose turn-order
// check records Prankster onto that slot — can't change what the player sees. Without the snapshot,
// a Krookodile whose Intimidate fired at battle start displayed as "Prankster" in the viewer.
AI_SINGLE_BATTLE_TEST("Frontier INFO: a speculative ability record does not corrupt an already-revealed foe ability")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_SLOWKING) { Ability(ABILITY_OWN_TEMPO); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_KROOKODILE) { Ability(ABILITY_INTIMIDATE); Moves(MOVE_EARTHQUAKE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPLASH); }
    } THEN {
        // Intimidate fired on switch-in: genuinely revealed (bit set, snapshot taken).
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT(gBattleStruct->infoRevealedAbility[B_SIDE_OPPONENT][0] == ABILITY_INTIMIDATE);

        // Simulate the AI's speculative evaluation recording a different ability onto the
        // active foe's slot (mid-calc, so it updates the knowledge model but takes no reveal).
        gAiLogicData->aiCalcInProgress = TRUE;
        RecordAbilityBattle(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), ABILITY_PRANKSTER);
        gAiLogicData->aiCalcInProgress = FALSE;

        // The AI's model is clobbered, but the player-facing snapshot the viewer reads is intact.
        EXPECT(gAiPartyData->mons[B_SIDE_OPPONENT][0].ability == ABILITY_PRANKSTER);
        EXPECT(gBattleStruct->infoRevealedAbility[B_SIDE_OPPONENT][0] == ABILITY_INTIMIDATE);
    }
}

// FORK: FEATURE_INNATE_ABILITIES + B_FRONTIER_BATTLE_INFO. An innate ability is a passive
// independent of the chosen ability, so witnessing an innate (here an innate Levitate blocking
// a Ground move) must NOT reveal the chosen ability. The chosen-ability reveal bit stays clear
// (the viewer keeps showing "?"), while only the matching innate slot's bit is set, so the line
// reads "? (+Levitate)". Magnemite's chosen ability is Magnet Pull, and its innates are
// Levitate (slot 0) and Sturdy (slot 1); only Levitate is witnessed here.
SINGLE_BATTLE_TEST("Frontier INFO: an innate Levitate reveal does not reveal the chosen ability")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MAGNEMITE, ABILITY_LEVITATE));
        ASSUME(GetSpeciesInnate(SPECIES_MAGNEMITE, 0) == ABILITY_LEVITATE);
        ASSUME(GetSpeciesInnate(SPECIES_MAGNEMITE, 1) == ABILITY_STURDY);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        // Magnemite's chosen ability is forced to Honey Gather (no in-battle effect, so the
        // engine never records it), isolating the reveal under test to the innate Levitate
        // blocking the Ground move.
        PLAYER(SPECIES_SANDSLASH) { Moves(MOVE_EARTHQUAKE); }
        OPPONENT(SPECIES_MAGNEMITE) { Ability(ABILITY_HONEY_GATHER); Moves(MOVE_THUNDER_WAVE); }
    } WHEN {
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } THEN {
        // Innate Levitate was seen blocking the Ground move: its slot bit is set...
        EXPECT(gBattleStruct->infoRevealedInnates[B_SIDE_OPPONENT][0] & (1u << 0));
        // ...but the unseen innate Sturdy (slot 1) and the chosen ability stay hidden,
        // so the viewer shows "? (+Levitate)".
        EXPECT((gBattleStruct->infoRevealedInnates[B_SIDE_OPPONENT][0] & (1u << 1)) == 0);
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
    }
}
