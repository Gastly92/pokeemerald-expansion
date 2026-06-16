#include "global.h"
#include "test/battle.h"

// FORK: coverage for the DETERMINISTIC_MOVE_RESULTS flag (config/deterministic.h).
// Determinism flags default off in the test baseline (see TestInitConfigData), so each
// test opts in with WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE); the stock random
// outcomes run unmodified in the inherited suite.

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: a 2-5 hit move always hits three times")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(IsMultiHitMove(MOVE_DOUBLE_SLAP));
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_DOUBLE_SLAP); }
        OPPONENT(SPECIES_CHANSEY);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_SLAP); }
    } SCENE {
        MESSAGE("The Pokémon was hit 3 time(s)!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Loaded Dice raises a multi-hit move to the guaranteed maximum")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetItemHoldEffect(ITEM_LOADED_DICE) == HOLD_EFFECT_LOADED_DICE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_LOADED_DICE); Moves(MOVE_DOUBLE_SLAP); }
        OPPONENT(SPECIES_CHANSEY);
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_SLAP); }
    } SCENE {
        MESSAGE("The Pokémon was hit 5 time(s)!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Population Bomb hits five times without Loaded Dice")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetMoveEffect(MOVE_POPULATION_BOMB) == EFFECT_POPULATION_BOMB);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_POPULATION_BOMB); }
        OPPONENT(SPECIES_CHANSEY);
    } WHEN {
        TURN { MOVE(player, MOVE_POPULATION_BOMB); }
    } SCENE {
        MESSAGE("The Pokémon was hit 5 time(s)!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Population Bomb hits ten times while holding Loaded Dice")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetMoveEffect(MOVE_POPULATION_BOMB) == EFFECT_POPULATION_BOMB);
        ASSUME(GetItemHoldEffect(ITEM_LOADED_DICE) == HOLD_EFFECT_LOADED_DICE);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_LOADED_DICE); Moves(MOVE_POPULATION_BOMB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POPULATION_BOMB); }
    } SCENE {
        MESSAGE("The Pokémon was hit 10 time(s)!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Population Bomb hits ten times with Skill Link")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetMoveEffect(MOVE_POPULATION_BOMB) == EFFECT_POPULATION_BOMB);
        PLAYER(SPECIES_PIKIPEK) { Ability(ABILITY_SKILL_LINK); Moves(MOVE_POPULATION_BOMB); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_POPULATION_BOMB); }
    } SCENE {
        MESSAGE("The Pokémon was hit 10 time(s)!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Protect fails when used on consecutive turns")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_PROTECT); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(50); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_PROTECT); MOVE(opponent, MOVE_TACKLE); } // protects
        TURN { MOVE(player, MOVE_PROTECT); MOVE(opponent, MOVE_TACKLE); } // fails the second time
    } SCENE {
        MESSAGE("Wobbuffet protected itself!");
        MESSAGE("But it failed!");
        MESSAGE("The opposing Wobbuffet used Tackle!");
        HP_BAR(player);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: a speed tie is won by higher raw base Speed")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        // Equal final Speed, so the tie is broken by base Speed (Ninjask 160 > Wobbuffet 33).
        PLAYER(SPECIES_NINJASK) { Speed(100); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("Ninjask used Celebrate!");
        MESSAGE("The opposing Wobbuffet used Celebrate!");
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: a rampage move lasts exactly two turns")
{
    // Exactly two Thrash hits, then the user confuses itself from fatigue. A third
    // forced Thrash would push the confusion past the two provided turns.
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_THRASH); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THRASH); }
        TURN { SKIP_TURN(player); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THRASH, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THRASH, player);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_CONFUSION, player);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Wrap binds for a fixed number of turns")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_WRAP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WRAP); }
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WRAP, player);
        HP_BAR(opponent); // direct damage
        HP_BAR(opponent); // residual 1
        HP_BAR(opponent); // residual 2
        HP_BAR(opponent); // residual 3
        HP_BAR(opponent); // residual 4 (DETERMINISTIC_WRAP_TURNS)
        NOT HP_BAR(opponent); // freed afterward
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Grip Claw extends a binding move's duration")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(GetItemHoldEffect(ITEM_GRIP_CLAW) == HOLD_EFFECT_GRIP_CLAW);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_GRIP_CLAW); Moves(MOVE_WRAP); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_WRAP); }
        TURN {}
        TURN {}
        TURN {}
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WRAP, player);
        HP_BAR(opponent); // direct damage
        HP_BAR(opponent); // residual 1
        HP_BAR(opponent); // residual 2
        HP_BAR(opponent); // residual 3
        HP_BAR(opponent); // residual 4
        HP_BAR(opponent); // residual 5
        HP_BAR(opponent); // residual 6
        HP_BAR(opponent); // residual 7 (DETERMINISTIC_WRAP_GRIP_CLAW_TURNS)
        NOT HP_BAR(opponent); // freed afterward
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Tri Attack burns a target with higher Attack")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TRI_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Attack(200); SpAttack(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_ATTACK); }
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_ATTACK, player);
        STATUS_ICON(opponent, burn: TRUE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Tri Attack frostbites a target with higher Sp. Atk")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        WITH_CONFIG(B_USE_FROSTBITE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TRI_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Attack(50); SpAttack(200); }
    } WHEN {
        TURN { MOVE(player, MOVE_TRI_ATTACK); }
        TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TRI_ATTACK, player);
        FREEZE_OR_FROSTBURN_STATUS(opponent, TRUE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Dire Claw paralyzes a target whose Speed beats its defenses")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        ASSUME(MoveHasAdditionalEffect(MOVE_DIRE_CLAW, MOVE_EFFECT_DIRE_CLAW));
        PLAYER(SPECIES_WOBBUFFET) { Speed(100); Moves(MOVE_DIRE_CLAW); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(200); Defense(50); SpDefense(50); }
    } WHEN {
        TURN { MOVE(player, MOVE_DIRE_CLAW); }
        TURN {}
    } SCENE {
        STATUS_ICON(opponent, paralysis: TRUE);
    }
}

SINGLE_BATTLE_TEST("DETERMINISTIC_MOVE_RESULTS: Roar drags out the next party member in order")
{
    GIVEN {
        WITH_CONFIG(DETERMINISTIC_MOVE_RESULTS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_ROAR); }
        OPPONENT(SPECIES_WOBBUFFET);  // slot 0, active
        OPPONENT(SPECIES_BULBASAUR);  // slot 1, next in order
        OPPONENT(SPECIES_CHARMANDER); // slot 2
    } WHEN {
        TURN { MOVE(player, MOVE_ROAR); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ROAR, player);
        MESSAGE("The opposing Bulbasaur was dragged out!");
    }
}
