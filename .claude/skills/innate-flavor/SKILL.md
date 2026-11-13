---
name: innate-flavor
description: Run the innate flavor sweep — one Battle Factory extended-roster species at a time, judge every implemented innate it does not already have against its Bulbapedia biology and media, let the maintainer decide each candidate, and record a yes/no for every innate in fork-docs/innate_flavor/. Triggers when the user asks for the flavor sweep, the "next species" in it, or to run it on a named species. Separate from line-review: it touches innates only, and the maintainer makes every call.
---

# Innate flavor sweep

A slow, maintainer-driven pass: **one species per turn**, every decision recorded.
Not a line review — no overrides, no frontier sets, no batching, and nothing is
applied until the maintainer has decided.

## Scope

- **Species:** the extended roster (`src/fork/frontier_extended_mons.c`), in the
  order its distinct `.species` first appear (ascending dex). Pre-evolutions are not
  reviewed on their own.
- **Pool:** `sImplementedInnates[]` in `test/fork/innate_abilities.c`, minus the
  innates the species already has. Don't wire new abilities here.
- **Many innates is fine.** There is no cap; don't reject a fit to keep a row short.
- **One PR per generation** (I #1-151 … IX #906-1025), branch
  `claude/innate-flavor-gen<N>` (or the branch the session designates), one commit
  per species.

## Per species

1. **Pick the species.** The next roster species with no file in
   `fork-docs/innate_flavor/`, unless the maintainer names one. Resolve it with
   `python3 -I .claude/skills/line-review/tools/forms.py <NAME>`.
2. **Gather evidence:**
   - `python3 -I .claude/skills/innate-flavor/tools/catalog.py <NAME>` — current
     innates, canon abilities, and every remaining innate with its description and
     canon-user / innate-holder counts.
   - The repo `.description` (dex text) from `src/data/pokemon/species_info/*.h`.
   - `python3 -I .claude/skills/line-review/tools/bulbapedia.py <NAME>` — Biology,
     Forms, anime, manga. Cite it as Bulbapedia with the episode/chapter and medium.
   - For any candidate, `python3 .claude/skills/line-review/tools/users.py <ABILITY>`
     to see who has it canonically, and grep `src/fork/innate_abilities.c` for who
     carries it as an innate — the family it would be joining.
3. **Present, don't apply.** In chat, give the maintainer:
   - a short evidence summary (dex text + the Bulbapedia actions that matter);
   - **proposed yes** — each with the specific evidence and the family it joins;
   - **near misses** — plausible but proposed no, each with a one-line reason;
   - one line saying every other innate is a proposed **no** with no flavor link,
     and that any of them can be pulled out for discussion.

   Then stop and wait. The maintainer decides each yes and near miss.
4. **Record.** Write `fork-docs/innate_flavor/<NNNN>_<SPECIES>.md` from the
   template below: **every** innate in the pool gets a row with Yes or No. Yes rows
   and discussed noes carry a reason; an undiscussed no may leave the reason blank.
   Mark who decided only where the maintainer overrode the proposal.
5. **Apply the yeses** to `src/fork/innate_abilities.c` on the species **and its whole
   line and forms** (pre-evolutions, regional forms that share the flavor, Megas /
   G-Max that don't inherit automatically) — `python3 -I .claude/skills/line-review/tools/inn.py`
   edits rows. Say in the record which rows were written. No comments in the data file.
6. **Commit and push** — subject `Innate flavor: <Species> — <added innates | no changes>`.
   Then offer the next species.

Run `make check TESTS="Innate"` after the first species of a generation and before
opening the PR; a new innate can collide with a species ability override, and that
test names the row to convert.

## Record template

```markdown
# <Species> (#<NNNN>)

**Before:** <innates>  ·  **After:** <innates>  ·  **Rows written:** <species list | none>

## Evidence
- **Dex:** "<repo .description>"
- **Bulbapedia (<medium>, <episode/chapter>):** <the action that matters>

## Decisions
| Innate | Fit | Reason |
| --- | --- | --- |
| HEALER | Yes | ... |
| HARVEST | No | ... |
| ADAPTABILITY | No | |
```

Order the table: Yes, then discussed No, then the remaining No alphabetically.
