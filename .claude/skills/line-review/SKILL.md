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
3. **For each file, per the rubric:** report what's already there, whether it
   makes flavorful sense, and concrete candidate additions/changes.
4. **Present proposals before editing, then WAIT for a yes.** Lay out findings + a
   specific proposal per file and get the maintainer's yes/no/swaps first — flavor
   picks are their call. Then apply the approved changes. A **deferral is not an
   approval**, an **unanswered question is not a yes**, and "let's return to the
   line review" means resume the *review*, not ship the backlog. Expect most first-
   pass flavor picks to be rejected — that's the process working.

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
- **Overrides:** the chosen ability must **not** duplicate a species innate and
  must be a *stable* pick (`:x:` never-an-innate in
  `fork-docs/INNATE_ABILITIES_PROGRESS.md`, or an implemented innate the species
  doesn't carry). Only repurpose a *real* slot that's redundant and not
  test-pinned; filling an empty slot is always safe.
- **Megas whose ability is an innate:** when a Mega's own ability is an
  implemented innate, give the Mega that ability as an *innate* (Step 1) and
  override *every* real slot of the Mega form to the **base form's** chosen
  override ability, so the base's observable trait carries through the
  transformation (the Venusaur pattern: base → Grassy Surge override; Mega →
  Thick Fat innate + Grassy Surge override). See the rubric Step 2, point 4.
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
  Aim for **at least 2 quality sets per species**, each filling a different niche —
  a bar, not a quota. More is welcome when each set earns its place; two excellent
  sets beat five where three are filler. **Cover both formats** across the line's sets
  (`FORMAT_SINGLES` / `FORMAT_DOUBLES` / `FORMAT_BOTH`) — doubles sets can lean on
  redirection/support (Rage Powder, Follow Me, Helping Hand, Fake Out, spread
  moves), singles sets want self-sufficiency. Held items are **one lens among
  several** (iterate items for fun ideas, but many items are weak and not worth
  building around — a set can start from a move, ability, or gimmick just as
  well). Account for the fork's `DETERMINISTIC_*` flags and `BUFF_*` item
  improvements (Shell Bell, Leech Seed) when choosing moves and items —
  **deterministic does NOT mean "always happens"**: secondary effects land only on
  a super-effective hit (or STAB for Normal moves), crit *stages* stay dead while
  crit *items* give one guaranteed crit, and paralysis loses full-para and the
  Speed drop entirely. Read the "`DETERMINISTIC_*` regime" section of the rubric —
  and `include/config/deterministic.h` itself, which is the source of truth —
  before building a set around any mechanic. The set's `.ability` must resolve to a real
  slot and not be an innate (or use `ABILITY_NONE`).
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
