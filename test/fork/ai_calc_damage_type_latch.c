#include "global.h"
#include "test/battle.h"
#include "battle_ai_util.h"

// AI_CalcDamage must not read a move-type latch an earlier caller left set. SetTypeBeforeUsingMove
// only ever *sets* gBattleStruct->dynamicMoveType, so without a reset on the way in, a leftover
// Water (the AI scoring Weather Ball in rain) priced the next plain move -- U-turn into Politoed --
// as Water, at half damage. See the UPSTREAM: note in AI_CalcDamage (src/battle_ai_util.c).
SINGLE_BATTLE_TEST("AI_CalcDamage ignores a move type left latched by an earlier calc")
{
    struct SimulatedDamage clean, stale;
    uq4_12_t effectiveness;

    GIVEN {
        PLAYER(SPECIES_CINDERACE) { Moves(MOVE_U_TURN); }
        OPPONENT(SPECIES_POLITOED) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { }
    } THEN {
        clean = AI_CalcDamageSaveBattlers(MOVE_U_TURN, B_BATTLER_0, B_BATTLER_1, &effectiveness, GIMMICK_NONE, GIMMICK_NONE);
        gBattleStruct->dynamicMoveType = TYPE_WATER;
        stale = AI_CalcDamageSaveBattlers(MOVE_U_TURN, B_BATTLER_0, B_BATTLER_1, &effectiveness, GIMMICK_NONE, GIMMICK_NONE);
        EXPECT_EQ(stale.minimum, clean.minimum);
        EXPECT_EQ(stale.maximum, clean.maximum);
    }
}
