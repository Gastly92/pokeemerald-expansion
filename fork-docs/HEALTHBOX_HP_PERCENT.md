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

The Mega, Primal, Tera and Dynamax icons share the slot. With this flag the icon sits at the
slot's **right edge**, and the HP % sits between it and the name, 2px clear of the icon. Without an
icon the % is right-aligned to the edge, so it shifts left once when a mon Mega Evolves,
Terastallizes or Dynamaxes (each of those redraws the whole healthbox, which reprints the %).
The % prints at the level's height, so its digits share the name's baseline. A reading with a `3`
can still look a pixel high at a glance, because the `3`'s bottom row has no dark pixels; printing
the % 1px lower was tried and looked worse overall.

The icon's position is set every frame in upstream's `SpriteCb_GimmickIndicator`
(`src/battle_gimmick.c`), through a `FORK:` hook calling `ApplyHealthboxIndicatorPosition`. That
also lowers the **Dynamax** icon 1px from upstream's spot, again tuned by eye.

The digits print in `FONT_SMALL_NARROWER` (4px each instead of 5; same height and baseline) and
the `%` in `FONT_SMALL_NARROW` (5px instead of 7), so the widest reading, `99%`, is 13px. That
fits between the longest name field and the icon with 2px to spare on each side. The `%` glyph
itself is the same in every small font. A taller custom `%` was tried and rejected: at 4px wide it
reads as a `Z` and crowds the digits.

`100%` would not fit, which is one more reason full HP shows nothing. Tests in
`test/fork/healthbox_hp_percent.c` fail if a font change closes the gaps or if upstream moves the
icon's positioning out from under the hook.

## Upstream's `B_HP_PERCENTAGE_DISPLAY`

Upstream has its own foe-only HP % (`include/config/battle.h`), which makes the singles foe box
taller and, in doubles, only shows the % after pressing START. It is left **off**: it rounds down
(so it could disagree with the Foe page), it keeps the level, and it does not build with
`B_CLEAN_HEALTHBOX`, which has no tail-less version of its taller box art.
