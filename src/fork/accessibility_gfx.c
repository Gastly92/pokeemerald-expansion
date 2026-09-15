#include "global.h"
#include "battle.h"           // battle_interface.h's prerequisites
#include "battle_interface.h" // TAG_HEALTHBAR_PAL
#include "palette.h"
#include "sprite.h"
#include "constants/rgb.h"
#include "config/accessibility.h"
#include "fork/accessibility_gfx.h"

// FORK: When COLOR_BLIND is on, recolor the HP bar's healthy (>50%) band from
// green to the EXP bar's blue. The bar fill is a separate "healthbar" sprite
// using TAG_HEALTHBAR_PAL, whose palette holds the two green shades at entries
// 10-11 (yellow at 12-13, red at 14-15), so swapping just those two entries
// turns the green band blue and leaves the yellow/red bands untouched. See the
// COLOR_BLIND comment in include/config/accessibility.h.
//
// Lives here rather than beside its call site in src/battle_gfx_sfx_util.c: it
// sat directly after upstream's unused BattleLoadAllHealthBoxesGfxAtOnce(), and
// when the 1.17.0 sync deleted that function git pulled this one into a 43-line
// conflict against nothing. As a fork file with a one-line call at the hook
// point, that deletion auto-merges.
void ApplyHealthbarColorBlindPalette(void)
{
#if COLOR_BLIND
    static const u16 sHpBarBlue[] = { RGB(13, 27, 31), RGB(8, 25, 31) }; // light + main EXP-style blue
    u32 palIndex = IndexOfSpritePaletteTag(TAG_HEALTHBAR_PAL);

    if (palIndex != 0xFF)
        LoadPalette(sHpBarBlue, OBJ_PLTT_ID(palIndex) + 10, sizeof(sHpBarBlue));
#endif
}
