#include "global.h"
#include "test/battle.h"

// FORK: Water Affinity is the second "Affinity" ability -- it grants the holder a latent third
// type (Water) in battle, applied at the GetBattlerTypes() chokepoint. See src/fork/type_affinity.c
// and fork-docs/NEW_ABILITIES.md. Its holders are sea-dwelling species that aren't officially Water
// (Lugia, Masquerain, Beartic, Dhelmise, Cursola, Cetitan). These probe the angles the shared
// machinery reaches on a *second* family member -- the STAB it grants, the weakness it adds and the
// resistance it adds -- plus the switch-in message, which must name Water rather than Psychic.
// The family's composition rules (Tera suppression, an occupied third slot) are covered once for
// the whole family in test/battle/ability/psychic_affinity.c.

SINGLE_BATTLE_TEST("Water Affinity grants the holder Water-type STAB", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }
    PARAMETRIZE { ability = ABILITY_WATER_AFFINITY; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_SURF) == TYPE_WATER);
        ASSUME(gSpeciesInfo[SPECIES_LUGIA].types[0] != TYPE_WATER);
        ASSUME(gSpeciesInfo[SPECIES_LUGIA].types[1] != TYPE_WATER);
        PLAYER(SPECIES_LUGIA) { Ability(ability); }
        OPPONENT(SPECIES_SNORLAX); // pure Normal: neutral to Water, bulky enough to survive
    } WHEN {
        TURN { MOVE(player, MOVE_SURF); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, UQ_4_12(1.5), results[1].damage); // 1.5x STAB only in the affinity run
    }
}

SINGLE_BATTLE_TEST("Water Affinity adds the Water type's weaknesses", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }
    PARAMETRIZE { ability = ABILITY_WATER_AFFINITY; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_THUNDERBOLT) == TYPE_ELECTRIC);
        ASSUME(gTypeEffectivenessTable[TYPE_ELECTRIC][TYPE_WATER] > UQ_4_12(1.0));
        PLAYER(SPECIES_WOBBUFFET); // low offense, so Lugia survives even the 4x hit
        OPPONENT(SPECIES_LUGIA) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDERBOLT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, UQ_4_12(2.0), results[1].damage); // Electric goes 2x -> 4x on Psychic/Flying/Water
    }
}

// The other half of the trade: the latent type also brings its RESISTANCES, so the Ice move
// that Lugia's Flying half is weak to comes back down to neutral.
SINGLE_BATTLE_TEST("Water Affinity adds the Water type's resistances", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_PRESSURE; }
    PARAMETRIZE { ability = ABILITY_WATER_AFFINITY; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_ICE_BEAM) == TYPE_ICE);
        ASSUME(gTypeEffectivenessTable[TYPE_ICE][TYPE_WATER] < UQ_4_12(1.0));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_LUGIA) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, MOVE_ICE_BEAM); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, UQ_4_12(2.0), results[0].damage); // Ice goes 2x -> 1x once Water is added
    }
}

SINGLE_BATTLE_TEST("Water Affinity announces its latent type on switch-in")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_LUGIA) { Ability(ABILITY_WATER_AFFINITY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_WATER_AFFINITY);
        MESSAGE("Lugia's affinity awakened its latent Water type!");
    }
}
