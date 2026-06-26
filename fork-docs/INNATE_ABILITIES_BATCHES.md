# Innate abilities — batching plan

A worksheet for taking the **pending** abilities in
[`INNATE_ABILITIES_PROGRESS.md`](INNATE_ABILITIES_PROGRESS.md) and wiring them as
innates in efficient groups. This doc is the *what to batch and why*; the
[`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) recipe is the authority on *how to
wire one* (the 5-step Definition-of-Done — data, allowlist, effect, Step 3.5
frontier-slot freeing, tests, indexes). **Read the recipe before starting any
batch; this doc does not repeat it.**

## Scope & coverage

Covers only the `:white_large_square:` (pending) rows in the progress doc.
`:x:` rows are rejected and out of scope; `:white_check_mark:` rows are done.

**There are 133 pending abilities, and every one is assigned to a batch below.**
The batch index at the end is the coverage checklist — if a pending ability
isn't in it, the doc is stale and must be fixed. (Earlier drafts only covered the
cleanest ~73; the rest are now folded in, including a Tier 5 bucket for the
genuinely bespoke ones.)

## Why batching works (and when it doesn't)

The recipe's Step 3 is the only per-ability variable, and it sorts abilities into
*wiring classes*. The key observation: **abilities in the same class frequently
live at the exact same code site** — a `switch`/`if` we already touched for a done
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
driver/hook** (on-contact, switch-in) or has **off-field AI heuristics** that read
a bare `== ABILITY_X` (recipe Step 3's AI warning). Those still group by
*trigger*, but each member carries real engineering.

> **Divergence reminder (recipe "an innate is a pure boon"):** before wiring,
> ask *"does the real ability ever hurt its holder?"* If no → 1:1 copy. If yes →
> keep the upside, drop the cost, and note the divergence. Most batches below are
> clean upsides (1:1); the few with a cost are flagged.

> **Every batch still owes Step 3.5.** Run `grep -n ABILITY_X
> src/fork/frontier_extended_mons.c` for *each* ability in the batch and free any
> hardcoded set. This is the step that gets forgotten.

> **Confirmed vs to-verify sites.** Line numbers marked *(confirmed)* were
> grepped at the commit that wrote this doc and drift after an upstream sync;
> everything else is marked *(grep to confirm)* — re-anchor before relying on it.

---

## Tier 1 — Calc-modifier batches (shared site, AI-free, mostly 1:1)

Highest-value work: a single `switch`, calc-modifier class, AI automatic. Model
them on the pinch-ability batch.

### Batch A — Offensive move-power boosters
**14 · `CalcAttackStat` · 1:1 · AI free**

Iron Fist, Reckless, Strong Jaw, Tough Claws, Sharpness, Mega Launcher,
Steelworker, Steely Spirit, Rocky Payload, Sand Force, Analytic, Adaptability,
Punk Rock (sound-power), **Stakeout** (2× vs a just-switched-in target).

- **Site (confirmed):** `CalcAttackStat` / attack-modifier path in
  `src/battle_util.c`, cases clustered ~L6849–7304, **beside the wired
  `ABILITY_TECHNICIAN` (L6849)** and the pinch block. Add an `IsInnateActive()`
  credit beside each chosen-ability read.
- **Divergence:** none (flat boost). 1:1.
- **Watch:** Steely Spirit also boosts *allies'* Steel moves (L7006). Punk Rock
  spans offense + the Batch B sound-damage half (L7883). Stakeout is conditional
  but lives in the same attack-stat path.

### Batch B — Defensive damage reducers
**7 · `GetDefenderAbilitiesModifier` · 1:1 · AI free**

Solid Rock, Multiscale, Fur Coat, Ice Scales, Heatproof, Friend Guard, Water
Bubble (fire-half).

- **Site (confirmed):** `GetDefenderAbilitiesModifier`, ~L7365–7930, **beside the
  wired `ABILITY_FILTER` (L7862) and `ABILITY_THICK_FAT` (L7365)**.
- **Freebie:** **Solid Rock shares Filter's adjacent `case` (L7862–7864)** —
  essentially already wired.
- **Watch:** Friend Guard is *ally*-side (L7930). Heatproof/Water Bubble also
  touch burn-damage / Fire-power sites (L7018–7019, L9497).

### Batch N — Status-conditional stat boosts
**5 · attack/defense/speed calc · 1:1 · AI free**

Guts (Atk +50% when statused; also negates burn's physical cut), Marvel Scale
(Def +50%), Quick Feet (Spe +50%; also negates paralysis speed cut), Toxic Boost
(physical +50% when poisoned), Flare Boost (special +50% when burned).

- **Sites (grep to confirm):** the boosts live in `CalcAttackStat` /
  `GetBattlerTotalSpeedStat` / defense-stat reads — same family as Batch A. Guts
  and Quick Feet have an extra clause (suppress the burn/paralysis penalty).
- **AI:** automatic (shared calc). **Watch:** the burn/paralysis-penalty
  suppression is the only non-calc bit.

### Batch O — Crit-rate / crit-damage modifiers
**3 · crit calc · 1:1 · AI free**

Super Luck (+1 crit stage), Sniper (crit damage ×2.25 vs ×1.5), Merciless
(auto-crit vs poisoned target).

- **Site (grep to confirm):** the crit-chance / `CalcCritChanceStage` and
  crit-damage modifier in `src/battle_util.c`. One area.

### Batch P — Accuracy / type-effectiveness / effect-chance modifiers
**6 · accuracy & secondary-effect calc · mixed · AI free**

Tinted Lens (NVE → full damage), Scrappy (hit Ghost with Normal/Fighting; the
Intimidate-immunity half waits on Batch L's driver), Wonder Skin (status moves
less accurate vs holder), Tangled Feet (evasion when confused), Shield Dust
(immune to added effects), Serene Grace (doubles added-effect chance).

- **Sites (grep to confirm):** Tinted Lens in the type-effectiveness damage
  multiplier; Scrappy in `CalcTypeEffectivenessMultiplierInternal` (the
  Ghost-immunity bypass); Wonder Skin/Tangled Feet in `GetTotalAccuracy`
  (same place Keen Eye/Compound Eyes were wired); **Shield Dust + Serene Grace
  both sit at the secondary-effect application site** (one negates, one doubles —
  pair them).

### Batch Q — Priority granters
**2 · `GetBattleMovePriority` · 1:1 · AI free**

Gale Wings (Flying-move +1 priority at full HP), Triage (healing-move +3
priority).

- **Site (confirmed family):** `GetBattleMovePriority` in `src/battle_main.c` —
  **the exact function Prankster was wired into**, so mirror the Prankster
  `IsInnateActive()` clause. AI turn-order prediction runs the same calc → free.

### Batch R — Terrain modifiers
**2 · speed/defense calc · 1:1 · AI free**

Surge Surfer (Spe ×2 on Electric Terrain — *the weather-speed-doubler pattern,
terrain edition*; wire at `GetBattlerTotalSpeedStat` exactly like Slush Rush),
Grass Pelt (Def ×1.5 on Grassy Terrain).

---

## Tier 2 — Trivial clone batches (same effect as an existing/sibling ability)

| Batch | Abilities | Site | Notes |
| :-- | :-- | :-- | :-- |
| C — Double physical Attack | Huge Power, Pure Power | `battle_util.c` L7213–7214 (shared `case`) *(confirmed)* | 1:1; both in one `case`. AI free. |
| D — Full stat-drop protection | Clear Body, White Smoke | `battle_stat_change.c` L890–892 (adjacent to `:x:` Full Metal Body) *(confirmed)* | Trait swap. **Off-field AI reads** — wire those too. |
| E — Single-stat-drop protection | Hyper Cutter (Atk), Big Pecks (Def) | `battle_stat_change.c` L909, L911 *(confirmed)* | **Fold into Batch D — same block.** Keen Eye (acc) already done here. |
| F — Priority-move block | Queenly Majesty, Dazzling, Armor Tail | grep to confirm (`battle_move_resolution.c`/`battle_main.c`) | Identical effect; one site. Mirror Prankster's AI helper if a heuristic reads it. |
| G — Redirection-ignore | Propeller Tail, Stalwart | grep to confirm | Identical effect; one site. |

---

## Tier 3 — Single-site trait passives (one-line swaps)

The recipe's *easy class*: find `GetBattlerAbility(b) == ABILITY_X`, swap to
`BattlerHasAbility(b, ABILITY_X)`, mark `// FORK: innate-aware`.

### Batch H — Trapping abilities
**3 · one block · 1:1**

Shadow Tag, Arena Trap, Magnet Pull — one block in `src/battle_util.c`
**L5279–5285** *(confirmed)*. **AI risk:** the switch AI reasons about trapping
(`battle_ai_switch.c`) — credit the innate (`SpeciesHasInnate` off-field).

### Batch I — Status-condition immunities
**6 · single-site each · 1:1 · same class as done Limber/Immunity/Insomnia**

Magma Armor (freeze), Water Veil (burn), Own Tempo (confuse), Inner Focus
(flinch), Leaf Guard (status-immune in sun), Overcoat (powder + sandstorm).

- **Sites (confirmed sample):** Water Veil L5661/L9496, Magma Armor L5732/L9505,
  Own Tempo L5840/L9472 in `battle_util.c` (status-set site + status-clear
  `switch`). ≤2 sites each.
- **Watch:** Own Tempo/Inner Focus also have an Intimidate-immunity half that
  waits on Batch L. Overcoat spans sandstorm-damage (end-turn + AI predictors,
  like Sand Rush) and powder immunity.

### Batch S — Miscellaneous single-site traits
**12 · independent one-liners (group for review economy, not a shared site)**

Suction Cups (no forced switch-out), Guard Dog (no forced switch-out; the
Intimidate-boost half waits on Batch L), Rock Head (no recoil), Long Reach (moves
aren't contact), Skill Link (multistrike always max), Infiltrator (ignore
screens/substitute), Corrosion (poison Steel/Poison), Sticky Hold (item can't be
removed), **Unseen Fist + Piercing Drill** (hit through Protect — *identical
pair*), **Heavy Metal + Light Metal** (double/halve weight — *same weight-calc
site*).

- Each is its own `grep -n ABILITY_X src/` + comparison swap. The two bracketed
  pairs share a site; the rest are independent but mechanically trivial, so they
  ride together in one review. **Suction Cups + Guard Dog** both touch the
  forced-switch check — wire that half together.

---

## Tier 4 — Active / on-event abilities (need drivers — smaller PRs)

These fire a script/effect at an event and need a **driver + hook** (recipe Step
3, active class). Batch by *trigger*; each member is real work and several have
off-field AI heuristics. **Focused PRs, not one mega-batch.**

### Batch J — End-of-turn effects (reuse the Speed Boost driver)
**10**

Rain Dish, Ice Body (weather heals), Shed Skin (30% status cure), Hydration (rain
cure), Healer (ally cure), Harvest (berry recreate), Cud Chew (re-eat berry),
Pickup (grab a consumed item), Bad Dreams (chip sleeping foes), Poison Heal (heal
instead of poison damage).

- **Cheapest active class:** the end-turn driver exists
  (`TryActivateInnateEndTurnEffects`); the recipe says adding one is *a one-line
  addition to `IsActiveEndTurnInnate`*, reusing the matching
  `AbilityBattleEffects(ABILITYEFFECT_ENDTURN, …)` case (script + pop-up free).
- **Watch:** Poison Heal isn't strictly end-turn-add — it *replaces* the
  status-damage step; wire at the poison-damage site, not the heal driver.

### Batch K — On-contact / on-hit / on-faint reactions (need a NEW driver)
**13**

Rough Skin + Iron Barbs (1/8 contact damage, identical), Gooey + Tangling Hair
(Speed −1 on contact, identical), Aftermath + Innards Out (KO/faint damage),
Cursed Body (disable on hit), Steam Engine (Speed on Fire/Water hit), Thermal
Exchange (Atk on Fire hit + burn-immunity half), Wind Power (charge on wind hit),
Pickpocket (steal on contact) + Magician (steal on damage), Liquid Ooze (damage
HP-drainers).

- The on-contact/on-hit driver is **not built yet** (recipe: "on-contact actives
  still need their own event hooks"). Build it once (model on the end-turn
  driver), then the identical pairs make this efficient. Split into sub-PRs
  (contact-damage, contact-stat-drop, steal, on-faint).

### Batch L — Switch-in actives (need a NEW driver)
**8**

Intimidate, Download, Anticipation, Forewarn, Frisk, Unnerve, Supersweet Syrup,
Hospitality.

- Switch-in driver also **not built yet**. Intimidate is the marquee one and the
  dependency for every Intimidate-immunity half flagged in Batches I/P/S/M
  (Own Tempo, Inner Focus, Scrappy, Guard Dog, Oblivious-done). Build the driver,
  then add members. Heaviest batch — split aggressively.

### Batch M — On-KO / on-hit stat boosts (script per ability)
**11**

Moxie, Justified, Rattled, Stamina, Water Compaction, Berserk, Anger Point,
Defiant, Competitive, Soul-Heart, Steadfast.

- Each fires a stat-change script on its trigger (KO, taking a hit, a crit, a
  stat-drop). Related triggers, distinct hooks; **off-field AI setup heuristics**
  read several. Lower priority than J/K.

### Batch T — Berry / item synergy
**4**

Gluttony (eat berry at 1/2 HP), Ripen (double berry effect), Cheek Pouch (heal on
berry eat), Unburden (Spe ×2 when item consumed/lost).

- Mixed classes: Gluttony is a threshold trait, Ripen a berry-effect modifier,
  Cheek Pouch an on-eat event, Unburden a speed-calc clause gated on an
  item-loss flag (needs a per-battler flag set at the loss site).

### Batch U — Ally-support (doubles-oriented)
**5**

Battery (ally special +30%), Power Spot (ally moves +30%), Telepathy (dodge ally
attacks), Aroma Veil (self+allies immune to infatuation/Taunt/etc.), Flower Veil
(Grass allies status-immune & undroppable).

- Battery/Power Spot are ally-side `CalcAttackStat` modifiers (Tier-1-like but
  partner-keyed, so confirm the doubles path). Telepathy/Aroma Veil/Flower Veil
  are traits. Low priority (singles-irrelevant for much of the roster).

---

## Tier 5 — Bespoke / deferred (one-off, multi-site, or dependency-blocked)

Don't batch these — each needs its own design pass, and some depend on rejected
abilities. Listed so they're not silently dropped.

| Ability | Why it's bespoke |
| :-- | :-- |
| Magic Guard | Multi-site immunity to *all* indirect damage (sandstorm, poison/burn, recoil, hazards, Life Orb, …) — a Levitate-scale sweep across every chip-damage source. |
| Mold Breaker | Cross-cutting: must make the holder's moves ignore the *target's* ability everywhere `IgnoresTargetAbility`-style checks run. Wide blast radius. |
| Magic Bounce | Reflects status moves back — needs the move-redirection/bounce machinery, not a simple clause. |
| Mirror Armor | Bounces stat-*drops* back at the source — reactive, ties into the stat-change site (Batch D family) but with its own redirect. |
| Dancer | Copies dance moves reactively mid-turn — bespoke move-trigger hook. |
| Opportunist | Copies a foe's stat *boosts* — reactive, needs a boost-watch hook. |
| Comatose | Permanent pseudo-sleep + status immunity — interacts with sleep mechanics & display as a near-form. |
| Quick Draw | 30% random "go first" — turn-order RNG; determinism-sensitive (cross-check `config/deterministic.h`). |
| Aura Break | Reverses Dark/Fairy Aura — but **Dark Aura & Fairy Aura are `:x:`** (won't be innates), so this has no innate counterpart to oppose; likely stays pending indefinitely. |
| Flower Gift | Sun-gated ally Atk/SpD boost **and** a form change — form machinery, not a clause. |
| Mega Sol | Treats weather as harsh sun for the holder's moves — niche custom weather-view clause. |

---

## Suggested order of attack

1. **A, B, N, O, P, Q, R** (Tier 1 calc batches) + **C–G** (clones) +
   **H, I, S** (trait swaps) — ~80 abilities, all calc/single-site, AI mostly
   free, no new drivers. This is the bulk and the low-risk majority.
2. **J + T** (end-turn / berry) — reuse the existing end-turn driver.
3. **K** — build the on-contact driver, then its identical pairs.
4. **L** — build the switch-in driver (unblocks the Intimidate-immunity halves).
5. **M, U** — on-event stat boosts and ally support.
6. **Tier 5** — bespoke, one at a time, last.

That front-loads ~80 abilities across low-risk PRs before any new driver is built.

## Per-batch checklist (run the recipe's Definition of Done for each)

- [ ] **Step 1** — species rows per ability (canon users in any slot + tight
  flavor set; merge into existing rows where a species already has an innate).
- [ ] **Step 2** — add each ability to the allowlist comment in
  `src/fork/innate_abilities.c` + SCOPE note in `include/fork/innate_abilities.h`.
- [ ] **Step 3** — wire the effect at *every* site (`grep -n ABILITY_X src/`),
  **including AI effect reads** (`grep src/battle_ai_*.c`); confirm pure-boon-vs-1:1
  and note any divergence.
- [ ] **Step 3.5** — `grep -n ABILITY_X src/fork/frontier_extended_mons.c` and free
  every hardcoded set (override-table rows for ability-locked species).
- [ ] **Step 4** — tests in `test/fork/innate_abilities.c`;
  `make check TESTS="FEATURE_INNATE_ABILITIES"`; full `make check` if a shared
  battle file was touched; ROM builds under `UNUSED_ERROR=1 DEPRECATED_ERROR=1`.
- [ ] **Step 5** — flip each row to `:white_check_mark:` in
  `INNATE_ABILITIES_PROGRESS.md`, update `FORK.md` and the
  [wiring reference](INNATE_ABILITIES.md#per-ability-wiring-reference), and tick
  the batch off below.

## Batch index (coverage checklist — all 133 pending abilities)

Nothing started yet — all batches open.

| Batch | Class | # | Status |
| :-- | :-- | :-: | :-: |
| A — Offensive move-power boosters | calc, AI-free | 14 | open |
| B — Defensive damage reducers | calc, AI-free | 7 | open |
| N — Status-conditional stat boosts | calc, AI-free | 5 | open |
| O — Crit-rate / crit-damage modifiers | calc, AI-free | 3 | open |
| P — Accuracy / type-eff / effect-chance | calc, AI-free | 6 | open |
| Q — Priority granters | calc, AI-free | 2 | open |
| R — Terrain modifiers | calc, AI-free | 2 | open |
| C — Double physical Attack | clone | 2 | open |
| D — Full stat-drop protection | trait (+AI) | 2 | open |
| E — Single-stat-drop protection | trait (fold into D) | 2 | open |
| F — Priority-move block | clone | 3 | open |
| G — Redirection-ignore | clone | 2 | open |
| H — Trapping | trait (+AI) | 3 | open |
| I — Status-condition immunities | trait | 6 | open |
| S — Misc single-site traits | trait | 12 | open |
| J — End-of-turn effects | active (existing driver) | 10 | open |
| K — On-contact/on-hit/on-faint | active (new driver) | 13 | open |
| L — Switch-in actives | active (new driver) | 8 | open |
| M — On-KO/on-hit stat boosts | active | 11 | open |
| T — Berry/item synergy | active/trait | 4 | open |
| U — Ally-support (doubles) | calc/trait | 5 | open |
| Tier 5 — Bespoke/deferred | one-off | 11 | open |
| **Total** | | **133** | |
