#include "global.h"
#include "test/battle.h"

// Coverage for BUFF_MICLE_BERRY (config/buff.h): at 1/4 HP, once an ally has fainted, the Berry
// revives the most recently fainted ally at 50% HP (full with Ripen) instead of boosting accuracy.
// BUFF_* flags default off in the test baseline (see TestInitConfigData), so each test opts in with
// WITH_CONFIG. The stock accuracy boost is covered by test/battle/hold_effect/micle_berry.c.

ASSUMPTIONS
{
    ASSUME(GetItemHoldEffect(ITEM_MICLE_BERRY) == HOLD_EFFECT_MICLE_BERRY);
    ASSUME(GetMoveEffect(MOVE_DRAGON_RAGE) == EFFECT_FIXED_HP_DAMAGE);
    ASSUME(GetMoveFixedHPDamage(MOVE_DRAGON_RAGE) == 40);
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: at 1/4 HP the Berry revives a fainted ally at 50% HP")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        MESSAGE("Wynaut fainted!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_RAGE, opponent);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        MESSAGE("Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
        NOT MESSAGE("Wobbuffet boosted the accuracy of its next move using Micle Berry!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 50);
        EXPECT_EQ(player->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: Ripen revives the ally at full HP")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        PLAYER(SPECIES_APPLIN) { MaxHP(160); HP(80); Ability(ABILITY_RIPEN); Item(ITEM_MICLE_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_RAGE, opponent);
        ABILITY_POPUP(player, ABILITY_RIPEN);
        MESSAGE("Applin's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 100);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: the Berry revives the most recently fainted ally")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
        PLAYER(SPECIES_ZIGZAGOON) { HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 2); }
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        MESSAGE("Wynaut fainted!");
        MESSAGE("Zigzagoon fainted!");
        MESSAGE("Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Zigzagoon was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 0);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][2], MON_DATA_HP), 50);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: with no fainted ally the Berry waits, then fires on the holder's switch-in")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
        TURN { SWITCH(player, 1); MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 0); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_RAGE, opponent);
        NOT MESSAGE("Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Wynaut fainted!");
        MESSAGE("Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), 50);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: works for an opposing holder too")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
        TURN { MOVE(player, MOVE_DRAGON_RAGE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_HP), 50);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: flag off keeps the stock accuracy boost")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, FALSE);
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(opponent, MOVE_DRAGON_RAGE); }
    } SCENE {
        MESSAGE("Wobbuffet boosted the accuracy of its next move using Micle Berry!");
        NOT MESSAGE("Wynaut was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 0);
    }
}

DOUBLE_BATTLE_TEST("BUFF_MICLE_BERRY: in doubles a partner that fainted on the field is sent back in")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(160); HP(80); Item(ITEM_MICLE_BERRY); }
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_SCRATCH, target: playerRight); }
        TURN { MOVE(opponentLeft, MOVE_DRAGON_RAGE, target: playerLeft); }
        TURN { MOVE(playerRight, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wynaut fainted!");
        MESSAGE("Wobbuffet's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, playerRight);
    } THEN {
        EXPECT_EQ(playerRight->hp, 50);
        EXPECT_EQ(playerLeft->item, ITEM_NONE);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: Stuff Cheeks eats the Berry at any HP to revive, and Cheek Pouch heals the holder")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        ASSUME(GetMoveEffect(MOVE_STUFF_CHEEKS) == EFFECT_STUFF_CHEEKS);
        PLAYER(SPECIES_WYNAUT) { HP(1); MaxHP(100); }
        PLAYER(SPECIES_GREEDENT) { Ability(ABILITY_CHEEK_POUCH); MaxHP(300); HP(250); Item(ITEM_MICLE_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(player, MOVE_STUFF_CHEEKS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_STUFF_CHEEKS, player);
        MESSAGE("Greedent's Micle Berry worked a miracle!");
        ABILITY_POPUP(player, ABILITY_CHEEK_POUCH);
        MESSAGE("Wynaut was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP), 50);
        EXPECT_EQ(player->statStages[STAT_DEF], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(player->hp, 300);
    }
}

SINGLE_BATTLE_TEST("BUFF_MICLE_BERRY: a Harvested Berry revives a second ally")
{
    GIVEN {
        WITH_CONFIG(BUFF_MICLE_BERRY, TRUE);
        WITH_CONFIG(DETERMINISTIC_ABILITIES, TRUE);
        ASSUME(GetMoveEffect(MOVE_SONIC_BOOM) == EFFECT_FIXED_HP_DAMAGE);
        ASSUME(GetMoveFixedHPDamage(MOVE_SONIC_BOOM) == 20);
        PLAYER(SPECIES_EXEGGUTOR) { Ability(ABILITY_HARVEST); MaxHP(160); HP(60); Item(ITEM_MICLE_BERRY); }
        PLAYER(SPECIES_WYNAUT) { HP(0); MaxHP(100); }
        PLAYER(SPECIES_ZIGZAGOON) { HP(0); MaxHP(100); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SONIC_BOOM); }
        TURN { MOVE(opponent, MOVE_SONIC_BOOM); }
    } SCENE {
        MESSAGE("Exeggutor's Micle Berry worked a miracle!");
        MESSAGE("Wynaut was revived and is ready to fight again!");
        MESSAGE("Exeggutor harvested its Micle Berry!");
        MESSAGE("Exeggutor's Micle Berry worked a miracle!");
        MESSAGE("Zigzagoon was revived and is ready to fight again!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP), 50);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][2], MON_DATA_HP), 50);
    }
}
