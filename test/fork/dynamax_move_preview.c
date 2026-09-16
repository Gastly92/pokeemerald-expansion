#include "global.h"
#include "test/battle.h"
#include "battle_dynamax.h"

// FORK: the move-selection menu previews each slot's Max Move name by calling
// GetMaxMove() once per slot (MoveSelectionDisplayMoveNames in
// src/battle_controller_player.c). GetMaxMove resolved the base move's type through
// SetTypeBeforeUsingMove, which only ever *sets* gBattleStruct->dynamicMoveType and
// never clears it - it assumes the caller cleared the latch, which the move-execution
// path does and the menu does not.
//
// So the first slot with a dynamic type leaked it into every later slot, since
// GetBattleMoveType short-circuits on the latch. Arceus holding a Mind Plate showed
// Judgment as Max Mindstorm (right) and the Shadow Ball below it as Max Mindstorm too
// (wrong), while actually using Shadow Ball fired Max Phantasm - the menu and the engine
// disagreed. GetMaxMove now saves, clears and restores the latch, so each slot resolves
// on its own move.
//
// The type-changing effects that can seed the leak are the EFFECT_CHANGE_TYPE_ON_ITEM /
// WEATHER_BALL / HIDDEN_POWER / TERRAIN_PULSE / REVELATION_DANCE / TERA_BLAST family
// handled by GetDynamicMoveType; Judgment plus a plate is just the easiest to arrange.

// Exactly what the menu previews: four slots resolved one after another, in order.
SINGLE_BATTLE_TEST("DYNAMAX FORK: each move slot's Max Move preview resolves on its own move")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_SHADOW_BALL) == TYPE_GHOST);
        ASSUME(gTypesInfo[TYPE_PSYCHIC].maxMove == MOVE_MAX_MINDSTORM);
        ASSUME(gTypesInfo[TYPE_GHOST].maxMove == MOVE_MAX_PHANTASM);
        ASSUME(gTypesInfo[TYPE_GROUND].maxMove == MOVE_MAX_QUAKE);
        PLAYER(SPECIES_ARCEUS) {
            Item(ITEM_MIND_PLATE);
            Moves(MOVE_JUDGMENT, MOVE_SHADOW_BALL, MOVE_EARTH_POWER, MOVE_RECOVER);
        }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_JUDGMENT); }
    } THEN {
        enum BattlerId playerId = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        // The menu walks slots 0..3 in order, so Judgment's Psychic is resolved first and
        // is what used to leak. Before the fix, slots 1 and 2 both read Max Mindstorm.
        EXPECT_EQ(GetMaxMove(playerId, MOVE_JUDGMENT), MOVE_MAX_MINDSTORM);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_SHADOW_BALL), MOVE_MAX_PHANTASM);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_EARTH_POWER), MOVE_MAX_QUAKE);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_RECOVER), MOVE_MAX_GUARD);
    }
}

// Same four moves resolved back to front: the plate move is no longer first, so this
// fails only if a resolution leaks *backwards* into an earlier-resolved slot.
SINGLE_BATTLE_TEST("DYNAMAX FORK: a Max Move preview does not depend on slot order")
{
    GIVEN {
        PLAYER(SPECIES_ARCEUS) {
            Item(ITEM_MIND_PLATE);
            Moves(MOVE_JUDGMENT, MOVE_SHADOW_BALL, MOVE_EARTH_POWER, MOVE_RECOVER);
        }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_JUDGMENT); }
    } THEN {
        enum BattlerId playerId = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_RECOVER), MOVE_MAX_GUARD);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_EARTH_POWER), MOVE_MAX_QUAKE);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_SHADOW_BALL), MOVE_MAX_PHANTASM);
        EXPECT_EQ(GetMaxMove(playerId, MOVE_JUDGMENT), MOVE_MAX_MINDSTORM);
    }
}

// The engine side was always right; this pins it, so a future change cannot "fix" the
// preview by breaking what actually fires. Judgment keeps the plate's type, Shadow Ball
// keeps its own - the two halves of what the player sees.
SINGLE_BATTLE_TEST("DYNAMAX FORK: a plate holder's non-Judgment Max Move keeps its own type")
{
    GIVEN {
        PLAYER(SPECIES_ARCEUS) { Item(ITEM_MIND_PLATE); Moves(MOVE_JUDGMENT, MOVE_SHADOW_BALL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SHADOW_BALL, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        MESSAGE("Arceus used Max Phantasm!");
    }
}

SINGLE_BATTLE_TEST("DYNAMAX FORK: a plate holder's Judgment takes the plate's Max Move")
{
    GIVEN {
        PLAYER(SPECIES_ARCEUS) { Item(ITEM_MIND_PLATE); Moves(MOVE_JUDGMENT, MOVE_SHADOW_BALL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_JUDGMENT, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, player);
        MESSAGE("Arceus used Max Mindstorm!");
    }
}

// FORK: GetMaxMovePower's second parameter is the MAX move, not the base move - that is
// how the damage calc calls it, GetMaxMovePower(ctx->baseMove, ctx->move) in
// CalcMoveBasePower. It reads that parameter for the MOVE_EFFECT_FIXED_POWER check which
// pins G-Max Drum Solo, G-Max Hydrosnipe and G-Max Fireball to 160 base power.
// MoveSelectionDisplayMoveDescription passed the base move for both, so the PWR field in
// the move-description window tested the base move for that effect, never found it, and
// showed those three the ordinary tier-derived power while the engine used 160. The two
// spellings below are exactly the disagreement; the window now passes the max move.
// (The window's PWR text has no test hook, so this pins the function contract the caller
// has to satisfy rather than the caller itself.)
TEST("DYNAMAX FORK: a fixed-power G-Max move's power comes from the max move, not the base move")
{
    ASSUME(MoveHasAdditionalEffect(MOVE_G_MAX_DRUM_SOLO, MOVE_EFFECT_FIXED_POWER));
    ASSUME(!MoveHasAdditionalEffect(MOVE_GRASSY_GLIDE, MOVE_EFFECT_FIXED_POWER));

    EXPECT_EQ(GetMaxMovePower(MOVE_GRASSY_GLIDE, MOVE_G_MAX_DRUM_SOLO), 160);
    EXPECT_NE(GetMaxMovePower(MOVE_GRASSY_GLIDE, MOVE_GRASSY_GLIDE), 160);
}
