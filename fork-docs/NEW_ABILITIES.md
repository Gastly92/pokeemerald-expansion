# New abilities — how to add a custom ability

Fork feature (**no flag; always compiled**). We define our own abilities on top of
upstream's, give them real in-battle effects, and hand them to species — either through the
fork's [ability-override table](#step-5--give-a-species-the-ability) or a normal `gSpeciesInfo`
slot. This doc is the extension playbook; the in-code comments (`src/data/abilities.h`, the
hooks in `src/battle_util.c`, `src/fork/type_affinity.c`) stay the source of truth for exact
semantics.

The worked example throughout is the **Affinity** family — the fork's first custom ability
mechanic — whose members are **`Psychic Affinity`** (a latent Psychic type in battle, on
**Butterfree**) and **`Water Affinity`** (a latent Water type, on **Lugia**). The second member
cost four lines of engine code: the family's machinery is shared, so [adding
one](#adding-a-family-member) is a data change.

A second worked example, **`Halo`** (assigned to **Clefable**), is written up in its own
[case study](#case-study-halo) below. It is worth reading alongside Affinity because it shows
a different shape of effect: where an Affinity ability changes only its *holder*, Halo is an
**aura** whose effect applies to every battler on the field, and where Affinity pays for
itself with a type weakness, Halo pays for itself in **PP**.

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
Water).

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

### Adding a family member

Once the machinery exists a new member is a data change — this is exactly what
`Water Affinity` did:

1. New constant `ABILITY_WATER_AFFINITY` (Step 1) + data entry (Step 2).
2. One line in `GetAbilityAffinityType`: `case ABILITY_WATER_AFFINITY: return TYPE_WATER;`.
3. Fall the new ability into the family's shared `case` in the `ABILITYEFFECT_ON_SWITCHIN`
   switch, which reuses `BattleScript_TypeAffinityActivates` — the message template fills the
   type name from the ability, so it needs no new string.
4. Assign it (Step 5) and test it (Step 4). The family's composition rules (Tera suppression,
   an already-occupied third slot) are shared, so they're covered once for the family in
   `test/battle/ability/psychic_affinity.c`; a new member's own test only needs the angles
   that are specific to *its* type — for Water Affinity, the Water STAB it grants, the
   Electric weakness and the Ice resistance it adds, and its switch-in message.

**Picking holders.** An Affinity is a trade, so it should land on species whose flavour asks
for the type *and* whose learnset can cash the STAB — that's what makes the added weaknesses a
choice rather than a tax. Water Affinity's six holders are all sea-dwellers that aren't
officially Water: **Lugia** (guardian of the sea), **Masquerain** (Surskit loses its Water half
on evolution), **Beartic** (swims the frigid seas), **Dhelmise** (a sunken anchor in seaweed),
**Cursola** (coral) and **Cetitan** (a whale).

## Scope / known limitations

- **Effect must be wired by hand.** Unlike innates (an allowlist with shared drivers), a
  custom ability's behaviour only exists where you add a hook. An id with a data entry but no
  engine hook is inert (that's what the `-------` / "Unimplemented." placeholders are).
- **AI is automatic only for shared-chokepoint effects.** Type/damage effects placed in
  `GetBattlerTypes` / `GetOtherModifiers` are predicted for free; an effect elsewhere may need
  explicit AI handling to be valued correctly.
- **Affinity is in-battle only** by design (matches Trick-or-Treat) — the summary screen and
  Pokédex still show the species' normal types.

## Case study: Halo

**`Halo`** (`ABILITY_HALO`, on **Clefable**) is the fork's second custom ability, and it is
deliberately a different *shape* from Affinity — worth reading as a contrast.

> While a Halo holder is on the field, **no single hit may take more than
> `HALO_DAMAGE_CAP_PERCENT`% (40) of its target's max HP** — for *every* battler present, foes,
> allies and the holder alike. The holder alone pays for it: each move it uses costs
> `HALO_PP_TAX` (1) extra PP.

**It is an aura, not armour.** The cap keys off "is a Halo holder anywhere on the field", not
"is the target the holder", so it protects the holder's opponents too. This is what makes it
balanced enough to be interesting: the holder cannot use the cap to win a damage race, because
its own attacks are bounded by exactly the same ceiling.

**Why 40.** It is the same ceiling the fork already applies to (formerly one-hit-KO) OHKO moves
via `DETERMINISTIC_OHKO_MAX_HP_PERCENT`, so the number is not new to the engine. It also makes
the ability legible in one sentence: 40 × 2 < 100 but 40 × 3 > 100, so **a battler under a Halo
can never be knocked out in fewer than three hits**.

**Why the downside is PP.** A custom ability needs a real cost (see the design note at the top).
Two constraints picked this one:

- Clefable carries **innate Magic Guard**, so any cost expressed as recoil or chip damage would
  simply be erased. The cost had to be non-HP.
- This fork already prices two separate mechanics in PP — `DETERMINISTIC_PARALYSIS_PP_TAX` and
  the whole `DETERMINISTIC_ACCURACY_EVASION` PP economy — so PP is the fork's established
  currency for drawbacks rather than a bolted-on one.

At +1 PP the holder's effective PP is halved, which matters most where it should: a Gen-9
recovery move has 5 base PP (8 with PP Ups), so a Halo'd staller gets **4 heals, not 8**. The
cap stops it being burst down; the PP tax puts it on a clock.

**One hook each.** The clamp is the last step of `ApplyModifiersAfterDmgRoll()`
(`src/battle_util.c`), which both the live calc and the AI's simulation
(`AI_ApplyModifiersAfterDmgRoll`, `src/battle_ai_util.c`) funnel through — so the AI predicts
capped damage for free, exactly as Affinity gets correct AI type reads for free. The PP tax is
one clause in the existing `ppToDeduct` accumulator (`src/battle_move_resolution.c`), stacking
additively with Pressure and the paralysis tax. A switch-in popup + *"A halo appears above
{mon}!"* (`STRINGID_HALOAPPEARED`, `BattleScript_HaloActivates`) announces it.

**Deliberate exemptions — both are counterplay, not oversights:**

- **Fixed-damage moves** (Seismic Toss, Night Shade, Endeavor, Super Fang, Psywave) run through
  `DoFixedDamageMoveCalc()` and never reach the clamp.
- **The cap is per hit, not per turn**, because it lives in the per-hit calc. A multi-hit move
  (always 3 hits under `DETERMINISTIC_MOVE_RESULTS`, 5 with Skill Link / Loaded Dice) can take
  several caps' worth in one turn.

**Composition.** Gastro Acid / Neutralizing Gas put the aura out (the holder is resolved via
`GetBattlerAbility`); the cap is idempotent, so two holders on the field cannot stack it; and
the aura ends when the holder leaves the field, like Cloud Nine or Fairy Aura.

**Assignment: Mega Clefable and Wigglytuff.** Mega Clefable takes it on **all three**
`SPECIES_CLEFABLE_MEGA` slots (base Clefable is untouched and keeps its slot-1 Magic Bounce
override); **Wigglytuff** takes it on slot 0 alone, replacing a redundant Cute Charm.

The two are deliberately different shapes, and the contrast is the thing to understand. Mega
Clefable needs all three slots because of the transformation mechanic below; Wigglytuff has no
Mega, so one slot is enough and its Factory sets name Halo directly. That also makes Wigglytuff
the **first unconditional Halo** — Mega Clefable must win the one-gimmick-per-trainer slot to
get its aura, while Wigglytuff has it from turn one of every battle — and the first roster set
anywhere to actually select the ability. Weigh that before adding a third non-Mega holder.

This is worth understanding, because it is a pattern future abilities can reuse. Factory sets
are authored on the *base* species, so a Clefable set selects Magic Bounce (its only real,
non-innate slot — slots 0 and 2 are Cute Charm and Unaware, both innates, and a set may not
name an ability the species already has innately). On Mega Evolving, `abilityNum` is preserved
and re-resolved against the new species (`CopyMonAbilityAndTypesToBattleMon`), so whichever
slot the set picked lands on Halo — which is why **all three** Mega slots carry it. Nothing is
lost in the swap either: Magic Bounce is an *innate* on the Mega form, so a transformed
Clefable keeps it while gaining Halo as its chosen ability.

The trade-off is deliberate. On Clefable, Halo is **upside on transformation** rather than a
turn-one trait,
and under `FEATURE_FREE_GIMMICKS` a given mon often will not Mega (see the free-gimmicks note in
[`LINE_REVIEW.md`](LINE_REVIEW.md)). That is the cost of keeping the base form's identity
intact; in exchange the aura reads as something the creature *ascends* into. Note this is not
the same failure mode as an ability that is inert for a given moveset — when Halo does come up
it works regardless of what the set is holding or clicking.

Covered by `test/battle/ability/halo.c`.

**Halo is a shared ability, not a family** — it takes no parameter, so other gentle/"cute"
species (Togekiss, Comfey, Blissey, Audino, Sylveon, Alcremie) can simply be given the same
ability rather than a variant of it. Wigglytuff was the first such addition, via the Jigglypuff
line review.
