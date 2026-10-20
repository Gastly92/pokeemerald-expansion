#include "global.h"
#include "test/battle.h"

// Coverage for FEATURE_ANNOUNCE_FLINCH_PREVENTION (config/feature.h, src/fork/flinch_prevention.c).
// Feature flags default off in the test baseline, so each test opts in with WITH_CONFIG. Fake Out
// is a guaranteed flinch, so it isolates the announcement from the deterministic flinch gate; the
// gate itself is covered at the end with Bite.

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: Shield Dust announces a flinch it blocked")
{
    bool32 announce;
    PARAMETRIZE { announce = TRUE; }
    PARAMETRIZE { announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, announce);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_VENOMOTH) { Ability(ABILITY_SHIELD_DUST); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FAKE_OUT, player);
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
            MESSAGE("The opposing Venomoth's Shield Dust prevents flinching!");
        }
        else
        {
            NONE_OF {
                ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
                MESSAGE("The opposing Venomoth's Shield Dust prevents flinching!");
            }
        }
        MESSAGE("The opposing Venomoth used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: Inner Focus announces a flinch it blocked")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_ZUBAT) { Ability(ABILITY_INNER_FOCUS); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FAKE_OUT, player);
        ABILITY_POPUP(opponent, ABILITY_INNER_FOCUS);
        MESSAGE("The opposing Zubat's Inner Focus prevents flinching!");
        MESSAGE("The opposing Zubat used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: Covert Cloak announces a flinch it blocked, without a pop-up")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, TRUE);
        ASSUME(gItemsInfo[ITEM_COVERT_CLOAK].holdEffect == HOLD_EFFECT_COVERT_CLOAK);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_COVERT_CLOAK); Speed(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FAKE_OUT, player);
        NOT ABILITY_POPUP(opponent);
        MESSAGE("The opposing Wobbuffet's Covert Cloak prevents flinching!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: an innate Shield Dust is named in the pop-up and the message")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, TRUE);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); }
        OPPONENT(SPECIES_VENOMOTH) { Ability(ABILITY_TINTED_LENS); Speed(50); } // innate Shield Dust
    } WHEN {
        TURN { MOVE(player, MOVE_FAKE_OUT); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FAKE_OUT, player);
        ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
        MESSAGE("The opposing Venomoth's Shield Dust prevents flinching!");
        MESSAGE("The opposing Venomoth used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: no announcement when the target already moved this turn")
{
    // Venomoth outspeeds, so the flinch could not have landed and nothing is announced.
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); }
        OPPONENT(SPECIES_VENOMOTH) { Ability(ABILITY_SHIELD_DUST); Speed(100); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_BITE); }
    } SCENE {
        MESSAGE("The opposing Venomoth used Celebrate!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BITE, player);
        NONE_OF {
            ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
            MESSAGE("The opposing Venomoth's Shield Dust prevents flinching!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_ANNOUNCE_FLINCH_PREVENTION: with deterministic flinch, only a flinch the gate would have let through is announced")
{
    u32 species;
    bool32 announce;
    // Bite is Dark: super effective vs Psychic (the gate lets the flinch through), neutral vs Normal.
    PARAMETRIZE { species = SPECIES_WOBBUFFET; announce = TRUE; }
    PARAMETRIZE { species = SPECIES_SNORLAX;   announce = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_FLINCH_PREVENTION, TRUE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        WITH_CONFIG(DETERMINISTIC_FLINCH, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Attack(1); Speed(100); }
        OPPONENT(species) { Ability(ABILITY_SHIELD_DUST); Speed(50); MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_BITE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BITE, player);
        if (announce)
        {
            ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
        }
        else
        {
            NOT ABILITY_POPUP(opponent, ABILITY_SHIELD_DUST);
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_CELEBRATE, opponent);
    }
}
