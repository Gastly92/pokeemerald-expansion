#include "global.h"
#include "test/battle.h"
#include "fork/damage_preview.h"
#include "fork/innate_abilities.h"
#include "fork/status_preview.h"
#include "string_util.h"
#include "constants/characters.h"

// B_MOVE_STATUS_PREVIEW. The move menu says "<Type> Fails" (or "<Type> Bounces", for a visible
// Magic Bounce) for a status move the player can already tell will fail on the foe -- and never for one that only fails because of something
// the player has not seen (a hidden held item or chosen ability). IsStatusMoveKnownToFail is
// called directly; the menu just prints its answer.

SINGLE_BATTLE_TEST("Status preview: Spore into an innate Insomnia is known to fail")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_HYPNO, ABILITY_INSOMNIA));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_HYPNO) { Ability(ABILITY_INNER_FOCUS); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        // The chosen Inner Focus stays unseen; the innate alone is enough.
        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] &= ~1u;
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
    }
}

SINGLE_BATTLE_TEST("Status preview: Spore into a chosen Insomnia shows only once the ability is revealed")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_DROWZEE) { Ability(ABILITY_INSOMNIA); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
        // Insomnia says nothing about poison.
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC));
    }
}

SINGLE_BATTLE_TEST("Status preview: a revealed Insomnia does not stop a Mold Breaker's Spore")
{
    GIVEN {
        PLAYER(SPECIES_PINSIR) { Ability(ABILITY_MOLD_BREAKER); Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_DROWZEE) { Ability(ABILITY_INSOMNIA); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
    }
}

SINGLE_BATTLE_TEST("Status preview: Toxic into a Poison type fails unless the user has Corrosion")
{
    enum Ability ability;
    bool32 fails;

    PARAMETRIZE { ability = ABILITY_OBLIVIOUS; fails = TRUE; }
    PARAMETRIZE { ability = ABILITY_CORROSION; fails = FALSE; }

    GIVEN {
        PLAYER(SPECIES_SALANDIT) { Ability(ability); Moves(MOVE_TOXIC, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_GRIMER) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC), fails);
    }
}

SINGLE_BATTLE_TEST("Status preview: Thunder Wave into a Ground type is known to fail, Glare is not")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_THUNDER_WAVE, MOVE_GLARE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_SANDSHREW) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_THUNDER_WAVE));
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_GLARE));
    }
}

SINGLE_BATTLE_TEST("Status preview: a powder move into a Grass type is known to fail")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SLEEP_POWDER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ODDISH) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SLEEP_POWDER));
    }
}

SINGLE_BATTLE_TEST("Status preview: Safety Goggles stay hidden until they block a powder move, then show")
{
    // The unseen half is the next test; here the goggles have just blocked Sleep Powder.
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SLEEP_POWDER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_SAFETY_GOGGLES); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SLEEP_POWDER); }
    } SCENE {
        MESSAGE("Wobbuffet used Sleep Powder!");
        ITEM_POPUP(opponent, ITEM_SAFETY_GOGGLES);
        MESSAGE("The opposing Wobbuffet is not affected thanks to its Safety Goggles!");
        NOT STATUS_ICON(opponent, sleep: TRUE);
    } THEN {
        EXPECT(gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u);
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SLEEP_POWDER));
    }
}

SINGLE_BATTLE_TEST("Status preview: unseen Safety Goggles do not flag a powder move")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SLEEP_POWDER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_SAFETY_GOGGLES); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT((gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u) == 0);
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SLEEP_POWDER));
        // The item is back where it was.
        EXPECT_EQ(gBattleMons[B_BATTLER_1].item, ITEM_SAFETY_GOGGLES);
    }
}

SINGLE_BATTLE_TEST("Status preview: a foe already asleep or behind a Substitute is known to fail")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_SUBSTITUTE, MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SUBSTITUTE); }
    } THEN {
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
        gBattleMons[B_BATTLER_1].volatiles.substitute = FALSE;
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
        gBattleMons[B_BATTLER_1].status1 = STATUS1_SLEEP_TURN(2);
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE));
    }
}

SINGLE_BATTLE_TEST("Status preview: Leech Seed into a Grass type and Confuse Ray into Own Tempo are known to fail")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_CONFUSE_RAY) == EFFECT_CONFUSE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_LEECH_SEED, MOVE_CONFUSE_RAY, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_ODDISH) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_LEECH_SEED));
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_CONFUSE_RAY));
        gBattleMons[B_BATTLER_1].volatiles.confusionTimer = 2;
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_CONFUSE_RAY));
    }
}

SINGLE_BATTLE_TEST("Status preview: a status move with nothing in its way, or aimed at the user, is never flagged")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TOXIC, MOVE_SWORDS_DANCE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC));
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_SWORDS_DANCE));
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE));
    }
}

SINGLE_BATTLE_TEST("Status preview: a revealed Sap Sipper or Magic Bounce is known to block")
{
    enum Species species;
    enum Ability ability;
    enum Move move;

    PARAMETRIZE { species = SPECIES_GOODRA;  ability = ABILITY_SAP_SIPPER;  move = MOVE_SPORE; }
    PARAMETRIZE { species = SPECIES_ESPEON;  ability = ABILITY_MAGIC_BOUNCE; move = MOVE_TOXIC; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(move, MOVE_CELEBRATE); }
        OPPONENT(species) { Ability(ability); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, move));
        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, move));
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, move),
                  ability == ABILITY_MAGIC_BOUNCE ? STATUS_PREVIEW_BOUNCES : STATUS_PREVIEW_FAILS);
        EXPECT_EQ(gBattleMons[B_BATTLER_1].ability, ability);
    }
}

SINGLE_BATTLE_TEST("Status preview: hazards and status moves into an innate Magic Bounce read as bouncing")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_UMBREON, ABILITY_MAGIC_BOUNCE));
        ASSUME(GetMoveTarget(MOVE_STEALTH_ROCK) == TARGET_OPPONENTS_FIELD);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_STEALTH_ROCK, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_UMBREON) { Ability(ABILITY_INNER_FOCUS); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] &= ~1u;
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_STEALTH_ROCK), STATUS_PREVIEW_BOUNCES);
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SPIKES), STATUS_PREVIEW_BOUNCES);
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_STICKY_WEB), STATUS_PREVIEW_BOUNCES);
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC_SPIKES), STATUS_PREVIEW_BOUNCES);
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC), STATUS_PREVIEW_BOUNCES);
        EXPECT(IsStatusMoveKnownToFail(B_BATTLER_0, B_BATTLER_1, MOVE_STEALTH_ROCK));
        // Moves Magic Bounce cannot reflect are left alone.
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SWORDS_DANCE), STATUS_PREVIEW_NONE);
    }
}

SINGLE_BATTLE_TEST("Status preview: hazards into a foe without Magic Bounce, or past a Mold Breaker, are not flagged")
{
    enum Species species;
    enum Ability abilityAtk;

    PARAMETRIZE { species = SPECIES_WOBBUFFET; abilityAtk = ABILITY_SAND_RUSH; }
    PARAMETRIZE { species = SPECIES_UMBREON;   abilityAtk = ABILITY_MOLD_BREAKER; }

    GIVEN {
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_EXCADRILL) { Ability(abilityAtk); Moves(MOVE_STEALTH_ROCK, MOVE_CELEBRATE); }
        OPPONENT(species) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_STEALTH_ROCK), STATUS_PREVIEW_NONE);
    }
}

DOUBLE_BATTLE_TEST("Status preview: a hazard reads as bouncing when either foe has Magic Bounce")
{
    GIVEN {
        ASSUME(SpeciesHasInnate(SPECIES_UMBREON, ABILITY_MAGIC_BOUNCE));
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_STEALTH_ROCK, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_UMBREON) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_CELEBRATE); MOVE(playerRight, MOVE_CELEBRATE); }
    } THEN {
        // The readout's target is the Wobbuffet; its partner's Magic Bounce still sends the rocks back.
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_STEALTH_ROCK), STATUS_PREVIEW_BOUNCES);
        // A targeted move only bounces off its own target.
        EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC), STATUS_PREVIEW_NONE);
    }
}

// The menu text itself, minus the font code PrependFontIdToFit may put in front.
static bool32 MoveTypeRowReads(const u8 *expected)
{
    const u8 *s = gDisplayedStringBattle;
    if (s[0] == EXT_CTRL_CODE_BEGIN && s[1] == EXT_CTRL_CODE_FONT)
        s += 3;
    return StringCompare(s, expected) == 0;
}

SINGLE_BATTLE_TEST("Status preview: Spore into an innate Overcoat reads 'Grass Fails', and the Spore really fails")
{
    bool32 enabled;
    PARAMETRIZE { enabled = TRUE; }
    PARAMETRIZE { enabled = FALSE; }
    GIVEN {
        ASSUME(IsPowderMove(MOVE_SPORE));
        ASSUME(GetMoveType(MOVE_SPORE) == TYPE_GRASS);
        ASSUME(SpeciesHasInnate(SPECIES_KOMMO_O, ABILITY_OVERCOAT));
        ASSUME(GetSpeciesType(SPECIES_KOMMO_O, 0) != TYPE_GRASS && GetSpeciesType(SPECIES_KOMMO_O, 1) != TYPE_GRASS);
        WITH_CONFIG(FEATURE_INNATE_ABILITIES, enabled);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        // The chosen ability is not Overcoat, so only the innate can block.
        OPPONENT(SPECIES_KOMMO_O) { Ability(ABILITY_BULLETPROOF); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); }
    } SCENE {
        MESSAGE("Wobbuffet used Spore!");
        if (enabled)
            NONE_OF { STATUS_ICON(opponent, sleep: TRUE); }
        else
            STATUS_ICON(opponent, sleep: TRUE);
    } THEN {
        if (enabled)
        {
            EXPECT_EQ(opponent->status1 & STATUS1_SLEEP, 0);
            // The chosen Bulletproof stays unseen; the innate alone is enough.
            gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] &= ~1u;
            EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE), STATUS_PREVIEW_FAILS);
            EXPECT(TryPrintStatusMovePreview(B_BATTLER_0, MOVE_SPORE, TYPE_GRASS));
            EXPECT(MoveTypeRowReads(COMPOUND_STRING("Grass Fails")));
        }
        else
        {
            // Asleep now, so clear it: the question is only whether Overcoat would be flagged.
            opponent->status1 = 0;
            EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE), STATUS_PREVIEW_NONE);
        }
    }
}

SINGLE_BATTLE_TEST("Status preview: Sap Sipper revealed by absorbing a Spore reads 'Grass Fails' on the next turn")
{
    enum Move firstMove;
    PARAMETRIZE { firstMove = MOVE_CELEBRATE; } // nothing reveals it
    PARAMETRIZE { firstMove = MOVE_SPORE; }     // Sap Sipper triggers and shows its pop-up
    GIVEN {
        ASSUME(GetMoveType(MOVE_SPORE) == TYPE_GRASS);
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SPORE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_GOODRA) { Ability(ABILITY_SAP_SIPPER); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, firstMove); }
    } SCENE {
        if (firstMove == MOVE_SPORE)
        {
            ABILITY_POPUP(opponent, ABILITY_SAP_SIPPER);
            NONE_OF { STATUS_ICON(opponent, sleep: TRUE); }
        }
    } THEN {
        EXPECT_EQ(opponent->status1 & STATUS1_SLEEP, 0);
        if (firstMove == MOVE_SPORE)
        {
            EXPECT(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u);
            EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE), STATUS_PREVIEW_FAILS);
            EXPECT(TryPrintStatusMovePreview(B_BATTLER_0, MOVE_SPORE, TYPE_GRASS));
            EXPECT(MoveTypeRowReads(COMPOUND_STRING("Grass Fails")));
        }
        else
        {
            EXPECT_EQ(gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u, 0);
            EXPECT_EQ(GetStatusMovePreviewResult(B_BATTLER_0, B_BATTLER_1, MOVE_SPORE), STATUS_PREVIEW_NONE);
            EXPECT(!TryPrintStatusMovePreview(B_BATTLER_0, MOVE_SPORE, TYPE_GRASS));
        }
    }
}
