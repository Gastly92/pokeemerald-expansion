// B_MOVE_DAMAGE_PREVIEW -- the move menu's "% of the foe's HP" damage range.
// See include/fork/damage_preview.h and fork-docs/DAMAGE_PREVIEW.md.
//
// The calc itself is the AI's damage simulation (AI_CalcDamage), which already handles
// everything a menu readout needs and the real damage calc does not do on its own: an armed
// gimmick (Z-Move / Dynamax / Tera) that is not active yet, multi-hit strike counts, fixed-damage
// moves, Nature Power, Protean. What this file adds is the *hypothetical defender*: before the
// call it rewrites the defender into what the player can actually know about it, and afterwards
// puts every byte back. Nothing here may leave battle state changed -- it runs on every cursor
// move in the move menu.

#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_message.h"
#include "battle_util.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "fork/damage_preview.h"
#include "constants/battle.h"
#include "constants/characters.h"

// The two ends of the defender's unknown spread, mirroring CalculateMonStats() (src/pokemon.c).
// The Speed Tiers page of the INFO viewer bounds Speed the same way.
#define NATURE_HINDERING 90
#define NATURE_BOOSTING  110

static u32 CalcHpBound(enum Species species, u32 level, u32 iv, u32 ev)
{
    u32 base = GetSpeciesBaseHP(species);

    if (base == 1) // Shedinja
        return 1;
    return (((2 * base + iv + ev / 4) * level) / 100) + level + 10;
}

static u32 CalcStatBound(u32 base, u32 level, u32 iv, u32 ev, u32 nature)
{
    u32 n = (((2 * base + iv + ev / 4) * level) / 100) + 5;
    return n * nature / 100;
}

// What the player has seen of a foe. The INFO viewer's reveal bits are the source of truth
// (see fork-docs/BATTLE_INFO.md, "Reveal gating"); they are set for every battle, not only in
// the Frontier. A foe under an active Illusion has revealed nothing of its own: the bits belong
// to the real mon's party slot, and using them would leak what is behind the disguise.
static bool32 IsFoeAbilityKnown(enum BattlerId battler)
{
    if (GetIllusionMonSpecies(battler) != SPECIES_NONE)
        return FALSE;
    return (gBattleStruct->infoAbilityRevealed[GetBattlerSide(battler)] >> gBattlerPartyIndexes[battler]) & 1;
}

static bool32 IsFoeItemKnown(enum BattlerId battler)
{
    if (GetIllusionMonSpecies(battler) != SPECIES_NONE)
        return FALSE;
    return (gBattleStruct->infoItemRevealed[GetBattlerSide(battler)] >> gBattlerPartyIndexes[battler]) & 1;
}

// One end of the spread. `frail` is the top of the range: the least HP and defences the foe can
// have. Its offences are pushed the *other* way at the same time, so a move that reads the
// target's own stats (Foul Play off its Attack, Gyro Ball off its Speed) also lands at its worst
// for the foe in the frail case -- the two cases are the outer bounds, whichever way a move leans.
static void ApplySpreadBound(struct BattlePokemon *mon, const struct BattlePokemon *real, enum Species species, u32 level, u32 hpScaleNum, u32 hpScaleDen, bool32 frail)
{
    u32 defIv = frail ? 0 : MAX_PER_STAT_IVS;
    u32 defEv = frail ? 0 : MAX_PER_STAT_EVS;
    u32 defNature = frail ? NATURE_HINDERING : NATURE_BOOSTING;
    u32 offIv = frail ? MAX_PER_STAT_IVS : 0;
    u32 offEv = frail ? MAX_PER_STAT_EVS : 0;
    u32 offNature = frail ? NATURE_BOOSTING : NATURE_HINDERING;
    u32 maxHP = CalcHpBound(species, level, defIv, defEv) * hpScaleNum / hpScaleDen;

    if (maxHP == 0)
        maxHP = 1;
    mon->maxHP = maxHP;
    // Keep the health bar's fraction: Brine, Wring Out, Hard Press etc. read the target's
    // current HP, and the bar already shows the player that fraction.
    mon->hp = maxHP * real->hp / real->maxHP;
    if (mon->hp == 0)
        mon->hp = 1;

    // A transformed foe has copied its target's stats, which the player is looking at.
    if (real->volatiles.transformed)
        return;

    mon->defense   = CalcStatBound(GetSpeciesBaseDefense(species),   level, defIv, defEv, defNature);
    mon->spDefense = CalcStatBound(GetSpeciesBaseSpDefense(species), level, defIv, defEv, defNature);
    mon->attack    = CalcStatBound(GetSpeciesBaseAttack(species),    level, offIv, offEv, offNature);
    mon->spAttack  = CalcStatBound(GetSpeciesBaseSpAttack(species),  level, offIv, offEv, offNature);
    mon->speed     = CalcStatBound(GetSpeciesBaseSpeed(species),     level, offIv, offEv, offNature);
}

bool32 GetDamagePreviewRange(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Gimmick gimmick, u32 *loPct, u32 *hiPct)
{
    struct BattlePokemon realDef;
    enum Ability savedAbility[MAX_BATTLERS_COUNT];
    enum Item savedItem[MAX_BATTLERS_COUNT];
    enum Ability savedAiAbilities[MAX_BATTLERS_COUNT];
    enum Item savedAiItems[MAX_BATTLERS_COUNT];
    enum HoldEffect savedAiHoldEffects[MAX_BATTLERS_COUNT];
    u32 savedDragonDarts;
    enum Species species;
    u32 level, hpScaleNum, hpScaleDen;
    u32 lo = UINT32_MAX, hi = 0;
    bool32 any = FALSE;

    if (move == MOVE_NONE || IsBattleMoveStatus(move) || !IsBattlerAlive(battlerDef) || gBattleMons[battlerDef].maxHP == 0)
        return FALSE;

    realDef = gBattleMons[battlerDef];
    species = realDef.species;
    level = realDef.level;
    if (GetIllusionMonSpecies(battlerDef) != SPECIES_NONE)
    {
        // The disguise is what the player is fighting, as far as they know.
        species = GetIllusionMonSpecies(battlerDef);
        level = GetMonData(GetIllusionMonPtr(battlerDef), MON_DATA_LEVEL, NULL);
    }
    // A Dynamaxed foe's bar is its multiplied HP, so the % is read against that.
    hpScaleNum = realDef.maxHP;
    hpScaleDen = (GetActiveGimmick(battlerDef) == GIMMICK_DYNAMAX) ? GetNonDynamaxMaxHP(battlerDef) : realDef.maxHP;
    if (hpScaleDen == 0)
        hpScaleDen = hpScaleNum = 1;

    savedDragonDarts = gAiLogicData->dragonDartsHitsBothTarget;
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        savedAbility[b] = gBattleMons[b].ability;
        savedItem[b] = gBattleMons[b].item;
        savedAiAbilities[b] = gAiLogicData->abilities[b];
        savedAiItems[b] = gAiLogicData->items[b];
        savedAiHoldEffects[b] = gAiLogicData->holdEffects[b];
    }

    // The AI's calc reads abilities and items from its own knowledge model, which under
    // AI_FLAG_OMNISCIENT is the truth. Feed it the player's knowledge instead: your own side in
    // full, the foe's side only as revealed. The raw fields are blanked too, for the parts of the
    // calc that read the battler directly rather than the context.
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        bool32 abilityKnown = IsOnPlayerSide(b) || IsFoeAbilityKnown(b);
        bool32 itemKnown = IsOnPlayerSide(b) || IsFoeItemKnown(b);

        gAiLogicData->abilities[b] = abilityKnown ? GetBattlerAbility(b) : ABILITY_NONE;
        gAiLogicData->items[b] = itemKnown ? gBattleMons[b].item : ITEM_NONE;
        gAiLogicData->holdEffects[b] = itemKnown ? GetBattlerHoldEffect(b) : HOLD_EFFECT_NONE;
    }
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        if (gAiLogicData->abilities[b] == ABILITY_NONE)
            gBattleMons[b].ability = ABILITY_NONE;
        if (gAiLogicData->items[b] == ITEM_NONE)
            gBattleMons[b].item = ITEM_NONE;
    }

    // Under Illusion the calc must see the disguise: its typing and its species, which is also
    // what FEATURE_INNATE_ABILITIES reads innates from. A Terastallized mon's type is its Tera
    // type either way, which GetBattlerType resolves before it reads these.
    if (species != realDef.species)
    {
        gBattleMons[battlerDef].species = species;
        gBattleMons[battlerDef].types[0] = GetSpeciesType(species, 0);
        gBattleMons[battlerDef].types[1] = GetSpeciesType(species, 1);
        gBattleMons[battlerDef].types[2] = TYPE_MYSTERY;
    }

    for (u32 frail = 0; frail < 2; frail++)
    {
        struct AiCalcValues aiCalc = {
            .move = move,
            .gimmickAtk = gimmick,
            .gimmickDef = GIMMICK_NONE,
            .weather = AI_GetWeather(),
            .terrain = gFieldTimers.terrain,
        };
        struct SimulatedDamage dmg;
        u32 maxHP;

        ApplySpreadBound(&gBattleMons[battlerDef], &realDef, species, level, hpScaleNum, hpScaleDen, frail);
        maxHP = gBattleMons[battlerDef].maxHP;
        dmg = AI_CalcDamage(&aiCalc, battlerAtk, battlerDef);
        if (dmg.maximum == 0)
            continue;

        any = TRUE;
        // Both ends round DOWN (42.2-53.6% reads 42-53%). Rounding the top up would turn a 99.2%
        // max into "100%" and promise a KO from full HP that cannot happen; truncating means the
        // readout only says 100 when the hit really can take the whole bar.
        if (dmg.minimum * 100 / maxHP < lo)
            lo = dmg.minimum * 100 / maxHP;
        if (dmg.maximum * 100 / maxHP > hi)
            hi = dmg.maximum * 100 / maxHP;
    }

    gBattleMons[battlerDef] = realDef;
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        gBattleMons[b].ability = savedAbility[b];
        gBattleMons[b].item = savedItem[b];
        gAiLogicData->abilities[b] = savedAiAbilities[b];
        gAiLogicData->items[b] = savedAiItems[b];
        gAiLogicData->holdEffects[b] = savedAiHoldEffects[b];
    }
    gAiLogicData->dragonDartsHitsBothTarget = savedDragonDarts;

    if (!any)
        return FALSE;
    *loPct = lo;
    *hiPct = hi;
    return TRUE;
}

// The foe the readout is about: the one opposite, or its partner once that one is down. The
// same choice the stock effectiveness icon makes for its single readout.
static enum BattlerId GetPreviewTarget(enum BattlerId battler)
{
    enum BattlerId target = GetOppositeBattler(battler);

    if (!IsBattlerAlive(target) && IsDoubleBattle() && IsBattlerAlive(GetPartnerBattler(target)))
        target = GetPartnerBattler(target);
    return target;
}

bool32 TryPrintMoveDamagePreview(enum BattlerId battler, enum Move move, enum Type type)
{
    enum BattlerId target = GetPreviewTarget(battler);
    enum Gimmick gimmick = gBattleStruct->gimmick.usableGimmick[battler];
    u32 lo, hi;
    u8 *end;

    if (!B_MOVE_DAMAGE_PREVIEW)
        return FALSE;
    if (!IsGimmickSelected(battler, gimmick))
        gimmick = GIMMICK_NONE;
    if (!GetDamagePreviewRange(battler, target, move, gimmick, &lo, &hi))
        return FALSE;

    // The type row is 64px. "<Type> lo-hi%" fits it in FONT_NARROWER for every type name as
    // long as the numbers stay under three digits, so anything past a KO is shown as 100:
    // "85-100%" reads as "can KO", and a range that always KOs collapses to "KO".
    if (hi > 100)
        hi = 100;
    end = StringCopy(gDisplayedStringBattle, gTypesInfo[type].name);
    *end++ = CHAR_SPACE;
    if (lo >= 100)
    {
        end = StringCopy(end, COMPOUND_STRING("KO"));
    }
    else
    {
        end = ConvertIntToDecimalStringN(end, lo, STR_CONV_MODE_LEFT_ALIGN, 3);
        if (hi != lo)
        {
            *end++ = CHAR_HYPHEN;
            end = ConvertIntToDecimalStringN(end, hi, STR_CONV_MODE_LEFT_ALIGN, 3);
        }
        *end++ = CHAR_PERCENT;
        *end = EOS;
    }
    PrependFontIdToFit(gDisplayedStringBattle, end, FONT_NARROW, WindowWidthPx(B_WIN_MOVE_TYPE));
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_TYPE);
    return TRUE;
}
