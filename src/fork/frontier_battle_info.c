// FORK: read-only "battle info" viewer shown in Frontier facilities in place of
// the (disabled) BAG action. See include/config/frontier.h (B_FRONTIER_BATTLE_INFO)
// and include/frontier_battle_info.h for the integration notes. This whole file
// is fork-only, so it never conflicts on an upstream sync; it only *reads* battle
// state, never mutates it.

#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_main.h" // FORK: GetBattlerTotalSpeedStat for the player's effective Speed
#include "battle_util.h"
#include "bg.h"
#include "config_changes.h" // FORK: GetConfig(FEATURE_INNATE_ABILITIES)
#include "fork/innate_abilities.h" // FORK: FEATURE_INNATE_ABILITIES
#include "fork/frontier_battle_info.h"
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
#include "constants/characters.h" // FORK: TEXT_COLOR_* for the styled title/footer text
#include "constants/pokemon.h"
#include "constants/rgb.h"
#include "constants/songs.h"

bool8 gFrontierBattleInfoActive = FALSE;

// FORK: where to return when the viewer is closed. The in-battle action menu sets
// this to ReshowBattleScreenAfterMenu (back to the battle); the in-battle "choose
// a Pokémon" party menu sets it to a callback that re-opens that party menu. Set
// via OpenFrontierBattleInfo() before CB2_FrontierBattleInfo runs; defaults to the
// battle screen so a stray open can never strand the player.
static MainCallback sInfoExitCallback = NULL;

// FORK: styling. The viewer used to be a flat full-screen white window of plain
// text. We now dress it up purely by reusing existing assets/helpers (no new art):
//   - a window frame (the player's chosen options frame) around the window,
//   - the standard menu palette so text can use the conventional TEXT_COLOR_* set,
//   - a transparent window over a soft backdrop colour, and
//   - coloured page titles / footer for a clear visual hierarchy.
// The text window fills the full width (28 tiles); its char block is then nearly
// full, so the frame lives on its own BG (FRAME_BG) whose char block has room for
// the 9 frame tiles. That BG has no window, so we hand it a heap tilemap buffer the
// same way AddWindow() does for the text window (a static buffer would grow EWRAM
// into the heap region and corrupt it).
#define INFO_WIN_WIDTH   28
#define INFO_WIN_HEIGHT  18

#define FRAME_BG         1
#define FRAME_PAL_NUM    14   // BG palette slot for the frame (window uses 15)
#define FRAME_BASE_TILE  1    // first of the 9 frame tiles in FRAME_BG's (empty) char block

// The 9-tile frame in DrawTextBorderOuter()'s tile order (+4 is the unused centre).
#define FRAME_TILE_TL   (FRAME_BASE_TILE + 0)
#define FRAME_TILE_TOP  (FRAME_BASE_TILE + 1)
#define FRAME_TILE_TR   (FRAME_BASE_TILE + 2)
#define FRAME_TILE_L    (FRAME_BASE_TILE + 3)
#define FRAME_TILE_R    (FRAME_BASE_TILE + 5)
#define FRAME_TILE_BL   (FRAME_BASE_TILE + 6)
#define FRAME_TILE_BOT  (FRAME_BASE_TILE + 7)
#define FRAME_TILE_BR   (FRAME_BASE_TILE + 8)

// Heap tilemap buffer for FRAME_BG, allocated on open and freed on close.
static u16 *sFrameTilemapBuffer = NULL;

enum
{
    // FORK: Speed Tiers leads — it's the page that actually drives a turn
    // decision, so it's both first in the L/R cycle and the default landing page.
    INFO_PAGE_SPEED,
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

// FORK: remember the last page/foe the player was viewing, so re-opening the
// viewer (often once per turn) returns to where they left off instead of always
// resetting to the front. The viewer is modal, so saving on close is enough to
// have the value ready for the next open. Defaults to the Speed Tiers page on
// the very first open of the session.
static u8 sLastPage = INFO_PAGE_SPEED;
static s8 sLastFoeIndex = 0;

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
    // FORK: frame layer — its own char block (the text window nearly fills BG0's),
    // higher priority so the border sits cleanly around the window.
    {
        .bg = FRAME_BG,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
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

// FORK: soft cool-grey backdrop shown through the (now transparent) window, so the
// screen reads as a framed panel rather than a flat white sheet. Kept light to keep
// the dark body text legible.
static const u16 sBgColor[] = {RGB(23, 25, 30)};

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

// FONT_NARROW (same height as FONT_NORMAL, narrower glyphs) fits more text per
// line, so dense hazard/screen/condition lists clip far less often. The window is
// transparent over the backdrop, so text is drawn with a transparent background
// (else each glyph cell would paint an opaque box over the backdrop). Colours are
// indices into gStandardMenuPalette, loaded into the window's palette slot.
static void PrintLineEx(u8 windowId, const u8 *str, u32 x, u32 y, u8 fg, u8 shadow)
{
    u8 color[3] = { TEXT_COLOR_TRANSPARENT, fg, shadow };

    AddTextPrinterParameterized4(windowId, FONT_NARROW, x, y, 0, 0, color, 0, str);
}

static void PrintLine(u8 windowId, const u8 *str, u32 x, u32 y)
{
    PrintLineEx(windowId, str, x, y, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY);
}

// Page heading (top row), in red to anchor the page.
static void PrintTitle(u8 windowId, const u8 *str)
{
    PrintLineEx(windowId, str, 0, 0, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED);
}

// Navigation hint pinned to the bottom row, in blue to read as chrome, not data.
static void PrintFooter(u8 windowId, const u8 *str)
{
    PrintLineEx(windowId, str, 0, (INFO_WIN_HEIGHT * 8) - 14, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE);
}

// FORK: compact "n/N" page counter, right-aligned on the footer row, so the
// player can see how many pages exist and where they are. Same blue chrome as
// the footer hint; the two never collide because every footer hint is short
// enough to leave the right edge free.
static void PrintPageIndicator(u8 windowId, u32 page)
{
    u8 str[8];
    u8 *p = ConvertIntToDecimalStringN(str, page + 1, STR_CONV_MODE_LEFT_ALIGN, 1);

    *p++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(p, INFO_PAGE_COUNT, STR_CONV_MODE_LEFT_ALIGN, 1);
    PrintLineEx(windowId, str, (INFO_WIN_WIDTH * 8) - GetStringWidth(FONT_NARROW, str, 0),
                (INFO_WIN_HEIGHT * 8) - 14, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE);
}

// FORK: draw the window frame ring on FRAME_BG, around the text window's bounds.
// Mirrors DrawTextBorderOuter() but targets FRAME_BG (the window lives on BG0).
static void DrawInfoFrame(void)
{
    u32 l = sInfoWindowTemplate.tilemapLeft;
    u32 t = sInfoWindowTemplate.tilemapTop;
    u32 w = sInfoWindowTemplate.width;
    u32 h = sInfoWindowTemplate.height;

    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_TL,  l - 1, t - 1, 1, 1, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_TOP, l,     t - 1, w, 1, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_TR,  l + w, t - 1, 1, 1, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_L,   l - 1, t,     1, h, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_R,   l + w, t,     1, h, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_BL,  l - 1, t + h, 1, 1, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_BOT, l,     t + h, w, 1, FRAME_PAL_NUM);
    FillBgTilemapBufferRect(FRAME_BG, FRAME_TILE_BR,  l + w, t + h, 1, 1, FRAME_PAL_NUM);
    CopyBgTilemapBufferToVram(FRAME_BG);
}

// FORK: DETERMINISTIC_DAMAGE replaces the random damage roll with a fixed,
// turn-scaling percentage (DETERMINISTIC_DAMAGE_PERCENT). Surface the current
// turn and that multiplier so the player can read the exact roll they'll get.
static void DrawDeterministicDamageLine(u8 windowId, u8 *line, u32 y)
{
    u8 *p = StringCopy(line, COMPOUND_STRING("Turn "));
    // gBattleTurnCounter is 0 on the first turn; show it 1-based.
    p = ConvertIntToDecimalStringN(p, gBattleTurnCounter + 1, STR_CONV_MODE_LEFT_ALIGN, 3);
    p = StringCopy(p, COMPOUND_STRING("  Damage "));
    p = ConvertIntToDecimalStringN(p, DETERMINISTIC_DAMAGE_PERCENT, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(p, COMPOUND_STRING("%"));
    PrintLine(windowId, line, 0, y);
}

static void DrawFieldPage(u8 windowId)
{
    // Large enough to hold a side with every screen/hazard active at once (the
    // text is clipped to the window width on screen, but the buffer must fit it).
    u8 line[128];
    u8 *p;
    u32 y = 0;

    PrintTitle(windowId, COMPOUND_STRING("BATTLE INFO  -  FIELD"));
    y += LINE_H;

    // FORK: the per-turn deterministic damage multiplier (only when that mode is on).
    if (GetConfig(DETERMINISTIC_DAMAGE))
    {
        DrawDeterministicDamageLine(windowId, line, y);
        y += LINE_H;
    }

    p = StringCopy(line, COMPOUND_STRING("Weather: "));
    StringAppend(p, GetInfoWeatherName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    p = StringCopy(line, COMPOUND_STRING("Terrain: "));
    StringAppend(p, GetInfoTerrainName());
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // FORK: the side hazard/screen rows are prefixed "You"/"Foe" inline (rather than
    // sitting under standalone "Your side:"/"Foe side:" headers) to free the vertical
    // room the turn/damage line above needs in this fixed-height window.
    p = StringCopy(line, COMPOUND_STRING("You Hazards: "));
    BuildHazardLine(p, B_SIDE_PLAYER);
    PrintLine(windowId, line, 0, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("You Screens: "));
    BuildScreenLine(p, B_SIDE_PLAYER);
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    p = StringCopy(line, COMPOUND_STRING("Foe Hazards: "));
    BuildHazardLine(p, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 0, y);
    y += LINE_H;
    p = StringCopy(line, COMPOUND_STRING("Foe Screens: "));
    BuildScreenLine(p, B_SIDE_OPPONENT);
    PrintLine(windowId, line, 0, y);

    PrintFooter(windowId, COMPOUND_STRING("L/R: Page    B: Close"));
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

// FORK: The mon whose *identity* the viewer should display for a foe party slot.
// A foe with an active Illusion (Zoroark/Zorua) is disguised as another party
// member, so revealing its real species before the Illusion breaks would leak it.
// While that slot's on-field battler is ILLUSION_ON, return the disguise mon the
// player actually sees (this mirrors the health box, which also reads
// GetIllusionMonPtr for species/nickname/gender/level). Once the Illusion is
// broken (ILLUSION_OFF) this falls back to the real party slot, so the true
// species reveals exactly when the player learns it. Identity reads (species,
// gender, level) use this; HP/FNT and reveal-gated data (moves, ability, item)
// stay keyed to the real slot, matching what the health box and reveal flags show.
static struct Pokemon *GetFoeDisplayMon(struct Pokemon *foeParty, u32 foeIndex)
{
    for (u32 i = 0; i < gBattlersCount; i++)
    {
        if (!IsOnPlayerSide(i) && GetBattlerTrainer(i) == B_TRAINER_OPPONENT_A
            && gBattlerPartyIndexes[i] == foeIndex
            && gBattleStruct->illusion[i].state == ILLUSION_ON)
        {
            struct Pokemon *disguise = GetIllusionMonPtr(i);
            if (disguise != NULL)
                return disguise;
        }
    }
    return &foeParty[foeIndex];
}

// FORK: append the species' type(s) as text ("Fire/Flying", or a single name for a
// pure type), via GetSpeciesType so FEATURE_NEW_TYPES re-typings are reflected.
static u8 *AppendTypeNames(u8 *p, enum Species species)
{
    enum Type t1 = GetSpeciesType(species, 0);
    enum Type t2 = GetSpeciesType(species, 1);

    p = StringCopy(p, gTypesInfo[t1].name);
    if (t2 != t1)
    {
        *p++ = CHAR_SLASH;
        p = StringCopy(p, gTypesInfo[t2].name);
    }
    return p;
}

// FORK: append the foe mon's gimmick state. activeGimmick persists per party mon for
// the forms that stay transformed (Mega/Ultra Burst/Dynamax/Tera); a Z-Move reverts
// and is detected via the per-mon monGimmickUsed record. Nothing is shown until the
// mon has actually used or entered a gimmick (always observed in battle).
static u8 *AppendFoeGimmickLabel(u8 *p, struct Pokemon *foeParty, u32 foeIndex)
{
    enum Gimmick gimmick = gBattleStruct->gimmick.activeGimmick[B_TRAINER_OPPONENT_A][foeIndex];

    switch (gimmick)
    {
    case GIMMICK_MEGA:        return StringCopy(p, COMPOUND_STRING("  Mega"));
    case GIMMICK_ULTRA_BURST: return StringCopy(p, COMPOUND_STRING("  Ultra Burst"));
    case GIMMICK_DYNAMAX:     return StringCopy(p, COMPOUND_STRING("  Dynamax"));
    case GIMMICK_TERA:
        p = StringCopy(p, COMPOUND_STRING("  Tera "));
        return StringCopy(p, gTypesInfo[GetMonData(&foeParty[foeIndex], MON_DATA_TERA_TYPE, NULL)].name);
    default:
        if (gBattleStruct->gimmick.monGimmickUsed[B_TRAINER_OPPONENT_A][foeIndex])
            return StringCopy(p, COMPOUND_STRING("  Z-Move"));
        return p;
    }
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

    p = StringCopy(line, COMPOUND_STRING("BATTLE INFO  -  FOE "));
    p = ConvertIntToDecimalStringN(p, foeIndex + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    *p++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(p, count, STR_CONV_MODE_LEFT_ALIGN, 1);
    PrintTitle(windowId, line);
    y += LINE_H;

    if (!seen)
    {
        PrintLine(windowId, COMPOUND_STRING("Not yet seen."), 0, y);
        PrintFooter(windowId, COMPOUND_STRING("<>: Mon  L/R: Page  B: Close"));
        return;
    }

    // Name, gender and level (known once the mon has appeared); flag if fainted.
    // Identity reads go through the display mon so an active Illusion shows the
    // disguise, not the real species; HP/FNT stays on the real slot (its health
    // box shows the real HP, and a fainting mon's Illusion has already broken).
    struct Pokemon *displayMon = GetFoeDisplayMon(foeParty, foeIndex);
    enum Species displaySpecies = GetMonData(displayMon, MON_DATA_SPECIES, NULL);
    u32 gender = GetMonGender(displayMon);
    p = StringCopy(line, GetSpeciesName(displaySpecies));
    if (gender == MON_MALE)
        *p++ = CHAR_MALE;
    else if (gender == MON_FEMALE)
        *p++ = CHAR_FEMALE;
    // FORK: Frontier levels are fixed (always 100/50), so show the mon's type(s) here
    // instead - far more useful at a glance - plus its gimmick state (Mega/Tera/etc.)
    // once used. Both are public the moment the mon is seen / transforms.
    *p++ = CHAR_SPACE;
    *p++ = CHAR_SPACE;
    p = AppendTypeNames(p, displaySpecies);
    p = AppendFoeGimmickLabel(p, foeParty, foeIndex);
    if (GetMonData(&foeParty[foeIndex], MON_DATA_HP, NULL) == 0)
        StringCopy(p, COMPOUND_STRING("  FNT"));
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // Ability — only once genuinely revealed in battle (gAiPartyData pre-knows it
    // under AI_FLAG_OMNISCIENT, so gate on our own reveal flag instead). The value
    // is read from our own reveal-time snapshot (infoRevealedAbility), not from
    // gAiPartyData->ability: the latter is later clobbered by the AI's speculative
    // switch/move evaluation, which would otherwise display the wrong ability.
    //
    // FORK: FEATURE_INNATE_ABILITIES — the foe's innate abilities (always-active
    // passives) share this single line instead of a separate "Innate:" row. The
    // dedicated row pushed every line below it down by LINE_H, sliding the bottom
    // Moves slot under the navigation bar; folding the innates into a parenthetical
    // here (mirroring the space-separated hazard list on the Field page) keeps the
    // page within its fixed height. The chosen ability is listed plainly; innates
    // follow in "(+Name, ...)" with a leading '+' to mark them as additional
    // passives. The chosen ability and each innate are revealed *independently* —
    // an innate is its own passive, so seeing one (e.g. an innate Levitate block a
    // Ground move) does NOT reveal the chosen ability, which keeps showing "?" until
    // it too is witnessed (so the line reads "? (+Levitate)"). Each tracks its own
    // reveal bit, so the viewer shows only what has actually been seen in action; a
    // silent passive innate (Regenerator, Unaware, ...) is never recorded and so
    // never leaks here.
    bool32 abilitySeen = (gBattleStruct->infoAbilityRevealed[B_SIDE_OPPONENT] & (1u << foeIndex)) != 0;
    enum Ability seenAbility = gBattleStruct->infoRevealedAbility[B_SIDE_OPPONENT][foeIndex];
    p = StringCopy(line, COMPOUND_STRING("Ability: "));
    if (abilitySeen && seenAbility != ABILITY_NONE)
        p = StringCopy(p, gAbilitiesInfo[seenAbility].name);
    else
        p = StringCopy(p, COMPOUND_STRING("?"));
    if (GetConfig(FEATURE_INNATE_ABILITIES))
    {
        enum Species foeSpecies = GetMonData(&foeParty[foeIndex], MON_DATA_SPECIES, NULL);
        u32 revealedInnates = gBattleStruct->infoRevealedInnates[B_SIDE_OPPONENT][foeIndex];
        bool32 anyInnate = FALSE;
        for (u32 slot = 0; ; slot++)
        {
            enum Ability innate = GetSpeciesInnate(foeSpecies, slot);
            if (innate == ABILITY_NONE)
                break;
            // Only innates the player has individually witnessed in battle.
            if (!(revealedInnates & (1u << slot)))
                continue;
            // Skip an innate that just duplicates the revealed chosen ability
            // (e.g. a species that still carries Levitate as its primary), so the
            // line doesn't echo the same name twice.
            if (abilitySeen && innate == seenAbility)
                continue;
            p = StringCopy(p, anyInnate ? COMPOUND_STRING(", ") : COMPOUND_STRING(" (+"));
            p = StringCopy(p, gAbilitiesInfo[innate].name);
            anyInnate = TRUE;
        }
        if (anyInnate)
            p = StringCopy(p, COMPOUND_STRING(")"));
    }
    PrintLine(windowId, line, 0, y);
    y += LINE_H;

    // Held item — only once its situation has been genuinely revealed in battle.
    // The reveal bit is set both when an item's effect activates *and* when the
    // item is removed in view of the player (Knock Off/Thief/Trick all route
    // through StealTargetItem -> RecordItemEffectBattle, and a consumed berry
    // records its effect as it triggers). So once revealed, an empty held-item
    // slot means the player saw it leave -> show "None", not "?".
    enum Item heldItem = GetMonData(&foeParty[foeIndex], MON_DATA_HELD_ITEM, NULL);
    bool32 itemSeen = (gBattleStruct->infoItemRevealed[B_SIDE_OPPONENT] & (1u << foeIndex)) != 0;
    p = StringCopy(line, COMPOUND_STRING("Item: "));
    if (!itemSeen)
        StringCopy(p, COMPOUND_STRING("?"));
    else if (heldItem != ITEM_NONE)
        StringCopy(p, GetItemName(heldItem));
    else
        StringCopy(p, COMPOUND_STRING("None"));
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

    PrintFooter(windowId, COMPOUND_STRING("<>: Mon  L/R: Page  B: Close"));
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

    PrintTitle(windowId, COMPOUND_STRING("BATTLE INFO  -  CONDITIONS"));
    y += LINE_H;

    y = DrawConditionsForSide(windowId, TRUE, y);
    DrawConditionsForSide(windowId, FALSE, y);

    PrintFooter(windowId, COMPOUND_STRING("L/R: Page    B: Close"));
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

    PrintTitle(windowId, COMPOUND_STRING("BATTLE INFO  -  STAT CHANGES"));
    y += LINE_H;

    y = DrawStatsForSide(windowId, TRUE, y);
    DrawStatsForSide(windowId, FALSE, y);

    PrintFooter(windowId, COMPOUND_STRING("L/R: Page    B: Close"));
}

// FORK: Speed tier report. From base stats + level alone, a foe's *possible*
// Speed stat spans a known range — the player needn't know the exact EV/IV
// investment or nature to bound it. Lowest = 0 IVs, 0 EVs, a hindering nature
// (x0.9); highest = 31 IVs, 252 EVs, a boosting nature (x1.1). The formula
// mirrors CalculateMonStats() in src/pokemon.c. This is the *raw* stat range:
// it deliberately ignores in-battle modifiers not derivable from base stats
// (Choice Scarf, paralysis, Tailwind, stat stages, Speed-changing abilities).
// The player's own active mons' actual Speed is shown alongside for comparison.
static u32 CalcSpeedBound(u32 baseSpeed, u32 level, u32 iv, u32 ev, u32 natureNum)
{
    u32 n = (((2 * baseSpeed + iv + ev / 4) * level) / 100) + 5;
    // natureNum is 90 (hindering), 100 (neutral) or 110 (boosting); see
    // ModifyStatByNature(), which applies the factor as `stat * num / 100`.
    return n * natureNum / 100;
}

// FORK: append a speed-comparison glyph to a foe row. The reference is the
// player's fastest active mon's *effective* Speed (GetBattlerTotalSpeedStat,
// which already folds in the player's own Choice Scarf, Tailwind, paralysis and
// stat stages - all known to the player about their own side). The foe range is
// the raw base-stat span only; it deliberately ignores the foe's hidden item and
// ability, so this is purely a number comparison (your known Speed vs the foe's
// *possible* base Speed), NOT a guarantee of turn order. A down arrow means your
// Speed already exceeds the foe's best-case base Speed; an up arrow that even the
// foe's worst-case base Speed exceeds yours; no arrow that it depends on the
// foe's unseen investment. Colour-free on purpose (the arrows read for everyone).
static u8 *AppendSpeedGlyph(u8 *p, bool32 haveRef, u32 ref, u32 lo, u32 hi)
{
    if (haveRef && hi < ref)
    {
        *p++ = CHAR_SPACE;
        *p++ = CHAR_DOWN_ARROW;
    }
    else if (haveRef && lo > ref)
    {
        *p++ = CHAR_SPACE;
        *p++ = CHAR_UP_ARROW;
    }
    *p = EOS;
    return p;
}

static void DrawSpeedPage(u8 windowId)
{
    u8 line[64];
    u8 *p;
    u32 y = 0;
    struct Pokemon *foeParty = GetTrainerParty(B_TRAINER_OPPONENT_A);
    u32 slots[PARTY_SIZE];
    u32 los[PARTY_SIZE];
    u32 his[PARTY_SIZE];
    u32 n = 0;
    u32 ref = 0;
    bool32 haveRef = FALSE;

    PrintTitle(windowId, COMPOUND_STRING("BATTLE INFO  -  SPEED TIERS"));
    y += LINE_H;

    // The player's active mons: their actual *effective* Speed - everything the
    // player already knows about their own side (Choice Scarf, Tailwind,
    // paralysis, stat stages) is folded in, so the foe ranges read against a
    // concrete reference. The fastest such mon anchors the comparison arrows.
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        u32 spe;

        if (!IsOnPlayerSide(battler) || !IsBattlerAlive(battler))
            continue;

        spe = GetBattlerTotalSpeedStat(battler, GetBattlerAbility(battler), GetBattlerHoldEffect(battler));
        if (!haveRef || spe > ref)
        {
            ref = spe;
            haveRef = TRUE;
        }

        p = StringCopy(line, COMPOUND_STRING("You: "));
        p = StringCopy(p, gBattleMons[battler].nickname);
        p = StringCopy(p, COMPOUND_STRING("  Spe "));
        ConvertIntToDecimalStringN(p, spe, STR_CONV_MODE_LEFT_ALIGN, 4);
        PrintLine(windowId, line, 0, y);
        y += LINE_H;
    }

    // Collect only the foe slots the player has actually seen sent out (same
    // reveal gate as the Foe page; unseen slots are omitted entirely, so the
    // report never leaks an unrevealed mon - or even the foe's party size), then
    // sort by the top of each possible Speed range so the list reads as a speed
    // tier (fastest first). The "Foe N" label keeps the true party slot.
    for (u32 i = 0; i < PARTY_SIZE; i++)
    {
        enum Species species = GetMonData(&foeParty[i], MON_DATA_SPECIES, NULL);
        struct Pokemon *displayMon;
        u32 level, baseSpeed;

        if (species == SPECIES_NONE || GetMonData(&foeParty[i], MON_DATA_IS_EGG, NULL))
            continue;
        if (!gBattleStruct->partyState[B_TRAINER_OPPONENT_A][i].sentOut)
            continue;

        // Use the display mon so an active Illusion reports the disguise's Speed
        // tier (what the player believes they face), not the real mon's.
        displayMon = GetFoeDisplayMon(foeParty, i);
        level = GetMonData(displayMon, MON_DATA_LEVEL, NULL);
        baseSpeed = GetSpeciesBaseSpeed(GetMonData(displayMon, MON_DATA_SPECIES, NULL));

        slots[n] = i;
        los[n] = CalcSpeedBound(baseSpeed, level, 0, 0, 90);
        his[n] = CalcSpeedBound(baseSpeed, level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS, 110);
        n++;
    }

    // Selection sort by descending high bound (n <= PARTY_SIZE, so trivial).
    for (u32 a = 0; a < n; a++)
    {
        u32 best = a;

        for (u32 b = a + 1; b < n; b++)
        {
            if (his[b] > his[best])
                best = b;
        }
        if (best != a)
        {
            u32 t;

            t = slots[a]; slots[a] = slots[best]; slots[best] = t;
            t = los[a];   los[a]   = los[best];   los[best]   = t;
            t = his[a];   his[a]   = his[best];   his[best]   = t;
        }
    }

    if (n == 0)
    {
        PrintLine(windowId, COMPOUND_STRING("No foes seen yet."), 0, y);
    }
    else
    {
        for (u32 k = 0; k < n; k++)
        {
            struct Pokemon *displayMon = GetFoeDisplayMon(foeParty, slots[k]);

            p = StringCopy(line, COMPOUND_STRING("Foe "));
            p = ConvertIntToDecimalStringN(p, slots[k] + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            p = StringCopy(p, COMPOUND_STRING(": "));
            p = StringCopy(p, GetSpeciesName(GetMonData(displayMon, MON_DATA_SPECIES, NULL)));
            *p++ = CHAR_SPACE;
            p = ConvertIntToDecimalStringN(p, los[k], STR_CONV_MODE_LEFT_ALIGN, 3);
            *p++ = CHAR_HYPHEN;
            p = ConvertIntToDecimalStringN(p, his[k], STR_CONV_MODE_LEFT_ALIGN, 3);
            AppendSpeedGlyph(p, haveRef, ref, los[k], his[k]);
            PrintLine(windowId, line, 0, y);
            y += LINE_H;
        }
    }

    PrintFooter(windowId, COMPOUND_STRING("L/R: Page    B: Close"));
}

static void RedrawInfo(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    // Transparent fill so the backdrop colour shows through behind the text.
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    switch (gTasks[taskId].tPage)
    {
    case INFO_PAGE_CONDITIONS:
        DrawConditionsPage(windowId);
        break;
    case INFO_PAGE_STATS:
        DrawStatsPage(windowId);
        break;
    case INFO_PAGE_SPEED:
        DrawSpeedPage(windowId);
        break;
    case INFO_PAGE_FOE:
        DrawFoePage(windowId, gTasks[taskId].tFoeIndex);
        break;
    case INFO_PAGE_FIELD:
    default:
        DrawFieldPage(windowId);
        break;
    }
    PrintPageIndicator(windowId, gTasks[taskId].tPage);
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
        // FRAME_BG's tilemap buffer is ours (not a window buffer), so free it here.
        TRY_FREE_AND_SET_NULL(sFrameTilemapBuffer);
        SetMainCallback2(sInfoExitCallback != NULL ? sInfoExitCallback : ReshowBattleScreenAfterMenu);
    }
}

static void Task_InfoProcessInput(u8 taskId)
{
    u32 count = GetFoePartyCount(GetTrainerParty(B_TRAINER_OPPONENT_A));

    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        // Remember where the player was for the next open (the viewer is modal,
        // so it can't be reopened before this runs).
        sLastPage = gTasks[taskId].tPage;
        sLastFoeIndex = gTasks[taskId].tFoeIndex;
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

// FORK: open the read-only viewer, returning to returnCallback when the player
// closes it. Lets the same viewer be reached from the battle action menu (return
// to the battle screen) and the in-battle party menu (return to the party menu).
void OpenFrontierBattleInfo(MainCallback returnCallback)
{
    sInfoExitCallback = returnCallback;
    SetMainCallback2(CB2_FrontierBattleInfo);
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
        ShowBg(FRAME_BG);
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
    {
        const struct TilesPal *frame = GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType);

        LoadPalette(sBgColor, BG_PLTT_ID(0), sizeof(sBgColor));
        // gStandardMenuPalette carries the conventional TEXT_COLOR_* colours, so
        // PrintTitle/PrintFooter can colour text without a bespoke palette.
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        // The player's chosen window frame (from Options), reused as our border.
        LoadBgTiles(FRAME_BG, frame->tiles, 0x120, FRAME_BASE_TILE);
        LoadPalette(frame->pal, BG_PLTT_ID(FRAME_PAL_NUM), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    }
    case 4:
        taskId = CreateTask(Task_InfoFadeIn, 0);
        gTasks[taskId].tWindowId = AddWindow(&sInfoWindowTemplate);
        // Resume on the page/foe the player last viewed (defaults to Speed Tiers).
        gTasks[taskId].tPage = sLastPage;
        gTasks[taskId].tFoeIndex = sLastFoeIndex;
        PutWindowTilemap(gTasks[taskId].tWindowId);
        // FRAME_BG has no window, so give it its own tilemap buffer (heap, like
        // AddWindow does for the text window) and draw the border ring into it.
        sFrameTilemapBuffer = AllocZeroed(BG_SCREEN_SIZE);
        SetBgTilemapBuffer(FRAME_BG, sFrameTilemapBuffer);
        DrawInfoFrame();
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
