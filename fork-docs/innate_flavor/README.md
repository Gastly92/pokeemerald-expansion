# Innate flavor sweep

One file per Battle Factory extended-roster species, `<NNNN>_<SPECIES>.md`. Each
records a yes/no, decided by the maintainer, for every implemented innate the
species did not already have, judged against its dex text and its Bulbapedia
biology, anime and manga. The procedure is the `innate-flavor` skill
(`.claude/skills/innate-flavor/SKILL.md`).

The log itself is `fork-docs/innate_flavor.csv`: one row per species and innate,
with `canon` (a canon ability, so required), `had` (an innate before the decision),
`proposed` (Claude's verdict, written before the maintainer decides and not shown to
them), `fit` (the maintainer's decision; `Pending` until made), `reason`, `source` and `decided`. Every verdict is checked
against the dex text and Bulbapedia's Biology, anime and manga sections; `source`
names the sections that support a Yes and is blank for a No.

