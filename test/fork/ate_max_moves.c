#include "global.h"
#include "test/battle.h"

// FORK: -ate abilities (Pixilate, Refrigerate, Aerilate, Galvanize, Dragonize) convert
// Max Moves, matching the games: "If Max Strike is selected with Pixilate, it will turn
// into Max Starfall when used."
//
// GetDynamicMoveType() used to skip the -ate branch entirely while GIMMICK_DYNAMAX was
// active, so a Max Move kept the base move's unconverted type. That desynced the move
// menu from the engine, because the gimmick is only *armed* while the menu is open: the
// preview resolved with no active gimmick and showed Pixilate Sylveon's Hyper Voice as
// Max Starfall, then the engine resolved it with Dynamax active and fired Max Strike.
// The fork's gimmick picker made this trivial to hit - cycling onto Dynamax repaints the
// move names from the same preview path.
//
// Only the 20% -ate power boost stays suppressed under Dynamax, which is how the
// ABILITY_NORMALIZE branch alongside it already behaved.

SINGLE_BATTLE_TEST("DYNAMAX FORK: Pixilate converts a Max Move to the -ate type")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_HYPER_VOICE) == TYPE_NORMAL);
        ASSUME(gTypesInfo[TYPE_FAIRY].maxMove == MOVE_MAX_STARFALL);
        PLAYER(SPECIES_SYLVEON) { Ability(ABILITY_PIXILATE); Moves(MOVE_HYPER_VOICE); }
        OPPONENT(SPECIES_DRATINI);
    } WHEN {
        TURN { MOVE(player, MOVE_HYPER_VOICE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        // Before the fix this fired Max Strike, which a Dragon-type resists nothing of.
        MESSAGE("Sylveon used Max Starfall!");
        MESSAGE("It's super effective!");
    }
}

SINGLE_BATTLE_TEST("DYNAMAX FORK: Refrigerate converts a Max Move to the -ate type")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SCRATCH) == TYPE_NORMAL);
        ASSUME(gTypesInfo[TYPE_ICE].maxMove == MOVE_MAX_HAILSTORM);
        PLAYER(SPECIES_AURORUS) { Ability(ABILITY_REFRIGERATE); Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        MESSAGE("Aurorus used Max Hailstorm!");
    }
}

SINGLE_BATTLE_TEST("DYNAMAX FORK: an -ate ability does not add its power boost to a Max Move", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_CUTE_CHARM; }
    PARAMETRIZE { ability = ABILITY_PIXILATE; }

    GIVEN {
        // Wobbuffet is Psychic, so both Normal and Fairy are neutral into it; the only
        // difference left between the two runs is Sylveon's Fairy STAB on Max Starfall.
        ASSUME(GetMoveType(MOVE_HYPER_VOICE) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_SYLVEON, 0) == TYPE_FAIRY);
        PLAYER(SPECIES_SYLVEON) { Ability(ability); Moves(MOVE_HYPER_VOICE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(700); HP(700); }
    } WHEN {
        TURN { MOVE(player, MOVE_HYPER_VOICE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // STAB only (1.5x), not STAB + -ate (1.8x).
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("DYNAMAX FORK: Z-Moves still ignore -ate abilities")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_HYPER_VOICE) == TYPE_NORMAL);
        ASSUME(gTypesInfo[TYPE_NORMAL].zMove == MOVE_BREAKNECK_BLITZ);
        PLAYER(SPECIES_SYLVEON) { Ability(ABILITY_PIXILATE); Moves(MOVE_HYPER_VOICE); Item(ITEM_NORMALIUM_Z); }
        OPPONENT(SPECIES_DRATINI);
    } WHEN {
        TURN { MOVE(player, MOVE_HYPER_VOICE, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ZMOVE_ACTIVATE, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BREAKNECK_BLITZ, player);
        NOT { MESSAGE("It's super effective!"); }
    }
}
