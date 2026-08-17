# The in-battle INFO viewer (`B_FRONTIER_BATTLE_INFO`)

In Frontier facilities the bag is disabled, so its action slot is dead space. This
fork turns it into **INFO**: a read-only, five-page reference screen showing field
state, both sides' conditions and stat changes, a foe speed-tier comparison, and the
foe's revealed party data.

Its whole design problem is **what the player is allowed to know**. The battle engine
holds far more about the foe than the player has witnessed, and most of the code here
exists to keep the viewer honest about that line. Read the "Reveal gating" section
before touching anything.

- **Flag:** `B_FRONTIER_BATTLE_INFO` (`include/config/frontier.h`)
- **Code:** `src/fork/frontier_battle_info.c` (`CB2_FrontierBattleInfo`)
- **Status:** ⚠️ partial — the Foe page reads opponent A only (see Known limitations)

## Where it appears

Two entry points, both battle-only:

1. **The BAG action slot**, in facilities matching `BATTLE_TYPE_FRONTIER_NO_PYRAMID`
   (Tower, Dome, Palace, Arena, Factory, Pike). The **Pyramid is excluded** — it keeps
   a working bag, so its slot is not free.
2. **SELECT from the in-battle "choose a Pokémon" party menu** — e.g. while picking a
   replacement for a fainted mon — so the player can review field and foe state before
   committing to a switch.

The return target is a parameter (`OpenFrontierBattleInfo(returnCallback)`): the
action-menu path returns to the battle screen, the party-menu path re-opens the party
menu (`CB2_OpenBattleInfoFromPartyMenu` / `CB2_ReturnToPartyMenuFromBattleInfo` in
`src/party_menu.c`, gated on `ShouldReplaceBagWithInfo()`).

Opening it **does not consume the turn** — it reuses the `B_ACTION_DEBUG` controller
path.

## The five pages

**L/R** cycle in both directions, with a right-aligned `n/N` indicator
(`PrintPageIndicator`).

| Page | Shows |
|---|---|
| **Speed Tiers** | Each revealed foe's *possible* Speed range vs. your effective Speed (see below) |
| **Field** | Weather, terrain, entry hazards and side screens for both sides |
| **Conditions** | Each on-field battler's primary status + notable volatiles (confusion, leech seed, taunt…), both sides |
| **Stat Changes** | Each on-field battler's non-default stat stages, e.g. `Atk+2 Spe-1`, both sides |
| **Foe** | The foe's revealed-only party data; `<>` cycles mons — species/gender/level, `FNT` when fainted, moves/PP/ability/held item |

Under `DETERMINISTIC_DAMAGE` the Field page also prints the current turn and that
turn's fixed damage multiplier, so the player can read the exact roll
(`DrawDeterministicDamageLine`).

**Speed Tiers leads the cycle and is the default page on first open**, because it is
the page that actually drives a turn decision.

### Position memory resets between battles — deliberately

The last-viewed page and foe index persist across re-opens **within a battle only**.
They live in `gBattleStruct->infoViewerPage` / `infoViewerFoeIndex`, which is
zero-allocated each battle, specifically so the position resets to Speed Tiers / foe 0
rather than carrying a stale foe tab forward. A file static did carry it, and could
land on a previous battle's foe index — showing an unrevealed slot.

## The Speed Tiers page

The point of this page is to let the player place a foe on the speed tier **without
knowing its exact spread**, using only what is derivable from public data.

**Foe rows show a range, `min-max`, computed from base stats alone** at the foe's known
level: min = 0 IVs / 0 EVs / hindering nature ×0.9, max = 31 IVs / 252 EVs / boosting
nature ×1.1. Only *seen* slots are listed — unseen ones are omitted entirely, so
neither an unrevealed mon **nor the foe's party size** leaks. Rows sort fastest-first
by the top of their range, so the list reads as a true tier.

**Your rows use effective Speed instead** — `GetBattlerTotalSpeedStat`, which folds in
everything you already know about your own side: Choice Scarf, Tailwind, paralysis,
stat stages, Speed abilities.

The fastest of your active mons anchors a per-foe comparison **glyph**:

| Glyph | Meaning |
|---|---|
| `▼` (`CHAR_DOWN_ARROW`) | your effective Speed already exceeds that foe's **best-case** base Speed |
| `▲` (`CHAR_UP_ARROW`) | even the foe's **worst-case** base Speed exceeds yours |
| blank | the ranges overlap |

Two deliberate choices here:

- **A glyph, not a colour** — legible for colour-blind players (cf. the `COLOR_BLIND`
  HP bar flag).
- **It is a number comparison, not a turn-order guarantee.** Your *known* Speed vs. the
  foe's *possible base* Speed. The foe's range still deliberately ignores its own
  hidden Choice Scarf, paralysis or Speed ability, because the player has not seen
  those.

Code: `DrawSpeedPage` / `CalcSpeedBound` / `AppendSpeedGlyph`, mirroring
`CalculateMonStats`.

## Reveal gating — the part that keeps breaking

**Do not source anything from `gAiPartyData`.** It is the AI's *knowledge* model —
pre-filled under `AI_FLAG_OMNISCIENT` and padded with STAB/status assumptions — not a
record of what the player saw. Everything on the Foe page is gated on genuine reveal
instead:

| Datum | Reveal source |
|---|---|
| Seen at all | `gBattleStruct->partyState[…].sentOut` (species/gender/level/fainted then read from the party) |
| Moves | `gBattleStruct->infoUsedMoves`, set at the move-use site |
| Ability | `gBattleStruct->infoAbilityRevealed`, set in `RecordAbilityBattle` |
| Held item | `gBattleStruct->infoItemRevealed`, set in `RecordItemEffectBattle` + the visible-removal sites below |

### The two "Prankster" bugs

Both were leaks through the AI, and both fixes must stay:

1. **Reveals only fire while a turn is actually executing.** `BattleInfoCanRevealNow()`
   skips the reveal when `gAiLogicData->aiCalcInProgress` is set, so the AI's
   *speculative* move evaluation cannot reveal the foe's ability or item before the
   player has seen it. Without this gate the foe's ability read **"Prankster"** at
   battle start and on every switch-in, because `GetBattleMovePriority`'s Prankster
   check records through this path for every status move while the AI computes turn
   order.
2. **The displayed ability comes from a reveal-time snapshot**,
   `gBattleStruct->infoRevealedAbility[side][slot]`, written under the same gate — *not*
   from `gAiPartyData->mons[].ability`. That live model is freely overwritten by the
   AI's speculative move/switch evaluation (a benched Prankster mon simulated in the
   active slot records Prankster onto it) while the already-set reveal bit stays put —
   which made a foe whose Intimidate fired at battle start still display as
   **"Prankster"**.

### Item reveal sites

The item bit is set wherever an item visibly reveals itself, so an item the player
watched activate or leave reads as its name or `None`, never `?`:

- A fired hold effect at the `ItemBattleEffects` chokepoint (`battle_hold_effects.c`) —
  Life Orb recoil, Leftovers/berry heal, status orbs, stat-boost berries. Silent
  effects like Amulet Coin return `ITEM_NO_EFFECT` and are skipped.
- Knock Off (`battle_move_resolution.c`).
- The `Cmd_removeitem` destruction path (Fling / Corrosive Gas / Incinerate).
- Steals, via `StealTargetItem`.

### Illusion safety

A foe whose Illusion (Zoroark/Zorua) is currently `ILLUSION_ON` shows the **disguise**
mon's species, gender, level and Speed-tier range — not the real one — via
`GetFoeDisplayMon`, mirroring what the health box does (it also reads
`GetIllusionMonPtr`). The true species reveals only once the Illusion breaks.

### Innates are not reveal-gated

Under `FEATURE_INNATE_ABILITIES`, a mon's innates are a **static property of its
species**, like its type line — fully determined the moment the species is known. So
unlike the genuinely-hidden 1-of-N chosen-ability roll, they are *not* gated: the
viewer lists every innate of the **displayed** species as soon as the mon is seen
(Illusion-safe via `displaySpecies`, so a disguised Zoroark never leaks its real
identity through its innate list).

They share the Ability line rather than taking a dedicated `Innate:` row, which would
push the bottom Moves slot under the nav bar — the page is at its fixed height. The
chosen ability prints plainly and the innates follow in parentheses:

```
Magnet Pull (+Levitate, Sturdy)
? (+Levitate, Sturdy)        <- chosen ability still unseen
```

Only the *chosen* ability stays gated. `RecordAbilityBattle` will not mark it revealed
when what the player witnessed was an innate pop-up (`gBattleScripting.abilityPopupOverwrite`,
an innate Levitate/Sturdy forcing the pop-up to its name) rather than the chosen
ability — so an innate reveal never leaks the chosen one.

## Styling

**Stock assets only — no new art.** The screen uses:

- The player's chosen Options window frame, drawn around the window
  (`DrawTextBorderOuter` + `GetWindowFrameTilesPal`).
- The standard menu palette (`gStandardMenuPalette`) with conventional `TEXT_COLOR_*`
  text.
- A transparent window over a soft backdrop colour, with coloured page titles (red) and
  footer (blue) for hierarchy.

The full-width window's char block is nearly full, so **the frame is drawn on its own
BG**, with a heap tilemap buffer freed on close — a static buffer would grow EWRAM into
the heap.

## Upstream footprint

Small and `FORK:`-tagged: the player controller, `RecordAbilityBattle` /
`RecordItemEffectBattle`, the move-use site, the item reveal/removal sites, and one
menu-label string.

## Known limitations

- **The Foe page reads opponent A only.** Fine for singles and single-trainer doubles;
  a two-opponent multi battle shows just the first foe trainer's party.
