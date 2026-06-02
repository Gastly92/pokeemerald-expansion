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

// Returns the foe's current PP for a revealed move, reading live data from the
// active battler when that party slot is on the field, else from the party mon.
// Returns 0xFF when the move isn't found.
static u32 GetFoeMovePP(struct Pokemon *foeParty, u32 partyIndex, enum Move move)
{
    u32 i;

    for (i = 0; i < gBattlersCount; i++)
    {
        if (!IsOnPlayerSide(i) && gBattlerPartyIndexes[i] == partyIndex
            && GetBattlerTrainer(i) == B_TRAINER_OPPONENT_A)
        {
            for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
            {
                if (gBattleMons[i].moves[slot] == move)
                    return gBattleMons[i].pp[slot];
            }
        }
    }

    for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
    {
        if (GetMonData(&foeParty[partyIndex], MON_DATA_MOVE1 + slot, NULL) == move)
            return GetMonData(&foeParty[partyIndex], MON_DATA_PP1 + slot, NULL);
    }

    return 0xFF;
}

// ---------------------------------------------------------------------------
// Rendering.
// ---------------------------------------------------------------------------

#define LINE_H  14

static void PrintLine(u8 windowId, const u8 *str, u32 x, u32 y)
{
    AddTextPrinterParameterized(windowId, FONT_NORMAL, str, x, y, 0, NULL);
}

static void DrawFieldPage(u8 windowId)
{
    // Large enough to hold a side with every screen/hazard active at once (the
    // text is clipped to the window width on screen, but the buffer must fit it).
    u8 line[128];
    u8 *p;
    u32 y = 0;

    PrintLine(windowId, COMPOUND_STRING("BATTLE INFO  -  FIELD"), 0, y);
    y += LINE_H + 2;

    p = StringCopy(line, COMPOUND_STRING("Weather: "));
    StringAppend(p, GetInfoWeatherName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    p = StringCopy(line, COMPOUND_STRING("Terrain: "));
    StringAppend(p, GetInfoTerrainName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H + 2;

    PrintLine(windowId, COMPOUND_STRING("Your side:"), 0, y);
    y += LINE_H;
    BuildHazardLine(line, B_SIDE_PLAYER);
    PrintLine(windowId, line, 8, y);
    y += LINE_H;
    BuildScreenLine(line, B_SIDE_PLAYER);
    PrintLine(windowId, line, 8, y);
    y += LINE_H + 2;

    PrintLine(windowId, COMPOUND_STRING("Foe side:"), 0, y);
    y += LINE_H;
    BuildHazardLine(line, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 8, y);
    y += LINE_H;
    BuildScreenLine(line, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 8, y);

    PrintLine(windowId, COMPOUND_STRING("R: Foe info    B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

static void DrawFoePage(u8 windowId, u32 foeIndex)
{
    u8 line[64];
    u8 *p;
    u32 y = 0;
    u32 count = gAiPartyData->count[B_SIDE_OPPONENT];
    struct AiPartyMon *foe = &gAiPartyData->mons[B_SIDE_OPPONENT][foeIndex];
    struct Pokemon *foeParty = GetTrainerParty(B_TRAINER_OPPONENT_A);

    p = StringCopy(line, COMPOUND_STRING("BATTLE INFO  -  FOE "));
    p = ConvertIntToDecimalStringN(p, foeIndex + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    *p++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(p, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    PrintLine(windowId, line, 0, y);
    y += LINE_H + 2;

    if (!foe->wasSentInBattle || foe->species == SPECIES_NONE)
    {
        PrintLine(windowId, COMPOUND_STRING("Not yet seen."), 0, y);
        PrintLine(windowId, COMPOUND_STRING("<>: Mon  R: Field  B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
        return;
    }

    // Name and level.
    p = StringCopy(line, GetSpeciesName(foe->species));
    *p++ = CHAR_SPACE;
    *p++ = CHAR_LV;
    ConvertIntToDecimalStringN(p, foe->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // Ability (only shown once revealed).
    p = StringCopy(line, COMPOUND_STRING("Ability: "));
    if (foe->ability != ABILITY_NONE)
        StringCopy(p, gAbilitiesInfo[foe->ability].name);
    else
        StringCopy(p, COMPOUND_STRING("?"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // Held item (only shown once its effect has been revealed in battle).
    p = StringCopy(line, COMPOUND_STRING("Item: "));
    if (foe->heldEffect != HOLD_EFFECT_NONE)
        StringCopy(p, GetItemName(GetMonData(&foeParty[foeIndex], MON_DATA_HELD_ITEM, NULL)));
    else
        StringCopy(p, COMPOUND_STRING("?"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H + 2;

    PrintLine(windowId, COMPOUND_STRING("Moves:"), 0, y);
    y += LINE_H;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        enum Move move = foe->moves[i];

        if (move == MOVE_NONE)
        {
            PrintLine(windowId, COMPOUND_STRING("  ?"), 0, y);
        }
        else
        {
            u32 pp = GetFoeMovePP(foeParty, foeIndex, move);

            p = StringCopy(line, COMPOUND_STRING("  "));
            p = StringCopy(p, GetMoveName(move));
            if (pp != 0xFF)
            {
                p = StringCopy(p, COMPOUND_STRING("  "));
                p = ConvertIntToDecimalStringN(p, pp, STR_CONV_MODE_LEFT_ALIGN, 2);
                *p++ = CHAR_SLASH;
                ConvertIntToDecimalStringN(p, GetMovePP(move), STR_CONV_MODE_LEFT_ALIGN, 2);
            }
            else
            {
                *p = EOS;
            }
            PrintLine(windowId, line, 0, y);
        }
        y += LINE_H;
    }

    PrintLine(windowId, COMPOUND_STRING("<>: Mon  R: Field  B: Close"), 0, (INFO_WIN_HEIGHT * 8) - 14);
}

static void RedrawInfo(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (gTasks[taskId].tPage == INFO_PAGE_FOE)
        DrawFoePage(windowId, gTasks[taskId].tFoeIndex);
    else
        DrawFieldPage(windowId);
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
    u32 count = gAiPartyData->count[B_SIDE_OPPONENT];

    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gTasks[taskId].func = Task_InfoFadeOut;
    }
    else if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tPage ^= 1;
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
