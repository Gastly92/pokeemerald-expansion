#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

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
