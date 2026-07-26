#include "global.h"
#include "test/battle.h"

// FORK: Psychic Affinity is the first "Affinity" ability -- it grants the holder a latent
// third type (Psychic) in battle, applied at the GetBattlerTypes() chokepoint. See
// src/fork/type_affinity.c and fork-docs/NEW_ABILITIES.md. These probe the mechanic from all
// three angles: the STAB it grants, the weakness it adds, and its suppression under Tera.

SINGLE_BATTLE_TEST("Psychic Affinity grants the holder Psychic-type STAB", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_COMPOUND_EYES; }
    PARAMETRIZE { ability = ABILITY_PSYCHIC_AFFINITY; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_PSYCHIC) == TYPE_PSYCHIC);
        ASSUME(gSpeciesInfo[SPECIES_BUTTERFREE].types[0] != TYPE_PSYCHIC);
        ASSUME(gSpeciesInfo[SPECIES_BUTTERFREE].types[1] != TYPE_PSYCHIC);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ability); }
        OPPONENT(SPECIES_SNORLAX); // pure Normal: neutral to Psychic, bulky enough to survive
    } WHEN {
        TURN { MOVE(player, MOVE_PSYCHIC); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, UQ_4_12(1.5), results[1].damage); // 1.5x STAB only in the affinity run
    }
}

SINGLE_BATTLE_TEST("Psychic Affinity adds the Psychic type's weaknesses", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_COMPOUND_EYES; }
    PARAMETRIZE { ability = ABILITY_PSYCHIC_AFFINITY; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_DARK_PULSE) == TYPE_DARK);
        ASSUME(gTypeEffectivenessTable[TYPE_DARK][TYPE_PSYCHIC] > UQ_4_12(1.0));
        PLAYER(SPECIES_WOBBUFFET); // low offense, so frail Butterfree survives even a 2x hit
        OPPONENT(SPECIES_BUTTERFREE) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, MOVE_DARK_PULSE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, UQ_4_12(2.0), results[1].damage); // Dark goes 1x -> 2x once Psychic is added
    }
}

SINGLE_BATTLE_TEST("Psychic Affinity announces its latent type on switch-in")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_BUTTERFREE) { Ability(ABILITY_PSYCHIC_AFFINITY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_PSYCHIC_AFFINITY);
        MESSAGE("Butterfree's affinity awakened its latent Psychic type!");
    }
}

SINGLE_BATTLE_TEST("Psychic Affinity's added type is suppressed while Terastallized", s16 damage)
{
    u32 tera;
    PARAMETRIZE { tera = FALSE; }
    PARAMETRIZE { tera = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_DARK_PULSE) == TYPE_DARK);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_BUTTERFREE) { Ability(ABILITY_PSYCHIC_AFFINITY); TeraType(TYPE_BUG); }
    } WHEN {
        TURN { if (tera) MOVE(opponent, MOVE_CELEBRATE, gimmick: GIMMICK_TERA);
               else MOVE(opponent, MOVE_CELEBRATE);
               MOVE(player, MOVE_DARK_PULSE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Non-tera: Bug/Flying/Psychic takes 2x from Dark. Tera Bug: pure Bug, the latent
        // Psychic is suppressed, so Dark is back to 1x.
        EXPECT_MUL_EQ(results[1].damage, UQ_4_12(2.0), results[0].damage);
    }
}
