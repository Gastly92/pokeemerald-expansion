#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// A single-target status move that CancelerTargetFailure blocks prints its own message
// ("It doesn't affect..."), and CancelerSubstitute used to add "But it failed!" after it,
// because it saw no affected target and assumed a Substitute had stopped the move.

SINGLE_BATTLE_TEST("Status move blocked by innate Overcoat doesn't also say But it failed")
{
    GIVEN {
        ASSUME(IsPowderMove(MOVE_SLEEP_POWDER));
        ASSUME(SpeciesHasInnate(SPECIES_FORRETRESS, ABILITY_OVERCOAT));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_VENOMOTH) { Moves(MOVE_SLEEP_POWDER); }
        OPPONENT(SPECIES_FORRETRESS) { Ability(ABILITY_STURDY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SLEEP_POWDER); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_OVERCOAT);
        MESSAGE("It doesn't affect the opposing Forretress…");
        NONE_OF { MESSAGE("But it failed!"); }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    } THEN {
        EXPECT_EQ(opponent->status1 & STATUS1_SLEEP, 0);
    }
}

SINGLE_BATTLE_TEST("Status move blocked by the target doesn't also say But it failed")
{
    u32 species, item, move;
    PARAMETRIZE { species = SPECIES_BULBASAUR; item = ITEM_NONE;           move = MOVE_SLEEP_POWDER; } // Grass vs powder
    PARAMETRIZE { species = SPECIES_WOBBUFFET; item = ITEM_SAFETY_GOGGLES; move = MOVE_SLEEP_POWDER; }
    PARAMETRIZE { species = SPECIES_SANDSHREW; item = ITEM_NONE;           move = MOVE_THUNDER_WAVE; } // type immunity
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, FALSE);
        PLAYER(SPECIES_SMEARGLE) { Moves(move); }
        OPPONENT(species) { Item(item); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, move); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, move, player);
            MESSAGE("But it failed!");
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}

// Control: the check this guards is still what makes a status move into a Substitute fail.
SINGLE_BATTLE_TEST("Status move into a Substitute still says But it failed")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); Moves(MOVE_SLEEP_POWDER); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_SUBSTITUTE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_SLEEP_POWDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponent);
        MESSAGE("Wobbuffet used Sleep Powder!");
        MESSAGE("But it failed!");
    }
}
