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
4. **Present proposals before editing.** Lay out findings + a specific proposal
   per file and get the maintainer's yes/no/swaps first — flavor picks are their
   call. Then apply the approved changes.

## Hard constraints (see the rubric for detail)

- **Innates:** only *already-implemented* abilities (the `sImplementedInnates[]`
  allowlist in `test/fork/innate_abilities.c`) — an unwired pick fails CI. Don't
  wire new abilities in a line review.
- **Overrides:** the chosen ability must **not** duplicate a species innate and
  must be a *stable* pick (`:x:` never-an-innate in
  `fork-docs/INNATE_ABILITIES_PROGRESS.md`, or an implemented innate the species
  doesn't carry). Only repurpose a *real* slot that's redundant and not
  test-pinned; filling an empty slot is always safe.
- **Frontier movesets:** no move restrictions — anything flavorful or powerful.
  The set's `.ability` must resolve to a real slot and not be an innate (or use
  `ABILITY_NONE`).

## Verify

After applying changes: `make check TESTS="Frontier extended roster"` and
`make check TESTS="Innate"` (filtered — let CI run the full suite on the PR).
Keep rows in dex order. Update `fork-docs/FORK.md` only if the change warrants an
index entry.
