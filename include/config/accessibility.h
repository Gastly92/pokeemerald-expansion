#ifndef GUARD_CONFIG_ACCESSIBILITY_H
#define GUARD_CONFIG_ACCESSIBILITY_H

// FORK: fork-owned config file for accessibility options. These flags make the
// game easier to play for users with specific needs; FALSE = stock
// pokeemerald-expansion behavior. They are opt-in (default FALSE) because they
// change the look/feel for every player, not just balance.

// When TRUE, recolors the in-battle HP bar's healthy (>50%) band from green to
// the EXP bar's blue. The stock green -> yellow -> red ramp is hard to read for
// the most common (red-green) color blindness, where green and red are the
// confusable extremes; blue is distinguishable across all types, so the result
// is blue (>50%) -> yellow (>20%) -> red (<=20%), which separates much better.
//
// Implemented as a 2-entry palette swap on TAG_HEALTHBAR_PAL in
// ApplyHealthbarColorBlindPalette() (src/battle_gfx_sfx_util.c): the bar fill is
// a separate "healthbar" sprite whose palette holds the two green shades at
// entries 10-11 (yellow at 12-13, red at 14-15), so only the green band is
// touched. No new graphics. Battle bar only for now; the party-menu/summary HP
// bars use a different mechanism and still follow the stock colors.
#define COLOR_BLIND FALSE

#endif // GUARD_CONFIG_ACCESSIBILITY_H
