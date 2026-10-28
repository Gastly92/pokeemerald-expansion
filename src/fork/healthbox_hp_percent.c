#include "global.h"
#include "battle.h"
#include "battle_gimmick.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "fork/frontier_battle_info.h"
#include "fork/healthbox_hp_percent.h"
#include "constants/characters.h"

void BuildHealthboxHpPercentText(u8 *dst, s32 currHp, s32 maxHp)
{
    u32 percent;

    if (maxHp > 0 && currHp >= maxHp)
    {
        // A full bar already says "untouched", so the slot stays empty until the first hit.
        *dst = EOS;
        return;
    }

    percent = HpPercentRoundedUp(currHp > 0 ? currHp : 0, maxHp > 0 ? maxHp : 0);
    dst = ConvertIntToDecimalStringN(dst, percent, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dst++ = CHAR_PERCENT;
    *dst = EOS;
}

// The digits print in FONT_SMALL_NARROWER (4px each against FONT_SMALL's 5, same height and
// baseline) and the '%' in FONT_SMALL_NARROW (5px against 7), so "99%" is 13px: 2px narrower than
// the "100" a gimmick icon sits beside, which leaves a visible gap to the icon. With the digits at
// full width, a digit whose left column is filled (a 9, say) touched it.
#define DIGIT_FONT   FONT_SMALL_NARROWER
#define PERCENT_FONT FONT_SMALL_NARROW

// Splits "91%" into its digits and its '%' (the last character). An empty text has neither.
static const u8 *SplitPercentText(u8 *digits, const u8 *text)
{
    u32 len = StringLength(text);

    if (len == 0)
    {
        *digits = EOS;
        return text;
    }
    StringCopyN(digits, text, len - 1);
    digits[len - 1] = EOS;
    return &text[len - 1];
}

u32 GetHealthboxHpPercentTextWidth(const u8 *text)
{
    u8 digits[4];
    const u8 *percent = SplitPercentText(digits, text);

    return GetStringWidth(DIGIT_FONT, digits, 0) + GetStringWidth(PERCENT_FONT, percent, 0);
}

// The level slot's right edge, in the healthbox's second sprite (UpdateLvlInHealthbox's numbers).
static u32 GetSlotRight(enum BattlerId battler)
{
    return IsOnPlayerSide(battler) ? 32 : 24;
}

// SpriteCB_HealthBoxOther keeps the second sprite 64px right of the main one, and both are 64px
// wide and positioned by their centre, so the second sprite's left edge is 32px right of the
// main sprite's x.
#define SECOND_SPRITE_LEFT 32

// The gimmick icon is 8px wide; the HP % keeps 2px clear of it.
#define INDICATOR_WIDTH 8
#define INDICATOR_GAP   2

// The % prints at the level's y in UpdateLvlInHealthbox, so its digits share the name's baseline.
#define TEXT_Y 3

void ApplyHealthboxIndicatorPosition(struct Sprite *sprite, enum BattlerId battler)
{
    s32 left = gSprites[gHealthboxSpriteIds[battler]].x + SECOND_SPRITE_LEFT + GetSlotRight(battler) - INDICATOR_WIDTH;

    // Flush with the slot's right edge. The height stays upstream's for every icon: nudging the
    // Dynamax icon 1px either way was tried in play and looked worse.
    sprite->x = left - sprite->centerToCornerVecX;
}

void PrintHealthboxHpPercent(u32 healthboxSpriteId, enum BattlerId battler, s32 currHp, s32 maxHp, const union TextColor textColor, u32 bgColor)
{
    u8 text[4], digits[4];
    const u8 *percent;
    // The healthbox's second sprite holds the level slot, as in UpdateLvlInHealthbox.
    u32 spriteId = gSprites[healthboxSpriteId].oam.affineParam;
    // Same rectangle as UpdateLvlInHealthbox, so the % replaces the level exactly. It is
    // right-aligned to the slot's edge, or just left of the gimmick icon when one is showing.
    u32 left = IsOnPlayerSide(battler) ? 8 : 0;
    u32 right = GetSlotRight(battler);
    u32 x;

    if (GetIndicatorPalTag(battler) != TAG_NONE)
        right -= INDICATOR_WIDTH + INDICATOR_GAP;

    BuildHealthboxHpPercentText(text, currHp, maxHp);
    FillSpriteRectColor(spriteId, left, 5, 24, 11, bgColor);
    if (text[0] == EOS)
        return;

    percent = SplitPercentText(digits, text);
    x = right - GetHealthboxHpPercentTextWidth(text);
    AddSpriteTextPrinterParameterized6(spriteId, DIGIT_FONT, x, TEXT_Y, 0, 0, textColor, 0, digits);
    x += GetStringWidth(DIGIT_FONT, digits, 0);
    AddSpriteTextPrinterParameterized6(spriteId, PERCENT_FONT, x, TEXT_Y, 0, 0, textColor, 0, percent);
}
