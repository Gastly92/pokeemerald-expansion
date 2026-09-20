#include "global.h"
#include "test/battle.h"

// FORK: regression coverage for a wrong-battler stat-change message.
//
// ShouldDoTrainerSlide() used to set gBattleScripting.battler BEFORE deciding whether a slide
// actually runs, so every "no slide" answer still left the global pointing at the OPPONENT.
// RunTurnActionsFunctions() re-probes it every frame while a Z-Move gimmick is active
// (TryTrainerSlideGimmick), so in a trainer battle the global was stomped between a script
// setting it and the next message being queued. Battle Bond is where it shows: the boost lands
// on Greninja, but all three lines name the foe it just knocked out.
//
// The trainer-battle + Z-Move + AI combination is what makes it fire, which is why it only ever
// turned up in Frontier play -- hence AI_SINGLE_BATTLE_TEST rather than a scripted opponent.

AI_SINGLE_BATTLE_TEST("Battle Bond's boosts are announced for Greninja after a Z-Move KO in a trainer battle")
{
    GIVEN {
        WITH_CONFIG(B_BATTLE_BOND, GEN_9);
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        AI_FLAGS(AI_FLAG_BASIC_TRAINER);
        PLAYER(SPECIES_CINCCINO) { Item(ITEM_LIFE_ORB); Speed(50); HP(20); Attack(1); }
        PLAYER(SPECIES_GRENINJA_BATTLE_BOND) { Ability(ABILITY_BATTLE_BOND); Item(ITEM_EXPERT_BELT); Speed(50); }
        OPPONENT(SPECIES_DONPHAN) { Speed(1); Attack(200); HP(80); Moves(MOVE_EARTHQUAKE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); EXPECT_MOVE(opponent, MOVE_EARTHQUAKE); SEND_OUT(player, 1); }
        TURN { MOVE(player, MOVE_HYDRO_PUMP, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        MESSAGE("The opposing Donphan fainted!");
        ABILITY_POPUP(player, ABILITY_BATTLE_BOND);
        MESSAGE("Greninja's Attack rose!");
        MESSAGE("Greninja's Sp. Atk rose!");
        MESSAGE("Greninja's Speed rose!");
        NONE_OF {
            MESSAGE("The opposing Donphan's Attack rose!");
            MESSAGE("The opposing Donphan's Sp. Atk rose!");
            MESSAGE("The opposing Donphan's Speed rose!");
        }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
    }
}

// The plain case, kept from the first pass: upstream's Gen9 Battle Bond tests assert the stat
// stages and the pop-up but never the message text.
SINGLE_BATTLE_TEST("Battle Bond's stat boosts are announced for Greninja, not the fainted foe (Gen9+)")
{
    GIVEN {
        WITH_CONFIG(B_BATTLE_BOND, GEN_9);
        PLAYER(SPECIES_GRENINJA_BATTLE_BOND) { Ability(ABILITY_BATTLE_BOND); }
        OPPONENT(SPECIES_RHYPERIOR) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WATER_GUN); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Rhyperior fainted!");
        ABILITY_POPUP(player, ABILITY_BATTLE_BOND);
        MESSAGE("Greninja's Attack rose!");
        MESSAGE("Greninja's Sp. Atk rose!");
        MESSAGE("Greninja's Speed rose!");
    }
}
