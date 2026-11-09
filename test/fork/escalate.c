#include "global.h"
#include "test/battle.h"

// Escalate (src/fork/escalate.c): at the end of each turn, every holder on the field adds
// ESCALATE_BONUS_PERCENT (2) to the DETERMINISTIC_DAMAGE multiplier, on top of that flag's own
// +1% per turn. Determinism flags default off in the test baseline, so each test opts in.
//
// The damage figures reuse test/fork/deterministic_damage.c's verified spread: Glaceon (Lv75,
// Atk 123) Ice Fang vs Garchomp (Def 163) deals 180 at 92-93%, 184 at 94-96% and 192 at 97-99%.

SINGLE_BATTLE_TEST("Escalate raises the damage multiplier by 2% at the end of each turn")
{
    s16 damage[3];
    ASSUME(DETERMINISTIC_DAMAGE_BASE_PERCENT == 92);
    ASSUME(DETERMINISTIC_DAMAGE_TURN_INCREMENT == 1);
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_GLACEON) { Level(75); Attack(123); Ability(ABILITY_ESCALATE); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_FANG); } // 92%
        TURN { MOVE(player, MOVE_ICE_FANG); } // 92 + 1 + 2 = 95%
        TURN { MOVE(player, MOVE_ICE_FANG); } // 92 + 2 + 4 = 98%
    } SCENE {
        for (u32 i = 0; i < 3; i++)
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, player);
            HP_BAR(opponent, captureDamage: &damage[i]);
            ABILITY_POPUP(player, ABILITY_ESCALATE);
            MESSAGE("Glaceon escalated the battle!");
        }
    } THEN {
        EXPECT_EQ(damage[0], 180);
        EXPECT_EQ(damage[1], 184);
        EXPECT_EQ(damage[2], 192);
    }
}

SINGLE_BATTLE_TEST("Escalate is field-wide: a foe's Escalate raises the holder's damage taken")
{
    s16 damage[2];
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_GLACEON) { Level(75); Attack(123); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); Ability(ABILITY_ESCALATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_FANG); } // 92%
        TURN { MOVE(player, MOVE_ICE_FANG); } // 95%
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ABILITY_POPUP(opponent, ABILITY_ESCALATE);
        MESSAGE("The opposing Garchomp escalated the battle!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        EXPECT_EQ(damage[0], 180);
        EXPECT_EQ(damage[1], 184);
    }
}

SINGLE_BATTLE_TEST("Escalate's bonus stays after the holder leaves the field")
{
    s16 damage;
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_GLACEON) { Level(75); Attack(123); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); Ability(ABILITY_ESCALATE); }
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); Ability(ABILITY_ROUGH_SKIN); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); } // ends with +2
        TURN { MOVE(player, MOVE_CELEBRATE); } // ends with +4
        TURN { SWITCH(opponent, 1); MOVE(player, MOVE_ICE_FANG); } // 92 + 2 + 4 = 98%, holder gone
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, player);
        HP_BAR(opponent, captureDamage: &damage);
    } THEN {
        EXPECT_EQ(damage, 192); // 94% without the kept bonus would deal 184
    }
}

SINGLE_BATTLE_TEST("Escalate does not activate on the turn the holder switched in")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_DRAGAPULT) { Ability(ABILITY_ESCALATE); }
    } WHEN {
        TURN { SWITCH(opponent, 1); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet used Celebrate!");
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_ESCALATE); }
        MESSAGE("Wobbuffet used Celebrate!");
        ABILITY_POPUP(opponent, ABILITY_ESCALATE);
        MESSAGE("The opposing Dragapult escalated the battle!");
    }
}

SINGLE_BATTLE_TEST("Escalate does not activate if the holder is knocked out during the turn")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_ESCALATE); HP(1); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_ESCALATE); }
        MESSAGE("Wynaut used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("Escalate does nothing without DETERMINISTIC_DAMAGE")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_ESCALATE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(player, ABILITY_ESCALATE); }
    }
}

DOUBLE_BATTLE_TEST("Escalate stacks: two holders add 4% per turn")
{
    s16 damage;
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_DAMAGE, TRUE);
        PLAYER(SPECIES_GLACEON) { Level(75); Attack(123); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GARCHOMP) { Defense(163); MaxHP(9999); HP(9999); Moves(MOVE_CELEBRATE); Ability(ABILITY_ESCALATE); }
        OPPONENT(SPECIES_DRAGAPULT) { Moves(MOVE_CELEBRATE); Ability(ABILITY_ESCALATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE); }
        TURN { MOVE(playerLeft, MOVE_ICE_FANG, target: opponentLeft); MOVE(playerRight, MOVE_CELEBRATE); } // 92 + 1 + 4 = 97%
    } SCENE {
        ABILITY_POPUP(opponentLeft, ABILITY_ESCALATE);
        ABILITY_POPUP(opponentRight, ABILITY_ESCALATE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ICE_FANG, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &damage);
    } THEN {
        EXPECT_EQ(damage, 192); // 95% from a single holder would deal 184
    }
}
