#include "global.h"
#include "test/battle.h"

// UPSTREAM: regression test for the Custap Berry pop-up appearing over the wrong battler.
// CheckChangingTurnOrderEffects (src/battle_main.c) walks battlers with
// `battler = gBattlerAttacker = gBattleScripting.battler++`, so by the time
// BattleScript_CustapBerryActivation runs, BS_SCRIPTING already points one battler past the
// holder. The script showed its pop-up on BS_SCRIPTING, so an opposing holder's pop-up was
// drawn for a nonexistent battler (unset coords: top-left, partly off screen) and a player
// holder's appeared on the opponent's side. It now uses BS_ATTACKER, like Quick Claw.
// Field report: an opposing Golem's Custap Berry pop-up sitting above-left of its healthbox.

SINGLE_BATTLE_TEST("Custap Berry pop-up appears on the player holder")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); MaxHP(160); HP(40); Item(ITEM_CUSTAP_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ITEM_POPUP(player, ITEM_CUSTAP_BERRY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
    }
}

SINGLE_BATTLE_TEST("Custap Berry pop-up appears on the opposing holder")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(2); }
        OPPONENT(SPECIES_GOLEM) { Speed(1); MaxHP(160); HP(40); Item(ITEM_CUSTAP_BERRY); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ITEM_POPUP(opponent, ITEM_CUSTAP_BERRY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponent);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
    }
}

DOUBLE_BATTLE_TEST("Custap Berry pop-up appears on the holder in a double battle")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Speed(4); }
        PLAYER(SPECIES_WYNAUT) { Speed(3); }
        OPPONENT(SPECIES_GOLEM) { Speed(1); MaxHP(160); HP(40); Item(ITEM_CUSTAP_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); }
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_SCRATCH, target: playerLeft); }
    } SCENE {
        ITEM_POPUP(opponentLeft, ITEM_CUSTAP_BERRY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponentLeft);
    }
}
