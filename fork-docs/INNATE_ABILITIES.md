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
**`BATTLE_ARMOR`**/**`SHELL_ARMOR`**, **`SPEED_BOOST`** (a +1 Speed boost at
the end of every turn — the first *active, scripted* end-turn innate, fired
through an end-turn driver; see the active-ability recipe below), the on-hit
contact/faint reactions, and **`INTIMIDATE`** (a −1 Attack drop on every
opposing battler at switch-in — the first *active, scripted* switch-in innate,
fired through a switch-in driver; see the active-ability recipe below). The full
current set is enumerated in `src/fork/innate_abilities.c` and the SCOPE list in
`include/fork/innate_abilities.h`.

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

## Why some abilities are never wired (the `:x:` set)

`INNATE_ABILITIES_PROGRESS.md` marks abilities we will **not** wire as innates with
`:x:`, but that tracker is a scratch record that gets deleted once the feature is
complete — so the *reasons* live here, permanently. An ability is `:x:` when it fails
a gate above: it's not a pure boon, or it can't be identity-neutral, AI-tractable, or
determinism-safe. The buckets (each rejected ability sits in exactly one):

- **Identity / form / type-transform** — breaks the identity-neutral invariant
  (innates are never copied/swapped/displayed as identity; `-ate` abilities retype
  moves). Trace, Color Change, Forecast, Imposter, Illusion, Zen Mode, Stance
  Change, Schooling, Disguise, Power Construct, Multitype, RKS System, Protean,
  Libero, Mimicry, Battle Bond, Ice Face, Hunger Switch, Zero to Hero, Commander, As
  One (×2), Tera Shift/Shell/Teraform Zero, Embody Aspect (×4), Gulp Missile, Shields
  Down, Refrigerate, Pixilate, Aerilate, Galvanize, Liquid Voice, Normalize,
  Dragonize.
- **Ability copy / swap / nullify** — same invariant, from the ability side. Mummy,
  Wandering Spirit, Receiver, Power of Alchemy, Neutralizing Gas, Lingering Aroma,
  Synchronize.
- **Global-field / weather / terrain / auras** — change the field for *everyone*, so
  not a personal boon (big AI + power swing). Drizzle, Sand Stream, Drought, Snow
  Warning, Electric/Psychic/Misty/Grassy Surge, Primordial Sea, Desolate Land, Delta
  Stream, Orichalcum Pulse, Hadron Engine, Cloud Nine, Air Lock, Dark Aura, Fairy
  Aura, Vessel/Sword/Tablets/Beads of Ruin, Victory Star.
- **Pure drawback (no boon to extract)** — strip the cost and nothing remains.
  Truant, Slow Start, Defeatist, Stall, Klutz, Gorilla Tactics, Mycelium Might.
- **Double-edged (upside welded to downside)** — Simple, Contrary, Moody, No Guard,
  Fluffy, Anger Shell, Dry Skin, Rivalry.
- **Non-volatile status-on-contact/hit** — auto-applies a *major* status, which is
  mutually exclusive, so it can *block the holder's own* status move (an innate
  Static paralyses a target you wanted to Toxic). Not a pure boon — unlike the
  *volatile* Cute Charm (infatuation), which **is** wired. Static, Flame Body, Poison
  Point, Effect Spore, Poison Touch, Toxic Chain, Poison Puppeteer, Spicy Spray.
- **On-hit field / hazard setters** — reactive global-field effects. Sand Spit, Seed
  Sower, Toxic Debris, Cotton Down, Screen Cleaner, Curious Medicine.
- **Hidden type / move / immunity stacking** — a hidden immunity layered on the
  chosen ability is opaque (a surprise the opponent can't read) and stacks messily;
  several are also stable frontier-override picks. Volt Absorb, Water Absorb, Flash
  Fire, Motor Drive, Lightning Rod, Storm Drain, Sap Sipper, Earth Eater, Well-Baked
  Body, Wind Rider, Bulletproof, Soundproof, Damp, Wonder Guard.
- **Forced self-switch / HP-threshold disruption** — forced, often bad. Wimp Out,
  Emergency Exit.
- **Overworld / economy** — innates deliberately never touch the overworld. Run
  Away, Honey Gather, Ball Fetch.
- **Doubles / ally-only niche** — narrow, partner-keyed. Plus, Minus, Symbiosis,
  Costar.
- **Bespoke / signature-complex** — no clean clause; own design pass, low ROI.
  Parental Bond, Sheer Force, Solar Power, Protosynthesis, Quark Drive.

**`:x:` is not always permanent.** An ability rejected only because its driver or its
implemented *clone* didn't exist yet can be promoted to `:white_large_square:` once
it does — that's how the clone-of-implemented set (Chilling Neigh = Moxie, Full Metal
Body = Clear Body, Transistor = Steelworker, …) became **Batch Y** in
`INNATE_ABILITIES_BATCHES.md`. Before promoting an `:x:`, confirm it isn't
load-bearing as a stable frontier-override pick (`src/fork/frontier_extended_mons.c`);
un-rejecting one forces the Step 3.5 sweep to re-point every override that hands it
out.

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

**Sweep the `DETERMINISTIC_*` config surfaces too** (`include/config/deterministic.h` — this fork's
flagship battle changes). The deterministic configs REROUTE mechanics through fork-owned sites that a
vanilla-only `grep ABILITY_X` misses: accuracy/evasion becomes a PP economy
(`GetDeterministicMoveTargetPPTax` / `GetAccEvasionStageDelta`, plus `CalculatePPWithBonus`'s
accuracy-scaled max PP — which also skews naive PP expectations in tests, see the Wonder Skin
Confuse-Ray-not-Toxic note), additional effects gate on SE/STAB instead of rolling
(`DETERMINISTIC_ADDITIONAL_EFFECTS`), held-item procs fire once and get consumed only when their
effect would actually land (`DETERMINISTIC_HOLD_EFFECTS`'s would-it-land mirrors in
`battle_hold_effects.c`), and ability/status chances fire deterministically. If the ability touches
accuracy, evasion, secondary effects, flinches, crits, status chances or held-item procs, grep
`DETERMINISTIC` around each effect site and wire + test the innate under the relevant config. Worked
examples: the Sand Veil / Snow Cloak / Wonder Skin / Tangled Feet PP taxes; Quick Feet's paralysis-tax
exemption; Serene Grace under `DETERMINISTIC_ADDITIONAL_EFFECTS`; Shield Dust's King's-Rock consume
mirror under `DETERMINISTIC_HOLD_EFFECTS` and its gated-in-effect block under
`DETERMINISTIC_ADDITIONAL_EFFECTS`.

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
  - **on-contact / on-hit actives** (Rough Skin, Iron Barbs, Gooey, Tangling Hair, …)
    now have their driver: **`TryActivateInnateOnHitEffects`** (`src/fork/innate_abilities.c`),
    the on-hit analogue of the end-turn driver — re-entrant, hooked from the new
    `MOVEEND_ABILITIES_INNATE` step (`src/battle_move_resolution.c`), delegating to the
    upstream `ABILITYEFFECT_MOVE_END` case. Adding a further on-hit active (Aftermath,
    Cursed Body, Steam Engine, …) is a one-line addition to `IsActiveOnHitInnate`. See the
    `### ABILITY_ROUGH_SKIN / …` wiring block below. (Cute Charm / Stench predate the driver
    and keep their own inline prechecks, so they are not in `IsActiveOnHitInnate`.)
  - **switch-in actives** (Intimidate, …) now have their driver: **`TryActivateInnateSwitchInEffects`**
    (`src/fork/innate_abilities.c`), the switch-in analogue of the end-turn / on-hit drivers —
    re-entrant, hooked from the new `FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE` step
    (`include/constants/battle_switch_in.h`, dispatched in `FirstEventBlockEvents`,
    `src/battle_switch_in.c`) right after the chosen-ability switch-in block, delegating to the
    upstream `ABILITYEFFECT_ON_SWITCHIN` case. The hook lives inside the `switchinevents` state
    machine that drives *every normal switch-in* (battle intro, pivot moves, post-faint replacement,
    forced switch); the `switchinabilities` sites (ability-swap / Tera / form change) are deliberately
    NOT hooked, so a species-bound innate never re-fires when a foe Skill-Swaps or the holder
    Mega-evolves. Adding a further switch-in active that runs through `ABILITYEFFECT_ON_SWITCHIN`
    (Download, …) is a one-line addition to `SwitchInInnateAbilityEffect` (the ability→effect map). An
    active that runs through a *different* switch-in case at a *different* phase (Unnerve →
    `ABILITYEFFECT_UNNERVE`, Hospitality → `ABILITYEFFECT_DEPENDS_ON_ALLY`) also adds a new hook at that
    phase passing its `abilityEffect` to the driver. See the `### ABILITY_INTIMIDATE` and
    `### ABILITY_UNNERVE / ABILITY_HOSPITALITY` wiring blocks below.

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
   > (Overgrow) is implemented and the new chosen ability is itself stable (`:x:`). **But audit a
   > real-slot repurpose first**: the override table is consulted unconditionally by `GetSpeciesAbility`
   > (not feature-gated), so the row deletes that real ability from the species game-wide — grep
   > `test/battle/` for `Ability(ABILITY_X)` on that species before repurposing (empty-`ABILITY_NONE`
   > slots need no audit; nothing observes them). See the Batch P block's refined rule.
6. Update the roster header's INNATE ABILITIES note to mention the new ability.

### Step 4 — test it

Add a case to `test/fork/innate_abilities.c`. Opt into the feature with
`WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE)` (the test baseline forces all
`FEATURE_*` flags off, so the inherited suite keeps exercising stock behavior).
Cover: the innate's effect fires; it does **not** fire with the feature off; for
trait/immunity abilities, that suppression (Gastro Acid / Mold Breaker) and
Trace/identity still behave like the real ability; and **one test per
`DETERMINISTIC_*` surface the ability touches** (Step 3's deterministic sweep) —
the shipping default runs with those configs ON, so an innate that only works
under stock RNG is broken in the real game. Run:

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
- [ ] **Step 3** — effect wired at *every* site (`grep -n ABILITY_X src/`), including the AI's *effect* reads (`grep src/battle_ai_*.c`) **and the `DETERMINISTIC_*` reroutes** (PP-economy taxes, would-it-land consume mirrors, gated additional effects — grep `DETERMINISTIC` around each effect site); new battle-state fields zero-init with `gBattleStruct` and reset per battle.
- [ ] **Step 3.5 — ran `grep -n ABILITY_X src/fork/frontier_extended_mons.c`** and freed every hardcoded set (override-table rows for ability-locked / all-abilities-innate species). *This is the step that gets forgotten.*
- [ ] **Step 4** — tests added, **including the `DETERMINISTIC_*` interactions the ability touches** (the shipping default); `make check TESTS="FEATURE_INNATE_ABILITIES"` green; **full `make check` green** if a shared battle file was touched; ROM builds under `UNUSED_ERROR=1 DEPRECATED_ERROR=1`.
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
pierces an innate Immunity/Pastel Veil exactly as it would the real ability. **Switch-in ally-cure
(Batch V, now wired):** the real Pastel Veil's switch-in ALLY-cure
(`BattleScript_PastelVeilActivates`, looping self+partner to cure pre-existing poison on the holder's
switch-in) — previously left unwired for want of a switch-in-with-script driver — now rides the switch-in
driver Batch L built (`TryActivateInnateSwitchInEffects`): `SwitchInInnateAbilityEffect` maps
`ABILITY_PASTEL_VEIL` to `ABILITYEFFECT_ON_SWITCHIN`, so the `FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE`
hook runs the full script (holder + ally) with the pop-up overwritten to Pastel Veil when the chosen
ability differs. That block runs BEFORE the `ABILITYEFFECT_IMMUNITY` self-cure chokepoint, so at a
genuine switch-in the script clears the holder's own poison too and the self-cure above (still wired for
Immunity and non-switch-in re-checks) no-ops. NOTE: an innate Pastel Veil matches the chosen ability 1:1,
including the upstream quirk that a holder poisoned AT THE SAME TIME as its ally clears only its own
poison (the pop-up clobbers the cure script's loop counter — reproducible with the chosen ability, so not
a fork divergence). CANON-ONLY (no flavor picks): Immunity goes to
Gligar (combined with its innate Sand Veil) and Snorlax/Snorlax-Gmax (combined with innate Unaware),
each whose real ability data carries Immunity in some slot. (Zangoose also has slot-0 Immunity in its
data, but is given innate Toxic Boost instead — see the Batch N reference: the two are contradictory and
Toxic Boost is Zangoose's actual frontier identity.) Pastel Veil goes to
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
**Composes with `DETERMINISTIC_ADDITIONAL_EFFECTS` for free:** under that flag a chance-based effect is gated on a
super-effective / STAB hit, but the resolver (`TryTriggerAdditionalEffect`, `src/fork/deterministic_moves.c`) makes a
Serene Grace / Rainbow boost *certain* by detecting a **computed chance above the move's base**
(`percentChance > additionalEffect->chance`) — NOT a direct `== ABILITY_SERENE_GRACE` check. Because the innate feeds
its boost through `CalcSecondaryEffectChance`, an innate holder trips that gate exactly like the real ability, so no
extra deterministic wiring is needed.
This populates the canon Serene Grace users so they keep the signature effect-doubling no matter which slot a build
picks (Togepi/Togetic/Togekiss, the Chansey/Happiny/Blissey line, Dunsparce/Dudunsparce incl. the three-segment form,
Jirachi, Shaymin-Sky, the seasonal Deerling/Sawsbuck, Meloetta incl. the Pirouette form), plus a tight graceful/elegant
flavor set lacking the real ability: the Gardevoir line (Ralts/Kirlia/Gardevoir/Gardevoir-Mega, the elegant "Embrace
Pokémon"), the serene beauty Milotic, and the lunar-blessing Cresselia. Several flavor/canon picks already carry other
innates, so they take a combined `INNATES(...)` list. Frontier roster sets that hardcoded Serene Grace are freed
(Step 3.5) to a complementary REAL slot where one exists (Chansey/Blissey → Healer, Togekiss → Super Luck, Dudunsparce →
Rattled); the three sole-real-ability species take a fork-owned chosen-ability override (`species_ability_overrides.c`):
Jirachi → Victory Star, Shaymin-Sky → Effect Spore, Meloetta → Punk Rock.

### ABILITY_MULTISCALE / ABILITY_SOLID_ROCK / ABILITY_FUR_COAT / ABILITY_ICE_SCALES / ABILITY_HEATPROOF / ABILITY_FRIEND_GUARD / ABILITY_WATER_BUBBLE

The "defensive damage reducers" (Batch B): all seven cut the damage the holder (or its ally) takes,
handled by additive `IsInnateActive()` clauses beside the existing chosen-ability reads in the damage
calc (`src/battle_util.c`), with a `chosen != ABILITY_X` guard so a mon running the real ability never
double-applies. NO pure-boon divergence — each real ability is a clean upside that never hurts its holder,
so every innate is a plain 1:1 copy. All are breakable, so suppression parity (feature flag + Gastro Acid /
Neutralizing Gas / not-on-field + an attacker's Mold Breaker) matches the real ability via `IsInnateActive()`.
AI is correct for FREE for the damage-calc halves: the reductions live in the shared damage calc the AI runs
keyed off the real battler, so it both threatens and respects them. Per-ability sites:
- **Multiscale** (halve damage at full HP): the `GetDefenderAbilitiesModifier` clause (beside the chosen
  Multiscale / Shadow Shield case). The one DEDICATED AI read — `BattlerHasMaxHPProtection` (`src/battle_ai_util.c`,
  which tells the AI a battler survives at full HP, the same helper Sturdy was wired into) — is made innate-aware
  with `BattlerHasAbility(battler, ABILITY_MULTISCALE)`.
- **Solid Rock** (−25% from supereffective hits): shares Filter's effect 1:1, so it just rides the existing
  innate-Filter clause in `GetDefenderAbilitiesModifier` (now `IsInnateActive(FILTER) || IsInnateActive(SOLID_ROCK)`).
- **Fur Coat** (double Defense vs physical): the defense-stat modifier clause (beside the chosen Fur Coat case),
  gated on `usesDefStat`.
- **Ice Scales** (halve special damage): the `GetDefenderAbilitiesModifier` clause, gated on a special move.
- **Heatproof** (halve Fire damage + halve burn damage): the Fire-damage clause in the CalcAttackStat defender
  switch (shared with Water Bubble below), plus the burn-damage halving at the end-turn site
  (`HandleEndTurnBurn`, `src/battle_end_turn.c`) — `ability == ABILITY_HEATPROOF || BattlerHasAbility(...)`, with
  only the real ability recorded (the innate stays silent). Two DEDICATED AI burn reads are made innate-aware:
  `ShouldBurn` (`src/battle_ai_util.c`) and the switch-in status-damage predictor `GetSwitchinStatusDamage`
  (`src/battle_ai_switch.c`), each via `BattlerHasAbility(battler, ABILITY_HEATPROOF)`.
- **Friend Guard** (ally-side −25%): the `GetDefenderPartnerAbilitiesModifier` clause (beside the chosen Friend
  Guard case), keyed off the partner; like the real ability it does not reduce confusion self-hits.
- **Water Bubble** (halve Fire damage + double the holder's Water moves + burn immunity): wired in FULL as a
  pure-boon innate, not just the Batch B "fire-half". (1) Fire-damage halving shares Heatproof's clause in the
  CalcAttackStat defender switch. (2) Water-move doubling is an `IsInnateActive(battlerAtk, ABILITY_WATER_BUBBLE)`
  clause in the offensive-booster innate block (`CalcAttackStat`), beside the chosen Water Bubble case. (3) Burn
  immunity is wired at the burn status-set site in `CanSetNonVolatileStatus` (`src/battle_util.c`), mirroring the
  Limber / Pastel Veil precedents: the innate blocks the burn and overwrites the ability pop-up to Water Bubble
  (since `CreateAbilityPopUp` reads the primary slot) only when the chosen ability differs, leaving the real Water
  Veil / Water Bubble path byte-for-byte. Routing through `CanSetNonVolatileStatus` makes `CanBeBurned` and its AI
  callers innate-aware for free. (The burn-*cure*-on-ability-gain site is left unwired: an innate is permanent, so
  a Water Bubble holder can never be burned in the first place, leaving nothing to cure.)

Species selection is canon users (every species whose ability data carries the ability in any slot, so the
signature survives whichever slot a build picks), plus a tight flavor extension where it completes an evolution
line or theme: Heatproof adds the rest of Rolycoly's coal/lava line (Carkol, Coalossal, Coalossal-Gmax), and
Friend Guard adds the support fairies that complete the Clefairy / Jigglypuff / Chansey lines (Clefable,
Wigglytuff, Chansey, Blissey) alongside the canon pre-evos. The potent reducers (Multiscale, Solid Rock, Fur
Coat, Ice Scales, Water Bubble) stay canon-only. Several species already carry other innates, so they take a
combined `INNATES(...)` list. Frontier roster sets that hardcoded these are freed (Step 3.5) to a complementary
REAL slot where one exists (Dragonite's Multiscale → chosen Inner Focus; Camerupt's Solid Rock → Magma Armor;
Frosmoth's Ice Scales → Shield Dust; Persian-Alola's Fur Coat → Rattled; Rhyperior's Solid Rock → Lightning Rod;
Araquanid's Water Bubble → Water Absorb); the all-real-abilities-innate / dead-weight-slot cases take a fork-owned
chosen-ability override (`species_ability_overrides.c`): Lugia → Storm Drain, Carracosta → Water Absorb, Maushold →
No Guard, Bronzong → Soundproof, Sinistcha → Flash Fire — each a stable `:x:` pick.

### ABILITY_GUTS / ABILITY_MARVEL_SCALE / ABILITY_QUICK_FEET / ABILITY_TOXIC_BOOST / ABILITY_FLARE_BOOST

The "status-conditional stat boosts" (Batch N): each grants the holder a stat/damage boost while it carries a
status condition (Guts +50% physical Attack while statused, Marvel Scale +50% Defense while statused, Quick Feet
+50% Speed while statused, Toxic Boost +50% physical power while poisoned, Flare Boost +50% special power while
burned). All five are **clean upsides** — the boost only ever helps the holder, and even the burn/paralysis
*penalty* clauses (Guts negates burn's physical-damage cut, Quick Feet ignores the paralysis Speed/PP/priority
penalty) are part of the real ability's upside — so each innate is a plain **1:1 copy**, no pure-boon divergence.
Suppression parity holds via `IsInnateActive()` / `BattlerHasAbility()`: none of the five is breakable, so Mold
Breaker never touches them (Gastro Acid / Neutralizing Gas / not-on-field are the relevant suppressors). The
innate is NOT recorded as identity (no pop-up — these have none even as real abilities). Per-ability sites:
- **Guts** — the attack-stat boost rides the `ctx->innatesEnabled` block of `CalcAttackStat` (`src/battle_util.c`),
  beside the chosen-ability Guts case (`gBattleMons[atk].status1 & STATUS1_ANY && physical move`, guarded
  `!= ABILITY_GUTS`). The burn-physical-cut negation is a second clause in `GetBurnOrFrostBiteModifier`
  (same file): the `0.5x` burn cut is skipped for an innate Guts holder too.
- **Marvel Scale** — a clause beside the innate Fur Coat one in the defense-stat modifier path
  (`CalcAttackStat`, `src/battle_util.c`): `+50% Defense` while statused, gated on `usesDefStat` (a physical hit),
  guarded `!= ABILITY_MARVEL_SCALE`.
- **Quick Feet** — the `+50%` Speed read in `GetBattlerTotalSpeedStat` (`src/battle_main.c`) gains an
  `|| IsInnateActive(battler, ABILITY_QUICK_FEET)` clause (statused holder). Its paralysis-penalty exemptions are
  mirrored at all three fork paralysis sites: the (non-deterministic) Speed-drop in `GetBattlerTotalSpeedStat`,
  the `DETERMINISTIC_PARALYSIS` priority tax in `GetBattleMovePriority` (`src/battle_main.c`), and the
  `DETERMINISTIC_PARALYSIS` PP tax in both `CancelerPPDeduction` (`src/battle_move_resolution.c`) and its AI mirror
  (`src/battle_util.c`) — each now treats an innate Quick Feet holder as exempt, like the real ability.
- **Toxic Boost / Flare Boost** — clauses in the `ctx->innatesEnabled` offensive block of
  `CalcMoveBasePowerAfterModifiers` (`src/battle_util.c`), beside the chosen-ability cases (Toxic Boost +50% to a
  physical move while poisoned, Flare Boost +50% to a special move while burned), each guarded `!= ABILITY_X`.

**AI** is correct for FREE for everything in the shared damage/turn-order calcs (the attack/defense/speed/power
reads run keyed off the real battler, so the AI both threatens and respects an innate holder). The dedicated AI
*effect* reads — all about whether the AI should inflict a status on a battler that would *benefit* from it — are
made innate-aware in `src/battle_ai_util.c`: `DoesBattlerBenefitFromAllVolatileStatus` (Marvel Scale / Quick Feet /
Guts), `ShouldPoison` and the poison-harmlessness check (Toxic Boost), and `ShouldBurn` and the burn-harmlessness
check (Flare Boost) each credit an innate holder via `BattlerHasAbility()` so the AI won't, e.g., poison an innate
Toxic Boost foe. The ability-transfer scoring (`BattlerBenefitsFromAbilityScore`, the Guts case) is left alone
since innates are never transferable.

**Species selection is canon-only (no flavor picks)** — a status-fueled stat boost is potent, so like the pinch /
weather abilities the set stays to species whose ability data carries the booster in any slot, so the signature
survives whichever slot a build picks. Guts: the Rattata, Machop, Taillow, Shinx, Timburr lines, Flareon,
Heracross (+ Mega, mirrored as a pure boon), the Larvitar/Makuhita/Tyrogue/Ursaring/Ursaluna lines, Throh,
Squawkabilly (Green/Blue), Conkeldurr, Obstagoon, … . Quick Feet: Jolteon, Granbull, the Teddiursa/Ursaring,
Poochyena, Zigzagoon (Hoenn + Galar), Shroomish lines. Marvel Scale: the Dratini line and Milotic. Flare Boost:
the Drifloon line. Many already carry other innates, so they take a combined `INNATES(...)` list with the booster
added (Heracross + Swarm, the Makuhita/Hariyama + Thick Fat, Larvitar + Sand Veil, Ursaring + Quick Feet, the
Timburr line + Iron Fist, Obstagoon + Reckless, Milotic + Cute Charm + Serene Grace).

**TOXIC_BOOST's user is Zangoose, which carries innate Toxic Boost INSTEAD of innate Immunity.** Zangoose's real
ability data is Immunity (slot 0) / Toxic Boost (HA), and a prior batch gave it innate Immunity — but the two are
**contradictory** (Immunity blocks the poison Toxic Boost needs), so a Zangoose can't usefully carry both as
always-on innates. Toxic Boost is Zangoose's actual competitive/frontier identity (its sets run Toxic Orb + Facade),
which innate Immunity silently neuters, so this batch **reassigns** Zangoose's innate from Immunity → Toxic Boost.
Innate Immunity still lives on its other canon users, Gligar and Snorlax (the Immunity tests were repointed to
Snorlax). This is the one place where a species' canon slot-0 ability is *not* its innate — the general rule when a
species has two contradictory candidate innates (a status-immunity vs a same-status-requiring boost) is to keep the
one that matches how the species is actually used; Zangoose is currently the only such case (every other
toxic-themed species is a Poison-type, type-immune to poison, so none competes for Toxic Boost).

**Step 3.5 (frontier roster):** sets that hardcoded a Batch N ability are freed to a complementary REAL slot
(Hariyama/Conkeldurr → Sheer Force; Flareon → Flash Fire; Machamp → No Guard; Heracross/Mightyena → Moxie;
Ursaring → Unnerve; Milotic → Competitive; Obstagoon → Defiant; Luxray/Squawkabilly → Intimidate; Throh →
Inner Focus; Ursaluna → Bulletproof; Swellow → Scrappy; Linoone → Gluttony; Drifblim → Unburden; Raticate →
Run Away). Only **Zangoose** needed a `species_ability_overrides.c` row: its two Toxic Boost (Toxic Orb) sets are
freed to a chosen **Sheer Force** in its empty slot 1 — not Immunity, which would block the poison its innate Toxic
Boost needs (a stable `:x:` pick that also skips Life Orb recoil on the SD set).

### ABILITY_SUPER_LUCK / ABILITY_SNIPER / ABILITY_MERCILESS

The "crit-rate / crit-damage modifiers" (Batch O): all three live in the critical-hit calc in
`src/battle_util.c`. **Super Luck** adds +1 to the holder's crit stage; **Merciless** auto-crits a target
that is poisoned or badly poisoned; **Sniper** boosts critical-hit *damage* (the crit multiplier becomes
×2.25 instead of ×1.5). All three are **clean upsides** that never hurt the holder, so each innate is a plain
**1:1 copy** — no pure-boon divergence. Wiring:

- **Super Luck** — the `+1` crit-stage read in both `CalcCritChanceStage` and the Gen-1 `CalcCritChanceStageGen1`
  gains an `|| (ctx->innatesEnabled && IsInnateActive(ctx->battlerAtk, ABILITY_SUPER_LUCK))` clause beside the
  cached chosen-ability test.
- **Merciless** — the auto-crit-vs-poisoned read in the same two functions gains the same innate clause
  (`ctx->innatesEnabled && IsInnateActive(...)`) beside the cached chosen-ability test.
- **Sniper** — `GetAttackerAbilitiesModifier` returns the crit-damage ×1.5 from a `switch (abilityAtk)`; an
  innate-Sniper clause is added *after* the switch, gated `isCrit && abilityAtk != ABILITY_SNIPER &&
  GetConfig(FEATURE_INNATE_ABILITIES) && IsInnateActive(...)`, so it only runs on a crit (off the non-crit
  hot path) and never double-applies with the chosen-ability path. (This function takes no `DamageContext`,
  so it reads `GetConfig()` directly rather than the cached `ctx->innatesEnabled`.)

Suppression parity holds via `IsInnateActive()`: none of the three is breakable, so Mold Breaker never touches
them (Gastro Acid / Neutralizing Gas / not-on-field are the relevant suppressors), exactly like the real
abilities.

**AI.** The crit calc (`CalcCritChanceStage`) runs in the AI's *per-move × target* damage prediction every turn,
so it is a genuine hot path — in doubles it is the exact code `ai_thinking_time.c` caps with a tight frame
ceiling. Gating the innate crit reads on the cached `ctx->innatesEnabled` field is cheap, **but only if the flag
is populated**, and the AI's crit-chance predictor (`ShouldCalcCritDamage`) deliberately leaves it `0` (from the
`{0}` init). So the AI's per-eval crit-chance prediction *does not* credit an innate Super Luck / Merciless /
Battle Armor — a deliberate approximation: crediting them cost enough per eval to blow the ceiling for a
negligible accuracy gain. This only affects the AI's fine-grained damage prediction; it is not a correctness
issue. What the AI *does* keep:
- **Real battle is unaffected** — the actual crit roll (`DoMoveDamageCalc` / `DoFutureSightAttackDamageCalc`)
  sets `ctx->innatesEnabled` before `IsCriticalHit`, so innate crits fire exactly as designed.
- **Crit *damage* modifiers still see innates** — the AI's `CalculateMoveDamageVars` → `DoMoveDamageCalcVars`
  sets the flag before the modifier passes, so an innate Sniper's crit-damage boost is predicted (when a crit is
  predicted by non-innate means).
- **Strategic crit heuristics are innate-aware** via `BattlerHasAbility()` (these are *not* in the crit hot
  loop): the Focus Energy / Laser Focus setup score and the Dire-Hit-item heuristic (Super Luck / Sniper,
  `src/battle_ai_main.c` / `src/battle_ai_items.c`) and the "poison the target" score (Merciless,
  `src/battle_ai_util.c`).

(Lesson for the next crit-touching batch: the crit-chance calc is AI-hot and budget-bound — keep it
`ctx->innatesEnabled`-gated and do **not** set that flag on the AI's `ShouldCalcCritDamage` context.)

**Species (canon-only, no flavor picks** — crit boosts are potent and hard to justify thematically): every
species whose ability data carries the ability in any slot, in dex order, so the signature survives whichever
slot a build picks. Sniper: the Beedrill, Spearow/Fearow, Horsea/Seadra/Kingdra, Spinarak/Ariados,
Remoraid/Octillery, Skorupi/Drapion, Binacle/Barbaracle and Sobble/Drizzile/Inteleon lines (Mega Beedrill /
Mega Barbaracle / Inteleon-Gmax mirror the base per the Mega convention). Super Luck: the Togepi line, the
Murkrow/Honchkrow line, Absol (+ its Megas), and the Pidove line. Merciless: the Mareanie/Toxapex line. Each
new ability is merged into the existing innate row where a species already carries one (e.g. Ariados keeps
Insomnia/Swarm, Kingdra keeps Swift Swim, Toxapex keeps Limber/Regenerator, Togekiss keeps Serene Grace).

**Frontier (Step 3.5):** these abilities were previously *pending* (`:white_large_square:`), so many frontier
sets had used them as a chosen pick beside an already-innate slot. Only the sets whose species has a
complementary **real** `:x:` slot are re-pointed, needing no override — **Octillery → Moody** and
**Kingdra → Damp** (each runs the new ability *plus* its innate). The remaining canon sets are deliberately
**left on their now-innate-redundant real ability** (Sniper / Super Luck / Merciless) rather than freed.
Freeing those would require `species_ability_overrides.c` rows **repurposing real slots** (none of these
species has an empty slot to fill), and that table is consulted unconditionally by `GetSpeciesAbility`
(it is *not* feature-gated) — so the row removes the real ability **globally** even with the feature off,
breaking upstream tests that explicitly select it (e.g. `crit_chance.c` / `deterministic_critical_hits.c`
do `PLAYER(SPECIES_TOGEKISS) { Ability(ABILITY_SUPER_LUCK); }`). A set left on its real
Sniper/Super Luck/Merciless keeps working unchanged — the chosen ability simply equals the innate
(redundant but harmless; the effect sites guard against double-applying), so only the second-ability
*upgrade* is forgone. (An earlier version of this note also blamed the override table's linear scan for
the `ai_thinking_time.c` budget — overstated: in-battle reads use the cached `gBattleMons[].ability`, not
`GetSpeciesAbility`. **Empty-slot** override rows remain fine — see the Batch P refined rule.)

### ABILITY_SHIELD_DUST / ABILITY_TINTED_LENS / ABILITY_SCRAPPY / ABILITY_WONDER_SKIN / ABILITY_TANGLED_FEET

The "accuracy / type-effectiveness / effect-chance modifiers" (Batch P). All five are **clean upsides**
that never hurt the holder, so each innate is a plain **1:1 copy** — no pure-boon divergence. (Tangled
Feet's trigger — being confused — is a bad state, but the *ability's own effect* only ever helps; the
innate doesn't cause the confusion.) Wiring:

- **Shield Dust** (blocks the additional effects of moves used *against* the holder) — one engine
  chokepoint: `IsMoveEffectBlockedByTarget` (`src/battle_util.c`) gains an
  `IsInnateActive(gBattlerTarget, ABILITY_SHIELD_DUST)` else-branch after the chosen-ability /
  Covert Cloak branches (which stay byte-for-byte untouched); identity bookkeeping
  (`RecordAbilityBattle`) is skipped for the innate. Every blocked source funnels through this
  predicate — move secondaries (`SetMoveEffect`), ability riders (Poison Touch / Toxic Chain), Fling,
  the Sparkling-Aria spread carve-out, and King's Rock-style item flinches — so one clause covers all.
  Two `DETERMINISTIC_*` reroutes needed their own wiring: under `DETERMINISTIC_HOLD_EFFECTS` the
  King's-Rock would-it-land consume mirror (`battle_hold_effects.c`) now treats a Shield Dust target
  (real or innate — and a Covert Cloak holder) as a non-landing flinch, so the rock isn't consumed for
  a flinch the chokepoint then blocks; and under `DETERMINISTIC_ADDITIONAL_EFFECTS` a gated-in
  (SE/STAB-guaranteed) secondary is still blocked — both covered by dedicated tests.
- **Tinted Lens** (2x damage on not-very-effective moves) — `GetAttackerAbilitiesModifier`
  (`src/battle_util.c`), an innate clause after the chosen-ability switch, gated
  `typeEffectivenessModifier <= 0.5 && abilityAtk != ABILITY_TINTED_LENS &&
  GetConfig(FEATURE_INNATE_ABILITIES) && IsInnateActive(...)` — the NVE guard keeps the lookups off
  the neutral/SE hot path, mirroring the innate-Sniper clause above it.
- **Scrappy, Ghost-hit half** (Normal/Fighting hit Ghost-types) — `MulByTypeEffectiveness`
  (`src/battle_util.c`), an else-if beside the chosen Scrappy/Mind's Eye branch that lifts the 0x
  immunity for an innate Scrappy (no `RecordAbilityBattle` — not the displayed ability).
- **Scrappy, Intimidate-immunity half (GEN_8+)** — `IsIntimidateBlocked`
  (`src/battle_stat_change.c`): the existing innate-Oblivious block is generalized to a small
  `innateImmunity` pick (Oblivious first, then Scrappy), mirroring the switch cases and overwriting
  the pop-up/record to the innate (the Levitate/Sturdy `abilityPopupOverwrite` precedent). This half
  did NOT need the Batch L switch-in driver — that driver is only for *casting* Intimidate as an
  innate; *defending* against a real Intimidate is a passive trait at this site.
- **Wonder Skin** (incoming status moves capped at 50% accuracy) — `GetTotalAccuracy`
  (`src/battle_util.c`), an innate clause beside the `defAbility` test (reordered so the
  status-move/`moveAcc > 50` guards run first), plus the `GetDeterministicMoveTargetPPTax` status
  branch (+1 PP on status moves under `DETERMINISTIC_ACCURACY_EVASION`, like the Sand Veil precedent).
- **Tangled Feet** (evasion doubled while confused) — `GetTotalAccuracy`, a guarded block after the
  defender-ability switch (`defAbility != ABILITY_TANGLED_FEET && confused && IsInnateActive(...)`,
  the exact Sand Veil / Snow Cloak pattern), plus the offensive branch of
  `GetDeterministicMoveTargetPPTax` (+1 PP while confused).

Suppression parity holds via `IsInnateActive()`: Shield Dust, Scrappy, Wonder Skin and Tangled Feet are
breakable, so an attacker's Mold Breaker pierces them exactly like the real abilities (Tinted Lens is
attacker-side, so only Gastro Acid / Neutralizing Gas / not-on-field apply).

**AI.** Tinted Lens and the Scrappy Ghost-hit live in the shared damage/type calc, so the AI's damage
and effectiveness prediction credits them for FREE (keyed off the real battler). Wonder Skin / Tangled
Feet accuracy flows through the shared `GetTotalAccuracy` the AI's accuracy table uses — also free.
The dedicated AI *effect* reads were wired by hand (`grep src/battle_ai_*.c`):
- Shield Dust (the AI as attacker discounting its own secondaries): `IsAdditionalEffectBlocked`,
  the flinch-scoring reads in `ShouldTryToFlinch` + the Dynamax flinch checker, the stat-drop
  secondary in `CanLowerStat`, and the damaging-hazard-move gate in `AI_ShouldSetUpHazards`
  (all `src/battle_ai_util.c`), each crediting an innate Shield Dust beside the chosen-ability read
  (under the same explicit Mold-Breaker guard where one exists).
- Scrappy: the Foresight-is-pointless score (`src/battle_ai_main.c`) and the Intimidate-benefit
  switch check (`src/battle_ai_switch.c`, extending the innate-Oblivious clause).

**AI frame budget (bisected before re-baselining):** the doubles/no-flags thinking-time baseline sat
*exactly at* its ceiling (21/21 — zero fractional headroom), and with this batch it measures 22. A full
bisection showed the +1 is NOT algorithmic: reverting *all* of the batch's engine code (battle_util /
battle_stat_change / battle_ai_*) still measures 22; reverting only the species-table rows (engine at
HEAD) still measures 22; only reverting *everything* returns 21. I.e. the tip is frame quantization
plus ROM-layout shift from ~1.2 KB of new species data — every future batch that adds species rows
would hit it regardless of how optimal its code is. `AI_FRAME_CEILING_DOUBLES_NO_FLAGS` was therefore
re-baselined 21 → 22 (the Technician batch's `SINGLES_SMART_TRAINER` 8 → 9 precedent). The hunt still
produced three real improvements, kept: (1) the AI's six off-field `SpeciesHasInnate` reads (Batches
Levitate/Sturdy/Immunity) were **not config-gated**, so they walked the whole innate table even with
the feature off — and, worse, credited innates that don't function (a feature-off misprediction bug);
all six now check `GetConfig(FEATURE_INNATE_ABILITIES)` first. (2) the innate Sniper / Tinted Lens
clauses in `GetAttackerAbilitiesModifier` now take the cached `ctx->innatesEnabled` instead of calling
`GetConfig()` per evaluation (the Batch O caching discipline). (3) `SpeciesHasInnate` is now
**sublinear**: `GetSpeciesInnateList` binary-searches a lazily built species-sorted row index (~1 KB
EWRAM bss) instead of walking the ~500-row table linearly — with the feature ON in shipped play, every
`IsInnateActive` paid that walk on the AI-hot calcs, a cost CI never measures because tests force the
feature off. The source table stays dex-sorted for humans; the "species-keyed lookup matches the raw
table" integrity test guards the index.

**Species (canon-only, no flavor picks):** every species whose ability data carries the ability in any
slot, in dex order, merged into existing rows where the species already carries an innate. Shield Dust:
Caterpie, Weedle, Wurmple + Dustox, Scatterbug + Vivillon, Cutiefly/Ribombee, Snom/Frosmoth and
Venomoth (Spewpa is deliberately excluded — its own data is Shed Skin/Friend Guard, not Shield Dust,
and the accuracy-abilities integrity test pins that exclusion).
Tinted Lens: Butterfree(+Gmax), Venonat/Venomoth, Hoothoot/Noctowl, Illumise, Yanmega, Sigilyph,
Braviary-Hisui, Nymble/Lokix. Scrappy: Kangaskhan(+Mega, mirroring the base), Farfetch'd-Galar/
Sirfetch'd, Miltank, Taillow/Swellow, Loudred/Exploud, Herdier/Stoutland, Pancham/Pangoro,
Decidueye-Hisui, Flamigo. **Mega Lopunny is deliberately omitted**: its only — and therefore always
chosen — ability IS Scrappy, so an innate could never be observed (the sole-ability-redundant rule).
Wonder Skin: Skitty/Delcatty, Venomoth, Sigilyph, Bruxish. Tangled Feet: the Pidgey line (+Mega
Pidgeot, mirroring the base), Doduo/Dodrio, Spinda, Chatot, Mr. Rime, Flamigo.

**Frontier (Step 3.5):** these abilities were pending, so many sets had spent their `.ability` slot on
them. Every set was freed, three ways (this batch also *refined* the Batch O override rule — see below):
- **Complementary real slot** (no override needed): Kangaskhan → Inner Focus, Miltank → Sap Sipper,
  Exploud → Soundproof, Sigilyph → Magic Guard, Braviary-Hisui → Sheer Force, Pangoro → Mold Breaker,
  Flamigo → Costar. (Inner Focus / Magic Guard / Mold Breaker are still pending, so those re-points get
  revisited when their batches land — real-slot picks are allowed to be pending; the alternatives were
  dead slots.)
- **Empty-slot override rows** (`species_ability_overrides.c`, the established Venusaur/Blaziken shape —
  filling an `ABILITY_NONE` slot deletes nothing and no upstream test can select an empty slot):
  Butterfree → Effect Spore, Dustox → Poison Point, Swellow → Quick Feet, Decidueye-Hisui → Sniper,
  Sirfetch'd → Super Luck, Frosmoth → Snow Warning, Lokix → Tough Claws.
- **Left on the now-innate-redundant real ability** (harmless — the effect sites guard against
  double-applying): Venomoth, Dodrio, Noctowl, Illumise, Yanmega, Ribombee — species with **no empty
  slot**, where an override would have to repurpose a *real* slot.

**The refined override rule** (supersedes the blanket "no new rows" reading of the Batch O note): the
override table is consulted **unconditionally** by `GetSpeciesAbility` (`src/pokemon.c`) — it is NOT
gated by `FEATURE_INNATE_ABILITIES` — so a row **replaces that slot game-wide even with innates off**.
Filling an **empty** slot is therefore always safe; repurposing a **real** slot deletes that ability
from the species everywhere and breaks any upstream test that selects it (`Ability(ABILITY_X)` —
e.g. `scrappy.c` pins Kangaskhan's Scrappy, `fling.c`/`stench.c` pin Vivillon's Shield Dust), so it
needs a per-slot audit (the Sceptile/Bronzong-style dead-weight repurposes were each audited). The
Batch O note's other concern — the linear override scan costing AI budget — was overstated: in-battle
ability reads use the cached `gBattleMons[].ability`, not `GetSpeciesAbility` (~37 call sites, mostly
creation/form-change/AI party reads); the real sensitivity was the frame-boundary baseline, addressed
by the ceiling re-baseline above.

### ABILITY_GALE_WINGS / ABILITY_TRIAGE

The "priority granters" (Batch Q): both raise the priority of a class of the holder's moves. Both are
**clean upsides** that never hurt the holder, so each innate is a plain **1:1 copy** — no pure-boon
divergence. Wired at the single effect site `GetBattleMovePriority` (`src/battle_main.c`) — **the exact
function Prankster was wired into**, so each mirrors the Prankster `IsInnateActive()` clause:

- **Gale Wings** (+1 priority to the holder's Flying-type moves, only at full HP under
  `B_GALE_WINGS >= GEN_7`) — the chosen-ability branch's `ability == ABILITY_GALE_WINGS` test becomes
  `(ability == ABILITY_GALE_WINGS || IsInnateActive(battler, ABILITY_GALE_WINGS))`, leaving the full-HP
  and Flying-type guards shared. The full-HP gate is a restriction on the *ability*, not a cost to the
  holder, so the innate honors it identically (a below-full-HP innate grants no priority) — that's the
  1:1 copy, not a divergence.
- **Triage** (+3 priority to the holder's healing moves, `healingMove`-flagged) — same shape:
  `(ability == ABILITY_TRIAGE || IsInnateActive(battler, ABILITY_TRIAGE)) && IsHealingMove(move)`.

Both `IsInnateActive` reads are feature-gated and species-based, so with the feature off each is a
strict no-op and the real-ability path stays byte-for-byte unchanged. No script / pop-up / driver —
priority is a pure turn-order calc.

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); neither
ability is breakable, so Mold Breaker never touches them, same as the real abilities.

**AI.** Both are FREE: the AI's turn-order prediction (`AI_WhoStrikesFirst` → `GetBattleMovePriority`)
runs the same calc keyed off the real battler (every AI caller passes the real `battler`/`battlerAtk`
and its chosen ability, so the innate clause reads the actual battler), so the AI both threatens and
respects an innate Gale Wings / Triage. The one dedicated AI *effect* read is the doubles Psychic-Terrain
heuristic in `src/battle_ai_field_statuses.c` (Psychic Terrain blanks priority moves regardless of
source) — already innate-aware for Prankster; the fork helper `AI_IsInnateOnSide()` now sits beside the
chosen-only `AI_IsAbilityOnSide()` reads for Gale Wings and Triage too (both the foe-harass and
self-avoid branches), so the AI values/avoids the terrain for an innate-priority side.

**Species (canon-only, no flavor picks — a priority boost is potent, the same reasoning that kept
Prankster's flavor set tight):** Gale Wings → the Fletchling line (Fletchling, Fletchinder, Talonflame —
its canon Hidden-Ability users; no Mega/regional forms to mirror). Triage → Comfey (its only canon user),
merged into Comfey's existing `INNATES(LEVITATE, NATURAL_CURE)` row.

**Frontier (Step 3.5):** four hardcoded sets were freed. Both Talonflame Gale-Wings sets take a
complementary REAL slot — chosen **Flame Body** (`:x:` in the progress doc → stable), burning contact
attackers. Comfey is the ability-constrained case: its Triage (now innate) and Natural Cure (already
innate) are both redundant, and its only other real ability, Flower Veil, is still pending
(`:white_large_square:` → future churn). Its slot-1 Triage is pinned by upstream tests
(`upper_hand.c`, `ai_doubles.c`) so it can't be repurposed; instead the innate-redundant, test-unpinned
slot-2 Natural Cure is repurposed (`species_ability_overrides.c`) to chosen **Sweet Veil** — an
already-implemented `:white_check_mark:` innate (stable, like Slurpuff's Unaware) that Comfey lacks
natively and that is thematic (its soothing aroma keeps the doubles team awake).

### ABILITY_SURGE_SURFER / ABILITY_GRASS_PELT

The "terrain modifiers" (Batch R): each is a stat calc keyed off a field terrain — the terrain edition of
the weather speed-doublers / defensive boosters. Both are **clean upsides** that never hurt the holder, so
each innate is a plain **1:1 copy** — no pure-boon divergence. Each is wired at a single shared-calc site:

- **Surge Surfer** (Speed ×2 on Electric Terrain) — wired in `GetBattlerTotalSpeedStat`
  (`src/battle_main.c`), beside the four weather speed-doublers: the chosen-ability branch's
  `ability == ABILITY_SURGE_SURFER` test becomes
  `(ability == ABILITY_SURGE_SURFER || IsInnateActive(battler, ABILITY_SURGE_SURFER))`, leaving the
  `gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN` guard shared. It sits in the same `else if` chain as
  Quick Feet (a statused Quick Feet holder wins the chain), exactly like the real ability.
- **Grass Pelt** (Defense ×1.5 on Grassy Terrain) — the upstream effect lives in a
  `switch (ctx->abilities[battlerDef])` in `CalcDefenseStat` (`src/battle_util.c`) that dispatches on the
  *chosen* ability, so (like the Batch B / N reducers Fur Coat and Marvel Scale beside it) the innate
  clause is an additive `if` *after* the switch: `usesDefStat && (fieldStatuses & GRASSY_TERRAIN) &&
  ctx->abilities[battlerDef] != ABILITY_GRASS_PELT && ctx->innatesEnabled &&
  IsInnateActive(battlerDef, ABILITY_GRASS_PELT)`. The `!= ABILITY_GRASS_PELT` guard stops a chosen
  Grass Pelt from double-applying; `usesDefStat` gates it to physical hits, the same gate the real
  ability uses.

Both `IsInnateActive` reads are feature-gated and species-based, so with the feature off each is a strict
no-op and the real-ability path stays byte-for-byte unchanged. No script / pop-up / driver — these are
pure stat calcs, so no `DETERMINISTIC_*` surface is touched (neither reads accuracy / secondary effects /
crits / held items).

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); neither
ability is breakable, so Mold Breaker never touches them, same as the real abilities.

**AI.** On-field prediction is FREE: both effects live in shared calcs the AI runs keyed off the real
battler — `GetBattlerTotalSpeedStat` (turn-order) for Surge Surfer, `CalcDefenseStat` (damage) for Grass
Pelt — so the AI threatens/respects both automatically. The one dedicated AI *effect* read is the
terrain-*setting* heuristic in `src/battle_ai_field_statuses.c` ("should I set this terrain?"): a new fork
companion `DoesInnateBenefitFromFieldStatus(battler, fieldStatus)` — mirroring the existing
`DoesInnateBenefitFromWeather` — is ORed in beside the chosen-only
`DoesAbilityBenefitFromFieldStatus(...)` reads at the Electric- and Grassy-Terrain call sites, so the AI
values setting the terrain for an innate-Surge-Surfer / innate-Grass-Pelt side too.

**Species (canon-only, no flavor picks):** Surge Surfer → Raichu-Alola (its only canon user). Grass Pelt →
the Skiddo line (Skiddo, Gogoat — its canon Hidden-Ability users).

**Frontier (Step 3.5):** three hardcoded sets were freed. Raichu-Alola is the ability-locked case — its
*only* real ability was Surge Surfer, now innate — so it takes a fork-owned chosen **Lightning Rod**
override (`species_ability_overrides.c`; `:x:` in the progress doc → stable, on-theme for the Electric
mouse: draws in Electric moves for immunity + a Sp. Atk boost), the same trick as Cornerstone Ogerpon.
Gogoat's Grass-Pelt set instead frees its slot to its complementary REAL **Sap Sipper** (slot 0; Grass
immunity + Attack boost), which stacks with the innate Grass Pelt Defense boost.

### ABILITY_HUGE_POWER / ABILITY_PURE_POWER

The "physical-Attack doublers" (Batch C): each doubles the holder's physical Attack. Both are **clean
upsides** that never hurt the holder, so each innate is a plain **1:1 copy** — no pure-boon divergence.
Upstream handles the two identically in **one shared `case ABILITY_HUGE_POWER: case ABILITY_PURE_POWER:`**
of the attack-stat `switch (ctx->abilities[battlerAtk])` in `CalcAttackStat` (`src/battle_util.c`), so the
innate is a single additive clause *after* the switch (same shape as Batch A's Stakeout / Rocky Payload /
Guts, in the `if (ctx->innatesEnabled)` block):

```c
if (IsBattleMovePhysical(move)
 && atkAbility != ABILITY_HUGE_POWER && atkAbility != ABILITY_PURE_POWER
 && (IsInnateActive(battlerAtk, ABILITY_HUGE_POWER) || IsInnateActive(battlerAtk, ABILITY_PURE_POWER)))
    modifier = uq4_12_multiply_half_down(modifier, UQ_4_12(2.0));
```

Because the two share one effect (×2 physical), the guard skips whenever the **chosen** ability is
*either* one — the switch already applied the ×2 — so a mon running the real Huge Power *or* Pure Power
never double-applies (a holder with chosen Huge Power + innate Pure Power still doubles only once, correctly).
`IsInnateActive` is feature-gated and species-based, so with the feature off the clause is a strict no-op
and the real-ability path stays byte-for-byte unchanged. No script / pop-up / driver, and no
`DETERMINISTIC_*` surface is touched (a pure attack-stat calc — no accuracy / secondary effects / crits /
held items).

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); neither
ability is breakable, so Mold Breaker never touches them, same as the real abilities.

**AI.** On-field damage prediction is FREE: the ×2 lives in the shared `CalcAttackStat` the AI runs keyed
off the real battler. The `BattlerBenefitsFromAbilityScore` read at `case ABILITY_HUGE_POWER: case
ABILITY_PURE_POWER:` (`src/battle_ai_util.c`) is an ability-*value* rating for Trace/Skill-Swap-style
decisions (keyed off the hypothetical ability, not the battler's real one), NOT an effect read, so it is
correctly left untouched — same call the other calc-modifier batches leave alone.

**Species (canon-only, no flavor picks):** Huge Power → the Marill line (Azurill, Marill, Azumarill — each
merged with its existing innate Thick Fat) and the Diggersby line (Bunnelby, Diggersby). Pure Power → the
Meditite line (Meditite, Medicham). Mega Mawile / Mega Starmie carry Huge Power as their *only, always-chosen*
ability, so the innate could never be observed — omitted as redundant (their non-Mega bases are NOT Huge
Power users, so there is no base-creature trait to persist through the Mega). Mega Medicham is likewise
sole-Pure-Power and not in the roster, so it is omitted for the same reason.

**Frontier (Step 3.5):** seven hardcoded sets were freed. The three Azumarill sets frees their slot to its
complementary REAL **Sap Sipper** (slot 2 HA; `:x:` → stable; Grass immunity + Attack boost). Medicham
(chosen Pure Power now innate; its only other real ability, Telepathy, is dead in singles) takes a chosen
**Reckless** via a fork-owned override on its EMPTY slot 1 (`species_ability_overrides.c`) — an
implemented `:white_check_mark:` innate it lacks, stable like Slurpuff's Unaware, powering up its High Jump
Kick STAB. Diggersby's *other* two real abilities (Pickup, Cheek Pouch) are both still *pending* innates, so
rather than bake in churn its now-redundant slot-2 Huge Power is repurposed via override to a chosen
**Scrappy** (implemented `:white_check_mark:`, stable; audited — no test pins Diggersby's Huge Power) so its
Normal STAB (Return / Quick Attack) hits Ghosts.

### ABILITY_CLEAR_BODY / ABILITY_WHITE_SMOKE / ABILITY_HYPER_CUTTER / ABILITY_BIG_PECKS

The "stat-drop protectors" (Batch D+E, folding the single-stat Batch E into the full-protection Batch D):
**Clear Body** and **White Smoke** keep *any* of the holder's stats from being lowered by another mon's
move or ability; **Hyper Cutter** protects **Attack**, **Big Pecks** protects **Defense**. All four are
**clean upsides** that never hurt the holder, so each innate is a plain **1:1 copy** — no pure-boon
divergence.

**Effect site — one shared block in `IsAbilityBlocked` (`src/battle_stat_change.c`).** The prior Keen Eye /
Illuminate accuracy block (an innate whose accuracy-drop immunity landed here in Batch P) is **generalized**:
a new helper `GetInnateStatDropProtector(battler, stat, &fullProtection)` returns the innate ability that
would block a drop of `stat` on `battler` — Clear Body / White Smoke for any stat (setting `*fullProtection`),
Hyper Cutter for Attack, Big Pecks for Defense, Keen Eye / Illuminate for accuracy — or `ABILITY_NONE`. The
block runs only when the **chosen** ability doesn't already block the drop (the existing
`!CanAbilityPreventStatLoss && !AbilityPreventsSpecificStatDrop` guard), so the real-ability path is
untouched. On a hit, a **full** protector uses `MarkStatsAsDone(NUM_BATTLE_STATS)` +
`BattleScript_AbilityNoStatLoss` ("…'s stats were not lowered!"); a **single-stat** protector uses
`MarkStatsAsDone(st->stat)` + `BattleScript_AbilityNoSpecificStatLoss` ("…'s <stat> was not lowered!").
Because `CreateAbilityPopUp` reads the *primary* slot, the block sets
`gBattleScripting.abilityPopupOverwrite = innate` (and `gLastUsedAbility` / `RecordAbilityBattle` to the
innate) so the pop-up/reveal show the innate, not the chosen ability — the Keen Eye / Sturdy / Levitate
overwrite precedent. No driver is needed (the scripts/messages already exist upstream). **No
`DETERMINISTIC_*` surface is touched** — stat drops from *primary* status moves and from *secondary*
additional effects both route through this same `IsAbilityBlocked`, so the deterministic
additional-effects/hold-effect reroutes need no separate mirror.

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / Ability Shield /
not-on-field); all four are **breakable**, so an attacker's Mold Breaker pierces an innate one exactly as it
would the real ability.

**AI.** Stat-drop reasoning lives in **dedicated AI helpers**, not the shared damage calc, so two effect
reads were made innate-aware (mirroring the Keen Eye / Speed Boost precedent already in these functions):
`CanLowerStat` (`src/battle_ai_util.c`) — the AI won't waste a stat-lowering move on an innate Clear Body /
White Smoke holder (any stat), an innate Hyper Cutter holder's Attack, or an innate Big Pecks holder's
Defense; and `CanIntimidateLowerOpponentAtk` (`src/battle_ai_switch.c`) — the AI won't switch an Intimidator
in expecting to lower the Attack of an innate Clear Body / White Smoke / Hyper Cutter opponent. The
`BattlerBenefitsFromAbilityScore` read at `case ABILITY_CLEAR_BODY: case ABILITY_WHITE_SMOKE:`
(`src/battle_ai_util.c`) is an ability-*value* rating for Trace/Skill-Swap-style decisions (keyed off the
hypothetical ability, not the battler's real one), NOT an effect read, so it is correctly left untouched.

**Species (canon-only, no flavor picks):** every species carrying one of the four in its real ability data,
in any slot, plus the Mega/Gigantamax mirrors (the FORMS pure-boon convention) — e.g. Clear Body on the
Tentacool, Metagross (+ Mega), Regi trio, Klink, Carbink, Diancie (+ Mega), Dreepy and Nacli lines; White
Smoke on Torkoal, Heatmor and the Sizzlipede line (+ Gmax); Hyper Cutter on the Krabby (+ Gmax), Pinsir
(+ Mega), Gligar, Mawile (+ Mega), Trapinch, Corphish and Crabrawler lines; Big Pecks on the Pidgey (+ Mega),
Pidove, Ducklett, Vullaby, Fletchling and Rookidee lines, Chatot and Bombirdier. A species already carrying
another innate is *merged* into its existing row.

**Frontier (Step 3.5):** 28 hardcoded sets were freed. Most take a **complementary REAL slot** with no
override: Kingler's Hyper Cutter → chosen **Sheer Force** (`:x:`), Pinsir's → chosen **Moxie** (HA),
Crabominable's → chosen **Anger Point**, Torkoal's White Smoke → chosen **Drought** (`:x:`), Centiskorch's →
chosen **Flash Fire** (`:x:`), Dragapult's Clear Body → chosen **Infiltrator**. The
*all-real-abilities-innate* mons take a fork-owned override (`species_ability_overrides.c`): **Pidgeot** /
**Chatot** (Keen Eye + Tangled Feet + Big Pecks all innate) → a chosen **No Guard** (`:x:`) / **Punk Rock**
(`:white_check_mark:`), **Crawdaunt** (Hyper Cutter + Shell Armor + Adaptability) → chosen **Sniper**
(`:white_check_mark:`; its slot-2 Adaptability is pinned by `adaptability.c`, so the unpinned slot-1 Shell
Armor is repurposed), **Bombirdier** (Big Pecks + Keen Eye + Rocky Payload) → chosen **Reckless**
(`:white_check_mark:`), **Klinklang** (Clear Body innate; Plus/Minus dead in singles, slot-0 Plus pinned by
doubles/anim tests) → chosen **Motor Drive** (`:x:`) in its unpinned slot-1 Minus, and **Metagross /
Regirock / Regice / Registeel / Carbink / Diancie** (Clear Body [+ Sturdy] innate, EMPTY slot 1) → chosen
**Tough Claws / Solid Rock / Ice Scales / Bulletproof / Solid Rock / Solid Rock** — each stable and thematic.

### ABILITY_DAZZLING / ABILITY_QUEENLY_MAJESTY / ABILITY_ARMOR_TAIL

The "priority-move blockers" (Batch F): opponents cannot use increased-priority moves against the holder
**or its allies**. All three are **clean upsides** that never hurt the holder, so each innate is a plain
**1:1 copy** — no pure-boon divergence.

**Effect site — one shared block in `CancelerPriorityBlock` (`src/battle_move_resolution.c`).** The canceler
loops over the opposing battlers and asks whether any of them blocks the incoming priority move. That read
was routed through a new helper `GetBattlerDazzlingAbility(battler, chosenAbility)`, which returns the
blocking ability the battler carries as either its already-cached **chosen** ability (the plain
`IsDazzlingAbility` test, unchanged) or an active **innate** (`IsInnateActive`, innate-only so it can't leak
the chosen slot), else `ABILITY_NONE`. Because `CreateAbilityPopUp` reads the *primary* slot, the block sets
`gBattleScripting.abilityPopupOverwrite = ability` **only when the chosen ability differs**, so the pop-up
shows the innate while the real-ability path stays byte-for-byte unchanged (`gLastUsedAbility` /
`RecordAbilityBattle` / `BattleScript_PokemonCannotUseMove` all reused). No driver is needed. **No
`DETERMINISTIC_*` surface is touched** — a priority block is a turn-order gate, not an accuracy / secondary /
crit / held-item effect.

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / Ability Shield /
not-on-field); all three are **breakable**, so an attacker's Mold Breaker / Teravolt pierces an innate one
exactly as it would the real ability.

**AI.** The priority-block reasoning lives in a **dedicated helper** (`Ai_IsPriorityBlocked`,
`src/battle_ai_util.c`), NOT the shared damage calc, so it had to be made innate-aware: its two
`IsDazzlingAbility(aiData->abilities[...])` reads (the defender and, in doubles, its partner) now go through
the same `GetBattlerDazzlingAbility` helper, so the AI won't waste a priority move (First Impression, Quick
Attack, …) into an innate blocker. On-field only, so `IsInnateActive` covers it.

**Species (canon-only, no flavor picks — blocking every priority move against a mon *and its allies* is a
potent, hard-to-justify-broadly effect, so the set stays the three canon carriers):** **Dazzling** on
Bruxish, **Queenly Majesty** on Tsareena (merged into its existing Sweet Veil row), **Armor Tail** on
Farigiraf. (Tsareena's pre-evolutions and Girafarig don't carry the ability in their real data, so they're
correctly omitted.)

**Frontier (Step 3.5):** all six hardcoded sets were freed. The **Tsareena** sets take their complementary
REAL slot-0 **Leaf Guard** (Sweet Veil is already Tsareena's innate, so it can't be reused), the **Farigiraf**
sets take their complementary REAL HA **Sap Sipper** (`:x:`, a Grass immunity + Attack boost). **Bruxish**
is the *all-real-abilities-innate* case — Dazzling **and** Strong Jaw (Batch A) **and** Wonder Skin (Batch P)
are all now innate — so it takes a fork-owned override (`species_ability_overrides.c`) repurposing its
innate-redundant, test-unpinned slot-1 Strong Jaw to a chosen **Sheer Force** (`:x:`, powers up its biting
kit); its slot-0 Dazzling stays a real ability because `dazzling.c` / `bide.c` / `last_resort.c` pin it.

### ABILITY_PROPELLER_TAIL / ABILITY_STALWART

The "redirection-ignore" abilities (Batch G): the holder's moves ignore every form of move redirection and
hit the originally-selected target. Both abilities have the **identical** effect and are **clean upsides**
that never hurt the holder, so each innate is a plain **1:1 copy** — no pure-boon divergence.

**Effect sites — the shared redirection points, all in `src/battle_move_resolution.c` plus one in
`src/battle_anim_effects_1.c`:**
- **`IsAffectedByFollowMe`** — the Follow Me / Rage Powder redirect gate. Beside the existing chosen-ability
  `IsAbilityAndRecord(battlerAtk, ability, ABILITY_PROPELLER_TAIL/STALWART)` tests, an
  `IsInnateActive(battlerAtk, …)` clause makes an innate holder immune to the redirect too.
- **The Lightning Rod / Storm Drain redirect loop in `HandleMoveTargetRedirection`** — the loop that scans for
  an ability-redirector is skipped when the attacker ignores redirection; the two `!IsAbilityAndRecord(...)`
  guards each gained a matching `!IsInnateActive(cv->battlerAtk, …)` clause.
- **The Ally Switch retarget in `AnimTask_...` (`src/battle_anim_effects_1.c`)** — after an Ally Switch swaps
  the two allies' positions, a move aimed at one of them normally follows the swap; Snipe Shot / Stalwart /
  Propeller Tail keep the original target. The `ability == ABILITY_PROPELLER_TAIL/STALWART` reads there gained
  `|| IsInnateActive(i, …)` so an innate holder keeps its target through the switch as well.

`IsInnateActive` is feature-gated and species-based, so with the feature off every clause is a strict no-op;
none of them record or leak the chosen slot (the chosen-ability path keeps its `IsAbilityAndRecord`).
**No `DETERMINISTIC_*` surface is touched** — redirection is a pure targeting decision, not an accuracy /
secondary / crit / status / held-item effect. No script/pop-up is needed (redirection is silent).

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / Ability Shield /
not-on-field); neither ability is breakable, so Mold Breaker never touches them, same as the real abilities.

**AI.** Move-redirection prediction lives in a **dedicated helper** (`IsMoveRedirectionPrevented`,
`src/battle_ai_util.c`), NOT the shared damage calc, so it had to be made innate-aware: beside its
`atkAbility == ABILITY_PROPELLER_TAIL/STALWART` reads, an `IsInnateActive(battlerAtk, …)` clause (the
attacker is always on-field here) lets the AI correctly predict its own redirection-ignore from an innate
when picking a spread/redirect-sensitive target.

**Species (canon-only, no flavor picks — redirection-ignore is an abstract, doubles-oriented mechanic that is
hard to justify thematically and potent in the format, so the set stays the canon carriers):**
**Propeller Tail** on the Arrokuda / Barraskewda line (merged into their existing Swift Swim rows),
**Stalwart** on Duraludon / Duraludon-Gmax / Archaludon (merged into Archaludon's existing Sturdy row) and
**Skarmory-Mega** (a fork form whose ability data is all-Stalwart; merged into its existing Sturdy row).

**Frontier (Step 3.5):** both hardcoded sets were freed. They are the two **Barraskewda** sets, whose
`.ability` slot was already spent on Propeller Tail (Swift Swim was innate first). Barraskewda is the
*all-real-abilities-innate* case — its only real abilities (Swift Swim, Propeller Tail) are now both innate —
so it takes a fork-owned override (`species_ability_overrides.c`) filling its EMPTY slot 1 with a chosen
**Water Absorb** (`:x:`, a Water immunity + heal, the same pick used for the other water mons in that table),
so both sets now run Water Absorb **and** the innate Swift Swim / Propeller Tail. Duraludon / Archaludon have
no frontier set, so no roster change was needed there.

### ABILITY_SHADOW_TAG / ABILITY_ARENA_TRAP / ABILITY_MAGNET_PULL

The "trapping" abilities (Batch H): the holder keeps opposing mons from switching out or fleeing.
**Shadow Tag** traps any foe (except one that itself carries Shadow Tag, chosen or innate, under
`B_SHADOW_TAG_ESCAPE >= GEN_4`); **Arena Trap** traps grounded foes; **Magnet Pull** traps Steel-types
(regardless of grounding). All three are **clean upsides** that never hurt the holder, so each innate is a
plain **1:1 copy** — no pure-boon divergence.

**Effect site — one shared chokepoint, `IsAbilityPreventingEscape` (`src/battle_util.c`).** The three
per-battler `ability == ABILITY_X` reads were factored into a new helper
`GetBattlerEscapePreventionAbility(battler, trapper)` that returns the *specific* trapping ability (or
`ABILITY_NONE`), each read swapped to the chosen-or-innate predicate `BattlerHasAbility()` — including the
Shadow-Tag self-exemption (`!BattlerHasAbility(battler, ABILITY_SHADOW_TAG)`), so a mon's *own* innate Shadow
Tag frees it just like the real ability. `IsAbilityPreventingEscape` now loops calling the helper. Because
`IsInnateActive` (inside `BattlerHasAbility`) is feature-gated and species-based, the whole thing is a strict
no-op with the feature off and never leaks the chosen slot. **No `DETERMINISTIC_*` surface is touched** —
trapping is a pure switch-legality decision, not an accuracy / secondary / crit / status / held-item effect.

**Display.** The escape/switch UI names the trapping ability, and for an innate trapper the naive read shows
the holder's *unrelated chosen* ability. Returning the ability from the helper lets the **can't-switch party
menu** ("… is preventing switching out with its {ability}!", via the `PARTY_ACTION_ABILITY_PREVENTS` emit in
`src/battle_main.c`) name the real trapper; the RUN-button `gLastUsedAbility` is set from it too. In the
vanilla real-ability case the helper returns the same ability shown before, so display is byte-for-byte
unchanged. **Known minor limitation:** the RUN-button `STRINGID_PREVENTSESCAPE` text and the wild-Teleport
"made it ineffective" ability pop-up read the battler's *primary* slot deep in the message system
(`sBattlerAbilities` / `showabilitypopup`), so those two surfaces still name the chosen ability for an innate
trapper. The trap itself always works; only that flavor text is imperfect, and only in wild battles.

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / Ability Shield /
not-on-field); none of the three is breakable, so Mold Breaker never touches them, same as the real abilities.

**AI.** Trapping reasoning lives in **dedicated helpers**, NOT the shared damage calc, so each was made
innate-aware:
- **`IsBattlerTrapped`** (`src/battle_ai_util.c`) — beside its `AI_IsAbilityOnSide(battlerAtk, ABILITY_X)`
  reads (which side has the trapping ability), an `AI_IsInnateOnSide(battlerAtk, ABILITY_X)` clause credits an
  innate trapper, and the Shadow-Tag self-exemption gained `&& !IsInnateActive(battlerDef, ABILITY_SHADOW_TAG)`
  so an innate holder is correctly treated as free to leave.
- **`AI_CanSwitchinAbilityTrapOpponent`** (`src/battle_ai_switch.c`, the "switch in my trapper" heuristic) —
  the trapper is usually a *benched* candidate with no battler index, so the function gained a `trapperSpecies`
  parameter and checks `SpeciesHasInnate(trapperSpecies, ABILITY_X)` (config-gated) off-field, mirroring
  Levitate's benched-mon absorb wiring; the on-field opposing-battler Shadow-Tag mutual check gained
  `|| IsInnateActive(opposingBattler, ABILITY_SHADOW_TAG)`. Callers pass the candidate's species (and
  `SPECIES_NONE` for the Trace branch, which copies only the chosen ability).

The `AI_DecideKnownAbilityForTurn` "treat a trapping ability as always known" read stays chosen-only — it is
identity bookkeeping about the mon's *displayed* ability, not an effect read (innates are never the chosen
identity). The overworld wild-encounter lures (Magnet Pull / Arena Trap in `src/wild_encounter.c`) likewise
stay keyed to the lead's chosen ability — innates are a battle-only feature.

**Species (canon-only, no flavor picks — preventing the foe from switching is a potent utility effect, so the
set stays the canon carriers).** **Shadow Tag:** Wobbuffet, Wynaut, the Gothita line, and the Litwick line
(merged into its existing Levitate rows). Mega Gengar is *omitted* as redundant (its only ability IS Shadow
Tag, so an innate could never be observed). **Arena Trap:** Diglett / Dugtrio (Kantonian only — the Alolan
forms lack it) and Trapinch (merged into its Hyper Cutter row). **Magnet Pull:** Magnemite / Magneton /
Magnezone, the Alolan Geodude line, and Nosepass / Probopass (all merged into existing rows). Meltan is
*omitted* as redundant (sole-ability, not a frontier set).

**Frontier (Step 3.5):** eight hardcoded sets were freed. **Golem-Alola** simply repoints to its real
`:x:` Galvanize slot (Magnet Pull & Sturdy now innate). The rest take fork-owned overrides
(`species_ability_overrides.c`): **Dugtrio** (all three abilities innate) → chosen **Sand Stream** (repurposing
its innate-redundant Sand Force slot; self-synergistic with innate Sand Veil / Sand Force); **Magnezone** and
**Probopass** (all abilities innate) → chosen **Lightning Rod** (`:x:`, draws Electric for immunity + Sp. Atk);
**Gothitelle** → chosen **Unaware** repurposing its innate-redundant Shadow Tag slot (its real Frisk / Competitive
slots are kept intact for those future innates). Each repurposed slot was audited against `test/battle/`
`Ability(ABILITY_X)` pins (Dugtrio's Arena Trap, Magnezone's Magnet Pull / Sturdy are pinned and were left alone).
**Wobbuffet's set was deliberately *not* freed**: its only complementary slot is the empty slot 1, and filling it
via an override would change Wobbuffet's game-wide ability data — but Wobbuffet is a ubiquitous test mon whose empty
slot is exercised by `Ability(ABILITY_NONE)` (e.g. `test/battle/ai/gimmick_z_move.c`), so the set keeps its chosen
Shadow Tag (redundant with the now-innate one, but harmless) rather than risk those tests.

### ABILITY_MAGMA_ARMOR / ABILITY_WATER_VEIL / ABILITY_OWN_TEMPO / ABILITY_INNER_FOCUS / ABILITY_LEAF_GUARD / ABILITY_OVERCOAT

The **status-condition immunities** (Batch I): each blocks a specific status/effect on its
holder — Magma Armor (freeze/frostbite), Water Veil (burn), Own Tempo (confusion), Inner Focus
(flinching), Leaf Guard (all non-volatile status *while the holder is in harsh sunlight*), Overcoat
(powder moves + sandstorm/hail chip damage). All are **1:1 clean-upside copies** — the same class as
the already-done Limber / Immunity / Insomnia — so no pure-boon divergence.

**Effect sites (all `src/battle_util.c` unless noted):**
- **Magma Armor / Water Veil** — the freeze and burn cases of `CanSetNonVolatileStatus` gain an
  `IsInnateActive()` clause beside the cached chosen-ability test (Water Veil rides the existing
  Water Bubble branch). Because `CanBeBurned`/`CanBeFrozen` (and the AI's status-move scoring
  through them) route through `CanSetNonVolatileStatus`, those are innate-aware for free.
- **Own Tempo** — `CanBeConfused` (blocks confusion; a silent pure-boon immunity, no pop-up — the
  confuse move's `jumpifability` reads only the chosen slot, matching the Levitate silent-immunity
  precedent) plus the AI's `AI_CanBeConfused` (`src/battle_ai_util.c`).
- **Inner Focus** — the `MOVE_EFFECT_FLINCH` case of `SetMoveEffect` (`src/battle_script_commands.c`)
  and the `DETERMINISTIC_HOLD_EFFECTS` King's-Rock would-it-land mirror (`src/battle_hold_effects.c`);
  the AI's flinch-move scoring (`ShouldTryToFlinch` + the reliable-effect flinch check,
  `src/battle_ai_util.c`).
- **Leaf Guard** — made innate-aware at its single helper `IsLeafGuardProtected`
  (`src/battle_script_commands.c`), which covers the battle effect (`CanSetNonVolatileStatus`), the
  Rest-prevention gate, the end-turn status site, and the AI switch read in one edit. Sun-gated: no
  sun ⇒ no immunity.
- **Overcoat** — `IsAffectedByPowderMove` (powder immunity) and the sandstorm/hail end-turn block
  (`src/battle_end_turn.c`); the AI's sand/hail damage predictors + switch-in weather impact +
  powder-absorb switch heuristic (`src/battle_ai_util.c` / `src/battle_ai_switch.c`).

**Intimidate immunity (Inner Focus + Own Tempo).** Both are also unaffected by Intimidate (GEN_8+),
wired beside the Oblivious/Scrappy innate detection in `IsIntimidateBlocked`
(`src/battle_stat_change.c`) and the Intimidate switch-in heuristic (`src/battle_ai_switch.c`).

**Switch-in status cure.** Each status-immunity innate also self-cures its status on switch-in
(`TryImmunityAbilityHealStatus`), exactly like the real ability: Water Veil/Water Bubble cure burn,
Magma Armor cures freeze/frostbite, Own Tempo cures confusion (reachable only when a Mold Breaker
move pierced the innate first — confusion is a volatile cleared on switch-out otherwise). The pop-up/
record overwrite shows the innate, not the chosen ability.

**Suppression parity** holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field);
Magma Armor / Water Veil / Own Tempo / Inner Focus / Leaf Guard / Overcoat are all breakable, so an
attacker's Mold Breaker pierces the innate exactly as it would the real ability.

**Species selection — canon-only, with two contradiction carve-outs and one flavor set.** Every canon
user (any ability slot, forms + Mega parity per the FORMS/Mega rules) carries the innate. Magma Armor
adds a tight molten/burning-body flavor set that can't freeze (Torkoal, the Coalossal line). Two
contradiction carve-outs (like Zangoose's Immunity/Toxic Boost): **Spinda keeps innate Tangled Feet,
not Own Tempo** (Own Tempo would block the confusion Tangled Feet needs). **Sole-ability species are
omitted as redundant** (their sole chosen ability already grants the effect) *unless* they are a
frontier set — **Zarude** (sole Leaf Guard) and **Enamorus-Therian** (sole Overcoat) instead take the
innate + a fork-owned chosen override (Tough Claws / Sheer Force), like Ogerpon-Cornerstone.

**Frontier slot freeing (Step 3.5) is partial for this batch.** These status-immunity species are
unusually innate-dense — many already carry several innates — so most frontier sets that hardcoded one
of these six abilities are on species whose *remaining* real slots are all likewise innate (or a
drawback, or a still-pending innate). The cleanly-tractable sets were freed: 11 species took an
**empty-slot override** to a stable pick (Dragonite→Reckless, Meganium→Grassy Surge, Forretress→Filter,
Raikou→Lightning Rod, Entei→Flame Body, Suicune→Water Absorb, Huntail/Floatzel→Water Absorb,
Revavroom→Sheer Force, plus Zarude→Tough Claws, Enamorus-Therian→Sheer Force), and 5 were repointed to
an existing stable real slot (Kommo-o→Bulletproof, Lurantis→Contrary, Slowking-Galar→Curious Medicine,
Lickilicky→Cloud Nine, Oranguru→Symbiosis). The remaining ~24 all-innate species (Slowbro/Slowking,
Hitmonchan, Kangaskhan, the Slow/Lilligant/Tsareena/etc. lines) keep `.ability` on the now-innate
ability — functional and CI-safe (it still resolves to a real slot), just redundant — because freeing
them would require overrides that *delete* a real ability game-wide (the override table is not
feature-gated), each needing a per-species `test/battle` audit; that broad sweep is deferred to a
focused follow-up rather than risked here.

### ABILITY_SUCTION_CUPS / ABILITY_GUARD_DOG

Both resist being forced out of battle (Batch S). Wired at the two C forced-switch sites in
`src/battle_move_resolution.c`: the Dragon Tail / Circle Throw hit-switch (`EFFECT_HIT_SWITCH_TARGET`,
Guard Dog silently breaks, Suction Cups shows its anchor pop-up) and the Red Card activation
(`TryRedCardActivation`). Roar / Whirlwind (`EFFECT_ROAR`) resolve through a *battle script* whose
`jumpifability` reads only the chosen slot, so `BS_JumpIfRoarFails` (`src/battle_script_commands.c`) is
made innate-aware there: an innate Guard Dog fails the phaze plainly, an innate Suction Cups jumps to
`BattleScript_AbilityPreventsPhasingOut` with the pop-up overwritten to Suction Cups. Both are 1:1
clean-upside copies (no downside). **Guard Dog's Intimidate half (Batch V, now wired):** the forced-switch
block shipped in Batch S; Guard Dog's **Intimidate-immunity + Attack-boost** half — deferred until the
Intimidate switch-in driver existed — is now wired at the Intimidate-reaction site
`IsIntimidateBlocked` (`src/battle_stat_change.c`). An innate Guard Dog (chosen ability differs, so the
`ABILITY_GUARD_DOG` switch case misses it) is immune to Intimidate's Attack drop and instead boosts its
own Attack by 1 stage, mirroring the chosen case (Flower Veil ordering, min-stage guard,
`BattleScript_DefiantActivates` script) with the pop-up/record overwritten to Guard Dog. AI: the two
`ShouldSwitchIfIntimidateBenefit` reads (`src/battle_ai_switch.c`) credit an innate Guard Dog beside
Defiant / Competitive / Rattled, so the AI won't switch an Intimidator in expecting to weaken an
innate-Guard-Dog foe; the soft incoming-ability *value* scorer in `battle_ai_util.c`
(`case ABILITY_INTIMIDATE`, which reads a foe's chosen ability via `DoesIntimidateRaiseStats`) stays
keyed to the chosen slot, consistent with how the Intimidate driver left its softest heuristics. AI: the
Roar-scoring reads in `src/battle_ai_main.c` (`EFFECT_ROAR` + Suction Cups, both the score-penalty and
the doubles no-op) credit an innate Suction Cups. Canon-only (no flavor picks). Guard Dog's canon users
(Mabosstiff, Okidogi) are frontier sets whose slot is freed via a fork-owned override (Strong Jaw /
Toxic Chain).

### ABILITY_ROCK_HEAD

Negates recoil damage from the holder's own moves (Batch S), 1:1 clean-upside copy. Wired at the single
recoil site in `src/battle_move_resolution.c` (`EFFECT_RECOIL` / `EFFECT_CHLOROBLAST`) beside the chosen
`IsAbilityAndRecord(...ABILITY_ROCK_HEAD)` test — an `IsInnateActive` clause that does NOT record (the
chosen slot stays identity, mirroring the Batch G Propeller Tail/Stalwart pattern). No pop-up (silent).
AI is innate-aware: `AI_IsDamagedByRecoil` (`src/battle_ai_util.c`) credits an innate Rock Head so the AI
doesn't over-fear its own recoil. Canon-only. Many canon Rock Head users are frontier sets; those whose
every real ability is now innate take a fork-owned override.

### ABILITY_LONG_REACH

Makes all of the holder's moves non-contact (Batch S), 1:1 clean-upside copy. Wired at the single contact
chokepoint `IsMoveMakingContact` (`src/battle_util.c`) — an `IsInnateActive` clause after the chosen
`abilityAtk == ABILITY_LONG_REACH` test (no record). Because every contact-triggered effect (Rocky
Helmet, Rough Skin/Iron Barbs, Static/Flame Body, Pickpocket, King's Rock-on-contact, …) flows through
this one predicate, the innate is covered everywhere for free. AI is innate-aware: the Rocky
Helmet/Iron Barbs move-comparison in `AI_CompareDamagingMoves` and `AI_MoveMakesContact`
(`src/battle_ai_util.c`) credit an innate Long Reach. Canon-only (the Rowlet line).

### ABILITY_SKILL_LINK

Multistrike moves always hit the maximum number of times (Batch S), 1:1 clean-upside copy. Wired at the
multi-hit-count sites in `src/battle_move_resolution.c` (`CancelerMultihitMoves` +
`ShouldSkipAccuracyCalcPastFirstHit`) beside the chosen reads, including the `DETERMINISTIC_MOVE_RESULTS`
reroute (Skill Link forces `DETERMINISTIC_MULTI_HIT_MAX_COUNT` instead of `..._COUNT`, and guarantees
Population Bomb's loaded-dice count). AI hit-count prediction (`src/battle_ai_util.c`, three sites incl.
the deterministic-Population-Bomb and deterministic-multihit branches) is innate-aware, so the AI values
an innate Skill Link's guaranteed damage. No pop-up. Canon-only.

### ABILITY_INFILTRATOR

Ignores the foe's Light Screen / Reflect / Aurora Veil, Safeguard, Mist and Substitute (Batch S), 1:1
clean-upside copy. Wired at each foe-barrier site: `GetDefenderAbilitiesModifier`'s screen check
(`src/battle_util.c`), `IsSafeguardProtected` (`src/battle_util.c`), `IsMistProtected`
(`src/battle_stat_change.c`) and the Substitute-block resolver in `src/battle_script_commands.c` (the
`IsAbilityAndRecord` site gets an `IsInnateActive` clause, no record). The overworld
`OW_INFILTRATOR` wild-encounter lure stays keyed to the chosen ability (not a battle effect). AI is
innate-aware: the Mist-ignore check (`src/battle_ai_util.c`) and the Substitute/Shed-Tail scoring
(`src/battle_ai_main.c`) credit an innate Infiltrator. No pop-up. Canon-only.

### ABILITY_CORROSION

Lets the holder poison / badly-poison Poison- and Steel-type targets (Batch S), 1:1 clean-upside copy.
Wired at the single type-immunity gate in `CanSetNonVolatileStatus` (`src/battle_util.c`) — the innate
clause sits beside the chosen `abilityAtk != ABILITY_CORROSION` test, so a move-based poison from an
innate Corrosion holder lands on a Steel/Poison foe exactly as the real ability does (Corrosion only
ever applies to the holder's own poisoning, so this one site is the whole effect). No pop-up. Canon-only.

### ABILITY_STICKY_HOLD

Keeps the holder's item from being stolen or removed (Batch S), 1:1 clean-upside copy. Wired at the
common removal sites: Knock Off and Thief/Covet steal (`src/battle_move_resolution.c`, pop-up overwritten
to Sticky Hold), Trick/Switcheroo (`src/battle_script_commands.c`, pop-up overwrite), Incinerate and Bug
Bite (`src/battle_script_commands.c`, silent), and Magician (`src/battle_util.c`, silent). AI is
innate-aware: the Knock Off/Corrosive Gas/Thief scoring and the item-swap heuristics
(`src/battle_ai_main.c`, `src/battle_ai_util.c`) credit an innate Sticky Hold. **KNOWN LIMITATION:** an
innate Sticky Hold does NOT block a chosen Pickpocket's on-contact steal, nor Corrosive Gas — both route
through a battle script's `jumpifability` (chosen-slot-only), a cross-cutting change deferred out of
scope. Canon-only.

### ABILITY_UNSEEN_FIST / ABILITY_PIERCING_DRILL

Contact moves hit through the target's Protect (Batch S), an identical pair, both 1:1 clean-upside
copies. Wired at the two shared Protect sites in `src/battle_util.c` (`IsBattlerProtected`) and
`src/battle_move_resolution.c` (`CancelerPriorityBlock`-style protect resolver) — both already read both
abilities, so the innate clause adds `IsInnateActive` for each beside the chosen reads. AI is
innate-aware: `AI_CanContactBypassProtect` (`src/battle_ai_util.c`) credits both innates. No pop-up.
Canon-only. NOTE: Excadrill-Mega (the sole Piercing Drill user) is ability-locked to Piercing Drill in
all three slots, so its innate can't be observed distinctly from its chosen ability — the test asserts
membership only, since the effect site is identical to Unseen Fist's (which is fully exercised).

### ABILITY_HEAVY_METAL / ABILITY_LIGHT_METAL

Double / halve the holder's weight (Batch S), 1:1 clean-upside copies. Wired at the single weight calc
`GetBattlerWeight` (`src/battle_util.c`) beside the chosen `ability == ABILITY_HEAVY_METAL` /
`... LIGHT_METAL` tests, so every weight-based interaction (Low Kick / Grass Knot power against the
holder, its own Heavy Slam / Heat Crash, Sky Drop, Heavy Ball) reflects the innate. AI weight reads run
through the same calc, so they are innate-aware for free. No pop-up. Canon-only. NOTE: Duraludon (and its
Gigantamax) carry innate LIGHT_METAL only — although its real ability data lists BOTH Heavy and Light
Metal, they are contradictory as simultaneous innates (weight x2 vs /2), so the defensively useful
slot-0 Light Metal is chosen and Heavy Metal dropped; innate Heavy Metal still lives on the Aggron and
Copperajah lines.

### ABILITY_RAIN_DISH / ABILITY_ICE_BODY / ABILITY_SHED_SKIN / ABILITY_HYDRATION / ABILITY_HEALER / ABILITY_HARVEST / ABILITY_CUD_CHEW / ABILITY_PICKUP / ABILITY_BAD_DREAMS

The **end-of-turn effects** (Batch J), all 1:1 clean-upside copies. These nine are *active, scripted
end-turn* innates: they reuse the **existing Speed Boost driver** — added to `IsActiveEndTurnInnate`
(`src/fork/innate_abilities.c`), which `TryActivateInnateEndTurnEffects` dispatches from the
`THIRD_EVENT_BLOCK_ABILITIES_INNATE` end-turn step by delegating to the upstream
`AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, …)` case. Reusing the upstream case means the
heal / status-cure / item recovery / chip damage / script are identical to the real ability for free. The
only per-site edits in `src/battle_util.c` are forcing `gBattleScripting.abilityPopupOverwrite` to the
innate (via `gLastUsedAbility`) when the chosen ability differs — the Speed Boost pop-up precedent — so the
real-ability path stays byte-for-byte unchanged. Effects: **Rain Dish** heals 1/16 max HP in rain;
**Ice Body** heals 1/16 in snow/hail; **Shed Skin** self-cures a status (30%, always under
`DETERMINISTIC_ABILITIES`); **Hydration** cures status in rain; **Healer** cures an adjacent ally's status
(30%, always under `DETERMINISTIC_ABILITIES`); **Harvest** recovers a used Berry; **Cud Chew** re-eats a
Berry the turn after eating it; **Pickup** grabs an item consumed this turn; **Bad Dreams** chips
sleeping/Comatose foes 1/8 max HP each turn. Two extra fixes were needed: (1) the end-turn **Pickup**
recycle read the *chosen* ability to decide whether to grab the target's item — `Cmd_tryrecycleitem`
(`src/battle_script_commands.c`) now uses `BattlerHasAbility(gBattlerAttacker, ABILITY_PICKUP)` so an innate
Pickup grabs the foe's consumed item instead of failing on its own empty slot; (2) **Bad Dreams**' script
only shows (and clears) its pop-up when it damages a valid target, so the innate pop-up overwrite is gated
on a `BadDreamsHasValidTarget` check to avoid leaking the overwrite to the next pop-up when no foe is
asleep. Also the driver pins `gBattlerAbility = battler` before delegating, since the upstream cases show
the pop-up on `gBattlerAbility` but don't all set it (a foe eating a Berry earlier in the turn can leave it
stale). AI is innate-aware where the effect lives outside the shared calc: the weather-heal / status-cure
switch-in predictions (`GetSwitchinWeatherImpact`, `GetSwitchinStatusDamage`,
`GetSwitchinRecurringHealing`, the Rest-worthiness reads in `battle_ai_main.c`, the sleep-switch check and
hail-damage predictor) credit an innate Rain Dish / Ice Body / Shed Skin / Hydration. Canon-only for all
nine EXCEPT **Bad Dreams**, whose sole canon user (Darkrai) always has it chosen and so can't *observe* an
innate Bad Dreams — a tight dream-eater flavor pair (the Munna line, whose real abilities are all
non-Bad-Dreams) carries an observable innate Bad Dreams. Sole-ability species whose only ability is a
Batch J ability are omitted as redundant (Cascoon/Silcoon/Kakuna/Metapod/Pupitar/Audino-Mega/Darkrai-Mega)
unless they are a frontier set (Manaphy/Phione keep their rows).

### ABILITY_POISON_HEAL

Heals 1/8 max HP at the end of every turn while poisoned/badly-poisoned instead of losing HP (Batch J),
a 1:1 clean-upside copy. **NOT** a driver innate — it *replaces* the poison-damage step rather than adding
an end-turn effect, so it is wired at the poison-damage site `HandleEndTurnPoison` (`src/battle_end_turn.c`)
by swapping the chosen-only `ability == ABILITY_POISON_HEAL` for `BattlerHasAbility(battler,
ABILITY_POISON_HEAL)`, with the pop-up overwritten to the innate when the chosen ability differs. The AI's
poison-harm heuristics — `GetPoisonDamage`, `ShouldPoison`, the "poison is harmless to this foe" score
(`src/battle_ai_util.c`), and the switch-in toxic-spikes / recurring-heal / status-damage sims
(`src/battle_ai_switch.c`) — all credit an innate Poison Heal (mirroring the existing Toxic Boost / Immunity
fork pattern: `BattlerHasAbility` for an on-field battler, `SpeciesHasInnate` in the off-field switch sim).
The `BattlerBenefitsFromAbilityScore` Trace/transfer-scoring case is intentionally left chosen-only (innates
are never traced/swapped), as is the overworld poison-damage check (`event_object_movement.c`, keyed to the
chosen ability like other overworld reads). Canon-only (Gliscor, the Shroomish line). Its frontier sets
(Breloom, Gliscor) keep their now-innate `.ability` for now except Breloom, freed to a stable chosen Effect
Spore (its Toxic Orb still procs the innate heal); the rest of the Batch J frontier freeing is a deferred
follow-up (see `INNATE_ABILITIES_BATCHES.md`).

### ABILITY_GLUTTONY / ABILITY_RIPEN / ABILITY_CHEEK_POUCH / ABILITY_UNBURDEN

The berry/item-synergy set (Batch T), all **1:1 clean-upside copies** (none of the real abilities ever
hurts its holder). Each is wired beside its chosen-ability read with an `IsInnateActive()` clause; the four
are `canon-only` (no flavor picks — the effects are berry/item-conditional and hard to justify off-roster).

- **Gluttony** — eats a pinch Berry at 1/2 HP instead of 1/4. One site: `HasEnoughHpToEatBerry`
  (`src/battle_util.c`), where the 1/2-HP branch gains `|| IsInnateActive(battler, ABILITY_GLUTTONY)`.
  No dedicated AI read (the AI reaches the same threshold through this shared helper), no pop-up (a
  passive threshold; the Berry's own animation still shows).
- **Ripen** — doubles every Berry effect. ~11 sites: the Jaboca/Rowap/Enigma chip and the stat-raise
  berries (`GetBattlerAbility(battlerDef) == ABILITY_RIPEN` reads) and the cached-`ability` heal / PP /
  confuse-heal / Starf-style raises in `src/battle_hold_effects.c`; plus the resist-Berry damage cut
  (`src/battle_util.c`, in the shared damage calc via `ctx->innatesEnabled && IsInnateActive(...)`, so the
  AI's prediction is innate-aware for free) and the Micle-Berry accuracy boost. The two dedicated AI reads
  — the berry-KO ignore modifier (`src/battle_ai_util.c`) and the Recycle score (`src/battle_ai_main.c`) —
  each credit an innate Ripen so the AI reasons about the doubled berry. No pop-up.
- **Cheek Pouch** — heals 1/3 max HP whenever the holder eats a Berry. One site: `TryCheekPouch`
  (`src/battle_script_commands.c`), gaining `|| IsInnateActive(battler, ABILITY_CHEEK_POUCH)`. It runs a
  heal **script** with a pop-up (`BattleScript_CheekPouchActivates` -> `BattleScript_AbilityPopUp`), so the
  innate overwrites the pop-up (`gBattleScripting.abilityPopupOverwrite = ABILITY_CHEEK_POUCH`) only when the
  chosen ability differs — the real-ability path stays byte-for-byte unchanged (the Speed Boost / Sturdy
  precedent).
- **Unburden** — doubles Speed once the holder's held item is consumed or lost. Two sites: the flag is
  armed in `CheckSetUnburden` (`src/battle_util.c`, gaining the innate clause) at every item-loss point, and
  the doubling itself sits in `GetBattlerTotalSpeedStat` (`src/battle_main.c`) beside the chosen read. Since
  the AI's turn-order prediction runs that same speed function keyed off the real battler, the AI threatens
  and respects an innate Unburden's boost for free.

Suppression parity holds for all four via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field);
none is `breakable`, so Mold Breaker never touches them, same as the real abilities. **Species:** the canon
users of each ability get their rows (Sceptile and its Mega mirror an innate Unburden even though the fork
override repurposed the selectable slot — a pure-boon persistence, like the Mega convention). Several species
whose real abilities are now **all** innate with no free complementary slot (Snorlax, Linoone, Hitmonlee,
Liepard, Thievul, Dedenne, Appletun) keep their now-redundant chosen frontier ability rather than a game-wide
override sweep — a deferred follow-up, mirroring Batch J. The frontier sets whose species can free the slot do
(Raticate-Alola -> Hustle, the Drifblim/Hawlucha/Sneasler Unburden sets -> a real complementary slot, and the
Simi trio / Victreebel / Greedent -> a new empty-slot override in `src/fork/species_ability_overrides.c`).

### ABILITY_ROUGH_SKIN / ABILITY_IRON_BARBS / ABILITY_GOOEY / ABILITY_TANGLING_HAIR

The first sub-group of the on-hit / on-contact set (Batch K), all **1:1 clean-upside copies** — a contact
reaction only ever hurts the *attacker*, never the holder. Rough Skin / Iron Barbs chip a contact attacker
1/8 max HP; Gooey / Tangling Hair lower a contact attacker's Speed by 1. All four are `canon-only` (no flavor
picks). They are the **first active, scripted ON-HIT innates**, and they introduce a **new on-hit driver**
that later Batch K sub-PRs reuse — the on-hit analogue of the Speed Boost end-turn driver.

- **The driver — `TryActivateInnateOnHitEffects(battler, *index, move)`** (`src/fork/innate_abilities.c`).
  Re-entrant, modeled byte-for-byte on `TryActivateInnateEndTurnEffects`: it scans the holder's innate list
  from a per-battler cursor and, for the first *active on-hit* innate (`IsActiveOnHitInnate`) that is active
  (`IsInnateActive`) and not the chosen ability, delegates to the **existing** upstream contact handler with
  the innate passed explicitly — `AbilityBattleEffects(ABILITYEFFECT_MOVE_END, battler, innate, move, TRUE)`.
  Reusing the upstream case means the recoil damage / stat drop / script / pop-up are identical to the real
  ability for free. `battler` is the holder that was hit (`gBattlerTarget`); the case bodies read
  `gBattlerAttacker` as the contact-maker.
- **The hook — a new `MOVEEND_ABILITIES_INNATE` step** (`include/constants/battle_move_resolution.h`) inserted
  right after `MOVEEND_ABILITIES` (the chosen-ability contact block), dispatched by `MoveEndAbilitiesInnate`
  (`src/battle_move_resolution.c`). Re-entrancy mirrors the end-turn hook: the handler **holds** the moveend
  state (returns `MOVEEND_RESULT_RUN_SCRIPT` without advancing) while the driver returns TRUE, keeping the
  cursor (`gBattleStruct->eventState.moveEndInnateIndex`); once the list is exhausted it resets the cursor and
  advances. `MoveEndSetValues` also zeroes the cursor per strike. So adding a second on-hit active is a one-line
  addition to `IsActiveOnHitInnate` — no driver change needed.
- **The pop-up.** The Rough Skin / Iron Barbs and Gooey / Tangling Hair effect sites (`src/battle_util.c`) set
  `gBattleScripting.abilityPopupOverwrite` to the innate **only when the chosen ability differs**, so a real
  ability stays byte-for-byte unchanged (the Speed Boost / Sturdy precedent).
- **Coexistence with Cute Charm / Stench.** Those two predate this driver and keep their own inline prechecks at
  the top of `ABILITYEFFECT_MOVE_END` / `ABILITYEFFECT_MOVE_END_ATTACKER`, so they are deliberately **not** listed
  in `IsActiveOnHitInnate` (listing them would fire them twice).
- **AI.** The contact-move-avoidance reads in `src/battle_ai_util.c` (`AI_IsMoveEffectInMinus` and
  `CompareMoveEffects`) credit an innate Iron Barbs / Rough Skin via `BattlerHasAbility`, so the AI still shies
  a contact move away from a mon whose chip ability is innate-only. Gooey / Tangling Hair have no dedicated AI
  read (the generic contact-move logic already covers the Speed drop).

Suppression parity holds for all four via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field);
none is `breakable`, so Mold Breaker never touches them. **Species:** the canon users of each ability get their
rows (merged into existing rows where the species already carried an innate) — Rough Skin (Carvanha / Sharpedo
+ Mega, the Gible line + both Garchomp Megas, Druddigon), Iron Barbs (Ferroseed / Ferrothorn, Togedemaru),
Gooey (the Goomy line + both Hisui forms, Wiglett / Wugtrio), Tangling Hair (Diglett-Alola / Dugtrio-Alola).
The Megas mirror the base creature's contact reaction (pure-boon persistence, like the Mega convention). Step
3.5 freed twelve frontier sets: Druddigon -> Sheer Force, Togedemaru -> Lightning Rod, Goodra -> Sap Sipper
(complementary REAL slots); Sharpedo -> Strong Jaw, Garchomp -> Sand Stream, Ferrothorn -> Filter, Wugtrio ->
Water Absorb (fork-owned overrides); Dugtrio-Alola's Tangling Hair set is kept because that slot is test-pinned.

### ABILITY_AFTERMATH / ABILITY_INNARDS_OUT

The second sub-group of Batch K (the on-hit set), both **1:1 clean-upside copies** — an on-faint retaliation
only ever hurts the *attacker*. **Aftermath** chips the attacker 1/4 max HP when a **contact** move KOs the
holder (Damp still blocks it); **Innards Out** deals the attacker the exact HP the holder lost, from **any**
move. Both `canon-only` (no flavor picks).

- **Reuses the existing on-hit driver** — no new infra. Both fire from the **same** upstream
  `ABILITYEFFECT_MOVE_END` case as the contact reactions, so each is a **one-line addition to
  `IsActiveOnHitInnate`** (`src/fork/innate_abilities.c`). The subtlety versus the contact reactions is
  *timing*: these fire **after the holder has fainted**. That works because the moveend loop still reaches the
  `MOVEEND_ABILITIES` / `MOVEEND_ABILITIES_INNATE` steps on a just-fainted target (vanilla Aftermath already
  fires there), and `IsInnateActive()` still credits the holder — a fainted-but-not-yet-switched battler has
  `notOnField` still FALSE during move-end, so the innate is active. Both effect sites (`src/battle_util.c`)
  use `BattleScript_AftermathDmg`.
- **The pop-up.** The Aftermath (the damage `else` branch, not the Damp branch) and Innards Out effect sites set
  `gBattleScripting.abilityPopupOverwrite` to the innate **only when the chosen ability differs**, so a real
  ability stays byte-for-byte unchanged (the Speed Boost / Rough Skin precedent).
- **AI.** Neither has a dedicated AI *effect* read (`grep src/battle_ai_*.c` is empty for both), so **no AI
  wiring is needed** — the AI does not currently reason about on-faint retaliation, chosen or innate.

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); neither is
`breakable`, so Mold Breaker never touches them. **Species:** every canon carrier gets a row (merged into
existing rows where the species already carried an innate) — Aftermath: the Voltorb line + both Hisui forms,
the Drifloon line, the Stunky line, the Trubbish line (+ Garbodor-Gmax); Innards Out: Pyukumuku and the fork's
Mega Victreebel. Step 3.5 freed five frontier sets, all via fork-owned overrides since every affected species
now has its useful real abilities innate: Drifblim ×2 (Aftermath/Unburden/Flare Boost all innate) -> chosen
Unaware (unpinned slot-0 repurpose), Skuntank (Aftermath/Stench/Keen Eye all innate) -> chosen Poison Touch,
Garbodor (Aftermath innate; slot-1 Weak Armor a wall drawback) -> chosen Poison Touch, Pyukumuku (Innards
Out/Unaware innate) -> chosen Water Absorb (empty slot 1; its slot-0 Innards Out stays a real, test-pinned
ability).

### ABILITY_STEAM_ENGINE / ABILITY_THERMAL_EXCHANGE / ABILITY_WIND_POWER

The third sub-group of Batch K (the on-hit set), all **1:1 clean-upside copies** — each only ever helps the
holder. **Steam Engine** raises Speed +6 when the holder is hit by a **Fire- or Water-type** move; **Thermal
Exchange** raises Attack +1 when hit by a **Fire-type** move *and* grants **burn immunity**; **Wind Power**
charges the holder's next Electric move (Charge volatile) when hit by a **wind** move *or* when **Tailwind**
takes effect on its side. All three `canon-only` (no flavor picks).

- **Reuses the existing on-hit driver** — no new infra for the on-hit boost/charge. All three fire from the
  **same** upstream `ABILITYEFFECT_MOVE_END` case as the contact reactions, so each is a **one-line addition to
  `IsActiveOnHitInnate`** (`src/fork/innate_abilities.c`), delegating to the upstream case so the stat change /
  charge / script / pop-up match the real ability. Effect sites are in `src/battle_util.c` (Steam Engine's
  `BattleScript_AbilityStatChange` Speed +6, Thermal Exchange's Attack +1, Wind Power's `BattleScript_WindPowerActivates`).
  These trigger at 100% on the right hit — **no RNG / `DETERMINISTIC_*` surface** (unlike Effect Spore's roll).
- **The pop-up.** Each effect site sets `gBattleScripting.abilityPopupOverwrite` to the innate **only when the
  chosen ability differs** (the Speed Boost / Rough Skin precedent), so a real ability stays byte-for-byte
  unchanged. Wind Power's shared `case` with Electromorphosis is guarded by `GetBattlerAbility != gLastUsedAbility`,
  which neither a chosen Electromorphosis nor a same-slot Wind Power ever trips.
- **Thermal Exchange's burn immunity** is a second, non-driver effect: an `IsInnateActive()` clause beside the
  chosen-ability test in `CanSetNonVolatileStatus` (`src/battle_util.c`, beside Water Veil/Immunity — records/shows
  the innate when the chosen ability differs), plus the matching **switch-in burn cure** folded into the Water
  Veil/Water Bubble block in `TryImmunityAbilityHealStatus`. The burn-immunity AI (`CanBeBurned` and its callers)
  is innate-aware for free through `CanSetNonVolatileStatus`. Thermal Exchange is `breakable`, so Mold Breaker
  pierces the innate exactly like the real ability (then the cure runs once Mold Breaker is no longer active).
- **Wind Power's Tailwind trigger** is a second effect site (`BS_TryWindRiderPower`, `src/battle_script_commands.c`):
  the `default` case credits an innate Wind Power via `IsInnateActive()` (placed in `default` so it can't
  double-fire beside the chosen Wind Power / Wind Rider cases), reusing `BattleScript_WindPowerActivates`.
- **AI.** On-hit boost/charge lives outside the shared damage calc, so the dedicated AI reads are made
  innate-aware: Wind Power's self-charge score (`GetWindAbilityScore`) and switch-in Tailwind sim
  (`SetBattlerVolatilesForSwitchin`, `src/battle_ai_switch.c`) each credit an innate via `IsInnateActive()`, and the
  doubles partner-fire heuristics for Steam Engine / Thermal Exchange are handled by extending the existing
  `scoringPartnerAbility` promotion in `src/battle_ai_main.c` (an ally's innate is promoted like the real ability
  so the AI values hitting an innate-only holder to trigger its boost, passing the promoted value into
  `ShouldTriggerAbility`). Thermal Exchange's burn-immunity AI is free (above).

Suppression parity holds via `IsInnateActive()` for all three (Gastro Acid / Neutralizing Gas / not-on-field);
only Thermal Exchange is `breakable`. **Species (canon-only):** Steam Engine -> the Rolycoly line (Rolycoly /
Carkol / Coalossal + Coalossal-Gmax), Thermal Exchange -> the Frigibax line (Frigibax / Arctibax / Baxcalibur +
the fork's Mega), Wind Power -> the Wattrel line (Wattrel / Kilowattrel) — each merged into the species' existing
innate row. Step 3.5 freed three frontier sets: Coalossal (Steam Engine now innate) -> its complementary real
slot-2 **Flash Fire**; both Baxcalibur sets (Thermal Exchange + Ice Body both innate = all real abilities innate)
-> a fork-owned override filling empty slot 1 with **Snow Warning**, whose snow turns on the innate Ice Body heal.
Wattrel/Kilowattrel have no frontier set to free.

### ABILITY_CURSED_BODY

The fourth sub-group of Batch K (the on-hit set), a **1:1 clean-upside copy** — it only ever hampers the FOE.
When the holder takes damage from a move, **Cursed Body** has a 30% chance (always, under `DETERMINISTIC_ABILITIES`
— the shipping default) to **disable the move the attacker just used**, exactly like the real ability.

- **Reuses the existing on-hit driver** — no new infra. It fires from the **same** upstream `ABILITYEFFECT_MOVE_END`
  case as the contact reactions, so it is a **one-line addition to `IsActiveOnHitInnate`**
  (`src/fork/innate_abilities.c`), delegating to the upstream case (`ABILITY_CURSED_BODY`, `src/battle_util.c`) so
  the disable / `BattleScript_CursedBodyActivates` script / pop-up match the real ability. Unlike the contact
  reactions it does **not** require contact — any damaging hit that satisfies `IsBattlerTurnDamaged` can trigger it
  (Aroma Veil on the attacker's side and Struggle are still exempt, from the upstream case).
- **The `DETERMINISTIC_ABILITIES` surface.** The upstream case already gates the roll as
  `GetConfig(DETERMINISTIC_ABILITIES) || RandomPercentage(RNG_CURSED_BODY, 30)` (a pre-existing `FORK:` change —
  Cursed Body always disables under the deterministic config), so the innate inherits the always-disable behavior
  for free; the shipping default (deterministic on) is exactly what the tests exercise.
- **The pop-up.** The effect site sets `gBattleScripting.abilityPopupOverwrite` to the innate (`gLastUsedAbility`)
  **only when the chosen ability differs** (the Speed Boost / Rough Skin precedent), so a real Cursed Body stays
  byte-for-byte unchanged. The driver skips an innate equal to the chosen ability, so a real Cursed Body never
  disables twice beside the chosen-ability block.
- **AI.** None needed — Cursed Body has **no** dedicated `battle_ai_*.c` read (grep is empty), so there is nothing
  to make innate-aware (the same as Aftermath / Innards Out).

Suppression parity holds via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); Cursed Body is not
`breakable`, so Mold Breaker never touches it, same as the real ability. **Species (canon-only):** every canon
Cursed Body user in any real slot — the **Shuppet / Banette** line (incl. **Mega Banette** as a pure-boon mirror),
**Froslass** (+ the fork's Mega), the **Frillish / Jellicent** line, the **Sinistea / Polteageist** forms, the
**Dreepy / Dragapult** line, **Corsola-Galar**, and merged onto **Marowak-Alola**'s existing Rock Head row.
**Gengar / Gengar-Gmax are omitted** as redundant: Cursed Body is their **sole** (therefore always-chosen) ability,
so an innate could never be observed (the Mega Lopunny / Scrappy precedent) — they keep their innate Levitate rows.
Step 3.5 freed two frontier sets to a complementary real slot with a stable `:x:` pick — **Jellicent** (Cursed Body
now innate) -> chosen **Water Absorb**, **Polteageist** -> its only other real slot **Weak Armor**. The
sole/all-abilities-innate sets keep their now-redundant chosen Cursed Body (still correct — the chosen runs it, the
innate is redundant-but-skipped): **Froslass x2** (Snow Cloak + Cursed Body both innate) and **Banette x1** (only
the still-pending Frisk left), deferred as a focused follow-up like Batch J/T. Gengar's four sets are untouched
(Gengar isn't in the innate table, so its chosen Cursed Body is its real, observed ability).

### ABILITY_PICKPOCKET / ABILITY_MAGICIAN / ABILITY_LIQUID_OOZE

The fifth and **final** sub-group of Batch K (the on-hit set), all **1:1 clean-upside copies** — each only ever
hurts the FOE. **Pickpocket** steals a contact attacker's held item when the holder has none; **Magician** steals a
held item off a target the holder *damaged* when the holder has none; **Liquid Ooze** makes an HP-draining move
(Absorb / Giga Drain / Leech Seed / Dream Eater) *damage* the attacker for the drained amount instead of healing it.
All three `canon-only` (no flavor picks). This sub-group completes Batch K, so **Batch K is done**.

- **Pickpocket — a one-line innate-aware swap, NOT the on-hit driver.** Unlike the other Batch K abilities, Pickpocket
  has its own dedicated move-end step (`MoveEndPickpocket`, `src/battle_move_resolution.c`) that already scans every
  battler reading the cached chosen ability; wiring the driver would double-fire. So the effect site just gains an
  `IsInnateActive(battlerDef, ABILITY_PICKPOCKET)` clause beside the `cv->abilities[battlerDef] == ABILITY_PICKPOCKET`
  read. `BattleScript_Pickpocket` shows an ability pop-up, so the site sets `gBattleScripting.abilityPopupOverwrite =
  ABILITY_PICKPOCKET` when the chosen ability differs (the Speed Boost precedent).
- **Magician — a NEW attacker-side on-hit driver.** Magician is *attacker*-side (it fires from the upstream
  `ABILITYEFFECT_MOVE_END_FOES_FAINTED` case, the same one that later serves Moxie/Beast Boost in Batch M), so the
  target-side on-hit driver doesn't reach it. The fork adds `TryActivateInnateOnHitAttackerEffects` ->
  `IsActiveOnHitAttackerInnate` (`src/fork/innate_abilities.c`), the attacker-side analogue of the target-side on-hit
  driver — re-entrant, delegating to `AbilityBattleEffects(ABILITYEFFECT_MOVE_END_FOES_FAINTED, battler, innate, move,
  TRUE)` so only the Magician case runs and the item steal / `BattleScript_MagicianActivates` / pop-up match the real
  ability. It is hooked from a new `MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE` step
  (`include/constants/battle_move_resolution.h`) inserted right after the chosen-ability foes-fainted block, dispatched
  by `MoveEndAbilityEffectFoesFaintedInnate` (`src/battle_move_resolution.c`). It **reuses the target-side driver's
  `moveEndInnateIndex` cursor**, which the earlier `MOVEEND_ABILITIES_INNATE` step (running earlier in the same move)
  always resets to 0 before this step is reached, so no new state field is needed. The Magician effect site
  (`src/battle_util.c`) sets `gBattleScripting.abilityPopupOverwrite` to the innate when the chosen ability differs.
  Adding a *further* attacker-side on-hit active is now a one-line addition to `IsActiveOnHitAttackerInnate`.
- **Liquid Ooze — a passive calc modifier (no driver), like Filter / Unaware.** The drain reversal lives in three
  drain sites: `SetHealScript` (`src/battle_move_resolution.c`, damaging drain moves), `SetUpLeechSeedDrain`
  (`src/battle_util.c`, the shared Leech Seed drain used by the end-turn tick and the re-seed re-drain), and the
  non-`BUFF_LEECH_SEED` Leech Seed tick (`src/battle_end_turn.c`). Each gains a `BattlerHasAbility` /
  `IsInnateActive` clause beside the cached chosen-ability read, and — because all three Liquid Ooze battle scripts
  (`BattleScript_EffectAbsorbLiquidOoze`, `BattleScript_LeechSeedTurnDrainLiquidOoze` and its re-drain twin) show an
  ability pop-up — sets `gBattleScripting.abilityPopupOverwrite = ABILITY_LIQUID_OOZE` when the chosen ability differs.
- **AI.** Only Liquid Ooze has dedicated AI *effect* reads (the item steals are not modeled by the AI, chosen or
  innate — `grep src/battle_ai_*.c` is empty for both). Liquid Ooze's drain-avoidance heuristics — `ShouldAbsorb` and
  `AI_IsMoveEffectInMinus` (Absorb / Dream Eater) in `src/battle_ai_util.c`, plus the three Leech-Seed scoring reads in
  `src/battle_ai_main.c` — each credit an innate via `IsInnateActive`, so the AI still avoids draining an innate-Liquid-
  Ooze foe.

Suppression parity holds for all three via `IsInnateActive()` (Gastro Acid / Neutralizing Gas / not-on-field); none is
`breakable`, so Mold Breaker never touches them. **Known limitation (Sticky Hold vs innate Pickpocket):** exactly as for
a *chosen* Pickpocket, an innate Sticky Hold on the attacker does not block an innate Pickpocket's on-contact steal — the
attacker-side Sticky-Hold precheck reads the cached chosen ability, the same cross-cutting `jumpifability` limitation
already documented for Sticky Hold. **Species (canon-only):** Pickpocket -> the **Seedot / Nuzleaf / Shiftry**,
**Sneasel (+ Hisui) / Weavile**, **Binacle / Barbaracle**, **Impidimp / Morgrem / Grimmsnarl (+ Gmax)**, **Shroodle**,
and **Tinkatink / Tinkatuff / Tinkaton** lines; Magician -> the **Fennekin / Braixen / Delphox** line (+ the fork's
**Mega Delphox** as a pure-boon mirror), **Klefki**, and both **Hoopa** forms; Liquid Ooze -> the **Tentacool /
Tentacruel** and **Gulpin / Swalot** lines — each merged into the species' existing innate row. **Step 3.5** touched
sixteen frontier sets, all **deferred** (like Batch J/T and the Cursed Body sub-group): every affected species now has
all its useful real abilities innate (or only the still-pending Frisk free), so they keep their now-redundant chosen
ability — still correct (the chosen runs it; the innate is redundant-but-skipped). **Tentacruel** / **Swalot** keep
chosen Liquid Ooze; the three **Weavile** and three **Grimmsnarl** sets keep chosen Pickpocket; the three **Delphox**,
two **Klefki** and three **Hoopa / Hoopa-Unbound** sets keep chosen Magician — each a real, roster-legal slot.

### ABILITY_INTIMIDATE

The first sub-group of **Batch L** and the **first active, scripted SWITCH-IN innate** — a **1:1 clean-upside copy**
(Intimidate only ever hurts the foe). On switch-in the holder lowers **every opposing battler's Attack by 1 stage**.

**Driver + hook (the new infrastructure).** Intimidate introduces the **switch-in driver**
`TryActivateInnateSwitchInEffects` (`src/fork/innate_abilities.c` -> `IsActiveSwitchInInnate`), the switch-in analogue
of the Speed Boost end-turn driver and the on-hit driver. It is **re-entrant** via a per-battler cursor
(`switchInInnateIndex` in `gBattleStruct->eventState`) and delegates to the **existing upstream**
`AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, battler, ABILITY_INTIMIDATE, …)` case, so the Attack drop / script /
pop-up — and **every downstream reaction** (the target's Clear Body / White Smoke / Hyper Cutter / Big Pecks stat-drop
protection, the Own Tempo / Inner Focus / Oblivious / Scrappy / Guard Dog Intimidate-immunity halves already made
innate-aware in Batches D-E/I/P/S, plus Defiant / Competitive / Rattled / Adrenaline Orb) — match the real ability for
free. It is hooked from the new `FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE` step
(`include/constants/battle_switch_in.h`, dispatched in `FirstEventBlockEvents`, `src/battle_switch_in.c`) right after the
chosen-ability switch-in block, **inside the `switchinevents` state machine that drives every normal switch-in** (battle
intro, pivot moves, post-faint replacement, forced switch). The re-entrant block holds
`FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE` (keeping the cursor) while the driver returns TRUE and resets the cursor +
advances once it returns FALSE; the outer per-battler loop also resets the cursor when it moves to the next battler.
**The `switchinabilities` sites (ability-swap / Tera / form change) are deliberately NOT hooked** — an innate Intimidate
is species-bound, so it must not re-fire when a foe Skill-Swaps or the holder Mega-evolves (the chosen path already
handles those for a gained *chosen* ability). Adding a further switch-in active (Download, Unnerve, …) is a **one-line**
addition to `IsActiveSwitchInInnate`.

**Pop-up / identity.** The switch-in pop-up reads the primary slot, so the effect site (the `ABILITY_INTIMIDATE`
`ABILITYEFFECT_ON_SWITCHIN` case in `src/battle_util.c`) forces `gBattleScripting.abilityPopupOverwrite = ABILITY_INTIMIDATE`
only when the chosen ability differs (the Speed Boost / Sturdy precedent); `BattleScript_AbilityPopUp` clears the overwrite
after showing it, and `recordability` still records the **chosen** ability, so identity stays deterministic. The driver
also pins `gBattlerAbility = battler` before delegating so the pop-up targets the holder.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field); the driver
also skips an innate equal to the chosen ability, so a mon whose chosen ability *is* Intimidate drops the foe **once, not
twice**. Intimidate is not `breakable`, so Mold Breaker never touches it.

**AI.** Intimidate's reasoning lives in **dedicated** `src/battle_ai_switch.c` helpers (not the shared calc), so it had to
be wired: the switch-in Attack-drop **simulation** (`SetBattlerStatStagesForSwitchin`, so the AI values switching an
innate-Intimidate mon in — off-field, keyed on `SpeciesHasInnate`, feature-gated), the **Intimidate-cycling** switch
heuristic (`ShouldSwitchIfAbilityBenefit` gains a pre-check mirroring Regenerator / Natural Cure, crediting an innate via
`BattlerHasAbility` even when the chosen ability differs, then routing through `ShouldSwitchIfIntimidateBenefit`), and the
foe-Intimidate **free-switch timing** read (`BattlerHasAbility(opposingBattler, …)`). The incoming-ability *value* scorer
in `battle_ai_util.c` (`case ABILITY_INTIMIDATE`) is left keyed to the chosen ability — a soft ability-swap-move heuristic,
not a hard effect read, consistent with how Unaware left its softest heuristics.

**Species (canon-only, no flavor picks** — Intimidate is a strong, common ability with a ~45-species canon set, so like
Prankster the flavor set is deliberately omitted). Every canon Intimidate user in **any** real slot gets a row (merged into
an existing innate row where present), keyed **exactly per form**: the Ekans / Growlithe (+ Hisui) / Arcanine (+ Hisui) /
Tauros (+ Paldea forms) / Gyarados / Snubbull / Granbull / Qwilfish (+ Hisui) / Overqwil / Stantler / Wyrdeer / Mightyena /
Masquerain / Mawile / Salamence / Staravia / Staraptor / Shinx / Luxio / Luxray / Herdier / Stoutland / Sandile / Krokorok /
Krookodile / Scraggy / Scrafty / Litten / Torracat / Incineroar / Squawkabilly (all four plumages) / Maschiff / Mabosstiff
/ Hitmontop lines, plus base creatures' **Megas as pure-boon mirrors** (Gyarados / Salamence / Mawile Megas keep base
Intimidate though their Mega ability differs). **Sole-Intimidate Megas are OMITTED as redundant** (Manectric-Mega,
Scrafty-Mega — their sole, always-chosen ability IS Intimidate, so an innate could never be observed, the Mega Lopunny /
Scrappy precedent). **Landorus-Therian** is sole-Intimidate but a **frontier set**, so — like Ogerpon-Cornerstone — it takes
the innate **and** a fork-owned chosen **Sheer Force** override (`src/fork/species_ability_overrides.c`; Sheer Force is `:x:`
(never an innate -> stable) and its Incarnate forme's signature), and its two frontier sets now run Sheer Force on top of the
innate Intimidate. **Step 3.5** touched ~40 frontier sets: Landorus-Therian is freed via the override above; the rest are
**deferred** (like Batch J/T/K) — they keep their now-redundant chosen Intimidate (still correct: the chosen runs it, the
innate is redundant-but-skipped) rather than a game-wide complementary-slot sweep.

### ABILITY_ANTICIPATION / ABILITY_FOREWARN / ABILITY_FRISK

**Batch L's second sub-group** — three **switch-in information reveals**, each a **1:1 clean-upside copy**
(pure information; none ever hurts the holder). On switch-in **Anticipation** shows a warning message if any
foe knows a super-effective or OHKO move, **Forewarn** reveals one of a foe's strongest moves, and **Frisk**
reveals the foes' held items.

**Driver + hook (reused).** All three ride the **existing switch-in driver** `TryActivateInnateSwitchInEffects`
(`src/fork/innate_abilities.c` -> `IsActiveSwitchInInnate`) that Intimidate introduced — adding each was a
**one-line** `IsActiveSwitchInInnate` case, no driver/hook change. The driver delegates to the upstream
`AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, battler, <innate>, …)` case, so the message / reveal / script /
pop-up match the real ability for free.

**Pop-up / identity.** Each switch-in message script (`BattleScript_SwitchInAbilityMsg` for Anticipation /
Forewarn, `BattleScript_FriskActivates` for Frisk) calls `BattleScript_AbilityPopUp`, which reads the primary
slot — so each effect site (the three `ABILITYEFFECT_ON_SWITCHIN` cases in `src/battle_util.c`) forces
`gBattleScripting.abilityPopupOverwrite = gLastUsedAbility` (the innate being processed) only when the chosen
ability differs (the Speed Boost / Rain Dish precedent), and `BattleScript_AbilityPopUp` clears it after
showing it. `recordability` still records the **chosen** ability, so identity stays deterministic.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field);
none is `breakable`, so Mold Breaker never touches them.

**AI.** None of the three has a **dedicated** `battle_ai_*.c` effect read (`grep` confirms zero), so **no AI
wiring is needed**: they change no stat or state the switch-in sim reasons about, and the AI already benefits
from the revealed move/item records through the shared move/item bookkeeping keyed off the real battler.

**Species (canon-only, no flavor picks** — a switch-in reveal is pure information with no thematic hook
off-roster). Every canon user in **any** real slot gets a row (merged into an existing innate row where
present), keyed **exactly per form**: **Frisk** -> the Gothita / Shuppet (+ Banette, incl. **Mega Banette as a
pure-boon mirror**) / Duskull / Flittle / Espathra / Munkidori / Wigglytuff / Exeggutor-Alola / Typhlosion-Hisui
/ Sentret / Yanma / Stantler / Wyrdeer / Phantump / Pumpkaboo (all sizes) / Gourgeist (all sizes) / Noibat /
Orbeetle (+ Gmax) / Impidimp lines; **Forewarn** -> the Munna / Drowzee / Smoochum lines; **Anticipation** ->
Ferrothorn / the Barboach / Flittle / Ponyta-Galar / Eevee-Starter / Wormadam (all cloaks) / Croagunk /
Hatenna (+ Gmax) lines. **Flittle carries both Frisk and Anticipation**, matching its real ability data. No
species is sole-ability for any of the three, so there are no redundant omissions or override rows. **Step 3.5**:
the ~14 frontier sets that hardcoded chosen Frisk / Anticipation now carry it innately, so they keep their
now-redundant chosen ability (still correct: the chosen runs it, the innate is redundant-but-skipped) — the
complementary-slot freeing is **deferred** like Batch J/T/K and the Intimidate sub-group.

### ABILITY_DOWNLOAD / ABILITY_SUPERSWEET_SYRUP

**Batch L's third sub-group** — two **switch-in stat-change** innates, each a **1:1 clean-upside copy** (a
self-boost / foe-debuff that only ever helps the holder, so no pure-boon divergence). On switch-in **Download**
compares each foe's Defense vs Sp. Def and raises the holder's **Attack or Sp. Atk** (whichever hits the weaker
defense, via `GetDownloadStat`) by 1 stage; **Supersweet Syrup** lowers **every opposing battler's evasiveness**
by 1 stage, **once per battle** (tracked per party mon in `GetBattlerPartyState(battler)->supersweetSyrup`, so it
survives switch-out and never re-fires).

**Driver + hook (reused).** Both ride the **existing switch-in driver** `TryActivateInnateSwitchInEffects`
(`src/fork/innate_abilities.c` -> `IsActiveSwitchInInnate`) that Intimidate introduced — adding each was a
**one-line** `IsActiveSwitchInInnate` case, no driver/hook change. The driver delegates to the upstream
`AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, battler, <innate>, …)` case, so the stat change / script /
pop-up (Download via `BattleScript_AbilityStatChange`, Supersweet Syrup via `BattleScript_SupersweetSyrupActivates`)
match the real ability for free.

**Pop-up / identity.** Both scripts show the ability pop-up (which reads the primary slot), so each effect site
(the two `ABILITYEFFECT_ON_SWITCHIN` cases in `src/battle_util.c`) forces
`gBattleScripting.abilityPopupOverwrite = gLastUsedAbility` (the innate being processed) only when the chosen
ability differs (the Speed Boost / Intimidate precedent). `recordability` still records the **chosen** ability,
so identity stays deterministic.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field);
neither is `breakable`, so Mold Breaker never touches them.

**AI.** Unlike the information-reveal trio, both change a stat the AI's switch-in simulation reasons about, so
each has a **dedicated** effect read that had to be made innate-aware. `SetBattlerStatStagesForSwitchin`
(`src/battle_ai_switch.c`) handles both keyed off `aiAbility` (the chosen ability); the fork adds a mirror block
after the existing Intimidate mirror that applies the same swing when the mon carries the ability **innately**
(`SpeciesHasInnate(species, X)`, gated on `FEATURE_INNATE_ABILITIES` so feature-off never scans innates) — Download
boosts the holder's own `GetDownloadStat` stat, Supersweet Syrup drops the foe's evasion (respecting the target's
Contrary / Defiant / Competitive exactly like the real case). The Eject-Pack free-switch **timing** read (a foe
whose Intimidate / Supersweet Syrup triggers switches before the turn starts) also now credits an innate
Supersweet Syrup foe via `BattlerHasAbility`, mirroring the Intimidate treatment on the same line.

**Species (canon-only, no flavor picks** — a switch-in stat swing is potent, so like Intimidate the set stays the
canon users). **Download** -> the Porygon / Porygon2 / Porygon-Z line (merged onto their existing Analytic /
Levitate rows) and **every Genesect form** (base + Douse / Shock / Burn / Chill drives, keyed exactly per form).
**Supersweet Syrup** -> the Dipplin / Hydrapple line (merged onto their existing Sticky Hold rows). No omissions:
Genesect is sole-Download but **is** a frontier set, so it takes the innate (see Step 3.5) rather than being
dropped as redundant. **Step 3.5**: the Porygon2 Download frontier set is freed to its complementary REAL slot-0
**Trace** (`:x:` stable — copies a foe ability); the all-real-abilities-innate Porygon-Z sets and the sole-ability
Genesect sets keep their now-redundant chosen Download (still correct: the chosen runs it, the innate is
redundant-but-skipped) — **deferred** like Batch J/T/K and the Intimidate sub-group. Supersweet Syrup has no other
frontier set to free (Dipplin is off-roster; Hydrapple already runs a fork-owned chosen Grassy Surge override).

### ABILITY_UNNERVE / ABILITY_HOSPITALITY

**Batch L's fourth/final sub-group** — two **switch-in effects**, each a **1:1 clean-upside copy** (foe Berry
denial / ally heal, never a downside). On switch-in **Unnerve** denies every opposing battler its Berries (shows
the "too nervous to eat Berries" message) and **Hospitality** restores **1/4 of the ally's max HP** in a double
battle.

**Driver + hook — the generalization these two forced.** Every earlier Batch L member runs through the upstream
`ABILITYEFFECT_ON_SWITCHIN` case, so the driver `TryActivateInnateSwitchInEffects` delegated to *that* case only.
Unnerve and Hospitality do **not**: their effects live in the separate upstream cases `ABILITYEFFECT_UNNERVE` and
`ABILITYEFFECT_DEPENDS_ON_ALLY`, which upstream dispatches at **different points of the switch-in sequence** (the
`SWITCH_IN_EVENTS_UNNERVE` event and the `SECOND_EVENT_ABILITIES` step, respectively — not the
`FIRST_EVENT_BLOCK_GENERAL_ABILITIES` block the on-switch-in innates hook). So the driver gained an
`abilityEffect` parameter selecting which switch-in phase a call handles, and a new
`SwitchInInnateAbilityEffect(ability)` maps each switch-in innate to the `ABILITYEFFECT_*` that runs it
(`src/fork/innate_abilities.c`). Each of the **three** phases now hooks the driver right after its chosen-ability
counterpart, passing that phase's effect: the existing `FIRST_EVENT_BLOCK_GENERAL_ABILITIES_INNATE`
(`ABILITYEFFECT_ON_SWITCHIN`), a new **`SWITCH_IN_EVENTS_UNNERVE_INNATE`** top-level event
(`ABILITYEFFECT_UNNERVE`, mirroring the chosen-ability Unnerve pass), and a new **`SECOND_EVENT_ABILITIES_INNATE`**
step (`ABILITYEFFECT_DEPENDS_ON_ALLY`) — both added to `include/constants/battle_switch_in.h` and dispatched in
`src/battle_switch_in.c`. Each battler carries at most one Unnerve / Hospitality innate, so the two new hooks fire
once per battler and always advance (like their upstream counterparts) rather than needing the re-entrant cursor
the on-switch-in block uses for multi-innate mons. The driver still skips an innate equal to the chosen ability
(so a chosen-Unnerve mon denies Berries once, not twice) and honors `IsInnateActive()` suppression.

**Functional site vs. the switch-in message (Unnerve).** Unnerve's Berry block is **not** driven by the switch-in
message — the message (and the `unnerveActivated` volatile it sets) is cosmetic. The actual gate is the passive
`IsUnnerveAbilityOnOpposingSide` (`src/battle_util.c`, read from `IsUnnerveBlocked` at every Berry-eat site), which
scanned only `GetBattlerAbility`. It is made **innate-aware** with a `BattlerHasAbility(battlerDef, ABILITY_UNNERVE)`
check, so an innate Unnerve denies the opposing side its Berries exactly like the real ability; the switch-in driver
adds only the matching message + pop-up for parity. Hospitality's heal, by contrast, lives entirely in its
`ABILITYEFFECT_DEPENDS_ON_ALLY` case, so the driver alone carries it.

**Pop-up / identity.** Both switch-in scripts (`BattleScript_SwitchInAbilityMsg` for Unnerve,
`BattleScript_HospitalityActivates` for Hospitality) call `BattleScript_AbilityPopUp`, which reads the primary slot,
so each effect site (`src/battle_util.c`) forces `gBattleScripting.abilityPopupOverwrite = gLastUsedAbility` (the
innate being processed) only when the chosen ability differs (the Speed Boost / Intimidate precedent); the driver
pins `gBattlerAbility = battler` so the pop-up targets the holder. `recordability` still records the **chosen**
ability, so identity stays deterministic.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field);
neither is `breakable`, so Mold Breaker never touches them.

**AI.** Unnerve has one **dedicated** effect read — `GetSwitchinSingleUseItemHealing` (`src/battle_ai_switch.c`),
which discounts a Berry the AI would rely on when the opposing battler has Unnerve — made innate-aware with
`IsInnateActive(opposingBattler, ABILITY_UNNERVE)` beside the chosen-ability read (the passive
`IsUnnerveAbilityOnOpposingSide` block already covers the AI's shared damage/berry calcs for free). Hospitality
changes no stat or state the AI's switch-in simulation reasons about (`SetBattlerStatStagesForSwitchin` handles
stat stages, not the ally-heal), so it needs no AI wiring.

**Species (canon-only, no flavor picks).** **Unnerve** → every canon user in **any** real slot, keyed exactly per
form (merged into an existing innate row where present): the Ekans / Meowth (+ Galar / Gmax) / Persian / Aerodactyl
(+ Mega) / Mewtwo (+ Mega X/Y) / Ursaring / Ursaluna / Houndour / Houndoom (+ Mega) / Tyranitar (+ Mega) / Masquerain
/ Vespiquen / Joltik / Galvantula / Axew / Fraxure / Haxorus / Litleo / Pyroar / Bewear / Rookidee / Corvisquire /
Corviknight (+ Gmax) lines, plus base creatures' **Megas as pure-boon mirrors** (Aerodactyl / Mewtwo X-Y / Houndoom /
Tyranitar Megas keep base Unnerve though their Mega ability differs). **Sole-Unnerve Calyrex is OMITTED as
redundant** (not a frontier set; its sole chosen ability already grants it — the Mega Lopunny / Scrappy precedent).
**Hospitality** → the Poltchageist / Sinistcha line (all four forms — Counterfeit / Artisan / Unremarkable /
Masterpiece — merged onto their existing Heatproof / Levitate rows). **Step 3.5**: Sinistcha's Hospitality frontier
set is freed to its complementary chosen **Flash Fire** override (the sibling Sinistcha set already runs it — both
its real abilities, Hospitality and Heatproof, are now innate); the ~14 frontier sets that hardcoded chosen Unnerve
now carry it innately, so they keep their now-redundant chosen Unnerve (still correct: the chosen runs it, the
innate is redundant-but-skipped) — the complementary-slot freeing **deferred** like Batch J/T/K and the earlier
Batch L sub-groups.

### ABILITY_DEFIANT / ABILITY_COMPETITIVE

**Batch M's first sub-group** — two **stat-drop reactions**, each a **1:1 clean-upside copy** (they react to a
*foe's* debuff, so they only ever help the holder). When a foe lowers one of the holder's stats — a stat-lowering
move, Intimidate, or an opposing Sticky Web — **Defiant** raises the holder's **Attack** by 2 stages and
**Competitive** its **Sp. Atk** by 2 stages.

**Single scripted reaction site.** Both fire from the native command **`BS_TryDefiantRattled`**
(`src/battle_script_commands.c`), which the shared stat-drop message script (`BattleScript_DecreaseStatChangeMessage`,
`data/battle_scripts_1.s`) runs after *any* stat drop. It read only `GetBattlerAbility(battler)`, so when the chosen
ability isn't itself reactive (not Defiant / Competitive / Rattled) it now credits an innate Defiant / Competitive
via `IsInnateActive()` and overwrites the pop-up to it (`gBattleScripting.abilityPopupOverwrite` — `CreateAbilityPopUp`
reads the primary slot). The real-ability path (chosen Defiant / Competitive / Rattled) is **byte-for-byte unchanged**.
Because the reaction funnels through the shared stat-drop message, an innate Defiant / Competitive reacts to
**Intimidate** (chosen *or* innate) and **Sticky Web** for free — no extra wiring. The activation gate
`ShouldDefiantCompetitiveActivate` (the `MAX_STAT_STAGE` / Gen-9 Sticky-Web check) is reused unchanged, passed the
innate ability. The negative-stat-change **animation** suppression in `TryPlayStatChangeAnimation`
(`src/battle_stat_change.c`) is also made innate-aware, so the holder doesn't flash a down-arrow before the innate
re-raises the stat, matching the real ability.

**No driver.** Unlike the switch-in / on-hit actives, Defiant / Competitive don't run through
`AbilityBattleEffects` — the reaction has always been a dedicated native command, so the innate hooks that command
directly rather than the innate drivers.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field);
neither is `breakable`, so Mold Breaker never touches them.

**AI.** Two **dedicated** effect reads (the reaction isn't in the shared damage calc, so they aren't automatic):
`IncreaseStatDownScore` (`src/battle_ai_util.c`) — the "don't bother lowering a foe's stat if it will just bounce
back" check, which read `DoesAbilityRaiseStatsWhenLowered(chosen)` — and `ShouldSwitchIfIntimidateBenefit`
(`src/battle_ai_switch.c`) — the "don't switch an Intimidator into a foe that *wants* to be Intimidated" check,
which read `DoesIntimidateRaiseStats(chosen)`. Both now also credit an innate Defiant / Competitive foe via
`IsInnateActive()`. The soft **incoming-ability value scorer** (`battle_ai_util.c`, `DoesIntimidateRaiseStats` from
the ability-swap-move value path) is **left keyed to the chosen ability**, mirroring the Intimidate batch's decision
(a soft heuristic about *acquiring* an ability, not a hard on-field effect read). The doubles partner-fire scoring
(`ShouldTriggerAbility` / the `scoringPartnerAbility` switch in `battle_ai_main.c`) has **no** Defiant / Competitive
case, so there's nothing to make innate-aware there.

**Species (canon-only, no flavor picks — a +2 swing is potent and hard to justify thematically off-roster).**
**Defiant** → every canon user in **any** real slot, keyed exactly per form (merged into an existing innate row where
present): the Mankey / Primeape / Annihilape, Farfetchd (Kantonian), Pawniard / Bisharp / Kingambit, Braviary
(Kantonian), Tornadus-Incarnate, Thundurus-Incarnate, Purugly, Passimian, Obstagoon, and Falinks (base) lines.
**Competitive** → the Jigglypuff line (Igglybuff / Jigglypuff / Wigglytuff), Milotic, the Piplup / Prinplup / Empoleon
line (Competitive is their Hidden Ability under the shipping `P_UPDATED_ABILITIES >= GEN_9`; the pre-Gen-9 `#else`
gives them Defiant, which is compiled out, so they're Competitive-only here), Gothita / Gothorita / Gothitelle,
Meowstic-F, Wattrel / Kilowattrel, and Boltund. **Sole-ability species are OMITTED as redundant** (their sole chosen
ability already grants it, so an innate could never be observed — the Mega Lopunny / Scrappy precedent): **Zapdos-Galar**
(sole Defiant), **Articuno-Galar** (sole Competitive), **Ogerpon / Ogerpon-Teal** (sole Defiant), and **Falinks-Mega**
(sole Defiant — its existing `BATTLE_ARMOR` innate row stays, since *that* is observable while its chosen ability is
Defiant). **Innate Rattled** — which also reacts through `BS_TryDefiantRattled`, but only to Intimidate (Speed +1) — is
a **separate Batch M sub-group**, deliberately not credited here. **Step 3.5**: the ~30 frontier sets that hardcoded
chosen Defiant / Competitive already resolve to the species' real slot (these are canon users), so they keep their
now-redundant chosen ability (still correct: the chosen runs it, the innate is redundant-but-skipped) — the
complementary-slot freeing **deferred** as a focused follow-up, like Batch J/T/K and the Batch L sub-groups.

### ABILITY_JUSTIFIED / ABILITY_STAMINA / ABILITY_WATER_COMPACTION / ABILITY_ANGER_POINT

**Batch M's second sub-group** — four **on-hit stat reactions**, each a **1:1 clean-upside copy** (they react to
*being hit*, so they only ever help the holder). When the holder is damaged by a move: **Justified** raises its
**Attack** by 1 stage if the move is **Dark**-type; **Stamina** raises its **Defense** by 1 stage on **any** move;
**Water Compaction** raises its **Defense** by 2 stages if the move is **Water**-type; **Anger Point** maxes its
**Attack** (to +6 / `MAX_STAT_STAGE`) when the holder takes a **critical hit**.

**Reuses the existing on-hit driver — no new infra.** All four fire from the **same** upstream
`ABILITYEFFECT_MOVE_END` case (`src/battle_util.c`) as the Batch K contact reactions, so each is a **one-line
addition to `IsActiveOnHitInnate`** (`src/fork/innate_abilities.c`). The driver delegates to that case with the innate
passed explicitly, so the stat change / `BattleScript_AbilityStatChange` script / pop-up match the real ability for
free. Here `battler` (the delegated case's parameter) is `gBattlerTarget`, the holder that was hit.

**The pop-up.** Each of the four effect sites (`src/battle_util.c`) sets `gBattleScripting.abilityPopupOverwrite` to
the innate (`gLastUsedAbility`) **only when the chosen ability differs**, so a real ability stays byte-for-byte
unchanged (the Speed Boost / Rough Skin precedent). The driver skips an innate equal to the chosen ability, so a
real Justified / Stamina / … never fires twice beside the chosen-ability block.

**No `DETERMINISTIC_*` surface.** All four trigger at 100% on the right hit (Anger Point on a guaranteed/rolled
crit — the crit itself is the RNG, not the ability), so there is nothing to gate under `DETERMINISTIC_ABILITIES`,
unlike Cursed Body's disable roll.

**AI.** The reactions live outside the shared damage calc, so the dedicated AI *effect* reads are made
innate-aware:
- **`AI_CheckBadMove`** (`src/battle_ai_main.c`) — the "don't feed the on-hit boost" penalty. Its `switch(abilityDef)`
  reads the chosen ability, so a pre-switch clause credits an **innate Justified** foe (a Dark damaging move boosts
  it) via `IsInnateActive`. Stamina / Water Compaction / Anger Point have **no** such avoid-read — their trigger
  isn't a move-type the AI can dodge (any move / a Water hit it may still want / a crit it doesn't choose), matching
  upstream, which only lists Justified (and Rattled) here.
- **Doubles partner-fire scoring** (`src/battle_ai_main.c`) — the `scoringPartnerAbility` promotion block (the Steam
  Engine precedent) now also promotes an **innate Justified / Water Compaction / Anger Point** on the ally (keyed on
  a Dark / Water / always-crit move) so the AI values hitting an innate-only holder to trigger its boost; the
  Justified and Water Compaction `case`s were switched to read the promoted `scoringPartnerAbility` in their
  `ShouldTriggerAbility` calls (Anger Point's case needs no such call).
- **Self always-crit + Beat Up** — the "my partner's always-crit move will max my Attack" read (`aiData->abilities[battlerAtk]
  == ABILITY_ANGER_POINT`, `src/battle_ai_main.c`) and `ShouldBeatUpForJustified` (`src/battle_ai_util.c`) both credit
  an innate holder via `IsInnateActive`.

**Suppression parity** holds for all four via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas,
not-on-field); none is `breakable`, so Mold Breaker never touches them.

**Species (canon-only, no flavor picks — a reactive stat boost is potent and hard to justify thematically
off-roster).** Every canon user in **any** real slot, keyed exactly per form (merged into an existing innate row
where present), plus base creatures' **Megas** as pure-boon mirrors:
- **Justified** → the **Growlithe / Arcanine** (Kantonian; Hisui carries Rock Head, not Justified), **Absol** (+ Mega),
  **Gallade** (+ Mega), and **Lucario** (+ Mega, + the fork's **Mega-Z** which carries Justified in its own ability
  data) lines.
- **Stamina** → the **Mudbray / Mudsdale** line and **Archaludon**.
- **Water Compaction** → the **Sandygast / Palossand** line.
- **Anger Point** → the **Mankey / Primeape**, **Tauros** (+ all three Paldea forms — Combat / Blaze / Aqua),
  **Camerupt** (+ Mega), **Sandile / Krokorok / Krookodile**, and **Crabrawler / Crabominable** lines.

**Sole-ability species are OMITTED as redundant** (their sole chosen ability already grants it, so an innate could
never be observed — the Mega Lopunny / Scrappy precedent, matching Batch M's Defiant / Competitive sub-group): the
**Swords of Justice** trio (**Cobalion / Terrakion / Virizion**) and **Keldeo** (both formes), all sole-Justified.
They *are* frontier sets, but their `.ability = ABILITY_JUSTIFIED` is their real (and only) slot, so it keeps working
untouched; the innate + a fork-owned chosen override (the Landorus-Therian route) is **deferred** as a focused
follow-up.

**Step 3.5**: the frontier sets that hardcoded these abilities — chosen Justified (Absol, Gallade, Lucario),
Stamina (Mudsdale, Archaludon), Water Compaction (Palossand), Anger Point (Crabominable) — already resolve to the
species' real slot, so they keep their now-redundant chosen ability (still correct: the chosen runs it, the innate
is redundant-but-skipped, and for the several sets whose *other* real abilities are also now innate the set simply
runs all of them). A few of these slots were freed in an **earlier** batch to what was then a pending ability
(Gallade's Sharpness sweep → chosen Justified; Crabominable's Hyper Cutter / Iron Fist sweep → chosen Anger Point);
now that those become innate the chosen pick is redundant, but harmless — the complementary-slot re-pointing is
**deferred** as a focused follow-up, like the Defiant / Competitive sub-group and Batch J/T/K/L.

### ABILITY_RATTLED / ABILITY_STEADFAST

**Batch M's third sub-group** — the **fear-response Speed pair**, both **1:1 clean-upside copies** (they react to
being frightened, so only ever help the holder). When frightened, the holder's **Speed** rises by **1 stage**.
**Rattled** reacts to **two** triggers; **Steadfast** to one.

**Rattled spans the two Batch M sites already opened — no new infra:**
- **Hit by a Dark / Ghost / Bug move** → fired from the upstream `ABILITYEFFECT_MOVE_END` case (`src/battle_util.c`),
  so it is a **one-line addition to `IsActiveOnHitInnate`** (`src/fork/innate_abilities.c`), exactly like Justified.
  The driver delegates to that case with the innate passed explicitly, so the stat change /
  `BattleScript_AbilityStatChange` / pop-up match the real ability for free.
- **A foe's Intimidate** → fired from the shared native command **`BS_TryDefiantRattled`**
  (`src/battle_script_commands.c`), the same site as Defiant / Competitive. An innate Rattled is credited in the
  same "chosen ability isn't reactive" block (after Defiant / Competitive), so its switch `case` runs. That case is
  gated on `gBattleStruct->intimidateActivated` and `B_UPDATED_INTIMIDATE >= GEN_8`, so — **unlike** Defiant /
  Competitive, which react to *any* foe-caused stat drop (a move, Intimidate, or Sticky Web) — innate Rattled reacts
  **only to Intimidate**, matching the real ability.

**Steadfast** reacts to **flinching**, at the `CancelerFlinch` site (`src/battle_move_resolution.c`): the
chosen-ability test `cv->abilities[battlerAtk] == ABILITY_STEADFAST` gains `|| IsInnateActive(...)`, and the
`BattleScript_MoveUsedFlinchedAndSteadfast` path already handles the Speed raise + pop-up.

**The pop-up.** Each effect site sets `gBattleScripting.abilityPopupOverwrite` to the innate **only when the chosen
ability differs** (the Speed Boost precedent), so a real Rattled / Steadfast stays byte-for-byte unchanged. (Rattled's
`ABILITYEFFECT_MOVE_END` case previously lacked this overwrite — it was added, like Justified's.) The driver / credit
skips an innate equal to the chosen ability, so a real ability never fires twice.

**No `DETERMINISTIC_*` surface.** Both trigger at 100% on the right event (a Dark/Ghost/Bug hit, an Intimidate, a
flinch), so there is nothing to gate under `DETERMINISTIC_ABILITIES`.

**AI.** Rattled has dedicated *effect* reads (Steadfast has none — upstream's AI does not avoid flinching a Steadfast
holder, so no wiring is needed):
- **`AI_CheckBadMove`** (`src/battle_ai_main.c`) — a pre-`switch` clause credits an **innate Rattled** foe (a
  Dark/Ghost/Bug damaging move boosts its Speed), mirroring the innate-Justified clause beside it.
- **Doubles partner-fire scoring** (`src/battle_ai_main.c`) — the `scoringPartnerAbility` promotion block now also
  promotes an **innate Rattled** on the ally (keyed on a Dark/Ghost/Bug move), and the Rattled `case` was switched to
  read the promoted `scoringPartnerAbility` in its `ShouldTriggerAbility` call (the Justified / Steam Engine
  precedent).
- **Intimidate-cycling switch** (`ShouldSwitchIfIntimidateBenefit`, `src/battle_ai_switch.c`) — a foe's innate Rattled
  turns our Intimidate into a +1 Speed for it (Gen8+), so the AI won't switch its Intimidator out to re-fire it —
  added beside the innate Defiant / Competitive checks, gated on `B_UPDATED_INTIMIDATE >= GEN_8` to mirror how
  upstream's `DoesIntimidateRaiseStats` already flags a *chosen* Rattled.

The soft incoming-ability value scorer (`src/battle_ai_util.c`, the `ABILITY_INTIMIDATE` value case) is left keyed to
the chosen ability, mirroring the Defiant / Competitive decision.

**Suppression parity** holds via `IsInnateActive()` (feature flag, Gastro Acid, Neutralizing Gas, not-on-field);
neither is `breakable`, so Mold Breaker never touches them.

**Species (canon-only, no flavor picks).** Every canon user in **any** real slot, keyed exactly per form (merged into
an existing innate row where present):
- **Rattled** → the **Meowth-Alola / Persian-Alola**, **Magikarp**, **Ledyba**, **Bonsly / Sudowoodo**, **Whismur**,
  **Snubbull / Granbull**, **Poochyena**, **Dunsparce / Dudunsparce** (both segment forms), **Clamperl**,
  **Basculin-White-Striped**, **Cubchoo**, **Yamper**, **Toxel**, and **Wiglett / Wugtrio** lines.
- **Steadfast** → the **Machop / Machoke / Machamp** (+ Gmax), **Farfetch'd-Galar / Sirfetch'd**, **Tyrogue /
  Hitmontop**, **Scyther**, **Gallade** (+ Mega, as a pure-boon mirror), **Rockruff / Lycanroc-Midday**, and
  **Dubwool** lines.

**Sole-ability species are OMITTED as redundant** (their sole chosen ability already grants it — the Mega Lopunny /
Scrappy precedent): **Gimmighoul-Chest** (sole Rattled) and **Mega Mewtwo X** (sole Steadfast — its Pressure innate
row is kept). Where a line splits, only the members that *actually* carry the ability get a row (e.g. Magikarp but
not Gyarados, Cubchoo but not Beartic, Scyther but not Scizor/Kleavor, Lycanroc-Midday but not Midnight/Dusk).

**Deliberate contradiction omission (Steadfast vs innate Inner Focus).** The **Riolu / Lucario** line (incl. Mega /
Mega-Z) carries innate **Inner Focus**, which prevents flinching outright — so an innate Steadfast could **never**
trigger on them (the same class of conflict as Spinda's Tangled-Feet-vs-Own-Tempo note). Inner Focus (never flinch)
is the stronger, already-wired boon, so **Steadfast is dropped** on that line.

**Step 3.5**: the frontier sets that hardcoded chosen Rattled (Persian-Alola, Dunsparce / Dudunsparce) and Steadfast
(a Machamp-family set, Lycanroc-Midday) already resolve to the species' real slot, so they keep their now-redundant
chosen ability — still correct (the chosen runs it; the innate is redundant-but-skipped) — with the complementary-slot
re-pointing **deferred** as a focused follow-up, like the earlier Batch M sub-groups and Batch J/T/K/L.

### ABILITY_MOXIE / ABILITY_BERSERK / ABILITY_SOUL_HEART

**Batch M's fourth and final sub-group** — three **KO / on-damage / on-faint stat boosts**, each a **1:1 clean-upside
copy** (they react to a KO, a big hit, or a faint, so they only ever help the holder). **Moxie** raises the holder's
**Attack** by 1 stage for each foe it knocks out with a move; **Berserk** raises its **Sp. Atk** by 1 stage when an
attack drops its HP from above 1/2 to 1/2 or less; **Soul-Heart** raises its **Sp. Atk** by 1 stage every time **any**
Pokémon faints. This sub-group completes Batch M, so **Batch M is done**.

**Three distinct scripted sites — two reuse existing infra, one adds a small driver:**
- **Moxie** fires from the upstream **`ABILITYEFFECT_MOVE_END_FOES_FAINTED`** case (`src/battle_util.c`, the
  Moxie / Beast Boost / Chilling Neigh cluster), the exact case the **attacker-side** on-hit driver
  (`TryActivateInnateOnHitAttackerEffects`, hooked from `MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE`) already
  delegates to for Magician — so it is a **one-line addition to `IsActiveOnHitAttackerInnate`**
  (`src/fork/innate_abilities.c`). The driver delegates with the innate passed explicitly, so the stat change /
  `BattleScript_AbilityStatChange` / pop-up match the real ability for free. `NumFaintedBattlersByAttacker(battler)`
  counts the foes the holder KO'd this move, so a double KO still boosts twice, exactly like the real ability.
- **Berserk** fires from the upstream **`ABILITYEFFECT_COLOR_CHANGE`** case (`src/battle_util.c`, the Color Change /
  Berserk / Anger Shell cluster) — the **per-damaged-battler** move-end step (`MoveEndColorChange` iterates *every*
  damaged battler, so a spread move can trigger the reaction on each holder), **not** `ABILITYEFFECT_MOVE_END`. So it
  introduces a small **new on-damage driver `TryActivateInnateOnDamageEffects` -> `IsActiveOnDamageInnate`**
  (`src/fork/innate_abilities.c`), the on-damage analogue of the on-hit driver, hooked from the new
  **`MOVEEND_COLOR_CHANGE_INNATE`** step (`src/battle_move_resolution.c`) right after the chosen-ability
  `MOVEEND_COLOR_CHANGE` block and **looped over every battler** like it (a nested per-battler innate cursor). It
  delegates to the upstream `ABILITYEFFECT_COLOR_CHANGE` case, so the Sp. Atk raise / script / pop-up match the real
  ability. `HadMoreThanHalfHpNowDoesnt(battler)` is the upstream HP-crossing gate, reused unchanged.
- **Soul-Heart** fires from the native command **`BS_TryActivateSoulheart`** (`src/battle_script_commands.c`), run
  unconditionally by `BattleScript_FaintBattler` on **every** faint and already looping over every battler. It read
  only `GetBattlerAbility(b) == ABILITY_SOUL_HEART`, so it now also credits an innate Soul-Heart (chosen ability isn't
  Soul-Heart) and overwrites the pop-up to it. **No driver** — like Defiant / Competitive, the reaction has always
  been a dedicated command, so the innate hooks it directly.

**The pop-up.** Each of the three effect sites sets `gBattleScripting.abilityPopupOverwrite` to the innate **only when
the chosen ability differs**, so a real ability stays byte-for-byte unchanged (the Speed Boost / Rough Skin precedent).
Moxie's As One branches (never innates) still override the pop-up to their sub-ability as before. Each driver / credit
skips an innate equal to the chosen ability, so a real Moxie / Berserk / Soul-Heart never fires twice.

**No `DETERMINISTIC_*` surface.** All three trigger at 100% on the right event (a KO, an HP-crossing hit, a faint), so
there is nothing to gate under `DETERMINISTIC_ABILITIES`.

**AI.** Only **Moxie** has dedicated *effect* reads (the reactions live outside the shared damage calc; Berserk and
Soul-Heart have **no** `battle_ai_*.c` read, so no AI wiring is needed):
- **`AI_CheckBadMove` Protect self-faint check** (`src/battle_ai_main.c`) — "don't penalize Protect for fainting to
  secondary damage if the holder has a Moxie-type ability" — and the **sacrifice-the-ally spread scoring**
  (`src/battle_ai_main.c`) — "it benefits from the ally's death" — both read `IsMoxieTypeAbility(aiData->abilities[b])`.
  Beast Boost / Chilling Neigh / etc. are never innates, so only an **innate Moxie** needs crediting: each now also
  checks `IsInnateActive(b, ABILITY_MOXIE)`. The `ShouldTriggerAbility` Moxie case and the `scoringPartnerAbility`
  promotion are left as-is (Moxie isn't a react-to-being-hit ability, so it isn't in the on-hit partner-fire block).

**Suppression parity** holds via `IsInnateActive()` for all three (feature flag, Gastro Acid, Neutralizing Gas,
not-on-field); none is `breakable`, so Mold Breaker never touches them.

**Species.**
- **Moxie (canon-only, no flavor picks — a snowballing +1-per-KO is potent and hard to justify thematically
  off-roster).** Every canon user in **any** real slot, keyed exactly per form (merged into an existing innate row
  where present), plus base creatures' **Megas** as pure-boon mirrors: the **Pinsir** (+ Mega), **Gyarados** (+ Mega),
  **Honchkrow**, **Heracross** (+ Mega), **Mightyena**, **Salamence** (+ Mega), **Sandile / Krokorok / Krookodile**,
  **Scraggy / Scrafty**, **Litleo / Pyroar**, and **Quaxly / Quaxwell / Quaquaval** lines.
- **Berserk (canon-only).** Only **Drampa** — its non-Berserk slots (Sap Sipper / Cloud Nine) leave the innate
  **observable**. **Sole-Berserk species are OMITTED as redundant** (the Mega Lopunny / Scrappy precedent): **Galarian
  Moltres** (a frontier set that keeps its now-redundant chosen Berserk) and **Drampa-Mega** (a sole-Berserk Mega, so
  its pure-boon mirror could never be observed).
- **Soul-Heart (flavor-only — the Bad Dreams / Darkrai precedent).** Its sole canon user **Magearna** (all four forms:
  base / Original / the fork's two Megas) is **sole-Soul-Heart**, so an innate could never be observed on it — it is
  **OMITTED as redundant** (its frontier sets keep the now-redundant chosen Soul-Heart). Instead a tight
  soul-collector flavor set carries an **observable** innate Soul-Heart: the **Duskull / Dusclops / Dusknoir**
  grim-reaper line (Dusknoir ferries spirits) and **Spiritomb** (its 108 bound souls), each merged onto the species'
  existing innate row.

**Step 3.5**: the frontier sets that hardcoded chosen Moxie (Pinsir, Gyarados, Salamence, Krookodile, Scrafty, …),
Berserk (Galarian Moltres, Drampa), and Soul-Heart (Magearna) already resolve to the species' real slot, so they keep
their now-redundant chosen ability — still correct (the chosen runs it; the innate is redundant-but-skipped) — with the
complementary-slot re-pointing **deferred** as a focused follow-up, like the earlier Batch M sub-groups and Batch
J/T/K/L.

### ABILITY_BATTERY / ABILITY_POWER_SPOT / ABILITY_TELEPATHY / ABILITY_AROMA_VEIL / ABILITY_FLOWER_VEIL

The **ally-support batch (Batch U)** — team-oriented, mostly doubles-relevant effects. All five are **1:1
clean-upside copies** (none ever hurts its holder) and **canon-only** (no flavor picks: these are
partner/side-support effects with no thematic hook off their canon users). Each is suppression-safe via
`IsInnateActive()` and none is `breakable`, so Mold Breaker never touches them.

- **Battery / Power Spot (partner damage boosters).** Battery boosts the attacker's **special** moves ×1.3, Power
  Spot boosts **all** the attacker's moves ×1.3. Wired in `CalcAttackStat` (`src/battle_util.c`) in the
  **attacker-partner** block, right **beside the wired Steely Spirit innate clause** — an `IsInnateActive(partner, X)`
  credit guarded against the chosen-ability `case` above so it never double-applies. Pure calc modifiers, so the AI's
  damage prediction (which runs the same calc keyed off the real battler) is **innate-aware for free**; the
  `AI_GetAbilityValue` Power Spot case is an ability-*value* scorer (Trace territory, not an effect read) and is left
  keyed to the chosen ability. Canon-only: **Charjabug** (Battery), **Stonjourner** (Power Spot).

- **Telepathy (dodge an ally's move).** An innate Telepathy holder nullifies its **partner's** damaging move exactly
  like the real ability. Wired beside the cached chosen-ability read in the type-effectiveness / damage-modifier calc
  (`src/battle_util.c`) via `IsInnateActive`; when the innate did the dodging, `gLastUsedAbility` is forced to
  Telepathy so the message names it. Runs in the shared calc → AI damage prediction is innate-aware for free; the AI's
  **Wide Guard** heuristic (`src/battle_ai_main.c`, "don't bother if my Telepathy ally dodges the spread move anyway")
  is made innate-aware with a `BattlerHasAbility`-style check. Canon-only: the ~26 canon Telepathy users in any real
  slot (the **Ralts / Meditite / Munna / Elgyem / Wobbuffet / Dialga / Palkia / Giratina-Altered / Tapu** quartet **/
  Oranguru / Noibat / Blipbug / Rabsca** lines), plus **Gardevoir-Mega / Medicham-Mega** as pure-boon mirrors.

- **Aroma Veil (side-wide mental-status shield).** Protects the whole side from infatuation, Taunt, Disable, Encore,
  Torment and Heal Block. It is wired through the new **`IsInnateOnSide()`** companion to `IsAbilityOnSide()`
  (`src/battle_util.c`) — the innate-aware, feature-gated, species-based side check. Two classes of site:
  - **C guards** (`!IsAbilityOnSide(...)` → also `!IsInnateOnSide(...)`): the Cute Charm and Cursed Body blockers in
    `src/battle_util.c`, and the Attract (`Cmd_tryinfatuating`, `BS_TrySetInfatuation`) and Torment (`BS_TrySetTorment`)
    and Psychic-Noise-Heal-Block (`MOVE_EFFECT_PSYCHIC_NOISE`) sites in `src/battle_script_commands.c`.
  - **The script chokepoint** — `Cmd_jumpifability`'s `BS_ATTACKER_SIDE` / `BS_TARGET_SIDE` cases
    (`src/battle_script_commands.c`) fall back to `IsInnateOnSide` when the chosen-ability side check misses. This is
    the **only** script `jumpifability` side form and is used **solely by Aroma Veil**, so this one central edit makes
    an innate Aroma Veil block Taunt / Disable / Encore / Heal Block (which reach the block via script
    `jumpifability`) like the real ability. Each pop-up site overwrites to Aroma Veil when the protector's chosen
    ability differs (Speed Boost precedent). AI: `AI_CanBeInfatuated`, the `AI_CheckBadMove` Aroma-Veil switch cases
    (via a side-wide pre-check before the switch), and the "don't bait into Encore" read all credit an innate via
    `AI_IsInnateOnSide`. Canon-only: the **Spritzee / Milcery** (incl. the default **Alcremie** form + **Gmax**) **/
    Lechonk** (Oinkologne-F) **/ Dachsbun** lines. (Alcremie's 63 decorative sub-forms are represented by the default
    form only — the Vivillon-pattern precedent for cosmetic forms sharing one ability.)

- **Flower Veil (Grass-ally status + stat-drop shield).** Protects Grass-type allies from non-volatile status **and**
  from stat drops. Wired by making the two existing chokepoints innate-aware: `IsFlowerVeilProtected`
  (`src/battle_script_commands.c`, the status path — used by `CanSetNonVolatileStatus` and the AI switch-in check, so
  both become innate-aware at once) falls back to `IsInnateOnSide`, and `StatChange_IsFlowerVeilProtected`
  (`src/battle_stat_change.c`, the stat-drop path) also credits `IsInnateActive`. Both callers overwrite the pop-up to
  Flower Veil when the protector's chosen ability differs. AI: the Flower-Veil stat-drop-protect read
  (`src/battle_ai_util.c`), the Yawn/status switch read (`src/battle_ai_switch.c`), and the `AI_CheckBadMove` pre-check
  all credit an innate via `AI_IsInnateOnSide`. Canon-only: the **Flabébé / Floette / Florges** (all color forms) and
  **Comfey** lines.

**Step 3.5**: six frontier sets freed — **Musharna / Rabsca** (Telepathy now innate) → chosen **Synchronize**,
**Oranguru / Florges** (Telepathy / Flower Veil) → chosen **Symbiosis** (both `:x:`, stable, complementary real
slots), and **Stonjourner** (sole Power Spot, doubles-only) takes a fork-owned **Solid Rock** override
(`species_ability_overrides.c`, an implemented `:white_check_mark:` innate, stable) on its empty slot 1, like
Ogerpon-Cornerstone. The seven **Dialga / Palkia / Giratina / Orbeetle / Aromatisse** sets whose real abilities are
**all** now innate keep their now-redundant chosen ability — still correct (the chosen runs it; the innate is
redundant-but-skipped) — **deferred** as a focused follow-up, like Batch J/T/K/L. This completes Batch U.

### ABILITY_CHILLING_NEIGH / ABILITY_GRIM_NEIGH / ABILITY_ELECTROMORPHOSIS

**Batch Y's first sub-group (Y1)** — the promoted-from-rejected clones, each a **1:1 clean-upside copy** of an
already-implemented ability whose driver already exists, so the wiring is near-free (reuse the existing site).
**Chilling Neigh** raises the holder's **Attack** by 1 stage and **Grim Neigh** its **Sp. Atk** by 1 stage for
each foe it knocks out with a move — the on-KO half of **Moxie**. **Electromorphosis** charges the holder's next
Electric move (the Charge volatile) when it is hit by **any** damaging move — **Wind Power** minus the wind-move
gate. Each was `:x:` only because Moxie's / Wind Power's driver hadn't shipped when triaged; both drivers now
exist, so these are one-line additions.

**Two shared effect sites, no new C at either:**
- **Chilling Neigh / Grim Neigh** fire from the upstream **`ABILITYEFFECT_MOVE_END_FOES_FAINTED`** case
  (`src/battle_util.c`, the Moxie / Beast Boost / Chilling Neigh / Grim Neigh cluster) — the exact case the
  **attacker-side** on-hit driver (`TryActivateInnateOnHitAttackerEffects`, hooked from
  `MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE`) already delegates to for Moxie / Magician. So each is a
  **one-line addition to `IsActiveOnHitAttackerInnate`** (`src/fork/innate_abilities.c`). The case already
  reads `stat = STAT_SPATK` for Grim Neigh and `STAT_ATK` otherwise, counts `NumFaintedBattlersByAttacker`,
  and forces `gBattleScripting.abilityPopupOverwrite` to the innate when the chosen ability differs — all for
  free, so no edit to the effect site itself.
- **Electromorphosis** fires from the upstream **`ABILITYEFFECT_MOVE_END`** case (`src/battle_util.c`), which
  Wind Power and Electromorphosis already **share as a fall-through** (`case ABILITY_WIND_POWER:` gates on
  `IsWindMove` then falls through to `case ABILITY_ELECTROMORPHOSIS:`, which has no wind gate). So it is a
  **one-line addition to `IsActiveOnHitInnate`** — the target-side on-hit driver
  (`TryActivateInnateOnHitEffects`) delegates to that case, and the shared effect site already forces the
  pop-up to the innate when the chosen ability differs. `BattleScript_WindPowerActivates` is reused verbatim.

**No `DETERMINISTIC_*` surface.** All three trigger at 100% on the right event (a KO, a damaging hit), so there
is nothing to gate under `DETERMINISTIC_ABILITIES`.

**AI.** Only the neighs have dedicated *effect* reads. The two Moxie-type reads in `src/battle_ai_main.c` (the
`AI_CheckBadMove` Protect self-faint check — "don't penalize Protect for fainting to secondary damage if the
holder benefits from a KO" — and the sacrifice-the-ally spread scoring) read
`IsMoxieTypeAbility(aiData->abilities[b])`. They were already innate-aware for Moxie via
`IsInnateActive(b, ABILITY_MOXIE)`; that inline clause is replaced by the fork helper
**`IsMoxieTypeInnateActive(b)`** (`src/battle_ai_util.c`, beside `IsMoxieTypeAbility`), which credits an innate
Moxie **or** Beast Boost **or** Chilling Neigh **or** Grim Neigh (the Moxie-type set members that can be innates —
the As One combos never are; Beast Boost joined this helper in Batch Y7). **Electromorphosis needs no AI wiring**: Wind Power's dedicated reads
(`GetWindAbilityScore`, the switch-in Tailwind sim) are wind/Tailwind-specific and do not apply to a
charge-on-any-hit clone, and the Charge volatile itself is not a state the AI dodges.

**Suppression parity** holds via `IsInnateActive()` for all three (feature flag, Gastro Acid, Neutralizing Gas,
not-on-field); none is `breakable`, so Mold Breaker never touches them.

**Species (canon-only — the neighs are potent snowballing on-KO boosts like Moxie, the charge is
signature-specific).**
- **Chilling Neigh → Glastrier** and **Grim Neigh → Spectrier**, each the ability's **sole** canon user (the
  As One combo abilities on the Calyrex forms are `:x:` identity abilities, never innates). Both are
  **sole-ability genderless legends with a frontier set**, so — like **Landorus-Therian / Ogerpon-Cornerstone**
  — each takes the innate **plus a fork-owned chosen override** in its empty slot 1
  (`src/fork/species_ability_overrides.c`) so the innate is **observable**: **Glastrier → Snow Warning** (`:x:`,
  the ice-legend standard — its snow boosts its own Ice-type Defense for the Body Press set), **Spectrier →
  Infiltrator** (an implemented `:white_check_mark:` innate it does not carry — its Nasty Plot / Substitute
  sweeper ignores the foe's screens and Substitute).
- **Electromorphosis → Bellibolt** (canon slot 0); its **Static / Damp** slots leave the innate observable
  without an override.

**Step 3.5**: five frontier sets freed. Glastrier ×2 → chosen **Snow Warning** and Spectrier ×2 → chosen
**Infiltrator** (both via the new override rows), and Bellibolt's singles set → its complementary real
slot-1 **Static** (its doubles set already runs Static). Each now runs the chosen ability **and** the innate.
This is **Batch Y sub-group Y1**; the remaining Batch Y sub-groups (Y2–Y8) stay open.

### ABILITY_TRANSISTOR / ABILITY_DRAGONS_MAW

**Batch Y's second sub-group (Y2)** — two **flat type-power-booster clones** of Batch A's Steelworker /
Rocky Payload, each a **1:1 clean-upside copy** (a flat conditional power boost never hurts the holder).
**Transistor** boosts the holder's **Electric** moves and **Dragon's Maw** its **Dragon** moves. Both were
`:x:` only because they read as "just another type booster" at triage; they are now wired exactly like the
Batch A boosters they clone.

**One shared effect site, two lines.** Both are pure **`CalcAttackStat`** modifiers (`src/battle_util.c`),
added to the `if (ctx->innatesEnabled)` innate block that already holds **Rocky Payload / Stakeout / Guts /
Huge Power** (beside the Rocky Payload line):

```c
if (moveType == TYPE_ELECTRIC && atkAbility != ABILITY_TRANSISTOR && IsInnateActive(battlerAtk, ABILITY_TRANSISTOR))
    modifier = uq4_12_multiply(modifier, UQ_4_12(GetConfig(B_TRANSISTOR_BOOST) >= GEN_9 ? 1.3 : 1.5));
if (moveType == TYPE_DRAGON && atkAbility != ABILITY_DRAGONS_MAW && IsInnateActive(battlerAtk, ABILITY_DRAGONS_MAW))
    modifier = uq4_12_multiply(modifier, UQ_4_12(1.5));
```

Each mirrors its chosen-ability `case` in the attack-stat switch **exactly** — Transistor's multiplier follows
`B_TRANSISTOR_BOOST` (x1.3 in GEN_9+, else x1.5), Dragon's Maw is a flat x1.5. The `!= ABILITY_X` guard skips
the case the switch already applied so a holder running the real ability never double-applies.

**No `DETERMINISTIC_*` surface** (pure calc, no RNG).

**AI is free.** Neither has a dedicated `battle_ai_*.c` effect read (grep confirms only the ability-table
entries in `src/data/abilities.h`), and the boost lives in the shared damage calc the AI runs keyed off the
real battler, so on-field damage prediction is innate-aware automatically. **Suppression parity** holds via
`IsInnateActive()`; neither is `breakable`, so Mold Breaker never touches them.

**Species (canon-only — sole-ability signature legends).** **Transistor → Regieleki** (added beside its existing
innate Levitate) and **Dragon's Maw → Regidrago**, each the ability's **sole** canon user. Both are
**sole-ability genderless Regi legends with a frontier set**, so — like Glastrier / Spectrier (Y1) — each takes
the innate **plus a fork-owned chosen override** in its empty slot 1 (`src/fork/species_ability_overrides.c`) so
the innate is **observable**: **Regieleki → Lightning Rod** (`:x:`, the Raichu-Alola precedent — the Electron
Pokémon draws in Electric moves for immunity + a Sp. Atk boost), **Regidrago → Adaptability** (an implemented
`:white_check_mark:` innate it does not carry — self-synergistic with innate Dragon's Maw for a devastating
Choice Dragon breaker: 2x STAB on top of the 1.5x boost).

**Step 3.5**: four frontier sets freed. Regieleki ×2 → chosen **Lightning Rod** and Regidrago ×2 → chosen
**Adaptability** (both via the new override rows). Each now runs the chosen ability **and** the innate type-power
boost. This is **Batch Y sub-group Y2**; the remaining Batch Y sub-groups (Y3–Y8) stay open.

### ABILITY_PRISM_ARMOR / ABILITY_SHADOW_SHIELD / ABILITY_NEUROFORCE / ABILITY_SUPREME_OVERLORD

**Batch Y's third sub-group (Y3)** — four **damage / power calc clones**, each a **1:1 clean-upside copy**.
Three are the "unbreakable" cousins of already-wired Batch B / P defenders and attackers; the fourth is a
party-faint-gated power boost that reuses the Batch L switch-in driver to latch its counter.

**Prism Armor / Shadow Shield ride the existing Batch B innate clauses** in `GetDefenderAbilitiesModifier`
(`src/battle_util.c`) — no new clause, just an added `IsInnateActive(...)` disjunct beside the ones already
there:

```c
// -25% vs a supereffective hit (shares Filter / Solid Rock's clause)
if (ctx->typeEffectivenessModifier >= UQ_4_12(2.0)
 && ctx->abilities[ctx->battlerDef] != ABILITY_FILTER
 && ctx->abilities[ctx->battlerDef] != ABILITY_SOLID_ROCK
 && ctx->abilities[ctx->battlerDef] != ABILITY_PRISM_ARMOR
 && (IsInnateActive(ctx->battlerDef, ABILITY_FILTER) || IsInnateActive(ctx->battlerDef, ABILITY_SOLID_ROCK)
  || IsInnateActive(ctx->battlerDef, ABILITY_PRISM_ARMOR)))
    modifier = uq4_12_multiply(modifier, UQ_4_12(0.75));
// halve at full HP (shares Multiscale's clause)
if (IsBattlerAtMaxHp(ctx->battlerDef)
 && ctx->abilities[ctx->battlerDef] != ABILITY_MULTISCALE
 && ctx->abilities[ctx->battlerDef] != ABILITY_SHADOW_SHIELD
 && (IsInnateActive(ctx->battlerDef, ABILITY_MULTISCALE) || IsInnateActive(ctx->battlerDef, ABILITY_SHADOW_SHIELD)))
    modifier = uq4_12_multiply(modifier, UQ_4_12(0.5));
```

**Unbreakable, and that falls out for free.** Prism Armor and Shadow Shield have `.breakable = FALSE`
(unlike Solid Rock / Multiscale), and `IsInnateActive` reads **each ability's own** `breakable` flag through
`CanBreakThroughInnate`, so an attacker's **Mold Breaker cannot pierce** the innate Prism Armor / Shadow
Shield while it **does** pierce the innate Solid Rock / Multiscale — exactly the canon split. (Contrast tests
assert both halves.)

**Neuroforce is the offensive mirror of Tinted Lens** — a `GetAttackerAbilitiesModifier` clause beside the
innate Tinted Lens / Sniper ones, boosting the holder's **supereffective** hits x1.25:

```c
if (typeEffectivenessModifier >= UQ_4_12(2.0) && abilityAtk != ABILITY_NEUROFORCE
 && innatesEnabled && IsInnateActive(battlerAtk, ABILITY_NEUROFORCE))
    return UQ_4_12(1.25);
```

**Supreme Overlord is a `CalcAttackStat` modifier gated on a switch-in-latched counter.** The real ability
sets `supremeOverlordCounter[battler] = min(5, faintCounter[trainer])` in its `ABILITYEFFECT_ON_SWITCHIN`
case (showing a pop-up) and reads it back via `GetSupremeOverlordModifier` in `CalcAttackStat`. To match this
for an innate holder, Supreme Overlord is added to **`SwitchInInnateAbilityEffect`** (mapping it to
`ABILITYEFFECT_ON_SWITCHIN`, so the **Batch L switch-in driver** runs the real case — latching the counter and
showing the pop-up, overwritten to the innate when the chosen ability differs), plus a one-line innate clause
in `CalcAttackStat` beside the Batch A boosters:

```c
if (atkAbility != ABILITY_SUPREME_OVERLORD && IsInnateActive(battlerAtk, ABILITY_SUPREME_OVERLORD))
    modifier = uq4_12_multiply(modifier, GetSupremeOverlordModifier(battlerAtk));
```

`GetSupremeOverlordModifier` returns 1.0 when the counter is 0, so the clause is a safe no-op before any
teammate falls.

**No `DETERMINISTIC_*` surface** (all pure calc / switch-in).

**AI is nearly free.** All four live in the shared damage calc the AI runs keyed off the real battler, so
on-field damage prediction is innate-aware automatically. The **only** dedicated read is
`AI_IsBattlerAtMaxHp`'s full-HP-survival check (`battle_ai_util.c`), made innate-aware for **Shadow Shield**
beside the existing Multiscale disjunct. The AI switch-in sim's `SUPREME_OVERLORD` case is a no-op (it sets no
stat), so it needs no innate wiring. **Suppression parity** holds via `IsInnateActive()`.

**Species (canon-only).** The sole-ability legends **Prism Armor → Necrozma / Necrozma-Dusk-Mane /
Necrozma-Dawn-Wings** and **Shadow Shield → Lunala** (all frontier sets) take the innate **plus a fork-owned
chosen `ADAPTABILITY` override** in their empty slot 1 (`src/fork/species_ability_overrides.c`) — an
implemented `:white_check_mark:` innate they do not carry, self-synergistic 2x STAB for their Photon Geyser /
Moongeist-Beam sweeper sets — so the innate is **observable** and the frontier set is freed. **Neuroforce →
Necrozma-Ultra** is a transform-only Ultra Burst form and **not** a frontier set, so it takes the innate for
effect + test coverage but **no override** (its sole chosen Neuroforce already grants it; the innate is
observed via a test's forced chosen ability). **Supreme Overlord → Kingambit** joins its existing innate
**Defiant / Pressure** (making all three of its real abilities innate); its two frontier sets are freed from
the now-innate chosen Supreme Overlord to chosen **Defiant** (its slot-0 signature), keeping the innate
Supreme Overlord observable via its switch-in pop-up.

**Step 3.5**: seven frontier sets freed — Lunala ×2 + Necrozma-Dusk-Mane / Dawn-Wings / base → chosen
**Adaptability** (via the new override rows), Kingambit ×2 → chosen **Defiant**. This is **Batch Y sub-group
Y3**; the remaining Batch Y sub-groups (Y4–Y8) stay open.

### ABILITY_FULL_METAL_BODY / ABILITY_MINDS_EYE

**Batch Y's fourth sub-group (Y4)** — two **stat-drop / accuracy / hit-trait clones**, each a **1:1 clean-upside
copy**. Full Metal Body is the *unbreakable* cousin of Clear Body; Mind's Eye is Keen Eye + Scrappy combined.

**Full Metal Body rides the existing Batch D+E stat-drop-protection path** in `GetInnateStatDropProtector` /
`IsAbilityBlocked` (`src/battle_stat_change.c`) — a new full-protection clause beside Clear Body / White Smoke
that keeps **any** of the holder's stats from being lowered by another mon's move or ability, using the same
`MarkStatsAsDone(NUM_BATTLE_STATS)` + `BattleScript_AbilityNoStatLoss` script and the pop-up/record overwrite to
the innate. The two dedicated AI reads were made innate-aware beside their Clear Body disjuncts: `CanLowerStat`
(`src/battle_ai_util.c`) and `CanIntimidateLowerOpponentAtk` (`src/battle_ai_switch.c`).

**Unbreakable, and that falls out for free.** Full Metal Body has `.breakable = FALSE` (unlike Clear Body /
White Smoke), and `IsInnateActive` reads each ability's own `breakable` through `CanBreakThroughInnate`, so an
attacker's **Mold Breaker cannot pierce** the innate Full Metal Body while it **does** pierce the innate Clear
Body — exactly the canon split (contrast test asserts both). Its only canon user is **Solgaleo**.

**Mind's Eye is Keen Eye + Scrappy in one ability.** It clones three effects, each at the site its model already
touches — no new clause shape:
- **Evasion-ignore** (the Keen Eye half) — `GetTotalAccuracy` and the deterministic PP-economy delta
  `GetAccEvasionStageDelta` (`src/battle_util.c`), an `IsInnateActive(battlerAtk, ABILITY_MINDS_EYE)` disjunct beside
  the Compound Eyes / Keen Eye ones. PURE BOON: boost-only (`evasionStage > default`), like the Keen Eye clause.
- **Own accuracy can't be lowered** (the Keen Eye half) — a `stat == STAT_ACC` clause in
  `GetInnateStatDropProtector` (`src/battle_stat_change.c`) beside Keen Eye, pop-up overwritten to Mind's Eye.
- **Normal/Fighting hit Ghosts** (the Scrappy half) — the innate else-if in `MulByTypeEffectiveness`
  (`src/battle_util.c`) now also fires for an innate Mind's Eye (no `RecordAbilityBattle` — not the displayed ability).

Mind's Eye has **no Intimidate immunity** (unlike Scrappy), so it is deliberately *not* added to the
`IsIntimidateBlocked` / `CanIntimidateLowerOpponentAtk` Intimidate paths. **AI:** the evasion-ignore and Ghost-hit
live in shared calcs the AI runs keyed off the real battler (free); the two dedicated reads made innate-aware are the
accuracy branch of `CanLowerStat` (`src/battle_ai_util.c`) and the `EFFECT_FORESIGHT` "Foresight is pointless" score
(`src/battle_ai_main.c`). Mind's Eye is **breakable**, so Mold Breaker pierces the innate exactly like Keen Eye /
Scrappy (contrast test asserts it). Its only canon user is **Ursaluna-Bloodmoon**.

**No `DETERMINISTIC_*` surface beyond the one already covered** — the evasion-ignore is tested under
`DETERMINISTIC_ACCURACY_EVASION` (the PP-tax mirror), like Keen Eye; the stat-drop protection routes through the
same `IsAbilityBlocked` that needs no additional-effects/hold-effect mirror (the Batch D note).

**Species (canon-only, no flavor picks).** Both are sole-ability legends **and** frontier sets, so — like the Y2/Y3
legends — each takes the innate **plus a fork-owned chosen override** in its empty slot 1
(`src/fork/species_ability_overrides.c`): **Solgaleo → Tough Claws** (an implemented `:white_check_mark:` innate it
does not carry, powering its Sunsteel Strike / Close Combat / Flare Blitz contact STAB on top of the innate stat-drop
lock — the Metagross precedent), **Ursaluna-Bloodmoon → Unaware** (an implemented `:white_check_mark:` innate,
stable — ignoring the foe's boosts on its Calm Mind special tank, alongside the innate evasion-ignore + Ghost
coverage). So the innate is **observable** and the frontier set is freed.

**Step 3.5**: three frontier sets freed — Solgaleo ×2 → chosen **Tough Claws**, Ursaluna-Bloodmoon → chosen
**Unaware** (via the new override rows). This is **Batch Y sub-group Y4**; the remaining Batch Y sub-groups
(Y5–Y8) stay open.

### ABILITY_PURIFYING_SALT / ABILITY_GOOD_AS_GOLD

**Batch Y's fifth sub-group (Y5)** — two **status-immunity clones**, each a **1:1 clean-upside copy**. Purifying
Salt is a whole-status immunity + a Ghost-damage cut (Batch I + Batch B); Good as Gold is a blanket status-*move*
immunity.

**Purifying Salt is wired at two sites in `src/battle_util.c`:**
- **Status immunity** — the **catch-all Comatose/Purifying-Salt block** in `CanSetNonVolatileStatus` (the block
  that runs after `IsNonVolatileStatusBlocked` returns FALSE) gets an `|| IsInnateActive(battlerDef,
  ABILITY_PURIFYING_SALT)` disjunct, so the holder is immune to **every** non-volatile status (burn, poison,
  paralysis, sleep, freeze/frostbite). Because the per-status `CanBe*` wrappers (`CanBeParalyzed`,
  `CanBePoisoned`, …) call `CanSetNonVolatileStatus`, their **AI callers are innate-aware for free** — the AI
  won't throw a status move at an innate holder. When the chosen ability differs, `abilityDef` is reassigned to
  Purifying Salt (so it's recorded) and the pop-up is overwritten (the Magma Armor / Limber precedent); the
  message is `BattleScript_AbilityProtectsDoesntAffect` ("It doesn't affect …").
- **Ghost-damage cut** — a 1:1 clause in the target's-abilities block of `CalcAttackStat` **beside the innate
  Thick Fat** (the same site the real Purifying Salt's Ghost case lives): an innate Purifying Salt halves incoming
  Ghost damage. Silent calc modifier (no `RecordAbilityBattle`), so on-field AI damage prediction is correct for
  free (shared calc keyed off the real battler).

**PURE-BOON DIVERGENCE (Purifying Salt).** A real Purifying Salt **blocks the holder's own Rest** (it can't
sleep) — a cost. The innate deliberately does **not**: the `EFFECT_REST` gate in `src/battle_move_resolution.c`
stays **chosen-ability-only** (no `BattlerHasAbility`), exactly the Insomnia / Vital Spirit precedent, so an
innate holder still Rests to full HP and sleeps from its own move. (Rest's self-sleep runs through `trysetrest`
/ the script's chosen-slot `jumpifability`, not `CanSetNonVolatileStatus`, so the status-immunity wiring never
touches it.) It is **breakable**, so Mold Breaker pierces the innate exactly like the real ability (contrast +
Rest tests assert both).

**Good as Gold is wired in `CanAbilityAbsorbMove` (`src/battle_util.c`):** an innate holder blocks an incoming
status move (the same `TARGET_OPPONENTS_FIELD` / `TARGET_ALL_BATTLERS` exclusions as the real ability, so field
moves like Stealth Rock still land). A local `absorbAbility` carries the credited ability so the pop-up/record
show Good as Gold when the chosen ability differs. **AI:** the AI's own move scoring calls `CanAbilityAbsorbMove`,
so on-field prediction of the block is innate-aware for free; the four **dedicated** reads that check
`== ABILITY_GOOD_AS_GOLD` directly were made innate-aware with `BattlerHasAbility` — `AI_CanStatChangeBePrevented`
and `AI_ShouldSpicyExtract` (`src/battle_ai_util.c`) and the two `EFFECT_HELPING_HAND` partner reads
(`src/battle_ai_main.c`). Good as Gold is **breakable**, so Mold Breaker pierces it.

**BALANCE NOTE (Good as Gold).** This is a **very strong** innate — a blanket immunity to Thunder Wave, Toxic,
Will-O-Wisp, Spore, Leech Seed, and every stat-lowering status move. Wiring it as an innate is a **deliberate
power divergence**, kept **canon-only (Gholdengo alone)** to contain the blast radius, and called out in the
allowlist comment + `FORK.md`.

**No `DETERMINISTIC_*` surface** — a full status immunity / status-move immunity is checked before any chance
roll, so neither ability touches the accuracy/effect-chance/hold-effect reroutes.

**Species (canon-only, no flavor picks).** Purifying Salt → the **Nacli / Naclstack / Garganacl** salt line
(merged onto their existing Clear Body / Sturdy rows). Good as Gold → **Gholdengo** (its sole ability). Gholdengo
is a sole-ability frontier set, so — like the Y2/Y3/Y4 legends — it takes the innate **plus a fork-owned chosen
override** in its empty slot 1 (`src/fork/species_ability_overrides.c`): **Gholdengo → Sticky Hold** (an
implemented `:white_check_mark:` innate it does not itself carry, stable like Carnivine's Chlorophyll; thematic +
low-impact — the coin hoard won't be robbed), so the innate Good as Gold is **observable** and its three frontier
sets are freed (chosen Good as Gold → chosen Sticky Hold; the innate still blocks status moves).

**Step 3.5**: Gholdengo's three frontier sets freed → chosen **Sticky Hold** (via the new override row). The
**salt line is a Batch W deferral**: all three of Garganacl's real abilities (Purifying Salt / Sturdy / Clear
Body) are now innate with no free complementary slot, so its frontier sets keep their now-redundant chosen
Purifying Salt — still correct (the chosen ability runs; the innate is redundant-but-skipped there), matching the
Batch J/T all-abilities-innate deferrals. This is **Batch Y sub-group Y5**; the remaining Batch Y sub-groups
(Y6–Y8) stay open.

### ABILITY_INTREPID_SWORD / ABILITY_DAUNTLESS_SHIELD

**Batch Y's sixth sub-group (Y6)** — two **switch-in stat-boost clones**, each a **1:1 clean-upside copy**. The
first time the holder enters battle, Intrepid Sword raises its Attack and Dauntless Shield its Defense by 1 stage.

**Both ride the Batch L switch-in driver** (`TryActivateInnateSwitchInEffects`, `src/fork/innate_abilities.c`) —
a **one-line `SwitchInInnateAbilityEffect` case each** returning `ABILITYEFFECT_ON_SWITCHIN`, exactly like
Intimidate / Download / Supreme Overlord. The driver delegates to the upstream `ABILITYEFFECT_ON_SWITCHIN` case
(`src/battle_util.c`) with the innate passed explicitly, so the +1 stat change, the `BattleScript_AbilityStatChange`
script, and — crucially — the **once-per-battle latch** all match the real ability for free. The latch is the
per-party-mon `GetBattlerPartyState(battler)->intrepidSwordBoost` / `dauntlessShieldBoost` flag: under
`B_INTREPID_SWORD` / `B_DAUNTLESS_SHIELD` `>= GEN_9` (the default), the flag is set on the first trigger so the
boost fires only **once per battle**; under earlier gens it re-fires every switch-in. The flag is party-state, not
ability-keyed, so it works identically whether the holder runs Intrepid Sword as its chosen ability or as an
innate. Each effect site forces the pop-up to the innate when the chosen ability differs
(`if (GetBattlerAbility(battler) != gLastUsedAbility) gBattleScripting.abilityPopupOverwrite = gLastUsedAbility;` —
the Speed Boost / Download precedent).

**No pure-boon divergence** — both are self-only stat boosts with no cost. Neither is **breakable**, so Mold
Breaker never touches them (matching the real abilities). **No `DETERMINISTIC_*` surface** — an unconditional
switch-in boost has no chance roll.

**AI.** The switch-in stat simulation `SetBattlerStatStagesForSwitchin` (`src/battle_ai_switch.c`) — which lets
the AI value switching a boosting mon in — mirrors each self-boost for an **innate** holder via `SpeciesHasInnate`
(guarded on `aiAbility != ABILITY_INTREPID_SWORD` / `!= ABILITY_DAUNTLESS_SHIELD` so a chosen holder isn't
double-counted), right beside the existing innate Intimidate / Download / Supersweet Syrup mirrors. Like the
real-ability `case`s in the same function, it does **not** model the once-per-battle latch — it estimates the
boost.

**Species (canon-only, no flavor picks).** Intrepid Sword → **Zacian / Zacian-Crowned**; Dauntless Shield →
**Zamazenta / Zamazenta-Crowned**. All four are **sole-ability frontier sets**, so — like the Y2/Y3/Y4/Y5 legends
— each takes the innate **plus a fork-owned chosen override** in its empty slot 1
(`src/fork/species_ability_overrides.c`): **Zacian → Tough Claws** (an implemented `:white_check_mark:` innate it
does not itself carry, stable; powers its entirely-contact kit — Behemoth Blade / Play Rough / Close Combat /
Crunch / Wild Charge — like Solgaleo / Zarude) and **Zamazenta → Filter** (implemented `:white_check_mark:`,
stable, thematic for the "Shield" defender — blunts the supereffective hits its Body Press / Iron Defense wall
fears, like Melmetal / Stonjourner). The **same** chosen ability is given to both formes of each so the ability
stays consistent across the in-battle Hero ↔ Crowned form change. This makes the innate **observable** and frees
the frontier slot (chosen Intrepid Sword / Dauntless Shield → chosen Tough Claws / Filter; the innate still fires
at switch-in).

**Step 3.5**: all four frontier sets freed → chosen **Tough Claws** (Zacian ×2) / **Filter** (Zamazenta ×2) via
the new override rows + the `.ability` change in `src/fork/frontier_extended_mons.c`. This is **Batch Y sub-group
Y6**; the remaining Batch Y sub-groups (Y7–Y8) stay open (Y8 is blocked on Tier 5.5 Mold Breaker).

### ABILITY_BEAST_BOOST

**Batch Y's seventh sub-group (Y7)** — a single **on-KO best-stat clone**, a **1:1 clean-upside copy**. When the
holder knocks out a foe with a move, its **highest** stat rises by 1 stage — **Moxie**, best-stat edition. It was
`:x:` only because it's a clone that couldn't be observed on any of its (all sole-ability) canon users until the
frontier-override pattern existed; that pattern (Y2–Y6) is now routine.

**One shared effect site, no new C.** Beast Boost fires from the upstream **`ABILITYEFFECT_MOVE_END_FOES_FAINTED`**
case (`src/battle_util.c`) — the exact Moxie / Beast Boost / Chilling Neigh / Grim Neigh cluster the **attacker-side**
on-hit driver (`TryActivateInnateOnHitAttackerEffects`, hooked from `MOVEEND_ABILITY_EFFECT_FOES_FAINTED_INNATE`)
already delegates to. So it is a **one-line addition to `IsActiveOnHitAttackerInnate`**
(`src/fork/innate_abilities.c`) beside Moxie / the neighs. The case already special-cases Beast Boost with
`stat = GetHighestStatId(battler)`, counts `NumFaintedBattlersByAttacker`, and forces
`gBattleScripting.abilityPopupOverwrite` to the innate when the chosen ability differs — all for free, so no edit to
the effect site itself.

**No pure-boon divergence** — a self-only stat boost with no cost. **No `DETERMINISTIC_*` surface** — it triggers at
100% on a KO. Not **breakable**, so Mold Breaker never touches it (matching the real ability).

**AI.** Beast Boost is a Moxie-type on-KO ability, so the two Moxie-type reads in `src/battle_ai_main.c` (the Protect
self-faint check and the sacrifice-the-ally spread scoring) that pair `IsMoxieTypeAbility(chosen)` with
`IsMoxieTypeInnateActive(b)` already reach it — Beast Boost was **added to `IsMoxieTypeInnateActive`**
(`src/battle_ai_util.c`) so an innate holder is credited. No other AI site reads Beast Boost.

**Species (canon-only, no flavor picks).** Every canon Beast Boost user is an **Ultra Beast whose sole ability is
Beast Boost**, so the ten evolved/frontier UBs take the innate **plus a fork-owned chosen override** in their empty
slot 1 (`src/fork/species_ability_overrides.c`), each a stable `:x:` never-an-innate pick or an implemented
`:white_check_mark:` innate the species does not itself carry: **Nihilego → Merciless** (auto-crits its poisoned
targets), **Buzzwole → Iron Fist** (its punch kit), **Pheromosa → Tough Claws** (contact STAB), **Xurkitree →
Lightning Rod** (Electric immunity + Sp. Atk, the Regieleki precedent), **Celesteela / Guzzlord → Filter** (blunts
supereffective — Guzzlord's 4× Fairy), **Kartana → Sharpness** (its slicing Leaf Blade / Sacred Sword), **Naganadel
→ Sheer Force** (its Nasty Plot sweeper), **Stakataka → Solid Rock** (blunts supereffective on its Trick Room wall),
**Blacephalon → Infiltrator** (ignores screens / Substitute). This makes the innate **observable** and frees the
frontier slot. Merged into the existing innate **Levitate** rows where present (Nihilego / Xurkitree / Kartana /
Blacephalon float). The pre-evolution **Poipole** is **omitted as redundant** — sole Beast Boost, **not** a frontier
set, so its chosen ability already grants it and an innate could never be observed (the Calyrex / Mega Lopunny
precedent) — keeping only its existing innate Levitate row.

**Step 3.5**: all twenty frontier sets (two per UB) freed → their chosen override via the `.ability` change in
`src/fork/frontier_extended_mons.c`. This is **Batch Y sub-group Y7**; only **Y8** (Turboblaze / Teravolt, blocked on
Tier 5.5 Mold Breaker) remains open in Batch Y.
