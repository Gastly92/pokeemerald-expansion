# Innate abilities — how to add one

Fork feature, gated by `FEATURE_INNATE_ABILITIES` (`include/config/feature.h`).
A species' *innate* abilities are always active **in addition to** its single
chosen ability. This doc is the extension playbook; the flag comment and the
in-code comments (`include/innate_abilities.h`, `src/innate_abilities.c`) stay the
source of truth for exact semantics.

## The one rule: it's an allowlist, grown one ability at a time

Making an *arbitrary* ability work as an innate would mean routing every
"does this battler have ability X?" check through `BattlerHasAbility()` across
hundreds of upstream-owned sites — a large, perpetually merge-conflict-prone
sweep. We don't do that. Instead we **wire up one ability's behavior at a time**
and only let species declare innates from that supported set. Today the set is
**`LEVITATE`** (a passive Ground immunity) and **`REGENERATOR`** (a silent
1/3-HP switch-out heal).

So a future request like *"add ability X as an innate; species A/B/C should have
it"* breaks into two parts:

1. **Data** — always generic, no engine work. Add the species → list mapping.
2. **Effect wiring** — how much is automatic depends on the *kind* of ability
   (see the recipe). This is the part the allowlist gates.

## What the generic tooling already gives you (no per-ability work)

- **The species → innate table** (`src/innate_abilities.c`): a variable-length,
  `ABILITY_NONE`-terminated list per species (no fixed cap). `SpeciesHasInnate()`
  and `GetSpeciesInnate()` read it.
- **The trait predicate** `BattlerHasAbility(battler, ability)` (`src/battle_util.c`):
  TRUE if `ability` is the battler's chosen ability **or** an active innate.
- **Suppression parity** via `IsInnateActive()`: an innate honors Gastro Acid,
  Neutralizing Gas, Mold Breaker (on breakable abilities), Ability Shield, and
  not-on-field exactly like the same ability in a real slot. (Suppression parity
  only — an innate's *effect* may diverge by design: see the Levitate pure-boon
  note below, where an innate Levitate is intentionally a bit stronger than a real one.)
- **Identity stays deterministic**: innates are *never* copied/swapped/displayed
  as identity. Trace, Skill Swap, Role Play, the ability pop-up, and
  `RecordAbilityBattle` all keep reading only the primary slot
  (`GetBattlerAbility`). Do **not** change that.

## Recipe: "add ability X as an innate for species A/B/C"

### Step 1 — add the data (always)

In `src/innate_abilities.c`, give each species an `ABILITY_NONE`-terminated list.
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

A species may list several innates: `{ ABILITY_X, ABILITY_Y, ABILITY_NONE }`.

### Step 2 — put the ability on the allowlist

Add `ABILITY_X` to the **allowlist comment** in `src/innate_abilities.c` (and the
SCOPE note in `include/innate_abilities.h` if the supported set's character
changes). This is the human record of what's actually wired; keep it honest.

### Step 3 — wire the effect (the only per-ability work)

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
    (`src/frontier_battle_info.c`) already iterate the whole innate list, so they
    show any allowlisted ability with no per-ability work.

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
  Rough Skin). These need the re-entrant
  **`TryActivateInnateEffects(caseID, battler, *index, trigger)`** driver and a
  trigger hook. That driver + the switch-in / end-turn / move-end
  hooks were written and then removed when we scoped back to Levitate; restore
  them from git history as the reference implementation when the first active
  ability joins the allowlist:
  - driver + per-battler index fields: commit `198160fe` (`battle_util.c`,
    `battle.h`)
  - switch-in hook (`FIRST_EVENT_BLOCK_INNATE_ABILITIES`): commit `935f1674`
    (`battle_switch_in.c`)
  - end-turn hook (`THIRD_EVENT_BLOCK_ABILITIES_INNATE`): commit `198160fe`
    (`battle_end_turn.c`)
  - move-end / on-contact hooks (`MOVEEND_ABILITIES_INNATE`,
    `MOVEEND_ABILITIES_ATTACKER_INNATE`): commit `e93911db`
    (`battle_move_resolution.c`)

### Step 4 — test it

Add a case to `test/battle/innate_abilities.c`. Opt into the feature with
`WITH_CONFIG(FEATURE_INNATE_ABILITIES, TRUE)` (the test baseline forces all
`FEATURE_*` flags off, so the inherited suite keeps exercising stock behavior).
Cover: the innate's effect fires; it does **not** fire with the feature off; and,
for trait/immunity abilities, that suppression (Gastro Acid / Mold Breaker) and
Trace/identity still behave like the real ability. Run:

```bash
make -j$(nproc) check TESTS="FEATURE_INNATE_ABILITIES"
```

### Step 5 — update the index

Edit the **Innate abilities** row in `FORK.md`: add the ability to the supported
set and note any new wiring or limitation.

## Why "mostly automatic" depends on the ability

Steps 1, 2, 4, 5 are mechanical for every ability. Step 3 is the variable:
single-site passive traits are a one-line swap; calc-modifier passives are a
small clause; active abilities reuse the driver but need their trigger hook
restored. Keeping the allowlist small and explicit is what bounds the
upstream-file footprint and the merge-conflict surface — that is the whole point
of going ability by ability.
