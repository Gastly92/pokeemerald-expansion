#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_LEECH_SEED flag (config/buff.h). BUFF_* flags
// default off in the test baseline (see TestInitConfigData), so each test opts in
// with WITH_CONFIG(BUFF_LEECH_SEED, TRUE). The stock single-seeder behavior is
// exercised by the inherited tests in test/battle/move_effect/leech_seed.c.

ASSUMPTIONS
{
    ASSUME(GetMoveEffect(MOVE_LEECH_SEED) == EFFECT_LEECH_SEED);
}

SINGLE_BATTLE_TEST("BUFF_LEECH_SEED: Re-seeding an already-seeded foe drains it immediately")
{
    s16 drain;
    s16 heal;

    GIVEN {
        WITH_CONFIG(BUFF_LEECH_SEED, TRUE);
        PLAYER(SPECIES_WYNAUT) { HP(1); }
        OPPONENT(SPECIES_SHELLDER);
    } WHEN {
        TURN { MOVE(player, MOVE_LEECH_SEED); }   // first seed
        TURN { MOVE(player, MOVE_LEECH_SEED); }   // already seeded -> immediate re-drain
    } SCENE {
        // Turn 1: the seed sticks, then ticks at end of turn.
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LEECH_SEED, player);
        MESSAGE("The opposing Shellder was seeded!");
        HP_BAR(opponent);
        HP_BAR(player);
        // Turn 2: re-using Leech Seed drains immediately instead of failing.
        // Only the drain animation plays - not the full seeding animation - to
        // match the end-of-turn drain.
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_LEECH_SEED_DRAIN, opponent);
        HP_BAR(opponent, captureDamage: &drain);
        HP_BAR(player, captureDamage: &heal);
        MESSAGE("The opposing Shellder's health is sapped by Leech Seed!");
        // ...and the persistent seed still ticks at the end of turn 2.
        HP_BAR(opponent);
        HP_BAR(player);
    } THEN {
        EXPECT_MUL_EQ(drain, Q_4_12(-1), heal);
    }
}

SINGLE_BATTLE_TEST("BUFF_LEECH_SEED off: Re-seeding an already-seeded foe just fails")
{
    GIVEN {
        WITH_CONFIG(BUFF_LEECH_SEED, FALSE);
        PLAYER(SPECIES_WYNAUT) { HP(1); }
        OPPONENT(SPECIES_SHELLDER);
    } WHEN {
        TURN { MOVE(player, MOVE_LEECH_SEED); }
        TURN { MOVE(player, MOVE_LEECH_SEED); }   // fails, no immediate drain
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LEECH_SEED, player);
        MESSAGE("The opposing Shellder was seeded!");
        HP_BAR(opponent);
        HP_BAR(player);
        // Turn 2: re-seeding an already-seeded foe just fails (vanilla).
        MESSAGE("The opposing Shellder avoided the attack!");
    }
}

DOUBLE_BATTLE_TEST("BUFF_LEECH_SEED: Two battlers can seed the same foe and each drains it")
{
    GIVEN {
        WITH_CONFIG(BUFF_LEECH_SEED, TRUE);
        PLAYER(SPECIES_WYNAUT) { HP(1); }
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_SHELLDER);
        OPPONENT(SPECIES_CLOYSTER);
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_LEECH_SEED, target: opponentLeft);
            MOVE(playerRight, MOVE_LEECH_SEED, target: opponentLeft); // stacks on the same foe
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LEECH_SEED, playerLeft);
        MESSAGE("The opposing Shellder was seeded!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LEECH_SEED, playerRight);
        MESSAGE("The opposing Shellder was seeded!");
        // End of turn: opposing Shellder is drained once per seeder (low index first).
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_LEECH_SEED_DRAIN, opponentLeft);
        HP_BAR(opponentLeft);
        HP_BAR(playerLeft);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_LEECH_SEED_DRAIN, opponentLeft);
        HP_BAR(opponentLeft);
        HP_BAR(playerRight);
    }
}

SINGLE_BATTLE_TEST("BUFF_LEECH_SEED: Rapid Spin frees every stacked seed at once")
{
    GIVEN {
        WITH_CONFIG(BUFF_LEECH_SEED, TRUE);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_SHELLDER) { Moves(MOVE_RAPID_SPIN, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_LEECH_SEED); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_RAPID_SPIN); }
    } SCENE {
        MESSAGE("The opposing Shellder was seeded!");
        HP_BAR(opponent);
        HP_BAR(player);
        // Rapid Spin frees the seed (and with it every stacked seeder).
        ANIMATION(ANIM_TYPE_MOVE, MOVE_RAPID_SPIN, opponent);
        MESSAGE("The opposing Shellder was freed from Leech Seed!");
    } THEN {
        u32 seeders = gBattleMons[B_POSITION_OPPONENT_LEFT].volatiles.leechSeededBy;
        EXPECT_EQ(seeders, 0);
    }
}

SINGLE_BATTLE_TEST("BUFF_LEECH_SEED: Liquid Ooze punishes the immediate re-drain")
{
    GIVEN {
        WITH_CONFIG(BUFF_LEECH_SEED, TRUE);
        PLAYER(SPECIES_WYNAUT); // full HP: each tick takes Liquid Ooze recoil
        OPPONENT(SPECIES_GRIMER) { Ability(ABILITY_LIQUID_OOZE); }
    } WHEN {
        TURN { MOVE(player, MOVE_LEECH_SEED); }
        TURN { MOVE(player, MOVE_LEECH_SEED); }   // re-drain into Liquid Ooze hurts the seeder
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_LEECH_SEED, player);
        MESSAGE("The opposing Grimer was seeded!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_LEECH_SEED_DRAIN, opponent);
        // Turn 2 re-drain plays only the drain animation, not the seeding move.
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_LEECH_SEED_DRAIN, opponent);
        HP_BAR(opponent);
        MESSAGE("Wynaut sucked up the liquid ooze!");
        HP_BAR(player);
    }
}
