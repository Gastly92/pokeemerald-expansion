// B_MOVE_DAMAGE_PREVIEW -- the move menu's "% of the foe's HP" damage range.
// See include/fork/damage_preview.h and fork-docs/DAMAGE_PREVIEW.md.
//
// The calc itself is the AI's damage simulation (AI_CalcDamage), which already handles
// everything a menu readout needs and the real damage calc does not do on its own: an armed
// gimmick (Z-Move / Dynamax / Tera) that is not active yet, multi-hit strike counts, fixed-damage
// moves, Nature Power, Protean. What this file adds is the *hypothetical defender*: before the
// call it rewrites the defender into what the player can actually know about it, and afterwards
// puts every byte back. It does the same to the attacker when a Mega Evolution / Ultra Burst is
// armed, since the AI calc cannot change forms (BeginArmedFormPreview). Nothing here may leave
// battle state changed -- it runs on every cursor move in the move menu.

#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_controllers.h"
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_main.h"
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

// Why a damaging move came out at 0: a matchup the player knows is immune (a type immunity, a
// revealed Levitate / Bulletproof / Volt Absorb, Dazzling vs priority) reads "0%", while a move that
// merely fails here (Dream Eater on a waking foe, Poltergeist against an item the player has not
// seen) keeps the stock line -- the second kind can hinge on what the preview hid. Asked while the
// defender is still rewritten into what the player knows.
static bool32 IsKnownImmunity(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, uq4_12_t typeEffectiveness)
{
    struct DamageContext ctx = {0};

    if (typeEffectiveness == UQ_4_12(0.0))
        return TRUE;
    if (Ai_IsPriorityBlocked(battlerAtk, battlerDef, move, gAiLogicData))
        return TRUE;

    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = ctx.chosenMove = ctx.baseMove = move;
    ctx.moveType = GetBattleMoveType(move);
    ctx.weather = AI_GetWeather();
    ctx.terrain = gFieldTimers.terrain;
    ctx.abilities[battlerAtk] = gAiLogicData->abilities[battlerAtk];
    ctx.abilities[battlerDef] = AI_GetMoldBreakerSanitizedAbility(battlerAtk, gAiLogicData->abilities[battlerAtk],
        gAiLogicData->abilities[battlerDef], gAiLogicData->holdEffects[battlerDef], move);
    ctx.holdEffects[battlerAtk] = gAiLogicData->holdEffects[battlerAtk];
    ctx.holdEffects[battlerDef] = gAiLogicData->holdEffects[battlerDef];
    return AI_CanMoveBeBlockedByTarget(&ctx);
}

// The form an armed Mega Evolution / Ultra Burst turns the battler into, resolved the way
// ActivateMegaEvolution / ActivateUltraBurst (src/battle_util.c) resolve it; SPECIES_NONE when
// the gimmick changes no form.
static enum Species GetArmedFormSpecies(enum BattlerId battler, enum Gimmick gimmick)
{
    enum Ability ability = GetBattlerAbility(battler);
    enum Species species = gBattleMons[battler].species;
    enum Species target;

    if (GetActiveGimmick(battler) != GIMMICK_NONE)
        return SPECIES_NONE;
    switch (gimmick)
    {
    case GIMMICK_MEGA:
        target = GetBattleFormChangeTargetSpecies(battler, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_MOVE, ability);
        if (target == species)
            target = GetBattleFormChangeTargetSpecies(battler, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM, ability);
        break;
    case GIMMICK_ULTRA_BURST:
        target = GetBattleFormChangeTargetSpecies(battler, FORM_CHANGE_BATTLE_ULTRA_BURST, ability);
        break;
    default:
        return SPECIES_NONE;
    }
    return (target == species) ? SPECIES_NONE : target;
}

enum Gimmick GetArmedGimmick(enum BattlerId battler)
{
    enum Gimmick gimmick = gBattleStruct->gimmick.usableGimmick[battler];

    return IsGimmickSelected(battler, gimmick) ? gimmick : GIMMICK_NONE;
}

// The projection's scratch lives in EWRAM, not on the stack: it is taken under the AI damage
// calc, whose call depth leaves no room for two more battler-sized structs.
static EWRAM_DATA struct BattlePokemon sArmedFormSaved = {0};
static EWRAM_DATA struct Pokemon sArmedFormMon = {0};

bool32 BeginArmedFormPreview(enum BattlerId battler, enum Gimmick gimmick)
{
    enum Species target = GetArmedFormSpecies(battler, gimmick);
    struct Pokemon *mon = &sArmedFormMon;
    bool32 keepSpeed;

    if (target == SPECIES_NONE)
        return FALSE;

    // What TryBattleFormChange + RecalcBattlerStats do, on a copy of the party mon so nothing
    // outside gBattleMons[battler] is touched: the form's stats from the mon's own spread, and
    // its ability and types. Stat stages and everything else carry over as they do for real.
    sArmedFormSaved = gBattleMons[battler];
    *mon = *GetBattlerMon(battler);
    SetMonData(mon, MON_DATA_SPECIES, &target);
    keepSpeed = gBattleMons[battler].volatiles.speedSwapped && GetConfig(B_MEGA_EVO_SPEED_SWAP) >= GEN_CHAMPIONS;
    if (keepSpeed)
        CalculateMonStatsCont(mon, FALSE);
    else
        CalculateMonStats(mon);
    gBattleMons[battler].species = target;
    CopyMonLevelAndBaseStatsToBattleMon(battler, mon, !keepSpeed);
    CopyMonAbilityAndTypesToBattleMon(battler, mon);
    return TRUE;
}

void EndArmedFormPreview(enum BattlerId battler)
{
    gBattleMons[battler] = sArmedFormSaved;
}

enum Type GetDamagePreviewMoveType(enum BattlerId battler, enum Move move, enum Gimmick gimmick)
{
    bool32 armedForm = BeginArmedFormPreview(battler, gimmick);
    bool32 toggledGimmick = FALSE;
    enum Type type;

    // Set up the way AI_CalcDamage sets it up, so the name agrees with the range beside it:
    // the armed gimmick switched on, then the engine's own pre-move type resolution. Everything
    // SetTypeBeforeUsingMove latches is cleared again, as AI_CalcDamage clears it.
    if (gimmick != GIMMICK_NONE && GetActiveGimmick(battler) == GIMMICK_NONE)
    {
        toggledGimmick = TRUE;
        SetActiveGimmick(battler, gimmick);
    }
    gBattleStruct->dynamicMoveType = TYPE_NONE;
    SetTypeBeforeUsingMove(move, battler, GetBattlerAbility(battler), GetBattlerHoldEffect(battler));
    type = GetBattleMoveType(move);
    gBattleStruct->dynamicMoveType = TYPE_NONE;
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_NONE;
    gBattleStruct->battlerState[battler].ateBoost = FALSE;
    gSpecialStatuses[battler].gemBoost = FALSE;
    if (toggledGimmick)
        SetActiveGimmick(battler, GIMMICK_NONE);
    if (armedForm)
        EndArmedFormPreview(battler);
    return type;
}

bool32 GetDamagePreviewRange(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Gimmick gimmick, u32 *loPct, u32 *hiPct, enum DamagePreviewKO *ko)
{
    struct BattlePokemon realDef;
    enum Ability savedAbility[MAX_BATTLERS_COUNT];
    enum Item savedItem[MAX_BATTLERS_COUNT];
    enum Ability savedAiAbilities[MAX_BATTLERS_COUNT];
    enum Item savedAiItems[MAX_BATTLERS_COUNT];
    enum HoldEffect savedAiHoldEffects[MAX_BATTLERS_COUNT];
    u32 savedDragonDarts;
    u32 savedHpPercent;
    enum Species species;
    u32 level, hpScaleNum, hpScaleDen;
    u32 lo = UINT32_MAX, hi = 0;
    bool32 any = FALSE, immune = FALSE, koAlways = TRUE, koMaybe = FALSE, endures, armedForm;
    uq4_12_t typeEffectiveness = UQ_4_12(1.0);

    if (move == MOVE_NONE || IsBattleMoveStatus(move) || !IsBattlerAlive(battlerDef) || gBattleMons[battlerDef].maxHP == 0)
        return FALSE;

    // An armed Mega Evolution / Ultra Burst happens before the move, so the attacker is read as
    // the form it becomes: its stats, types and ability (Huge Power, Pixilate, Tough Claws...).
    armedForm = BeginArmedFormPreview(battlerAtk, gimmick);
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
    savedHpPercent = gAiLogicData->hpPercents[battlerDef];
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

    // A known Sturdy / Focus Sash / Disguise at full HP survives any single hit, so nothing is
    // promised as a KO. Asked here, while the AI's knowledge model holds only what the player
    // has seen; its cached HP% may be stale in the menu, so it is set from the bar first.
    gAiLogicData->hpPercents[battlerDef] = realDef.hp * 100 / realDef.maxHP;
    if (gAiLogicData->hpPercents[battlerDef] == 0)
        gAiLogicData->hpPercents[battlerDef] = 1;
    endures = CanEndureHit(battlerAtk, battlerDef, move);

    for (u32 frail = 0; frail < 2; frail++)
    {
        struct AiCalcValues aiCalc = {
            .move = move,
            .gimmickAtk = gimmick,
            .gimmickDef = GIMMICK_NONE,
            // A projected form's Drought / Sand Stream / Snow Warning sets its weather on the
            // way in, before the move.
            .weather = armedForm ? AI_GetSwitchinWeather(battlerAtk) : AI_GetWeather(),
            .terrain = gFieldTimers.terrain,
        };
        struct SimulatedDamage dmg;
        u32 maxHP;

        ApplySpreadBound(&gBattleMons[battlerDef], &realDef, species, level, hpScaleNum, hpScaleDen, frail);
        maxHP = gBattleMons[battlerDef].maxHP;
        dmg = AI_CalcDamage(&aiCalc, battlerAtk, battlerDef);
        typeEffectiveness = aiCalc.typeEffectiveness;
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
        // KO is judged against the foe's *current* HP (the bar's fraction at this spread), so a
        // 40-50% move on a foe in the red still reads as a KO.
        if (dmg.minimum < gBattleMons[battlerDef].hp)
            koAlways = FALSE;
        if (dmg.maximum >= gBattleMons[battlerDef].hp)
            koMaybe = TRUE;
    }
    if (!any)
        immune = IsKnownImmunity(battlerAtk, battlerDef, move, typeEffectiveness);

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
    gAiLogicData->hpPercents[battlerDef] = savedHpPercent;
    if (armedForm)
        EndArmedFormPreview(battlerAtk);

    if (!any)
    {
        if (!immune)
            return FALSE;
        *loPct = *hiPct = 0;
        *ko = DAMAGE_PREVIEW_NO_KO;
        return TRUE;
    }
    *loPct = lo;
    *hiPct = hi;
    if (endures)
        *ko = DAMAGE_PREVIEW_NO_KO;
    else if (koAlways)
        *ko = DAMAGE_PREVIEW_KO_ALWAYS;
    else if (koMaybe)
        *ko = DAMAGE_PREVIEW_KO_MAYBE;
    else
        *ko = DAMAGE_PREVIEW_NO_KO;
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

u8 *FormatDamagePreviewAmount(u8 *dst, u32 lo, u32 hi, enum DamagePreviewKO ko)
{
    // The KO verdict is spelled out in plain text: "KO" when every spread and roll KOs from the
    // foe's current HP, "lo%-KO" when some do. It never puts a number beside the INFO viewer's HP % that disagrees with the verdict
    // (a 46.8% low end on a 46.6% foe reads "KO", not "46%" against "HP 47%").
    if (ko == DAMAGE_PREVIEW_KO_ALWAYS)
        return StringCopy(dst, COMPOUND_STRING("KO"));

    // The type row is 64px; "<Type> lo-hi%" fits it in FONT_NARROWER for every type name while
    // the numbers stay under three digits, so anything past the full bar is shown as 100. That
    // only arises without a KO verdict, i.e. behind a known Sturdy / Focus Sash.
    if (hi > 100)
        hi = 100;
    if (lo > 100)
        lo = 100;
    dst = ConvertIntToDecimalStringN(dst, lo, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (ko == DAMAGE_PREVIEW_KO_MAYBE)
    {
        *dst++ = CHAR_PERCENT;
        *dst++ = CHAR_HYPHEN;
        return StringCopy(dst, COMPOUND_STRING("KO"));
    }
    if (hi != lo)
    {
        *dst++ = CHAR_HYPHEN;
        dst = ConvertIntToDecimalStringN(dst, hi, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    *dst++ = CHAR_PERCENT;
    *dst = EOS;
    return dst;
}

static bool32 PrintDamagePreview(enum BattlerId battler, enum Move move, enum Gimmick gimmick, enum Type type)
{
    enum BattlerId target = GetPreviewTarget(battler);
    enum DamagePreviewKO ko;
    u32 lo, hi;
    u8 *end;

    if (!B_MOVE_DAMAGE_PREVIEW)
        return FALSE;
    if (!GetDamagePreviewRange(battler, target, move, gimmick, &lo, &hi, &ko))
        return FALSE;
    // A Z-Move's row names the Z-Move's own type, which its caller passes in.
    if (gimmick != GIMMICK_Z_MOVE)
        type = GetDamagePreviewMoveType(battler, move, gimmick);

    end = StringCopy(gDisplayedStringBattle, gTypesInfo[type].name);
    *end++ = CHAR_SPACE;
    end = FormatDamagePreviewAmount(end, lo, hi, ko);
    PrependFontIdToFit(gDisplayedStringBattle, end, FONT_NARROW, WindowWidthPx(B_WIN_MOVE_TYPE));
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_TYPE);
    return TRUE;
}

bool32 TryPrintMoveDamagePreview(enum BattlerId battler, enum Move move, enum Type type)
{
    return PrintDamagePreview(battler, move, GetArmedGimmick(battler), type);
}

bool32 TryPrintZMoveDamagePreview(enum BattlerId battler, enum Move zMove, enum Type zMoveType)
{
    // The base move under the cursor, read the way MoveSelectionDisplayZMove reads it, so the
    // upstream hook point needs no new parameter.
    struct ChooseMoveStruct *moveInfo = (struct ChooseMoveStruct *)(&gBattleResources->bufferA[battler][4]);
    enum Move baseMove = moveInfo->moves[gMoveSelectionCursor[battler]];

    // The Z view is only up while the Z-Move is the armed choice. A status Z-Move (Extreme
    // Evoboost, off a damaging Last Resort) has no damage to show.
    if (IsBattleMoveStatus(zMove))
        return FALSE;
    return PrintDamagePreview(battler, baseMove, GIMMICK_Z_MOVE, zMoveType);
}
