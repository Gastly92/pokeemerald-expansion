# New abilities — how to add a custom ability

Fork feature (**no flag; always compiled**). We define our own abilities on top of
upstream's, give them real in-battle effects, and hand them to species — either through the
fork's [ability-override table](#step-5--give-a-species-the-ability) or a normal `gSpeciesInfo`
slot. This doc is the extension playbook; the in-code comments (`src/data/abilities.h`, the
hooks in `src/battle_util.c`, `src/fork/type_affinity.c`) stay the source of truth for exact
semantics.

The worked example throughout is the **Affinity** family — the fork's first custom ability
mechanic — whose first member is **`Psychic Affinity`** (grants a latent Psychic type in
battle). It's assigned to **Butterfree**.

## Design first: is this an ability, or an innate?

Before writing any code, decide which fork mechanism you want — they are **opposites**, and
the choice is what makes the feature coherent:

- An **innate** ([`INNATE_ABILITIES.md`](INNATE_ABILITIES.md)) is a **pure boon** layered
  *on top of* a mon's chosen ability. It may only ever help its holder; if the real ability
  has a cost, the innate keeps the upside and **drops the cost**.
- A **custom ability** (this doc) is a mon's **single chosen ability**, so it can carry a
  genuine **downside**. An ability that is all upside could just be an innate; giving it a
  drawback is exactly what makes it interesting *as a chosen ability* and impossible *as an
  innate*.

So the sweet spot for a new custom ability is a **decent upside paired with a real
downside** — a trade the player opts into. An Affinity ability fits perfectly: gaining a
type brings that type's STAB and resistances **but also its weaknesses**. That built-in
downside disqualifies it as an innate, which conversely makes it a maximally **stable** pick
for the ability-override table — the innate sweeps that periodically re-point override
abilities never have to touch it (see the stability rule in
`src/fork/species_ability_overrides.c`).

## Where custom ability ids live

Upstream's abilities run `0 … ABILITIES_COUNT_GEN9`. The fork's custom abilities occupy a
**reserved block just below that count** (`ABILITY_314`, `ABILITY_MEGA_SOL`,
`ABILITY_FIRE_MANE`, `ABILITY_SPICY_SPRAY`, … and now `ABILITY_PSYCHIC_AFFINITY`) in
`include/constants/abilities.h`. Some entries are numeric placeholders (`ABILITY_314`, with a
`-------` / "No special ability." data row) reserved for exactly this. **Prefer filling a
placeholder over appending** — it keeps `ABILITIES_COUNT` (and every `gAbilitiesInfo`-sized
array) stable and minimises the upstream-owned diff. Editing these two upstream-owned files is
unavoidable for a genuinely new ability (like adding a species or move — it touches shared
tables); keep the edits **additive and `FORK:`-tagged**.

## Recipe: "add ability X"

### Step 1 — declare the constant

In `include/constants/abilities.h`, rename a reserved placeholder (or add an id just before
`ABILITIES_COUNT_GEN9`):

```c
ABILITY_PSYCHIC_AFFINITY = 317, // FORK: "Affinity" family — grants a latent 3rd type in battle.
```

### Step 2 — add the data entry

In `src/data/abilities.h`, fill the matching `[ABILITY_X] = { … }` initializer with a
`.name`, `.description`, and `.aiRating`. **Both strings go through `charmap.txt`** — stick to
ASCII punctuation (no em-dash `—`; see the charmap note in `CLAUDE.md`). Keep the description
to one short line (it renders in a small box). `aiRating` is a coarse team-building / switch
heuristic; if the effect lives in the shared damage/type calc (Step 3), the AI already sees the
real numbers, so this is only a tiebreaker.

### Step 3 — wire the effect (the part that varies)

How much work this is depends entirely on the *kind* of effect. The cheapest and most
AI-friendly effects hook a **single shared chokepoint** that the on-field AI already reuses,
so its predictions stay correct for free. Two examples of that pattern in the fork:

- **Damage multipliers** → `GetAttackerAbilitiesModifier` / `GetDefenderAbilitiesModifier`
  (inside `GetOtherModifiers`) in `src/battle_util.c`.
- **Type identity** → `GetBattlerTypes()` in `src/battle_util.c` — the one accessor that
  STAB, defensive matchups, `IS_BATTLER_OF_TYPE`, and the whole AI funnel through. This is
  what the Affinity family uses (see the [case study](#case-study-the-affinity-family) below),
  mirroring how `FEATURE_NEW_TYPES` injects at `GetSpeciesType()`.

Other effect kinds hook elsewhere — an attack-stat boost goes in the attacker stat-modifier
switch (near `ABILITY_SOLAR_POWER`/`HUSTLE`), a switch-in or end-turn effect is scripted
through the battle-script drivers (see the `ABILITYEFFECT_ON_SWITCHIN` switch), an on-contact
reaction hooks the contact-effect path, and so on. Grep an existing ability with the shape you
want and copy its hook site. Keep the fork logic in a `src/fork/` file and register it at the
smallest possible hook point in the upstream file (`FORK:`-tagged).

### Step 4 — test it

Add `test/battle/ability/<ability>.c`. A custom ability is a **real ability**, not gated by
any `FEATURE_*` flag, so a plain battle test works with **no `WITH_CONFIG`** (unlike innate /
new-type tests). `Ability(...)` force-assigns any ability even if it is not one of the species'
real slots (via `forcedAbilities`), so you can put it on a flavourful holder directly. Probe
the mechanic from each angle it touches — for Psychic Affinity that's the STAB it grants
(offense), the weakness it adds (defense), its suppression under Tera, and the switch-in
message. Run just this file:

```bash
make -j$(nproc) check TESTS="Psychic Affinity"
```

### Step 5 — give a species the ability

Two ways:

1. **Fork ability-override table** (preferred, conflict-free) — add a row to
   `src/fork/species_ability_overrides.c` so the species presents the ability in a chosen slot
   **without editing upstream `gSpeciesInfo`**. This is what Butterfree uses:

   ```c
   { // 0012
       // Butterfree's real abilities are both innate, so its empty slot 1 takes Psychic Affinity …
       SPECIES_BUTTERFREE, 1,
       ABILITY_PSYCHIC_AFFINITY
   },
   ```

   If the species is a Battle Factory / extended-roster mon, also set the matching `.ability`
   on its set in `src/fork/frontier_extended_mons.c` (and retool its moves to actually use the
   ability — for Butterfree, swapping in `MOVE_PSYCHIC` to cash in the new STAB). The roster
   legality test (`test/fork/frontier_extended_roster.c`) fails CI if a set's `.ability`
   doesn't resolve to a real slot through the override hook, so keep the two in sync:

   ```bash
   make -j$(nproc) check TESTS="Frontier extended roster"
   ```

2. **Stock `gSpeciesInfo` slot** — edit the species' `.abilities = { … }` in
   `src/data/pokemon/species_info/*.h` (how `ABILITY_MEGA_SOL` etc. are attached). Touches
   upstream-owned data, so reserve it for cases the override table can't express.

### Step 6 — update the index

Add/extend the row in [`FORK.md`](FORK.md) (which ability, its effect, which species carry it,
any limitation).

## Case study: the Affinity family

An **Affinity** ability grants its holder a **latent third type in battle** — the mon gets
that type's STAB and resistances, but also its weaknesses. It's the fork's answer to
"lean a species into a flavour type it isn't officially" (Butterfree → Psychic; Lugia →
Water, planned).

**The engine already supports a third type.** `gBattleMons[battler].types[2]` is a real slot
(normally `TYPE_MYSTERY`), the same one the moves Trick-or-Treat and Forest's Curse write to.
The damage calc folds it into both STAB and the defensive matchup.

**One hook.** `src/fork/type_affinity.c` maps each Affinity ability to a type
(`GetAbilityAffinityType`) and injects it into an empty third slot (`TryApplyTypeAffinity`),
called from `GetBattlerTypes()`. Because every type-identity read funnels through that
accessor, the added type flows to STAB, matchups, `IS_BATTLER_OF_TYPE`, and the AI from that
single site. A switch-in case in the `ABILITYEFFECT_ON_SWITCHIN` handler announces it with an
ability popup + the message *"{mon}'s affinity awakened its latent {type} type!"*
(`STRINGID_TYPEAFFINITYAWAKENED`, `BattleScript_TypeAffinityActivates`).

**How it composes** (all handled by the one hook's placement + guards):

- **Terastal** — the Tera branch of `GetBattlerTypes` returns *before* the injection, so a
  Terastallized mon shows only its Tera type; the latent type returns when Tera ends.
- **`FEATURE_NEW_TYPES`** — independent slots: new-types rewrites the *base* types, Affinity
  fills the *third*. They stack.
- **Type-changing moves** (Trick-or-Treat, Soak, Reflect Type, Roost) — the injection only
  fills an **empty** third slot, so a move that writes `types[2]` keeps precedence.
- **Ability suppression** (Gastro Acid, Neutralizing Gas) — the type is read via
  `GetBattlerAbility`, so a suppressed Affinity ability drops its type too.
- **In-battle only** — like Trick-or-Treat, it changes battle types, not the Pokédex/summary.

### Adding a family member (e.g. Water Affinity for Lugia)

It's a data-only change once the machinery exists:

1. New constant `ABILITY_WATER_AFFINITY` (Step 1) + data entry (Step 2).
2. One line in `GetAbilityAffinityType`: `case ABILITY_WATER_AFFINITY: return TYPE_WATER;`.
3. One case in the `ABILITYEFFECT_ON_SWITCHIN` switch (or fold both abilities into a shared
   `case`), reusing `BattleScript_TypeAffinityActivates` — the message template already fills
   the type name from the ability.
4. Assign it (Step 5) and test it (Step 4).

## Scope / known limitations

- **Effect must be wired by hand.** Unlike innates (an allowlist with shared drivers), a
  custom ability's behaviour only exists where you add a hook. An id with a data entry but no
  engine hook is inert (that's what the `-------` / "Unimplemented." placeholders are).
- **AI is automatic only for shared-chokepoint effects.** Type/damage effects placed in
  `GetBattlerTypes` / `GetOtherModifiers` are predicted for free; an effect elsewhere may need
  explicit AI handling to be valued correctly.
- **Affinity is in-battle only** by design (matches Trick-or-Treat) — the summary screen and
  Pokédex still show the species' normal types.
