# Innate abilities — batching plan

A worksheet for taking the **pending** abilities in
[`INNATE_ABILITIES_PROGRESS.md`](INNATE_ABILITIES_PROGRESS.md) and wiring them as
innates in efficient groups. This doc is the *what to batch and why*; the
[`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) recipe is the authority on *how to
wire one* (the 5-step Definition-of-Done — data, allowlist, effect, Step 3.5
frontier-slot freeing, tests, indexes). **Read the recipe before starting any
batch; this doc does not repeat it.**

## Scope

Covers only the `:white_large_square:` (pending) rows in the progress doc.
`:x:` rows are rejected and out of scope; `:white_check_mark:` rows are done.
There are ~95 pending abilities.

## Why batching works (and when it doesn't)

The recipe's Step 3 is the only per-ability variable, and it sorts abilities into
*wiring classes*. The key observation: **abilities in the same class frequently
live at the exact same code site** — a `switch` we already touched for a done
ability. Wiring N of them there is barely more work than wiring one. This is
exactly how the pinch abilities (Overgrow/Blaze/Torrent/Swarm) and the weather
speed-doublers (Swift Swim/Chlorophyll/Sand Rush/Slush Rush) were each landed as
a single batch.

A batch is cheap when **all three** hold:
1. **Same code site** — they share a `switch`/`if` block already in the tree.
2. **Calc-modifier or single-site trait class** — no new driver/hook needed.
3. **AI is automatic** — the effect lives in the shared damage/turn-order calc
   the AI runs keyed off the real battler, so `IsInnateActive()` covers it for
   free (the recipe's litmus: *"if this were innate-only, would the AI still do
   the right thing?"*).

A batch is **not** cheap (split it, do fewer per PR) when it needs a **new event
driver/hook** (on-contact, switch-in) or has **off-field AI heuristics** that
read a bare `== ABILITY_X` (recipe Step 3's AI warning). Those still group by
*trigger*, but each member carries real engineering.

> **Divergence reminder (recipe "an innate is a pure boon"):** before wiring,
> ask *"does the real ability ever hurt its holder?"* If no → 1:1 copy. If yes →
> keep the upside, drop the cost, and note the divergence. Most batches below are
> clean upsides (1:1); the few with a cost are flagged.

> **Every batch still owes Step 3.5.** Run `grep -n ABILITY_X
> src/fork/frontier_extended_mons.c` for *each* ability in the batch and free any
> hardcoded set. This is the step that gets forgotten.

---

## Tier 1 — Largest clean batches (shared single site, AI-free, mostly 1:1)

These are the highest-value starting work: a single `switch`, calc-modifier class,
AI automatic. Model them on the pinch-ability batch.

### Batch A — Offensive move-power boosters
**~13 abilities · one site · 1:1 copies · AI free**

Iron Fist, Reckless, Strong Jaw, Tough Claws, Sharpness, Mega Launcher,
Steelworker, Steely Spirit, Rocky Payload, Sand Force, Analytic, Adaptability,
Punk Rock (sound-power half).

- **Site (confirmed):** `CalcAttackStat` / attack-modifier path in
  `src/battle_util.c` — the cases already cluster at ~L6849–7304, **right beside
  the already-wired `ABILITY_TECHNICIAN`** (L6849) and the pinch block. Each is an
  additive `case`/modifier; add an `IsInnateActive(battler, ABILITY_X)` credit
  beside the chosen-ability read, exactly like Technician.
- **Divergence:** none — a flat damage boost never hurts the holder. 1:1.
- **AI:** automatic (boost lives in the shared `CalcAttackStat`).
- **Watch:** Steely Spirit also boosts *allies'* Steel moves (second hit at
  L7006) — wire both. Punk Rock spans offense *and* the defensive reducer (Batch
  B) and sound-power site (L7883) — do its halves together or split it to one
  batch.

### Batch B — Defensive damage reducers
**~8 abilities · one site · 1:1 copies · AI free**

Solid Rock, Multiscale, Fur Coat, Ice Scales, Heatproof, Friend Guard, Water
Bubble (fire-half), Punk Rock (sound-half).

- **Site (confirmed):** `GetDefenderAbilitiesModifier` in `src/battle_util.c`,
  ~L7365–7930, **beside the already-wired `ABILITY_FILTER` (L7862) and
  `ABILITY_THICK_FAT` (L7365)**. Mirror Filter's `IsInnateActive()` clause.
- **Freebie:** **Solid Rock shares Filter's adjacent `case` (L7862–7864 are
  FILTER/SOLID_ROCK/PRISM_ARMOR)** — it's essentially already wired; just add it
  to the allowlist + data. Could even fold into the same PR as Filter follow-ups.
- **Divergence:** none — pure mitigation. 1:1.
- **AI:** automatic (shared damage calc).
- **Watch:** Friend Guard is an *ally*-side reducer (L7930) — confirm the
  doubles/partner path. Heatproof/Water Bubble also touch the burn-damage and
  Fire-power sites (L7018–7019, L9497) — grep all `ABILITY_X` hits.

---

## Tier 2 — Trivial clone batches (same effect as an existing/sibling ability)

Mechanically identical to another ability and share its `case`. Near-free once the
sibling is understood; some siblings are themselves pending, so wire the pair
together.

| Batch | Abilities | Site (confirmed) | Notes |
| :-- | :-- | :-- | :-- |
| C — Double physical Attack | Huge Power, Pure Power | `src/battle_util.c` L7213–7214 (shared `case`) | 1:1; both in one `case` already. AI free. |
| D — Full stat-drop protection | Clear Body, White Smoke | `src/battle_stat_change.c` L890–892 (adjacent to `FULL_METAL_BODY`, which is `:x:`) | Single-site trait swap. **Has off-field AI reads** (`battle_ai_*.c`) — wire those too. |
| E — Single-stat-drop protection | Hyper Cutter (Atk), Big Pecks (Def) | `src/battle_stat_change.c` L909, L911 (same block as D) | Fold into Batch D — same site. Keen Eye (acc) already done here. |
| F — Priority-move block | Queenly Majesty, Dazzling, Armor Tail | `grep` to confirm (likely `src/battle_move_resolution.c` / `battle_main.c`) | Identical effect; one site. Mirror the Prankster-style AI helper if a heuristic reads it. |
| G — Redirection-ignore | Propeller Tail, Stalwart | `grep` to confirm | Identical effect; one site. |

---

## Tier 3 — Single-site trait passives (one-line swaps, small groups)

Each is the recipe's *easy class*: find `GetBattlerAbility(b) == ABILITY_X`, swap
to `BattlerHasAbility(b, ABILITY_X)`, mark `// FORK: innate-aware`. Grouped by the
site they cluster at.

### Batch H — Trapping abilities
**3 · one tight block · 1:1**

Shadow Tag, Arena Trap, Magnet Pull — all in one block in `src/battle_util.c`
**L5279–5285**. **AI risk:** the switch AI reasons about trapping
(`battle_ai_switch.c`) — check those reads credit the innate (`SpeciesHasInnate`
off-field). Confirm the trapper's *own* escape isn't blocked (Shadow Tag self-check
at L5279 already guards `GetBattlerAbility != SHADOW_TAG`).

### Batch I — Status-condition immunities
**~6 · single-site each · 1:1 · same class as done Limber/Immunity/Insomnia**

Magma Armor (freeze), Water Veil (burn), Own Tempo (confuse + Intimidate-immune),
Inner Focus (flinch + Intimidate-immune), Leaf Guard (status-immune in sun),
Overcoat (powder + sandstorm + Effect-Spore-style immune).

- **Sites:** scattered but tight — e.g. Water Veil L5661/L9496, Magma Armor
  L5732/L9505, Own Tempo L5840/L9472 in `src/battle_util.c` (status-set site +
  the status-clear `switch`). Each ability has ≤2 sites; `grep -n ABILITY_X
  src/` per ability.
- **Watch:** Own Tempo and Inner Focus also need the **Intimidate-immunity** half
  — but Intimidate-as-innate isn't wired yet (Batch M), so the *immunity* side is
  a plain trait swap regardless. Overcoat spans sandstorm-damage (end-turn +
  AI predictors, like Sand Rush) and powder-move immunity — multiple sites.

---

## Tier 4 — Active / on-event abilities (need drivers — smaller PRs)

These fire a script/effect at an event and need a **driver + hook** (recipe Step
3, active class). Batch by *trigger*, but each member is real work and several
have off-field AI heuristics. **Do these as focused PRs, not one mega-batch.**

### Batch J — End-of-turn effects (reuse the Speed Boost driver)
Rain Dish, Ice Body (weather heals), Shed Skin (30% status cure), Hydration
(rain cure), Healer (ally cure), Harvest (berry recreate), Cud Chew (re-eat
berry).
- **Cheapest active class:** the end-turn driver already exists
  (`TryActivateInnateEndTurnEffects`); the recipe says adding one is *a one-line
  addition to `IsActiveEndTurnInnate`*. Reuses the existing
  `AbilityBattleEffects(ABILITYEFFECT_ENDTURN, …)` case per ability, so script +
  pop-up are free. **Strong batch despite being "active."**

### Batch K — On-contact reactions (need a NEW on-contact driver)
Rough Skin + Iron Barbs (1/8 contact damage, identical), Gooey + Tangling Hair
(Speed −1 on contact, identical), Aftermath (KO contact damage), Cursed Body
(disable), Steam Engine (Speed on Fire/Water hit).
- The on-contact driver is **not built yet** (recipe: "switch-in / on-contact
  actives still need their own event hooks"). Build the driver once (model on the
  end-turn driver), then the identical pairs make this efficient.

### Batch L — Switch-in actives (need a NEW switch-in driver)
Intimidate, Download, Anticipation, Forewarn, Frisk, Unnerve, Supersweet Syrup,
Hospitality.
- Switch-in driver also **not built yet**. Intimidate is the marquee one and has
  wide AI implications (every Intimidate-immunity ability above references it).
  Build the driver, then add members. Heaviest batch — split aggressively.

### Batch M — On-KO / on-hit stat boosts (script per ability)
Moxie, Justified, Rattled, Stamina, Water Compaction, Berserk, Anger Point,
Defiant, Competitive, Soul-Heart, Steadfast.
- Each fires a stat-change script on its trigger (KO, taking a hit, a crit, a
  stat-drop). Related triggers but distinct hooks; **off-field AI setup
  heuristics** likely read several. Lower priority than J/K.

---

## Suggested order of attack

1. **Batch A** (offensive boosters, ~13) — biggest single-site win, proven pattern.
2. **Batch B** (defensive reducers, ~8) — Solid Rock nearly free via Filter.
3. **Tier 2 clones** (C/D/E/F/G) — cheap, knock them out together.
4. **Batch H + I** (trapping, status immunities) — one-line swaps.
5. **Batch J** (end-turn actives) — reuses the existing driver.
6. **Batches K/L/M** — build the missing drivers; smaller, focused PRs.

That front-loads ~40 abilities across a few low-risk PRs before any new driver is
needed.

## Per-batch checklist (run the recipe's Definition of Done for each)

For every batch, per the recipe:
- [ ] **Step 1** — species rows for each ability (canon users in any slot + tight
  flavor set; merge into existing rows where a species already has an innate).
- [ ] **Step 2** — add each ability to the allowlist comment in
  `src/fork/innate_abilities.c` + SCOPE note in `include/fork/innate_abilities.h`.
- [ ] **Step 3** — wire the effect at *every* site (`grep -n ABILITY_X src/`),
  **including AI effect reads** (`grep src/battle_ai_*.c`). Confirm the
  pure-boon-vs-1:1 call and note any divergence.
- [ ] **Step 3.5** — `grep -n ABILITY_X src/fork/frontier_extended_mons.c` and free
  every hardcoded set (override-table rows for ability-locked species).
- [ ] **Step 4** — tests in `test/fork/innate_abilities.c`;
  `make check TESTS="FEATURE_INNATE_ABILITIES"`; full `make check` if a shared
  battle file was touched; ROM builds under `UNUSED_ERROR=1 DEPRECATED_ERROR=1`.
- [ ] **Step 5** — flip each row to `:white_check_mark:` in
  `INNATE_ABILITIES_PROGRESS.md`, update `FORK.md` and the
  [wiring reference](INNATE_ABILITIES.md#per-ability-wiring-reference), and tick
  the batch off in this doc.

## Status

Nothing started yet — all batches open. Site line numbers are accurate as of the
commit that added this doc; re-`grep` to re-anchor after an upstream sync, since
`battle_util.c` line numbers drift.
