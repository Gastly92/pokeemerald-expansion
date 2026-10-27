#include "global.h"
#include "battle.h"
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

void PrintHealthboxHpPercent(u32 healthboxSpriteId, enum BattlerId battler, s32 currHp, s32 maxHp, const union TextColor textColor, u32 bgColor)
{
    u8 text[4];
    // The healthbox's second sprite holds the level slot, as in UpdateLvlInHealthbox.
    u32 spriteId = gSprites[healthboxSpriteId].oam.affineParam;
    u32 width;

    BuildHealthboxHpPercentText(text, currHp, maxHp);
    // FONT_SMALL_NARROW's '%' is 2px narrower than FONT_SMALL's, which makes "99%" exactly as
    // wide as "100" (15px): the Mega/Tera/Dynamax icon sits where it does for a level-100 mon.
    width = GetStringWidth(FONT_SMALL_NARROW, text, 0);

    // Same rectangle and right edge as UpdateLvlInHealthbox, so the % replaces the level exactly.
    if (IsOnPlayerSide(battler))
    {
        FillSpriteRectColor(spriteId, 8, 5, 24, 11, bgColor);
        if (width != 0)
            AddSpriteTextPrinterParameterized6(spriteId, FONT_SMALL_NARROW, 32 - width, 3, 0, 0, textColor, 0, text);
    }
    else
    {
        FillSpriteRectColor(spriteId, 0, 5, 24, 11, bgColor);
        if (width != 0)
            AddSpriteTextPrinterParameterized6(spriteId, FONT_SMALL_NARROW, 24 - width, 3, 0, 0, textColor, 0, text);
    }
}
