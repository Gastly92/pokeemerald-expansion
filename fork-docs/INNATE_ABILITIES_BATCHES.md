# Innate abilities — batching plan

## ▶ Standing instruction: "let's do the next batch of innate abilities"

When the maintainer says that (or anything like it), this is the whole job — no
other context needed:

1. **Find the next batch.** It is the **first numbered row in the
   [Execution queue](#execution-queue) whose status is still `open`.** That single
   rule decides "next" — don't improvise the order. (The queue interleaves the two
   driver-build steps, so "next" never lands on a batch that's blocked on a missing
   driver.) Two caveats now that the tail is expanded: the `⟂`-marked **Batch W**
   (frontier-slot freeing) is a **parallel track**, not a numbered gate — skip it
   when picking "next" and do it in its own sessions whenever; and Tier 5 (steps
   24–35) is still **one ability per PR**, so "the batch" there is that single
   row's ability, not the whole tier.
2. **Do it by the recipe.** Open [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) and
   run its 5-step Definition of Done for that batch. **Re-grep every site first**
   (`grep -n ABILITY_X src/` and `grep src/battle_ai_*.c`) — the line numbers in
   this doc drift after upstream syncs, and the newer batches' sites are only
   *(grep to confirm)*.
3. **One batch → one branch (`claude/...`) → one PR** against `master`. Don't
   bundle two batches.
4. **Update the records (all of them):** flip each ability's row to
   `:white_check_mark:` in `INNATE_ABILITIES_PROGRESS.md`; add/extend its block in
   the [wiring reference](INNATE_ABILITIES.md#per-ability-wiring-reference) and
   `FORK.md`; and **mark the batch `done` in the Execution queue + the
   [batch index](#batch-index-coverage-checklist--all-133-pending-abilities) of
   this doc** (so the *next* session's step 1 finds the right row).

**Doc hygiene / lifecycle (don't skip):** mark a finished batch `done` **in
place — never delete its rows** while any batch is still `open`. The batch index
must keep summing to **133**; that sum is the tripwire proving every pending
ability is still accounted for, and deleting rows breaks it. Only once the entire
queue reads `done` is the doc retired — at that point delete it (or reduce it to a
one-line stub pointing at the progress doc + wiring reference), because the
progress doc and wiring reference are then the complete record.

---

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
- **Step 3.5 partially deferred (tracked follow-up).** The grep fired on ~46
  frontier sets. Five were freed to a stable `:x:` chosen ability (Breloom→Effect
  Spore, Goodra→Sap Sipper, Mr. Rime→Screen Cleaner, Arctovish→Water Absorb,
  Sandaconda→Sand Spit). The rest are left on their now-innate chosen ability —
  **still correct** (the chosen ability runs; the innate is redundant-but-skipped
  on that set), just not upgraded — because ~20 of the affected species now have
  **all** their real abilities innate (Blastoise, Dewgong, Chansey, Blissey,
  Ludicolo, Manaphy, Phione, Darkrai, the Avalugg/Exeggutor/Bellossom/Walrein/
  Luvdisc/Gorebyss/Whiscash/Tropius/Glaceon/Gliscor lines, …), so each needs a
  game-wide fork override + a per-species `test/battle/` audit — a focused
  follow-up, not bundled into this PR. See the INNATE ABILITIES note in
  `src/fork/frontier_extended_mons.c`.

### Batch K — On-contact / on-hit / on-faint reactions (need a NEW driver)
**13**

Rough Skin + Iron Barbs (1/8 contact damage, identical), Gooey + Tangling Hair
(Speed −1 on contact, identical), Aftermath + Innards Out (KO/faint damage),
Cursed Body (disable on hit), Steam Engine (Speed on Fire/Water hit), Thermal
Exchange (Atk on Fire hit + burn-immunity half), Wind Power (charge on wind hit),
Pickpocket (steal on contact) + Magician (steal on damage), Liquid Ooze (damage
HP-drainers).

- The on-contact/on-hit driver is now **built**: `TryActivateInnateOnHitEffects`
  (`src/fork/innate_abilities.c`), hooked from the new `MOVEEND_ABILITIES_INNATE`
  step, delegates to the upstream `ABILITYEFFECT_MOVE_END` case (see the
  `### ABILITY_ROUGH_SKIN / …` wiring block in `INNATE_ABILITIES.md`). The first
  sub-PR shipped the **contact-reaction** pair-set — Rough Skin + Iron Barbs
  (1/8 contact damage) and Gooey + Tangling Hair (Speed −1 on contact); the second
  shipped the **on-faint** pair — Aftermath (1/4 max HP on a contact KO) + Innards
  Out (holder's lost HP on any KO), fired from the same step once the holder faints;
  the third shipped the **on-hit stat/charge** trio — Steam Engine (Speed +6 on a
  Fire/Water hit), Thermal Exchange (Attack +1 on a Fire hit + burn immunity), Wind
  Power (charge on a wind hit, plus the Tailwind ally hook). Remaining sub-PRs reuse
  the driver (a one-line `IsActiveOnHitInnate` addition each): disable (Cursed Body),
  steal (Pickpocket + Magician — Magician is attacker-side,
  `ABILITYEFFECT_MOVE_END_ATTACKER`, so it needs the attacker-side hook too), and
  Liquid Ooze (damage HP-drainers).

### Batch L — Switch-in actives (need a NEW driver)
**8**

Intimidate, Download, Anticipation, Forewarn, Frisk, Unnerve, Supersweet Syrup,
Hospitality.

- The switch-in driver is now **built**: `TryActivateInnateSwitchInEffects`
  (`src/fork/innate_abilities.c`), hooked from the new
  `FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE` step (`src/battle_switch_in.c`),
  delegates to the upstream `ABILITYEFFECT_ON_SWITCHIN` case (see the
  `### ABILITY_INTIMIDATE` wiring block in `INNATE_ABILITIES.md`). The first sub-PR
  shipped **Intimidate** (−1 Attack on every foe at switch-in) — the marquee member
  and the dependency for the Intimidate-immunity halves in Batches I/P/S (Own Tempo,
  Inner Focus, Scrappy, Oblivious were already wired to shrug it off; Guard Dog's
  boost-on-intimidate half is still a follow-up). Remaining members reuse the driver
  (a one-line `IsActiveSwitchInInnate` addition each) plus their own effect wiring:
  Download, Anticipation, Forewarn, Frisk, Unnerve, Supersweet Syrup, Hospitality.
  Heaviest batch — split aggressively.

### Batch M — On-KO / on-hit stat boosts (script per ability)
**11**

Moxie, Justified, Rattled, Stamina, Water Compaction, Berserk, Anger Point,
Defiant, Competitive, Soul-Heart, Steadfast.

- Each fires a stat-change script on its trigger (KO, taking a hit, a crit, a
  stat-drop). Related triggers, distinct hooks; **off-field AI setup heuristics**
  read several. Lower priority than J/K. **Split aggressively** — the triggers
  don't share one site, so do a coherent sub-group per PR.
- **Done — the stat-drop-reaction pair (Defiant / Competitive).** Both fire from
  the single native command `BS_TryDefiantRattled` (`src/battle_script_commands.c`)
  when a foe lowers a stat (move, Intimidate, or Sticky Web). Made innate-aware by
  crediting an innate when the chosen ability isn't reactive + overwriting the
  pop-up; the anim suppression and the two dedicated AI reads (`IncreaseStatDownScore`,
  `ShouldSwitchIfIntimidateBenefit`) are innate-aware too. Rattled *also* runs
  through this command (Intimidate → Speed +1) but is left for a later sub-group.
- **Remaining (9):** Moxie / Soul-Heart (on-KO), Justified / Stamina / Water
  Compaction / Anger Point / Rattled (on-hit — several reuse the Batch K on-hit
  driver), Berserk (HP-threshold), Steadfast (on-flinch).

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

**Still one ability per session/PR** — these each need their own design pass, and
some depend on rejected abilities. What's new below is an **order**: the 11 are no
longer an unordered bucket but an execution sub-queue, easiest/most-contained and
**unblocked first**, bespoke reactive machinery in the middle, dependency-blocked
last. Take them top-down, exactly like the main queue. The intra-Tier-5 synergies
are flagged (Opportunist/Mirror Armor share the stat-change site; Mirror
Armor/Magic Bounce share "bounce-back" machinery) — adjacent so a session reuses
context, but still one PR each.

| # | Ability | Class | Why here / what it needs |
| :-: | :-- | :-- | :-- |
| 5.1 | **Mega Sol** | self-contained clause | Smallest: one weather-view clause so the holder's moves treat the weather as harsh sun (fork-custom ability). AI-free (shared move calc). Good opener to re-establish the recipe on a Tier-5 shape. |
| 5.2 | **Quick Draw** | self-contained clause, determinism-sensitive | Single turn-order clause (30% "go first" among same-priority). One site, but **must route/gate through `config/deterministic.h`** — the fork's determinism layer owns RNG-driven turn order; cross-check before touching the roll. |
| 5.3 | **Comatose** | trait (reuses Batch I) + asleep-read | Immune to all non-volatile status (the Batch I status-immunity pattern) **plus** a "counts as asleep" read at sleep-gated sites (Rest, Snore, Sleep Talk, Hex, sleep-move synergy). No new driver. **Divergence:** drop the form/display half — the innate is the pure-boon status-immunity + always-asleep, not a near-form. |
| 5.4 | **Magic Guard** | cross-cutting sweep, no new machinery | Levitate-scale: guard *every* indirect-damage source (sandstorm/hail, poison/burn, recoil, crash, hazards, Leech Seed, Curse, Nightmare, Life Orb, …) with `BattlerHasAbility`. Wide but mechanical — one comparison swap per chip site. Highest-value defensive innate. |
| 5.5 | **Mold Breaker** | cross-cutting sweep | Holder's moves ignore the *target's* ability at every `IgnoresTargetAbility`-style site. Wide blast radius; do **after** Magic Guard's sweep experience. **Pairs with Batch X below** (the `jumpifability` chosen-slot fix), which removes the same cross-slot blind spot Mold Breaker exposes. |
| 5.6 | **Opportunist** | reactive, small new hook | Copy a foe's stat *boosts*. Needs a boost-watch hook at the stat-change apply site (Batch D family). |
| 5.7 | **Mirror Armor** | reactive, redirect | Bounce stat *drops* back at the source. Same stat-change site as Opportunist **+** a redirect — do it right after 5.6 to reuse that context. |
| 5.8 | **Magic Bounce** | reactive, bounce machinery | Reflect status moves back at the user. Needs the move-bounce machinery (shares the "bounce-back" idea with Mirror Armor, but for whole moves). Heaviest reactive member. |
| 5.9 | **Dancer** | reactive, bespoke move-trigger | Copy a dance move mid-turn the instant any battler uses one. Bespoke mid-turn move-copy hook. |
| 5.10 | **Flower Gift** | calc clause + dropped form | Sun-gated ally Atk/SpD boost **is** a doubles calc clause (doable). **Divergence:** drop the Cherrim form change — the innate ships the stat boost only. Late because it's still bespoke (doubles-keyed, sun-gated). |
| 5.11 | **Aura Break** | **resolve, do not wire** | Its counterparts (Dark Aura / Fairy Aura) are `:x:` — there is **nothing to reverse**, so an innate clause would be dead code. Disposition step: mark it won't-wire (an effective `:x:` with a note) rather than ship a no-op. Left last so the decision is explicit, not silent. |

---

## Deferred cross-batch follow-ups (not pending-ability rows)

Tier 5 is the last of the **133 pending abilities**, but it is *not* the last of
the innate work. Three tracked follow-ups accrued while the earlier batches
shipped — sub-work on abilities that are already `:white_check_mark:` (or `:x:`),
so they are **excluded from the 133** (they must not inflate the batch-index sum)
but still belong in the plan. Two of them were **blocked on a driver that Batch L
has since built**, so they are now actionable.

### Batch V — Complete the partial halves Batch L unblocked ✅ DONE
**2 half-abilities · switch-in driver now exists · small, high-value finish**

When these were wired, the switch-in driver didn't exist yet, so each shipped with
a documented half missing. Batch L built `TryActivateInnateSwitchInEffects`
(`src/fork/innate_abilities.c`), which cleared both blockers — now both are wired:

- **Guard Dog** (Batch S) — the forced-switch-block half shipped in Batch S; its
  **Intimidate-immunity + Attack-boost-on-intimidate** half is now wired at
  `IsIntimidateBlocked` (`src/battle_stat_change.c`): an innate Guard Dog is immune
  to Intimidate's Attack drop and boosts its own Attack +1, mirroring the chosen
  `ABILITY_GUARD_DOG` case (pop-up/record overwritten to Guard Dog). The two
  `ShouldSwitchIfIntimidateBenefit` AI reads (`battle_ai_switch.c`) credit the innate.
- **Pastel Veil** (Batch I family) — its **switch-in ally-cure** half
  (`BattleScript_PastelVeilActivates`, cure self+partner poison on switch-in) now
  rides the switch-in driver: `SwitchInInnateAbilityEffect` maps `ABILITY_PASTEL_VEIL`
  to `ABILITYEFFECT_ON_SWITCHIN` and the effect site overwrites the pop-up when the
  chosen ability differs. The `ABILITYEFFECT_IMMUNITY` self-cure (Batch I) remains as
  a fallback for Immunity and non-switch-in re-checks.

### Batch W — Frontier-slot freeing sweep (Step 3.5 backlog)
**data cleanup · multi-session · low-risk · deferred across J/T/K/L/M/U**

Every active batch (J, T, K, L, M, U) deferred the same Step-3.5 tail: species in
`src/fork/frontier_extended_mons.c` whose real abilities are **all now innate**
keep a now-redundant *chosen* ability instead of freeing that complementary slot
to a stable `:x:` pick. This is **functionally correct today** (the chosen ability
still runs; the innate is redundant-but-skipped there) — it's an *upgrade*, not a
bug. Freeing each needs a game-wide fork override **plus a per-species
`test/battle/` audit**, so it's a mechanical, low-risk track best done in its own
focused sessions, in parallel with Tier 5. See the `INNATE ABILITIES` header note
in `frontier_extended_mons.c` and the per-batch `DEFERRED` notes in
`INNATE_ABILITIES.md` for the affected set (~40 Batch-J sets and the tails of T/K/
L/M/U).

### Batch X — Script `jumpifability` innate-awareness (cross-slot)
**cross-cutting polish · companion to Mold Breaker (5.5)**

A handful of effects route through a battle script's `jumpifability`, which reads
**only the chosen ability slot**, so an *innate* holder is invisible to them.
Documented instances: an innate **Sticky Hold** doesn't block a Pickpocket steal
or Corrosive Gas (`INNATE_ABILITIES.md` ~L2048), and an innate **Own Tempo**'s
confuse-move immunity relies on the same chosen-slot read (~L1922). Making the
script `jumpifability` chokepoint innate-aware (as was already done centrally for
the Aroma-Veil *side* form, `Cmd_jumpifability` ~L2889) closes these. Do it
alongside **Mold Breaker (5.5)**, which pokes the same cross-slot machinery.

---

## Execution queue

**This is the source of truth for "the next batch."** Work strictly top-down; the
next batch is the first row still marked `open`. Front-loads the ~80 low-risk
calc/trait abilities before any new driver is built, and inserts each driver-build
as its own queue step so a batch is never reached before the hook it needs exists.
Mark a row `done` (in place, don't delete) when its PR merges.

| # | Queue step | Kind | Status |
| :-: | :-- | :-- | :-: |
| 1 | Batch A — Offensive move-power boosters | calc, no driver | done |
| 2 | Batch B — Defensive damage reducers | calc, no driver | done |
| 3 | Batch N — Status-conditional stat boosts | calc, no driver | done |
| 4 | Batch O — Crit-rate / crit-damage modifiers | calc, no driver | done |
| 5 | Batch P — Accuracy / type-eff / effect-chance | calc, no driver | done |
| 6 | Batch Q — Priority granters | calc, no driver | done |
| 7 | Batch R — Terrain modifiers | calc, no driver | done |
| 8 | Batch C — Double physical Attack | clone, no driver | done |
| 9 | Batch D+E — Stat-drop protection (fold E into D) | trait, no driver | done |
| 10 | Batch F — Priority-move block | clone, no driver | done |
| 11 | Batch G — Redirection-ignore | clone, no driver | done |
| 12 | Batch H — Trapping | trait, no driver | done |
| 13 | Batch I — Status-condition immunities | trait, no driver | done |
| 14 | Batch S — Misc single-site traits | trait, no driver | done |
| 15 | Batch J — End-of-turn effects | active, **existing** driver | done |
| 16 | Batch T — Berry/item synergy | active/trait | done |
| 17 | **Build the on-contact/on-hit driver** (model on the end-turn driver) | infra | done |
| 18 | Batch K — On-contact/on-hit/on-faint | active, needs step 17 | done (all 13: Rough Skin / Iron Barbs / Gooey / Tangling Hair / Aftermath / Innards Out / Steam Engine / Thermal Exchange / Wind Power / Cursed Body / Pickpocket / Magician / Liquid Ooze) |
| 19 | **Build the switch-in driver** (unblocks Intimidate + its immunity halves) | infra | done (shipped with Intimidate, the marquee consumer, like step 17 shipped with Rough Skin) |
| 20 | Batch L — Switch-in actives | active, needs step 19 | done (all 8: Intimidate / Anticipation / Forewarn / Frisk / Download / Supersweet Syrup / Unnerve / Hospitality) |
| 21 | Batch M — On-KO/on-hit stat boosts | active | done (all 11: Defiant / Competitive — the stat-drop-reaction pair, wired at BS_TryDefiantRattled; Justified / Stamina / Water Compaction / Anger Point — the on-hit stat-boost sub-group, reusing the Batch K on-hit driver; Rattled / Steadfast — the fear-response Speed pair (Rattled spans the on-hit driver + BS_TryDefiantRattled, Steadfast the CancelerFlinch site); Moxie / Berserk / Soul-Heart — the KO / on-damage / on-faint sub-group (Moxie reuses the attacker-side on-hit driver via ABILITYEFFECT_MOVE_END_FOES_FAINTED, Berserk adds a small on-damage driver at the new MOVEEND_COLOR_CHANGE_INNATE step, Soul-Heart is credited at the BS_TryActivateSoulheart command)) |
| 22 | Batch U — Ally-support (doubles) | calc/trait | done (all 5: Battery / Power Spot — partner damage boosters in CalcAttackStat; Telepathy — dodge ally move; Aroma Veil — side mental-status shield via the new IsInnateOnSide() + Cmd_jumpifability side cases; Flower Veil — Grass-ally status + stat-drop shield) |
| 23 | Batch V — Complete Guard Dog + Pastel Veil partial halves (Batch L driver now exists) | active, driver exists | done (Guard Dog Intimidate-immunity + Attack-boost wired at IsIntimidateBlocked; Pastel Veil switch-in ally-cure wired via the switch-in driver) |
| 23+ | Batch Y — Promoted-from-rejected clones (18, sub-groups Y1–Y8; Y1 cheapest, Y8 blocked on 5.5 Mold Breaker) | active/calc/trait, drivers exist | open (Y1 done: Chilling Neigh / Grim Neigh / Electromorphosis; Y2 done: Transistor / Dragon's Maw; Y3 done: Prism Armor / Shadow Shield / Neuroforce / Supreme Overlord; Y4 done: Full Metal Body / Mind's Eye; Y5 done: Purifying Salt / Good as Gold; Y6 done: Intrepid Sword / Dauntless Shield; Y7 done: Beast Boost; Y8 open, interleave with Tier 5) |
| 24 | Tier 5.1 — Mega Sol | one-off | open |
| 25 | Tier 5.2 — Quick Draw (determinism-sensitive) | one-off | open |
| 26 | Tier 5.3 — Comatose | one-off | open |
| 27 | Tier 5.4 — Magic Guard | one-off | open |
| 28 | Batch X — Script `jumpifability` innate-awareness (companion to Mold Breaker) | cross-cutting polish | open |
| 29 | Tier 5.5 — Mold Breaker | one-off | open |
| 30 | Tier 5.6 — Opportunist | one-off | open |
| 31 | Tier 5.7 — Mirror Armor | one-off | open |
| 32 | Tier 5.8 — Magic Bounce | one-off | open |
| 33 | Tier 5.9 — Dancer | one-off | open |
| 34 | Tier 5.10 — Flower Gift | one-off | open |
| 35 | Tier 5.11 — Aura Break (resolve as won't-wire, don't ship a dead clause) | disposition | open |
| ⟂ | Batch W — Frontier-slot freeing sweep (Step 3.5 backlog, J/T/K/L/M/U) | data cleanup, multi-session | open (parallel track) |

> Step 9 folds Batch E into D (same code block). Steps 24–35 are the **ordered**
> Tier 5 one-offs (see the [Tier 5 sub-queue](#tier-5--bespoke--deferred-one-off-multi-site-or-dependency-blocked)) —
> still **one ability per session/PR**, just no longer picked at random. Steps 23,
> 28 and Batch W are the **deferred cross-batch follow-ups** (sub-work on already
> `:white_check_mark:`/`:x:` abilities — see that section); they are **not** part of
> the 133 count. Batch W is a `⟂` parallel track (do it in its own sessions
> whenever, not a strict gate on Tier 5).

## Per-batch checklist (run the recipe's Definition of Done for each)

- [ ] **Step 1** — species rows per ability (canon users in any slot + tight
  flavor set; merge into existing rows where a species already has an innate).
- [ ] **Step 2** — add each ability to the allowlist comment in
  `src/fork/innate_abilities.c` + SCOPE note in `include/fork/innate_abilities.h`.
- [ ] **Step 3** — wire the effect at *every* site (`grep -n ABILITY_X src/`),
  **including AI effect reads** (`grep src/battle_ai_*.c`) **and the
  `DETERMINISTIC_*` reroutes** (PP-economy taxes, consume mirrors, gated
  additional effects — grep `DETERMINISTIC` around each effect site); confirm
  pure-boon-vs-1:1 and note any divergence.
- [ ] **Step 3.5** — `grep -n ABILITY_X src/fork/frontier_extended_mons.c` and free
  every hardcoded set (override-table rows for ability-locked species).
- [ ] **Step 4** — tests in `test/fork/innate_abilities.c`;
  `make check TESTS="FEATURE_INNATE_ABILITIES"`; full `make check` if a shared
  battle file was touched; ROM builds under `UNUSED_ERROR=1 DEPRECATED_ERROR=1`.
- [ ] **Step 5** — flip each row to `:white_check_mark:` in
  `INNATE_ABILITIES_PROGRESS.md`, update `FORK.md` and the
  [wiring reference](INNATE_ABILITIES.md#per-ability-wiring-reference), and mark
  the batch `done` (in place) in **both** the [Execution queue](#execution-queue)
  and the batch index below.

## Batch index (coverage checklist — all 133 pending abilities)

Mark batches `done` in place as they merge — never delete rows while any batch is
`open` (the count must keep summing to 133). Retire the whole doc only once every
row is `done`.

| Batch | Class | # | Status |
| :-- | :-- | :-: | :-: |
| A — Offensive move-power boosters | calc, AI-free | 14 | done |
| B — Defensive damage reducers | calc, AI-free | 7 | done |
| N — Status-conditional stat boosts | calc, AI-free | 5 | done |
| O — Crit-rate / crit-damage modifiers | calc, AI-free | 3 | done |
| P — Accuracy / type-eff / effect-chance | calc, AI-free | 6 | done |
| Q — Priority granters | calc, AI-free | 2 | done |
| R — Terrain modifiers | calc, AI-free | 2 | done |
| C — Double physical Attack | clone | 2 | done |
| D — Full stat-drop protection | trait (+AI) | 2 | done |
| E — Single-stat-drop protection | trait (fold into D) | 2 | done |
| F — Priority-move block | clone | 3 | done |
| G — Redirection-ignore | clone | 2 | done |
| H — Trapping | trait (+AI) | 3 | done |
| I — Status-condition immunities | trait | 6 | done |
| S — Misc single-site traits | trait | 12 | done |
| J — End-of-turn effects | active (existing driver) | 10 | done |
| K — On-contact/on-hit/on-faint | active (new driver) | 13 | done (Rough Skin / Iron Barbs / Gooey / Tangling Hair / Aftermath / Innards Out / Steam Engine / Thermal Exchange / Wind Power / Cursed Body / Pickpocket / Magician / Liquid Ooze) |
| L — Switch-in actives | active (new driver) | 8 | done (all 8: Intimidate — the switch-in driver was built with it — the Anticipation / Forewarn / Frisk information-reveal sub-group, the Download / Supersweet Syrup switch-in-stat-change sub-group, and the Unnerve / Hospitality sub-group, which extended the driver with a per-phase abilityEffect selector to reach the ABILITYEFFECT_UNNERVE and ABILITYEFFECT_DEPENDS_ON_ALLY cases) |
| M — On-KO/on-hit stat boosts | active | 11 | done (Defiant / Competitive; Justified / Stamina / Water Compaction / Anger Point; Rattled / Steadfast; Moxie / Berserk / Soul-Heart) |
| T — Berry/item synergy | active/trait | 4 | done |
| U — Ally-support (doubles) | calc/trait | 5 | done (Battery / Power Spot / Telepathy / Aroma Veil / Flower Veil) |
| Tier 5 — Bespoke/deferred | one-off | 11 | open (ordered 5.1–5.11; take top-down) |
| **Total** | | **133** | |

### Deferred cross-batch follow-ups (excluded from the 133)

Sub-work on abilities that are already `:white_check_mark:`/`:x:`, so **not**
counted above (they must not perturb the 133 tripwire). Tracked here so the plan is
complete; see the [Deferred cross-batch follow-ups](#deferred-cross-batch-follow-ups-not-pending-ability-rows)
section for detail.

| Follow-up | Kind | Scope | Status |
| :-- | :-- | :-- | :-: |
| V — Guard Dog + Pastel Veil partial halves | active (Batch L driver now exists) | 2 half-abilities | done |
| W — Frontier-slot freeing sweep | data cleanup, multi-session | ~40 J-sets + T/K/L/M/U tails | open (parallel track) |
| X — Script `jumpifability` innate-awareness | cross-cutting polish | Sticky Hold / Own Tempo cross-slot reads | open (do with Mold Breaker 5.5) |
| Y — Promoted-from-rejected clones | active/calc/trait (drivers exist) | 18 (see sub-groups) | open (Y1, Y2, Y3, Y4, Y5, Y6, Y7 done; Y8 open) |

**Promoted from rejected (Batch Y — 18).** Each was `:x:` only because its
driver/clone hadn't shipped when triaged; each is a clean pure-boon **clone of an
implemented (or, for Turboblaze/Teravolt, a pending) ability**, so the wiring is
near-free — reuse the existing site. Flipped to `:white_large_square:` in the
progress doc and counted here, **not** in the 133 (that tripwire is frozen to the
original pending set). The full rejection rationale — including why the *remaining*
`:x:` set stays rejected — lives in the wiring reference
[`INNATE_ABILITIES.md` → "Why some abilities are never wired"](INNATE_ABILITIES.md#why-some-abilities-are-never-wired-the-x-set)
(a durable doc, unlike this batches doc and the progress tracker, which are both
retired/deleted once the feature is complete).
Take Batch Y one sub-group per PR, like any other batch:

| Sub-group | Members | Clone of (site) | Notes |
| :-- | :-- | :-- | :-- |
| Y1 — On-KO / on-hit (Tier A, done first) ✅ DONE | Chilling Neigh, Grim Neigh, Electromorphosis | Moxie's Atk-on-KO / its Sp. Atk twin / Wind Power's charge-on-hit (minus wind gate) | Done: one-line additions to `IsActiveOnHitAttackerInnate` (neighs, shared `ABILITYEFFECT_MOVE_END_FOES_FAINTED`) + `IsActiveOnHitInnate` (Electromorphosis, shared `ABILITYEFFECT_MOVE_END`). Moxie-type AI reads credit an innate neigh via `IsMoxieTypeInnateActive`. Glastrier / Spectrier take fork overrides (Snow Warning / Infiltrator); Bellibolt freed to Static. |
| Y2 — Type-power boosters ✅ DONE | Transistor, Dragon's Maw | Steelworker / Rocky Payload (`CalcAttackStat`, Batch A) | Done: two 1:1 calc clones beside Rocky Payload — Transistor x1.3 (GEN_9+, else x1.5) Electric, Dragon's Maw x1.5 Dragon. AI-free (shared damage calc; no dedicated AI read). Sole-ability Regi legends take the innate + a fork chosen override so it's observable and the frontier set is freed: Regieleki -> Lightning Rod, Regidrago -> Adaptability. |
| Y3 — Damage / crit calc ✅ DONE | Prism Armor, Shadow Shield, Neuroforce, Supreme Overlord | Solid Rock / Multiscale / (SE-damage mirror of Tinted Lens) / flat power boost (Batch A/B/O) | Done: Prism Armor rides Filter/Solid Rock's -25%-vs-SE clause, Shadow Shield rides Multiscale's halve-at-full-HP clause (both unbreakable, so Mold Breaker can't pierce, unlike the Batch B cousins), Neuroforce is the offensive +25%-SE mirror of Tinted Lens. Supreme Overlord rides the Batch L switch-in driver to latch its +10%/fallen-teammate counter + pop-up, read back in CalcAttackStat. AI-free save the Shadow Shield full-HP-survival read. Sole-ability legends Necrozma forms / Lunala take the innate + a chosen Adaptability override (observable + frontier freed); Kingambit joins Defiant/Pressure, sets choose Defiant. |
| Y4 — Stat-drop / accuracy / hit traits ✅ DONE | Full Metal Body, Mind's Eye | Clear Body (Batch D) / Keen Eye + Scrappy (Batch P + S) | Done: Full Metal Body rides Clear Body's full stat-drop protection (GetInnateStatDropProtector / IsAbilityBlocked + the two AI reads) but is UNBREAKABLE (.breakable = FALSE, Mold Breaker can't pierce — for free via IsInnateActive); Mind's Eye combines Keen Eye (evasion-ignore + accuracy can't be lowered) and Scrappy (Ghost-hit), no Intimidate immunity. Sole-ability frontier legends Solgaleo / Ursaluna-Bloodmoon take the innate + a fork chosen override (Tough Claws / Unaware). |
| Y5 — Status immunities ✅ DONE | Purifying Salt, Good as Gold | status immunity (Batch I) + Ghost-resist (Batch B) / status-*move* immunity | Done: Purifying Salt (Nacli line) blocks every non-volatile status (catch-all in CanSetNonVolatileStatus, so the CanBe* AI callers are innate-aware for free) + halves Ghost damage (beside innate Thick Fat); its only real cost — blocking the holder's own Rest — is dropped (pure boon, Insomnia precedent). Good as Gold (Gholdengo) blocks status moves (CanAbilityAbsorbMove + 4 dedicated AI reads via BattlerHasAbility) — **very strong**, a deliberate balance divergence kept canon-only, called out in the allowlist + `FORK.md`. Both breakable. Gholdengo takes a chosen Sticky Hold override (observable + frontier freed); the salt line is a Batch W all-innate deferral. |
| Y6 — Switch-in stat boosts ✅ DONE | Intrepid Sword, Dauntless Shield | Atk/Def +1 first switch-in (Batch L switch-in driver) | Done: two one-line `SwitchInInnateAbilityEffect -> ABILITYEFFECT_ON_SWITCHIN` additions; the real once-per-battle latch (intrepidSwordBoost / dauntlessShieldBoost party-state flag, active under B_* >= GEN_9) and pop-up come for free. AI switch-in stat sim (`SetBattlerStatStagesForSwitchin`) mirrors each self-boost via `SpeciesHasInnate`. All four canon carriers (Zacian / Zacian-Crowned / Zamazenta / Zamazenta-Crowned) are sole-ability frontier sets, so each takes the innate + a fork chosen override (Zacian -> Tough Claws, Zamazenta -> Filter) so the innate is observable + the frontier set freed. |
| Y7 — On-KO best-stat ✅ DONE | Beast Boost | Moxie, best-stat edition | Done: one-line addition to `IsActiveOnHitAttackerInnate` (shares Moxie's `ABILITYEFFECT_MOVE_END_FOES_FAINTED` case, which already reads `GetHighestStatId`). AI credited via `IsMoxieTypeInnateActive` (Beast Boost added). Every canon user is a sole-Beast-Boost Ultra Beast, so the ten evolved/frontier UBs (Nihilego / Buzzwole / Pheromosa / Xurkitree / Celesteela / Kartana / Guzzlord / Naganadel / Stakataka / Blacephalon) take the innate + a fork chosen override (Merciless / Iron Fist / Tough Claws / Lightning Rod / Filter / Sharpness / Filter / Sheer Force / Solid Rock / Infiltrator) so it's observable + the frontier set freed; non-frontier pre-evo Poipole omitted as redundant. |
| Y8 — Mold Breaker clones | Turboblaze, Teravolt | **Mold Breaker (pending Tier 5.5)** | **Blocked on 5.5** — trivial clones once Mold Breaker ships; do them immediately after. |
