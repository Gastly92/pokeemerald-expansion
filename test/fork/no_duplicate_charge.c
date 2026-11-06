#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"

// Coverage for FEATURE_NO_DUPLICATE_CHARGE (config/feature.h). Electromorphosis charges on every
// damaging hit, so a holder that also has Wind Power would announce one Charge twice when hit by a
// damaging wind move. With the feature on, Wind Power's on-hit trigger yields to Electromorphosis.
// Only innates can put both on one battler; each test pairs a chosen ability with an innate.

SINGLE_BATTLE_TEST("FEATURE_NO_DUPLICATE_CHARGE: innate Wind Power stays silent beside chosen Electromorphosis")
{
    bool32 dedupe;
    PARAMETRIZE { dedupe = TRUE; }
    PARAMETRIZE { dedupe = FALSE; }
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_DUPLICATE_CHARGE, dedupe);
        ASSUME(SpeciesHasInnate(SPECIES_KILOWATTREL, ABILITY_WIND_POWER));
        ASSUME(IsWindMove(MOVE_AIR_CUTTER));
        PLAYER(SPECIES_KILOWATTREL) { Ability(ABILITY_ELECTROMORPHOSIS); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AIR_CUTTER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_AIR_CUTTER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AIR_CUTTER, opponent);
        HP_BAR(player);
        ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
        MESSAGE("Being hit by Air Cutter charged Kilowattrel with power!");
        if (dedupe)
        {
            NONE_OF {
                ABILITY_POPUP(player, ABILITY_WIND_POWER);
                MESSAGE("Being hit by Air Cutter charged Kilowattrel with power!");
            }
        }
        else
        {
            ABILITY_POPUP(player, ABILITY_WIND_POWER);
            MESSAGE("Being hit by Air Cutter charged Kilowattrel with power!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_DUPLICATE_CHARGE: chosen Wind Power stays silent beside innate Electromorphosis")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_DUPLICATE_CHARGE, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_BELLIBOLT, ABILITY_ELECTROMORPHOSIS));
        ASSUME(IsWindMove(MOVE_AIR_CUTTER));
        PLAYER(SPECIES_BELLIBOLT) { Ability(ABILITY_WIND_POWER); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AIR_CUTTER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_AIR_CUTTER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AIR_CUTTER, opponent);
        HP_BAR(player);
        NONE_OF { ABILITY_POPUP(player, ABILITY_WIND_POWER); }
        ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
        MESSAGE("Being hit by Air Cutter charged Bellibolt with power!");
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_WIND_POWER);
            MESSAGE("Being hit by Air Cutter charged Bellibolt with power!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_DUPLICATE_CHARGE: Wind Power still charges on Tailwind beside Electromorphosis")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_DUPLICATE_CHARGE, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_KILOWATTREL, ABILITY_WIND_POWER));
        ASSUME(GetMoveEffect(MOVE_TAILWIND) == EFFECT_TAILWIND);
        PLAYER(SPECIES_KILOWATTREL) { Ability(ABILITY_ELECTROMORPHOSIS); Moves(MOVE_TAILWIND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TAILWIND); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TAILWIND, player);
        ABILITY_POPUP(player, ABILITY_WIND_POWER); // Tailwind is Wind Power's own trigger; Electromorphosis has none
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_DUPLICATE_CHARGE: Zapdos with both innates shows one Charge pop-up for a wind move")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_DUPLICATE_CHARGE, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_ZAPDOS, ABILITY_WIND_POWER));
        ASSUME(SpeciesHasInnate(SPECIES_ZAPDOS, ABILITY_ELECTROMORPHOSIS));
        ASSUME(IsWindMove(MOVE_AIR_CUTTER));
        PLAYER(SPECIES_ZAPDOS) { Ability(ABILITY_STATIC); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_AIR_CUTTER); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_AIR_CUTTER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AIR_CUTTER, opponent);
        HP_BAR(player);
        NONE_OF { ABILITY_POPUP(player, ABILITY_WIND_POWER); }
        ABILITY_POPUP(player, ABILITY_ELECTROMORPHOSIS);
        MESSAGE("Being hit by Air Cutter charged Zapdos with power!");
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_WIND_POWER);
            MESSAGE("Being hit by Air Cutter charged Zapdos with power!");
        }
    }
}

SINGLE_BATTLE_TEST("FEATURE_NO_DUPLICATE_CHARGE: Zapdos's innate Wind Power charges it on Tailwind")
{
    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        WITH_CONFIG(FEATURE_NO_DUPLICATE_CHARGE, TRUE);
        ASSUME(SpeciesHasInnate(SPECIES_ZAPDOS, ABILITY_WIND_POWER));
        PLAYER(SPECIES_ZAPDOS) { Ability(ABILITY_STATIC); Moves(MOVE_TAILWIND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_TAILWIND); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TAILWIND, player);
        ABILITY_POPUP(player, ABILITY_WIND_POWER);
    }
}
