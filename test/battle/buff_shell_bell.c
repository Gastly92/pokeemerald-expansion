#include "global.h"
#include "test/battle.h"

// FORK: coverage for the BUFF_SHELL_BELL flag (config/buff.h). BUFF_* flags
// default off in the test baseline (see TestInitConfigData), so each test opts in
// with WITH_CONFIG(BUFF_SHELL_BELL, TRUE). The stock 1/8 behavior is exercised by
// the inherited tests in test/battle/hold_effect/shell_bell.c (buff off).

ASSUMPTIONS
{
    ASSUME(gItemsInfo[ITEM_SHELL_BELL].holdEffect == HOLD_EFFECT_SHELL_BELL);
}

SINGLE_BATTLE_TEST("BUFF_SHELL_BELL: Shell Bell restores 1/4 HP of damage dealt")
{
    s16 damage = 0;
    s16 healed = 0;

    GIVEN {
        WITH_CONFIG(BUFF_SHELL_BELL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_SHELL_BELL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        HP_BAR(player, captureDamage: &healed);
    } THEN {
        EXPECT_EQ(damage / 4, -1 * healed);
    }
}

#define HITS 5
SINGLE_BATTLE_TEST("BUFF_SHELL_BELL: Shell Bell recovers 1/4 of total damage from all hits of a multi hit move")
{
    s16 multiHitDamage[HITS];
    s16 totalDamage = 0;
    s16 shellBellRecovery = 0;

    GIVEN {
        WITH_CONFIG(BUFF_SHELL_BELL, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_SHELL_BELL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); }
    } SCENE {
        for (u32 i = 0; i < HITS; i++) {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_BULLET_SEED, player);
            HP_BAR(opponent, captureDamage: &multiHitDamage[i]);
            totalDamage += multiHitDamage[i];
        }
        HP_BAR(player, captureDamage: &shellBellRecovery);
    } THEN {
        EXPECT_EQ(totalDamage / 4, -1 * shellBellRecovery);
    }
}
#undef HITS

SINGLE_BATTLE_TEST("BUFF_SHELL_BELL off: Shell Bell still restores the stock 1/8 HP of damage dealt")
{
    s16 damage = 0;
    s16 healed = 0;

    GIVEN {
        WITH_CONFIG(BUFF_SHELL_BELL, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_SHELL_BELL); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_EFFECT, player);
        HP_BAR(player, captureDamage: &healed);
    } THEN {
        EXPECT_EQ(damage / 8, -1 * healed);
    }
}
