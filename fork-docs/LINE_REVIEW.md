# Line review — the per-line updates & enhancements playbook

A **line review** is a repeatable pass over one Pokémon evolutionary *line*
(e.g. "the Venusaur line" = Bulbasaur → Ivysaur → Venusaur, plus every form:
Mega, G-Max, regional) that looks at the three fork-owned data files that give a
species its identity in this romhack and proposes **updates and enhancements**:

1. `src/fork/innate_abilities.c` — the always-on innate abilities.
2. `src/fork/species_ability_overrides.c` — the single chosen (observable) ability.
3. `src/fork/frontier_extended_mons.c` — the Battle Factory sets (held item, moves, spread).

The goal is flavor and fun first; competitive strength is welcome but **not**
required. This doc is the rubric; the `line-review` skill is the front door that
runs it. Invoke it by saying e.g. *"let's look at the Venusaur line for updates
and enhancements."*

---

## Golden rules (read before touching anything)

- **These are upstream-synced feature files, but they're fork-owned** — edits
  here never conflict on sync. Still keep rows additive and in dex order.
- **Cover the WHOLE line, every form.** Innates and frontier sets are keyed by
  exact species constant (`gBattleMons[].species` becomes the form constant after
  a Mega/G-Max/forme change — there is **no** base-species fallback). Grep all of
  `BULBASAUR|IVYSAUR|VENUSAUR` (incl. `_MEGA`, `_GMAX`, `_ALOLA`, …) in each file.
- **Present proposals before editing.** Flavor picks are subjective and the
  maintainer's call — for the first pass on a line, lay out the findings + a
  concrete proposal per file and get a yes/no (or swaps) before writing code.
- **Two CI tests gate the ability data** — keep them green (see each step).

---

## Step 1 — Innates (`src/fork/innate_abilities.c`)

For each species/form in the line:

1. **Read the existing `INNATES(...)` row.** Does the set make flavorful sense
   for the creature? (Venusaur carrying Chlorophyll / Overgrow / Harvest / Leaf
   Guard / Poison Heal is on-theme grass/poison plant flavor.)
2. **Consider additions.** Any ability that fits the creature's biology/lore and
   is a clean boon is a candidate.
3. **HARD CONSTRAINT — only allowlisted abilities.** An innate must be one whose
   behavior is actually *wired* at an effect site. The CI source of truth is
   `sImplementedInnates[]` in `test/fork/innate_abilities.c`, mirrored by the
   SCOPE list in `include/fork/innate_abilities.h`. **Naming an unwired ability
   fails the build.** A line review adds *already-implemented* innates to a
   species — it does **not** wire brand-new abilities (that's a separate, much
   larger task with its own doc updates).
4. **Forms:**
   - A form gets innates **only if it has its own row** — add/maintain rows for
     `_MEGA`, `_GMAX`, regional forms, etc.
   - **Megas are a pure boon:** mirror the base's list so the creature's traits
     persist, then add the Mega's own flavor (Mega Venusaur adds `THICK_FAT`).
   - **Watch grounded forms:** a form that shouldn't float must not inherit
     `LEVITATE` (see the Mega Gengar / Mega Mewtwo X notes in the file header).
5. **Keep the line internally consistent** unless a form justifies divergence —
   the three base-stage rows of a line usually carry the *same* innate list.

**Verify:** `make check TESTS="Innate"` (the innate test rejects unwired picks).

---

## Step 2 — Override (`src/fork/species_ability_overrides.c`)

Each row replaces one ability *slot* of a species with a real, selectable ability
so a frontier set can run a genuine second trait alongside the innates. For the
line:

1. **Read the existing override row(s), if any.** Confirm:
   - **The chosen ability is NOT one of the species' innates** (otherwise the
     one observable pick is a wasted duplicate — the whole point of the row).
   - **It's a STABLE pick:** either `:x:` in `fork-docs/INNATE_ABILITIES_PROGRESS.md`
     (never going to become an innate — e.g. Lightning Rod, Water Absorb, Sheer
     Force, Grassy Surge) **or** an already-`:white_check_mark:`-implemented innate
     the species does **not** itself carry. Avoid a `:white_large_square:` pending
     ability — it becomes future churn the moment it's wired as an innate.
   - **The freed slot is safe to repurpose:** filling an *empty* (`ABILITY_NONE`)
     slot is always safe. Repurposing a *real* slot deletes that ability from the
     species game-wide — only do it when the slot is redundant (that ability is
     now innate) **and** not pinned by an upstream test. Audit `Ability(ABILITY_X)`
     uses in `test/battle/` for that species before repurposing a real slot.
2. **Consider adding a row** if the line lacks one and a species' chosen slot is a
   redundant innate (or empty) — pick a stable, flavorful non-innate ability.
3. **Slot/dex ordering:** rows are sorted by National Dex number with a trailing
   `// <dex>` comment; forms share the base number and follow it.

**Verify:** `make check TESTS="Frontier extended roster"` — two tests enforce the
invariants ("every set's ability is legal for its species" + "no set's chosen
ability duplicates a species innate"). Note these only exercise slots a roster set
actually selects, so a new override row is best paired with a set that uses it.

---

## Step 3 — Frontier sets (`src/fork/frontier_extended_mons.c`)

The Battle Factory movesets have **no restrictions** — any move (flavor, gimmick,
or power) is fair game, and sets need not be competitive. This is the most
open-ended, creative step.

1. **Read the existing set(s) for the line.** Note what niche each fills so a new
   set adds variety rather than duplicating.
2. **Propose new sets** that are fun or flavorful — a signature-move set, a
   gimmick (Trick Room, weather, Baton Pass, status spreader), a lore set, etc.
   Multiple sets per species are fine; the Factory draws among them.
3. **Fields of `struct TrainerMon`** (authoring helpers in
   `include/fork/frontier_extended_mons.h`):
   - `.species` — the exact species/form constant.
   - `.tags` — **required**: `FORMAT_SINGLES`, `FORMAT_DOUBLES`, or `FORMAT_BOTH`
     (0/unset stalls the selector). Keep enough of each format across the roster.
   - `.heldItem` — any item.
   - `.moves` — up to 4; **no legality restriction**.
   - `.ability` — the chosen ability. Must resolve to a real slot for the species
     (via `GetSpeciesAbility`, i.e. through the override table) **and** must not be
     an innate — or set `ABILITY_NONE` to let the Factory pick. Both roster tests
     check this.
   - `.nature` — `NATURE(DEF_UP, ATK_DOWN)` style (boosted, lowered).
   - `.ev` — `EVS(.hp = 252, .def = 252, .spd = 4)` (names: hp/atk/def/spa/spd/spe).
   - `.teraType` — optional Tera type.
   - Optional: `.dynamaxLevel`, `.gender`, `.isShiny`, etc. (gmax mons get the
     Gigantamax Factor + max Dynamax Level automatically at draft).
4. **Keep dex order** (rows are grouped by generation with `// <dex>` markers).

**Verify:** the same `"Frontier extended roster"` tests (legality + non-innate).

---

## Wrap-up (after applying approved changes)

- **Update `fork-docs/FORK.md`** if the change is worth indexing (usually the
  innate/override/roster features already have rows; a per-line data tweak rarely
  needs a new row, but note anything with a known limitation).
- **Build both flag states** for any new/changed config-gated code, per
  `CLAUDE.md` (Building & testing). Data-only additions to these three files are
  covered by the roster/innate tests; run those filtered, and let CI run the full
  suite on the PR.
- **One line per branch/PR** unless the maintainer says otherwise.
