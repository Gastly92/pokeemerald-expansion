---
name: line-review
description: Review and enhance Pokémon evolutionary lines' fork data — innates, ability overrides, and Battle Factory movesets — one line, or a batch of lines given as a dex-number range or a whole generation. Triggers when the user asks to look at / review / update / enhance a species "line" (e.g. "let's look at the Venusaur line for updates and enhancements", "review the Gengar line", "any additions for the Dragonite line?") or a range of them ("let's review the lines from number 0 to 25", "lines 26-50", "the next ten lines", "all of Gen 5"). For this fork's three fork-owned data files only.
---

# Line review

Run a structured updates-and-enhancements pass over one Pokémon evolutionary
**line** (base + all evolutions + all forms: Mega, G-Max, regional) across the
three fork-owned data files. Flavor and fun come first; competitive power is
welcome but not required.

A request can name **one line** ("review the Gengar line") or a **batch** — a
range of National Dex numbers ("the lines from number 0 to 25") or a whole
generation ("all of Gen 5"), which is the default batch size. A batch is the same
review run once per line, packaged as one branch, commits grouped by line and one
PR; see "Batch mode" below and the rubric's "Batch reviews" section.

Gens I-IX have each had a complete pass (batches 1-16). A batch is now always a
*re-review*: judge the rows that are there today, and expect the finds to be the
recurring defect classes below rather than empty rows.

## Do this

1. **Read the rubric:** `fork-docs/LINE_REVIEW.md` is the source of truth for the
   criteria and constraints. Follow it.
2. **Scope the whole line, every form.** Grep all species constants for the line
   (base + evolutions + `_MEGA` / `_GMAX` / `_ALOLA` / etc.) in each of:
   - `src/fork/innate_abilities.c` — always-on innates (Step 1).
   - `src/fork/species_ability_overrides.c` — the chosen ability (Step 2).
   - `src/fork/frontier_extended_mons.c` — Battle Factory sets (Step 3).
3. **Run the three steps ONE AT A TIME, in order, without stopping for approval.**
   Innates → overrides → frontier sets (Part A audit → Part B new sets) → apply →
   verify → PR. Settle each step before starting the next: a step builds only on
   decisions already made, never on one still in flux — an override is justified
   against the innates in this change, and a set's `.ability` may only name a real
   slot or an override row settled in Step 2. Work them out of order and the whole
   line has to be re-derived.
4. **For each step, per the rubric:** report what's already there, whether it makes
   flavorful sense — citing both the repo `.description` **and** a wider-media
   check — and the concrete additions/changes you're making. **The media note is a
   required field in every per-line PR section**, with *"no usable media evidence"*
   as a legal value. The rubric has said "silence counts as skipping it" for the
   whole sweep and the pass was still skipped on most lines of Gens 7-9; the field
   exists because an omission otherwise leaves no trace. Flagging recall is not a
   liability — you are proposing something the maintainer can check, not asserting
   a fact the repo confirms.
5. **Then apply the edits, verify, and open a PR — the PR is where approval
   happens.** Flavor picks are still the maintainer's call; they make it on the PR
   after seeing the whole line. So the PR body must carry the per-step reasoning,
   the flavor evidence (with recall flagged as recall), the Part A verdicts
   including "keep as-is", what you rejected and why, and any coin-flip you
   resolved yourself. Expect changes to be requested — that's the process working.
   Apply review feedback on the **same branch**, re-deriving forward through the
   steps when a rejected innate invalidates a later pick.

## Batch mode (a dex-number range, or a whole generation)

When the request names a range instead of a line — *"the lines from number 0 to
25"* — the numbers are **National Dex numbers**, inclusive of both endpoints
(`0` is shorthand for the start; there is no #0). A request for **a whole
generation** (*"let's do all of Gen 5"*) is the same thing and is the default
batch size: I #1-151, II #152-251, III #252-386, IV #387-493, V #494-649,
VI #650-721, VII #722-809, VIII #810-905, IX #906-1025. A generation drags in
earlier-gen lines through cross-evolutions (Gen 9 pulled seven, Gen 8 six), and
those are part of the batch. Line counts run well below number counts — Gen 7's 88
numbers were 55 lines, Gen 9's 120 were 78.

Everything in "Do this" still applies **per line**, at the same depth.

0. **Resolve every species name to an id before believing anything about it** —
   `python3 .claude/skills/line-review/tools/forms.py <NAME>`, run from the repo
   root. That directory also holds `lines.py` (the lines a dex range covers),
   `line.py` (a species' rows and sets), `dex.py`, `users.py` (canon ability
   users), and the `inn.py`/`addset.py` editors; each carries its usage in a
   docstring.
   Rows and sets are routinely keyed on a form constant a bare name aliases to
   (`SPECIES_AEGISLASH` → `_SHIELD`, `SPECIES_MINIOR` → `_METEOR_RED` in two hops)
   and the reverse (`SPECIES_ALCREMIE` is `_STRAWBERRY_VANILLA_CREAM`). Grep
   `include/constants/species.h` for the name and check every constant sharing its
   id or prefix **before** concluding a row is missing. This is the sweep's most
   expensive recurring bug: four duplicate rows in Gen 6, one in Gen 7, a row for
   the non-existent `SPECIES_TOXTRICITY_GMAX` in Gen 8 that would have failed the
   build, and seven false "no row" alarms in Gen 9.
1. **Resolve the range to a list of lines first.** Derive it from the repo
   (`.natDexNum` and `.evolutions` in `src/data/pokemon/species_info/*.h`, unioning
   species that share a dex number). `lines.py` **now checks its own output for
   holes** and exits non-zero listing any number in the range that belongs to no
   line — if it prints `UNCOVERED`, a species is being parsed out of existence and
   the batch does not start until that is empty. (It used to be the reader's job:
   Gen 9 dropped Ogerpon that way, Gen 2 dropped Unown, and adding the check
   caught Mothim, Arceus and Minior too.) **Both endpoints are
   inclusive** — `0 to 25` includes #25. Walk each number, map it to the line that
   species belongs to, and dedupe *within this batch*. The **whole line comes
   along even where members fall outside the range** (#25 Pikachu pulls in Pichu
   #172, Raichu #26, Alolan Raichu, G-Max), and a number landing mid-line (#20
   Raticate) still pulls in the whole line.
2. **Review every line the range covers, and review it FRESH.** In range means
   reviewed, full stop — a previous review is neither a skip nor an anchor. Don't
   go digging up the old PR for what was decided or turned down; judge the rows
   in the three files **today**, on evidence gathered today. A candidate an
   earlier pass proposed and the maintainer rejected is **fair to raise again** —
   new information arrives, the rubric moves, minds change, and a re-raise costs
   one line in the PR body. Only **structural** rejections still bind (the ones
   written into `fork-docs/` and enforced by CI), which is why they live there.
   Fresh isn't restless, though: a change needs a reason of its own, so "no
   changes" stays a legitimate per-line result. State the resolved list back,
   ascending dex, then start — a range is an instruction, not a proposal, so don't
   wait for a yes.
3. **One line at a time, start to finish, then commit.** Steps 1 → 2 → 3 for a
   line, then its commit, then the next. Never batch a step across lines ("all the
   innates first") — the step coupling is per line.
4. **Commit and push after every line**, one commit per line, subject
   `Line review: <Line> line — <what changed>`. The container is ephemeral: a lost
   session should cost one line, not the batch. On a long batch, adjacent lines
   whose changes land in the same files for the same reason may share one commit
   naming the range, with a body enumerating each line. The **per-line section in
   the PR body is not optional** either way: commits may group, the review record
   may not.
5. **Watch for cross-line collisions** the batch makes visible — several lines
   converging on the same held item (item scarcity is per drafted team) or the
   same borrowed ability. Spread them.
6. **Verify once at the end**, not per line — the two filtered `make check` runs
   are a build each, and CI runs the full suite on the PR. Running them and fixing
   whatever they name is the batch's real completion criterion; see "Verify and
   ship" below for what the five gates cover and why they always find something.
7. **One branch `claude/lines-<first>-<last>-review`, one PR** for the batch, with
   **a section per line** in the body carrying everything a single-line PR body
   would (per-step reasoning, flavor evidence with recall flagged, Part A verdicts
   including *keep as-is*, rejections, open questions). Lines that came out **no
   changes** still get a section saying what was checked. If the batch can't be
   finished, open the PR with what's done plus a **remaining** list.

## Hard constraints (see the rubric for detail)

- **NO explanatory comments in the three data files.** They are data tables: a row
  carries its values and the `// <dex>` marker the file already uses, nothing else.
  Never annotate a row with why a build works, what an item/flag does, or how an
  ability interacts. Settled convention, actively enforced — prose added here gets
  stripped by a later PR. Put the reasoning in `fork-docs/`, on the config
  `#define`, or in the PR body.
- **Innates:** only *already-implemented* abilities (the `sImplementedInnates[]`
  allowlist in `test/fork/innate_abilities.c`) — an unwired pick fails CI. Don't
  wire new abilities in a line review. **A species with no row at all is a bug**,
  and it happens for a structural reason: when every one of a species' canon
  abilities is never-an-innate there is nothing to seed a row from, so it gets
  skipped (Kecleon, the Manectric line, the whole weather trio). Build the row
  from the rest of the dex entry instead.
- **Flavor test — count the canon users first** (group `.abilities` across
  `src/data/pokemon/species_info/*.h`). **One user = a signature**, welded to that
  creature's design; giving it away is inventing, not borrowing (Berserk→Drampa,
  Dragon's Maw→Regidrago). **Many users = read the family** and check the species
  belongs in it (Moxie's 16 are all swaggering predators; Keen Eye's 41 are all
  birds and sharp-eyed watchers). Never reverse-engineer flavor from mechanics — a
  dex line sharing a *word* isn't grounding ("blazes when enraged" is a mood
  indicator, not Berserk). Quote `.description` from the repo, never recalled flavor.
- **Check wider media before proposing — anime, movies, manga, spin-offs — and say so
  explicitly, including when the answer is "nothing usable."** A required input
  alongside the dex text, not a permission you may decline; silence reads as having
  skipped it. The dex-text rule above is about not passing recalled flavor off as the
  repo's `.description`; it does not make the games the only source. A four-line dex
  often supports nothing, while what a creature is shown *doing* on screen is sharper
  evidence (Ninetales' anime episode is entirely about it sealing a mansion so nobody
  can leave, plus the tail-curse — a far better case for Shadow Tag and Cursed Body
  than its dex line). Two conditions: **name the medium and flag that it is recall**
  the repo can't confirm, and **cite a specific action, not a vibe**. Flagging recall
  is the price of using media evidence, not a reason to avoid it — citing media only
  to *reject* a candidate is the tell that the honesty requirement has turned into an
  incentive to omit. The canon-user count is still the gate — media evidence picks
  *which* ability fits, and never licenses a 1-user signature.
- **Overrides:** the chosen ability must be a **never-an-innate** pick — one
  **absent from `sImplementedInnates[]`** (`test/fork/innate_abilities.c`, the
  single source of truth: on it = implemented innate, off it = never-an-innate).
  An innate-capable ability is **not** legal even when this species doesn't carry
  it — if an ability *can* be an innate it is given as an innate, and the one
  observable slot is spent on something that can only ever be observable. The
  legacy backlog is cleared, so `TEST("Innate abilities: no ability override or
  frontier set names an innate-capable ability")` is a **real CI gate** — a
  non-conforming pick fails the build. Wiring a new innate can still invalidate an
  existing row, so treat converting the line you touch as in-scope for Step 2. See
  `fork-docs/INNATE_ABILITIES.md` ("Direction"). Separately, an ability on
  `sReservedAbilities[]` (Illusion, welded to Zorua/Zoroark) is banned outright even
  though it *is* never-an-innate, under its own gate `TEST("Innate abilities: no
  ability override or frontier set names a reserved ability")`. Only repurpose a
  *real* slot that's redundant and not test-pinned; filling an empty slot is always
  safe.
- **Pre-evolutions get no override rows and no frontier sets.** An override exists
  to be selected by a set, and a pre-evo set is drafted against fully-evolved mons
  and loses — so both are dead data on a Vulpix or an Ivysaur. Spend them on the
  final stage plus any regional/Mega form that is its own final stage. The roster
  agrees: of 642 species with sets, the only non-final stages are **Chansey,
  Porygon2 and Dusclops, all on Eviolite** — that niche is the whole exception, so
  if you can't name what a pre-evo beats its evolution at, it gets nothing.
  **Innates are the opposite and still cover the whole line** — an innate is
  always-on identity a pre-evo carries whether or not anything drafts it.
- **Megas whose ability is an innate:** when a Mega's own ability is an
  implemented innate, give the Mega that ability as an *innate* (Step 1) and
  override *every* real slot of the Mega form to the **base form's** chosen
  override ability, so the base's observable trait carries through the
  transformation (the Venusaur pattern: base → Grassy Surge override; Mega →
  Thick Fat innate + Grassy Surge override). See the rubric Step 2, point 4.
- **Two set shapes are CI-gated, so check them in Part A.** A
  `TARGET_FOES_AND_ALLY` move (Earthquake, **Surf**, Sludge Wave, Discharge, Lava
  Plume, Boomburst …) on a `FORMAT_DOUBLES`/`FORMAT_BOTH` set damages the holder's
  own partner — swap for the single-target twin (Earthquake → High Horsepower,
  Surf → Muddy Water, or Earth Power on a special set); Explosion and its two
  siblings are exempt. And a Choice item never coexists with a status move except
  Trick, Switcheroo or Transform.
- **Five set defects the full sweep kept finding — check all five explicitly.**
  None is a matter of taste, and four are invisible field-by-field, appearing only
  when the set is read as a whole. **(1), (2) and (4) are now CI gates**, so they fail
  the build rather than needing to be spotted: (1) **a move on the wrong stat** (Celesteela's
  Heavy Slam on a 0-Attack spread, Pheromosa's Ice Beam off 4 Sp. Atk, Melmetal's
  4 Attack EVs) — read the nature and spread first, then every move's category
  against it; (2) **an item that can never trigger** (Popplio's Throat Spray with
  no sound move); (3) **an ability that can never fire** (Flapple's Hustle on a
  special set) — sometimes forced, but then say so in the PR; (4) **two sets that
  are one set**, same moves/spread/ability differing only by item, which wastes a
  Factory draw (Inteleon, Barraskewda, Polteageist, Kingambit); (5) **an untyped
  Hidden Power**, which is a 60 BP *Normal* move.
- **A line with no innate row has a signature** — its only canon abilities are all
  never-an-innate, so there was nothing to seed from. That is structural, not
  careless, and it means the row must be built from flavor. When there is nothing
  to seed from, ask **what the species is a version of**: Paradox mons echo a
  species, regional forms and Megas have a base, fusions take from both halves.
  Trim rather than transplant.
- **Restoration is the highest-yield check and is pure mechanism**: compare a row
  against its **pre-evolution's**, and put back what canon takes away on evolution.
  Over forty lines changed on this alone. Write a restored innate on **every form**,
  or it vanishes when the creature transforms.
- **Frontier sets run in two parts: Part A audits the EXISTING sets, Part B
  writes new ones** — Part A first, finished before Part B starts. Part A walks each existing
  set field by field: **Tera type** (what is it *for*? — a tactical immunity beats
  doubling an existing type, and it must match the item/ability/moves, because
  Terastallizing overwrites all three type slots and e.g. flips Black Sludge from
  healing to chip damage on a non-Poison Tera — but **not** STAB, which takes its
  own `GetTeraMultiplier` path off the base types and is never lost),
  **moves** (each earning its slot),
  **item** (still doing anything under `BUFF_*` / `DETERMINISTIC_HOLD_EFFECTS`),
  **nature/EVs/IVs** (spread matches what the set does; note
  `TRAINER_PARTY_IVS` takes **Speed 4th**, unlike `EVS()`'s named fields),
  **ability**, **format tag**, and **base-form viability**. "Keep as-is" is a fine
  verdict. See the rubric's Step 3 Part A for the full checklist.
- **Frontier movesets:** no move-legality restrictions — any move that's
  *flavorful* (or powerful) is fair game. **Flavor is the only test: do NOT gate a
  move on the learnset.** A move the species cannot learn in any game is still fine
  if it reads as this creature. Don't check `all_learnables.json` / level-up /
  teachable sets to validate a pick, and never drop a flavorful move because it
  isn't learnable. (Browsing the learnset for *ideas* is fine — treating a miss as a
  veto is not.) Contrast the innate rule above, where the canon-user count really is
  the gate. **The gate usually bites at generation, not veto:** you never open a
  learnset, you just never *think* of the moves outside it. Generate from the dex
  text and the design — "what does this creature DO?" — then find moves to match,
  freely including **other species' signature moves**. If you're about to conclude a
  species "has a narrow kit," that is the gate talking; every move in the game is
  available to every set.
  Aim for **at least 2 quality sets per species that gets sets at all** (see the
  pre-evolution rule above), each filling a different niche — a bar, not a quota. More is welcome when each set earns its place; two excellent
  sets beat five where three are filler. **Cover both formats** across the line's sets
  (`FORMAT_SINGLES` / `FORMAT_DOUBLES` / `FORMAT_BOTH`) — doubles sets can lean on
  redirection/support (Rage Powder, Follow Me, Helping Hand, Fake Out, spread
  moves), singles sets want self-sufficiency. Held items are **one lens among
  several** (iterate items for fun ideas, but many items are weak and not worth
  building around — a set can start from a move, ability, or gimmick just as
  well). **An item is also a scarcity cost:** only one of each item can appear per
  drafted team (`src/battle_frontier.c` rejects a duplicate `heldItem`), so a set
  on a crowded item is drafted less often — Leftovers is 21% of the roster and Life
  Orb 17%. Take a crowded item when the set genuinely builds around it; when you're
  reaching for Leftovers/Life Orb as a *default*, prefer a near-equivalent from the
  long tail so the set actually shows up. Account for the fork's `DETERMINISTIC_*` flags and `BUFF_*` item
  improvements (Shell Bell, Leech Seed) when choosing moves and items —
  **deterministic does NOT mean "always happens"**: secondary effects land only on
  a super-effective hit (or STAB for Normal moves), crit *stages* stay dead while
  crit *items* give one guaranteed crit, and paralysis loses full-para and the
  Speed drop entirely. **These flags CONVERT probabilistic mechanics into deterministic
  ones; they rarely delete them** — before concluding anything is dead under one,
  go find what it was rebuilt into (Blunder Policy rearmed onto Protect and
  immunities, Focus Band into a Sash, Quick Claw into a consumed one-shot). In
  particular accuracy/evasion became a **PP economy**, not a nullity: moves hit,
  but max PP is scaled by base accuracy and stages shift per-use cost, so No
  Guard, Keen Eye, Compound Eyes and evasion items are all still live.
  Read the "`DETERMINISTIC_*` regime" section of the rubric —
  and `include/config/deterministic.h` itself, which is the source of truth —
  before building a set around any mechanic. The set's `.ability` must resolve to a real
  slot and not be an innate of **that** species — the duplicate-innate tests are scoped
  per species, so borrowing an implemented innate another species carries is intended,
  not a loophole. **`ABILITY_NONE` is banned** by a fourth roster test; free a slot with
  a fork override instead.
- **Smogon is a legitimate input for EVALUATION, never for generation.** Competitive
  analysis sharpens both parts of Step 3 — a second opinion on whether a move earns
  its slot, whether a spread reaches a benchmark, whether a set is one you'd draft.
  Three limits: it is **learnset-constrained**, so building *from* a Smogon set
  re-imports the very gate the moveset rule above removes (generate from the creature
  first, then let Smogon critique it — never let it bound the move pool); this fork's
  **mechanics aren't vanilla**, so re-check any claim against `deterministic.h` /
  `buff.h` (Blizzard is 4 PP here, secondaries need a super-effective hit, paralysis
  is weaker, high-crit moves always crit through the gate); and the **format differs**
  (Factory drafting, item scarcity, no teambuilding or team preview, AI opponents).
  **Flavor still governs** — a Smogon-optimal set that reads as nothing is a worse
  outcome than a flavorful one that gives up power. Smogon breaks ties and catches
  mistakes; it doesn't pick the concept. Flag it as recall (or fetch it) — it's not
  in the repo.
- **Build sets for the BASE form; the Mega is upside.** Gimmicks are **once per
  trainer per battle** (`HasTrainerUsedGimmick`), and free gimmicks make the whole
  drafted team eligible, so they compete for one slot — a given mon often won't
  transform. A build that only works post-Mega is dead weight most battles. Corollary:
  several of a species' sets sharing one spread is usually a deliberate hedge, not
  monotony — don't "fix" it.
- **Every set must be one you'd actually draft.** The Factory draws among a species'
  sets, so a strictly-worse set is *negative* value. Filling an empty niche isn't
  enough. **"No changes" is a legitimate result** — a species already at two or more
  coherent sets is done unless a new one is genuinely as good; don't pad to a number.

## Verify and ship

After applying changes: `make check TESTS="Frontier extended roster"` and
`make check TESTS="Innate"` (filtered — let CI run the full suite on the PR).
**Expect these to find something.** Every run of this step across the whole sweep
caught a defect the per-line pass had walked past — 55 ally-hitting spread moves
the first time, and three or four items in each of the last four batches. Five
gates cover the whole dex unconditionally (they were ratchets bounded by a
reviewed-through-dex constant until the sweep finished; both constants are gone,
so there is nothing to bump), plus two added after it: a damaging move on the stat
a set dumped, two sets on one species that are the same set, and an item none of
the set's moves can activate. On a batch that touches many sets, run one filtered
check **early**, after two or three lines — a systematic mistake caught on line 3
costs one fix, the same mistake caught on line 60 costs sixty.
Keep rows in dex order. Update `fork-docs/FORK.md` only if the change warrants an
index entry. Then commit to a `claude/<line>-line-review` branch, push, and open a
PR against the fork's `master` with the body described above — one line per PR,
or one PR per **batch** with a commit and a body section per line (see "Batch
mode").
