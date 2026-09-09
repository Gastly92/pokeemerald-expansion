#include "global.h"
#include "test/battle.h"

// FORK: Illusion is only meant to break when the disguised battler takes damage, but when
// Dynamax ran out the battler's sprite silently reverted to its real species anyway.
// BattleScript_DynamaxEnds plays B_ANIM_FORM_CHANGE, whose AnimTask_TransformMon calls
// HandleSpeciesGfxDataChange, and that function read MON_DATA_SPECIES off the real party mon
// for every non-Transform reload. The healthbox kept showing the disguised nickname (it goes
// through GetIllusionMonPtr), so the two disagreed. HandleSpeciesGfxDataChange now prefers the
// Illusion mon for the reloading battler, the same way BattleLoadMonSpriteGfx already did.
//
// The sprite itself is eyes-on only: general animations are skipped in headless mode, so
// HandleSpeciesGfxDataChange never runs under `make check`. What these tests pin is the
// surrounding state the fix must not disturb - Dynamax expiring is not an Illusion break, and
// the Illusion still breaks normally afterwards.

SINGLE_BATTLE_TEST("Illusion does not wear off when Dynamax runs out")
{
    GIVEN {
        PLAYER(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        // Dynamax expires at the end of the third turn.
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        NONE_OF {
            ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ILLUSION_OFF, player);
            MESSAGE("Zoroark's illusion wore off!");
        }
    } THEN {
        EXPECT_EQ(gBattleStruct->illusion[B_BATTLER_0].state, ILLUSION_ON);
        EXPECT_EQ(GetBattlerVisualSpecies(B_BATTLER_0), SPECIES_WYNAUT);
    }
}

SINGLE_BATTLE_TEST("Illusion still breaks on the first hit after Dynamax runs out")
{
    GIVEN {
        PLAYER(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); Moves(MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        HP_BAR(player);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ILLUSION_OFF, player);
        MESSAGE("Zoroark's illusion wore off!");
    } THEN {
        EXPECT_EQ(gBattleStruct->illusion[B_BATTLER_0].state, ILLUSION_OFF);
    }
}
