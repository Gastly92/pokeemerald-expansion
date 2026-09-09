#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_ADDITIONAL_EFFECTS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so
// each test opts in with WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE). The
// stock random-chance secondary-effect tests (e.g. move.c, the scald/discharge
// tests) run unmodified with the flag off. Flinch is handled by its own flag
// (DETERMINISTIC_FLINCH), covered in deterministic_flinch.c.

SINGLE_BATTLE_TEST("DETERMINISTIC_ADDITIONAL_EFFECTS: a super-effective-capable move only applies its effect on a super effective hit")
{
    u32 species;
    bool32 burns;
    // Fire CAN be super effective, so its burn gates on the hit being super effective.
    PARAMETRIZE { species = SPECIES_TANGELA;   burns = TRUE;  } // Grass: Fire is 2x (super effective)
    PARAMETRIZE { species = SPECIES_WOBBUFFET;  burns = FALSE; } // Psychic: Fire is 1x (neutral)
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_FIRE_PUNCH, MOVE_EFFECT_BURN));
        ASSUME(GetMoveType(MOVE_FIRE_PUNCH) == TYPE_FIRE);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        // A non-Fire user with minimal Attack: isolates the super-effective rule and
        // keeps the bulky target alive so the burn can be observed.
        PLAYER(SPECIES_WOBBUFFET) { Attack(1); Moves(MOVE_FIRE_PUNCH); }
        OPPONENT(species) { MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_PUNCH); }
    } THEN {
        if (burns)
            EXPECT(opponent->status1 & STATUS1_BURN);
        else
            EXPECT(!(opponent->status1 & STATUS1_BURN));
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ADDITIONAL_EFFECTS: a move that can never be super effective only applies its effect from a STAB user")
{
    u32 species;
    bool32 paralyzes;
    // Normal can never be super effective, so Body Slam's paralysis gates on STAB.
    PARAMETRIZE { species = SPECIES_SNORLAX;    paralyzes = TRUE;  } // Normal: STAB
    PARAMETRIZE { species = SPECIES_WOBBUFFET;  paralyzes = FALSE; } // Psychic: not STAB
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_BODY_SLAM, MOVE_EFFECT_PARALYSIS));
        ASSUME(GetMoveType(MOVE_BODY_SLAM) == TYPE_NORMAL);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(species) { Attack(1); Moves(MOVE_BODY_SLAM); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_BODY_SLAM); }
    } THEN {
        if (paralyzes)
            EXPECT(opponent->status1 & STATUS1_PARALYSIS);
        else
            EXPECT(!(opponent->status1 & STATUS1_PARALYSIS));
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ADDITIONAL_EFFECTS: Serene Grace guarantees a normally-gated effect")
{
    // Fire vs Psychic is neutral (not super effective) and the user is not Fire (no
    // STAB), so without a chance-booster the burn would NOT land. Serene Grace turns
    // its stock chance-doubling into a guaranteed effect under the flag.
    GIVEN {
        ASSUME(MoveHasAdditionalEffect(MOVE_FIRE_PUNCH, MOVE_EFFECT_BURN));
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_SERENE_GRACE); Attack(1); Moves(MOVE_FIRE_PUNCH); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(600); HP(600); Defense(255); }
    } WHEN {
        TURN { MOVE(player, MOVE_FIRE_PUNCH); }
    } THEN {
        EXPECT(opponent->status1 & STATUS1_BURN);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_ADDITIONAL_EFFECTS: a guaranteed (100%) effect still always lands")
{
    // Nuzzle (Electric, 100% paralysis) is neither STAB for a Psychic user nor super
    // effective vs a Psychic target, but a guaranteed effect bypasses the rule.
    GIVEN {
        ASSUME(MoveHasAdditionalEffectWithChance(MOVE_NUZZLE, MOVE_EFFECT_PARALYSIS, 100));
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_NUZZLE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_NUZZLE); }
    } SCENE {
        STATUS_ICON(opponent, paralysis: TRUE);
    }
}

DOUBLE_BATTLE_TEST("DETERMINISTIC_ADDITIONAL_EFFECTS: G-Max Replenish always recycles allies' berries")
{
    // Upstream rolls a flat 50% for the berry recovery inside SetMoveEffect (the effect
    // itself is declared with no chance, so it never reaches TryTriggerAdditionalEffect).
    // With the flag on it always restores, for every value of the tag; the stock coin flip
    // is still covered by the flag-off tests in test/battle/gimmick/dynamax.c.
    PASSES_RANDOMLY(2, 2, RNG_G_MAX_REPLENISH);
    GIVEN {
        ASSUME(MoveHasAdditionalEffectSelf(MOVE_G_MAX_REPLENISH, MOVE_EFFECT_RECYCLE_BERRIES));
        ASSUME(GetItemHoldEffect(ITEM_APICOT_BERRY) == HOLD_EFFECT_SP_DEFENSE_UP);
        WITH_CONFIG(DETERMINISTIC_ADDITIONAL_EFFECTS, TRUE);
        PLAYER(SPECIES_SNORLAX) { Item(ITEM_APICOT_BERRY); GigantamaxFactor(TRUE); }
        PLAYER(SPECIES_MUNCHLAX) { Item(ITEM_APICOT_BERRY); Ability(ABILITY_THICK_FAT); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_STUFF_CHEEKS); MOVE(playerRight, MOVE_STUFF_CHEEKS); }
        TURN { MOVE(playerLeft, MOVE_SCRATCH, target: opponentLeft, gimmick: GIMMICK_DYNAMAX); }
    } SCENE {
        // turn 1
        MESSAGE("The Apicot Berry boosted Snorlax's Sp. Def!");
        MESSAGE("The Apicot Berry boosted Munchlax's Sp. Def!");
        // turn 2
        MESSAGE("Snorlax used G-Max Replenish!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_G_MAX_REPLENISH, playerLeft);
        MESSAGE("Snorlax found one Apicot Berry!");
        MESSAGE("Munchlax found one Apicot Berry!");
    }
}
