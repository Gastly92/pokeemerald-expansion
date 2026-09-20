#include "global.h"
#include "test/battle.h"

// FORK: the Gen9+ Battle Bond boost is announced for the ABILITY HOLDER, never for the
// foe it just knocked out. Upstream's Gen9 Battle Bond tests (test/battle/ability/battle_bond.c)
// assert the stat stages and the pop-up but never the message text, so a misattributed
// "{B_SCR_NAME_WITH_PREFIX}'s ... rose!" would pass there unnoticed. These two pin the
// message itself: the plain case, and the full reported sequence (foe Dynamaxes and KOs the
// lead with a Max Move, Greninja comes in and KOs it with an item-free Z-Move under
// FEATURE_FREE_GIMMICKS, with FEATURE_INNATE_ABILITIES on so Rhyperior really has the
// Bulletproof ability override that blanks Bullet Seed).

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

SINGLE_BATTLE_TEST("Battle Bond names Greninja after an item-free Z-Move KOs a Dynamaxed foe (Gen9+)")
{
    GIVEN {
        WITH_CONFIG(B_BATTLE_BOND, GEN_9);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_CINCCINO) { Speed(1); HP(1); }
        PLAYER(SPECIES_GRENINJA_BATTLE_BOND) { Ability(ABILITY_BATTLE_BOND); Item(ITEM_EXPERT_BELT); Speed(1); }
        OPPONENT(SPECIES_RHYPERIOR) { Ability(ABILITY_BULLETPROOF); HP(1); Speed(2); Attack(1); Item(ITEM_WEAKNESS_POLICY); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(1); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_EARTHQUAKE, gimmick: GIMMICK_DYNAMAX); MOVE(player, MOVE_BULLET_SEED); SEND_OUT(player, 1); }
        TURN { MOVE(opponent, MOVE_EARTHQUAKE); MOVE(player, MOVE_HYDRO_PUMP, gimmick: GIMMICK_Z_MOVE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Rhyperior fainted!");
        ABILITY_POPUP(player, ABILITY_BATTLE_BOND);
        MESSAGE("Greninja's Attack rose!");
        MESSAGE("Greninja's Sp. Atk rose!");
        MESSAGE("Greninja's Speed rose!");
    }
}
