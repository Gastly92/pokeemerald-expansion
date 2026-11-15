# Innate flavor sweep

A slow, maintainer-driven review of the innates on every Battle Factory extended-roster
species. One species at a time, one innate at a time. Claude gives evidence and a blind
verdict, the maintainer decides, and every decision is written to a log. A new
conversation can pick it up from this page alone.

## Files

| File | What it holds |
| --- | --- |
| `fork-docs/innate_flavor.csv` | The decision log: one row per species and innate. |
| `fork-docs/innate_flavor/excluded.csv` | Innates the sweep skips, with the reason (a CI gate already covers them). |
| `fork-docs/innate_flavor/FOLLOWUPS.md` | Open questions to settle after a species, not during it. |
| `.claude/skills/innate-flavor/tools/next.py` | Progress, the next species and innate, and the lock check. |
| `.claude/skills/innate-flavor/tools/episodes.py` | Plot paragraphs from a species' major anime episodes and movies. |
| `.claude/skills/line-review/tools/bulbapedia.py` | Bulbapedia's Biology, anime and manga sections. |
| `.claude/skills/line-review/tools/users.py` | The canon users of an ability. |

## Log columns

`species,innate,canon,had,proposed,fit,reason,source,decided`

- **canon:** Yes if the innate is one of the species' own canon abilities. A canon
  innate is required by the completeness test, so it is recorded as Yes without a turn.
- **had:** whether the species carried the innate before the decision.
- **proposed:** Claude's verdict, written before the maintainer answers and never shown
  to them.
- **fit:** the maintainer's decision, `Pending` until made. `had` → `fit` reads as add
  (No → Yes), remove (Yes → No), keep (Yes → Yes) or nothing (No → No).
- **reason:** a short reason. When the maintainer disagrees with the proposal, it is
  their reason, in their words.
- **source:** for a Yes, the sources that support it (`dex`, `biology`, `anime`,
  `manga`, or another medium the maintainer names). Blank for a No: every verdict is
  checked against all of them, and a No found nothing.
- **decided:** the date of the decision.

## The pool and the order

- **Species:** the extended roster (`src/fork/frontier_extended_mons.c`), in the order
  each `.species` first appears. Pre-evolutions are not reviewed on their own.
- **Innates:** `sImplementedInnates[]` in `test/fork/innate_abilities.c`, minus
  `excluded.csv`, in alphabetical order of in-game name ("Rock Head" before "Rocky
  Payload"). Innates the species already has are included: a No on one of them means
  remove it.
- `next.py` computes both. Never work from a remembered list.

## Starting a species

1. `python3 -I .claude/skills/innate-flavor/tools/next.py` gives the next species and
   innate.
2. Gather the evidence once, before the first innate:
   - the repo `.description` (dex text) in `src/data/pokemon/species_info/*.h`;
   - `python3 -I .claude/skills/line-review/tools/bulbapedia.py <SPECIES>`;
   - `python3 -I .claude/skills/innate-flavor/tools/episodes.py <SPECIES>`. It caps at 20
     episodes and lists the rest; fetch more with `--max` when they matter.
3. Do not look up earlier decisions on the same innate for other species. Each species
   is judged on its own evidence.

## Each innate, one per turn

1. Check what the ability does **in this fork**: `fork-docs/DETERMINISM.md`, the fork's
   innate notes in `fork-docs/INNATE_ABILITIES.md`, `fork-docs/NEW_ABILITIES.md` for
   fork-only abilities, and the code when those are unclear. Describe the fork's
   behaviour, not vanilla's.
2. Find who has it: `users.py <ABILITY>` for canon holders, and a grep of
   `src/fork/innate_abilities.c` for innate holders. Summarize what kind of creature the
   family is.
3. Write Claude's verdict into the log first, as a row with `fit = Pending`, then commit
   and push.
4. Present to the maintainer, with no verdict:
   - what the ability does in this fork;
   - canon and had;
   - who has it and what the family is;
   - **relevant snippets** only: quoted dex, Biology, episode and manga lines that bear
     on this ability, each naming its source. If nothing is relevant, say "none". A line
     that only shares a word with the ability is not relevant.
5. When the maintainer answers, set `fit` and `decided`. Say whether their verdict
   matched the proposal. If it did not, ask for their reason in a few words if they did
   not give one, and record it. Commit, push, and present the next innate.

## Finishing a species

1. `next.py <SPECIES>` must show 0 left.
2. Apply any changed decisions to `src/fork/innate_abilities.c` with
   `python3 .claude/skills/line-review/tools/inn.py`, and say which rows changed.
   Decisions are made for the roster species. Writing them to pre-evolutions, Megas or
   other forms is a follow-up for the maintainer to decide.
3. Run `next.py --check` and `make check TESTS="Innate"`.
4. Summarize the species: what it keeps, adds and loses, every disagreement with both
   sides, and any follow-ups.

## The lock

`next.py --check` treats every species whose sweep is complete as locked. It fails when:

- the species' row in `src/fork/innate_abilities.c` differs from the log's Yes
  decisions. To change a locked row, change the log first; or
- the pool has an innate the species has no decision for, for example one wired after
  its review. Assess that innate for the species and the check passes again.

CI runs it on every pull request and every push to `master`, in the fork-owned
`.github/workflows/fork-locks.yml`.

## Excluded innates

`excluded.csv` lists innates that need no per-species decision because a CI gate already
assigns them. Excluded innates get no rows. Add one only when a test enforces exactly who
carries it.
