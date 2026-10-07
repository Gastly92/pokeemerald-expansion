#include "global.h"
#include "test/battle.h"
#include "fork/damage_preview.h"

// B_MOVE_DAMAGE_PREVIEW. The move menu's "% of the foe's HP" range must (1) contain the
// damage the move really does, whatever the foe's hidden spread, (2) never use anything the
// player has not seen -- the foe's held item, its chosen ability, the mon behind an Illusion --
// and (3) leave the battle exactly as it found it, since it runs on every cursor move.
// GetDamagePreviewRange is called directly; the menu just prints its two numbers.

// Tyranitar at Lv100 sits at these four corners of its legal spread (base HP 100 / Def 110):
// the frailest is 0 IV / 0 EV / -Def, the bulkiest 31 IV / 252 EV / +Def.
#define TTAR_FRAIL_HP   310
#define TTAR_FRAIL_DEF  202
#define TTAR_BULKY_HP   404
#define TTAR_BULKY_DEF  350

// Both ends of the range are truncated, so the exact % lies in [lo, hi + 1).
// The KO verdict, where a test does not look at it.
static EWRAM_DATA enum DamagePreviewKO sKo = 0;

static bool32 PercentWithin(s32 damage, u32 maxHP, u32 lo, u32 hi)
{
    return (u32)damage * 100 >= lo * maxHP && (u32)damage * 100 < (hi + 1) * maxHP;
}

SINGLE_BATTLE_TEST("Damage preview: the range contains the damage dealt at either end of the foe's spread")
{
    u32 maxHP, defense;
    s16 damage;
    u32 lo = 0, hi = 0;
    bool32 shown = FALSE;

    PARAMETRIZE { maxHP = TTAR_FRAIL_HP; defense = TTAR_FRAIL_DEF; }
    PARAMETRIZE { maxHP = TTAR_BULKY_HP; defense = TTAR_BULKY_DEF; }

    GIVEN {
        PLAYER(SPECIES_GARCHOMP) { Moves(MOVE_EARTHQUAKE); }
        // Unnerve keeps Sand Stream's chip and Sp. Def boost out of the picture.
        OPPONENT(SPECIES_TYRANITAR) { Ability(ABILITY_UNNERVE); MaxHP(maxHP); HP(maxHP); Defense(defense); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_EARTHQUAKE); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &damage);
    } THEN {
        shown = GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE, GIMMICK_NONE, &lo, &hi, &sKo);
        EXPECT(shown);
        EXPECT(lo < hi);
        EXPECT(PercentWithin(damage, maxHP, lo, hi));
    }
}

SINGLE_BATTLE_TEST("Damage preview: a status move has no range")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(!GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_TOXIC, GIMMICK_NONE, &lo, &hi, &sKo));
    }
}

// Off the stack: four BattlePokemon are ~500 bytes, which a battle test's stack cannot spare.
static EWRAM_DATA struct BattlePokemon sMonsBefore[MAX_BATTLERS_COUNT] = {0};

SINGLE_BATTLE_TEST("Damage preview: an unrevealed held item is left out until the player has seen it")
{
    u32 loHidden, hiHidden, loSeen, hiSeen;

    GIVEN {
        PLAYER(SPECIES_CHARIZARD) { Moves(MOVE_FLAMETHROWER, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_METAGROSS) { Item(ITEM_ASSAULT_VEST); Moves(MOVE_TACKLE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_TACKLE); }
    } THEN {
        EXPECT((gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & 1u) == 0);
        memcpy(sMonsBefore, gBattleMons, sizeof(sMonsBefore));
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_FLAMETHROWER, GIMMICK_NONE, &loHidden, &hiHidden, &sKo));
        // Nothing the calc rewrote may survive it.
        EXPECT(memcmp(sMonsBefore, gBattleMons, sizeof(sMonsBefore)) == 0);
        EXPECT(gBattleMons[B_POSITION_OPPONENT_LEFT].item == ITEM_ASSAULT_VEST);

        gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_FLAMETHROWER, GIMMICK_NONE, &loSeen, &hiSeen, &sKo));
        // Once seen, the Assault Vest's 1.5x Sp. Def shows up in the range.
        EXPECT(hiSeen < hiHidden);
        EXPECT(loSeen < loHidden);
    }
}

SINGLE_BATTLE_TEST("Damage preview: an unrevealed ability is left out until the player has seen it")
{
    u32 loHidden, hiHidden, loSeen, hiSeen;

    GIVEN {
        PLAYER(SPECIES_WEAVILE) { Moves(MOVE_ICE_BEAM, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_DRAGONITE) { Ability(ABILITY_MULTISCALE); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT((gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & 1u) == 0);
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_ICE_BEAM, GIMMICK_NONE, &loHidden, &hiHidden, &sKo));
        EXPECT(gBattleMons[B_POSITION_OPPONENT_LEFT].ability == ABILITY_MULTISCALE);

        gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_ICE_BEAM, GIMMICK_NONE, &loSeen, &hiSeen, &sKo));
        // Multiscale halves a hit at full HP.
        EXPECT(hiSeen < hiHidden);
    }
}

SINGLE_BATTLE_TEST("Damage preview: a foe under Illusion is read as its disguise")
{
    u32 lo, hi;

    GIVEN {
        PLAYER(SPECIES_ALAKAZAM) { Moves(MOVE_PSYCHIC, MOVE_CELEBRATE); }
        // The real Zoroark is Dark, so immune to Psychic; the Machamp it shows is weak to it.
        OPPONENT(SPECIES_ZOROARK) { Ability(ABILITY_ILLUSION); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_MACHAMP) { Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(GetIllusionMonSpecies(B_BATTLER_1) == SPECIES_MACHAMP);
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_PSYCHIC, GIMMICK_NONE, &lo, &hi, &sKo));
        EXPECT(hi > 0);
        EXPECT(gBattleMons[B_POSITION_OPPONENT_LEFT].species == SPECIES_ZOROARK);
    }
}

SINGLE_BATTLE_TEST("Damage preview: an armed Z-Move's range contains the Z-Move's damage")
{
    u32 maxHP, defense;
    s16 damage;
    u32 lo = 0, hi = 0, loBase = 0, hiBase = 0;

    PARAMETRIZE { maxHP = TTAR_FRAIL_HP; defense = TTAR_FRAIL_DEF; }
    PARAMETRIZE { maxHP = TTAR_BULKY_HP; defense = TTAR_BULKY_DEF; }

    GIVEN {
        ASSUME(GetMoveType(MOVE_MUD_SLAP) == TYPE_GROUND);
        PLAYER(SPECIES_GARCHOMP) { Item(ITEM_GROUNDIUM_Z); Moves(MOVE_MUD_SLAP); }
        OPPONENT(SPECIES_TYRANITAR) { Ability(ABILITY_UNNERVE); MaxHP(maxHP); HP(maxHP); Defense(defense); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_MUD_SLAP, gimmick: GIMMICK_Z_MOVE); }
    } SCENE {
        MESSAGE("Garchomp used Tectonic Rage!");
        HP_BAR(opponent, captureDamage: &damage);
    } THEN {
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_MUD_SLAP, GIMMICK_Z_MOVE, &lo, &hi, &sKo));
        EXPECT(PercentWithin(damage, maxHP, lo, hi));
        // Tectonic Rage is 100 BP off Mud-Slap's 20 (Earthquake's would KO the frail end).
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_MUD_SLAP, GIMMICK_NONE, &loBase, &hiBase, &sKo));
        EXPECT(hi > hiBase);
    }
}

SINGLE_BATTLE_TEST("Damage preview: the KO verdict is read against the foe's current HP")
{
    u32 hp, lo, hi;
    enum DamagePreviewKO expected, ko;

    // Tackle cannot KO a Tyranitar from full, at any spread; from 1 HP it always does.
    PARAMETRIZE { hp = 100; expected = DAMAGE_PREVIEW_NO_KO; }
    PARAMETRIZE { hp = 1;   expected = DAMAGE_PREVIEW_KO_ALWAYS; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_TACKLE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_TYRANITAR) { Ability(ABILITY_UNNERVE); MaxHP(TTAR_BULKY_HP); HP(hp == 100 ? TTAR_BULKY_HP : 1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_TACKLE, GIMMICK_NONE, &lo, &hi, &ko));
        EXPECT_EQ(ko, expected);
    }
}

SINGLE_BATTLE_TEST("Damage preview: a hit that KOs only at the frail end of the spread is a possible KO")
{
    u32 lo, hi;
    enum DamagePreviewKO ko;

    GIVEN {
        PLAYER(SPECIES_GARCHOMP) { Moves(MOVE_EARTHQUAKE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_TYRANITAR) { Ability(ABILITY_UNNERVE); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        // Pick the foe's HP inside the range, so the bulky end survives and the frail end does not.
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE, GIMMICK_NONE, &lo, &hi, &ko));
        EXPECT(lo + 2 < hi);
        gBattleMons[B_BATTLER_1].hp = gBattleMons[B_BATTLER_1].maxHP * ((lo + hi) / 2) / 100;
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE, GIMMICK_NONE, &lo, &hi, &ko));
        EXPECT_EQ(ko, DAMAGE_PREVIEW_KO_MAYBE);
    }
}

SINGLE_BATTLE_TEST("Damage preview: a revealed Focus Sash at full HP is never promised as a KO")
{
    u32 lo, hi;
    enum DamagePreviewKO ko;

    GIVEN {
        PLAYER(SPECIES_GARCHOMP) { Moves(MOVE_EARTHQUAKE, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_PIKACHU) { Item(ITEM_FOCUS_SASH); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } THEN {
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE, GIMMICK_NONE, &lo, &hi, &ko));
        EXPECT_EQ(ko, DAMAGE_PREVIEW_KO_ALWAYS); // the Sash is hidden, so the player expects a KO
        gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] |= 1u;
        EXPECT(GetDamagePreviewRange(B_BATTLER_0, B_BATTLER_1, MOVE_EARTHQUAKE, GIMMICK_NONE, &lo, &hi, &ko));
        EXPECT_EQ(ko, DAMAGE_PREVIEW_NO_KO);
    }
}
