#include "global.h"
#include "test/test.h"
#include "string_util.h"
#include "text.h"
#include "fork/healthbox_hp_percent.h"

// B_HEALTHBOX_HP_PERCENT. The healthbox's level slot shows the HP %: empty at full HP, rounded
// up below it (matching the INFO viewer's Foe page), and never wider than the "100" it replaces,
// so a Mega/Tera/Dynamax icon beside it never has to move.

TEST("Healthbox HP %: empty at full HP, rounded up below it, 0% when fainted")
{
    u8 buf[8];

    BuildHealthboxHpPercentText(buf, 200, 200);
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("")), 0);
    BuildHealthboxHpPercentText(buf, 199, 200); // 99.5% would round up to 100; capped below full
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("99%")), 0);
    BuildHealthboxHpPercentText(buf, 101, 200); // 50.5% reads 51, as on the Foe page
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("51%")), 0);
    BuildHealthboxHpPercentText(buf, 100, 200);
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("50%")), 0);
    BuildHealthboxHpPercentText(buf, 1, 200); // a sliver left never reads 0
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("1%")), 0);
    BuildHealthboxHpPercentText(buf, 0, 200);
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("0%")), 0);
    BuildHealthboxHpPercentText(buf, 1, 1); // Shedinja: full until it faints
    EXPECT_EQ(StringCompare(buf, COMPOUND_STRING("")), 0);
}

TEST("Healthbox HP %: every reading leaves a gap before the gimmick icon")
{
    u8 buf[8];
    // The icon sits right where "100" starts (it replaces the "Lv" glyph), so a reading as wide
    // as "100" touches it whenever its first digit's left column is filled. Require 2px of room.
    u32 limit = GetStringWidth(FONT_SMALL, COMPOUND_STRING("100"), 0) - 2;
    u32 widest = 0;

    for (s32 hp = 0; hp <= 300; hp++)
    {
        u32 width;
        BuildHealthboxHpPercentText(buf, hp, 300);
        width = GetHealthboxHpPercentTextWidth(buf);
        if (width > widest)
            widest = width;
    }
    EXPECT_LE(widest, limit);
}
