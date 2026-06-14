#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"
#include "battle_ai_species_overrides.h"

// Tests for AI_FLAG_SMART_SPECIES_LOGIC (species-aware AI overrides).

AI_SINGLE_BATTLE_TEST("Smart species AI: Sharpedo Protects to bank Speed Boost, then Mega Evolves")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        ASSUME(GetSpeciesAbility(SPECIES_SHARPEDO_MEGA, 0) != ABILITY_SPEED_BOOST);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SPECIES_LOGIC);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SHARPEDO) { Item(ITEM_SHARPEDONITE); Ability(ABILITY_SPEED_BOOST); Moves(MOVE_PROTECT, MOVE_CRUNCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_PROTECT); } // bank +1 Speed, hold the Mega
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_CRUNCH, gimmick: GIMMICK_MEGA); } // Mega now, boost kept
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_MEGA_EVOLUTION, opponent);
    } THEN {
        EXPECT_EQ(opponent->species, SPECIES_SHARPEDO_MEGA);
    }
}

AI_SINGLE_BATTLE_TEST("Smart species AI: a Speed Boost mon whose Mega keeps Speed Boost does not stall (Blaziken)")
{
    GIVEN {
        ASSUME(GetSpeciesAbility(SPECIES_BLAZIKEN_MEGA, 0) == ABILITY_SPEED_BOOST);
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SPECIES_LOGIC);
        PLAYER(SPECIES_SCIZOR) { Moves(MOVE_CELEBRATE); } // 4x weak to Fire: Flare Blitz clearly best
        OPPONENT(SPECIES_BLAZIKEN) { Item(ITEM_BLAZIKENITE); Ability(ABILITY_SPEED_BOOST); Moves(MOVE_PROTECT, MOVE_FLARE_BLITZ); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); EXPECT_MOVE(opponent, MOVE_FLARE_BLITZ, gimmick: GIMMICK_MEGA); } // Mega keeps Speed Boost, so Mega at once
    } THEN {
        EXPECT_EQ(opponent->species, SPECIES_BLAZIKEN_MEGA);
    }
}

AI_SINGLE_BATTLE_TEST("Smart species AI: Palafin-Hero is kept in on a bad matchup instead of pivoting out")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT | AI_FLAG_SMART_SPECIES_LOGIC);
        PLAYER(SPECIES_MANECTRIC) { MaxHP(800); HP(800); Moves(MOVE_THUNDERBOLT); } // bulky: Palafin can't grind it down
        OPPONENT(SPECIES_PALAFIN_HERO) { MaxHP(500); HP(500); Moves(MOVE_WATER_GUN); } // loses the 1v1, but isn't OHKO'd
        OPPONENT(SPECIES_GOLEM) { Moves(MOVE_TACKLE); }                      // Ground type: a clean switch vs Electric
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT); EXPECT_MOVE(opponent, MOVE_WATER_GUN); } // veto runs before the switch checks -> stays in
    }
}

AI_SINGLE_BATTLE_TEST("Smart species AI: without the flag Palafin pivots out of the same bad matchup")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_TRY_TO_FAINT | AI_FLAG_CHECK_VIABILITY | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES | AI_FLAG_OMNISCIENT);
        PLAYER(SPECIES_MANECTRIC) { MaxHP(800); HP(800); Moves(MOVE_THUNDERBOLT); }
        OPPONENT(SPECIES_PALAFIN_HERO) { MaxHP(500); HP(500); Moves(MOVE_WATER_GUN); }
        OPPONENT(SPECIES_GOLEM) { Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT); EXPECT_SWITCH(opponent, 1); }
    }
}
