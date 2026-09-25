#include "global.h"
#include "test/battle.h"

// UPSTREAM: BattleScript_SolarPowerActivates (shared by Dry Skin in sun) prints no message, so its
// pop-up used to still be on screen when the next end-turn event ran: with Grassy Terrain up, the
// Solar Power pop-up sat over "healed by the grassy terrain!" and the 1/8 HP drop read as if it
// had never happened. The script now waits for the pop-up to slide out (waitabilitypopup). The
// runner can't see sprites, so these pin the sequence and HP maths the fix must preserve, and
// would hang if the wait never released.

AI_SINGLE_BATTLE_TEST("Solar Power's HP loss resolves before the Grassy Terrain heal")
{
    u32 speed;
    PARAMETRIZE { speed = 1; }
    PARAMETRIZE { speed = 200; }
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT);
        PLAYER(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_GRASSY_TERRAIN); }
        OPPONENT(SPECIES_SUNFLORA) { Ability(ABILITY_SOLAR_POWER); Item(ITEM_HEAT_ROCK); Speed(speed); MaxHP(160); HP(160); Moves(MOVE_SUNNY_DAY); }
    } WHEN {
        TURN { MOVE(player, MOVE_GRASSY_TERRAIN); EXPECT_MOVE(opponent, MOVE_SUNNY_DAY); }
    } SCENE {
        MESSAGE("The opposing Sunflora used Sunny Day!");
        ABILITY_POPUP(opponent, ABILITY_SOLAR_POWER);
        HP_BAR(opponent, damage: 20);
        MESSAGE("The opposing Sunflora is healed by the grassy terrain!");
        HP_BAR(opponent, damage: -10);
    } THEN {
        EXPECT_EQ(opponent->hp, 150);
    }
}

SINGLE_BATTLE_TEST("Dry Skin's sun HP loss resolves before the Grassy Terrain heal")
{
    GIVEN {
        PLAYER(SPECIES_PARASECT) { Ability(ABILITY_DRY_SKIN); MaxHP(160); HP(160); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_GRASSY_TERRAIN); MOVE(player, MOVE_SUNNY_DAY); }
    } SCENE {
        MESSAGE("Parasect used Sunny Day!");
        ABILITY_POPUP(player, ABILITY_DRY_SKIN);
        HP_BAR(player, damage: 20);
        MESSAGE("Parasect is healed by the grassy terrain!");
        HP_BAR(player, damage: -10);
    } THEN {
        EXPECT_EQ(player->hp, 150);
    }
}
