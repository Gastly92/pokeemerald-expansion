# The in-battle INFO viewer (`B_FRONTIER_BATTLE_INFO`)

In Frontier facilities the bag is disabled, so its action slot is dead space. This
fork turns it into **INFO**: a read-only, six-page reference screen showing field
state, both sides' conditions and stat changes, a foe speed-tier comparison, and the
foe's revealed party data and innates, and both sides' base stats.

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

## The seven pages

**L/R** cycle in both directions, with a right-aligned `n/N` indicator
(`PrintPageIndicator`).

| Page | Shows |
|---|---|
| **Speed Tiers** | Each revealed foe's *possible* Speed range vs. your effective Speed (see below) |
| **Field** | Weather, terrain, whole-field effects (Trick Room, Gravity, Magic/Wonder Room, the Sports, Fairy Lock, Ion Deluge) with turns left, plus entry hazards and side screens for both sides |
| **Conditions** | Each on-field battler's primary status + notable volatiles (confusion, leech seed, taunt…), both sides |
| **Stat Changes** | Each on-field battler's non-default stat stages, e.g. `Atk+2 Spe-1`, both sides |
| **Foe** | The foe's revealed-only party data; `<>` cycles mons — species/gender/level, `FNT` when fainted, moves/PP/ability/held item |
| **Base Stats** | One of your party mons (`{UP}{DOWN}`, bench included) and the selected foe as a stat table, each with the Mega/Primal form(s) it can reach |
| **Innates** | The same foe's innate list, one per row (`FEATURE_INNATE_ABILITIES` only — the page does not exist when the feature is off) |

The whole-field row (`BuildFieldEffectLine`) lists effects that belong to neither
side and are neither weather nor terrain, each with its turns left where it has a
timer. Entries are ordered by how often they decide a turn - **Trick Room first** -
because the row is a fixed height and clips at the window edge if several stack, so
the ordering decides what survives.

Under `DETERMINISTIC_DAMAGE` the Field page also prints the current turn and that
turn's fixed damage multiplier, so the player can read the exact roll
(`DrawDeterministicDamageLine`). With that line on, the page is at its full nine
rows and has no room for a tenth - check the arithmetic against `LINE_H` and the
footer's `y` before adding one.

**Speed Tiers leads the cycle and is the default page on first open**, because it is
the page that actually drives a turn decision.

### Position memory resets between battles — deliberately

The last-viewed page and foe index persist across re-opens **within a battle only**.
They live in `gBattleStruct->infoViewerPage` / `infoViewerFoeIndex`, which is
zero-allocated each battle, specifically so the position resets to Speed Tiers / foe 0
rather than carrying a stale foe tab forward. A file static did carry it, and could
land on a previous battle's foe index — showing an unrevealed slot.

The restored page is clamped against `InfoPageCount()` on open, since
`FEATURE_INNATE_ABILITIES` is runtime-toggleable and a saved position on the Innates
page must not survive the feature being turned off.

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

Because it stays a number comparison, **Trick Room inverts what the glyphs imply** -
the faster mon moves last. Rather than flipping the glyphs (which would make `▼` mean
two different things depending on hidden-ish field state), the page prints a
right-aligned **`TRICK ROOM`** marker on its title row while the room is up
(`PrintTrickRoomMarker`). It costs no body line, which matters: with two player rows
and six revealed foes the page is already full. The Field page carries the turn count.

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

### Held-item and depth reveals from a lens (`BUFF_ACCURACY_ITEMS_REVEAL`)

Wide Lens and Zoom Lens write reveal bits directly, as the one thing in the game that
sells *information* rather than damage or survival — see the flag comment in
`include/config/buff.h`. `ApplyAccuracyItemReveals()` runs from
`OpenFrontierBattleInfo()`, so the reveal is exactly what the lens can see at the moment
the player looks; nothing is latched per turn.

| Lens | Reveals | Condition |
|---|---|---|
| **Wide Lens** (breadth) | Held item of **every seen foe** | none — the holder is on the field |
| **Zoom Lens** (depth) | **One foe's chosen ability + full moveset** | that foe has used a move (`infoUsedMoves != 0`) |

Wide Lens earns its slot on the items that never announce themselves — Choice items,
Assault Vest, Heavy-Duty Boots, type items — which otherwise read `?` all battle. Zoom
Lens's "must have watched it act" is the same observe-then-know identity as its
moving-second PP window, widened from one turn to the battle.

Three gating rules the lens obeys, and must keep obeying:

- **Only `sentOut` foes.** A lens sharpens what is on the field and in the record; it
  never conjures a mon the player has not met.
- **It does not pierce Illusion.** Every reveal is skipped for a foe whose Illusion is
  currently `ON`, since the page is showing the disguise and writing the real ability or
  moveset would leak the Zoroark. (An Illusion is a projection, not concealment.)
- **The ability written is the CHOSEN one**, read straight from the party. Unlike
  `RecordAbilityBattle` — which reveals only what was *witnessed* and has to dodge innate
  pop-ups — the lens reads the mon directly, so there is no witnessed/chosen split.

The asymmetry is deliberate: the Factory AI runs `AI_FLAG_OMNISCIENT` (via
`AI_FLAG_SMART_TRAINER` in `B_FRONTIER_HARD_AI_FLAGS`) and already knows the player's
moves, abilities and items, so a lens on an AI mon does nothing. The lenses are the
player's way of closing exactly that gap.

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

Only the *chosen* ability stays gated. `RecordAbilityBattle` will not mark it revealed
when what the player witnessed was an innate pop-up (`gBattleScripting.abilityPopupOverwrite`,
an innate Levitate/Sturdy forcing the pop-up to its name) rather than the chosen
ability — so an innate reveal never leaks the chosen one.

## The Base Stats page

Base spreads as a table, right-aligned so forms compare straight down the column:
`HP Atk Def SpA SpD Spe BST`. **One of your mons above the selected foe**, each followed
by the Mega/Primal form(s) it can reach.

```
BATTLE INFO  -  BASE STATS 1/3                Bench 4/6
                HP  Atk  Def  SpA  SpD  Spe  BST
You Salamence   95  135   80  110   80  100  600
  Mega          95  145  130  120   90  120  700
Foe Charizard   78   84   78  109   85  100  534
  Mega X        78  130  111  130   85  100  634
  Mega Y        78  104   78  159  115  100  634
{UP}{DOWN}: You  <>: Foe  L/R: Page  B: Close        6/7
```

Nothing else in the game surfaces this. The summary screen shows *computed* stats — the
numbers after IVs, EVs, nature and level — and neither it nor anything else shows what a
Mega form would turn a mon into. That matters most in the Factory, where the rentals
aren't yours to begin with.

### Two cursors

The page is the only one with two axes, because it answers two questions at once:

- **`<>` walks the foe's revealed slots**, shared with the Foe and Innates pages via
  `tFoeIndex`, so an L/R step between the three stays on the same mon.
- **Up/Down walks your own party**, benched mons included — "what should I switch to" is
  exactly the question base stats answer. Every other foe-scoped page aliases Up/Down
  onto the foe tab; this one needs vertical for its own cursor, which is why
  `Task_InfoProcessInput` special-cases it.

A right-aligned marker on the **title row** — `You 2/4` or `Bench 4/6` — says which of
your mons is shown and whether it is on the field. It rides the title so it costs no body
row, the same trick the Speed page's `TRICK ROOM` marker uses. The on-field/benched word
is what keeps a benched spread from being misread as the matchup you are currently in,
and it earns its place most in doubles.

**Fainted slots are skipped**: they are not switch candidates, and the label column has no
room for an `FNT` marker beside a 12-character species name.

**The player cursor is not persisted**, unlike the foe tab. Your active mon changes every
time you switch, so a remembered index would open the page on a benched mon when you
wanted the one you are deciding about. Opening always lands on what is on the field (or,
from the party-menu entry point after your mon has fainted, on your first living mon).

### Your side is fact, the foe's is possibility

The same split the Speed Tiers page draws.

- **Your row is a projection — one row for what you *will* become.** For the mon on the
  field it goes through the engine's own eligibility check: `CanMegaEvolve` applies the
  real rules (that mon's gimmick slot, a held Z-Crystal blocking a Mega without free
  gimmicks, the one-per-trainer budget), so a mon that can't Mega this battle correctly
  shows no row. A **benched** mon has no battler to ask, so it gets the same resolution
  minus the per-mon part, still gated on your side's Mega budget being unspent.
- **The foe's rows are its species' whole list**, because its spread and item are hidden.

**The two resolutions cannot drift**, because the pick that decides *which* Mega goes
through one function. `FindMegaStoneForStats` (`include/fork/free_gimmicks.h`) chooses the
form from a mon's Attack/Sp. Atk, and both the live form change
(`GetBattleFormChangeTargetSpecies`) and this page's benched projection call it. It used
to be `GetMegaStoneForBattler` in `src/battle_util.c`, reading `gBattleMons` directly;
it moved to a fork header — as a `static inline`, so both callers keep inlining it —
because a benched party mon has no `gBattleMons` entry.

### Reveal gating

- **Base stats are not reveal-gated**, for the same reason innates aren't (below): they
  are a static property of the *species*, fully determined the moment it is known. The
  Speed Tiers page already exposes the foe's base Speed this way. The Foe page's gates
  still apply to the foe half — nothing for a slot that has not been sent out, and every
  species read goes through `GetFoeDisplayMon`, so a disguised Zoroark reports the spread
  of the mon the player believes they are facing.
- **The foe's form rows come from the species' form-change table, never from its held
  item.** "Charizard has Mega forms" is dex knowledge; "this Charizard holds Charizardite
  Y" is not, and reading the stone would leak the very item the Foe page deliberately
  prints as `?`. Under `FEATURE_FREE_GIMMICKS` (on in real builds) no stone is needed at
  all, so every row is a form the foe may genuinely turn into.

### Details

- **Form labels are built from the Mega Stone that produces the form.** Every Mega shares
  its base form's species *name* ("Charizard" for both X and Y, "Absol" for both Mega and
  Mega Z), so the suffix is taken from the stone — Charizardite **X**, Raichunite **Y**,
  Absolite **Z** — which keeps working as upstream adds Mega lines (the Gen 9 `_Z` megas
  under `P_GEN_9_MEGA_EVOLUTIONS` arrived exactly that way). This names the *form*; the
  stone is never read off the mon. Your own projected row needs no suffix — the pick has
  already been made from that mon's stats.
- **A mon that has already transformed says so** (`(Mega)`, `(Primal)`), rather than
  leaving a missing row to mean both that and "cannot Mega".
- **Rayquaza's move-based Mega counts**, via `FORM_CHANGE_BATTLE_MEGA_EVOLUTION_MOVE`.
  **Gigantamax does not**: Dynamax multiplies HP rather than replacing the spread, so a
  row for it would repeat the one above.
- **The row budget is proved, not hoped.** `INFO_MAX_DISPLAYED_ALT_FORMS` caps the foe's
  form rows and a `STATIC_ASSERT` proves the worst case fits — showing one of your mons
  rather than every active one is what buys the headroom. A sweep in
  `test/fork/frontier_battle_info_reveal.c` fails if any species declares more reachable
  forms than the cap, because the fix there is to rework the layout, not to silently drop
  a form the player can be hit by. The label column was measured against every species
  name in the dex (widest: `Foe Brambleghast`, 77px of 78), so nothing clips; the
  clip-to-fit path in `PrintRowLabel` is a safety net for future data and only ever
  shortens a *name*, never drops a row.

## The Innates page

Innates used to be spelled out inline on the Foe page's Ability row, as
`Magnet Pull (+Levitate, Sturdy)`. That row could not hold them: species in this fork
carry up to **eight** innates, which is ~120 characters of ability names against a
64-byte `line` buffer and a 224px window — so a full list both **smashed the stack
buffer** and ran off the right edge. It is now its own page.

- **It sits directly after the Foe page and shares `tFoeIndex`**, so `<>` cycles mons
  here too and one L/R step from a mon's Foe page lands on that same mon's innates.
- **The Foe page keeps a bounded pointer to it**: `Ability: Magnet Pull  +2 innates`.
  A count cannot overflow the row the way a list could.
- **It is the last page, and only exists when the feature is on.**
  `InfoPageCount()` returns `INFO_PAGE_COUNT - 1` when `FEATURE_INNATE_ABILITIES` is
  off, which drops it out of the L/R cycle and the `n/N` indicator rather than parking
  the player on a permanently blank page. The count is computed at runtime because the
  flag is runtime-registered (per-test `WITH_CONFIG`).
- **Layout:** title, species header, then up to `INNATE_ROWS` (7) rows. A list that
  fits stays in one full-width column; only a longer one splits into two, so the common
  3-5 innate case reads as a plain list rather than a cramped grid.
- **Both the count and the list come from `CollectDisplayedInnates()`**, so the number
  the Foe page advertises can never disagree with what this page prints. It drops an
  innate that merely duplicates the *revealed* chosen ability, so nothing echoes twice.
- **The page's capacity is a data invariant, not just a display detail.** It is exported
  as `INFO_MAX_DISPLAYED_INNATES`, and a table guard in `test/fork/innate_abilities.c`
  fails if any species declares more innates than the page can list — otherwise the
  count would promise entries the page silently truncates.

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
