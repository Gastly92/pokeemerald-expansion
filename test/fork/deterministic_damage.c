#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_DAMAGE flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData),
// so each test opts in with WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE). The stock
// per-roll damage spread is covered by test/battle/damage_formula.c.

// The expected per-turn values below are read straight off the verified Gen5+
// spread in test/battle/damage_formula.c ("Damage calculation matches Gen5+"):
// Glaceon (Lv75, Atk 123) Ice Fang vs Garchomp (Def 163) deals, by roll
// percentage, 92%->180, 93%->180, 94%->184, 95%->184, 96%->184, 97%->192,
// 98%->192, 99%->192, 100%->196. With the default base 92% / +1% per turn ramp,
// turn N simply walks up that table, so turns 1..9 are exactly those values.
SINGLE_BATTLE_TEST("DETERMINISTIC_DAMAGE: turn 1 uses the base multiplier and damage ramps each turn")
{
    s16 damage[9];
    ASSUME(DETERMINISTIC_DAMAGE_BASE_PERCENT == 92);
    ASSUME(DETERMINISTIC_DAMAGE_TURN_INCREMENT == 1);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        ASSUME(GetMoveCategory(MOVE_ICE_FANG) == DAMAGE_CATEGORY_PHYSICAL);
        PLAYER(SPECIES_GLACEON) { Level(75); Attack(123); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 1, 92%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 2, 93%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 3, 94%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 4, 95%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 5, 96%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 6, 97%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 7, 98%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 8, 99%
        TURN { MOVE(player, MOVE_ICE_FANG); MOVE(opponent, MOVE_CELEBRATE); } // turn 9, 100%
    } SCENE {
        for (u32 i = 0; i < 9; i++)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, player);
            HP_BAR(opponent, captureDamage: &damage[i]);
        }
    } THEN {
        EXPECT_EQ(damage[0], 180); // turn 1, 92%
        EXPECT_EQ(damage[1], 180); // turn 2, 93%
        EXPECT_EQ(damage[2], 184); // turn 3, 94%
        EXPECT_EQ(damage[3], 184); // turn 4, 95%
        EXPECT_EQ(damage[4], 184); // turn 5, 96%
        EXPECT_EQ(damage[5], 192); // turn 6, 97%
        EXPECT_EQ(damage[6], 192); // turn 7, 98%
        EXPECT_EQ(damage[7], 192); // turn 8, 99%
        EXPECT_EQ(damage[8], 196); // turn 9, 100%
    }
}

// The ramp is intentionally uncapped: once enough turns pass the multiplier
// climbs past 100% and the move deals more than its stock maximum roll. A clean,
// neutral, non-STAB hit with a large base (so each 1% step registers) makes this
// visible: Glaceon (Lv100, Atk 999) Tackle vs Garchomp (Def 80) deals 387 on
// turn 1 (92%), 421 on turn 9 (100% — the stock maximum), and 450 on turn 16
// (107%), i.e. damage keeps climbing above what any stock roll could produce.
SINGLE_BATTLE_TEST("DETERMINISTIC_DAMAGE: the multiplier is uncapped above 100%")
{
    s16 damage[16];
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL); // non-STAB, neutral vs Garchomp
        PLAYER(SPECIES_GLACEON) { Level(100); Attack(999); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(80); MaxHP(30000); HP(30000); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); } // turn 1, 92%
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); } // turn 9, 100%
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); } // turn 16, 107%
    } SCENE {
        for (u32 i = 0; i < 16; i++)
            HP_BAR(opponent, captureDamage: &damage[i]);
    } THEN {
        EXPECT_EQ(damage[0], 387);          // turn 1, 92%
        EXPECT_EQ(damage[8], 421);          // turn 9, 100% — the stock maximum roll
        EXPECT_EQ(damage[15], 450);         // turn 16, 107%
        EXPECT_GT(damage[15], damage[8]);   // uncapped: exceeds the stock 100% maximum
    }
}
