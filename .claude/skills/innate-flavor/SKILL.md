---
name: innate-flavor
description: Runs the innate flavor sweep, a maintainer-driven review of the innates on every Battle Factory extended-roster species, one species at a time and one implemented innate per turn, with Claude's verdict kept blind and every decision logged in fork-docs/innate_flavor.csv. Use when the user asks to start, continue or resume the flavor sweep, move to the next species or innate, or run it on a named species. Separate from line-review; it touches innates only and the maintainer makes every call.
---

# Innate flavor sweep

The maintainer decides; Claude gathers evidence, writes a blind verdict and keeps the log.
Log columns, the exclusion list and the lock are described in [reference.md](reference.md).

## Rules

- One innate per turn. Never show Claude's verdict before the maintainer answers.
- Judge each species on its own evidence. Do not look up how other species were decided.
- Describe what an ability does **in this fork**, not in vanilla.
- Commit and push after every logged change; the container is ephemeral.
- Do not edit `src/fork/innate_abilities.c` until the species is finished.

## Start or resume

1. Run `python3 -I .claude/skills/innate-flavor/tools/next.py`. It prints the next species and
   innate, computed from the roster, the pool and the log. Never work from a remembered list.
2. If the log has a `Pending` row, that innate is waiting for the maintainer: present it again.
3. For a new species, gather its evidence once:
   - the dex text (`.description`) in `src/data/pokemon/species_info/*.h`;
   - `python3 -I .claude/skills/line-review/tools/bulbapedia.py <SPECIES>` (Biology, anime, manga);
   - `python3 -I .claude/skills/innate-flavor/tools/episodes.py <SPECIES>` (major episode and movie
     plots; capped at 20, use `--max` for more).
4. A canon innate (one of the species' own abilities) is required by the completeness test.
   Log it as `Yes` without a turn and move on.

## Each innate

1. Check the fork's behaviour: `fork-docs/DETERMINISM.md`, `fork-docs/INNATE_ABILITIES.md`,
   `fork-docs/NEW_ABILITIES.md` for fork-only abilities, and the code when those are unclear.
2. Find the holders: `python3 .claude/skills/line-review/tools/users.py <ABILITY>` for canon users,
   and a grep of `src/fork/innate_abilities.c` for innate holders. Name what kind of creature the
   family is.
3. Append the row with Claude's verdict in `proposed` and `fit = Pending`. Commit and push.
4. Present, with no verdict:
   - what it does in this fork;
   - canon and had;
   - who has it, and what the family is;
   - **relevant snippets**: quoted dex, Biology, episode and manga lines that bear on this
     ability, each naming its source, or "none". A line that only shares a word with the
     ability is not relevant.
5. When the maintainer answers, set `fit` and `decided`, and say whether it matched the proposal.
   On a disagreement, record the maintainer's reason, asking for a few words if none was given.
   Commit, push, and present the next innate.

## Finish a species

1. `next.py <SPECIES>` shows 0 left.
2. Apply changed decisions to `src/fork/innate_abilities.c` with
   `python3 .claude/skills/line-review/tools/inn.py`. Writing a decision to pre-evolutions or forms
   is the maintainer's call; note open questions in `fork-docs/innate_flavor/FOLLOWUPS.md`.
3. Run `next.py --check` and `make check TESTS="Innate"`.
4. Summarize: what the species keeps, gains and loses, every disagreement with both sides, and any
   follow-ups.
