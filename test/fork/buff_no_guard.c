#include "global.h"
#include "test/battle.h"
#include "fork/damage_preview.h"

// BUFF_NO_GUARD -- "wild swings": a damaging move used by or against a No Guard battler has its
// power multiplied by (BUFF_NO_GUARD_POWER_BASE - accuracy) / 100, only while
// DETERMINISTIC_ACCURACY_EVASION is on. See config/buff.h.
//
// Machamp carries both Guts and No Guard, so a Guts/No Guard pair differs in nothing but the
// buff (no status, so Guts is inert). Wobbuffet is Psychic, so Stone Edge (Rock, 80%) and
// Focus Blast (Fighting, 70%) are neither STAB from it nor resisted by it.

static EWRAM_DATA enum DamagePreviewKO sKo = 0;

ASSUMPTIONS
{
    ASSUME(GetMoveAccuracy(MOVE_STONE_EDGE) == 80);
    ASSUME(GetMoveAccuracy(MOVE_FOCUS_BLAST) == 70);
    ASSUME(GetMoveAccuracy(MOVE_ROCK_SLIDE) == 90);
    ASSUME(GetMoveAccuracy(MOVE_ROCK_SMASH) == 100);
    ASSUME(BUFF_NO_GUARD_POWER_BASE == 200);
}

SINGLE_BATTLE_TEST("BUFF_NO_GUARD: a No Guard attacker's move gains the accuracy it gave up as power", s16 damage)
{
    enum Ability ability;
    enum Move move;

    PARAMETRIZE { move = MOVE_STONE_EDGE; ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { move = MOVE_STONE_EDGE; ability = ABILITY_GUTS; }
    PARAMETRIZE { move = MOVE_FOCUS_BLAST; ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { move = MOVE_FOCUS_BLAST; ability = ABILITY_GUTS; }
    PARAMETRIZE { move = MOVE_ROCK_SLIDE; ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { move = MOVE_ROCK_SLIDE; ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_MACHAMP) { Ability(ability); Moves(move); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.2), results[0].damage);
        EXPECT_MUL_EQ(results[3].damage, Q_4_12(1.3), results[2].damage);
        EXPECT_MUL_EQ(results[5].damage, Q_4_12(1.1), results[4].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_NO_GUARD: a move aimed at a No Guard battler gains the same power", s16 damage)
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_STONE_EDGE); }
        OPPONENT(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_STONE_EDGE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.2), results[0].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_NO_GUARD: two No Guard battlers apply the boost once", s16 damage)
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_MACHAMP) { Ability(ABILITY_NO_GUARD); Moves(MOVE_STONE_EDGE); }
        OPPONENT(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_STONE_EDGE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_NO_GUARD: a 100%-accurate move is unchanged", s16 damage)
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_ROCK_SMASH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_ROCK_SMASH); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}

SINGLE_BATTLE_TEST("BUFF_NO_GUARD: no boost with the flag off, or with accuracy rolls back on", s16 damage)
{
    enum Ability ability;
    bool32 buff, deterministic;

    PARAMETRIZE { buff = FALSE; deterministic = TRUE;  ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { buff = FALSE; deterministic = TRUE;  ability = ABILITY_GUTS; }
    PARAMETRIZE { buff = TRUE;  deterministic = FALSE; ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { buff = TRUE;  deterministic = FALSE; ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, buff);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, deterministic);
        PLAYER(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_STONE_EDGE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_STONE_EDGE, hit: TRUE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
        EXPECT_EQ(results[2].damage, results[3].damage);
    }
}

// The damage preview reads the same base-power chain, so it shows the boosted hit: the readout
// contains the damage actually dealt, and its top end sits above the Guts baseline's.
SINGLE_BATTLE_TEST("BUFF_NO_GUARD: the damage preview shows the boosted hit", s16 damage, u16 hi)
{
    enum Ability ability;
    u32 lo = 0, hi = 0, maxHP = 404;

    PARAMETRIZE { ability = ABILITY_NO_GUARD; }
    PARAMETRIZE { ability = ABILITY_GUTS; }

    GIVEN {
        WITH_CONFIG(BUFF_NO_GUARD, TRUE);
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_STONE_EDGE); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(maxHP); HP(maxHP); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_STONE_EDGE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } THEN {
        // Read the preview against the foe as it stood before the hit.
        gBattleMons[B_BATTLER_1].hp = maxHP;
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_STONE_EDGE, GIMMICK_NONE, &lo, &hi, &sKo));
        EXPECT((u32)results[i].damage * 100 >= lo * maxHP && (u32)results[i].damage * 100 < (hi + 1) * maxHP);
        results[i].hi = hi;
    } FINALLY {
        EXPECT_GT(results[0].hi, results[1].hi);
    }
}
