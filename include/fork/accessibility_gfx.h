#ifndef GUARD_FORK_ACCESSIBILITY_GFX_H
#define GUARD_FORK_ACCESSIBILITY_GFX_H

// FORK: accessibility-driven battle graphics tweaks (config/accessibility.h).
// Each entry point is a no-op when its flag is off, so the hook site in the
// upstream file is a single unconditional call with no #if around it.

// FORK: COLOR_BLIND — recolour the HP bar's healthy (>50%) band from green to the
// EXP bar's blue. Call once per battle, right after the healthbox/healthbar sprite
// palettes are loaded (BattleLoadAllHealthBoxesGfx, src/battle_gfx_sfx_util.c).
void ApplyHealthbarColorBlindPalette(void);

#endif // GUARD_FORK_ACCESSIBILITY_GFX_H
