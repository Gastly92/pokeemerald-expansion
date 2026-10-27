#include "global.h"
#include "test/battle.h"
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
    // With a gimmick icon at the box's right edge, the reading sits between the name field and
    // the icon, 2px clear of the icon; that leaves 2px to the name field at 13px. (Before the icon
    // moved, the same 13px limit kept a 2px gap on the icon's other side.)
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

// The icon is pinned to the healthbox's right edge by a hook in upstream's SpriteCb_GimmickIndicator,
// and the Dynamax icon sits 1px higher. If upstream moves the indicator's positioning, this fails.
SINGLE_BATTLE_TEST("Healthbox HP %: the gimmick icon sits at the box's right edge, Dynamax 1px higher")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_SCRATCH); }
        OPPONENT(SPECIES_GARDEVOIR) { Item(ITEM_GARDEVOIRITE); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH, gimmick: GIMMICK_DYNAMAX); MOVE(opponent, MOVE_CELEBRATE, gimmick: GIMMICK_MEGA); }
    } THEN {
        for (enum BattlerId battler = B_BATTLER_0; battler <= B_BATTLER_1; battler++)
        {
            struct Sprite *icon = &gSprites[gBattleStruct->gimmick.indicatorSpriteId[battler]];
            struct Sprite *healthbox = &gSprites[gHealthboxSpriteIds[battler]];
            // The level slot's right edge, relative to the healthbox's main sprite x.
            s32 slotRight = (battler == B_BATTLER_0) ? 64 : 56;

            icon->callback(icon);
            EXPECT_EQ(icon->x + icon->centerToCornerVecX + 8, healthbox->x + slotRight);
            EXPECT_EQ(icon->y2, healthbox->y2 - (battler == B_BATTLER_0 ? 1 : 0));
        }
    }
}
