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
"Lv" glyph and the level prints next to it as `100`, 15px wide, starting right where the icon
ends. The icon is pinned at that level-100 position and never moves.

A reading that fills those 15px touches the icon whenever its first digit's left column is filled
(a `9` does; the `1` of `100` happens not to). So the digits print in `FONT_SMALL_NARROWER` (4px
each instead of 5; same height and baseline) and the `%` in `FONT_SMALL_NARROW` (5px instead of
7). The widest reading, `99%`, is then 13px, leaving a 2px gap. The `%` glyph itself is the same
in every small font. A taller custom `%` was tried and rejected: at 4px wide it reads as a `Z` and
crowds the digits.

`100%` would not fit at all, which is one more reason full HP shows nothing. The width test in
`test/fork/healthbox_hp_percent.c` fails if a font change ever closes the gap.

## Upstream's `B_HP_PERCENTAGE_DISPLAY`

Upstream has its own foe-only HP % (`include/config/battle.h`), which makes the singles foe box
taller and, in doubles, only shows the % after pressing START. It is left **off**: it rounds down
(so it could disagree with the Foe page), it keeps the level, and it does not build with
`B_CLEAN_HEALTHBOX`, which has no tail-less version of its taller box art.
