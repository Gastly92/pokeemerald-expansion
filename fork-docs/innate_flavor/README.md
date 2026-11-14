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

## Rulings that carry across species

Decisions on one species that set how an innate is read everywhere. Apply them when
the innate comes up again, and revisit existing holders when their species is reached.

- **Flower Veil** — the canon holders *are* flowers (Floette, Comfey). Bearing a
  flower (Venusaur) matches the name, not the flavor.
- **Friend Guard** — a passive aura, like Magic Guard, from small, happy creatures
  like the canon holders (Clefairy, Jigglypuff, Happiny, Maushold). Guardians who
  actively protect allies (with vines, shells, bodies) are not this. Many current
  holders came in on the guardian reading; correct them as their species come up.
