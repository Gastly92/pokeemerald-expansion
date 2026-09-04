#include "global.h"
#include "test/battle.h"

// FORK: BattleScript_WeatherAbilityActivates plays `playanimation_var BS_BATTLER_0, sB_ANIM_ARG1`,
// but TryChangeBattleWeather only fills animArg1 on its ability-set-the-weather branch. Sand Spit
// deliberately passes ABILITY_NONE, so before the fix it played whatever animation id happened to
// be left in that byte: B_ANIM_STATS_CHANGE (0) on the first activation of a battle, the previous
// weather's animation once one had ticked, and a binding move's id truncated to 8 bits after a Wrap
// tick — the last of which indexes past the 64-entry sBattleAnims_General[] and hangs the battle.
// Each test pins the animation played in the window between "A sandstorm kicked up!" and the first
// end-of-turn "The sandstorm is raging." — the only animation in that window is Sand Spit's.

SINGLE_BATTLE_TEST("Sand Spit plays the sandstorm animation with no prior weather")
{
    GIVEN {
        PLAYER(SPECIES_SANDACONDA) { Ability(ABILITY_SAND_SPIT); }
        OPPONENT(SPECIES_LANDORUS);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SAND_SPIT);
        MESSAGE("A sandstorm kicked up!");
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
        MESSAGE("The sandstorm is raging.");
    }
}

SINGLE_BATTLE_TEST("Sand Spit's sandstorm animation is the sandstorm one, not the leftover")
{
    GIVEN {
        PLAYER(SPECIES_SANDACONDA) { Ability(ABILITY_SAND_SPIT); }
        OPPONENT(SPECIES_LANDORUS);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("A sandstorm kicked up!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_SANDSTORM_CONTINUES, player);
        MESSAGE("The sandstorm is raging.");
    }
}

SINGLE_BATTLE_TEST("Sand Spit does not replay the outgoing weather's animation")
{
    GIVEN {
        PLAYER(SPECIES_SANDACONDA) { Ability(ABILITY_SAND_SPIT); }
        OPPONENT(SPECIES_LANDORUS);
    } WHEN {
        TURN { MOVE(player, MOVE_RAIN_DANCE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("It started to rain!");
        MESSAGE("Rain continues to fall.");
        ABILITY_POPUP(player, ABILITY_SAND_SPIT);
        MESSAGE("A sandstorm kicked up!");
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_RAIN_CONTINUES, player);
        MESSAGE("The sandstorm is raging.");
    }
}
