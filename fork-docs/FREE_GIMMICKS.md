# Item-free battle gimmicks (`FEATURE_FREE_GIMMICKS`)

The four battle-transformation gimmicks — Mega Evolution, Z-Moves, Terastallization
and Dynamax — normally each need a held item, a key item, and sometimes a charge flag.
This fork drops all of those requirements and gives every eligible mon a **picker** to
choose among the gimmicks it can currently use.

The restrictions that make gimmicks interesting are kept: **one gimmick per mon, and
once per type per trainer**. Dropping the item requirement means every mon on a team is
eligible, so they *compete* for that single slot rather than one mon being the
designated user. That competition is the feature.

- **Flag:** `FEATURE_FREE_GIMMICKS` (`include/config/feature.h`), registered in the
  runtime config system — tests flip it with `WITH_CONFIG`, and the test baseline
  forces it **off**. **Enabled (`TRUE`)** in real builds.
- **Code:** `src/battle_gimmick.c`, `src/battle_util.c`, `src/fork/battle_ai_gimmick.c`
- **Tests:** `test/fork/free_gimmicks.c`

> **Building Factory sets under this flag?** The set-authoring consequences — build for
> the base form, the Mega is upside; spreads steer which X/Y Mega you get; the Mega
> form's innate row is what's live — are in
> [`LINE_REVIEW.md`](LINE_REVIEW.md) under "Free gimmicks". This doc is the engine side.

## What the flag changes

### 1. Item-free enablement

Mega Evolution needs no Mega Stone, Z-Moves no Z-Crystal, Terastallization no Tera Orb
(nor the `B_FLAG_TERA_ORB_*` charge flags), Dynamax no Dynamax Band (nor
`B_FLAG_DYNAMAX_BATTLE`). The bag, hold-item and charge checks in `CanMegaEvolve` /
`CanUseZMove` / `CanTerastallize` / `CanDynamax` are skipped.

AI battlers skip those gates **unconditionally**, flag or no flag.

Consequence worth knowing: the `B_FLAG_TERA_ORB_*` flags now only gate the *player's*
side, and only in a free-gimmicks-**off** build. The same goes for the key items a new
game hands out under `START_WITH_BATTLE_GIMMICK_ITEMS` — they matter only with this
flag off.

### 2. Stat-based X/Y Megas

With no stone to disambiguate, `GetMegaStoneForBattler()` (`src/battle_util.c`) picks
the form from the battler's stats: **physical → X** when Attack ≥ Sp. Atk, **special →
Y** otherwise, so a tie defaults to X. It does this by feeding the stat-appropriate
stone into the existing item-driven form-change machinery, so eligibility, the form
change itself and the fork's Speed-Boost AI override all resolve through one path.

Only Charizard and Mewtwo have X/Y forms.

### 3. Item-free Z derivation

Any move becomes its signature Z-Move (matched on species + move) or, failing that, its
type-based Z-Move.

### 4. The picker

Candidate gimmicks are tracked per battler as a **set** (`gimmick.gimmickCandidates[]`)
alongside the selected one (`usableGimmick`). During move selection, **Start cycles**
through every gimmick the mon can currently use, then off
(`CycleGimmickSelection` / `RefreshGimmickTriggerSprite`).

Two menu-state rules the picker depends on:

- **It always opens unarmed, on the base moves.** `PlayerHandleChooseMove` clears
  `playerSelect` before the initial move-name paint, so a stale armed selection — e.g.
  Dynamax's Max-move names — cannot leak into a fresh menu.
- **The last-move cursor survives a Dynamax.** The begin animation reuses the switch-in
  animation for its shrink/grow visual (the same mon re-entering, passed as
  `dontClearTransform = TRUE`), which reset the action/move cursors to the first slot.
  `BtlController_HandleSwitchInAnim` now clears them only on a genuine switch, so the
  picker reopens on your last-used move like regular moves do.

### 5. Restrictions are unchanged

One gimmick per mon and once per type per trainer fall straight out of the existing
`GetActiveGimmick` / `HasTrainerUsedGimmick` checks. A trainer can still use e.g. one
Mega *and* one Z across a battle — but each mon picks one.

**A transformed mon (Ditto/Imposter) cannot Mega Evolve.** `CanMegaEvolve` mirrors
`CanBattlerFormChange`'s `B_TRANSFORM_FORM_CHANGES >= GEN_5` transform guard. Item-free
Mega no longer reads the held stone, which previously let a transformed Ditto *arm*
Mega only for the form change to be silently refused after the gimmick was already
spent. Z-Move, Dynamax and Tera stay available to a transformed Ditto.

## AI

### It uses everything

`ShouldTrainerBattlerUseGimmick` returns TRUE for all gimmicks for AI battlers, so Tera
and Dynamax are no longer opt-in and every eligible AI mon gimmicks. With the
per-type-once tracking, the team spreads its gimmicks across mons by priority (the
existing `ShouldUseZMove` / `DecideTerastal` reconsideration still applies).

### It picks among its candidates

`AssignUsableGimmicks` only seeds `usableGimmick` to the enum-first candidate, and
nothing AI-side revisited it — `CycleGimmickSelection` is the *player's* Start press. So
once free gimmicks made almost every mon Z-Move eligible, Z-Move permanently preempted
Dynamax and Tera and AI trainers never used them; `DecideTerastal` bailed for the same
reason.

`AI_SelectBestGimmick` (`src/fork/battle_ai_gimmick.c`) now chooses from the full
candidate set **before** move scoring, using the tunable `AI_GIMMICK_PREFERENCE_ORDER`
list — persistent gimmicks first, one-shot Z-Move last. Ultra Burst is left alone when
available, since it only unlocks Necrozma's Z-Move.

### Knock Off keeps its utility

`ShouldUseZMove` no longer upgrades Knock Off into Black Hole Eclipse when the target
holds a worth-removing item and the Z-Move would not secure a KO the regular hit
cannot. The AI strips the item and saves its one-shot Z-Move instead
(`src/battle_ai_util.c`, covered by `test/fork/ai_zmove_knock_off.c`).

## Related fix: Max Move priority

Damaging Max Moves are now correctly priority 0. `GetBattleMovePriority`
(`src/battle_main.c`) read the *base* move's priority for Dynamaxed damaging moves, so
a Dynamaxed Sucker Punch (→ Max Darkness) kept its +1. Free gimmicks surfaced it by
letting any priority move Dynamax. Regression test in `test/battle/gimmick/dynamax.c`.

## Known limitations

- **Ultra Burst is out of scope** — Necrozma still needs to hold Ultranecrozium Z.
- **The AI's gimmick choice is a fixed preference order** (`AI_GIMMICK_PREFERENCE_ORDER`),
  not situationally scored.
- **The extended frontier roster still carries its now-inert Mega Stones and
  Z-Crystals** as held items. Re-itemizing those tuned sets is open balance work.
- **The draft restriction `TeamHasGimmickItemConflict` is neutralized** under the flag
  (see [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md)).

Trainer battles are noticeably tougher with this on, by design. The Mega message reuses
the item-neutral "fervent wish" line.
