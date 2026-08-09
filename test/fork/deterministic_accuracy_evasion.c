#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_ACCURACY_EVASION flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so each
// test opts in with WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE). With the flag on,
// accuracy/evasion stop deciding hit/miss and instead drive a PP economy; sub-100%
// moves always hit but have their max PP scaled down by their accuracy, OHKO moves
// deal a fixed % of max HP, and 50%-accurate moves gain a Hyper Beam-style recharge.

ASSUMPTIONS
{
    ASSUME(GetMovePP(MOVE_POUND) == 35);
    ASSUME(GetMoveAccuracy(MOVE_POUND) == 100);
    ASSUME(GetMovePP(MOVE_HYDRO_PUMP) == 5);
    ASSUME(GetMoveAccuracy(MOVE_HYDRO_PUMP) == 80);
    ASSUME(GetMoveEffect(MOVE_HORN_DRILL) == EFFECT_OHKO);
    ASSUME(GetMoveAccuracy(MOVE_HYPNOSIS) == 60);
    ASSUME(GetMoveNonVolatileStatus(MOVE_HYPNOSIS) == MOVE_EFFECT_SLEEP);
    ASSUME(GetMoveAccuracy(MOVE_SPORE) == 100);
    ASSUME(GetMoveNonVolatileStatus(MOVE_SPORE) == MOVE_EFFECT_SLEEP);
    ASSUME(GetMoveAccuracy(MOVE_ZAP_CANNON) == 50);
    ASSUME(GetMoveAccuracy(MOVE_SWIFT) == 0);
    ASSUME(GetMovePP(MOVE_SWIFT) == 20);
    ASSUME(gItemsInfo[ITEM_MICLE_BERRY].holdEffect == HOLD_EFFECT_MICLE_BERRY);
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a sub-100% move always hits, even into raised evasion")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_CELEBRATE, MOVE_HYDRO_PUMP); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_HYDRO_PUMP); MOVE(opponent, MOVE_DOUBLE_TEAM); }
    } SCENE {
        MESSAGE("Wobbuffet used Hydro Pump!");
        HP_BAR(opponent); // it lands despite the evasion boost
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: max PP is scaled down by base accuracy")
{
    u32 flag;
    u32 expectedPP;
    PARAMETRIZE { flag = FALSE; expectedPP = 5; } // stock
    PARAMETRIZE { flag = TRUE;  expectedPP = 4; } // floor(5 * 0.80)
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, flag);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_HYDRO_PUMP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], expectedPP); // Hydro Pump unused, so pp == its (scaled) max
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: max PP scaling rounds down")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(GetMovePP(MOVE_MEGAHORN) == 10);
        ASSUME(GetMoveAccuracy(MOVE_MEGAHORN) == 85);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_MEGAHORN); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], 8); // floor(10 * 0.85) = 8, not 9 (round-to-nearest)
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: 100% moves keep full PP and aren't scaled")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], 35);
    }
}

// A move whose MISS cost more than the wasted turn is priced at a discount
// (DETERMINISTIC_EXTRA_MISS_COST_PERCENT of its real accuracy), so the max-PP scaling
// charges it for the whole drawback the flag removes rather than one roll's worth:
// EFFECT_TRIPLE_KICK rolled accuracy once per strike, and EFFECT_RECOIL_IF_MISS staked
// half the user's max HP on the roll. Parametrized over every move in both effects so a
// change to either the constant or the move data shows up here.
SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a move whose miss cost more than a turn is priced below its accuracy")
{
    u32 move;
    u32 flag;
    u32 expectedPP;
    PARAMETRIZE { move = MOVE_TRIPLE_AXEL;    flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { move = MOVE_TRIPLE_AXEL;    flag = TRUE;  expectedPP = 6; }  // floor(10 * 0.90 * 0.72), not floor(10 * 0.90)
    PARAMETRIZE { move = MOVE_TRIPLE_KICK;    flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { move = MOVE_TRIPLE_KICK;    flag = TRUE;  expectedPP = 6; }  // same effect, so same price
    PARAMETRIZE { move = MOVE_HIGH_JUMP_KICK; flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { move = MOVE_HIGH_JUMP_KICK; flag = TRUE;  expectedPP = 6; }
    PARAMETRIZE { move = MOVE_AXE_KICK;       flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { move = MOVE_AXE_KICK;       flag = TRUE;  expectedPP = 6; }
    PARAMETRIZE { move = MOVE_JUMP_KICK;      flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { move = MOVE_JUMP_KICK;      flag = TRUE;  expectedPP = 6; }  // floor(10 * 0.95 * 0.72)
    PARAMETRIZE { move = MOVE_SUPERCELL_SLAM; flag = FALSE; expectedPP = 15; }
    PARAMETRIZE { move = MOVE_SUPERCELL_SLAM; flag = TRUE;  expectedPP = 10; } // floor(15 * 0.95 * 0.72)
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, flag);
        ASSUME(GetMoveEffect(move) == EFFECT_TRIPLE_KICK || GetMoveEffect(move) == EFFECT_RECOIL_IF_MISS);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, move); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], expectedPP); // unused, so pp == its (scaled) max
    }
}

// Population Bomb rolls accuracy per strike too, but is deliberately NOT discounted: it
// already pays on the strike-count axis (DETERMINISTIC_POPULATION_BOMB_COUNT). It still
// takes the ordinary accuracy scaling like any other 90% move.
SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: Population Bomb is priced at its plain accuracy")
{
    u32 flag;
    u32 expectedPP;
    PARAMETRIZE { flag = FALSE; expectedPP = 10; }
    PARAMETRIZE { flag = TRUE;  expectedPP = 9; } // floor(10 * 0.90), not the discounted price
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, flag);
        ASSUME(GetMoveEffect(MOVE_POPULATION_BOMB) == EFFECT_POPULATION_BOMB);
        ASSUME(GetMoveAccuracy(MOVE_POPULATION_BOMB) == 90);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_POPULATION_BOMB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], expectedPP);
    }
}

// The discount above is priced on the assumption that a crash move keeps its drawback
// where the target is genuinely unaffected. Protect sets MOVE_RESULT_PROTECTED, which is
// part of MOVE_RESULT_NO_EFFECT, which is what MoveEndMoveBlockRecoil gates the crash on —
// so the crash survives the flag. If this ever stops holding, the discount is too small.
SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a crash move still crashes into Protect")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        ASSUME(!MoveIgnoresProtect(MOVE_JUMP_KICK));
        ASSUME(GetMoveEffect(MOVE_JUMP_KICK) == EFFECT_RECOIL_IF_MISS);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_PROTECT); MOVE(player, MOVE_JUMP_KICK); }
    } SCENE {
        s32 maxHP = GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PROTECT, opponent);
        HP_BAR(player, damage: maxHP / 2);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: raised target evasion costs extra PP")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // 35 - 1 (base) - 1 (target +1 evasion)
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: accuracy and evasion cancel out")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HONE_CLAWS, MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_HONE_CLAWS); MOVE(opponent, MOVE_DOUBLE_TEAM); } // player +1 acc, opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[1], 34); // net stage 0 -> normal 1 PP
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: raised user accuracy recovers PP")
{
    // Pressure drains the move below max so the +1 PP recovery from raised accuracy is
    // observable (a refund into a full PP bar would just clamp away).
    u32 useHoneClaws;
    u32 expectedPP;
    PARAMETRIZE { useHoneClaws = FALSE; expectedPP = 33; } // 35 - 1 (base) - 1 (Pressure)
    PARAMETRIZE { useHoneClaws = TRUE;  expectedPP = 34; } // 35 - 1 - 1 + 1 (accuracy recovery)
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HONE_CLAWS, MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN { MOVE(player, useHoneClaws ? MOVE_HONE_CLAWS : MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[1], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: BrightPowder adds 1 PP to offensive moves")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_BRIGHT_POWDER); }
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], 33); // 35 - 1 (base) - 1 (BrightPowder)
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a move spends its last PP even when it would cost more")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        // Hydro Pump's scaled max is 4 PP; drain it to 1, then cost 2 into raised evasion.
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HYDRO_PUMP, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); HP(600); MaxHP(600); }
    } WHEN {
        TURN { MOVE(player, MOVE_HYDRO_PUMP); MOVE(opponent, MOVE_CELEBRATE); } // 4 -> 3
        TURN { MOVE(player, MOVE_HYDRO_PUMP); MOVE(opponent, MOVE_CELEBRATE); } // 3 -> 2
        TURN { MOVE(player, MOVE_HYDRO_PUMP); MOVE(opponent, MOVE_DOUBLE_TEAM); } // 2 -> 1, opponent +1 evasion
        TURN { MOVE(player, MOVE_HYDRO_PUMP); MOVE(opponent, MOVE_CELEBRATE); } // cost 2, only 1 left -> 0
    } THEN {
        EXPECT_EQ(player->pp[0], 0);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: OHKO moves deal a fixed % of max HP")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HORN_DRILL); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(100); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_HORN_DRILL); }
    } THEN {
        EXPECT_EQ(opponent->hp, 60); // 100 - 40% of max
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: OHKO moves are still blocked by Sturdy")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HORN_DRILL); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_STURDY); HP(100); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_HORN_DRILL); }
    } THEN {
        EXPECT_EQ(opponent->hp, 100); // Sturdy grants full immunity
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: OHKO moves are still blocked by a higher-level target")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Level(50); Moves(MOVE_HORN_DRILL); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(100); HP(100); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_HORN_DRILL); }
    } THEN {
        EXPECT_EQ(opponent->hp, 100); // higher level grants immunity
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: Micle Berry's move ignores the evasion PP tax")
{
    u32 item;
    u32 expectedPP;
    PARAMETRIZE { item = ITEM_NONE;        expectedPP = 33; } // 35 - 1 (base) - 1 (foe +1 evasion)
    PARAMETRIZE { item = ITEM_MICLE_BERRY; expectedPP = 35; } // evasion tax ignored (cost 1) + 1 PP refund -> net 0
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        // High Defense so the trigger hit can't KO; player is slower so it takes the hit
        // (dropping it to <= 1/4 HP and activating Micle) before using Pound that same turn.
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(100); HP(26); Defense(999); Speed(20); Item(item); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_DOUBLE_TEAM, MOVE_SCRATCH); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // foe +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_SCRATCH); } // Scratch drops player to <=1/4 -> Micle activates, then Pound benefits
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a 0-accuracy move ignores the evasion PP tax")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_SWIFT, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_SWIFT); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], 19); // flat 1 PP; the +1 evasion imposes no tax
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: No Guard's user ignores the evasion PP tax")
{
    u32 ability;
    u32 expectedPP;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; expectedPP = 33; } // 35 - 1 (base) - 1 (foe +1 evasion)
    PARAMETRIZE { ability = ABILITY_NO_GUARD;  expectedPP = 34; } // accuracy is 100% regardless of evasion
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: No Guard on the TARGET also ignores the evasion PP tax")
{
    u32 ability;
    u32 expectedPP;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; expectedPP = 33; } // 35 - 1 (base) - 1 (foe +1 evasion)
    PARAMETRIZE { ability = ABILITY_NO_GUARD;  expectedPP = 34; } // moves against a No Guard holder always hit too
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_DOUBLE_TEAM); } // opponent +1 evasion
        TURN { MOVE(player, MOVE_POUND); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: No Guard ignores the flat BrightPowder PP tax")
{
    u32 ability;
    u32 expectedPP;
    PARAMETRIZE { ability = ABILITY_TELEPATHY; expectedPP = 33; } // 35 - 1 (base) - 1 (BrightPowder)
    PARAMETRIZE { ability = ABILITY_NO_GUARD;  expectedPP = 34; } // 100% accuracy overrides the item
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_BRIGHT_POWDER); }
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: No Guard still recovers PP from raised accuracy")
{
    // Pure boon: No Guard drops the penalties without clamping away the boost recovery.
    // Pressure drains the move below max so the +1 PP recovery is observable.
    u32 useHoneClaws;
    u32 expectedPP;
    PARAMETRIZE { useHoneClaws = FALSE; expectedPP = 33; } // 35 - 1 (base) - 1 (Pressure)
    PARAMETRIZE { useHoneClaws = TRUE;  expectedPP = 34; } // 35 - 1 - 1 + 1 (accuracy recovery)
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_NO_GUARD); Moves(MOVE_HONE_CLAWS, MOVE_POUND, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN { MOVE(player, useHoneClaws ? MOVE_HONE_CLAWS : MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[1], expectedPP);
    }
}

DOUBLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a partner's Victory Star ignores the evasion PP tax")
{
    u32 allyAbility;
    u32 expectedPP;
    PARAMETRIZE { allyAbility = ABILITY_TELEPATHY;    expectedPP = 33; } // 35 - 1 (base) - 1 (foe +1 evasion)
    PARAMETRIZE { allyAbility = ABILITY_VICTORY_STAR; expectedPP = 34; } // the ally's accuracy boost ignores evasion
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_POUND, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Ability(allyAbility); Speed(50); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_DOUBLE_TEAM, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE);
               MOVE(opponentLeft, MOVE_DOUBLE_TEAM); MOVE(opponentRight, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_POUND, target: opponentLeft); MOVE(playerRight, MOVE_CELEBRATE);
               MOVE(opponentLeft, MOVE_CELEBRATE); MOVE(opponentRight, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(playerLeft->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a sub-100% sleep move sleeps outright (never misses)")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HYPNOSIS); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_HYPNOSIS); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fell asleep!");
    } THEN {
        EXPECT(opponent->status1 & STATUS1_SLEEP); // sub-100% sleep moves now sleep like any other
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a 100% sleep move (Spore) sleeps directly")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fell asleep!");
    } THEN {
        EXPECT(opponent->status1 & STATUS1_SLEEP);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ACCURACY_EVASION: a 50% accurate move requires a recharge turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_ACCURACY_EVASION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_ZAP_CANNON, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(800); MaxHP(800); }
    } WHEN {
        TURN { MOVE(player, MOVE_ZAP_CANNON); }
        TURN { SKIP_TURN(player); } // locked into recharge
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Zap Cannon!");
        MESSAGE("Wobbuffet must recharge!");
        MESSAGE("Wobbuffet used Celebrate!");
    }
}
