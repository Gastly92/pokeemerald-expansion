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
- **NO explanatory comments in the three data files.** They are data tables, not
  logic: a row carries only its values and the `// <dex>` marker the file already
  uses on every row. Do **not** annotate a row with why a build works, what an item
  or config flag does, how an ability interacts, or what a spread is hedging against
  — not above the row, not inline. This is settled convention, actively enforced
  (PRs #390-392 stripped exactly this kind of prose back out of all three files), so
  a comment added here is churn that a later PR deletes. The reasoning is still worth
  writing down — put it where prose belongs: `fork-docs/` for feature-level notes,
  the `#define`'s own comment in `include/config/*.h` for a flag's semantics, and the
  PR body for why *this* row was chosen. If a row seems to *need* a comment to be
  understood, that is a signal the explanation belongs in one of those places, not
  that this rule has an exception.
- **Cover the WHOLE line, every form.** Innates and frontier sets are keyed by
  exact species constant (`gBattleMons[].species` becomes the form constant after
  a Mega/G-Max/forme change — there is **no** base-species fallback). Grep all of
  `BULBASAUR|IVYSAUR|VENUSAUR` (incl. `_MEGA`, `_GMAX`, `_ALOLA`, …) in each file.
- **Present proposals before editing, then WAIT for a yes.** Flavor picks are
  subjective and the maintainer's call — for the first pass on a line, lay out the
  findings + a concrete proposal per file and get a yes/no (or swaps) before writing
  code. **A deferral is not an approval:** "let's do X first" leaves the proposal
  pending, not accepted. **An unanswered question is not a yes:** if you asked which
  of two picks to take and never got an answer, that pick is still open — don't
  resolve it for the maintainer and ship it. And "let's return to the line review"
  means resume the *review*, not build the backlog.
- **Expect to be wrong about flavor, and check before defending.** Most proposals in
  a first pass get rejected; that is the process working, not a failure. When a pick
  is challenged, go and verify (canon users, the repo's dex text, the engine) rather
  than arguing from memory — the evidence usually settles it in the maintainer's
  favor, and occasionally it will support the pick, which is worth saying plainly.
- **Two CI tests gate the ability data** — keep them green (see each step).

---

## Step 1 — Innates (`src/fork/innate_abilities.c`)

For each species/form in the line:

1. **Read the existing `INNATES(...)` row.** Does the set make flavorful sense
   for the creature? (Venusaur carrying Chlorophyll / Leaf Guard / Overgrow /
   Regenerator is on-theme grass/poison plant flavor — its own abilities plus a
   sun/plant guard and a plant's regrowth. Note two abilities that *look* on-type
   but read wrong here: Natural Cure is a nurturing-healer trait (Celebi/Chansey),
   not a plant's, and Poison Heal is dead weight on a Poison-type that can't be
   poisoned — flavor fit is about how an ability *reads on this creature*, not
   just sharing a type.)
2. **Aim for a GENEROUS set — but gate every pick on genuine flavor fit.** A
   base-stage line usually shouldn't sit at just its type ability (Blaze/Torrent);
   the fork is happy to stack several innates (Venusaur carries **four** —
   Chlorophyll, Leaf Guard, Overgrow, Regenerator — and Mega adds a fifth, Thick
   Fat). BUT "several" is not "any" — each pick must read as *this* creature.
   The test: measure the ability against how it reads on its **established
   users**. Intimidate belongs to menacing-presence bruisers (Gyarados, Salamence,
   Arcanine, Incineroar); handing it to a creature that *overpowers* rather than
   cows is a flavor miss even though it's a clean boon. Prefer picks that are
   canon (its own ability), Pokédex-supported (a trait the creature's dex flavor
   literally describes), or that belong to a same-typed relative (a shared-type
   neighbor's signature ability reads as in-family). Be honest about a narrow
   flavor space: some types have fewer clean fits than others ("plant" is roomy),
   so a flavor-honest line may stay short — don't pad to a number with reaches.

   **Count the canon users before proposing. It is cheap and it is the whole test.**
   (Walk `.abilities` across `src/data/pokemon/species_info/*.h` and group by
   ability.) The count tells you what kind of pick you are holding:
   - **1 user → it is a signature, not an ability.** Berserk belongs to Drampa
     alone; Dragon's Maw to Regidrago alone. Each is welded to one creature's
     specific design — Dragon's Maw is *named for* Regidrago being a giant maw — so
     giving it to another species is **inventing**, not borrowing. Reject on sight
     unless the species is that creature's close relative.
   - **Many users → read the family, then check the species actually belongs.**
     Moxie's 16 (Krookodile, Scrafty, Gyarados, Pinsir, Honchkrow, Heracross …) are
     uniformly swaggering predators, so it misreads on a creature whose dex says it
     "will never torch a weaker foe." Keen Eye's 41 are overwhelmingly birds and
     sharp-eyed watchers, so it misreads on Blastoise, whose accuracy is
     **ballistics, not eyesight**. Conversely the check can *support* a pick that
     looks wrong: Swift Swim's users include Carracosta, a heavy armored shelled
     turtle, so it is not the speedster-only ability it appears to be.

   **Derive the pick from the creature; never reverse-engineer it from mechanics.**
   The failure mode is choosing an ability because it is strong or synergises with
   the species' Factory set, then combing the dex for a sentence to justify it. A
   dex line that merely contains a matching *word* is not grounding: "the flame
   blazes when it is enraged" describes a mood indicator, not a rage power-up
   (Berserk); "its waterspouts are highly accurate" describes artillery, not vision
   (Keen Eye). If the honest reason for a pick is "its set leads with a recoil
   move," that is a mechanics-first pick — drop it.

   **Quote the repo's own dex text, never recalled flavor.** `.description` in
   `src/data/pokemon/species_info/*.h` is the source of truth, and it differs from
   the wider series (Charmander's "steam spouts when it rains" line is *not* in this
   repo, so a Magma Armor pitch resting on it has no support here). Read the field
   before citing it.
3. **Keep the line consistent by default, but differentiate by form when
   morphology/temperament justifies it.** The three base rows usually carry the
   *same* list — but they need not be identical: a wingless pre-evo shouldn't
   carry a flight ability its winged final stage earns, a placid pre-evo shouldn't
   carry a rage ability its vicious evolution earns. Escalate the list up the line
   where the creature changes.
4. **HARD CONSTRAINT — only allowlisted abilities.** An innate must be one whose
   behavior is actually *wired* at an effect site. The CI source of truth is
   `sImplementedInnates[]` in `test/fork/innate_abilities.c`, mirrored by the
   SCOPE list in `include/fork/innate_abilities.h`. **Naming an unwired ability
   fails the build.** A line review adds *already-implemented* innates to a
   species — it does **not** wire brand-new abilities (that's a separate, much
   larger task with its own doc updates).
5. **Forms:**
   - A form gets innates **only if it has its own row** — add/maintain rows for
     `_MEGA`, `_GMAX`, regional forms, etc.
   - **Megas are a pure boon:** mirror the base's list so the creature's traits
     persist, then add the Mega's own flavor (Mega Venusaur adds `THICK_FAT`, its
     canon ability). Tune a Mega's extras to how its Factory set actually plays
     (see the free-gimmicks note in Step 3): a Mega whose set leads with a recoil
     move wants `RECKLESS` *live*, not just as flavor; a special-attacking Mega
     wants a Sp. Atk booster, not a physical one.
     **When the Mega's own ability is itself an implemented innate** (e.g. Mega
     Venusaur's Thick Fat), adding it as an innate here means the Mega's single
     observable slot would waste on a duplicate — so pair this with an override on
     the Mega form that matches the base form's chosen ability (see Step 2, point 4).
   - **Watch grounded forms:** a form that shouldn't float must not inherit
     `LEVITATE` (see the Mega Gengar / Mega Mewtwo X notes in the file header).

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
3. **Don't override a base form to hand it its Mega's ability.** Under
   `FEATURE_FREE_GIMMICKS` a base Factory set Mega Evolves on its own (see the
   free-gimmicks note in Step 3) and *becomes* the Mega, gaining the Mega's real
   ability automatically. An override giving base Charizard `TOUGH_CLAWS` to feed
   a "Mega X" set is dead weight — the set is Mega X within a turn and has Tough
   Claws for real. Overrides earn their keep for mons whose chosen trait must be
   observable *without* transforming, not for pre-transform placeholders.
4. **DO override a MEGA form to match the base form when the Mega's own ability
   is already an innate.** Per Step 1, a Mega mirrors the base's innates and adds
   its own signature ability. When that signature is an *implemented* innate, the
   Mega already carries it as an always-on innate — so the Mega's single
   observable ability slot would otherwise default to that same ability, a wasted
   duplicate (exactly what Step 2's point 1 forbids). The fix: override **every
   real slot** of the Mega form to the **base form's chosen override ability**, so
   the base's observable trait persists through the transformation instead of
   collapsing into the now-innate Mega ability. Worked example — the whole
   Venusaur pattern: base Venusaur overrides to `GRASSY_SURGE`; Mega Venusaur adds
   `THICK_FAT` as an innate (Step 1) **and** overrides slots 0/1/2 to
   `GRASSY_SURGE`. A set authored on base Venusaur shows Grassy Surge, Megas turn
   one, and *still* shows Grassy Surge — now with Thick Fat live on top. (Contrast
   point 3: that's about not handing a *base* form its Mega's ability; this is
   handing the *Mega* the base's ability so the observable trait carries over.)
5. **Slot/dex ordering:** rows are sorted by National Dex number with a trailing
   `// <dex>` comment; forms share the base number and follow it.

**Verify:** `make check TESTS="Frontier extended roster"` — two tests enforce the
invariants ("every set's ability is legal for its species" + "no set's chosen
ability duplicates a species innate"). Note these only exercise slots a roster set
actually selects, so a new override row is best paired with a set that uses it.

---

## Step 3 — Frontier sets (`src/fork/frontier_extended_mons.c`)

The Battle Factory movesets have **no move-legality restrictions** — *any* move
is fair game, the only bar being that it reads as flavorful on the creature (a
signature move, a canon TM/tutor move, an on-type coverage move, a lore gimmick).
Sets need not be competitive. This is the most open-ended, creative step.

**Flavor is the ONLY test — do not gate a move on the learnset.** The bar is "does
this read as this creature," full stop. A move the species cannot learn in any game
is still fine if it fits: give a spider a web move, a psychic a mind move, a bruiser
a punch. You do **not** need to check `all_learnables.json`, the level-up tables, or
the teachable sets before proposing — and a move failing that check is **not** a
reason to drop it. (Consulting the learnset is a fine way to *find* flavorful ideas,
and citing it is fine supporting evidence for one; the failure mode is treating a
miss there as a veto, or spending the review's effort verifying legality it does not
have.) Contrast Step 1, where the canon-user count genuinely *is* the gate: innates
are constrained because an ability rewrites what a creature fundamentally is, while
a moveset is just four things it does today.

1. **Read the existing set(s) for the line.** Note what niche each fills so a new
   set adds variety rather than duplicating.
2. **Aim for ~4–5 sets per species**, each filling a distinct niche so the
   Factory has real variety to draw among — a signature-move set, a gimmick
   (Trick Room, weather, Baton Pass, status spreader), a lore set, a defensive
   staller, an offensive sweeper, etc. Fewer is fine when a species genuinely has
   a narrow kit; don't pad with near-duplicates.

   **Every set must be one you would actually want to draft.** The Factory draws
   among a species' sets, so a set that is strictly worse than its siblings has
   *negative* value — it dilutes the pool with a draw you would trade away. Filling
   "a different niche" is not sufficient on its own; the set has to be a genuinely
   competitive option against the others already there. Worked example: a physical
   Venusaur build fills an empty niche and is still wrong, because even with the
   Mega it swings 100 Atk against the special sets' 122 SpA — and without the Mega,
   82. Nobody would pick it, so it does not belong in the pool.

   **"No changes" is a legitimate result.** A species with four coherent,
   format-covering, base-form-viable sets is done. Adding a fifth to hit the range
   in this point is padding — the same reasoning as the "don't pad to a number"
   rule for innates in Step 1.
3. **Cover both battle formats across the line's sets.** Every set is tagged
   `FORMAT_SINGLES`, `FORMAT_DOUBLES`, or `FORMAT_BOTH` (see the `.tags` field in
   point 7), and the Factory
   draws from the pool matching the current format — so a line whose sets are all
   one format starves the other. Aim for a spread: some singles-only, some
   doubles-only, some that work in both. Let the *format* shape the set — doubles
   sets can lean on partner-facing tools (Rage Powder / Follow Me redirection,
   Helping Hand, Fake Out, spread moves, Trick Room support), while singles sets
   want self-sufficient sweeping/stalling. A set is `FORMAT_BOTH` only when it
   genuinely holds up in each; don't tag a doubles-support set `FORMAT_BOTH`.
4. **Consider the held item, but as one lens among several — not the main
   focus.** A good technique is to iterate through items and ask what would be
   *fun* on this creature: an item can define a set (Choice Specs sweeper, a
   pinch-Berry survivor, a weather-rock setter, a Toxic Orb + Poison Heal staller)
   and the four Venusaur sets show that (each pivots on a different item — Black
   Sludge, Leftovers, Rocky Helmet, Life Orb). But the item is a springboard, not
   a requirement: plenty of items are weak or pointless and aren't worth building
   around, and a set can just as well start from a move, an ability, or a gimmick
   with `ITEM_LEFTOVERS` (or nothing special) attached. Don't force a themed item
   onto every set.
5. **Account for this fork's mechanics when picking moves and items** — they
   change what's good in ways stock knowledge misses:
   - **`DETERMINISTIC_*` flags** (`include/config/deterministic.h`) strip RNG, so
     chance-based moves become *reliable*: Sleep Powder / Spore always land for a
     fixed `DETERMINISTIC_SLEEP_TURNS` sleep, `DETERMINISTIC_ACCURACY_EVASION`
     makes low-accuracy moves (Hydro Pump, Focus Blast, Stone Edge) always hit,
     `DETERMINISTIC_ADDITIONAL_EFFECTS` / `DETERMINISTIC_FLINCH` make secondary
     burns/paralysis/flinches (Scald, Air Slash, Iron Head) fire every time, and
     `DETERMINISTIC_CRITICAL_HITS` turns Focus Energy / Super Luck into guaranteed
     crits. `DETERMINISTIC_DAMAGE` ramps damage upward each turn, rewarding sets
     that survive to snowball. Lean into these — a "risky" move here is a sure one.
   - **`BUFF_*` item/mechanic improvements** (`include/config/buff.h`): Shell Bell
     heals 1/4 of damage dealt (up from 1/8), and Leech Seed stacks across seeders
     and re-drains instead of failing — both make those build-arounds far stronger
     than vanilla, so a Shell Bell bruiser or a Leech Seed staller is a live plan.
6. **Propose new sets** that are fun or flavorful and cover the niches above;
   Multiple sets per species are fine; the Factory draws among them.
7. **Fields of `struct TrainerMon`** (authoring helpers in
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
8. **Keep dex order** (rows are grouped by generation with `// <dex>` markers).

### Free gimmicks — a set *may* Mega, but is NOT guaranteed to (`FEATURE_FREE_GIMMICKS`)

This fork drops the held-item requirement for battle transformations, so **any
eligible Factory set _may_ Mega Evolve (or Dynamax / Tera) with no stone**, turn one,
via the gimmick picker.

**But it is still one gimmick per trainer per battle.** `HasTrainerUsedGimmick()`
(`src/battle_gimmick.c`) gates the trigger on a per-trainer `activated[]` flag, and the
fork adds a per-mon `monGimmickUsed` record on top. Dropping the stone requirement means
*every* mon on the drafted team is eligible — so as `src/fork/frontier_draft.c` puts it,
"a team is no longer limited to one." They **compete** for that single slot, and a given
mon Megas only when it wins that competition. Often it will not.

> **Therefore: build the set for the BASE form, and treat the Mega as upside.**
> This is the easiest rule in Step 3 to get backwards, and getting it backwards
> produces sets that are dead weight most of the time. A build that only makes sense
> after transforming is a bad build, because most battles it never transforms. Check
> every set against the base stat line first: a physical Venusaur set reads fine off
> Mega's 100 Atk and is *unplayable* off base Venusaur's 82.
>
> A corollary: **do not "fix" several of a species' sets sharing one spread.** Three
> Venusaur sets running Bold 252 HP / 252 Def is a deliberate hedge — that bulk is
> live whether or not the Mega arrives — not a failure of imagination.

Three further consequences a line review must account for:

1. **A "Mega" set is authored on the *base* species and transforms in battle.** The
   roster's Charizard sets are `SPECIES_CHARIZARD` (not `_MEGA_X/_Y`) holding a real
   competitive item (Life Orb, Heat Rock, Choice Specs), and they *may* become the Mega
   in the first turn. So don't "fix" a base set that looks like it lacks its Mega's
   tools — check what it Megas into first.
2. **For a multi-Mega species, the form is chosen by Attack vs Sp. Atk** (physical →
   **X**, special → **Y**, tie → **X**; see `test/fork/free_gimmicks.c`). The set's
   EV spread and nature therefore *steer* which Mega it becomes — a `spa`/Timid set
   lands Mega Y, an `atk`/Jolly set lands Mega X. A Charizard "sun" set needs no
   Sunny Day: as a special build it becomes Mega Y, whose **Drought** sets the sun
   its Heat Rock then extends. Build the spread to match the intended form.
3. **The Mega form's rows are what's live in battle after evolution.** Post-Mega the
   mon's `gBattleMons[].species` is the `_MEGA_*` constant, so its **innate row and
   ability come from that form**, not the base — which is why the Mega X / Mega Y
   innate rows (Step 1) matter more than the base row for Factory play, and why the
   base `.ability` field is a pre-transform placeholder (largely cosmetic once it
   Megas turn one). Tune the Mega's innates to how its steered set actually plays.

**Verify:** the same `"Frontier extended roster"` tests (legality + non-innate).

---

## Wrap-up (after applying approved changes)

- **Record what you rejected, and why.** Much of a line review's value is in the
  candidates considered and dropped — undocumented, they get re-proposed on the next
  pass. Put them in the PR body; if a pick was rejected on a durable *structural*
  ground rather than taste (e.g. a transformation ability, which is out of scope for
  innates entirely — see the "Identity / form / type-transform" bucket in
  `INNATE_ABILITIES.md`), record it in the relevant doc instead so it sticks.
- **Note the save-index cost when adding frontier sets.** Sets are inserted at the
  species' dex position, and saved rentals reference entries by array *index*, so any
  insertion invalidates an in-progress rented team in an existing save. Inherent to
  keeping dex order — surface it to the maintainer rather than burying it.
- **Update `fork-docs/FORK.md`** if the change is worth indexing (usually the
  innate/override/roster features already have rows; a per-line data tweak rarely
  needs a new row, but note anything with a known limitation).
- **Build both flag states** for any new/changed config-gated code, per
  `CLAUDE.md` (Building & testing). Data-only additions to these three files are
  covered by the roster/innate tests; run those filtered, and let CI run the full
  suite on the PR.
- **One line per branch/PR** unless the maintainer says otherwise.
