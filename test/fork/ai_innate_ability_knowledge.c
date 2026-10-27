#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "fork/innate_abilities.h"

// An innate blocking something (here Carbink's innate Clear Body silently stopping a
// secondary SpAtk drop) must not overwrite what the AI has learned about the mon's
// *chosen* ability. It used to: RecordAbilityBattle stored the innate as the chosen
// ability, so a revealed Bulletproof was forgotten after every Moonblast and the AI
// went back to Focus Blast, alternating between the two indefinitely.

AI_SINGLE_BATTLE_TEST("AI remembers a revealed Bulletproof after an innate Clear Body blocks a stat drop")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_CARBINK, ABILITY_CLEAR_BODY));
        ASSUME(IsBallisticMove(MOVE_FOCUS_BLAST));
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_CARBINK) { Level(50); Ability(ABILITY_BULLETPROOF); Moves(MOVE_RECOVER); }
        // Reveals Bulletproof: No Guard so the 70% Focus Blast always connects.
        OPPONENT(SPECIES_MACHOP) { Level(5); Ability(ABILITY_NO_GUARD); Moves(MOVE_FOCUS_BLAST); }
        // Mystical Fire always tries to lower SpAtk, which the innate Clear Body blocks silently.
        OPPONENT(SPECIES_GARDEVOIR) { Level(50); Ability(ABILITY_SYNCHRONIZE); Moves(MOVE_FOCUS_BLAST, MOVE_MYSTICAL_FIRE); }
    } WHEN {
        TURN { MOVE(player, MOVE_RECOVER); EXPECT_MOVE(opponent, MOVE_FOCUS_BLAST); }
        TURN { MOVE(player, MOVE_RECOVER); EXPECT_SWITCH(opponent, 1); }
        TURN { MOVE(player, MOVE_RECOVER); EXPECT_MOVE(opponent, MOVE_MYSTICAL_FIRE); }
        TURN { MOVE(player, MOVE_RECOVER); EXPECT_MOVE(opponent, MOVE_MYSTICAL_FIRE); }
        TURN { MOVE(player, MOVE_RECOVER); EXPECT_MOVE(opponent, MOVE_MYSTICAL_FIRE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_BULLETPROOF);
        NOT ABILITY_POPUP(player, ABILITY_BULLETPROOF);
    }
}
