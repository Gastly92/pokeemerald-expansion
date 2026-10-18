#include "global.h"
#include "test/battle.h"
#include "fork/innate_abilities.h"
#include "fork/rekindle.h"

// Rekindle: each battler the holder knocks out with a move heals it for 1/4 of its max HP.
// A custom ability is a real ability, so the chosen-ability tests need no WITH_CONFIG; only the
// innate test at the bottom toggles FEATURE_INNATE_ABILITIES.

SINGLE_BATTLE_TEST("Rekindle heals 1/4 of max HP when the holder knocks out a foe")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON); // a second foe, so the KO doesn't end the battle
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        ABILITY_POPUP(player, ABILITY_REKINDLE);
        HP_BAR(player);
    } THEN {
        EXPECT_EQ(player->hp, 1 + player->maxHP / REKINDLE_HEAL_FRACTION);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate at full HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    } THEN {
        EXPECT_EQ(player->hp, player->maxHP);
    }
}

SINGLE_BATTLE_TEST("Rekindle never heals past max HP")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); MaxHP(100); HP(99); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_REKINDLE);
    } THEN {
        EXPECT_EQ(player->hp, 100);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate under Heal Block")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_HEAL_BLOCK) == EFFECT_HEAL_BLOCK);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Speed(1); Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(2); Moves(MOVE_HEAL_BLOCK, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ZIGZAGOON) { Speed(2); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_HEAL_BLOCK); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_CELEBRATE); MOVE(player, MOVE_TACKLE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate when the knockout ends the battle")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_TACKLE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_TACKLE); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    }
}

// Like Moxie, every battler the move knocks out counts -- the holder's own ally included.
DOUBLE_BATTLE_TEST("Rekindle heals once for each battler the holder knocks out, allies included")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_EARTHQUAKE) == TARGET_FOES_AND_ALLY);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); }
        PLAYER(SPECIES_SNORUNT) { HP(1); }
        OPPONENT(SPECIES_GLALIE) { HP(1); }
        OPPONENT(SPECIES_ABRA) { HP(1); }
        OPPONENT(SPECIES_ABRA);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_EARTHQUAKE); SEND_OUT(opponentLeft, 2); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_EARTHQUAKE, playerLeft);
        ABILITY_POPUP(playerLeft, ABILITY_REKINDLE);
        HP_BAR(playerLeft);
    } THEN {
        EXPECT_EQ(playerLeft->hp, 1 + 3 * (playerLeft->maxHP / REKINDLE_HEAL_FRACTION));
    }
}

SINGLE_BATTLE_TEST("Rekindle does not activate when a foe faints to indirect damage")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_REKINDLE); HP(1); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Status1(STATUS1_POISON); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); SEND_OUT(opponent, 1); }
    } SCENE {
        MESSAGE("The opposing Wobbuffet fainted!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rekindle heals Charizard when it knocks out a foe")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_CHARIZARD, ABILITY_REKINDLE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_CHARIZARD) { Ability(ABILITY_BLAZE); HP(1); Moves(MOVE_SCRATCH); } // chosen Blaze; Rekindle only as innate
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } SCENE {
        if (enabled) {
            ABILITY_POPUP(player, ABILITY_REKINDLE); // pop-up shows the innate, not the chosen Blaze
            HP_BAR(player);
        } else {
            NONE_OF { ABILITY_POPUP(player, ABILITY_REKINDLE); }
        }
    } THEN {
        EXPECT_EQ(player->hp, enabled ? 1 + player->maxHP / REKINDLE_HEAL_FRACTION : 1);
    }
}

// The "burning fire" flavor set (fork-docs/INNATE_ABILITIES.md#ability_rekindle).
static const u16 sRekindleCarriers[] =
{
    SPECIES_CHARMANDER, SPECIES_CHARMELEON, SPECIES_CHARIZARD, SPECIES_CHARIZARD_MEGA_X,
    SPECIES_CHARIZARD_MEGA_Y, SPECIES_CHARIZARD_GMAX,
    SPECIES_GROWLITHE, SPECIES_ARCANINE, SPECIES_GROWLITHE_HISUI, SPECIES_ARCANINE_HISUI,
    SPECIES_PONYTA, SPECIES_RAPIDASH,
    SPECIES_MAGBY, SPECIES_MAGMAR, SPECIES_MAGMORTAR,
    SPECIES_MOLTRES, SPECIES_ENTEI, SPECIES_HO_OH, SPECIES_VICTINI,
    SPECIES_CYNDAQUIL, SPECIES_QUILAVA, SPECIES_TYPHLOSION, SPECIES_TYPHLOSION_HISUI,
    SPECIES_TORCHIC, SPECIES_COMBUSKEN, SPECIES_BLAZIKEN, SPECIES_BLAZIKEN_MEGA,
    SPECIES_CHIMCHAR, SPECIES_MONFERNO, SPECIES_INFERNAPE,
    SPECIES_TEPIG, SPECIES_PIGNITE, SPECIES_EMBOAR, SPECIES_EMBOAR_MEGA,
    SPECIES_PANSEAR, SPECIES_SIMISEAR,
    SPECIES_DARUMAKA, SPECIES_DARMANITAN_STANDARD, SPECIES_DARMANITAN_ZEN,
    SPECIES_LITWICK, SPECIES_LAMPENT, SPECIES_CHANDELURE, SPECIES_CHANDELURE_MEGA,
    SPECIES_HEATMOR,
    SPECIES_FLETCHINDER, SPECIES_TALONFLAME,
    SPECIES_LITLEO, SPECIES_PYROAR, SPECIES_PYROAR_MEGA,
    SPECIES_LITTEN, SPECIES_TORRACAT, SPECIES_INCINEROAR,
    SPECIES_SCORBUNNY, SPECIES_RABOOT, SPECIES_CINDERACE, SPECIES_CINDERACE_GMAX,
    SPECIES_SKELEDIRGE,
    SPECIES_CHARCADET, SPECIES_ARMAROUGE, SPECIES_CERULEDGE,
};

TEST("Rekindle is an innate of every species in its flavor set")
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sRekindleCarriers); i++)
        EXPECT(SpeciesHasInnate(sRekindleCarriers[i], ABILITY_REKINDLE));
}

// Sheer Force skips the move-end steps that hold the KO abilities; the innate step is detoured
// back in by RedirectSheerForceSkipToInnates(), so an innate Rekindle still fires.
SINGLE_BATTLE_TEST("FEATURE_INNATE_ABILITIES: innate Rekindle still heals after a Sheer Force boosted KO")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_DARMANITAN_STANDARD, ABILITY_REKINDLE));
        ASSUME(MoveIsAffectedBySheerForce(MOVE_FLAMETHROWER));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_DARMANITAN_STANDARD) { Ability(ABILITY_SHEER_FORCE); HP(1); Moves(MOVE_FLAMETHROWER); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_ZIGZAGOON);
    } WHEN {
        TURN { MOVE(player, MOVE_FLAMETHROWER); SEND_OUT(opponent, 1); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_REKINDLE);
        HP_BAR(player);
    } THEN {
        EXPECT_EQ(player->hp, 1 + player->maxHP / REKINDLE_HEAL_FRACTION);
    }
}
