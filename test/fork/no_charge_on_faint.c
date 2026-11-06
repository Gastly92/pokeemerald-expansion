#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Coverage for FEATURE_NO_CHARGE_ON_FAINT (config/feature.h). Stock Electromorphosis and Wind
// Power announce a Charge even when the triggering hit knocks the holder out, though the Charge
// is lost with the faint. With the feature on they stay silent in that case only.

SINGLE_BATTLE_TEST("FEATURE_NO_CHARGE_ON_FAINT: Electromorphosis stays silent when the hit knocks the holder out")
{
    bool32 skip;
    PARAMETRIZE { skip = TRUE; }
    PARAMETRIZE { skip = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_NO_CHARGE_ON_FAINT, skip);
        PLAYER(SPECIES_BELLIBOLT) { Ability(ABILITY_ELECTROMORPHOSIS); HP(1); Speed(1); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        HP_BAR(player);
        if (skip)
        {
            NONE_OF {
                ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
                MESSAGE("Being hit by Scratch charged Bellibolt with power!");
            }
        }
        else
        {
            ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
            MESSAGE("Being hit by Scratch charged Bellibolt with power!");
        }
        MESSAGE("Bellibolt fainted!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_CHARGE_ON_FAINT: Wind Power stays silent when the hit knocks the holder out")
{
    GIVEN {
        WITH_CONFIG(FEATURE_NO_CHARGE_ON_FAINT, TRUE);
        ASSUME(IsWindMove(MOVE_GUST));
        PLAYER(SPECIES_WATTREL) { Ability(ABILITY_WIND_POWER); HP(1); Speed(1); }
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_GUST); SEND_OUT(player, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_GUST, opponent);
        HP_BAR(player);
        NONE_OF { ABILITY_POPUP(player, ABILITY_WIND_POWER); }
        MESSAGE("Wattrel fainted!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_CHARGE_ON_FAINT: innate Electromorphosis stays silent when the hit knocks the holder out")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_CHARGE_ON_FAINT, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_BELLIBOLT, ABILITY_ELECTROMORPHOSIS));
        PLAYER(SPECIES_BELLIBOLT) { Ability(ABILITY_DAMP); HP(1); Speed(1); } // Electromorphosis only as innate
        PLAYER(SPECIES_WOBBUFFET) { Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); SEND_OUT(player, 1); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        HP_BAR(player);
        NONE_OF { ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS); }
        MESSAGE("Bellibolt fainted!");
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_CHARGE_ON_FAINT: Electromorphosis still charges when the holder survives the hit")
{
    GIVEN {
        WITH_CONFIG(FEATURE_NO_CHARGE_ON_FAINT, TRUE);
        PLAYER(SPECIES_BELLIBOLT) { Ability(ABILITY_ELECTROMORPHOSIS); Speed(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(10); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        HP_BAR(player);
        ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
        MESSAGE("Being hit by Scratch charged Bellibolt with power!");
    }
}
