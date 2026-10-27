# Healthbox HP % (`B_HEALTHBOX_HP_PERCENT`)

Every mon in this fork is level 100, so the `Lv100` on each battle healthbox tells the player
nothing. This flag puts the mon's HP as a percentage in that slot instead, on all four boxes,
yours and the foe's, in singles and doubles.

- **Flag:** `B_HEALTHBOX_HP_PERCENT` (`include/config/fork.h`, compile-time). `FALSE` restores the level.
- **Code:** `src/fork/healthbox_hp_percent.c`, hooked from `UpdateLvlInHealthbox` and
  `UpdateHpTextInHealthbox` in `src/battle_interface.c`
- **Test:** `test/fork/healthbox_hp_percent.c`

## What it shows

| HP | Slot |
|---|---|
| Full | *(empty)* |
| Below full | `1%` to `99%`, rounded **up** |
| Fainted | `0%` |

**Empty at full HP.** A full bar already says "untouched", so a number appearing is itself the
signal that a mon has been hit. Because the rounding caps at 99 below full, an empty slot always
means exactly full HP, even when the bar looks full at 98%.

**Rounded up, like the INFO viewer's Foe page.** Both use `HpPercentRoundedUp`
(`src/fork/frontier_battle_info.c`), so the box and the Foe page can never disagree, and both read
safely against the [damage preview](DAMAGE_PREVIEW.md)'s rounded-down range: "damage ≥ HP" on
screen is always a real KO.

**Counts with the bar.** The bar's drain animation redraws the HP text through
`UpdateHpTextInHealthbox` with the animated value every frame; the % is redrawn from the same call,
so it ticks down with the bar rather than jumping when the bar stops.

## Fitting beside a gimmick icon

The Mega, Primal, Tera and Dynamax icons share the level slot: the icon takes the place of the
"Lv" glyph and the level prints next to it as `100`, 15px wide. The % is printed in
`FONT_SMALL_NARROW`, whose `%` is 5px instead of 7px, so the widest reading, `99%`, is exactly
15px too. The icon is pinned at its level-100 position and never moves. `100%` would not fit
(20px), which is one more reason full HP shows nothing. The second test in the test file fails
if a font change ever makes a reading wider than `100`.

## Upstream's `B_HP_PERCENTAGE_DISPLAY`

Upstream has its own foe-only HP % (`include/config/battle.h`), which makes the singles foe box
taller and, in doubles, only shows the % after pressing START. It is left **off**: it rounds down
(so it could disagree with the Foe page), it keeps the level, and it does not build with
`B_CLEAN_HEALTHBOX`, which has no tail-less version of its taller box art.
