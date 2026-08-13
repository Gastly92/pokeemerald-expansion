---
name: line-review
description: Review and enhance one Pokémon evolutionary line's fork data — innates, ability overrides, and Battle Factory movesets. Triggers when the user asks to look at / review / update / enhance a species "line" (e.g. "let's look at the Venusaur line for updates and enhancements", "review the Gengar line", "any additions for the Dragonite line?"). For this fork's three fork-owned data files only.
---

# Line review

Run a structured updates-and-enhancements pass over one Pokémon evolutionary
**line** (base + all evolutions + all forms: Mega, G-Max, regional) across the
three fork-owned data files. Flavor and fun come first; competitive power is
welcome but not required.

## Do this

1. **Read the rubric:** `fork-docs/LINE_REVIEW.md` is the source of truth for the
   criteria and constraints. Follow it.
2. **Scope the whole line, every form.** Grep all species constants for the line
   (base + evolutions + `_MEGA` / `_GMAX` / `_ALOLA` / etc.) in each of:
   - `src/fork/innate_abilities.c` — always-on innates (Step 1).
   - `src/fork/species_ability_overrides.c` — the chosen ability (Step 2).
   - `src/fork/frontier_extended_mons.c` — Battle Factory sets (Step 3).
3. **Run the three steps ONE AT A TIME, each behind its own approval gate.**
   Innates → *yes* → overrides → *yes* → frontier sets (Part A audit → *yes* →
   Part B new sets) → *yes* → apply. Propose
   **only** the current step; do not preview or reason about the next one. Each
   step's proposal may rest only on the **approved** output of earlier steps,
   never on a pending one — an override cannot be justified against a proposed-
   but-unapproved innate, and a set's `.ability` cannot name an override that has
   not been agreed. Proposing all three at once is what this rule exists to
   prevent: it silently couples the later steps to picks that are about to be
   rejected, and the rework cascades.
4. **For the current step, per the rubric:** report what's already there, whether
   it makes flavorful sense — citing both the repo `.description` **and** a
   wider-media check — and concrete candidate additions/changes.
5. **WAIT for a yes before moving on.** Flavor picks are the maintainer's call. A
   **deferral is not an approval**, an **unanswered question is not a yes**, and
   "let's return to the line review" means resume the *review*, not ship the
   backlog. Expect most first-pass flavor picks to be rejected — that's the
   process working. Apply the edits for all three files once the last gate
   passes (or per step, if the maintainer asks for that).

## Hard constraints (see the rubric for detail)

- **NO explanatory comments in the three data files.** They are data tables: a row
  carries its values and the `// <dex>` marker the file already uses, nothing else.
  Never annotate a row with why a build works, what an item/flag does, or how an
  ability interacts. Settled convention, actively enforced — prose added here gets
  stripped by a later PR. Put the reasoning in `fork-docs/`, on the config
  `#define`, or in the PR body.
- **Innates:** only *already-implemented* abilities (the `sImplementedInnates[]`
  allowlist in `test/fork/innate_abilities.c`) — an unwired pick fails CI. Don't
  wire new abilities in a line review.
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
- **Overrides:** the chosen ability must **not** duplicate a species innate and
  must be a *stable* pick (`:x:` never-an-innate in
  `fork-docs/INNATE_ABILITIES_PROGRESS.md`, or an implemented innate the species
  doesn't carry). Only repurpose a *real* slot that's redundant and not
  test-pinned; filling an empty slot is always safe.
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
- **Frontier sets run in two parts: Part A audits the EXISTING sets, Part B
  proposes new ones** — Part A first, with its own yes. Part A walks each existing
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
  Speed drop entirely. Read the "`DETERMINISTIC_*` regime" section of the rubric —
  and `include/config/deterministic.h` itself, which is the source of truth —
  before building a set around any mechanic. The set's `.ability` must resolve to a real
  slot and not be an innate (or use `ABILITY_NONE`).
- **Smogon is a legitimate input for EVALUATION, never for generation.** Competitive
  analysis sharpens both parts of Step 3 — a second opinion on whether a move earns
  its slot, whether a spread reaches a benchmark, whether a set is one you'd draft.
  Three limits: it is **learnset-constrained**, so building *from* a Smogon set
  re-imports the very gate the moveset rule above removes (generate from the creature
  first, then let Smogon critique it — never let it bound the move pool); this fork's
  **mechanics aren't vanilla**, so re-check any claim against `deterministic.h` /
  `buff.h` (Blizzard is 3 PP here, secondaries need a super-effective hit, paralysis
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

## Verify

After applying changes: `make check TESTS="Frontier extended roster"` and
`make check TESTS="Innate"` (filtered — let CI run the full suite on the PR).
Keep rows in dex order. Update `fork-docs/FORK.md` only if the change warrants an
index entry.
