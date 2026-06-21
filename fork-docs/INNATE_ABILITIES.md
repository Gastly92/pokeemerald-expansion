# Innate abilities — how to add one

Fork feature, gated by `FEATURE_INNATE_ABILITIES` (`include/config/feature.h`).
A species' *innate* abilities are always active **in addition to** its single
chosen ability. This doc is the extension playbook; the flag comment and the
in-code comments (`include/fork/innate_abilities.h`, `src/fork/innate_abilities.c`) stay the
source of truth for exact semantics.

## The one rule: it's an allowlist, grown one ability at a time

Making an *arbitrary* ability work as an innate would mean routing every
"does this battler have ability X?" check through `BattlerHasAbility()` across
hundreds of upstream-owned sites — a large, perpetually merge-conflict-prone
sweep. We don't do that. Instead we **wire up one ability's behavior at a time**
and only let species declare innates from that supported set. Today the set is
**`LEVITATE`** (a passive Ground immunity), **`REGENERATOR`** (a silent
1/3-HP switch-out heal), **`UNAWARE`** (a passive calc modifier that ignores
the foe's stat-stage changes), **`STURDY`** (a full-HP endure + OHKO-move
immunity), **`NATURAL_CURE`** (a silent status cure on switch-out),
**`PRANKSTER`** (a +1 priority boost on status moves), the pinch abilities, the
weather speed-doublers, **`FILTER`**, **`PRESSURE`**, **`STENCH`**,
**`BATTLE_ARMOR`**/**`SHELL_ARMOR`**, and **`SPEED_BOOST`** (a +1 Speed boost at
the end of every turn — the first *active, scripted* end-turn innate, fired
through an end-turn driver; see the active-ability recipe below).

So a future request like *"add ability X as an innate; species A/B/C should have
it"* breaks into two parts:

1. **Data** — always generic, no engine work. Add the species → list mapping.
2. **Effect wiring** — how much is automatic depends on the *kind* of ability
   (see the recipe). This is the part the allowlist gates.

## The design principle: an innate is a *pure boon*

An innate is bonus value layered on top of a mon's chosen ability, so it should
**only ever help its holder — never carry the real ability's downside.** When the
real ability is a clean upside (Regenerator, Levitate's Ground immunity, **Sturdy** — it
never hurts its holder), the innate copies it 1:1. But when the real ability has a *cost* — a case where it would hurt
the holder — the innate keeps the upside and drops the cost. This is a deliberate,
suppression-independent divergence (`IsInnateActive()` still suppresses it exactly
like the real ability; only the *effect* diverges). Two worked examples:

- **Levitate** carries a hidden cost: a real Levitate forgoes the *beneficial* ground
  interactions (it can't soak field terrain or clear Toxic Spikes as a Poison-type).
  The innate keeps the Ground/​hazard immunity but stays grounded for those benefits
  (`IsBattlerGroundedForBenefit()`), so it's strictly a boon.
- **Unaware** carries a cost in the *drop* direction: a real Unaware blanks the foe's
  stat stage both ways, so it also ignores a foe's *drop* (e.g. an attacker that
  lowered its own Attack) and takes more damage / deals less for it. The innate ignores
  only the foe's *boosts* and keeps the foe's *drops* — always the favorable half
  (`InnateUnawareBoonStage()`).
- **Prankster** carries a cost in the *Dark-type* direction: a real Prankster's boosted
  status moves *fail* against Dark-types (Gen 7+), so it loses the ability to status them.
  The innate keeps the +1 priority but never sets `pranksterElevated` — the flag the
  Dark-type block keys off (`BlocksPrankster`) — so its status moves still land on
  Dark-types (`GetBattleMovePriority`, `src/battle_main.c`).

**When you wire a new ability, ask "does the real ability ever hurt its user?"** If
yes, wire the innate to skip that branch (and note the divergence in the allowlist
comment + `FORK.md`). If no, a 1:1 copy is already pure-boon.

## What the generic tooling already gives you (no per-ability work)

- **The species → innate table** (`src/fork/innate_abilities.c`): a variable-length,
  `ABILITY_NONE`-terminated list per species (no fixed cap). `SpeciesHasInnate()`
  and `GetSpeciesInnate()` read it.
- **The trait predicate** `BattlerHasAbility(battler, ability)` (`src/battle_util.c`):
  TRUE if `ability` is the battler's chosen ability **or** an active innate.
- **Suppression parity** via `IsInnateActive()`: an innate honors Gastro Acid,
  Neutralizing Gas, Mold Breaker (on breakable abilities), Ability Shield, and
  not-on-field exactly like the same ability in a real slot. (Suppression parity
  only — an innate's *effect* may diverge by design: see "an innate is a pure boon"
  above, where Levitate and Unaware are both intentionally a bit stronger than the real ability.)
- **Identity stays deterministic**: innates are *never* copied/swapped/displayed
  as identity. Trace, Skill Swap, Role Play, the ability pop-up, and
  `RecordAbilityBattle` all keep reading only the primary slot
  (`GetBattlerAbility`). Do **not** change that.

## Recipe: "add ability X as an innate for species A/B/C"

### Step 1 — add the data (always)

In `src/fork/innate_abilities.c`, give each species an `ABILITY_NONE`-terminated list.
Reuse one list for an evolution line:

```c
static const enum Ability sExampleLineInnates[] = { ABILITY_X, ABILITY_NONE };

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // ... existing rows ...
    { SPECIES_A, sExampleLineInnates },
    { SPECIES_B, sExampleLineInnates },
    { SPECIES_C, sExampleLineInnates },
};
```

**Who gets it — two groups (the established convention).** Every wired ability so far
populates the table in two passes, and new abilities should follow suit:

1. **Canon users (always).** Every species that carries ability X in its real ability
   data (`gSpeciesInfo[...].abilities[]`), in any slot — primary, secondary, or Hidden.
   Giving it as an innate lets them keep the signature behavior *no matter which slot a
   build picks*. List Mega/regional/Gmax/form constants **only where that form's ability
   data also carries X** (so the innate never appears on a form whose canon ability
   replaced it — e.g. a Mega whose ability becomes something else is omitted), so the
   innate survives a mid-battle form change where it should and not where it shouldn't.
2. **Flavor picks (optional, a judgment call).** A handful of species that *lack* the
   real ability but are strongly associated with its theme, so the innate is
   **observable** flavor (and is what most tests exercise). Precedent: Levitate's
   hover-by-design floaters, Unaware's too-dull/dazed/asleep mons, Sturdy's
   unbreakable-shell Shellder/Cloyster, Natural Cure's herbal/aromatic healers,
   Regenerator's regrowing Staryu/axolotls. Keep the set small and the theme tight, and
   **say so in the allowlist comment** — including a deliberate *"canon-only, no flavor
   picks"* decision when the ability is too strong or too hard to justify thematically
   (Prankster's `+1` priority is the worked example of opting out, then later adding a
   tight mischief-themed set: Hoopa, the Aipom line, the Zorua line).

A species may list several innates: `{ ABILITY_X, ABILITY_Y, ABILITY_NONE }`.

**One row per species — merge, don't duplicate.** `GetSpeciesInnateList()` returns
the *first* matching row, so a species can appear only once. If a species you're
adding is **already in the table** under a different innate (e.g. Quagsire already
carries an innate Regenerator), do **not** add a second row — it would be dead. Give
it a combined list and update the *existing* row in place:

```c
static const enum Ability sInnateRegeneratorUnaware[] = { ABILITY_REGENERATOR, ABILITY_UNAWARE, ABILITY_NONE };
// ... in the table, change the existing Quagsire row's list pointer to the combined one:
{ SPECIES_QUAGSIRE, sInnateRegeneratorUnaware }, // was sInnateRegenerator
```

Order within a list doesn't matter (membership lookups + display iterate the whole
list). Reuse one combined list across every line that needs the same pair.

### Step 2 — put the ability on the allowlist

Add `ABILITY_X` to the **allowlist comment** in `src/fork/innate_abilities.c` (and the
SCOPE note in `include/fork/innate_abilities.h` if the supported set's character
changes). This is the human record of what's actually wired; keep it honest.

### Step 3 — wire the effect (the only per-ability work)

**First, find *every* site.** `grep -n ABILITY_X src/` and wire each effect site —
an ability often reads in several scattered places, not one. "Single site" below is
the *easy class*, not a promise of one hit: Unaware took **four** (`battle_util.c`
damage calc ×2 + `GetTotalAccuracy` + `GetAccEvasionStageDelta`). Skip the pure
*identity* reads (`RecordAbilityBattle`, the ability pop-up's identity, the
single-valued `gAiLogicData->abilities[b]` *as the mon's displayed ability*) — those
stay the chosen-slot identity. Only the *effect* sites get the innate clause.

**Don't skip the AI's *effect* reads, though — and `grep src/battle_ai_*.c` for them
specifically.** This is the subtle one the early Unaware framing got wrong. The AI's
*damage/type* prediction runs through the **shared** `DamageContext` calc, so any
effect that lives *inside* that calc (Levitate's type immunity, Unaware's stat-ignore)
is correct for the AI **for free**, keyed off the real battler. But an effect that does
**not** live in the shared calc — an *event* effect (Regenerator's switch-out heal) or a
*survival* effect with its own predictor (Sturdy's endure / OHKO-immunity) — has the AI
reason about it in **dedicated helpers** that read `gAiLogicData->abilities[b] ==
ABILITY_X` (e.g. `CanEndureHit`, `BattlerHasMaxHPProtection`, the OHKO predictor, the
`battle_ai_switch.c` KO sim and pivot scoring). Those are **effect** reads, not identity
bookkeeping, and they are *not* covered by the shared calc — so they must be made
innate-aware (`BattlerHasAbility(b, X)` for an on-field battler; `SpeciesHasInnate(species,
X)` in the off-field switch sim, mirroring Levitate). Litmus test: *"if this mon's X were
innate-only, would the AI here still do the right thing?"* If the answer rides on a bare
`== ABILITY_X`, wire it. (Worked examples: **Sturdy** wired four AI sites — `CanEndureHit`,
the OHKO-move avoidance, `BattlerHasMaxHPProtection`, the switch-in KO sim — and **Regenerator**
wired its switch/pivot heuristics — `ShouldSwitchIfAbilityBenefit` (whose `switch(ability)` dispatch
needed a small pre-check + a factored-out helper so the innate is considered even when the chosen
ability differs), the bad-odds/hazard-switchin checks, and `ShouldPivot`.)

How much is needed depends on the ability class:

- **Passive trait checked at a single site** (the easy case — e.g. Soundproof,
  Oblivious, Sticky Hold, Run Away, Gorilla Tactics). Find the upstream check —
  usually `GetBattlerAbility(b) == ABILITY_X` — and change *that one comparison*
  to `BattlerHasAbility(b, ABILITY_X)`. Mark it `// FORK: innate-aware`. That's
  it; the predicate does the rest. (Watch for the cached-local idiom
  `enum Ability ability = GetBattlerAbility(b); ... ability == X` — there you
  either swap the comparison or add a `BattlerHasAbility` check alongside.)

- **Passive immunity / calc modifier** (like **Levitate**). The effect lives in a
  damage/grounding calc, so add an `IsInnateActive(battler, ABILITY_X)` (for an
  on-field battler) or `SpeciesHasInnate(species, ABILITY_X)` (off-field /
  species-level prediction, no battle state) clause next to the existing
  `ability == ABILITY_X` test. **Levitate is the fully-wired worked example** —
  use it as the checklist for what "full parity" touches:
  - core effect + markers + pop-up: `IsBattlerUngroundedByAbilityItemOrEffect`,
    `CalcTypeEffectivenessMultiplierInternal`,
    `CalcPartyMonTypeEffectivenessMultiplier` (`src/battle_util.c`). The type calc
    already sets `gLastUsedAbility`, calls `RecordAbilityBattle`, and forces
    `gBattleScripting.abilityPopupOverwrite` to the innate when it blocks.
  - **pure-boon divergence (Levitate-specific):** unlike a real Levitate, an innate
    Levitate keeps the mon grounded for the *beneficial* ground interactions —
    field terrain and Toxic Spikes absorption — while still floating over Ground
    moves and entry hazards. That's funnelled through `IsBattlerGroundedForBenefit()`
    (`src/battle_util.c` — grounded, or floating only by an innate Levitate), used at
    the terrain chokepoint (`IsBattlerTerrainAffected`), the Grassy-Terrain heal
    (`battle_end_turn.c`), and the Toxic-Spikes switch-in absorb (`battle_switch_in.c`).
    A real Levitate is excluded there, so it stays terrain-exempt/canon — only the
    innate is the boon.
  - AI: on-field damage/grounding is automatic (the AI runs the same calc keyed
    off the real battler), but off-field/partner sites need explicit help —
    `GetPartyMonAbilityForSwitchCalc`'s callers in `src/battle_ai_switch.c`
    (benched-mon absorb check), the partner-immunity switch in
    `src/battle_ai_main.c` (doubles spread-move scoring). Do **not** make the
    single-valued `gAiLogicData->abilities[]` / `AI_DecideKnownAbilityForTurn`
    return the innate — that array is the mon's chosen *identity*; query the
    innate alongside it instead.
  - facility/eligibility: Sky Battle (`CanMonParticipateInSkyBattle`,
    `src/battle_util.c`) and Battle Dome move rating (`GetEffectivenessPoints`,
    `src/battle_dome.c`).
  - displays: the summary screen Info page (`PrintMonAbilityName`,
    `src/pokemon_summary_screen.c`) and the Frontier battle-info viewer
    (`src/fork/frontier_battle_info.c`) already iterate the whole innate list, so they
    show any allowlisted ability with no per-ability work.

  **Unaware is the *minimal* calc-modifier worked example** — no pop-up, and no AI
  plumbing beyond what the shared calc gives for free. It needed only a small clause
  beside the four existing `ability == ABILITY_UNAWARE` comparisons in
  `src/battle_util.c`: the offensive and defensive stat-stage reads in the damage
  calc, plus the evasion/accuracy reads in `GetTotalAccuracy` and
  `GetAccEvasionStageDelta`. Each reads a cached `ctx->abilities[...]` / parameter,
  so the innate clause sits *alongside* the cached comparison (the cached-local idiom
  from the easy case). **But it is *not* a 1:1 copy — it's the pure-boon worked
  example for a calc modifier** (see "an innate is a pure boon" above): rather than
  blanking the foe's stat stage like the real ability (`stage = DEFAULT_STAT_STAGE`),
  the innate routes the stage through `InnateUnawareBoonStage()`, which caps it at
  default only when it's a *boost* and leaves a *drop* in place — so each site becomes
  `if (real Unaware) stage = DEFAULT; else stage = InnateUnawareBoonStage(b, stage);`.
  Because every Unaware site favors the *lower* stage for its holder, the same helper
  works at all four. Suppression parity is still automatic: Unaware is `breakable`, so
  an attacker's Mold Breaker drops both the chosen-ability path (`GetBattlerAbility`
  already returns `NONE`) and the innate (`IsInnateActive` → `CanBreakThroughInnate`).
  On-field AI damage prediction is correct for free because it reads the real
  battler's species through `IsInnateActive`. The off-field AI *setup* heuristics
  (the `AI_IsAbilityOnSide(ABILITY_UNAWARE)` "don't boost against an Unaware foe" sites
  in `ShouldRaiseAnyStat`, `GetAllyStatChangeScore`, and the Belly-Drum score, plus the
  Yawn evasion-dodge check in `battle_ai_switch.c`) **are** innate-aware: each pairs the
  chosen-ability read with the fork helper `AI_IsInnateOnSide()` (or `IsInnateActive()` at
  the single-battler switch site). They're all about the AI's own *boosts* being ignored,
  which an innate Unaware does exactly like the real one, so crediting the innate is correct.
  (This pattern — `AI_IsInnateOnSide()` beside `AI_IsAbilityOnSide()` — is the reusable way
  to make a side-level AI ability check innate-aware; Prankster's Psychic-Terrain heuristic
  uses the same helper.)

  **Sturdy is the *clean-upside* worked example for this class** — a multi-site passive
  immunity whose real ability has *no* downside, so the innate is a plain **1:1 copy** (no
  pure-boon divergence). It took an `IsInnateActive(b, ABILITY_STURDY)` clause beside the two
  cached `ABILITY_STURDY` reads in `src/battle_util.c`: the full-HP **endure** in
  `GetAdjustedDamage` (gated `B_STURDY >= GEN_5`) and the **OHKO-move immunity** in the OHKO
  accuracy gate. The "endured"/Sturdy pop-up & message need **no script** — they flow from the
  existing `MOVE_RESULT_STURDIED` / `MOVE_RESULT_ONE_HIT_KO_STURDY` flags. The one extra step a
  pop-up'd innate needs: because `CreateAbilityPopUp` reads the *primary* slot, set
  `gBattleScripting.abilityPopupOverwrite = ABILITY_STURDY` so the pop-up shows Sturdy and not the
  chosen ability — but **only when the chosen ability differs** (`cachedAbility != ABILITY_STURDY`),
  so the real-ability path stays byte-for-byte untouched. Same precedent as Levitate's
  `abilityPopupOverwrite`. Off-field AI survival heuristics are left unwired (the Unaware scope call).

- **Silent on-event effect** (fires at a single event site with no script /
  pop-up / animation — e.g. **Regenerator**, the worked example). No driver
  needed: find the upstream event site (for Regenerator, the switch-out handler
  `Cmd_switchoutabilities` in `src/battle_script_commands.c`) and add an
  `BattlerHasAbility(battler, ABILITY_X)` clause that applies the effect
  additively next to the chosen-ability path, guarded so the two don't
  double-apply. Watch the engine's constraints at that site — Regenerator writes
  the party mon's HP **directly** (`SetMonData(GetBattlerMon(b), MON_DATA_HP, …)`,
  exactly what the controller's `REQUEST_HP_BATTLE` does) rather than a second
  `BtlController_EmitSetMonData`, because the local switch-out command's single
  `bufferA` slot would otherwise clobber an emit a chosen ability already queued.

- **Active / on-event ability with a script** (fires a battle script on
  switch-in, end-of-turn, or on-contact — e.g. Intimidate, Speed Boost, Static,
  Rough Skin). These need a **driver** at the relevant event site plus a trigger
  hook. **Speed Boost is the live, worked example for the end-of-turn case** —
  use it as the reference rather than the older removed-from-history machinery:
  - driver: **`TryActivateInnateEndTurnEffects(battler, *index)`** (`src/fork/innate_abilities.c`)
    scans the species' innate list from `*index` and, for the first *active end-turn*
    innate (`IsActiveEndTurnInnate`) that is active (`IsInnateActive`) and not the chosen
    ability, delegates to the **existing** upstream end-turn handler with the innate
    passed explicitly: `AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate,
    MOVE_NONE, TRUE)`. Reusing the upstream case means the stat change / script /
    pop-up are identical to the real ability for free — the only edit at the effect
    site (`src/battle_util.c`) is forcing `gBattleScripting.abilityPopupOverwrite`
    to the innate when the chosen ability differs (the Sturdy/Levitate pop-up
    precedent), so the real-ability path stays byte-for-byte unchanged.
  - end-turn hook: a new `THIRD_EVENT_BLOCK_ABILITIES_INNATE` step
    (`include/constants/battle_end_turn.h`) inserted right after the chosen-ability
    block, dispatched in `HandleEndTurnThirdEventBlock` (`src/battle_end_turn.c`).
  - **re-entrancy:** the driver is re-entrant via a per-battler cursor
    (`endTurnInnateIndex` in `gBattleStruct->eventState`). Because a battle script fires
    one at a time, the hook **holds** the `THIRD_EVENT_BLOCK_ABILITIES_INNATE` step
    (keeping the cursor) while the driver returns TRUE, and only resets the cursor +
    advances the block once it returns FALSE (list exhausted). Each fired effect leaves
    the cursor past it, so a battler carrying **several** active end-turn innates fires
    each across successive passes of the loop. So adding a second end-turn active is just
    a one-line addition to `IsActiveEndTurnInnate` — no driver change needed.
  - **switch-in / on-contact actives** (Intimidate, Static, Rough Skin, …) still
    need their own event hooks; model them on the end-turn driver above (a per-event
    `TryActivateInnate…` that delegates to the matching `AbilityBattleEffects` case),
    not on the older `TryActivateInnateEffects` machinery that was removed from history.

### Step 3.5 — free the frontier roster slots (NOT optional — run the grep first)

**Do not skip this by assuming no set uses the ability — run the grep and let the
result decide.** This step is the easiest to glide past (it sits between the "real
work" of Step 3 and the wrap-up), but for any common ability it fires *hard*: the
pinch abilities (`OVERGROW`/`BLAZE`/`TORRENT`/`SWARM`) freed **55** sets across
every starter line. The extended frontier roster (`src/fork/frontier_extended_mons.c`)
is drafted under `B_FRONTIER_EXTENDED_MONS`, and many sets were built **before** an
ability became an innate, so they spent their single `.ability` slot on it. Once the
species carries the ability *innately*, that slot is better spent — free it to a
**complementary** chosen ability so the set runs both. This happened for Levitate
(commit `d5da59a3` freed Slowbro→`OWN_TEMPO`, Rotom→`LIGHTNING_ROD`, …), Unaware
(Clefable→`MAGIC_GUARD`, Pyukumuku→`INNARDS_OUT`, Dondozo→`WATER_VEIL`, …), and the
pinch abilities (Venusaur→`CHLOROPHYLL`, Charizard→`SOLAR_POWER`, Greninja→`PROTEAN`, …).

> **Pinch-ability caveat:** unlike a 1:1 innate (Sturdy etc.) where the chosen slot is
> *truly* redundant, a pinch innate uses a `chosen != ABILITY_X` guard in `CalcAttackStat`,
> so a set left on `.ability = ABILITY_BLAZE` keeps working but runs the *vanilla* (non-latched)
> boost and forgoes a second ability. Freeing it is therefore an *upgrade*, not dead-weight
> removal — but still do it, for consistency and the latch.

1. `grep -n ABILITY_X src/fork/frontier_extended_mons.c` for every set that hardcoded it.
2. For each, confirm the **species now carries the innate** (only those rows are
   freed — a set on a species *without* the innate must keep its real ability).
3. Replace `.ability = ABILITY_X` with a complementary ability and a short
   `// X now innate; chosen Y does Z` comment. The role comment on the `.heldItem`
   line (e.g. "Unaware wall") stays — it now describes the innate-backed playstyle.
4. **The replacement must be a real ability slot for that species** (`CreateFacilityMon`
   silently falls back to slot 0 otherwise). Pick from the species' own
   `gSpeciesInfo[...].abilities[]` — usually the Hidden Ability; skip a drawback ability
   (e.g. Durant's HA is `TRUANT` — use its `HUSTLE` slot instead, complementary means a *boon*).
5. **Ability-locked species** — where the `.ability` slot has nothing complementary to point
   at — take a row in the fork-owned override table `src/fork/species_ability_overrides.c`
   giving them a *flavorful* chosen ability in an empty slot, then set `.ability` to it. Two
   sub-cases: (a) the species' *only* real ability is the one now innate (Cornerstone Ogerpon =
   only Sturdy → `DEFIANT`; the innate-Levitate floaters Rotom/Hydreigon/the lake trio); and
   (b) ***all* of the species' real abilities are now innate** — the pinch case: the Fuecoco line
   is `{BLAZE, NONE, UNAWARE}` and *both* Blaze and Unaware are innate, so Skeledirge takes a
   chosen `CURSED_BODY` override. Either way the mon then runs that ability **and** the innate(s).
   This is also why such a species is **not** omitted from the innate table as "redundant":
   omission only applies to a sole-ability species that *isn't* in the roster (nothing observes
   its innate, so it'd be dead weight) — a sole-ability species that *is* a frontier set instead
   takes the innate + the override, so its slot pays off. `test/fork/frontier_extended_roster.c`
   fails CI if any `.ability` doesn't resolve to a real slot (through the override hook), so a bad
   pick can't slip through.

   > **Pick a *stable* chosen ability — cross-reference it against `INNATE_ABILITIES_PROGRESS.md`.**
   > Prefer an ability marked `:x:` there (rejected — it will *never* be wired as an innate: Lightning
   > Rod, Soundproof, Water Absorb, Sheer Force, …) over one still marked `:white_large_square:`
   > (pending). A `:white_large_square:` ability is on track to become an innate, and the moment it
   > does, *this* Step 3.5 sweep has to come back and re-point every override (and roster set) that
   > hands it out — so a pending pick is churn baked in, while a `:x:` pick is stable for good.
   > Sceptile→`LIGHTNING_ROD` is the model: Lightning Rod is `:x:`, so that override never needs
   > revisiting. (The whole table was audited on this rule — every row now hands out a `:x:` ability
   > except the two that hand out an already-implemented `:white_check_mark:` innate, Carnivine→
   > Chlorophyll and Tornadus-Therian→Prankster, which are likewise stable.) Separately, the slot a row *frees* must
   > already be redundant via an *implemented* (`:white_check_mark:`) innate — that's the row's whole
   > premise. A row may even repurpose a *real, non-empty* slot whose ability is dead weight on the
   > roster's sets (Sceptile's HA Unburden does nothing on its non-consumable-item sets, so with
   > Overgrow innately latched the slot is freed to `LIGHTNING_ROD`), so long as that freeing innate
   > (Overgrow) is implemented and the new chosen ability is itself stable (`:x:`).
6. Update the roster header's INNATE ABILITIES note to mention the new ability.

### Step 4 — test it

Add a case to `test/fork/innate_abilities.c`. Opt into the feature with
`WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE)` (the test baseline forces all
`FEATURE_*` flags off, so the inherited suite keeps exercising stock behavior).
Cover: the innate's effect fires; it does **not** fire with the feature off; and,
for trait/immunity abilities, that suppression (Gastro Acid / Mold Breaker) and
Trace/identity still behave like the real ability. Run:

```bash
make -j$(nproc) check TESTS="FEATURE_INNATE_ABILITIES"
```

The filtered run is for fast iteration. **Before pushing, run the *full* `make -j$(nproc) check`**
(not just `TESTS=`) whenever Step 3 touched a *shared* battle file — `battle_util.c`,
`battle_end_turn.c`, the `battle.h` structs, etc. — since those affect every battle, not just
the innate's. Build the ROM with CI's flags too (`UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -O all`).

### Step 5 — update the indexes

Two human-facing records, both fork-owned:

- **`FORK.md`** — edit the **Innate abilities** row: add the ability to the
  supported-set parenthetical in the *status* column, to the "Today the allowlist
  is …" sentence, and to the "Known limitations" list; note any new wiring.
- **`INNATE_ABILITIES_PROGRESS.md`** — flip the ability's row from
  `:white_large_square:` to `:white_check_mark:`.

## Why "mostly automatic" depends on the ability

Steps 1, 2, 4, 5 are mechanical for every ability; Step 3.5 fires whenever a
frontier set hardcoded the ability (run the grep — don't assume). Step 3 is the
variable: single-site passive traits are a one-line swap; calc-modifier passives are a
small clause; active abilities reuse the driver but need their trigger hook
restored. Keeping the allowlist small and explicit is what bounds the
upstream-file footprint and the merge-conflict surface — that is the whole point
of going ability by ability.

## Definition of Done (pre-push checklist)

Run through this every time — it exists because Step 3.5 and the full test run are
the two things easiest to skip:

- [ ] **Step 1** — species rows added (merged into existing rows where the species already has an innate).
- [ ] **Step 2** — allowlist comment in `src/fork/innate_abilities.c` + SCOPE note in `include/fork/innate_abilities.h` updated.
- [ ] **Step 3** — effect wired at *every* site (`grep -n ABILITY_X src/`), including the AI's *effect* reads (`grep src/battle_ai_*.c`); new battle-state fields zero-init with `gBattleStruct` and reset per battle.
- [ ] **Step 3.5 — ran `grep -n ABILITY_X src/fork/frontier_extended_mons.c`** and freed every hardcoded set (override-table rows for ability-locked / all-abilities-innate species). *This is the step that gets forgotten.*
- [ ] **Step 4** — tests added; `make check TESTS="FEATURE_INNATE_ABILITIES"` green; **full `make check` green** if a shared battle file was touched; ROM builds under `UNUSED_ERROR=1 DEPRECATED_ERROR=1`.
- [ ] **Step 5** — `FORK.md` (status parenthetical, allowlist sentence, known-limitations, wiring note **and** the frontier-freeing note) + `INNATE_ABILITIES_PROGRESS.md` flipped to `:white_check_mark:`.
