#include "global.h"
#include "test/battle.h"

// FORK: Halo projects a field-wide per-hit damage cap (HALO_DAMAGE_CAP_PERCENT% of the
// target's max HP) and charges its holder HALO_PP_TAX extra PP per move for it. See
// src/fork/halo.c and fork-docs/NEW_ABILITIES.md. These probe every angle the mechanic
// touches: the cap on the holder, the cap on OTHER battlers (it is an aura, not armour), the
// fixed-damage exemption, the per-hit (not per-turn) granularity, the PP upkeep, suppression,
// and the switch-in announcement.

SINGLE_BATTLE_TEST("Halo caps a hit on its holder at 40% of its max HP", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_SHIELD_DUST; }
    PARAMETRIZE { ability = ABILITY_HALO; }
    GIVEN {
        PLAYER(SPECIES_MACHAMP) { Moves(MOVE_CLOSE_COMBAT); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ability); MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(player, MOVE_CLOSE_COMBAT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[0].damage, 120); // uncapped, this hit is worth more than the cap
        EXPECT_EQ(results[1].damage, 120); // 40% of 300
    }
}

SINGLE_BATTLE_TEST("Halo caps hits on every battler, not only its holder", s16 damage)
{
    // The aura property: Halo sits on the ATTACKER here, and it still bounds the damage its
    // own attack deals to the foe.
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_SHIELD_DUST; }
    PARAMETRIZE { ability = ABILITY_HALO; }
    GIVEN {
        PLAYER(SPECIES_MACHAMP) { Ability(ability); Moves(MOVE_CLOSE_COMBAT); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(300); HP(300); }
    } WHEN {
        TURN { MOVE(player, MOVE_CLOSE_COMBAT); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_GT(results[0].damage, 120);
        EXPECT_EQ(results[1].damage, 120);
    }
}

SINGLE_BATTLE_TEST("Halo does not cap fixed-damage moves")
{
    // Seismic Toss deals the user's level and runs through DoFixedDamageMoveCalc(), which the
    // cap deliberately never sees: 100 damage lands in full despite the 80 (40% of 200) cap.
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_SEISMIC_TOSS) == EFFECT_LEVEL_DAMAGE);
        PLAYER(SPECIES_WOBBUFFET) { Level(100); Moves(MOVE_SEISMIC_TOSS); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_HALO); MaxHP(200); HP(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_SEISMIC_TOSS); }
    } SCENE {
        HP_BAR(opponent, damage: 100);
    }
}

SINGLE_BATTLE_TEST("Halo's cap is per hit, so a multi-hit move can exceed it in one turn")
{
    // Deliberate counterplay: the clamp lives in the per-hit damage calc, so a move that hits
    // three times (DETERMINISTIC_MOVE_RESULTS) can take up to three caps' worth in a turn.
    // The cap here is 12 (40% of 30), so no SINGLE hit could ever KO this target -- fainting
    // is therefore proof that more than one capped hit landed in the same turn.
    GIVEN {
        ASSUME(IsMultiHitMove(MOVE_BULLET_SEED));
        PLAYER(SPECIES_BRELOOM) { Moves(MOVE_BULLET_SEED); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_HALO); MaxHP(30); HP(30); }
    } WHEN {
        TURN { MOVE(player, MOVE_BULLET_SEED); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
    }
}

SINGLE_BATTLE_TEST("Halo's holder pays an extra PP for every move it uses")
{
    enum Ability ability;
    u32 expectedPP;
    PARAMETRIZE { ability = ABILITY_SHIELD_DUST; expectedPP = 34; } // 35 - 1 (normal)
    PARAMETRIZE { ability = ABILITY_HALO;        expectedPP = 33; } // 35 - 1 - 1 (halo upkeep)
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); Moves(MOVE_POUND); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POUND); }
    } THEN {
        EXPECT_EQ(player->pp[0], expectedPP);
    }
}

SINGLE_BATTLE_TEST("Halo's cap lifts once the ability is suppressed", s16 damage)
{
    u32 suppress;
    PARAMETRIZE { suppress = FALSE; }
    PARAMETRIZE { suppress = TRUE; }
    GIVEN {
        PLAYER(SPECIES_MACHAMP) { Moves(MOVE_CLOSE_COMBAT, MOVE_GASTRO_ACID, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_HALO); MaxHP(300); HP(300); }
    } WHEN {
        TURN { if (suppress) MOVE(player, MOVE_GASTRO_ACID); else MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CLOSE_COMBAT); }
    } SCENE {
        if (suppress)
            MESSAGE("The opposing Wobbuffet's Ability was suppressed!");
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, 120); // still capped
        EXPECT_GT(results[1].damage, 120); // Gastro Acid puts the halo out
    }
}

SINGLE_BATTLE_TEST("Halo announces itself on switch-in")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_CLEFABLE) { Ability(ABILITY_HALO); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_HALO);
        MESSAGE("A halo appears above Clefable!");
    }
}
