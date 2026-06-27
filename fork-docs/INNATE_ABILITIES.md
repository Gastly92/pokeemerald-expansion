# Innate abilities — how to add one

Fork feature, gated by `FEATURE_INNATE_ABILITIES` (`include/config/feature.h`).
A species' *innate* abilities are always active **in addition to** its single
chosen ability. This doc is both the extension playbook (below) **and** the
source of truth for the exact per-ability semantics — see the
[Per-ability wiring reference](#per-ability-wiring-reference) appendix at the
end. The compact allowlist in `src/fork/innate_abilities.c` and the SCOPE list
in `include/fork/innate_abilities.h` carry only the ability *names* and a
one-line gloss; the full record (sites, divergences, AI, species rationale)
lives here.

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
   build picks*. The lookup is keyed by **exact species with no base-species fallback**, so
   every form needs its own row to carry an innate (after a form change `gBattleMons[].species`
   becomes the form constant). **Mega forms** are populated as a **pure boon**: a Mega whose
   *base creature* has innates gets its own row mirroring the base's list, even if the Mega's
   own ability data replaced X (e.g. Mega Venusaur keeps Overgrow though its ability is Thick
   Fat) — the innate models the base creature's trait persisting through the Mega. **Grounded
   Megas are the exception**: omit (or trim) any innate the Mega physically can't have, e.g.
   Mega Gengar gets no row so it doesn't float, and Mega Mewtwo X keeps only Pressure (not
   Levitate). Regional/Gmax/forme constants outside the Mega convention are still listed only
   where that form's ability data carries X, so the innate survives a mid-battle form change
   where it should and not where it shouldn't.
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

## Per-ability wiring reference

This is the **source of truth for the exact semantics** of every allowlisted
ability: what it does, where it's wired, the deliberate pure-boon divergences,
the AI plumbing, and the species-selection rationale. It used to live as a giant
header comment in `src/fork/innate_abilities.c`; it was moved here to keep that
file's reads cheap. Each ability is a `###` block — grep this file for the
ability name (e.g. `### ABILITY_STURDY`) to read just the one you need rather
than the whole record. When you wire a new ability (Step 2 / Step 5), add or edit
its block here and keep the compact allowlist in
`src/fork/innate_abilities.c` + the SCOPE list in
`include/fork/innate_abilities.h` in sync (names only — the detail lives here).

### ABILITY_LEVITATE

Ground-immunity / ungrounding, handled in `src/battle_util.c`
(`IsBattlerUngroundedByAbilityItemOrEffect` and the type-effectiveness calc credit an
innate Levitate exactly like the real one). **DELIBERATE DIVERGENCE:** an innate Levitate is
a *pure boon*, NOT identical to a real Levitate. It still floats above Ground moves and
entry-hazard damage, but the fork also treats it as grounded for the *beneficial* ground
interactions — field terrain and Toxic Spikes absorption (via `IsBattlerGroundedForBenefit`,
`src/battle_util.c`). So an innate-Levitate mon reaps the terrain it/an ally sets and a
Poison-type still clears Toxic Spikes — things a real Levitate forgoes. This is why
terrain summoners (the Tapus, Miraidon) and Poison floaters happily carry the innate.

### ABILITY_REGENERATOR

Heals 1/3 max HP on switch-out, handled at the single switch-out
site in `src/battle_script_commands.c` (`Cmd_switchoutabilities`), additively alongside the
real Regenerator so a mon carrying it as an innate heals exactly like the real ability.
The heal is silent (no script/pop-up), so no driver is needed. Suppression parity holds:
the innate is gated by `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field),
same as the real ability's `GetBattlerAbility()` path. AI is innate-aware: the heal isn't in
any shared calc the AI runs, so the AI's dedicated Regenerator switch/pivot reads
(`ShouldSwitchIfAbilityBenefit`, the bad-odds and hazard-switchin checks in `src/battle_ai_switch.c`,
and `ShouldPivot` in `src/battle_ai_util.c`) credit an innate Regenerator via `BattlerHasAbility()`,
so the AI values an innate-only Regenerator's pivot heal. This populates the canon Regenerator
users so they keep their signature pivot heal regardless of which ability slot the build
picks, plus a few flavor regenerators (Staryu/Starmie's regrowing core, the axolotl Wooper
line, Zygarde's reassembling cells).

### ABILITY_UNAWARE

Ignores the foe's stat-stage changes in the damage and accuracy calcs,
handled in `src/battle_util.c` (the four calc sites that read `ABILITY_UNAWARE` — offensive and
defensive stat stages in the damage calc, plus evasion/accuracy in `GetTotalAccuracy` and
`GetAccEvasionStageDelta` — route the innate through `InnateUnawareBoonStage()` next to the
chosen-ability test). A pure calc-modifier passive like Levitate: no script / pop-up / driver.
Suppression parity holds via `IsInnateActive()` — Unaware is breakable, so an attacker's Mold
Breaker ignores an innate Unaware on the defender exactly as it would the real ability.
AI: on-field damage prediction is correct for free (the stat-ignore lives in the shared
damage calc, keyed off the real battler via `IsInnateActive`). The AI's off-field *setup*
heuristics — "don't bother boosting against an Unaware foe" (`ShouldRaiseAnyStat` and the Belly
Drum/half-HP-cost score in `battle_ai_main.c`), the doubles ally-stat-change score
(`GetAllyStatChangeScore`), and the Yawn evasion-dodge stay-in check (`battle_ai_switch.c`) —
read the chosen ability, so each now also credits an innate Unaware (`AI_IsInnateOnSide` beside
the `AI_IsAbilityOnSide` reads; `IsInnateActive` at the switch site). All are about the AI's own
*boosts* being ignored, which an innate Unaware (boost-ignoring) does just like the real one.
**DELIBERATE DIVERGENCE:** an innate Unaware is a *pure boon*, NOT identical to a real Unaware. A
real Unaware blanks the foe's stat stage in both directions (so it ignores a foe's *drop* too,
and takes more damage / deals less for it); the innate ignores only the foe's *boosts* and
keeps the foe's *drops* (always the favorable half — see `InnateUnawareBoonStage`, `battle_util.c`).
This populates the canon Unaware users so they keep the stat-ignore no matter which slot the
build picks, plus flavor picks too dull/dazed/asleep to notice the foe's buffs (Numel's
"doesn't notice being hit", the dazed Psyduck line, the ever-sleeping Komala, the unbothered
Snorlax line).

### ABILITY_STURDY

Endures a lethal hit at full HP (`B_STURDY >= GEN_5`) and is immune to OHKO
moves, handled at the two effect sites in `src/battle_util.c` (the `GetAdjustedDamage` endure and
the OHKO-move accuracy gate, each gets an `IsInnateActive()` clause beside the cached chosen-
ability test). NO pure-boon divergence: Sturdy is a clean upside that never hurts its holder,
so the innate is a 1:1 copy of the real ability. No driver/pop-up wiring is needed — the
"endured / Sturdy" messages and the ability pop-up flow from the existing `MOVE_RESULT_STURDIED`
/ `MOVE_RESULT_ONE_HIT_KO_STURDY` flags. Suppression parity holds via `IsInnateActive()`: Sturdy is
breakable, so an attacker's Mold Breaker pierces an innate Sturdy exactly as it would the real
ability. AI is innate-aware too: Sturdy's survival reasoning lives in DEDICATED AI helpers, NOT
the shared damage calc (so unlike Unaware it is NOT automatic and had to be wired) — the endure/KO
predictor (`CanEndureHit`), the OHKO-move avoidance, `BattlerHasMaxHPProtection` (`src/battle_ai_util.c`),
and the switch-in KO simulation (`src/battle_ai_switch.c`) each credit an innate Sturdy via
`BattlerHasAbility()`/`SpeciesHasInnate()`, so the AI doesn't blunder a hit it can't actually KO. This
populates the canon Sturdy users so they keep the signature endure no matter which slot the build
picks (Mega/regional/form constants are listed so the innate survives a mid-battle form change),
plus an "impenetrable shell" flavor line (Shellder/Cloyster, whose shell "even a missile can't
break") that lacks the real ability. Species whose ONLY ability is Sturdy are omitted as redundant
when unused by the frontier roster (Cosmoem, Togedemaru-Totem). Ogerpon-Cornerstone is the
exception — it is also sole-Sturdy, but because it IS a frontier set it instead takes the innate
AND a fork-owned chosen Defiant (`species_ability_overrides.c`), so its frontier slot isn't spent
on the now-innate Sturdy.

### ABILITY_NATURAL_CURE

Silently cures the holder's status1 on switch-out, handled at the
same single switch-out site as Regenerator in `src/battle_script_commands.c`
(`Cmd_switchoutabilities`), additively alongside the chosen Natural Cure path so a mon
carrying it as an innate self-cleanses exactly like the real ability. Like the innate
Regenerator there, it writes the party mon's status DIRECTLY (mirroring the controller's
`REQUEST_STATUS_BATTLE`) rather than a second `BtlController_EmitSetMonData`, so it can't
clobber the single bufferA slot a chosen ability (e.g. Slowking's Regenerator) may have
queued this switch-out. Silent (no script/pop-up), so no driver is needed. NO pure-boon
divergence: Natural Cure is a clean upside that never hurts its holder, so the innate is a
1:1 copy. Suppression parity holds via `IsInnateActive()`/`BattlerHasAbility()` (Gastro Acid /
Neutralizing Gas / not-on-field), same as the real ability's `GetBattlerAbility()` path. AI is
innate-aware: the cure isn't in any shared calc, so the AI's dedicated Natural Cure switch
reads credit an innate one via `BattlerHasAbility()` — the switch-to-cure heuristic
(`ShouldSwitchIfAbilityBenefit`, factored into `ShouldSwitchForNaturalCure` like Regenerator),
the Yawn anti-sleep switch (`ShouldSwitchIfBadlyStatused`), and the burned/frostbitten
force-switch move scoring (`src/battle_ai_main.c`). This populates the canon Natural Cure users
so they keep the signature self-cure no matter which slot the build picks, plus herbal/aromatic
healer flavor (the Chikorita line's restorative aroma, Bellossom's revitalizing dance).

### ABILITY_PRANKSTER

Gives the holder's status moves +1 priority, handled at the single
effect site in `src/battle_main.c` (`GetBattleMovePriority`): an `IsInnateActive()` clause sits
beside the chosen-ability `IsAbilityAndRecord()` test, so the boost applies for an innate
Prankster too. No script/pop-up/driver — priority is a pure turn-order calc. The AI gets it
for FREE: its turn-order prediction (`AI_WhoStrikesFirst` -> `GetBattleMovePriority`) runs the
same calc keyed off the real battler, so the AI both threatens and respects an innate
Prankster's priority. Suppression parity holds via `IsInnateActive()` (Gastro Acid /
Neutralizing Gas / not-on-field); Prankster is not breakable, so Mold Breaker never touches
it, same as the real ability. **DELIBERATE DIVERGENCE:** an innate Prankster is a *pure boon*,
NOT identical to a real Prankster. A real Prankster sets `gProtectStructs.pranksterElevated`,
which makes its boosted status moves FAIL against Dark-types (`B_PRANKSTER_DARK_TYPES >= GEN_7`);
the innate keeps the +1 priority but never sets that flag, so its status moves still land on
Dark-types — the favorable half, dropping the real ability's only cost. (Because the innate
never sets `pranksterElevated`, the AI's Dark-type avoidance check in `src/battle_ai_main.c`
correctly leaves an innate Prankster's status moves unpenalized — no wiring needed there.)
The doubles Psychic-Terrain heuristic in `src/battle_ai_field_statuses.c` IS made innate-aware
(Psychic Terrain blanks priority moves regardless of source): beside its chosen-only
`AI_IsAbilityOnSide(ABILITY_PRANKSTER)` reads, the fork helper `AI_IsInnateOnSide()` also credits
an innate Prankster, so the AI values/avoids the terrain for an innate-Prankster side too.
Two species groups: the canon Prankster users (the trickster lines keep the signature priority
no matter which slot the build picks; Mega/regional/Gmax forms are listed only where the form's
ability data ALSO carries Prankster — Grimmsnarl-Gmax yes; Banette/Sableye/Meowstic Megas and
the Therian formes have a DIFFERENT signature ability, so they are omitted like the Natural Cure
rule), plus a deliberately small, on-theme flavor set lacking the real ability (Hoopa the
"Mischief Pokémon," the playful Aipom line, the illusion-trickster Unovan Zorua line — the
flavor set is narrower than other abilities' because Prankster's +1 priority is potent).
Cottonee/Whimsicott, Klefki and Hoopa are also innate-Levitate floaters, so they take the
combined `INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER)` list.

### ABILITY_OVERGROW / ABILITY_BLAZE / ABILITY_TORRENT / ABILITY_SWARM

The "pinch" abilities:
+50% to Grass/Fire/Water/Bug moves respectively while the holder is low on HP. Handled by an
additive block in `CalcAttackStat` (`src/battle_util.c`), beside (not inside) the chosen-ability
switch. **DELIBERATE DIVERGENCE:** an innate pinch ability is a *pure boon* that LATCHES. A real
pinch ability only boosts while the holder is *currently* <=1/3 HP, so healing back up (notably
an innate Regenerator's switch-out heal, but also Leftovers / a Berry) strips the boost; the
innate instead sets `gBattleStruct`'s per-mon `reachedPinchHp` flag the first time the holder hits
<=1/3 HP (latched each end-of-turn in `src/battle_end_turn.c`) and keeps the boost for the rest of
the battle. The chosen-ability switch case is left untouched (a real pinch ability stays vanilla),
and the block's `chosen != ABILITY_X` guard means a starter running its real pinch ability never
double-applies. Suppression parity holds via `IsInnateActive()` (feature flag + Gastro Acid /
Neutralizing Gas / not-on-field); pinch abilities aren't breakable, so Mold Breaker never touches
them, same as the real ability. AI is correct for FREE: the boost lives in the shared damage calc
(`CalcAttackStat`), which the AI runs keyed off the real battler via `IsInnateActive()`, so it both
threatens and respects an innate pinch boost. Canon-only (no flavor picks): every species whose
ability data carries the pinch ability in any slot, so the signature survives whichever slot a
build picks (a Chlorophyll Venusaur / Solar Power Charizard / Protean Greninja keeps its boost);
forms are listed only where the form's ability data still carries it (Megas swap to Thick Fat /
Tough Claws / Drought / Mega Launcher / etc. and are omitted; Gigantamax forms and the Hisuian
starters keep theirs). The Bulbasaur, Chikorita and Fuecoco lines and Volbeat already carry other
innates, so they take a combined `INNATES(...)` list with the pinch ability added.

### ABILITY_SWIFT_SWIM / ABILITY_CHLOROPHYLL / ABILITY_SAND_RUSH / ABILITY_SLUSH_RUSH

The weather
speed-doublers: x2 Speed in rain / harsh sun / sandstorm / snow respectively (Sand Rush also
shrugs off sandstorm chip damage, like the real ability). Handled at the single speed-calc site
`GetBattlerTotalSpeedStat` (`src/battle_main.c`): each `ability == ABILITY_X` test gains an
`|| IsInnateActive(battler, ABILITY_X)` clause, so an innate holder doubles exactly like the real
ability. Sand Rush's sandstorm-damage immunity is mirrored at the end-turn damage site
(`src/battle_end_turn.c`) and the AI's two sandstorm-damage predictors (`DoesBattlerTakeSandstormDamage`
in `src/battle_ai_util.c`, `GetSwitchinWeatherImpact` in `src/battle_ai_switch.c`). NO pure-boon divergence:
a weather speed-doubler is a clean upside that never hurts its holder, so each innate is a 1:1 copy.
Suppression parity holds via `IsInnateActive()` (none of the four is breakable, so Mold Breaker never
touches them — same as the real ability). AI is innate-aware: turn-order prediction runs the same
`GetBattlerTotalSpeedStat` keyed off the real battler, so the AI both threatens and respects an innate
doubler's speed for FREE (innates are species-derived, so this never leaks a hidden chosen ability);
the AI's weather-SETTING heuristics (`DoesAbilityBenefitFromWeather` in `src/battle_ai_field_statuses.c`,
`DoesAbilityBenefitFromSunOrRain` in `src/battle_ai_main.c`) also credit an innate doubler so the AI sets
the matching weather to enable it. Canon-only (no flavor picks — a x2-Speed weather sweeper is potent,
so like the pinch abilities the set stays to species whose ability data carries it in any slot): the
signature survives whichever slot a build picks (a Rain Dish Ludicolo / Sand Force Excadrill keeps its
doubling), and forms are listed only where the form's ability data still carries it (Mega Swampert,
Gigantamax Drednaw/Venusaur, Hisuian Qwilfish/Lilligant/Overqwil, the seasonal Deerling/Sawsbuck).
Beartic carries BOTH Swift Swim (primary) and Slush Rush (HA), so it takes the combined pair. Many
species already carry other innates (the Bulbasaur/Tangela/Bellossom/Cottonee/Psyduck/Relicanth/...
lines), so they take a combined `INNATES(...)` list with the speed-doubler added.

### ABILITY_FILTER

Reduces the damage the holder takes from supereffective moves by 25%, handled
at the single defensive calc site `GetDefenderAbilitiesModifier` (`src/battle_util.c`): an
`IsInnateActive()` clause beside the chosen-ability Filter / Solid Rock / Prism Armor switch case
applies the 0.75 modifier (guarded against those three so it never double-applies, and stacking
correctly with any other defender-ability modifier). A pure calc-modifier passive like Unaware:
no script / pop-up / driver, and the innate is NOT recorded as identity. NO pure-boon divergence:
Filter is a clean upside that never hurts its holder, so the innate is a 1:1 copy of the real
ability. Suppression parity holds via `IsInnateActive()`: Filter is breakable, so an attacker's Mold
Breaker pierces an innate Filter exactly as it would the real ability. AI is correct for FREE: the
reduction lives in the shared damage calc the AI runs keyed off the real battler (like Unaware's
stat-ignore), so the AI both threatens and respects an innate Filter on-field; the off-field
switch-in damage prediction is left unwired (the Unaware scope call — a 25% reduction is not a
KO-flipping immunity like Levitate/Sturdy). Canon-only (no flavor picks — the Filter theme is hard
to attribute beyond its real users): every species whose ability data carries Filter in any slot
(Mr. Mime and Mime Jr.'s slot-1 Filter, Revavroom's HA, Mega Aggron whose Mega ability data is
Filter), so the signature survives whichever slot a build picks. Mega Aggron already carries innate
Sturdy (persisting from base Aggron), so it takes the combined `INNATES(STURDY, FILTER)` list.

### ABILITY_PRESSURE

Makes the holder's foes spend 1 extra PP per move used against it, handled at
the two PP-deduction sites that read `ABILITY_PRESSURE`: the real deduction in `CancelerPPDeduction`
(`src/battle_move_resolution.c`) and the fork-owned deterministic PP-refund mirror in `src/battle_util.c`
(both the spread-move loop and the single-target branch swap `GetBattlerAbility(x) == ABILITY_PRESSURE`
for `BattlerHasAbility(x, ABILITY_PRESSURE)`). A pure passive trait checked at a single kind of site:
no script / pop-up / driver, and the innate is NOT recorded as identity (the cosmetic "exerting its
Pressure!" switch-in message still fires only for the chosen ability, like all innate announcements).
NO pure-boon divergence: Pressure only ever costs the FOE extra PP, so it never hurts its holder —
the innate is a 1:1 copy of the real ability. Suppression parity holds via `BattlerHasAbility()` ->
`IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); Pressure is not breakable, so Mold
Breaker never touches it, same as the real ability. AI needs no wiring: nothing in `src/battle_ai_*.c`
reads `ABILITY_PRESSURE` for an effect (the PP tax isn't modeled in the AI's damage/turn calcs), so an
innate Pressure is exactly as (in)visible to the AI as a real one. Canon-only (no flavor picks — the
"exerts pressure" theme is hard to attribute beyond its real users, and the +1 PP tax is a potent
stall tool): every species whose ability data carries Pressure in any slot, so the signature survives
whichever slot a build picks (Aerodactyl/Aggron-style slot-2/HA Pressure included). Forms are listed
only where the form's ability data still carries Pressure (Giratina-Origin/Dialga-Origin/Palkia-Origin
keep it; the Galarian birds, the Mega/Kyurem-B/W and Mewtwo-Mega-Y forms swap to a different signature
and are omitted). Mewtwo (innate Levitate), Ho-Oh (innate Regenerator), Dusclops, the Deoxys formes
and Giratina-Altered (all innate Levitate) already carry an innate, so they take a combined
`INNATES(...)` list with Pressure added.

### ABILITY_STENCH

On a damaging hit, a 10% chance to make the target flinch (under
`DETERMINISTIC_ABILITIES`: a guaranteed flinch on the holder's first turn out, like a King's
Rock entry flinch). Handled at the single on-hit site in `src/battle_util.c`
(`ABILITYEFFECT_MOVE_END_ATTACKER`): the chosen-ability switch keys off `gLastUsedAbility`, so an
innate Stench whose chosen ability differs is run additively in a pre-check beside the switch
(`TryStenchFlinch`, guarded `chosen != ABILITY_STENCH` so a real Stench never flinches twice).
No script/pop-up/driver — the flinch flows through `SetMoveEffect(MOVE_EFFECT_FLINCH)`, and the
innate is NOT recorded as identity (no ability pop-up, exactly like the real Stench, which has
none either). NO pure-boon divergence: Stench only ever flinches the FOE, so it never hurts its
holder — the innate is a 1:1 copy of the real ability. It still doesn't stack with a King's Rock
flinch — `TryKingsRock` (`src/battle_hold_effects.c`) pre-empts the holder's own flinch item via a
`BattlerHasAbility(ATK, STENCH)` guard, made innate-aware so an innate holder's item bows out (and
isn't consumed) exactly like a chosen Stench's. The flinch is also still blocked by Shield Dust /
Covert Cloak, same as the real ability (those checks live in the shared flinch path, not the
ability dispatch). Because Stench sets the flinch via `SetMoveEffect` (not `TryTriggerAdditionalEffect`),
it bypasses `DETERMINISTIC_FLINCH`'s anti-lock cap exactly like the real ability / King's Rock / Fake
Out — no `flinchedLastTurn` check. Suppression parity holds via
`IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); Stench is not breakable, so Mold
Breaker never touches it, same as the real ability. AI needs no wiring: nothing in `src/battle_ai_*.c`
reads `ABILITY_STENCH` (the AI doesn't model the flinch chance), so an innate Stench is exactly as
(in)visible to the AI as a real one. Two species groups: the canon Stench users (Grimer/Muk,
Koffing/Weezing's HA, Stunky/Skuntank, the Trubbish/Garbodor line incl. Gmax, and Gloom's HA — each
keeps the signature flinch no matter which slot the build picks; Galarian Weezing swaps its HA to
Misty Surge and is omitted), plus a tight foul-odor flavor set lacking the real ability (Oddish and
Vileplume completing the canon Gloom line — the "smells atrocious" weed line — and the Gulpin line's
poison-gas bags). No frontier roster sets hardcoded Stench, so none needed freeing.

### ABILITY_BATTLE_ARMOR / ABILITY_SHELL_ARMOR

The two crit-immunity abilities (identical effect:
attacks landed on the holder are never critical hits), handled at the two crit-calc sites in
`src/battle_util.c` (`CalcCritChanceStage` and the Gen-1 `CalcCritChanceStageGen1`): each gains a clause
beside the cached chosen-ability test that forces `critChance = CRITICAL_HIT_BLOCKED` for an innate
holder too. A pure passive immunity checked at a single kind of site (like Sturdy): no script /
pop-up / driver. NO pure-boon divergence: crit immunity is a clean upside that never hurts its
holder, so each innate is a 1:1 copy of the real ability. The innate is NOT recorded as identity —
only the chosen-ability path calls `RecordAbilityBattle` (the innate blocks silently, like Filter),
so Trace/Skill Swap/the ability pop-up still read the chosen slot. Suppression parity holds via
`IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); neither is breakable, so Mold
Breaker never touches them, same as the real ability. AI is innate-aware: nothing in the shared
damage calc needs it (the calc's crit result already runs through `CalcCritChanceStage` keyed off the
real battler, so a predicted crit against an innate holder is correctly blocked for FREE), and the
one dedicated AI read — "don't waste Laser Focus on a crit-immune target" (`EFFECT_LASER_FOCUS` in
`src/battle_ai_main.c`) — now credits an innate holder via `BattlerHasAbility()`. Canon-only (no flavor
picks — crit immunity is hard to theme beyond an actual armored shell, and the set is already
large): every species whose ability data carries Battle Armor or Shell Armor in any slot, so the
signature survives whichever slot a build picks. Forms are listed only where the form's ability
data still carries it: Slowbro-Mega and Scolipede-Mega gain Shell Armor (their Mega ability data is
Shell Armor, replacing the base's), the Hisuian Sliggoo/Goodra carry Shell Armor where the base
forms carry Hydration instead (so only the Hisui rows get it), and the Drednaw-Gmax / Kingler-Gmax /
Lapras-Gmax forms keep theirs. Many species already carry other innates (Kabuto/Kabutops/Omastar/
Anorith/Armaldo's Swift Swim, the Turtwig and Oshawott lines' Overgrow/Torrent, Shellder/Cloyster/
Dwebble/Crustle's Sturdy, Slowbro-Mega's Regenerator, Escavalier's Swarm, Chewtle/Drednaw's Swift
Swim, Klawf's Regenerator), so they take a combined `INNATES(...)` list with the armor added. NOTE:
base Slowbro/Samurott-Hisui are intentionally NOT given the armor — their data carries Regenerator /
Sharpness there, not the armor, so only the form whose data actually carries it gets the innate.
Frontier roster sets that hardcoded an armor are freed (Step 3.5): Omastar/Kabutops keep their real
Weak Armor, Drapion its Sniper, Goodra-Hisui its Sap Sipper, Drednaw its Strong Jaw, while the
all-real-abilities-now-innate species take a fork-owned chosen override (Armaldo/Samurott → Water
Absorb, Torterra → Sand Stream, Turtonator → Flame Body) in `species_ability_overrides.c`.

### ABILITY_SPEED_BOOST

Raises the holder's Speed by 1 stage at the end of every turn. This is the
fork's first ACTIVE, scripted end-turn innate, so unlike the passive abilities above it needs an
end-turn driver: `TryActivateInnateEndTurnEffects` (below in `innate_abilities.c`) is hooked from the
`THIRD_EVENT_BLOCK_ABILITIES_INNATE` step of the end-turn loop (`src/battle_end_turn.c`), right after
the chosen-ability end-turn block. The driver delegates to the upstream end-turn handler with the
innate ability passed explicitly — `AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, ABILITY_SPEED_BOOST, ...)`
— so the +1 Speed, the stat-change script (`BattleScript_AbilityStatChange`) and the pop-up are
identical to the real ability; the pop-up is overridden to show Speed Boost (not the chosen
ability) at the effect site in `src/battle_util.c`, but ONLY when the chosen ability differs, so a
real Speed Boost stays byte-for-byte unchanged (Sturdy/Levitate precedent). The driver skips an
innate that equals the chosen ability so a real Speed Boost never boosts twice. The driver is
RE-ENTRANT (a per-battler cursor in `gBattleStruct->eventState`, see `TryActivateInnateEndTurnEffects`),
so a battler can carry several active end-turn innates and fire each in turn; Speed Boost is just
the only one on the allowlist today. NO pure-boon
divergence: Speed Boost is a clean upside that never hurts its holder, so the innate is a 1:1 copy.
Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field);
Speed Boost is not breakable, so Mold Breaker never touches it, same as the real ability. AI is
innate-aware: the boost isn't in any shared damage calc, but the two AI reads that key off a foe's
Speed Boost — "don't bother lowering an innate Speed Boost foe's Speed" in `CanLowerStat`
(`src/battle_ai_util.c`) and `CanStatChange` (`src/battle_stat_change.c`) — now credit an innate holder
via `IsInnateActive()`; the ability-transfer scoring (`BattlerBenefitsFromAbilityScore`) is left alone
since innates are never transferable. Canon-only (no flavor picks — a +1 Speed-per-turn snowball is
potent, like the pinch / weather-doubler / Pressure abilities): every species whose ability data
carries Speed Boost in any slot, so the signature survives whichever slot a build picks. Forms are
listed only where the form's ability data still carries it (Blaziken-Mega's ability data IS Speed
Boost, so it gets the innate; Sharpedo-Mega → Strong Jaw and Scolipede-Mega → Shell Armor are
omitted for Speed Boost). The Torchic line (innate Blaze) and the Venipede line (innate Swarm)
already carry an innate, so they take a combined `INNATES(...)` list with Speed Boost added.
Frontier roster sets that hardcoded Speed Boost are freed (Step 3.5): Ninjask → Infiltrator,
Sharpedo → Rough Skin, Yanmega → Tinted Lens, Scolipede → Poison Point, Espathra → Opportunist
(each a real, complementary slot), while Blaziken — whose only real abilities (Blaze, Speed Boost)
are now BOTH innate — takes a fork-owned chosen Sheer Force override in `species_ability_overrides.c`.

### ABILITY_LIMBER

The holder cannot be paralyzed, handled at the paralysis-immunity sites in
`src/battle_util.c`: the block site in `CanSetNonVolatileStatus` (the `MOVE_EFFECT_PARALYSIS` branch
gains an `IsInnateActive()` clause beside the chosen-ability test, and when an innate Limber — chosen
ability differs — blocks the paralysis it reassigns `abilityDef` to LIMBER and overwrites the pop-up
so the "protected by Limber" message/record shows Limber, the Levitate/Sturdy pop-up precedent), and
the switch-in cure site in `TryImmunityAbilityHealStatus` (an innate Limber cures pre-existing paralysis
on switch-in like the real ability, again with the pop-up overwritten to Limber). Also mirrored at the
out-of-battle Battle Pike status room (`DoesAbilityPreventStatus`, `src/battle_pike.c`) so an innate-Limber
party mon shrugs off the Pike's paralysis room exactly like a real Limber. A pure passive immunity
checked at a single kind of site (like Sturdy/Filter): no driver. NO pure-boon divergence: paralysis
immunity is a clean upside that never hurts its holder, so the innate is a 1:1 copy of the real ability.
Suppression parity holds via `IsInnateActive()`: Limber is breakable, so an attacker's Mold Breaker
pierces an innate Limber exactly as it would the real ability. AI is innate-aware FOR FREE: the AI's
paralysis reasoning runs through `CanBeParalyzed()`/`AI_CanParalyze()` -> `CanSetNonVolatileStatus()`, whose
fork clause reads `IsInnateActive(battlerDef, ABILITY_LIMBER)` keyed off the real on-field battler (not
the passed-in `abilityDef`), so the AI correctly never tries to paralyze an innate-Limber foe; nothing in
`src/battle_ai_*.c` reads `ABILITY_LIMBER` directly. Two species groups: the canon Limber users (the agile
cats Persian/Glameow, the boneless contortionists Hitmonlee/Hawlucha/Clobbopus/Graploct, the flexible
rabbits Buneary/Lopunny, the formless Ditto, and Stunfisk — each keeps the para-immunity no matter which
slot the build picks; Mega Lopunny → Scrappy and Galarian Stunfisk → Mimicry swap their data and are
omitted), plus a tight supple-serpent flavor set lacking the real ability (the coiling snakes Ekans/Arbok
and Seviper, whose limber bodies fit the theme). Purrloin/Liepard already carry innate Prankster and
Mareanie/Toxapex already carry innate Regenerator, so they take a combined `INNATES(...)` list with Limber
added. Frontier roster sets that hardcoded Limber are freed (Step 3.5): Persian → Technician, Lopunny →
Cute Charm, Liepard → Unburden, Toxapex → Merciless, Graploct → Technician (each a real, complementary
slot the now-innate Limber freed).

### ABILITY_CUTE_CHARM

When the holder is hit by a contact move, a 30% chance to infatuate the
attacker if they are of opposite genders (under `DETERMINISTIC_ABILITIES`, a guaranteed infatuation
regardless of gender). Wired innate-aware at the `ABILITYEFFECT_MOVE_END` on-hit site in
`src/battle_util.c`: the chosen-ability dispatch keys off the target's `gLastUsedAbility`, so an innate
Cute Charm whose chosen ability differs is run additively in a pre-check beside the switch
(`TryCuteCharmInfatuate`, guarded `!= ABILITY_CUTE_CHARM` so a real Cute Charm never infatuates twice).
The effect runs the same `BattleScript_CuteCharmActivates` (pop-up + infatuation), so the one extra
step a pop-up'd innate needs is forcing `gBattleScripting.abilityPopupOverwrite = ABILITY_CUTE_CHARM`
when the chosen ability differs (the Limber/Speed Boost pop-up precedent), so the pop-up shows Cute
Charm and not the chosen ability; a real Cute Charm stays byte-for-byte unchanged. NO pure-boon
divergence: Cute Charm only ever infatuates the FOE, so it never hurts its holder — the innate is a
1:1 copy of the real ability. Suppression parity holds via `IsInnateActive()` (Gastro Acid /
Neutralizing Gas / not-on-field); Cute Charm is not breakable, so Mold Breaker never touches it, same
as the real ability. AI is innate-aware: the only AI read of `ABILITY_CUTE_CHARM` is the
`DETERMINISTIC_ABILITIES` contact-punish predictor `AI_DeterministicContactAbilityPunishes`
(`src/battle_ai_util.c`), which now also credits an innate Cute Charm on the defender via
`BattlerHasAbility()` so the AI treats contact as a downside even when the chosen ability differs;
under non-deterministic play neither a real nor an innate Cute Charm is modeled, so parity holds.
Canon-only (no flavor picks — infatuation can fully disable a foe for a turn, a potent disruption,
so like Prankster / the pinch abilities the set stays to species whose ability data carries Cute
Charm in any slot): the signature survives whichever slot a build picks (Milotic's HA Cute Charm,
Stufful's HA, Skitty/Delcatty/Minccino/Cinccino/Lopunny/Sylveon/Enamorus, the Clefairy and
Jigglypuff lines). Forms are listed only where the form's ability data still carries Cute Charm:
Buneary (Run Away/Klutz/Limber), Bewear (Fluffy/Klutz/Unnerve), Enamorus-Therian (Overcoat) and
Clefable-Mega lack it in their data and are omitted; only base Lopunny / Stufful's own line member /
Enamorus-Incarnate carry it. Clefable already carries innate Unaware and Lopunny innate Limber, so
they take a combined `INNATES(...)` list with Cute Charm added. Frontier roster sets that hardcoded a
chosen Cute Charm are freed (Step 3.5): Enamorus → Contrary (its real HA), while Lopunny — whose only
real non-drawback abilities (Cute Charm, Limber) are now BOTH innate — takes a fork-owned chosen
Sheer Force override in `species_ability_overrides.c` (its slot-2 Limber, now innate-redundant). Audino
is intentionally NOT given the innate: its ability data lacks Cute Charm (Healer/Regenerator/Klutz);
it only runs Cute Charm as a fork-chosen ability via the override table, so its frontier set is left
to keep that chosen Cute Charm and needs no freeing.

### ABILITY_OBLIVIOUS

The holder cannot be infatuated or Taunted (`B_OBLIVIOUS_TAUNT >= GEN_6`) and
is unaffected by Intimidate (`B_UPDATED_INTIMIDATE >= GEN_8`). A passive trait checked at several
scattered immunity sites; no script/pop-up driver. Wired innate-aware at: the Attract infatuation
block (`Cmd_setdrowsy`/Attract in `src/battle_script_commands.c`), the Taunt block (`Cmd_settaunt`), the
generic infatuation setter (`BS_TrySetInfatuation`), the Captivate stat-drop immunity (`EFFECT_CAPTIVATE`
in `src/battle_stat_change.c`), the Intimidate immunity (`IsIntimidateBlocked` in `src/battle_stat_change.c`),
the Cute-Charm self-infatuation check on the contacting attacker (`src/battle_util.c`), and the switch-in
cure of pre-existing infatuation/Taunt (`TryImmunityAbilityHealStatus` in `src/battle_util.c`). Each pairs
the chosen-ability test with `IsInnateActive()`/`BattlerHasAbility()`; the visible blocks (Attract, Taunt,
Captivate, Intimidate, switch-in cure) overwrite the pop-up to Oblivious when the chosen ability differs
(the Limber/Cute Charm pop-up precedent), so a real Oblivious stays byte-for-byte unchanged. NO pure-boon
divergence: Oblivious is a clean upside that never hurts its holder, so the innate is a 1:1 copy.
Suppression parity holds via `IsInnateActive()`: Oblivious is breakable, so an attacker's Mold Breaker
pierces an innate Oblivious exactly as it would the real ability. AI is innate-aware: the foe-side reads
are credited via `IsInnateActive()`/`BattlerHasAbility()` — `AI_CanBeInfatuated` (don't Attract an innate-Oblivious
foe), `CanIntimidateLowerOpponentAtk` (don't switch in an Intimidator against one, in `src/battle_ai_switch.c`),
and the Cute-Charm contact-punish predictor's attacker-Oblivious check (`src/battle_ai_util.c`). The AI's Taunt
scoring does not model Oblivious immunity even for the real ability, so it needs no innate wiring (parity).
Canon-only (no flavor picks): the canon roster — the perpetually-dazed Slowpoke/Numel/Spheal lines, the
clueless Smoochum/Jynx, the spaced-out Swinub line, etc. — already embodies the "oblivious" theme, so no
extra flavor picks are warranted (keeping the set tight). Every species whose ability data carries Oblivious
in any slot gets it, so the immunity survives whichever slot a build picks; forms are listed only where the
form's data still carries it (the Slowpoke line's Galarian/Mega forms swap to Own Tempo/Shell Armor and are
omitted; Tsareena loses Oblivious on evolving, so only Bounsweet/Steenee get it). Many species already carry
other innates (the Slowpoke line's Regenerator, Illumise's Prankster, the Wailmer line's Pressure, Numel's and
Dondozo's Unaware, Feebas's Swift Swim), so they take a combined `INNATES(...)` list with Oblivious added.
Frontier roster sets that hardcoded a chosen Oblivious are freed (Step 3.5): Whiscash → Hydration (its real HA),
while Dondozo — whose only non-Water-Veil real abilities (Unaware, Oblivious) are now BOTH innate — takes its
real Water Veil slot (burn immunity), no override needed.

### ABILITY_SAND_VEIL / ABILITY_SNOW_CLOAK

The two weather evasion abilities: +25% evasion (a 0.8
accuracy modifier on incoming moves) while their weather is up — sandstorm for Sand Veil, hail/snow
for Snow Cloak — and immunity to that weather's chip damage (sandstorm / hail), exactly like the
real abilities. Wired as passive calc-modifiers (like Filter / Unaware): the evasion lives at the
accuracy site `GetTotalAccuracy` (`src/battle_util.c`), applied additively beside the chosen-ability
switch (guarded `defAbility != ABILITY_X` so a real holder never applies the 0.8 twice), plus the
deterministic PP-tax mirror `GetDeterministicMoveTargetPPTax` in the same file. The weather-damage
immunity is mirrored at the end-turn chip sites (`src/battle_end_turn.c`), exactly as the innate Sand
Rush's sandstorm immunity already is. No script / pop-up / driver, and the innate is NOT recorded as
identity. NO pure-boon divergence: both are clean upsides that never hurt their holder, so each
innate is a 1:1 copy of the real ability. Suppression parity holds via `IsInnateActive()`: both are
breakable, so an attacker's Mold Breaker pierces an innate Sand Veil / Snow Cloak exactly as it
would the real ability (and the move then ignores the evasion). AI is innate-aware: on-field
accuracy prediction is correct for FREE (`GetTotalAccuracy` runs keyed off the real battler), and the
dedicated weather-damage / weather-setting reads credit the innate too — the sandstorm/hail chip
predictors (`DoesBattlerTakeSandstormDamage` / `DoesBattlerTakeHailDamage` in `src/battle_ai_util.c`, the
switch-in `GetSwitchinWeatherImpact` in `src/battle_ai_switch.c`) and the weather-setting heuristic
(`DoesInnateBenefitFromWeather` in `src/battle_ai_field_statuses.c`, so the AI values setting the matching
weather to enable the evasion). The off-field accuracy prediction is left unwired (the Unaware/Filter
scope call — a 25% evasion boost is not a KO-flipping immunity). The overworld wild-encounter-rate
halving (`src/wild_encounter.c`) reads the party lead's chosen ability only and is deliberately left
alone: innates are a battle-only feature (no battle state out of battle). Canon-only (no flavor
picks — evasion is a contentious, can-be-frustrating mechanic, so like Prankster / the potent
abilities the set stays tight to species whose ability data carries it in any slot): the signature
survives whichever slot a build picks (Garchomp's slot-1 Sand Veil, Gliscor's, Donphan's HA, etc.).
Forms are listed only where the form's ability data still carries it (Garchomp-Mega-Z keeps Sand Veil,
but the regular Garchomp-Mega swaps to Sand Force and is omitted; Sandaconda-Gmax keeps Sand Veil;
Vanilluxe / Cetitan / Tyranitar lose it on evolving and are omitted, so only the pre-evos get it).
Many species already carry other innates (Sandshrew/Sandslash's Sand Rush, the Geodude line and
Donphan's Sturdy, Stunfisk's Limber, the Swinub line's Oblivious, Articuno's Pressure, the Vanillite
line and Froslass's Levitate, the Sandshrew-Alola line / Cubchoo / Beartic's Slush Rush), so they take
a combined `INNATES(...)` list with the evasion ability added. Frontier roster sets that hardcoded a
chosen Sand Veil / Snow Cloak are freed (Step 3.5): Glaceon → Ice Body, Froslass → Cursed Body and
Wugtrio → Gooey each take a real complementary slot, while the species whose ALL relevant real
abilities are now innate take a fork-owned chosen override (`species_ability_overrides.c`) — Sandslash
and Donphan → Sand Stream, Sandslash-Alola / Articuno / Beartic → Snow Warning — each a stable `:x:`
weather-setter that also turns on the mon's own evasion innate.

### ABILITY_COMPOUND_EYES / ABILITY_KEEN_EYE / ABILITY_ILLUMINATE

The accuracy abilities, all wired as
passive calc-modifiers (like Unaware / Filter). THE FORK SETS `DETERMINISTIC_ACCURACY_EVASION`, so a
move's accuracy never decides hit/miss — instead accuracy/evasion stages are a per-use PP economy
(see `config/deterministic.h`). What each ability does there:

- The fork models all three (Compound Eyes' real +30% accuracy is repurposed to match Keen Eye /
  Illuminate's real ignore-the-target's-evasion) as IGNORE THE TARGET'S EVASION, so the holder is
  never PP-taxed by an evasive foe. Wired at the two accuracy sites in `src/battle_util.c` — the raw
  hit calc `GetTotalAccuracy` and the deterministic PP-economy delta `GetAccEvasionStageDelta` (the latter
  read by both the real deduction in `CancelerPPDeduction` and the AI's PP mirror). PURE BOON at both:
  the innate ignores only a foe's evasion *boost* (`evasionStage > default`), keeping a foe's evasion
  *drop* in the holder's favor — like `InnateUnawareBoonStage`. That boost guard also keeps the
  `IsInnateActive` lookups off the common no-boost path (the AI runs the hit calc constantly), so there
  is no measurable AI-thinking-time cost.
- Keen Eye / Illuminate ADDITIONALLY keep the holder's own accuracy from being lowered, wired at the
  stat-drop site `IsAbilityBlocked` (`src/battle_stat_change.c`), beside the chosen-ability path, with the
  pop-up/record overwritten to the innate (Limber/Oblivious precedent) so it shows Keen Eye / Illuminate.

Illuminate's in-battle effect only exists under `B_ILLUMINATE_EFFECT >= GEN_9` (the fork's default), so
every innate Illuminate clause mirrors that gate; below Gen 9 an innate Illuminate is inert in battle
(it would only affect the overworld encounter rate, which innates deliberately never touch). NO pure-boon
divergence: all three are clean upsides that never hurt their holder, so each innate is a 1:1 copy.
Suppression parity via `IsInnateActive()`: Compound Eyes isn't breakable (and its holder is the attacker,
so Mold Breaker never touches it); Keen Eye / Illuminate are breakable, so an attacker's Mold Breaker
pierces the innate accuracy-drop immunity exactly as it would the real ability. The innate is NOT
recorded as identity for the silent evasion-ignore (a pure calc modifier like Unaware). AI: the PP-economy
delta lives in the shared `GetAccEvasionStageDelta` keyed off the real battler, so the AI both threatens and
respects an innate evasion-ignore for FREE; the one dedicated AI read — "don't bother lowering an innate
Keen Eye / Illuminate foe's accuracy" in `CanLowerStat` (`src/battle_ai_util.c`) — is made innate-aware. The
ability-transfer scoring (`BattlerBenefitsFromAbilityScore`, the Compound Eyes case) is left alone since
innates are never transferable, and the overworld held-item-rarity / encounter-rate reads (Compound Eyes
in `src/pokemon.c`, Keen Eye in `src/battle_pike.c` & `src/wild_encounter.c`) are deliberately untouched —
innates are a battle-only feature, the same call made for Sand Veil's wild-encounter halving. CANON-ONLY
(no flavor picks — accuracy/evasion is a contentious mechanic, like the weather-evasion abilities, so the
set stays tight to species whose ability data carries it): every species whose data carries Compound Eyes
(the Butterfree/Venonat/Yanma/Dustox/Nincada/Joltik/Galvantula/Scatterbug-Vivillon/Blipbug-Dottler/Rellor
lines), Keen Eye (the many bird lines plus Sneasel/Sableye/Skunk/Drapion/Lycanroc/...), or Illuminate
(Staryu/Starmie, Chinchou/Lanturn, Volbeat, Watchog, the Morelull line) keeps it no matter which slot the
build picks. Forms are listed where the form's data still carries it (Butterfree-Gmax keeps Compound Eyes;
Sneasel-Hisui / Braviary-Hisui keep Keen Eye), and per the FORMS convention the Megas mirror the base as a
pure boon (Pidgeot-Mega keeps Keen Eye though its Mega data is No Guard; Sableye-Mega keeps the base's
Keen Eye + Prankster though its Mega data is Magic Bounce). Watchog carries BOTH Keen Eye and Illuminate.
Many species already carry other innates (Yanma's Speed Boost; the Joltik/Galvantula/Blipbug/Dottler
Swarm; Staryu/Starmie's Natural Cure + Regenerator; Volbeat's Prankster + Swarm; Sableye/Sableye-Mega/
Meowstic-M's Prankster; Skarmory's Sturdy; Glameow's Limber; Stunky/Skuntank's Stench; Lycanroc-Midday's
Sand Rush), so they take a combined `INNATES(...)` list with the accuracy ability added. Frontier roster
sets that hardcoded one of these are freed (Step 3.5): Butterfree → Tinted Lens, Galvantula → Unnerve,
Pidgeot → Big Pecks, Fearow → Sniper, Furret → Frisk, Lycanroc → Steadfast (each a real complementary
slot), while the species whose ALL relevant real abilities are now innate take a fork-owned chosen
override (`species_ability_overrides.c`) — Skarmory → Bulletproof and Volbeat → Victory Star, each a
stable `:x:` pick. (Sableye is the exception: its only free real slot is Stall, a drawback that the
vanilla Stall tests rely on, so it can't be overridden — its roster sets keep a redundant-but-harmless
chosen Keen Eye instead.)

### ABILITY_INSOMNIA / ABILITY_VITAL_SPIRIT / ABILITY_SWEET_VEIL

The sleep-immunity abilities: the
holder (and, for Sweet Veil, its whole side) cannot be put to sleep or made drowsy (Yawn). All three
are wired at ONE central chokepoint — the `MOVE_EFFECT_SLEEP` case of `CanSetNonVolatileStatus`
(`src/battle_util.c`) — which every sleep path funnels through: direct sleep moves and Yawn (via
`Cmd_trynonvolatilestatus`), secondary sleep effects (`SetMoveEffect`), Effect Spore / G-Max Snooze (via
`CanBeSlept`), and the AI's `AI_CanPutToSleep` (also `CanBeSlept`). Insomnia / Vital Spirit gain an
`IsInnateActive(battlerDef, ...)` clause beside the chosen-ability test (reassigning `abilityDef` + the
pop-up overwrite when the chosen ability differs, the Limber/Oblivious precedent); Sweet Veil gains a
side-wide `IsInnateOnSide()` clause beside its `IsAbilityOnSide()` test (same pop-up handling). One more
manual site: the end-turn drowsy->sleep (`HandleEndTurnYawn`, `src/battle_end_turn.c` — its inline
Insomnia/Vital Spirit gate becomes `!BattlerHasAbility(...)` and its Sweet Veil side check gains
`IsInnateOnSide`). Also mirrored at the out-of-battle Battle Pike sleep room (`DoesAbilityPreventStatus`,
`src/battle_pike.c`) for innate Insomnia/Vital Spirit, like the Limber precedent. **DELIBERATE PURE-BOON
DIVERGENCE:** a real Insomnia/Vital Spirit/Sweet Veil also BLOCKS the holder's own Rest (it can't sleep),
a *cost*; the innate intentionally does NOT block Rest (the `EFFECT_REST` gate in
`src/battle_move_resolution.c` and the Sweet-Veil Rest gate in `BS_JumpIfAbilityPreventsRest` are left
chosen-ability-only), so the innate keeps the upside (enemy-sleep immunity) and drops the cost — a mon
may still Rest fully (heals + sleeps normally). The corollary: unlike the innate Limber/Oblivious, the
innate Insomnia/Vital Spirit deliberately does NOT wire the switch-in self-cure of pre-existing sleep
(`TryImmunityAbilityHealStatus`). That hook fires post-move too, so curing the holder's sleep would
un-sleep a fresh Rest the same turn (a free, sleepless Recover); dropping it is what keeps Rest a clean
pure boon. The rare sleep-while-suppressed case (slept under Mold Breaker, then suppression ends) simply
runs its normal sleep counter down instead of auto-curing. AI is innate-aware
for FREE via the shared `CanBeSlept` chokepoint (`AI_CanPutToSleep` won't try to sleep an innate-immune
foe); the redundant hardcoded Sweet Veil cases in `AI_CheckBadMove` (`src/battle_ai_main.c`) are left
chosen-only since `AI_CanPutToSleep` already covers them (Insomnia/Vital Spirit have no such case even for
the real ability). Suppression parity via `IsInnateActive()`: all three are breakable, so an attacker's
Mold Breaker pierces an innate one exactly as the real ability. The overworld wild-encounter read of
Vital Spirit (`src/wild_encounter.c`) is deliberately untouched (innates are battle-only). CANON-ONLY (no
flavor picks — sleep immunity is a strong defensive boon and this is a 4-ability batch, so the set stays
tight to species whose ability data carries it in any slot): every such species keeps the immunity no
matter which slot a build picks, with forms listed where their own data carries it (Galarian Mr. Mime's
Vital Spirit; Megas mirror the base as a pure boon — Banette-Mega keeps Insomnia+Levitate). Alcremie's
60+ cosmetic cream/sweet form constants are omitted to avoid bloating the table (Sweet Veil is its
slot-0 default ability, so it keeps it regardless; the single pre-evo Milcery carries the innate), and
sole-Sweet-Veil Ribombee-Totem is omitted (sole-ability + not in the roster, the Togedemaru-Totem
precedent). Mewtwo-Mega-Y (its data is Insomnia in all three slots, so the chosen ability is always
Insomnia) is left as its existing Levitate row — an innate Insomnia would be dead weight. Many species
already carry other innates (Hoothoot/Noctowl's Keen Eye, Spinarak/Ariados's Swarm, Murkrow's Prankster,
the Shuppet/Pumpkaboo/Gourgeist Levitate, Capsakid/Scovillain's Chlorophyll, Rockruff/Lycanroc-Midnight's
Keen Eye, Bounsweet/Steenee's Oblivious), so they take a combined `INNATES(...)` list. Frontier roster sets
that hardcoded a chosen Insomnia / Vital Spirit / Sweet Veil are freed (Step 3.5) to a complementary real
slot where one exists, or a fork-owned chosen override (`species_ability_overrides.c`) where all the
species' real abilities are now innate.

### ABILITY_EARLY_BIRD

The holder wakes from sleep twice as fast (its sleep counter drops by 2 per turn).
A clean-upside pure boon (1:1 copy), wired at the two real sleep-counter sites — the move-use wake check
in `CancelerForSleep` (`src/battle_move_resolution.c`) and the per-turn wake check in `src/battle_util2.c` —
each gains an `IsInnateActive()`/`BattlerHasAbility()` clause so `toSub` becomes 2 for an innate holder too
(the move-resolution site uses `IsInnateActive` beside `IsAbilityAndRecord`, so the innate is NOT recorded as
identity). AI is made innate-aware at its three Early Bird reads: the wake-turn predictor `IsWakeupTurn`
and the Rest-value heuristic (`src/battle_ai_util.c` / `src/battle_ai_main.c`) and the Yawn stay-in switch
heuristic (`src/battle_ai_switch.c`). No script/pop-up/driver. Suppression parity via `IsInnateActive()`
(Early Bird is not breakable, so Mold Breaker never touches it, same as the real ability). CANON-ONLY
(no flavor picks, matching the batch): every species whose data carries Early Bird in any slot, forms
where their data carries it (Megas mirror the base as a pure boon — Houndoom-Mega / Kangaskhan-Mega keep
Early Bird). Many already carry other innates (Ledyba/Ledian's Swarm, the Sunkern/Seedot/Nuzleaf/Shiftry
Chlorophyll), so they take a combined `INNATES(...)` list.

### ABILITY_IMMUNITY / ABILITY_PASTEL_VEIL

The poison-immunity abilities: the holder (and, for
Pastel Veil, its whole side) cannot be poisoned or badly poisoned. Both are wired at the single
chokepoint every poison path funnels through — the `MOVE_EFFECT_POISON` / `MOVE_EFFECT_TOXIC` cases
of `CanSetNonVolatileStatus` (`src/battle_util.c`) — which Immunity gains as an `IsInnateActive()`
clause beside the chosen-ability test (reassigning `abilityDef` + overwriting the pop-up to Immunity
when the chosen ability differs, the Limber/Insomnia precedent), and Pastel Veil gains as a
side-wide `IsInnateOnSide()` clause beside its `IsAbilityOnSide()` test (same pop-up handling). Also
wired into `TryImmunityAbilityHealStatus` (`src/battle_util.c`) so an innate Immunity / Pastel Veil
cures the holder's own pre-existing poison/toxic on switch-in like the real ability, same pop-up
overwrite. Also mirrored at the out-of-battle Battle Pike poison room (`DoesAbilityPreventStatus`,
`src/battle_pike.c`) and at the AI's Toxic Spikes switch-in damage prediction (`IsSwitchinTSpikesAffected`
and the Toxic Spikes branch of `GetSwitchinHazardsDamage`, `src/battle_ai_switch.c` — both gain a
`SpeciesHasInnate()` / `AI_IsInnateOnSide()` clause beside the real-ability checks, the Sturdy
precedent in the same file) so the AI never predicts phantom Toxic Spikes poison damage for an
innate-immune switch-in candidate. NO pure-boon divergence: poison immunity is a clean upside that
never hurts its holder, so both innates are a 1:1 copy of the real ability. Suppression parity
holds via `IsInnateActive()` / `AI_IsInnateOnSide()`: both are breakable, so an attacker's Mold Breaker
pierces an innate Immunity/Pastel Veil exactly as it would the real ability. **KNOWN LIMITATION:** the
real Pastel Veil's switch-in ALLY-cure (`BattleScript_PastelVeilActivates`, looping self+partner to
cure pre-existing poison on the holder's switch-in) is NOT replicated for an innate holder — that
would need a brand-new generic "active switch-in ability with a script" driver, for which no
precedent exists yet (only an end-turn equivalent, Speed Boost's `TryActivateInnateEndTurnEffects`);
an innate Pastel Veil still cures and blocks the holder's OWN poison via the chokepoints above, just
not its ally's pre-existing poison on switch-in. CANON-ONLY (no flavor picks): Immunity goes to
Gligar (combined with its innate Sand Veil), Snorlax/Snorlax-Gmax (combined with innate Unaware),
and Zangoose, each whose real ability data carries Immunity in some slot; Pastel Veil goes to
Galarian Ponyta/Rapidash, the only species whose real ability data carries it. Frontier roster sets
that hardcoded Pastel Veil are freed (Step 3.5): Rapidash-Galar → Anticipation (its real Hidden
Ability slot).

### ABILITY_THICK_FAT

Halves the damage the holder takes from Fire- and Ice-type moves, handled at the
single effect site in `src/battle_util.c` (`CalcAttackStat`, the "target's abilities" switch that
applies the defender's move-type damage modifiers): an `IsInnateActive()` clause beside the chosen-
ability `ABILITY_THICK_FAT` case applies the same `x0.5` to a Fire/Ice move, guarded `chosen !=
ABILITY_THICK_FAT` so a real Thick Fat never double-halves. A pure calc-modifier passive like Filter:
no script / pop-up / driver, and the innate is NOT recorded as identity (`RecordAbilityBattle` stays on
the chosen-ability path). NO pure-boon divergence: Thick Fat is a clean upside that never hurts its
holder, so the innate is a 1:1 copy of the real ability. Suppression parity holds via `IsInnateActive()`:
Thick Fat is breakable, so an attacker's Mold Breaker pierces an innate Thick Fat exactly as it would
the real ability. AI is correct for FREE: the reduction lives in the shared damage calc (`CalcAttackStat`)
the AI runs keyed off the real battler (like Filter's / Unaware's reads), so the AI both threatens and
respects an innate Thick Fat on-field; the off-field switch-in damage prediction is left unwired (the
Unaware/Filter scope call — a 50% Fire/Ice reduction is not a KO-flipping immunity like Levitate/Sturdy).
Two species groups: the canon Thick Fat users (every species whose ability data carries it in any slot,
so the signature survives whichever slot a build picks — the Marill/Azumarill, Seel/Dewgong, Spheal line,
Swinub line, Makuhita line, Spoink line, Miltank, Purugly, Snorlax line, Alolan Rattata line, Tepig/Pignite,
Appletun, Lechonk line, and Cetoddle line, plus Mega Venusaur whose Mega ability data is Thick Fat — listed
only where the form's ability data carries it; Raticate-Alola-Totem, sole-Thick-Fat and unused by the roster,
is omitted as redundant like the Sturdy precedent), plus a tight blubber-themed flavor pick lacking the real
ability (the Wailmer/Wailord whale line — insulated by thick blubber against heat and cold). Frontier roster
sets that hardcoded Thick Fat are freed (Step 3.5): Raticate-Alola → Gluttony (eats its Sitrus early),
Dewgong/Walrein → Ice Body, Snorlax → Gluttony (Immunity also innate), Miltank → Sap Sipper, Hariyama/Cetitan
→ Sheer Force, Grumpig → Own Tempo, Purugly → Defiant, Appletun → Ripen, and the Mamoswine sets — whose three
real abilities (Oblivious/Snow Cloak/Thick Fat) are ALL now innate — take a fork-owned chosen Snow Warning
override (`species_ability_overrides.c`), self-synergistic with their innate Snow Cloak.

### ABILITY_TECHNICIAN

Boosts the holder's moves of base power 60 or less by 50%, handled at the single
effect site in `src/battle_util.c` (`CalcMoveBasePowerAfterModifiers`, the attacker-abilities switch
that applies move-power modifiers): an `IsInnateActive()` clause beside the chosen-ability
`ABILITY_TECHNICIAN` case applies the same `x1.5` when `basePower <= 60`, guarded `chosen !=
ABILITY_TECHNICIAN` so a real Technician never double-boosts. A pure calc-modifier passive like Filter /
Thick Fat: no script / pop-up / driver, and the innate is NOT recorded as identity (`RecordAbilityBattle`
stays on the chosen-ability path). NO pure-boon divergence: Technician is a clean upside that never hurts
its holder, so the innate is a 1:1 copy of the real ability. Suppression parity holds via `IsInnateActive()`:
Technician is NOT breakable, so Mold Breaker never touches it (same as the real ability) — Gastro Acid /
Neutralizing Gas / not-on-field are the relevant suppressors. AI is correct for FREE: the boost lives in the
shared move-power calc the AI runs keyed off the real battler (like Filter's / Thick Fat's reads, and unlike
Sturdy's dedicated survival helpers), so the AI both threatens and respects an innate Technician on-field; no
dedicated `== ABILITY_TECHNICIAN` AI reads exist, so nothing else needs wiring. Canon-only (no flavor picks):
a flat +50% on every weak move is potent, so like the pinch / weather-speed abilities the set stays to species
whose ability data carries Technician in any slot, so the signature survives whichever slot a build picks (the
Meowth/Persian lines incl. their Alolan forms, the Scyther/Scizor line incl. Mega, Hitmontop, Smeargle, the
Mr. Mime / Mime Jr. line, Breloom, Roserade, Kricketune, Ambipom, the Minccino/Cinccino line, Marshadow, the
Toxtricity forms incl. Gigantamax, the Clobbopus/Grapploct line, the Maushold forms, and Fezandipiti). Many of
these already carry other innates (Persian's Limber, Mr. Mime's / Mime Jr.'s Filter, the Scyther/Scizor line's
Swarm, Kricketune's Swarm, Roserade's Natural Cure, Ambipom's Prankster, the Minccino/Cinccino line's Cute
Charm, the Clobbopus/Grapploct line's Limber), so they take a combined `INNATES(...)` list with Technician
added. Frontier roster sets that hardcoded Technician are freed (Step 3.5): Persian-Alola → Fur Coat, Kanto
Persian → Unnerve, Mr. Mime → Soundproof, Scizor → Light Metal, Hitmontop → Intimidate, Breloom → Effect Spore,
Roserade → Poison Point, Ambipom → Skill Link, Maushold → Friend Guard, Fezandipiti → Toxic Chain; and the
three species whose every real ability is now innate take a fork-owned chosen-ability override
(`species_ability_overrides.c`): Marshadow (sole Technician) → Illusion, Kricketune (Swarm + Technician) →
Sheer Force, Grapploct (Limber + Technician) → Water Absorb.

### ABILITY_IRON_FIST / ABILITY_RECKLESS / ABILITY_STRONG_JAW / ABILITY_TOUGH_CLAWS / ABILITY_SHARPNESS / ABILITY_MEGA_LAUNCHER / ABILITY_STEELWORKER / ABILITY_STEELY_SPIRIT / ABILITY_ROCKY_PAYLOAD / ABILITY_SAND_FORCE / ABILITY_ANALYTIC / ABILITY_ADAPTABILITY / ABILITY_PUNK_ROCK / ABILITY_STAKEOUT

The Batch A **offensive move-power boosters**: each gives the holder a conditional damage multiplier on
its own moves (Iron Fist +20% punching, Reckless +20% recoil/crash, Strong Jaw +50% biting, Tough Claws
+30% contact, Sharpness +50% slicing, Mega Launcher +50% pulse, Steelworker / Steely Spirit +50% Steel,
Rocky Payload +50% Rock, Sand Force +30% Ground/Rock/Steel in sandstorm, Analytic +30% when moving last,
Adaptability STAB ×2 instead of ×1.5, Punk Rock +30% sound, Stakeout ×2 vs a just-switched-in target).
All are **clean upsides** (a conditional power boost never hurts the holder), so each innate is a plain
**1:1 copy** — NO pure-boon divergence. Handled beside the matching chosen-ability case at the shared
damage sites in `src/battle_util.c`: most in `CalcMoveBasePowerAfterModifiers` (Iron Fist, Reckless,
Strong Jaw, Tough Claws, Sharpness, Mega Launcher, Steelworker, Steely Spirit, Punk Rock, Sand Force,
Analytic), with Stakeout / Rocky Payload in `CalcAttackStat` and Adaptability in
`GetSameTypeAttackBonusModifier`. Each innate clause is gated `chosen != ABILITY_X && IsInnateActive(...)`
so a holder running the real ability never double-applies. **Steely Spirit also boosts an ALLY's** Steel
moves: the partner-abilities block in `CalcMoveBasePowerAfterModifiers` gets an innate-aware clause too, so
an innate-Steely-Spirit partner powers the attacker's Steel moves like the real ability. **Mega Launcher**
additionally boosts Heal Pulse's healing (`Cmd_healpartystatus`-adjacent Heal Pulse handler in
`src/battle_script_commands.c`, switched to `BattlerHasAbility`). **Adaptability** is also credited at the
Terastal STAB calc (`GetTeraMultiplier`, `src/battle_terastal.c`). Suppression parity holds via
`IsInnateActive()`: none of the fourteen is breakable, so Mold Breaker never touches them (same as the real
ability) — Gastro Acid / Neutralizing Gas / not-on-field are the relevant suppressors.

**Sand Force is the one with a side benefit beyond raw power**: like Sand Rush / Sand Veil it also makes the
holder **immune to sandstorm chip damage**, mirrored at the end-turn damage site (`src/battle_end_turn.c`)
and the AI's sandstorm-damage predictors (`DoesBattlerTakeSandstormDamage` in `src/battle_ai_util.c`,
`GetSwitchinWeatherImpact` in `src/battle_ai_switch.c`); the AI's weather-setting heuristic
(`DoesInnateBenefitFromWeather` in `src/battle_ai_field_statuses.c`) also credits it so the AI sets sandstorm
to enable the boost. (In practice every Sand Force user is Rock/Ground/Steel-typed and thus already
type-immune to sandstorm, so the immunity is belt-and-suspenders, but it is wired for parity.)

**AI is correct for FREE** for the damage itself: every boost lives in the shared damage calc the AI runs
keyed off the real battler (like Technician / Filter, unlike Sturdy's dedicated helpers). The few dedicated
`== ABILITY_X` AI *effect* reads are made innate-aware: the Stakeout / Analytic "prefer a damaging move over
a status one" nudge and the Adaptability "Conversion is more valuable" nudge (both `src/battle_ai_main.c`),
plus the Sand Force sandstorm sites above.

**Canon-only (no flavor picks)** — a flat conditional power boost is potent, so like the pinch / weather
abilities the set stays to species whose ability data carries the booster in any slot (every form is listed
where its own data carries it, and a Mega whose base creature has the innate mirrors it as a pure boon —
e.g. Gallade-Mega keeps Sharpness, Starmie-Mega keeps Analytic, Excadrill-Mega keeps Sand Force, even where
the Mega's own ability differs). Many users already carry other innates, so they take a combined
`INNATES(...)` list with the booster added. Frontier roster sets that hardcoded a Batch A ability are freed
(Step 3.5) to a complementary REAL slot where one exists (e.g. Gigalith's Sand Force → chosen Sand Stream,
Kleavor's Sharpness → chosen Sheer Force, Dracovish's Strong Jaw → chosen Water Absorb, Hitmonchan's Iron
Fist → chosen Inner Focus); sets whose only complementary slots are themselves already innate are left as-is
(still correct — the chosen Batch A ability provides the boost). Three sole-real-ability species take a
fork-owned chosen-ability override (`species_ability_overrides.c`): Clawitzer (sole Mega Launcher) → Water
Absorb, Melmetal (sole Iron Fist) → Filter, Lycanroc-Dusk (sole Tough Claws) → Sand Rush.

### ABILITY_SERENE_GRACE

Doubles the chance of the additional effects of the holder's moves (flinch, status,
stat changes, …), handled at the single shared effect site `CalcSecondaryEffectChance` (`src/battle_util.c`):
the cached `hasSereneGrace` test gains an `|| IsInnateActive(battler, ABILITY_SERENE_GRACE)` clause, so an
innate holder's secondary-effect chances double exactly like the real ability. The King's Rock / Razor Fang
flinch boost (`src/battle_hold_effects.c`, gated `B_SERENE_GRACE_BOOST >= GEN_5`) is the second effect site —
its `GetBattlerAbility() == ABILITY_SERENE_GRACE` read becomes `BattlerHasAbility(battlerAtk, ABILITY_SERENE_GRACE)`.
NO pure-boon divergence: Serene Grace is a clean upside (a real Serene Grace likewise doubles every additional
effect, self-targeting ones included), so the innate is a plain 1:1 copy — no script / pop-up / driver. Suppression
parity holds via `IsInnateActive()` (feature flag + Gastro Acid / Neutralizing Gas / not-on-field); Serene Grace is
not breakable, so Mold Breaker never touches it, same as the real ability. AI is correct for FREE where the prediction
runs through the shared chance calc: the AI's reliability check (`MoveEffectIsGuaranteed` / `CalcSecondaryEffectChance`
in `AI_IsAdditionalEffectReliable`) passes the real on-field `battlerAtk`, so an innate Serene Grace is credited
automatically. The two DEDICATED AI heuristics that read the chosen ability directly — the "flinching is worthwhile"
nudge (`ShouldTryToFlinch`) and the confusion-move synergy score (`IncreaseConfusionScore`, both `src/battle_ai_util.c`) —
are made innate-aware with an `IsInnateActive(battlerAtk, ABILITY_SERENE_GRACE)` clause beside the chosen-ability read.
This populates the canon Serene Grace users so they keep the signature effect-doubling no matter which slot a build
picks (Togepi/Togetic/Togekiss, the Chansey/Happiny/Blissey line, Dunsparce/Dudunsparce incl. the three-segment form,
Jirachi, Shaymin-Sky, the seasonal Deerling/Sawsbuck, Meloetta incl. the Pirouette form), plus a tight graceful/elegant
flavor set lacking the real ability: the Gardevoir line (Ralts/Kirlia/Gardevoir/Gardevoir-Mega, the elegant "Embrace
Pokémon"), the serene beauty Milotic, and the lunar-blessing Cresselia. Several flavor/canon picks already carry other
innates, so they take a combined `INNATES(...)` list. Frontier roster sets that hardcoded Serene Grace are freed
(Step 3.5) to a complementary REAL slot where one exists (Chansey/Blissey → Healer, Togekiss → Super Luck, Dudunsparce →
Rattled); the three sole-real-ability species take a fork-owned chosen-ability override (`species_ability_overrides.c`):
Jirachi → Victory Star, Shaymin-Sky → Effect Spore, Meloetta → Punk Rock.
