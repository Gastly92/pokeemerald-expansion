// FORK: read-only "battle info" viewer shown in Frontier facilities in place of
// the (disabled) BAG action. See include/config/frontier.h (B_FRONTIER_BATTLE_INFO)
// and include/frontier_battle_info.h for the integration notes. This whole file
// is fork-only, so it never conflicts on an upstream sync; it only *reads* battle
// state, never mutates it.

#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_util.h"
#include "bg.h"
#include "config_changes.h" // FORK: GetConfig(FEATURE_INNATE_ABILITIES)
#include "innate_abilities.h" // FORK: FEATURE_INNATE_ABILITIES
#include "frontier_battle_info.h"
#include "gpu_regs.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "move.h"
#include "palette.h"
#include "pokemon.h"
#include "reshow_battle_screen.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/rgb.h"
#include "constants/songs.h"

bool8 gFrontierBattleInfoActive = FALSE;

#define INFO_WIN_WIDTH   28
#define INFO_WIN_HEIGHT  18

enum
{
    INFO_PAGE_FIELD,
    INFO_PAGE_CONDITIONS,
    INFO_PAGE_STATS,
    INFO_PAGE_FOE,
    INFO_PAGE_COUNT,
};

// Task data layout.
#define tWindowId  data[0]
#define tPage      data[1]
#define tFoeIndex  data[2]

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sInfoWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = INFO_WIN_WIDTH,
    .height = INFO_WIN_HEIGHT,
    .paletteNum = 0xF,
    .baseBlock = 0x1,
};

static const u16 sBgColor[] = {RGB_WHITE};

// ---------------------------------------------------------------------------
// State readers (read-only; mirror what the player is allowed to know).
// ---------------------------------------------------------------------------

bool32 ShouldReplaceBagWithInfo(void)
{
#if B_FRONTIER_BATTLE_INFO
    // The bag is disabled in every Frontier facility except the Pyramid (which
    // keeps a working bag), so only those facilities swap BAG -> INFO.
    return (gBattleTypeFlags & BATTLE_TYPE_FRONTIER_NO_PYRAMID) != 0;
#else
    return FALSE;
#endif
}

static const u8 *GetInfoWeatherName(void)
{
    if (gBattleWeather & B_WEATHER_RAIN)
        return COMPOUND_STRING("Rain");
    if (gBattleWeather & B_WEATHER_SUN)
        return COMPOUND_STRING("Harsh Sunlight");
    if (gBattleWeather & B_WEATHER_SANDSTORM)
        return COMPOUND_STRING("Sandstorm");
    if (gBattleWeather & B_WEATHER_HAIL)
        return COMPOUND_STRING("Hail");
    if (gBattleWeather & B_WEATHER_SNOW)
        return COMPOUND_STRING("Snow");
    if (gBattleWeather & B_WEATHER_FOG)
        return COMPOUND_STRING("Fog");
    if (gBattleWeather & B_WEATHER_STRONG_WINDS)
        return COMPOUND_STRING("Strong Winds");
    return COMPOUND_STRING("None");
}

static const u8 *GetInfoTerrainName(void)
{
    if (gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN)
        return COMPOUND_STRING("Grassy");
    if (gFieldStatuses & STATUS_FIELD_MISTY_TERRAIN)
        return COMPOUND_STRING("Misty");
    if (gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN)
        return COMPOUND_STRING("Electric");
    if (gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
        return COMPOUND_STRING("Psychic");
    return COMPOUND_STRING("None");
}

// Appends "<label>" then, if count > 1, "x<count>", then a space.
static u8 *AppendEntry(u8 *p, const u8 *label, u32 count)
{
    p = StringAppend(p, label);
    if (count > 1)
    {
        *p++ = CHAR_x;
        p = ConvertIntToDecimalStringN(p, count, STR_CONV_MODE_LEFT_ALIGN, 2);
    }
    *p++ = CHAR_SPACE;
    *p = EOS;
    return p;
}

// Appends "<label>(<turns>) " when turns > 0.
static u8 *AppendTimedEntry(u8 *p, const u8 *label, u32 turns)
{
    p = StringAppend(p, label);
    if (turns > 0)
    {
        *p++ = CHAR_LEFT_PAREN;
        p = ConvertIntToDecimalStringN(p, turns, STR_CONV_MODE_LEFT_ALIGN, 2);
        *p++ = CHAR_RIGHT_PAREN;
    }
    *p++ = CHAR_SPACE;
    *p = EOS;
    return p;
}

static void BuildHazardLine(u8 *dst, u32 side)
{
    u8 *p = dst;

    *p = EOS;
    if (gSideTimers[side].spikesAmount)
        p = AppendEntry(p, COMPOUND_STRING("Spikes"), gSideTimers[side].spikesAmount);
    if (gSideTimers[side].toxicSpikesAmount)
        p = AppendEntry(p, COMPOUND_STRING("T.Spikes"), gSideTimers[side].toxicSpikesAmount);
    if (IsHazardOnSide(side, HAZARDS_STEALTH_ROCK))
        p = AppendEntry(p, COMPOUND_STRING("Rocks"), 1);
    if (IsHazardOnSide(side, HAZARDS_STICKY_WEB))
        p = AppendEntry(p, COMPOUND_STRING("Web"), 1);
    if (IsHazardOnSide(side, HAZARDS_STEELSURGE))
        p = AppendEntry(p, COMPOUND_STRING("Steelsurge"), 1);

    if (p == dst)
        StringCopy(dst, COMPOUND_STRING("None"));
}

static void BuildScreenLine(u8 *dst, u32 side)
{
    u8 *p = dst;

    *p = EOS;
    if (gSideStatuses[side] & SIDE_STATUS_REFLECT)
        p = AppendTimedEntry(p, COMPOUND_STRING("Reflect"), gSideTimers[side].reflectTimer);
    if (gSideStatuses[side] & SIDE_STATUS_LIGHTSCREEN)
        p = AppendTimedEntry(p, COMPOUND_STRING("L.Screen"), gSideTimers[side].lightscreenTimer);
    if (gSideStatuses[side] & SIDE_STATUS_AURORA_VEIL)
        p = AppendTimedEntry(p, COMPOUND_STRING("Veil"), gSideTimers[side].auroraVeilTimer);
    if (gSideStatuses[side] & SIDE_STATUS_TAILWIND)
        p = AppendTimedEntry(p, COMPOUND_STRING("Tailwind"), gSideTimers[side].tailwindTimer);
    if (gSideStatuses[side] & SIDE_STATUS_SAFEGUARD)
        p = AppendTimedEntry(p, COMPOUND_STRING("Safeguard"), gSideTimers[side].safeguardTimer);
    if (gSideStatuses[side] & SIDE_STATUS_MIST)
        p = AppendTimedEntry(p, COMPOUND_STRING("Mist"), gSideTimers[side].mistTimer);
    if (gSideStatuses[side] & SIDE_STATUS_LUCKY_CHANT)
        p = AppendTimedEntry(p, COMPOUND_STRING("L.Chant"), gSideTimers[side].luckyChantTimer);

    if (p == dst)
        StringCopy(dst, COMPOUND_STRING("None"));
}

// Current PP for a foe's move slot: live data from the active battler when that
// party slot is on the field, else the stored party PP.
static u32 GetFoeMoveSlotPP(struct Pokemon *foeParty, u32 partyIndex, u32 slot)
{
    for (u32 i = 0; i < gBattlersCount; i++)
    {
        if (!IsOnPlayerSide(i) && gBattlerPartyIndexes[i] == partyIndex
            && GetBattlerTrainer(i) == B_TRAINER_OPPONENT_A)
            return gBattleMons[i].pp[slot];
    }

    return GetMonData(&foeParty[partyIndex], MON_DATA_PP1 + slot, NULL);
}

// ---------------------------------------------------------------------------
// Rendering.
// ---------------------------------------------------------------------------

#define LINE_H  14

static void PrintLine(u8 windowId, const u8 *str, u32 x, u32 y)
{
    // FONT_NARROW (same height as FONT_NORMAL, narrower glyphs) fits more text
    // per line, so dense hazard/screen/condition lists clip far less often.
    AddTextPrinterParameterized(windowId, FONT_NARROW, str, x, y, 0, NULL);
}

static void DrawFieldPage(u8 windowId)
{
    // Large enough to hold a side with every screen/hazard active at once (the
    // text is clipped to the window width on screen, but the buffer must fit it).
    u8 line[128];
    u8 *p;
    u32 y = 0;

    PrintLine(windowId, COMPOUND_STRING("BATTLE INFO  -  FIELD"), 0, y);
    y += LINE_H;

    p = StringCopy(line, COMPOUND_STRING("Weather: "));
    StringAppend(p, GetInfoWeatherName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    p = StringCopy(line, COMPOUND_STRING("Terrain: "));
    StringAppend(p, GetInfoTerrainName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    PrintLine(windowId, COMPOUND_STRING("Your side:"), 0, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("Hazards: "));
    BuildHazardLine(p, B_SIDE_PLAYER);
    PrintLine(windowId, line, 8, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("Screens: "));
    BuildScreenLine(p, B_SIDE_PLAYER);
    PrintLine(windowId, line, 8, y);
    y += LINE_H;

    PrintLine(windowId, COMPOUND_STRING("Foe side:"), 0, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("Hazards: "));
    BuildHazardLine(p, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 8, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("Screens: "));
    BuildScreenLine(p, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 8, y);

    PrintLine(windowId, COMPOUND_STRING("R: Next page    B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

// Number of mons in the foe's (opponent A's) party.
static u32 GetFoePartyCount(struct Pokemon *foeParty)
{
    u32 count = 0;

    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&foeParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
            && !GetMonData(&foeParty[i], MON_DATA_IS_EGG, NULL))
            count++;
    }
    return count;
}

static void DrawFoePage(u8 windowId, u32 foeIndex)
{
    u8 line[64];
    u8 *p;
    u32 y = 0;
    struct Pokemon *foeParty = GetTrainerParty(B_TRAINER_OPPONENT_A);
    u32 count = GetFoePartyCount(foeParty);
    // What the player has actually seen of this slot: "seen" comes from the
    // engine's sent-out flag, while the foe's *revealed* moves/ability/held item
    // are recorded into gAiPartyData as they show up in battle. Species and level
    // are read straight from the party once the mon has been on the field.
    bool32 seen = gBattleStruct->partyState[B_TRAINER_OPPONENT_A][foeIndex].sentOut;
    struct AiPartyMon *revealed = (gAiPartyData != NULL)
        ? &gAiPartyData->mons[B_SIDE_OPPONENT][foeIndex] : NULL;

    p = StringCopy(line, COMPOUND_STRING("BATTLE INFO  -  FOE "));
    p = ConvertIntToDecimalStringN(p, foeIndex + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    *p++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(p, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    if (!seen)
    {
        PrintLine(windowId, COMPOUND_STRING("Not yet seen."), 0, y);
        PrintLine(windowId, COMPOUND_STRING("<>: Mon  R: Next  B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
        return;
    }

    // Name, gender and level (known once the mon has appeared); flag if fainted.
    u32 gender = GetMonGender(&foeParty[foeIndex]);
    p = StringCopy(line, GetSpeciesName(GetMonData(&foeParty[foeIndex], MON_DATA_SPECIES, NULL)));
    if (gender == MON_MALE)
        *p++ = CHAR_MALE;
    else if (gender == MON_FEMALE)
        *p++ = CHAR_FEMALE;
    *p++ = CHAR_SPACE;
    *p++ = CHAR_LV;
    p = ConvertIntToDecimalStringN(p, GetMonData(&foeParty[foeIndex], MON_DATA_LEVEL, NULL), STR_CONV_MODE_LEFT_ALIGN, 3);
    if (GetMonData(&foeParty[foeIndex], MON_DATA_HP, NULL) == 0)
        StringCopy(p, COMPOUND_STRING("  FNT"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // Ability — only once genuinely revealed in battle (gAiPartyData pre-knows it
    // under AI_FLAG_OMNISCIENT, so gate on our own reveal flag instead).
    bool32 abilitySeen = (gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & (1u << foeIndex)) != 0;
    p = StringCopy(line, COMPOUND_STRING("Ability: "));
    if (abilitySeen && revealed != NULL && revealed->ability != ABILITY_NONE)
        StringCopy(p, gAbilitiesInfo[revealed->ability].name);
    else
        StringCopy(p, COMPOUND_STRING("?"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // FORK: FEATURE_INNATE_ABILITIES — list the foe's innate abilities (always-active
    // passives on top of the chosen ability above). Gated on the same in-battle reveal
    // flag as the chosen ability, so the viewer keeps showing only what has been seen
    // in action and doesn't even leak that the mon has innates until then.
    if (GetConfig(FEATURE_INNATE_ABILITIES) && abilitySeen)
    {
        enum Species foeSpecies = GetMonData(&foeParty[foeIndex], MON_DATA_SPECIES, NULL);
        bool32 anyInnate = FALSE;
        p = StringCopy(line, COMPOUND_STRING("Innate: "));
        for (u32 slot = 0; slot < MAX_INNATE_ABILITIES; slot++)
        {
            enum Ability innate = GetSpeciesInnate(foeSpecies, slot);
            if (innate == ABILITY_NONE)
                continue;
            if (anyInnate)
                p = StringCopy(p, COMPOUND_STRING(", "));
            p = StringCopy(p, gAbilitiesInfo[innate].name);
            anyInnate = TRUE;
        }
        if (anyInnate)
        {
            PrintLine(windowId, line, 0, y);
            y += LINE_H;
        }
    }

    // Held item — only once its effect has been genuinely revealed in battle.
    enum Item heldItem = GetMonData(&foeParty[foeIndex], MON_DATA_HELD_ITEM, NULL);
    bool32 itemSeen = (gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & (1u << foeIndex)) != 0;
    p = StringCopy(line, COMPOUND_STRING("Item: "));
    if (itemSeen && heldItem != ITEM_NONE)
        StringCopy(p, GetItemName(heldItem));
    else
        StringCopy(p, COMPOUND_STRING("?"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    PrintLine(windowId, COMPOUND_STRING("Moves:"), 0, y);
    y += LINE_H;
    // Only the move slots the foe has *actually used* (tracked in gBattleStruct),
    // read straight from its real moveset. PP shows current / max-with-PP-Ups, so
    // it stays correct under B_FRONTIER_MAX_PP.
    u32 usedMask = gBattleStruct->infoUsedMoves[B_SIDE_OPPONENT][foeIndex];
    u8 ppBonuses = GetMonData(&foeParty[foeIndex], MON_DATA_PP_BONUSES, NULL);
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = GetMonData(&foeParty[foeIndex], MON_DATA_MOVE1 + i, NULL);

        if (!(usedMask & (1u << i)) || move == MOVE_NONE)
        {
            PrintLine(windowId, COMPOUND_STRING("  ?"), 0, y);
        }
        else
        {
            p = StringCopy(line, COMPOUND_STRING("  "));
            p = StringCopy(p, GetMoveName(move));
            p = StringCopy(p, COMPOUND_STRING("  "));
            p = ConvertIntToDecimalStringN(p, GetFoeMoveSlotPP(foeParty, foeIndex, i), STR_CONV_MODE_LEFT_ALIGN, 2);
            *p++ = CHAR_SLASH;
            ConvertIntToDecimalStringN(p, CalculatePPWithBonus(move, ppBonuses, i), STR_CONV_MODE_LEFT_ALIGN, 2);
            PrintLine(windowId, line, 0, y);
        }
        y += LINE_H;
    }

    PrintLine(windowId, COMPOUND_STRING("<>: Mon  R: Next  B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

// Name of the battler's primary (status1) condition, or NULL if healthy.
static const u8 *GetStatus1Name(u32 status1)
{
    if (status1 & STATUS1_SLEEP)
        return COMPOUND_STRING("Asleep");
    if (status1 & STATUS1_PARALYSIS)
        return COMPOUND_STRING("Paralysis");
    if (status1 & STATUS1_FREEZE)
        return COMPOUND_STRING("Frozen");
    if (status1 & STATUS1_BURN)
        return COMPOUND_STRING("Burn");
    if (status1 & STATUS1_TOXIC_POISON)
        return COMPOUND_STRING("Bad Poison");
    if (status1 & STATUS1_POISON)
        return COMPOUND_STRING("Poison");
    if (status1 & STATUS1_FROSTBITE)
        return COMPOUND_STRING("Frostbite");
    return NULL;
}

// Builds a space-separated list of the battler's primary status + notable
// volatile conditions, or "None". Curated to the conditions a player would
// track, not every internal volatile flag.
static void BuildConditionLine(u8 *dst, enum BattlerId battler)
{
    u8 *p = dst;
    const u8 *status = GetStatus1Name(gBattleMons[battler].status1);
    struct Volatiles *v = &gBattleMons[battler].volatiles;

    *p = EOS;
    if (status != NULL)
        p = AppendEntry(p, status, 1);
    if (v->confusionTurns)
        p = AppendEntry(p, COMPOUND_STRING("Confusion"), 1);
    if (v->infatuation)
        p = AppendEntry(p, COMPOUND_STRING("Infatuation"), 1);
    if (v->leechSeed)
        p = AppendEntry(p, COMPOUND_STRING("Leech Seed"), 1);
    if (v->nightmare)
        p = AppendEntry(p, COMPOUND_STRING("Nightmare"), 1);
    if (v->cursed)
        p = AppendEntry(p, COMPOUND_STRING("Curse"), 1);
    if (v->perishSong)
        p = AppendEntry(p, COMPOUND_STRING("Perish Song"), 1);
    if (v->yawn)
        p = AppendEntry(p, COMPOUND_STRING("Drowsy"), 1);
    if (v->saltCure)
        p = AppendEntry(p, COMPOUND_STRING("Salt Cure"), 1);
    if (v->wrapped)
        p = AppendEntry(p, COMPOUND_STRING("Trapped"), 1);
    if (v->substitute)
        p = AppendEntry(p, COMPOUND_STRING("Substitute"), 1);
    if (v->tauntTimer)
        p = AppendEntry(p, COMPOUND_STRING("Taunt"), 1);
    if (v->encoredMove)
        p = AppendEntry(p, COMPOUND_STRING("Encore"), 1);
    if (v->disabledMove)
        p = AppendEntry(p, COMPOUND_STRING("Disable"), 1);
    if (v->torment)
        p = AppendEntry(p, COMPOUND_STRING("Torment"), 1);
    if (v->healBlock)
        p = AppendEntry(p, COMPOUND_STRING("Heal Block"), 1);
    if (v->embargo)
        p = AppendEntry(p, COMPOUND_STRING("Embargo"), 1);
    if (v->root)
        p = AppendEntry(p, COMPOUND_STRING("Ingrain"), 1);
    if (v->aquaRing)
        p = AppendEntry(p, COMPOUND_STRING("Aqua Ring"), 1);
    if (v->magnetRise)
        p = AppendEntry(p, COMPOUND_STRING("Magnet Rise"), 1);
    if (v->telekinesis)
        p = AppendEntry(p, COMPOUND_STRING("Telekinesis"), 1);
    if (v->smackDown)
        p = AppendEntry(p, COMPOUND_STRING("Grounded"), 1);
    if (v->electrified)
        p = AppendEntry(p, COMPOUND_STRING("Electrified"), 1);

    if (p == dst)
        StringCopy(dst, COMPOUND_STRING("None"));
}

static u32 DrawConditionsForSide(u8 windowId, bool32 playerSide, u32 y)
{
    // Sized for the pathological worst case of every tracked status + volatile at
    // once (~230 chars); a real mon never stacks that many, but never overflow.
    u8 line[256];
    u8 *p;

    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (IsOnPlayerSide(battler) != playerSide || !IsBattlerAlive(battler))
            continue;

        p = StringCopy(line, playerSide ? COMPOUND_STRING("You: ") : COMPOUND_STRING("Foe: "));
        StringCopy(p, gBattleMons[battler].nickname);
        PrintLine(windowId, line, 0, y);
        y += LINE_H;

        BuildConditionLine(line, battler);
        PrintLine(windowId, line, 8, y);
        y += LINE_H;
    }
    return y;
}

static void DrawConditionsPage(u8 windowId)
{
    u32 y = 0;

    PrintLine(windowId, COMPOUND_STRING("BATTLE INFO  -  CONDITIONS"), 0, y);
    y += LINE_H;

    y = DrawConditionsForSide(windowId, TRUE, y);
    DrawConditionsForSide(windowId, FALSE, y);

    PrintLine(windowId, COMPOUND_STRING("R: Next page    B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

static const u8 *GetStatAbbr(u32 stat)
{
    switch (stat)
    {
    case STAT_ATK:     return COMPOUND_STRING("Atk");
    case STAT_DEF:     return COMPOUND_STRING("Def");
    case STAT_SPATK:   return COMPOUND_STRING("SpA");
    case STAT_SPDEF:   return COMPOUND_STRING("SpD");
    case STAT_SPEED:   return COMPOUND_STRING("Spe");
    case STAT_ACC:     return COMPOUND_STRING("Acc");
    case STAT_EVASION: return COMPOUND_STRING("Eva");
    default:           return COMPOUND_STRING("?");
    }
}

// Builds "Atk+2 Spe-1 ..." for the battler's non-default stat stages, or "None".
static void BuildStatChangeLine(u8 *dst, enum BattlerId battler)
{
    static const u8 sStatOrder[] = { STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED, STAT_ACC, STAT_EVASION };
    u8 *p = dst;

    *p = EOS;
    for (u32 i = 0; i < ARRAY_COUNT(sStatOrder); i++)
    {
        u32 stat = sStatOrder[i];
        s32 delta = gBattleMons[battler].statStages[stat] - DEFAULT_STAT_STAGE;

        if (delta == 0)
            continue;
        p = StringAppend(p, GetStatAbbr(stat));
        if (delta > 0)
        {
            *p++ = CHAR_PLUS;
        }
        else
        {
            *p++ = CHAR_HYPHEN;
            delta = -delta;
        }
        p = ConvertIntToDecimalStringN(p, delta, STR_CONV_MODE_LEFT_ALIGN, 1);
        *p++ = CHAR_SPACE;
        *p = EOS;
    }

    if (p == dst)
        StringCopy(dst, COMPOUND_STRING("None"));
}

static u32 DrawStatsForSide(u8 windowId, bool32 playerSide, u32 y)
{
    u8 line[64];
    u8 *p;

    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (IsOnPlayerSide(battler) != playerSide || !IsBattlerAlive(battler))
            continue;

        p = StringCopy(line, playerSide ? COMPOUND_STRING("You: ") : COMPOUND_STRING("Foe: "));
        StringCopy(p, gBattleMons[battler].nickname);
        PrintLine(windowId, line, 0, y);
        y += LINE_H;

        BuildStatChangeLine(line, battler);
        PrintLine(windowId, line, 8, y);
        y += LINE_H;
    }
    return y;
}

static void DrawStatsPage(u8 windowId)
{
    u32 y = 0;

    PrintLine(windowId, COMPOUND_STRING("BATTLE INFO  -  STAT CHANGES"), 0, y);
    y += LINE_H;

    y = DrawStatsForSide(windowId, TRUE, y);
    DrawStatsForSide(windowId, FALSE, y);

    PrintLine(windowId, COMPOUND_STRING("R: Next page    B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

static void RedrawInfo(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    switch (gTasks[taskId].tPage)
    {
    case INFO_PAGE_CONDITIONS:
        DrawConditionsPage(windowId);
        break;
    case INFO_PAGE_STATS:
        DrawStatsPage(windowId);
        break;
    case INFO_PAGE_FOE:
        DrawFoePage(windowId, gTasks[taskId].tFoeIndex);
        break;
    case INFO_PAGE_FIELD:
    default:
        DrawFieldPage(windowId);
        break;
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------
// Task / callback flow (mirrors the debug menu's open + return-to-battle path).
// ---------------------------------------------------------------------------

static void Task_InfoFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(ReshowBattleScreenAfterMenu);
    }
}

static void Task_InfoProcessInput(u8 taskId)
{
    u32 count = GetFoePartyCount(GetTrainerParty(B_TRAINER_OPPONENT_A));

    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gTasks[taskId].func = Task_InfoFadeOut;
    }
    else if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (++gTasks[taskId].tPage >= INFO_PAGE_COUNT)
            gTasks[taskId].tPage = 0;
        RedrawInfo(taskId);
    }
    else if (JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (--gTasks[taskId].tPage < 0)
            gTasks[taskId].tPage = INFO_PAGE_COUNT - 1;
        RedrawInfo(taskId);
    }
    else if (gTasks[taskId].tPage == INFO_PAGE_FOE && count != 0
             && (JOY_NEW(DPAD_RIGHT) || JOY_NEW(DPAD_DOWN)))
    {
        PlaySE(SE_SELECT);
        if (++gTasks[taskId].tFoeIndex >= (s16)count)
            gTasks[taskId].tFoeIndex = 0;
        RedrawInfo(taskId);
    }
    else if (gTasks[taskId].tPage == INFO_PAGE_FOE && count != 0
             && (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_UP)))
    {
        PlaySE(SE_SELECT);
        if (--gTasks[taskId].tFoeIndex < 0)
            gTasks[taskId].tFoeIndex = count - 1;
        RedrawInfo(taskId);
    }
}

static void Task_InfoFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_InfoProcessInput;
}

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_FrontierBattleInfo(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        ResetVramOamAndBgCntRegs();
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ResetAllBgsCoordinates();
        CloseMainBattleScreen();
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadPalette(sBgColor, BG_PLTT_ID(0), sizeof(sBgColor));
        LoadPalette(GetOverworldTextboxPalettePtr(), BG_PLTT_ID(15), PLTT_SIZEOF(8));
        gMain.state++;
        break;
    case 4:
        taskId = CreateTask(Task_InfoFadeIn, 0);
        gTasks[taskId].tWindowId = AddWindow(&sInfoWindowTemplate);
        gTasks[taskId].tPage = INFO_PAGE_FIELD;
        gTasks[taskId].tFoeIndex = 0;
        PutWindowTilemap(gTasks[taskId].tWindowId);
        RedrawInfo(taskId);
        gMain.state++;
        break;
    case 5:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        gMain.state = 0;
        return;
    }
}
