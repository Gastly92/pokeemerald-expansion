#include "global.h"
#include "test/battle.h"
#include "config_changes.h"
#include "fork/innate_abilities.h"

// Regression for an upstream bug (UPSTREAM: tag in MoveEndClearBits): after bouncing a
// status move, the bouncer's targetsDone[] was never cleared, so a slower bouncer's own
// move later that turn silently skipped the original attacker and dealt no damage.

SINGLE_BATTLE_TEST("Magic Bounce: a slower bouncer's own attack still hits the turn it bounces a move")
{
    GIVEN {
        ASSUME(GetMoveCategory(MOVE_SPORE) == DAMAGE_CATEGORY_STATUS);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_ESPEON) { Ability(ABILITY_MAGIC_BOUNCE); Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); MOVE(opponent, MOVE_PSYCHIC); }
    } SCENE {
        MESSAGE("Wobbuffet's Spore was bounced back!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SPORE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PSYCHIC, opponent);
        HP_BAR(player);
    } THEN {
        EXPECT_LT(player->hp, player->maxHP);
    }
}

// The reported case: a Prankster Spore bounced by innate Magic Bounce, then Psychic.
SINGLE_BATTLE_TEST("Magic Bounce: innate Magic Bounce Mr. Mime's Psychic hits the Smeargle whose Spore it bounced")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_MR_MIME, ABILITY_MAGIC_BOUNCE));
        ASSUME(SpeciesHasInnate(SPECIES_SMEARGLE, ABILITY_PRANKSTER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_SMEARGLE) { Ability(ABILITY_MOODY); }
        OPPONENT(SPECIES_MR_MIME) { Ability(ABILITY_SOUNDPROOF); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); MOVE(opponent, MOVE_PSYCHIC); }
    } SCENE {
        MESSAGE("Smeargle's Spore was bounced back!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SPORE, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PSYCHIC, opponent);
        HP_BAR(player);
    } THEN {
        EXPECT_LT(player->hp, player->maxHP);
    }
}
