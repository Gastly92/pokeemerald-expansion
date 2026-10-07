# Move damage preview (`B_MOVE_DAMAGE_PREVIEW`)

When picking a move, the type row under the PP counter shows how much of the foe's HP the
selected move could take:

```
PP ◎   10/10
Ground 46-61%
```

- **Flag:** `B_MOVE_DAMAGE_PREVIEW` (`include/config/fork.h`, compile-time)
- **Code:** `src/fork/damage_preview.c` (`GetDamagePreviewRange`, `TryPrintMoveDamagePreview`,
  `TryPrintZMoveDamagePreview`), hooked from `MoveSelectionDisplayMoveType` in
  `src/battle_controller_player.c` and, for the Z-Move view, `ZMoveSelectionDisplayMoveType` in
  `src/battle_z_move.c`
- **Test:** `test/fork/damage_preview.c`

## What the range spans

The player does not know the foe's spread, so the range covers every legal one at the foe's
(visible) level:

| End | Foe's HP and defences | Damage roll |
|---|---|---|
| **Low** (bulkiest) | 31 IV / 252 EV / boosting nature | lowest |
| **High** (frailest) | 0 IV / 0 EV / hindering nature | highest |

The foe's *offensive* stats and Speed are pushed the other way in each case, so moves that
read the target's own stats (Foul Play, Gyro Ball) are bounded too. The health bar's current
fraction is kept, for moves that read the target's current HP. Under `DETERMINISTIC_DAMAGE`
the roll is the fixed turn multiplier, so only the spread widens the range.

Everything the player **does** know is used as normal: your own mon's stats, ability, item
and stat stages, the foe's stat stages, types (Tera included), weather, terrain, screens, and
an armed Z-Move / Dynamax / Tera. Multi-hit moves count their hits.

## What it hides

Built on the AI's damage simulation (`AI_CalcDamage`), with the foe rewritten into what the
player can know and then restored byte-for-byte:

- **Unrevealed ability / held item → treated as none.** "Revealed" is the INFO viewer's
  reveal bits (see [`BATTLE_INFO.md`](BATTLE_INFO.md#reveal-gating--the-part-that-keeps-breaking)),
  which are tracked in every battle. So a hidden Assault Vest or Multiscale does not shrink
  the range until it has shown itself. Innates are species data and always apply.
- **Illusion:** a disguised foe is computed as the disguise — its species, typing, base stats
  and innates — so the range never betrays the Zoroark.

## Display

- Replaces the `TYPE/` label: `<Type> lo-hi%`, narrowed to fit the 64px row.
- **Both ends round down** (42.2-53.6% reads `42-53%`), so the top end never shows 100 for a hit that cannot take the full bar.
  The INFO viewer's foe `HP n%` rounds the **other** way (up), so on-screen "damage ≥ HP" is
  always a real KO: a 46.2% hit on a 46.6% foe reads `46%` vs `HP 47%`, not `46%` vs `46%`.
- **KO is spelled out**, judged against the foe's **current** HP (the bar's fraction at each end
  of the spread), so a `40-50%` move on a foe at 30% counts:
  - `Ground KO` — KOs at every spread and roll;
  - `Ground 40%-KO` — KOs at some (the frail end, or a high roll); `40%` is the low end;
  - `Ground 40-50%` — cannot KO.

  Plain text, no colour: it stays calm on the menu and reads for colour-blind players. Because the verdict uses exact HP, the readout never shows a number that the INFO
  viewer's `HP n%` contradicts.
- Without a KO verdict the range is capped at 100, which only happens behind a known Sturdy & co.:
  a **known** Sturdy, Focus Sash or intact Disguise at full HP blocks the verdict, since the foe
  would hang on; the AI's `CanEndureHit` decides, fed only what the
  player has seen. An unrevealed Sash still reads as a KO — the player can't know.
- **Z-Moves:** with a Z-Move armed, the Z view's type row shows the Z-Move's range (its own
  power for signature Z-Moves). A status Z-Move keeps the stock line.
- Status moves, and moves the calc says cannot damage, keep the stock `TYPE/<Type>` line.
- **Target:** the foe opposite, or its partner once that one is down — the same single
  readout the stock effectiveness icon uses. In doubles it does not follow the target cursor.
- A selected Mega Evolution is not projected; the range is for the current form.
