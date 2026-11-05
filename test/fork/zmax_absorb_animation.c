#include "global.h"
#include "test/battle.h"

// A Z-Move or Max Move that the target blocks before the accuracy step (an absorbing ability,
// Levitate, a type immunity) must not play its attack animation. The animation used to play
// *after* the ability pop-up, so every assertion here sits after ABILITY_POPUP / the message:
// a NOT ANIMATION placed before them cannot see it (upstream's "Max Moves don't bypass absorbing
// abilities" test in test/battle/gimmick/dynamax.c has exactly that blind spot).
// The fix is the UPSTREAM: early end in CancelerAccuracyCheck (src/battle_move_resolution.c).

#define ABSORB_PARAMS                                                                                                              \
    PARAMETRIZE { move = MOVE_SPARK;     item = ITEM_ELECTRIUM_Z; zMove = MOVE_GIGAVOLT_HAVOC;    maxMove = MOVE_MAX_LIGHTNING;  species = SPECIES_LANTURN;    ability = ABILITY_VOLT_ABSORB;  hp = 1; }   \
    PARAMETRIZE { move = MOVE_SPARK;     item = ITEM_ELECTRIUM_Z; zMove = MOVE_GIGAVOLT_HAVOC;    maxMove = MOVE_MAX_LIGHTNING;  species = SPECIES_LANTURN;    ability = ABILITY_VOLT_ABSORB;  hp = 100; } \
    PARAMETRIZE { move = MOVE_SPARK;     item = ITEM_ELECTRIUM_Z; zMove = MOVE_GIGAVOLT_HAVOC;    maxMove = MOVE_MAX_LIGHTNING;  species = SPECIES_ELECTIVIRE; ability = ABILITY_MOTOR_DRIVE;  hp = 100; } \
    PARAMETRIZE { move = MOVE_EMBER;     item = ITEM_FIRIUM_Z;    zMove = MOVE_INFERNO_OVERDRIVE; maxMove = MOVE_MAX_FLARE;      species = SPECIES_HEATRAN;    ability = ABILITY_FLASH_FIRE;   hp = 100; } \
    PARAMETRIZE { move = MOVE_VINE_WHIP; item = ITEM_GRASSIUM_Z;  zMove = MOVE_BLOOM_DOOM;        maxMove = MOVE_MAX_OVERGROWTH; species = SPECIES_MILTANK;    ability = ABILITY_SAP_SIPPER;   hp = 100; } \
    PARAMETRIZE { move = MOVE_MUD_BOMB;  item = ITEM_GROUNDIUM_Z; zMove = MOVE_TECTONIC_RAGE;     maxMove = MOVE_MAX_QUAKE;      species = SPECIES_BRONZONG;   ability = ABILITY_LEVITATE;     hp = 100; }

SINGLE_BATTLE_TEST("Z-Moves blocked by an ability don't play their attack animation")
{
    enum Move move, zMove, maxMove;
    enum Item item;
    enum Species species;
    enum Ability ability;
    u32 hp;
    ABSORB_PARAMS
    GIVEN {
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); Moves(move); }
        OPPONENT(species) { Ability(ability); HP(hp); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_Z_MOVE); }
        (void)maxMove;
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_ZMOVE_ACTIVATE, player);
        ABILITY_POPUP(opponent, ability);
        NOT ANIMATION(ANIM_TYPE_MOVE, zMove, player);
    }
}

SINGLE_BATTLE_TEST("Max Moves blocked by an ability don't play their attack animation")
{
    enum Move move, zMove, maxMove;
    enum Item item;
    enum Species species;
    enum Ability ability;
    u32 hp;
    ABSORB_PARAMS
    GIVEN {
        WITH_CONFIG(B_REDIRECT_ABILITY_IMMUNITY, GEN_5);
        PLAYER(SPECIES_WOBBUFFET) { Moves(move); }
        OPPONENT(species) { Ability(ability); HP(hp); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, move, gimmick: GIMMICK_DYNAMAX); }
        (void)zMove; (void)item;
    } SCENE {
        ABILITY_POPUP(opponent, ability);
        NOT ANIMATION(ANIM_TYPE_MOVE, maxMove, player);
    }
}

SINGLE_BATTLE_TEST("Z-Moves and Max Moves into a type immunity don't play their attack animation")
{
    enum Gimmick gimmick;
    enum Move animMove;
    PARAMETRIZE { gimmick = GIMMICK_Z_MOVE;  animMove = MOVE_BREAKNECK_BLITZ; }
    PARAMETRIZE { gimmick = GIMMICK_DYNAMAX; animMove = MOVE_MAX_STRIKE; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Item(gimmick == GIMMICK_Z_MOVE ? ITEM_NORMALIUM_Z : ITEM_NONE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_GENGAR);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE, gimmick: gimmick); }
    } SCENE {
        MESSAGE("It doesn't affect the opposing Gengar…");
        NOT ANIMATION(ANIM_TYPE_MOVE, animMove, player);
    }
}

SINGLE_BATTLE_TEST("Z-Moves blocked by an ability don't animate with item-free gimmicks either")
{
    enum Gimmick gimmick;
    enum Move animMove;
    PARAMETRIZE { gimmick = GIMMICK_Z_MOVE;  animMove = MOVE_GIGAVOLT_HAVOC; }
    PARAMETRIZE { gimmick = GIMMICK_DYNAMAX; animMove = MOVE_MAX_LIGHTNING; }
    GIVEN {
        WITH_CONFIG(FEATURE_FREE_GIMMICKS, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPARK); }
        OPPONENT(SPECIES_LANTURN) { Ability(ABILITY_VOLT_ABSORB); HP(1); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPARK, gimmick: gimmick); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_VOLT_ABSORB);
        NOT ANIMATION(ANIM_TYPE_MOVE, animMove, player);
    }
}

SINGLE_BATTLE_TEST("Z-Moves and Max Moves that connect still play their attack animation")
{
    enum Gimmick gimmick;
    enum Move animMove;
    PARAMETRIZE { gimmick = GIMMICK_Z_MOVE;  animMove = MOVE_GIGAVOLT_HAVOC; }
    PARAMETRIZE { gimmick = GIMMICK_DYNAMAX; animMove = MOVE_MAX_LIGHTNING; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Item(gimmick == GIMMICK_Z_MOVE ? ITEM_ELECTRIUM_Z : ITEM_NONE); Moves(MOVE_SPARK); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SPARK, gimmick: gimmick); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, animMove, player);
        HP_BAR(opponent);
    }
}
