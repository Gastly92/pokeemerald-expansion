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

## How a review runs — three sequential steps, then one PR

**The three steps are worked one at a time, in order, and the review runs
through to a pull request without stopping for approval along the way:**

> Step 1 innates → Step 2 overrides → Step 3 frontier sets (Part A audit →
> Part B new sets) → apply the edits → verify → open the PR → review feedback.

Settle each step before starting the next: decide the current step, write the
decision down, and let the following step build on it. Don't interleave the
steps, and don't reopen Step 1 while drafting Step 3 sets.

**Why the order is a rule and not a style preference.** The three files are
coupled in one direction: innates determine which abilities are still free,
overrides determine which abilities a set can legally name, and sets consume
both. Work them out of order and a later pick rests on an earlier one that is
still moving, so the whole line has to be re-derived. Two concrete consequences
worth stating, because they are the ones that get violated:

- **An override is justified against the innates you actually settled on.** "Slot
  0 is redundant because Gluttony is innate" is only true if Gluttony's innate row
  is part of this change.
- **A set's `.ability` may only name a real slot of the species or an override row
  settled in Step 2.** Both CI tests below enforce this for real.

**The PR is the review surface.** Flavor picks are still the maintainer's call —
that call is just made on the PR, after seeing the whole line, instead of three
times mid-session. So the PR body has to carry enough to make it: what each step
changed, the flavor evidence behind each pick (repo dex text, canon-user count,
wider media flagged as recall), and what was considered and dropped. Expect
changes to be requested; that is the process working. See "Wrap-up" below for the
PR body shape and how to handle the review comments that come back.

Directional feedback the maintainer volunteers mid-review ("a Body Slam set would
suit the Alolan one") is a hint to spec properly **when its step comes up** —
carry it into that step and act on it there, rather than bolting it onto the step
you are currently on.
---

## Batch reviews — a range of dex numbers in one pass

A review can be asked for as a **batch**: *"let's review the lines from number 0
to 25"*, *"lines 26-50"*, *"the next ten lines."* A batch changes the
**packaging** (one branch, one commit per line, one PR) and nothing else — every
line in it still gets the full three-step review at the same depth, with the same
evidence and the same hard constraints.

### Resolving the range to a list of lines

Numbers are **National Dex numbers**, and **both endpoints are inclusive** —
`0 to 25` means "from the start through #25", and #25's line **is** in the batch
(`0` is just a shorthand for the start; there is no #0). Walk each number in the
range, map it to the evolutionary line that species belongs to, and **dedupe** —
a line is reviewed once no matter how many of its members the range covers.

- **The whole line comes along, including members outside the range.** #25
  Pikachu pulls in Pichu (#172) and Raichu (#26) plus Alolan Raichu and the
  G-Max form — a range never half-reviews a line, and a number that lands
  mid-line (#20 Raticate) still pulls in the whole Rattata line.
- **Every line the range covers gets a pass — a previous review is NOT a skip.**
  If a species is in the range, its line is reviewed, full stop. Ranges overlap
  by design (inclusive endpoints make `0-25` and `25-50` share #25; `20-40`
  re-covers ground on purpose) and the whole dex has already had a first pass, so
  "already reviewed" would skip nearly everything and make a batch a no-op. A
  second pass is the point: the rubric has moved, innates have been wired,
  neighbouring sets have changed. The **only** dedupe is *within* one batch — a
  line is reviewed once per batch no matter how many of its members the range
  covers.
- **Review each line FRESH — do not anchor on the previous review.** Don't go
  reading the old PR to find out what was decided or what was turned down. Judge
  the rows that are in the three files *today*, on the evidence you gather today.
  A candidate a previous pass proposed and the maintainer rejected is **fair to
  propose again**: new information turns up, the rubric moves, and minds change —
  re-raising it costs one line in the PR body and the maintainer says no again in
  a second, while suppressing it can bury the right answer forever. The only
  rejections that still bind are the **structural** ones written into
  `fork-docs/` and enforced by CI (a transformation ability is out of scope for
  innates; an innate-capable ability can't be an override) — those are rules, not
  tastes, and they live in the docs precisely so a fresh pass still meets them.
- **Fresh does not mean restless.** A change still needs a reason of its own —
  a set is replaced because it is *worse* than the alternative, never because a
  re-pass ought to produce a diff. "No changes" stays a legitimate result for a
  line (Part A's *keep as-is* verdict is the same judgement), and it still earns
  its PR section saying what was checked and why it stands.
- **Order the batch by ascending dex**, on each line's lowest in-range number.
- **Write the resolved list down before starting any work.** It is the batch's
  table of contents and the PR's section list. Note against each line whether it
  has a previous review (with its PR number), since that is what tells you how
  deep the pass has to go. State it back to the maintainer, then start — a range
  is an instruction, not a proposal, so don't stop for confirmation on the list.

### Working the batch

- **One line at a time, start to finish.** Steps 1 → 2 → 3 (Part A → Part B) for
  a line, then its commit, then the next line. Don't interleave lines and don't
  batch the steps across lines ("all the innates first") — the step coupling in
  "How a review runs" is per line, and working them across lines re-derives every
  line at once.
- **Cross-line coupling is real, so keep the batch in view.** Two lines in one
  batch can reach for the same held item or the same borrowed ability. A batch is
  the one vantage point where item crowding is visible — if three of the batch's
  sets are converging on Leftovers, spread them (see the scarcity note in Step 3).
- **Commit and push after every line.** One commit per line, scoped to that
  line's rows, subject `Line review: <Line> line — <what changed>` — the subject
  a single-line PR would have carried. The container is ephemeral: a lost session
  should cost one line, not ten.
- **Verify once, at the end of the batch.** `make check TESTS="Frontier extended
  roster"` and `make check TESTS="Innate"` are a build each; running them per
  line burns the budget for no extra signal, and CI runs the full suite on the PR.
- **Budget context deliberately.** Ten lines is a lot of reading. Grep for the
  line's rows and read around the hits (per `CLAUDE.md`); don't re-read this
  rubric or a whole data file once per line.

### Shipping the batch

- **Branch `claude/lines-<first>-<last>-review`; one PR for the whole batch**,
  opened when the last line is committed.
- **The PR body gets one section per line**, and each section carries exactly what
  a single-line PR body carries (per-step changes and reasoning, flavor evidence
  with recall flagged as recall, Part A verdicts including *keep as-is*, rejected
  candidates, open questions). Above them, a short batch header listing the
  resolved lines and flagging the ones that came out **no changes** — which is a
  legitimate result for a line, but still gets a section saying what was checked.
- **If the batch can't be finished** (context, time, a blocker), open the PR
  anyway with the lines that are done and a **remaining** list naming the rest.
  Half a batch shipped beats ten lines lost with the session.
- **Review feedback is per line.** Apply it on the same branch, as a new commit
  naming the line it fixes, and re-derive forward through *that* line's steps —
  a rejection in one line only touches another if they shared a resource (the
  same item, the same borrowed ability), which is worth checking and usually
  isn't the case.
- **Run the filtered checks as the last step of the batch.** Five CI gates cover
  the whole dex unconditionally: innate-row coverage, pre-evolution coverage, a
  legal observable slot per drafted species, no ally-hitting spread move on a
  doubles set, and no status move stranded under a Choice item. Run
  `make check TESTS="Innate"` and `TESTS="Frontier extended roster"` and fix
  whatever they name — that is the batch's real completion criterion, and it
  catches the class of defect a per-line reading misses.

  These five were **review ratchets** while the sweep ran, bounded by a
  "reviewed through this dex number" constant each batch raised as it landed.
  The sweep finished at Pecharunt and both constants were retired, so the gates
  now simply hold. The bumps earned their keep on the way: the first found **55
  ally-hitting spread moves inside territory the sweep had already been through**,
  and the last three each caught missing rows or ally-hitting moves the per-line
  pass had walked straight past.

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
- **Run the review to completion, then open a PR — don't gate on a yes between
  steps.** Work the three steps in order (see "How a review runs" above), apply the
  edits, verify, and put the findings and reasoning for all three steps in the PR
  body. The maintainer reviews there and requests changes; that is the approval
  step. When two picks are genuinely a coin-flip on taste, take one, ship it, and
  name it in the PR body as an open question — an unresolved question is a line in
  the PR, not a reason to stop the review.
- **Expect to be wrong about flavor, and check before defending.** Many picks get
  rejected in review; that is the process working, not a failure. When a pick is
  challenged on the PR, go and verify (canon users, the repo's dex text, the engine)
  rather than arguing from memory — the evidence usually settles it in the
  maintainer's favor, and occasionally it will support the pick, which is worth
  saying plainly.
- **Seven CI tests gate this data** — keep them green (see each step). All seven
  are absolute now: two on abilities (an override or set naming an innate-capable
  or reserved ability) and five on coverage and set shape. The latter five were
  review ratchets during the sweep and lost their dex bound when it finished. See
  "Shipping the batch" above.

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
   - **1 user → it is a signature.** Berserk belongs to Drampa alone; Dragon's Maw
     to Regidrago alone. That is the strongest possible statement about how the
     ability *reads* — it reads as that creature — so borrowing one needs a flavor
     case good enough to survive the comparison, and the PR body should name who it
     is borrowed from. It is **not a veto**. The maintainer settled this in Aug 2026
     after four batches of raising it: the rows that prompted the question (Dancer,
     Cotton Down, Mummy, Wandering Spirit, Well-Baked Body, Victory Star and the
     rest) stand, and a new borrow is judged on flavor like any other pick.
   - **Many users → read the family, then check the species actually belongs.**
     Moxie's 16 (Krookodile, Scrafty, Gyarados, Pinsir, Honchkrow, Heracross …) are
     uniformly swaggering predators, so it misreads on a creature whose dex says it
     "will never torch a weaker foe." Keen Eye's 41 are overwhelmingly birds and
     sharp-eyed watchers, so it misreads on Blastoise, whose accuracy is
     **ballistics, not eyesight**. Conversely the check can *support* a pick that
     looks wrong: Swift Swim's users include Carracosta, a heavy armored shelled
     turtle, so it is not the speedster-only ability it appears to be.

   **Legendaries and mythicals are judged exactly like anything else.** Earlier
   batches held them to a minimal row out of caution — filling one only where a CI
   gate demanded it, and arguing only from the species' own canon family. That is
   not the rule (maintainer decision, Aug 2026): be as generous with a legendary as
   the flavor honestly supports, the same way you would with a Rattata.

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

   **Check the wider media before proposing — the anime, the movies, the manga, the
   spin-offs.** This is a *required* input alongside the dex text, not a permission you
   may decline. The rule directly above governs *dex text* (don't pass remembered flavor
   off as this repo's `.description`); it is not a rule that the games are the only
   admissible source. A `.description` is four lines and frequently says nothing an
   ability can be built on, whereas what a creature is shown *doing* on screen is often
   the sharper evidence. Worked example from the Ninetales pass: the repo dex gives only
   *"each of the nine tails embody an enchanted power,"* which is thin support for
   anything. Its anime episode is *entirely* a Ninetales sealing a mansion so the cast
   physically cannot leave, plus the tail-curse enacted on a character who grabs its
   tails — which is a far better case for Shadow Tag and Cursed Body than the dex line
   could ever carry alone.

   **Report the result either way.** *"No usable media evidence for this line"* is a
   normal, expected line in a proposal — stating it is what proves the pass was run.
   Silence is indistinguishable from having skipped it, so silence counts as skipping
   it. Two conditions on using it:
   - **Name the medium and flag that it is recall.** The repo cannot confirm it, so
     say so plainly rather than presenting it with the same confidence as a quoted
     `.description`. Being wrong about an episode is fine; being wrong *silently* is
     what corrupts the evidence base.
   - **Cite a specific action, not a vibe.** "It traps the cast inside a mansion for
     the whole episode" is evidence. "I think it's mysterious in the anime" is not,
     and neither is a half-remembered episode title with no scene attached.

   Flagging recall is the *price* of using media evidence, not a reason to avoid it.
   A pick that misremembers an episode and says so is a better outcome than a review
   that quietly never looked — the first is correctable in one round-trip, the second
   is invisible. Do not let the honesty requirement become an incentive to omit. The
   tell that it has: media cited only to *reject* a candidate (the direction where
   being wrong costs nothing) and never to generate one.

   The canon-user count (above) is still the gate. Media evidence tells you *which*
   ability expresses the creature; it never licenses a 1-user signature.
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

**A species with NO row at all is a bug, and the reason it happens is
structural.** Twenty-two were found by hand across the Gen 1-3 sweep. The cause
is always the same: when every one of a species' canon abilities is
*never-an-innate*, there is nothing to seed a row from and it gets skipped
entirely. Kecleon (Color Change, Protean), the Manectric line (Static, Lightning
Rod, Minus) and the whole weather trio (Drizzle, Drought, Air Lock) all landed
this way. **The row still has to exist** — build it from the rest of the dex
entry and the design instead. Two consequences worth knowing before you start a
line: a species like this will *never* suggest its own innates, and its
observable slot is forced to hold whatever never-an-innate ability is left over,
which is often somebody's signature (see Step 2).

**Verify:** `make check TESTS="Innate"` — five tests, three of them the coverage
gates: every species with a set has a row, every pre-evolution of one does too,
and every species the roster drafts has at least one slot that can legally hold
an observable ability.

**Before Step 2:** settle the innate list. Which slots Step 2 may repurpose is
determined by the innates you have actually decided on, so don't start Step 2
with the list still in flux.

---

## Step 2 — Override (`src/fork/species_ability_overrides.c`)

Each row replaces one ability *slot* of a species with a real, selectable ability
so a frontier set can run a genuine second trait alongside the innates. For the
line:

1. **Read the existing override row(s), if any.** Confirm:
   - **The chosen ability is NOT one of the species' innates** (otherwise the
     one observable pick is a wasted duplicate — the whole point of the row).
   - **It's a STABLE pick: an ability absent from `sImplementedInnates[]`**
     (`test/fork/innate_abilities.c`) — one that can **never** become an innate
     (Lightning Rod, Water Absorb, Sheer Force, Grassy Surge, Bulletproof …). That
     array is the single source of truth: on it = implemented innate, off it =
     never-an-innate. That is the whole rule. An ability
     that is innate-*capable* (anything on the `sImplementedInnates[]` allowlist)
     is **not** a legal override, even when this species does not currently carry
     it: an innate-capable ability belongs in an `INNATES(...)` row, where it is
     always-on and costs nothing, so spending the single observable slot on one
     both wastes that slot's only purpose — a trait the species can express no
     other way — and leaves a latent duplicate that collapses the moment a future
     line review gives the species that ability innately.
     **This rule was previously written the other way round** ("or an
     already-implemented innate the species does not itself carry"), which is
     wrong and left a large legacy backlog — since cleared in one sweep, so every
     row and set now conforms. The guard is
     `TEST("Innate abilities: no ability override or frontier set names an
     innate-capable ability")` in `test/fork/innate_abilities.c`, which is **no
     longer `KNOWN_FAILING`**: it is a real CI gate, so a non-conforming pick now
     fails the build. New picks must be `:x:`.

   - **A RESERVED ABILITY IS BANNED OUTRIGHT, never-an-innate or not.** A handful
     of abilities are welded to one species line, listed in `sReservedAbilities[]`
     (`test/fork/innate_abilities.c`) — **Illusion** (Zorua/Zoroark) is the first.
     They are `:x:` abilities, so the innate-capable gate above waves them through;
     the second gate, `TEST("Innate abilities: no ability override or frontier set
     names a reserved ability")`, is what rejects them. A set may name one only for
     a species whose vanilla `gSpeciesInfo` already grants it. See
     [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md#reserved-abilities-welded-to-one-line-banned-everywhere-else).

   - **A CONVERSION CAN STILL LAND IN A REVIEW.** Wiring a new innate makes that
     ability innate-capable, which can retroactively invalidate an existing
     override row or set that names it. When that happens on the line you are
     reviewing, **converting it is in scope for Step 2**, the same as proposing a
     new row: pick a `:x:` replacement, and repoint every set whose `.ability`
     named the old one. Check the line with:

     ```bash
     make check TESTS="no ability override or frontier set names an innate-capable ability"
     ```

     and grep its output for your species. Two things follow that are easy to get
     wrong. First, **a conversion can cost the line a trait it was built around** —
     if the old pick was the observable half of a package (Farfetch'd's Super Luck
     feeding its Leek crits), say so plainly in the proposal rather than quietly
     swapping in something weaker; the maintainer may prefer a different `:x:`
     pick, or to accept the loss. Second, **the repointed sets reach into Step 3**:
     a set cannot keep an ability the override no longer supplies, so flag the
     affected sets in Step 2 and treat their `.ability` field as settled there,
     leaving the rest of each set to its normal Part A audit.
     See [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) ("Direction") for the
     rationale, the current counts, and the promotion criterion.
   - **The freed slot is safe to repurpose:** filling an *empty* (`ABILITY_NONE`)
     slot is always safe. Repurposing a *real* slot deletes that ability from the
     species game-wide — only do it when the slot is redundant (that ability is
     now innate) **and** not pinned by an upstream test. Audit `Ability(ABILITY_X)`
     uses in `test/battle/` for that species before repurposing a real slot.
   **Never take an ability's `.description` as its behavior — read the effect
   site.** Those strings are ~45-char upstream UI text and are routinely ambiguous
   or outright duplicated between abilities that do different things. The trap this
   doc was written from: `ABILITY_POISON_POINT` and `ABILITY_POISON_TOUCH` ship with
   the byte-identical description `"Poisons foe on contact."`, yet they fire in
   opposite directions — Poison Point sets `gEffectBattler = gBattlerAttacker`
   (poisons whoever hits the holder) while Poison Touch sets
   `gEffectBattler = gBattlerTarget` (poisons what the holder hits). Which one a
   species wants is the whole question, and the description cannot answer it. Grep
   `ABILITY_X` in `src/battle_util.c` and read who the effect lands on.

2. **Consider adding a row** if the line lacks one and a species' chosen slot is a
   redundant innate (or empty) — pick a stable, flavorful non-innate ability.

   **Check the pick is not inert under the fork's flags before anything else.**
   An override is the species' one observable trait, so an ability that does
   nothing here is worse than a plain one. Two were shipped and had to be
   replaced: **Victory Star** on Volbeat and again on Jirachi — an accuracy boost
   in a fork where `DETERMINISTIC_ACCURACY_EVASION` has removed accuracy from the
   game — and **Moody** on Glalie, which is not merely inert but actively lowers
   the stat the set was built on. Also watch for an override that is live in
   principle but dead on *this line's sets*: Solar Power with no Sunny Day,
   Poison Touch with no contact move, Storm Drain's Special Attack boost on a
   purely physical set. That is a weaker objection — a set can be added to use it
   — but it belongs in the PR body either way.

   **The structural bind, worth recognising rather than fighting.** For several
   species the ability the dex actually describes turns out to be innate-capable,
   which makes it *illegal* as an override — so it goes on the innate row and the
   observable slot has to be filled by whatever never-an-innate ability is left,
   which is often somebody's signature. Absol is the clearest case: its dex is
   "senses even subtle changes in the sky and the land to predict natural
   disasters", which is Forewarn exactly, and Forewarn is innate-capable. Forewarn
   became its innate; the observable slot still holds Yveltal's Dark Aura. When
   this happens, say so in the PR body rather than reaching for a worse pick.
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

   **This is the most-missed rule in the file** — about twenty Megas were found
   with the innate half done and the override half absent, across every batch of
   the Gen 1-3 sweep. CI now catches it: "every reviewed species the roster drafts
   has a legal observable slot" fails when all of a form's slots hold
   innate-capable abilities, which is exactly this bug's signature. **The
   three outcomes, so you can tell at a glance which applies:**
   - Mega's ability is on `sImplementedInnates[]` → **innate + override all
     slots** (Venusaur, Sableye, Mawile, Aggron, Medicham, Banette, Metagross,
     Latias, Latios, Chimecho, Manectric).
   - Mega's ability is **never-an-innate** → **the Mega keeps it, no rows at all**
     (Gardevoir/Pixilate, Tyranitar/Sand Stream, Salamence/Aerilate,
     Rayquaza/Delta Stream, Glalie/Refrigerate, and both Primals).
   - Mega's ability already equals the base's override → **nothing to do**
     (Sceptile/Lightning Rod, Pinsir/Aerilate, Camerupt/Sheer Force).
5. **Pre-evolutions don't need an override row.** An override earns its keep by
   letting a *Factory set* run a real second trait alongside the innates — so a
   species with no sets has nothing to spend it on, and a row there is dead data.
   Give the row to the stage that actually gets drafted (normally the final one,
   plus any regional/Mega form that is its own final stage) and leave the pre-evos
   alone. The exception follows Step 3's: a pre-evo that *does* carry sets (the
   Eviolite walls) is treated as a final stage for this purpose.
   Note the asymmetry with Step 1 — **innates still cover the whole line**, because
   an innate is always-on identity that a Vulpix has whether or not anything drafts
   it, while an override only exists to be selected by a set.
6. **Slot/dex ordering:** rows are sorted by National Dex number with a trailing
   `// <dex>` comment; forms share the base number and follow it.

**Verify:** `make check TESTS="Frontier extended roster"` — four tests enforce the
invariants: "every set's ability is legal for its species" and "no set's chosen
ability duplicates a species innate" cover the sets that select a slot, "no
species ability override duplicates a species innate" sweeps the **whole override
table** independently, so a redundant row fails CI even before any set uses it, and
"no set uses ABILITY_NONE" bans the let-the-Factory-pick escape hatch. Pairing a new
override with a set that uses it is still good practice (it proves the slot resolves
end-to-end), but it is no longer what catches a duplicate.

**The two duplicate-innate tests are scoped PER SPECIES, which is what makes the
second stable branch legal.** Both gate on `SpeciesHasInnate(species, ability)`, and
that helper walks only *that species'* innate list (`src/fork/innate_abilities.c`) —
it never asks whether the ability is an innate on some *other* species. So borrowing
an implemented innate the species doesn't carry is not a near-miss the tests tolerate;
it is the intended design, and the table already does it at scale (`ABILITY_UNAWARE`
in 10 rows, `ABILITY_STICKY_HOLD` in 3, plus Harvest and Gooey). The only thing that
fails is naming an ability *this* species already has innately.

**Before Step 3:** settle the override rows. A set's `.ability` may only name an
ability that is already a real slot for the species or comes from an override row
in this change.

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

**The rule above governs the VETO. The failure that actually happens is at
GENERATION** — and it is invisible, because it leaves no trace to catch. You never
open a learnset, so you never "reject a move for being illegal"; you simply never
*think* of the moves outside it, because the candidates you recall for a species
are the ones it canonically learns. The tell is a proposal that reads like the
species' in-game moveset with the item slot doing all the creative work, followed
by the conclusion that the species has a shallow kit. **A species' kit is never
narrow — the whole move list is available to every set.** If a line review is about
to say "this species only has about six usable moves," that sentence is proof the
gate is on, not evidence about the species.

**Generate from the creature, not from its moveset.** Read the dex `.description`
and the design — and the wider media, per Step 1's rule on the anime/movies/manga,
which pays off even better here than it does for abilities, since a move only has to
depict one thing the creature is shown doing — ask *what does this thing actually
do*, and only then go looking for moves that match — including **other species' signature moves**, which are usually
the best-written expression of a concept and are fully fair game here (the
signature/canon-user gate belongs to Step 1, and applies to abilities only). Worked
examples from the Beedrill pass: its dex says *"if angered, they will attack in a
swarm,"* so **Attack Order** (Vespiquen's swarm-summon) is the dex line rendered as
a move; *"extremely territorial… no one should ever approach its nest"* is an
ambusher, so **First Impression** (Golisopod's +2-priority strike) fits better than
anything Beedrill learns; and a cornered venomous defender wants **Baneful Bunker**
over plain Protect. None of the three are in its learnset, and all three read more
truly than the moves that are.

**Smogon is a legitimate input — for EVALUATION, never for generation.** Competitive
analysis (Smogon dex entries, standard sets, spread benchmarks, what a species is
actually considered good and bad at, what checks it) genuinely sharpens both parts of
this step: it is a good second opinion on whether a move earns its slot, whether a
spread reaches anything, and whether a set is one you would really draft. Use it to
enhance the analysis. Three limits, because it is describing a different game:

- **It is learnset-constrained, and this step is not.** Every Smogon set is legal by
  construction, so building *from* one silently re-imports the exact gate the
  generation rule above spends its length removing — the "narrow kit" failure, arrived
  at by a more respectable route. Generate from the creature first (dex, design, other
  media), *then* let Smogon critique what you built. Never let it bound the move pool.
- **This fork's mechanics are not vanilla, so a competitive claim can be flatly wrong
  here.** Blizzard's max PP is scaled by `DETERMINISTIC_ACCURACY_EVASION` (5 base
  x 70 accuracy = 3) and then raised back to **4** by the facility's max PP Ups
  (`B_FRONTIER_MAX_PP`, `src/battle_frontier.c`) — quote the in-battle number, not
  the scaled base, for any move you are pricing;
  secondaries fire only on super-effective hits; paralysis loses full-para and the Speed
  drop; high-crit moves always crit through the strong-hit gate; Shell Bell and Leech
  Seed are buffed. Re-check any Smogon-derived claim against
  `include/config/deterministic.h` and `include/config/buff.h` before acting on it.
- **The format is different.** Battle Factory drafting means item scarcity (one of each
  item per team), no player-side teambuilding, no team preview, AI opponents, and
  gimmicks competing for one slot per trainer. Smogon assumes full teambuilding and
  free switching; its item and spread advice does not transfer unexamined.

And **flavor still governs.** A Smogon-optimal set that reads as nothing in particular
is a worse outcome here than a flavorful one that gives up some power — this doc is
flavor-first by design, and competitive strength is welcome, not required. Smogon
breaks ties and catches mistakes; it does not pick the concept. Same honesty rule as
the other-media clause in Step 1: it is not in the repo, so flag it as recall (or fetch
the actual analysis if the session has network) rather than presenting it as verified.

**Step 3 runs in two parts, in order: Part A audits what is already there, Part B
adds new sets.** Finish Part A before starting Part B — same reasoning as the step
order itself. Auditing first also tells you what Part B needs: a
species whose one existing set turns out to be fine needs a *different* new set
than one whose existing set is about to be rewritten.

1. **PART A — audit the existing set(s) before proposing anything new.** Go
   through each existing set for the line field by field and give a verdict:
   *keep as-is*, *change X*, or *replace*. **"Keep as-is" is a fine verdict** — the
   point is to have actually checked, not to find something to change.

   a. **Tera type — every set has one, so ask what it is actually FOR.** Three
      legitimate jobs, roughly best to worst: (i) a **tactical immunity or
      resistance flip** (Tera Flying to blank an Earthquake, Tera Fairy into a
      Dragon, Tera Water on a Fire-weak mon) — usually the highest-value use;
      (ii) **STAB for a move the set actually carries** — a Tera type matching no
      move on the set does nothing offensively; (iii) **doubling an existing
      type** for 2× STAB, which is legitimate but normally the weakest, since it
      trades the whole resistance profile and the immunity option for raw power
      on moves that were already STAB.

      **The trap: Terastallizing OVERWRITES ALL THREE type slots**
      (`types[0] = types[1] = types[2] = teraType`, `src/battle_util.c`), so it
      silently breaks anything keyed on the original type. The canonical case is
      **Black Sludge**: it heals only `IS_BATTLER_OF_TYPE(itemBattler, TYPE_POISON)`
      and *damages* everything else (`src/battle_hold_effects.c`), so a Poison-type
      holding Black Sludge with a non-Poison Tera type turns its own recovery into
      chip damage the moment it Teras. Same class of bug: losing an immunity the
      set was built around (a Ghost that Teras out of its Normal/Fighting
      immunity), or switching off a type-gated ability. Check the Tera type
      against the **item, the ability and every move** before calling it fine.

      **STAB is the ONE thing the overwrite does NOT break — do not "fix" a set
      over it.** Damage takes a different path once Terastallized:
      `ApplyModifiersAfterDmgRoll()` (`src/battle_util.c`) swaps
      `GetSameTypeAttackBonusModifier()` for `GetTeraMultiplier()`
      (`src/battle_terastal.c`), which keys off **`IS_BATTLER_OF_BASE_TYPE`** — the
      `ignoreTera` variant, reading the `gBattleMons[].types[]` array that the Tera
      branch of `GetBattlerTypes()` returns early without touching. So a move
      matching the Tera type **and** an original type gets **2.0×**; one matching
      only the Tera type, **1.5×**; and one matching only an *original* type keeps
      its full **1.5×**. Terastallizing never costs a set offense — it only ever
      adds. Worked example: Tera Fairy on a Normal/Fairy mon takes Dazzling Gleam
      to 2.0× and leaves Hyper Voice at 1.5×. The overwrite's consequences are
      **defensive and type-gated only** (matchups, items, abilities), never STAB.

      Also: **Tera is a gimmick**, so under `FEATURE_FREE_GIMMICKS` it competes for
      the one-per-trainer slot exactly like a Mega (see the free-gimmicks section
      below). A set that *needs* to Tera to function is unreliable for the same
      reason a set that needs its Mega is. Tera is upside.

   b. **Moves — is each of the four earning its slot?** Look for redundant
      coverage, a move strictly outclassed by another the set could run, a
      secondary effect that can never fire under the deterministic gate (see the
      `DETERMINISTIC_*` section — never build on a secondary landing against a
      neutral target), missing STAB, no answer to a common immunity, or four
      attacking moves where one utility slot would do more.

      Also check for **moves that fight each other inside one set** — four turned
      up in Gen 1-3: Rock Polish beside Hammer Arm (which lowers the Speed the
      Polish just raised); Calm Mind beside Draco Meteor (which drops the Special
      Attack the Calm Minds raised — Draco Meteor belongs on a Choice set, where
      you switch out after firing it); Knock Off beside Poltergeist (Knock Off
      disarms the target Poltergeist needs); and Sunny Day on a set whose ability
      is Solar Power, which then drains 1/8 HP a turn from a staller.

      **Two format-specific move facts worth knowing.** `Poltergeist` is close to
      unconditional here: every set in the roster specifies a `heldItem`, so every
      drafted opponent is holding one — which makes it a 110 BP Ghost STAB rather
      than a gamble, and strictly better than Shadow Claw's 70. And a **rental's
      friendship is `MAX_FRIENDSHIP`** (`src/battle_frontier.c`), so `Return` is
      always at full 102 BP while `Frustration` is always at its floor.
   c. **Held item — is it still doing anything, and is it too crowded?** Re-check
      it against the set's moves and ability, and against the fork's changes
      (`BUFF_*`, `DETERMINISTIC_HOLD_EFFECTS` — several items behave differently
      here than stock knowledge expects). Then check its **scarcity**: only one of
      each item can appear per drafted team, so a set on Leftovers or Life Orb
      (21% and 17% of the roster) is drafted measurably less often than one on a
      tail item. See the scarcity note under point 4 below — an existing set
      sitting on a default Leftovers is a prime candidate to move off it.
   d. **Nature, EVs, IVs — does the spread match what the set actually does?**
      A nature dropping a stat the set uses, EVs invested in an unused attacking
      stat, or a Speed investment that reaches no relevant benchmark.

      **The three recurring shapes**, all found repeatedly in the Gen 1-3 sweep:
      (i) a **damaging move on the category the set did not build** — Glalie ran
      a special Freeze-Dry on a Jolly 252-Attack spread, Walrein a physical Body
      Slam on a Modest 252-Special-Attack one; (ii) a **nature boosting a stat
      with 4 EVs in it** (Delcatty's Modest over 4 SpA, Minun's Timid over 4 Spe);
      (iii) **252 Speed on a species that cannot use it** — below roughly base 60
      the investment reaches nothing, and a set carrying priority (Aqua Jet,
      Sucker Punch, Ice Shard) has already conceded it moves second, so those EVs
      belong in HP. Speed *is* earned when a move doubles it: Dragon Dance, Rock
      Polish, Agility, Autotomize, Shell Smash, or a Swift Swim / Chlorophyll
      innate the set turns on itself.

      **Do not turn (i) into a mechanical rule** — it was tried, and a category-
      versus-nature sweep returns 133 hits of which most are correct by design.
      Foul Play attacks off the *target's* Attack; Body Press off Defence; Seismic
      Toss and Night Shade are fixed damage; Counter and Mirror Coat return what
      they took; and U-turn, Flip Turn, Rapid Spin, Fake Out, Knock Off and Icy
      Wind are drafted for their effect, not their damage. This one is a
      judgement call, which is why it is here and not in CI. On IVs:
      only ~42 of 1200+ sets set `.iv` at all, and `CreateFacilityMon` applies it
      only `if (fmon->iv)`, so leaving it unset means "facility default" and is
      normal — the usual reason to set one is minimum Speed for Trick Room.
      **Footgun:** `TRAINER_PARTY_IVS(hp, atk, def, speed, spatk, spdef)` takes
      **Speed as its 4th argument**, whereas `EVS()` uses named fields in struct
      order (`hp/atk/def/spa/spd/spe`). Reading the IV macro as if it matched the
      EV field order silently zeroes Sp. Atk instead of Speed.
   e. **Ability — still the right pick** given the species' innates (CI already
      enforces legal-and-non-innate; this is the flavor//usefulness question).
   f. **Format tag** — does a `FORMAT_BOTH` set genuinely hold up in both, and is
      the line's coverage across singles/doubles balanced?

      **Spread moves that hit your own ally are the single most common defect in
      this file, and CI now gates it.** A `TARGET_FOES_AND_ALLY` move damages the
      holder's partner every time it is clicked, so it is wrong on any
      `FORMAT_DOUBLES` or `FORMAT_BOTH` set. Nineteen moves are in this class:
      Earthquake, Bulldoze, Magnitude, **Surf**, Sludge Wave, Discharge, Lava
      Plume, Petal Blizzard, Boomburst, Sparkling Aria, Searing Shot, Mind Blown,
      Brutal Swing, Corrosive Gas, Parabolic Charge, Synchronoise, Teeter Dance,
      and the self-KO pair. **Surf is the one that hides**: its source line reads
      `B_UPDATED_MOVE_DATA >= GEN_4 ? TARGET_FOES_AND_ALLY : TARGET_BOTH`, so a
      text search for the constant misses it while the build resolves to the
      ally-hitting branch. Do not trust a grep — ask `GetMoveTarget`.
      Usual repairs: **Earthquake → High Horsepower** (same coverage,
      single-target, −5 BP), **Surf → Muddy Water** (both 90 BP; Muddy Water is
      `TARGET_BOTH`, foes only), **Sludge Wave → Sludge Bomb**, **Discharge →
      Thunderbolt**, **Boomburst → Hyper Voice**; on a *special* set the Ground
      slot usually wants **Earth Power**, which fixes the stat mismatch at the
      same time. The alternative repair — retagging to `FORMAT_SINGLES` — is
      legitimate but shrinks the doubles pool, so prefer the move swap.
      **Exempt:** Explosion, Self-Destruct and Misty Explosion. Hitting everything
      adjacent is what they are, and pressing one is a deliberate last act.

      **A Choice item and a status move do not coexist.** The lock makes the
      status move either unreachable (you clicked something else) or a dead end
      (you clicked it and are stuck). Sharpedo shipped Choice Scarf beside Destiny
      Bond. Only three exceptions, all cases where the lock costs nothing: Trick
      and Switcheroo (handing the item over *is* the payload) and Transform
      (Ditto's set is four copies of it). Also gated by CI.
   g. **Base-form viability** — re-check the set against the base stat line per
      the free-gimmicks rule below.

   Note what niche each surviving set fills, so Part B adds variety rather than
   duplicating.

   **Before Part B:** finish the Part A verdicts. What Part B should add depends
   on which existing sets survived and in what shape.

2. **PART B — write the new sets. Aim for at least 2 quality sets per species** — that is the bar, not a
   quota to fill. Each should occupy a distinct niche so the Factory has real
   variety to draw among: a signature-move set, a gimmick (Trick Room, weather,
   Baton Pass, status spreader), a lore set, a defensive staller, an offensive
   sweeper. More than two is welcome when each new set genuinely earns its place;
   two excellent sets beat five where three are filler, because of the draft
   dilution rule below. **Never pad with near-duplicates to reach a number.**

   **"Narrow kit" is the most-abused clause in this doc — earn it before you use
   it.** Because every move in the game is legal (see the generation rule above), a
   kit is only ever narrow in *flavor*, never in availability. Before invoking this
   as a reason to stop, name the creature's concept and show that the move list has
   nothing left to express it. "I can't think of more moves it learns" is the
   learnset gate wearing a disguise, and it will happily justify a two-set pool for
   a species with plenty of design space left.

   **Every set must be one you would actually want to draft.** The Factory draws
   among a species' sets, so a set that is strictly worse than its siblings has
   *negative* value — it dilutes the pool with a draw you would trade away. Filling
   "a different niche" is not sufficient on its own; the set has to be a genuinely
   competitive option against the others already there. Worked example: a physical
   Venusaur build fills an empty niche and is still wrong, because even with the
   Mega it swings 100 Atk against the special sets' 122 SpA — and without the Mega,
   82. Nobody would pick it, so it does not belong in the pool.

   **"No changes" is a legitimate result.** A species already at two or more
   coherent, format-covering, base-form-viable sets is done unless you have a set
   that is genuinely as good as what is there. Adding one to hit a number is
   padding — the same reasoning as the "don't pad to a number" rule for innates in
   Step 1.

   **Pre-evolutions don't get sets.** The "would you actually draft this" bar above,
   applied to a whole stage: a Vulpix or an Ivysaur set is drafted into the same team
   slot as fully-evolved mons and simply loses, so building one adds a draw nobody
   wants. The two-sets-per-species bar means **per species that gets sets at all** —
   it is not an instruction to give the base stage a pool. Spend the line's budget on
   the final stage and on any regional/Mega form that is its own final stage.

   The roster already works this way and is worth checking against: of **642 species
   with sets, the only non-final stages are Chansey, Porygon2 and Dusclops** — and
   all four of those sets hold **Eviolite**, which is the entire reason they qualify.
   That is the exception in full: a pre-evo gets sets when Eviolite (or a comparable
   niche) makes it genuinely better at something than its evolution. If you cannot
   name what it beats its evolution at, it does not get a set.
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

   **A held item is also a SCARCITY cost, not just a stat line — check how crowded
   it is before choosing it.** The Factory draft rejects any candidate whose
   `heldItem` already appears on the team being built (`src/battle_frontier.c`, the
   "Ensure this Pokemon's held item isn't a duplicate" loop — a non-`ITEM_NONE`
   match makes the draft skip that mon and roll again). Only **one of each item can
   appear per team**, for the player's rental team and for each opponent's team
   alike. So a set holding a heavily-used item is *drafted less often*: it loses
   every roll where some other mon already took that item.

   The distribution is extremely lopsided, so this matters more than it sounds.
   Measure it before picking — `grep -o "\.heldItem = ITEM_[A-Z_0-9]*"
   src/fork/frontier_extended_mons.c | sort | uniq -c | sort -rn` — which as of this
   writing gives **Leftovers 260 sets (21%) and Life Orb 209 (17%)**, i.e. ~39% of
   the roster fighting over two item slots, against 87 distinct items total and a
   long tail used once or twice.

   Practical rule: when an item is a genuine build-around (Flame Orb on a Guts mon,
   a weather rock, Choice Specs), take it regardless of crowding — the set needs it.
   But when you are reaching for Leftovers or Life Orb as a **default** because
   nothing else suggested itself, prefer a near-equivalent from the tail; the set
   plays about as well and actually shows up. A Figy Berry on a Gluttony mon does a
   comparable job to Leftovers *and* has essentially no competition.
5. **Account for this fork's mechanics when picking moves and items** — they
   change what's good in ways stock knowledge misses:
   - **`DETERMINISTIC_*` flags** (`include/config/deterministic.h`) replace the
     fork's RNG with fixed, *state-based* outcomes. "Deterministic" does **not**
     mean "always happens" — several of these are conditional, and guessing which
     is the single most common way a line review gets a set wrong. Read
     ["The `DETERMINISTIC_*` regime"](#the-deterministic_-regime--what-actually-changes-for-set-building)
     below before proposing any move or item.
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
     an innate. **`ABILITY_NONE` is banned** — a fourth roster test
     (*"no set uses ABILITY_NONE"*, `test/fork/frontier_extended_roster.c`) fails on
     it outright. Letting the Factory pick yields an unlabeled ability that, for the
     fork's all-innate species, can only land on a redundant innate; gating the
     override table behind `FEATURE_INNATE_ABILITIES` removed the last reason to
     allow it, so every set names a real chosen ability, freeing a slot with a fork
     override where needed. (Note the test file's *own* comment in the
     duplicate-innate test still describes `ABILITY_NONE` as acceptable — it predates
     the ban.)
   - `.nature` — `NATURE(DEF_UP, ATK_DOWN)` style (boosted, lowered).
   - `.ev` — `EVS(.hp = 252, .def = 252, .spd = 4)` (names: hp/atk/def/spa/spd/spe).
   - `.teraType` — optional Tera type.
   - Optional: `.dynamaxLevel`, `.gender`, `.isShiny`, etc. (gmax mons get the
     Gigantamax Factor + max Dynamax Level automatically at draft).
8. **Keep dex order** (rows are grouped by generation with `// <dex>` markers).

**Why the helpers exist.** `EVS()`, `NATURE()` and the `{ // NNNN` dex comment (the
same convention as `innate_abilities.c`) are readability-only — they compile to
identical bytes. They replace positional `TRAINER_PARTY_EVS` and raw `NATURE_BOLD`
constants specifically so a set can be read and edited on a phone without counting
argument positions or recalling that Bold is +Def/−Atk. All three are defined in
`include/fork/frontier_extended_mons.h`.

### Easy to get backwards — check these before writing the reasoning

Every row below was got *wrong* at least once during the Gen 1-3 sweep, in a PR
body, and had to be corrected afterwards. The rubric already said most of them;
saying them again in one table is cheaper than another correction.

| Claim that feels right | What is actually true |
|---|---|
| "High-crit moves are dead under `DETERMINISTIC_CRITICAL_HITS`" | **They are live, and better than in vanilla.** A move with any high crit ratio always crits through the strong-hit gate. Slash, Night Slash, Cross Poison, Stone Edge, **Crabhammer** and **Shadow Claw** are picks, not filler. The dead route is crit *stage stacking*. |
| "Terastallizing into your own type is a no-op" | It changes no typing, but it **upgrades that type's STAB from 1.5x to 2.0x** — a real 1.33x on every STAB move. Say "no *typing* change", and weigh the STAB gain against what a different Tera would buy. |
| "Moody is inert" | **It is actively harmful.** `DETERMINISTIC_ABILITIES` has it raise the lowest raw stat by two stages and *lower the highest by one*, so on a 252/252 spread it reliably lowers a stat the set was built on. |
| "Tangled Feet / Sand Veil / Snow Cloak are dead, evasion is gone" | Evasion is **converted, not deleted**: an evasion source on the target becomes a **+1 PP tax on the attacker** (`GetDeterministicMoveTargetPPTax`). A test pins this for Spinda. Beware the corollary — giving such a species Own Tempo makes it unconfusable and silently kills its own Tangled Feet. |
| "Harvest recovers a berry half the time outside sun" | **It always recovers one** here (`deterministic.h`, Harvest). A Sitrus Berry on a Harvest species is renewable healing, which beats Leftovers on any large-HP body. |
| "Serene Grace doubles a secondary's odds" | It **bypasses the strong-hit gate entirely**, so the effect is *certain*. Jirachi's innate Serene Grace makes Iron Head's flinch, Zen Headbutt's flinch and Ice Punch's freeze land every single time. |
| "Sheer Force means always take the stronger move" | Sheer Force cancels **Life Orb recoil** on any move it boosts, so a *weaker* move with a secondary can beat a stronger one without: Ancient Power at 60 BP x1.3 and no recoil ties an 80 BP Power Gem that pays it. |
| "Surf hits both foes" | Surf is `TARGET_FOES_AND_ALLY` at this fork's `B_UPDATED_MOVE_DATA` — it hits your **own partner**. The source reads `TARGET_BOTH` in a ternary, so grepping for the constant misses it. See the spread-move checklist in Step 3. |
| "Sheer Force only trades away *chance-based* secondaries" | `MoveIsAffectedBySheerForce` tests `chance > 0`, so it **strips 100% effects too** — Fake Out's flinch, Chilling Water's Attack drop, Pounce's Speed drop. Never put Fake Out on a set that names Sheer Force. |
| "Wide Lens / Zoom Lens are inert" | **Not any more** — `BUFF_ACCURACY_ITEMS` gave both a job. They cancel the *attacker's* side of the PP economy: Wide Lens the flat evasion taxes, Zoom Lens those **plus** the whole stat-stage half (the target's evasion boosts *and* the holder's own accuracy drops) while it moves second. Pure boon — never refunds below the base 1 PP. Both also feed the INFO viewer: Wide Lens reveals every seen foe's held item, Zoom Lens one foe's ability and full moveset. `GetAccuracyItemRelief()` in `src/fork/deterministic_moves.c`; a roster test fails a set holding a lens its own ability already makes redundant. BrightPowder is live too — it taxes attackers a PP. |
| "This species has no innate row / no sets" | **Resolve the constant to a species id before believing it.** Rows and sets are frequently keyed on a form constant that the bare name aliases to — `SPECIES_AEGISLASH` → `_SHIELD`, `SPECIES_SCATTERBUG` → `_ICY_SNOW`, `SPECIES_ZYGARDE` → `_50`, and `SPECIES_GOURGEIST` → `_AVERAGE` while the roster actually drafts `_SUPER`. **Minior chains two hops** (`SPECIES_MINIOR` → `_METEOR` → `_METEOR_RED`), so one substitution is not enough. Grepping `SPECIES_X_` with a trailing underscore misses a bare `SPECIES_X`; grepping the bare name misses the form rows. The Gen 6 batch added four duplicate innate rows this way (shadowing real ones and breaking the Effect Spore test) and missed an entire Hoopa set — the gates caught both, but only at the end-of-batch check rather than during the per-line pass. |
| "A form's ability slot is free to override if the ability is redundant" | Check `form_change_tables.h` first. Cherrim's Overcast ↔ Sunshine change is gated on `ABILITY_FLOWER_GIFT` under `B_WEATHER_FORMS >= GEN_5` (the shipped config), so overriding that slot would strand it in one form for the whole battle. |
| "No Guard is inert — every move hits anyway" | **Live, and good.** Semi-invulnerability is resolved *before* the accuracy gate (see the `FORK:` comment in `DoesMoveMissTarget`), and `CanBreakThroughSemiInvulnerablityInternal` returns TRUE for No Guard on either side — so it hits through Fly, Dig, Dive, Bounce and Phantom Force. It also zeroes the evasion PP tax outright (`GetDeterministicMoveTargetPPTax`). |
| "This canon ability is missing from the row, so it is a gap" | Check the rest of the row first: **another innate may make its trigger impossible.** Inner Focus never flinches, so an innate Steadfast can never fire (Riolu line); Own Tempo is never confused, so it kills its own Tangled Feet (Spinda). Both are deliberate omissions with tests pinning them — and an added innate does not *fail* such a test, it invalidates its `ASSUME` and silently turns a pass into `ASSUMPTIONS_FAILED`, so watch the passed count, not just the red count. |

### The `DETERMINISTIC_*` regime — what actually changes for set-building

`include/config/deterministic.h` is the **source of truth**, and every flag's
`#define` carries a long comment describing its exact rule. Read the relevant one
before claiming a move or item behaves a certain way; this section is a map of
what to look at, not a replacement for it. Where a proposal turns on a mechanic,
cite the header (or the effect site it names) rather than stock Pokémon knowledge
— several rules here inverted a "well-known" interaction.

**The three traps, in the order they bite:**

**First, the concept that unifies several of these: the "strong hit" gate.** The
fork replaces a number of separate random rolls with one shared condition,
`DeterministicAdditionalEffectApplies()` in `src/fork/deterministic_moves.c`:

> the hit was **super effective** — or, for a move type that can *never* be super
> effective (Normal), the move is **STAB**.

Learn this one rule and several flags below collapse into it. It governs **secondary
effects**, **flinch**, and **high-crit-ratio moves** alike. When you see a mechanic
that used to be a percentage, the first question is whether it now rides this gate.

1. **Secondary effects are GATED, not guaranteed.** This is the big one.
   `DETERMINISTIC_ADDITIONAL_EFFECTS` does *not* make a 30% burn fire every time.
   A chance-based additional effect lands only when the hit was **super
   effective** — or, for a type that can never be super effective (Normal), only
   when the move is **STAB**. So Fire Punch burns only a Fire-weak target; Body
   Slam paralyzes only from a Normal-type user; Poison Jab poisons only Grass and
   Fairy. Effects already at ≥100% are unchanged, and Serene Grace / the Pledge
   Rainbow bypass the gate entirely (they make the effect *certain* rather than
   doubling odds) — which makes Serene Grace far stronger here than in vanilla.
   **Never build a set around a secondary landing on a neutral target.**
2. **Crits have three separate routes, and the crit *stage* is the dead one.**
   `DETERMINISTIC_CRITICAL_HITS` removes the random roll, and the Gen-7 odds table
   (`{24, 8, 2, 1, 1}`) only reaches 1/1 at **crit stage 3** — so stacking +1s never
   gets there (a high-crit move plus Scope Lens is stage 2). Reasoning from the
   stage alone gives the wrong answer for every crit set. The routes that *do* fire,
   each an independent branch:
   - **The strong-hit gate.** A move with any high crit ratio
     (`GetMoveCriticalHitStage(move) > 0` — Slash, Night Slash, Leaf Blade, Cross
     Poison, Attack Order, Stone Edge …) **always crits on a super-effective hit**,
     or from a STAB user for Normal-type moves. This makes high-crit-ratio moves
     *strong* here, not dead — the opposite of the naive reading.
   - **Crit items**, via `DETERMINISTIC_HOLD_EFFECTS`, which branches in
     `IsCriticalHit()` *before* the stage check: first landed attack crits, item
     consumed.
   - **Guaranteed sources**: always-crit moves, Laser Focus, Focus Energy / Dragon
     Cheer (one-shot), Super Luck on its first turn, and **Merciless vs. a poisoned
     target**.

   Note the high-crit route is gated on `!ctx->aiCalc`: the AI's damage prediction
   runs the crit calc before type effectiveness is known, so it does *not* foresee
   these crits. That is deliberate (thinking-time budget), and it means a
   high-crit-ratio move is quietly better against the AI than the AI expects.
3. **Deterministic ≠ better.** `DETERMINISTIC_PARALYSIS` deletes full-paralysis
   *and* the Speed drop, replacing them with a PP and priority tax — so paralysis
   is far weaker here, not stronger. `DETERMINISTIC_STATUS` turns confusion into a
   single self-hit that a status move shakes off for free. Check the direction of
   each change before leaning on it.

| Flag | What it means for a set |
|---|---|
| `DETERMINISTIC_ADDITIONAL_EFFECTS` | Secondaries land **only on a super-effective hit** (or on STAB for Normal-type moves). ≥100% effects unchanged; Serene Grace / Pledge Rainbow bypass the gate. Flinch obeys the same gate. |
| `DETERMINISTIC_FLINCH` | Anti-lock cap: a gated flinch can't be re-applied to a target that flinched last turn. Fake Out and ≥100% flinches are exempt. Inner Focus / Shield Dust / Covert Cloak unchanged. |
| `DETERMINISTIC_CRITICAL_HITS` | Crits only when guaranteed. **A high-crit-ratio move always crits through the strong-hit gate** (super effective, or STAB for Normal) — so Slash/Night Slash/Cross Poison/Attack Order are live picks, not dead ones. Also: always-crit moves, Laser Focus, **Merciless vs. a poisoned target**, Focus Energy and Dragon Cheer (one-shot), Super Luck on its first turn. Crit *stage* stacking is the dead route — it needs stage 3 and +1s never reach it. Battle/Shell Armor and Lucky Chant still block. |
| `DETERMINISTIC_HOLD_EFFECTS` | Chance items become guaranteed **one-shots**, then are consumed. Crit items (Scope Lens/Razor Claw, Lucky Punch, Leek) → first landed attack crits. Focus Band → a Sash that works from **any** HP, entry turn only. Quick Claw → first in bracket, entry turn. King's Rock/Razor Fang → one guaranteed flinch (bypasses the anti-lock cap). Lansat → next attack crits. Blunder Policy rearmed onto Protect/immunity/semi-invulnerable "blunders". Starf raises the highest stat. |
| `DETERMINISTIC_ABILITIES` | Contact-status abilities **always attempt**: Poison Point, Poison Touch, Static, Flame Body, Cute Charm (gender requirement dropped), Toxic Chain, Cursed Body. Effect Spore instead lowers the **contact attacker's accuracy** by one stage (no status). Shed Skin always cures; Harvest always recovers. Stench and Quick Draw fire on the first turn on the field, then always. Rivalry keys off shared **type**, not gender. |
| `DETERMINISTIC_MOVE_RESULTS` | 2–5 hit moves always hit **3** times — **5** with Loaded Dice or Skill Link (Population Bomb: 5, or 10). **Protect-family always fails on consecutive turns.** Binding moves last 4 turns, 7 with Grip Claw. Rampage 2 turns. Speed ties resolve on a fixed ladder. Roar/Whirlwind/Dragon Tail drag the next party member **in slot order**. |
| `DETERMINISTIC_STATUS` | Sleep is a fixed `DETERMINISTIC_SLEEP_TURNS` (3) — but the wake-up turn is still an acting turn, so N costs the target **N−1** actions. Rest is exempt. Confusion = one 40-BP self-hit on the next attacking move, then clears; a status move shakes it off free. Infatuation lasts 2 actions and halves damage instead of blocking it. |
| `DETERMINISTIC_PARALYSIS` | **No full-paralysis and no Speed drop.** Paralysis costs +1 PP per move and −1 priority. Quick Feet is exempt from both. |
| `DETERMINISTIC_ACCURACY_EVASION` | Every move that reaches the accuracy roll hits, so Hydro Pump / Focus Blast / Stone Edge are reliable — paid for as a **PP economy** (low-accuracy moves get reduced max PP; accuracy/evasion stages shift per-use PP cost). OHKO moves deal 40% max HP and keep their immunities. Formerly-50% moves (Zap Cannon, Inferno) now need a recharge turn. |
| `DETERMINISTIC_DAMAGE` | Damage roll is fixed at 92% on turn 1 and **+1% per turn, uncapped** — so it passes 100% from turn 9. Rewards sets that survive to snowball. |

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

**Then apply:** with Part B settled, apply the edits for all three steps, run the
filtered tests, and move to the wrap-up.

---

## Wrap-up — apply, open the PR, then act on the review

### 1. Apply and verify

- **Build both flag states** for any new/changed config-gated code, per
  `CLAUDE.md` (Building & testing). Data-only additions to these three files are
  covered by the roster/innate tests; run those filtered
  (`make check TESTS="Frontier extended roster"`, `make check TESTS="Innate"`) and
  let CI run the full suite on the PR.
- **Update `fork-docs/FORK.md`** if the change is worth indexing (usually the
  innate/override/roster features already have rows; a per-line data tweak rarely
  needs a new row, but note anything with a known limitation).
- **One line per branch/PR** unless the maintainer says otherwise. Branch as
  `claude/<line>-line-review`; PR against the fork's `master`. A **batch** is the
  standing exception: one branch `claude/lines-<first>-<last>-review`, one commit
  per line, one PR — see "Batch reviews" above.

### 2. Write the PR body — it is the review surface

Nothing was approved mid-session, so the PR body is where the maintainer sees the
reasoning for the first time. It has to stand on its own; a diff of ability
constants and move slots does not. Structure it by step (in a batch, by line
first, then by step within each line's section):

- **Per step (innates / overrides / frontier sets), what changed and why.** For
  each pick, the flavor evidence that carries it: the repo's `.description` quoted
  verbatim, the canon-user count for an ability, and any wider-media evidence
  **flagged as recall** the repo can't confirm (same honesty rule as Step 1 —
  naming the medium and the specific action). A reviewer should be able to reject a
  pick without opening the source files.
- **Part A verdicts for existing frontier sets**, including the *keep as-is* ones —
  "checked and unchanged" is information; silence reads as "not looked at."
- **Record what you rejected, and why.** Much of a line review's value is in the
  candidates considered and dropped, and the PR body is where that record lives —
  it is what the maintainer weighs the accepted picks against, and what a future
  decision about this line is made from. It is a **record, not a veto**: a later
  pass reviews the line fresh and may well raise a dropped candidate again, which
  is intended (see "Batch reviews"). The exception is a rejection on a durable
  *structural* ground rather than taste (e.g. a transformation ability, which is
  out of scope for innates entirely — see the "Identity / form / type-transform"
  bucket in `INNATE_ABILITIES.md`): record that in the relevant doc, where it
  binds every future pass, instead of in a PR body nobody re-reads.
- **Open questions** — the coin-flip picks you resolved yourself, called out so the
  maintainer can flip them back cheaply.
- **Note the save-index cost when adding frontier sets.** Sets are inserted at the
  species' dex position, and saved rentals reference entries by array *index*, so any
  insertion invalidates an in-progress rented team in an existing save. Inherent to
  keeping dex order — surface it in the PR rather than burying it.

### 3. Act on the review

The maintainer reviews the PR and requests changes there. Expect rejections —
flavor is subjective and most first-pass picks don't survive; that is the process,
not a failure.

- **Apply requested changes on the same branch and push** — don't open a second PR
  for the same line.
- **Respect the step coupling when reworking.** A rejected innate can invalidate the
  override that assumed it and the set that named that override; re-derive forward
  through the steps rather than patching the one row that was commented on.
- **Verify before defending.** When a pick is challenged, go check (canon users, the
  repo's dex text, `deterministic.h`) rather than arguing from memory, and say
  plainly when the evidence supports the maintainer — or, less often, the pick.
- **A rejected candidate goes into the PR body's rejected list** — as the record
  of what this review weighed, not as a ban on ever raising it again. If the ground
  was *structural* rather than taste, put it in the relevant `fork-docs/` doc, which
  is the only place a rejection binds a later pass.
