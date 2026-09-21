#include "global.h"
#include "test/battle.h"

// UPSTREAM: regression test for the damage-reducing Berry pop-up naming the wrong item.
// The pop-up is shown from BattleScript_BerryReduceAnimation, called at PRE_ANIM_RESIST_BERRY
// in CancelerPreAnimActivations (src/battle_move_resolution.c). That path never set
// gLastUsedItem -- only the later message step did -- so the pop-up printed whatever item had
// activated before, and "????" when nothing had. Field report: a Roseli Berry on Kommo-o
// reading "????" against an opposing Max Starfall.

SINGLE_BATTLE_TEST("Damage-reducing Berry pop-up names the Berry")
{
    GIVEN {
        ASSUME(GetItemHoldEffect(ITEM_ROSELI_BERRY) == HOLD_EFFECT_RESIST_BERRY);
        ASSUME(GetItemHoldEffectParam(ITEM_ROSELI_BERRY) == TYPE_FAIRY);
        ASSUME(GetMoveType(MOVE_DISARMING_VOICE) == TYPE_FAIRY);
        PLAYER(SPECIES_KOMMO_O) { Item(ITEM_ROSELI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DISARMING_VOICE); }
    } SCENE {
        ITEM_POPUP(player, ITEM_ROSELI_BERRY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        MESSAGE("Kommo-o's Roseli Berry lessened the damage it took!");
    }
}

SINGLE_BATTLE_TEST("Damage-reducing Berry pop-up names the Berry against a Max Move")
{
    GIVEN {
        PLAYER(SPECIES_KOMMO_O) { Item(ITEM_ROSELI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DISARMING_VOICE, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        ITEM_POPUP(player, ITEM_ROSELI_BERRY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, player);
        MESSAGE("Kommo-o's Roseli Berry lessened the damage it took!");
    }
}

// The pop-up must name the Berry even when a different item activated earlier in the battle,
// which is the case the stale-global bug hid behind.
SINGLE_BATTLE_TEST("Damage-reducing Berry pop-up is not the previously used item")
{
    GIVEN {
        PLAYER(SPECIES_KOMMO_O) { Item(ITEM_ROSELI_BERRY); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_FAIRY_GEM); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_DISARMING_VOICE); }
    } SCENE {
        ITEM_POPUP(opponent, ITEM_FAIRY_GEM);
        ITEM_POPUP(player, ITEM_ROSELI_BERRY);
        MESSAGE("Kommo-o's Roseli Berry lessened the damage it took!");
    }
}
