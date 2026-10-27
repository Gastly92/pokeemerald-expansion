#ifndef GUARD_HEALTHBOX_HP_PERCENT_H
#define GUARD_HEALTHBOX_HP_PERCENT_H

#include "text.h"
#include "constants/battle.h"

// B_HEALTHBOX_HP_PERCENT (config/fork.h): every battle healthbox shows its mon's HP as a % in
// the slot that used to hold "Lv100". See fork-docs/HEALTHBOX_HP_PERCENT.md.

// Writes the slot's text into dst (at least 4 bytes): "" at full HP, "1%".."99%" below full
// (rounded up like the INFO viewer's Foe page), "0%" when fainted. Never wider than a 3-digit
// level as printed (see GetHealthboxHpPercentTextWidth), which is what lets the gimmick icon keep
// its usual spot. Exposed for test/fork/healthbox_hp_percent.c.
void BuildHealthboxHpPercentText(u8 *dst, s32 currHp, s32 maxHp);

// That text's printed width in pixels: digits in FONT_SMALL_NARROWER, the '%' in FONT_SMALL_NARROW.
// Exposed for the width test in test/fork/healthbox_hp_percent.c.
u32 GetHealthboxHpPercentTextWidth(const u8 *text);

// Redraws the slot on one healthbox. textColor and bgColor are the box's own (battle_interface.c's
// sHealthBoxTextColor / HEALTHBOX_BG_INDEX), passed in so they stay defined in one place.
void PrintHealthboxHpPercent(u32 healthboxSpriteId, enum BattlerId battler, s32 currHp, s32 maxHp, const union TextColor textColor, u32 bgColor);

#endif // GUARD_HEALTHBOX_HP_PERCENT_H
