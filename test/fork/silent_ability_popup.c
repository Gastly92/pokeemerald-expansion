#include "global.h"
#include "test/battle.h"

// FEATURE_ANNOUNCE_SILENT_ABILITIES: an ability that only changes a hit's damage shows its
// pop-up, before the hit, the first time it takes effect.

SINGLE_BATTLE_TEST("Silent abilities: a damage ability's pop-up shows before the hit that reveals it")
{
    enum Species species;
    enum Ability ability;
    enum Move move;

    PARAMETRIZE { species = SPECIES_FURFROU;   ability = ABILITY_FLUFFY;     move = MOVE_TACKLE; }
    PARAMETRIZE { species = SPECIES_DRAGONITE; ability = ABILITY_MULTISCALE; move = MOVE_TACKLE; }
    PARAMETRIZE { species = SPECIES_SNORLAX;   ability = ABILITY_THICK_FAT;  move = MOVE_EMBER; }
    PARAMETRIZE { species = SPECIES_BRONZONG;  ability = ABILITY_HEATPROOF;  move = MOVE_EMBER; }
    PARAMETRIZE { species = SPECIES_MR_MIME;   ability = ABILITY_FILTER;     move = MOVE_SHADOW_BALL; }

    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(move); }
        OPPONENT(species) { Ability(ability); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, move); }
    } SCENE {
        ABILITY_POPUP(opponent, ability);
        ANIMATION(ANIM_TYPE_MOVE, move, player);
        HP_BAR(opponent);
    } THEN {
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
    }
}

SINGLE_BATTLE_TEST("Silent abilities: the pop-up shows once per mon, not on every hit")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_FURFROU) { Ability(ABILITY_FLUFFY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_FLUFFY);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_FLUFFY); }
    }
}

SINGLE_BATTLE_TEST("Silent abilities: no pop-up when the ability changes nothing")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_ROCK_SLIDE); }
        // Rock Slide makes no contact, so Fluffy does nothing; the innate Fur Coat that does
        // act is always known and never pops up.
        OPPONENT(SPECIES_FURFROU) { Ability(ABILITY_FLUFFY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_ROCK_SLIDE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent); }
    } THEN {
        EXPECT(!(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u));
    }
}

SINGLE_BATTLE_TEST("Silent abilities: no pop-up with the feature off, though the ability is still revealed")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_FURFROU) { Ability(ABILITY_FLUFFY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        NONE_OF { ABILITY_POPUP(opponent); }
    } THEN {
        EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
    }
}

SINGLE_BATTLE_TEST("Silent abilities: the attacker's Scrappy shows when it lets a Normal move hit a Ghost type")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_KANGASKHAN) { Ability(ABILITY_SCRAPPY); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_GENGAR) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_SCRAPPY);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_TACKLE, player);
        HP_BAR(opponent);
    }
}

SINGLE_BATTLE_TEST("Silent abilities: an ability with its own pop-up is not shown twice")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_EARTHQUAKE); }
        OPPONENT(SPECIES_BRONZONG) { Ability(ABILITY_LEVITATE); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_LEVITATE);
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_LEVITATE); }
    }
}

SINGLE_BATTLE_TEST("Silent abilities: the attacker's Infiltrator shows when it hits through Reflect")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_CROBAT) { Ability(ABILITY_INFILTRATOR); Speed(1); Moves(MOVE_WING_ATTACK); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); Moves(MOVE_REFLECT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_REFLECT); MOVE(player, MOVE_WING_ATTACK); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_REFLECT, opponent);
        ABILITY_POPUP(player, ABILITY_INFILTRATOR);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WING_ATTACK, player);
    }
}

SINGLE_BATTLE_TEST("Silent abilities: Battle Armor pops up when it blocks a guaranteed crit")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_FROST_BREATH); }
        OPPONENT(SPECIES_KABUTOPS) { Ability(ABILITY_BATTLE_ARMOR); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_FROST_BREATH); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_BATTLE_ARMOR);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FROST_BREATH, player);
        NONE_OF { MESSAGE("A critical hit!"); }
    }
}

DOUBLE_BATTLE_TEST("Silent abilities: a spread move shows each target's pop-up before the hit")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_HEAT_WAVE); }
        PLAYER(SPECIES_WYNAUT) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SNORLAX) { Ability(ABILITY_THICK_FAT); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_BRONZONG) { Ability(ABILITY_HEATPROOF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_HEAT_WAVE); }
    } SCENE {
        ABILITY_POPUP(opponentLeft, ABILITY_THICK_FAT);
        ABILITY_POPUP(opponentRight, ABILITY_HEATPROOF);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HEAT_WAVE, playerLeft);
    }
}

SINGLE_BATTLE_TEST("Silent abilities: a multi-hit move shows the pop-up once")
{
    GIVEN {
        WITH_CONFIG(FEATURE_ANNOUNCE_SILENT_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_DOUBLE_KICK); }
        OPPONENT(SPECIES_FURFROU) { Ability(ABILITY_FLUFFY); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_KICK); }
    } SCENE {
        ABILITY_POPUP(opponent, ABILITY_FLUFFY);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_KICK, player);
        HP_BAR(opponent);
        NONE_OF { ABILITY_POPUP(opponent, ABILITY_FLUFFY); }
    }
}
