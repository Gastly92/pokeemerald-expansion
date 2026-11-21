# Innate flavor sweep: reference

## Files

| File | Holds |
| --- | --- |
| `fork-docs/innate_flavor.csv` | The decision log, one row per species and innate. |
| `excluded.csv` | Innates the sweep skips, each with the CI gate that already covers it. |
| `tools/next.py` | Progress, the next species and innate, and the lock check. |
| `tools/episodes.py` | Plot paragraphs from a species' major anime episodes and movies. |

## Log columns

`species,innate,canon,had,proposed,fit,reason,source,decided`

- **canon**: Yes if the innate is one of the species' own canon abilities.
- **had**: whether the species carried it before the decision.
- **proposed**: Claude's verdict, written before the maintainer answers and never shown to them.
- **fit**: the maintainer's decision; `Pending` until made. Read with `had`: No → Yes adds,
  Yes → No removes, Yes → Yes keeps, No → No changes nothing.
- **reason**: a short reason. On a disagreement, the maintainer's reason in their words.
- **source**: for a Yes, the sources that support it (`dex`, `biology`, `anime`, `manga`, or
  another medium the maintainer names). Blank for a No.
- **decided**: the date of the decision.

## The pool and the order

- **Species**: the extended roster (`src/fork/frontier_extended_mons.c`), in the order each
  `.species` first appears. Pre-evolutions are not reviewed on their own.
- **Innates**: `sImplementedInnates[]` in `test/fork/innate_abilities.c` minus `excluded.csv`,
  sorted by in-game name ("Rock Head" before "Rocky Payload"). Innates the species already has
  are included; a No on one means remove it.

## Exclusions

`excluded.csv` lists innates that need no per-species decision because a test already decides
who carries them: legendary signatures and single-group abilities, each with its test named in the file. Excluded innates get no rows. Add one only
when a test enforces exactly who carries it.

## The lock

`next.py --check` treats every species whose sweep is complete as locked, and fails when:

- its row in `src/fork/innate_abilities.c` differs from its `fit = Yes` decisions, ignoring
  excluded innates, which their holder tests decide. To change a
  locked row, change the log first; or
- the pool has an innate it has no decision for, such as one wired after its review. Assessing
  that innate for the species makes the check pass again.

CI runs it on every pull request and push to `master` in `.github/workflows/fork-locks.yml`, a
required status check.
