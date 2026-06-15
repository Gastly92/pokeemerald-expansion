# New types — how to re-type a species

Fork feature, gated by `FEATURE_NEW_TYPES` (`include/config/feature.h`). When on,
a species' types are overwritten with a fork-defined typing in place of the stock
`gSpeciesInfo` types. This doc is the extension playbook; the flag comment and the
in-code comments (`include/fork/new_types.h`, `src/fork/new_types.c`) stay the source of
truth for exact semantics.

## How it works: one chokepoint

Every species type-identity read in the codebase routes through one accessor,
`GetSpeciesType(species, slot)` (`src/pokemon.c`):

- battle setup fills `gBattleMons[battler].types[]` from it, so type
  matchups/STAB use the override;
- the type icons, the Pokédex, and the summary screen read it;
- `IsSpeciesOfType()`, `GetTeraTypeFromPersonality()`, and the
  `MON_DATA_TERA_TYPE` default were routed through it too, so type *identity* is
  uniform.

So the override is applied in exactly one place — inside `GetSpeciesType()` — and
flows everywhere. The fork-owned table (`src/fork/new_types.c`) is **not** in
`gSpeciesInfo`, so upstream syncs never touch it and the upstream species data
stays untouched.

The in-battle *dynamic* type (`gBattleMons[].types`, which Soak / Reflect Type /
Roost / etc. mutate mid-battle) simply starts from this override; nothing else in
the battle engine needs to change.

## Recipe: "re-type species X to A/B"

### Step 1 — add the data (the only required step)

In `src/fork/new_types.c`, add a row to `sSpeciesTypeOverrides`. A pure (single-type)
species repeats the type in both slots, mirroring `MON_TYPES()`:

```c
static const struct SpeciesTypeOverride sSpeciesTypeOverrides[] =
{
    // ... existing rows ...
    { SPECIES_X,      { TYPE_A, TYPE_B } }, // dual-type
    { SPECIES_Y,      { TYPE_A, TYPE_A } }, // pure type
};
```

Forms are independent species, so each form you want re-typed (base, regional,
mega, …) needs its own row.

### Step 2 — test it

Add a case to `test/fork/new_types.c`. Opt into the feature with
`WITH_CONFIG(FEATURE_NEW_TYPES, TRUE)` (the test baseline forces all `FEATURE_*`
flags off, so the inherited suite keeps exercising stock behavior). The cleanest
probes are defensive matchup *flips* the new typing creates versus the old one
(a new immunity, a new weakness), asserted off the battle messages, plus an
`ASSUME(GetSpeciesTypeOverride(...))` to lock the table data. Cover both the
feature-on behavior and that the feature-off baseline keeps stock typing. Run:

```bash
make -j$(nproc) check TESTS="FEATURE_NEW_TYPES"
```

### Step 3 — update the index

Edit the **New types** row in `FORK.md` if the change is notable (the set of
re-typed species, any limitation).

## Scope / known limitations

- The override is keyed on **species**, not on an individual mon, so it re-types
  every instance of that species (wild, trainer, player). That is the intent.
- It changes *type identity* only. It does not touch the species' movepool,
  abilities, or learnset; pair a re-typing with other changes if the species
  should also get matching STAB moves.
