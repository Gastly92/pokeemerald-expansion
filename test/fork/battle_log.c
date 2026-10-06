#include "global.h"
#include "test/battle.h"
#include "fork/battle_log.h"

// B_FRONTIER_BATTLE_INFO's Battle Log page. The log records every move used and the damage
// it dealt as a % of the target's max HP, newest first. These tests read the recorder directly;
// the page only lays its entries out.

static u32 ExpectedPercent(s32 damage, u32 maxHP)
{
    u32 pct = (damage * 100 + maxHP / 2) / maxHP;
    return (pct == 0 && damage != 0) ? 1 : pct;
}

SINGLE_BATTLE_TEST("Battle Log: records both sides' moves, newest first, with damage as a % of max HP")
{
    s16 damage;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); MaxHP(400); HP(400); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        HP_BAR(opponent, captureDamage: &damage);
    } THEN {
        const struct BattleLogEntry *celebrate = BattleLogGetNewest(0);
        const struct BattleLogEntry *tackle = BattleLogGetNewest(1);

        EXPECT_EQ(BattleLogCount(), 2);

        EXPECT_EQ(celebrate->move, MOVE_CELEBRATE);
        EXPECT(!celebrate->attackerOnPlayerSide);
        EXPECT(!celebrate->hasTarget);

        EXPECT_EQ(tackle->move, MOVE_TACKLE);
        EXPECT(tackle->attackerOnPlayerSide);
        EXPECT_EQ(tackle->attackerSpecies, SPECIES_WOBBUFFET);
        EXPECT(tackle->hasTarget);
        EXPECT_EQ(tackle->turn, 0);
        EXPECT(!tackle->ko);
        EXPECT_EQ(BattleLogEntryPercent(tackle), ExpectedPercent(damage, 400));
    }
}

SINGLE_BATTLE_TEST("Battle Log: a multi-hit move is one entry carrying the sum of its hits")
{
    s16 hit1, hit2;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_DOUBLE_KICK); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(400); HP(400); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &hit1);
        HP_BAR(opponent, captureDamage: &hit2);
    } THEN {
        const struct BattleLogEntry *kick = BattleLogGetNewest(1);

        EXPECT_EQ(BattleLogCount(), 2);
        EXPECT_EQ(kick->move, MOVE_DOUBLE_KICK);
        EXPECT_EQ(kick->hpLost, hit1 + hit2);
        EXPECT_EQ(BattleLogEntryPercent(kick), ExpectedPercent(hit1 + hit2, 400));
    }
}

SINGLE_BATTLE_TEST("Battle Log: a knockout is flagged")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } THEN {
        const struct BattleLogEntry *tackle = BattleLogGetNewest(0);

        EXPECT_EQ(tackle->move, MOVE_TACKLE);
        EXPECT(tackle->hasTarget);
        EXPECT(tackle->ko);
    }
}

SINGLE_BATTLE_TEST("Battle Log: a hit taken by a Substitute is marked as such")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_SUBSTITUTE); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUBSTITUTE); MOVE(player, MOVE_TACKLE); }
    } THEN {
        const struct BattleLogEntry *tackle = BattleLogGetNewest(0);

        EXPECT_EQ(tackle->move, MOVE_TACKLE);
        EXPECT(tackle->hasTarget);
        EXPECT(tackle->substitute);
        EXPECT_EQ(tackle->hpLost, 0);
    }
}

DOUBLE_BATTLE_TEST("Battle Log: a spread move gets one entry per target it hit")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_SURF); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(30); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_SURF);
            MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, MOVE_CELEBRATE);
            MOVE(opponentRight, MOVE_CELEBRATE);
        }
    } THEN {
        u32 surfHits = 0;
        u32 targets = 0;

        for (u32 i = 0; i < BattleLogCount(); i++)
        {
            const struct BattleLogEntry *entry = BattleLogGetNewest(i);

            if (entry->move != MOVE_SURF)
                continue;
            surfHits++;
            EXPECT(entry->hasTarget);
            EXPECT_EQ((u32)entry->attacker, B_POSITION_PLAYER_LEFT);
            targets |= 1u << entry->target;
        }
        // Surf hits both foes and the user's ally.
        EXPECT_EQ(surfHits, 3);
        EXPECT_EQ(targets, (1u << B_POSITION_OPPONENT_LEFT) | (1u << B_POSITION_OPPONENT_RIGHT) | (1u << B_POSITION_PLAYER_RIGHT));
    }
}
