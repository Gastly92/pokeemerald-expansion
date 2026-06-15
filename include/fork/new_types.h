#ifndef GUARD_NEW_TYPES_H
#define GUARD_NEW_TYPES_H

// FORK: new typings (FEATURE_NEW_TYPES, config/feature.h).
//
// Some species' types are overwritten with a fork-defined typing. The
// species->types mapping lives in a fork-owned table (src/new_types.c) rather
// than in gSpeciesInfo, so it never conflicts on upstream sync and leaves the
// upstream species data untouched.
//
// The override is applied at the single canonical type accessor
// GetSpeciesType() (src/pokemon.c): every species type-identity read in the
// codebase routes through it (battle setup fills gBattleMons[].types from it,
// and the type icons / Pokédex / summary screen / IsSpeciesOfType all consult
// it), so re-typing here flows everywhere with one hook. The in-battle dynamic
// type (gBattleMons[].types, mutated by Soak/Reflect Type/Roost/etc.) starts
// from this override and is otherwise untouched.
//
// This header exposes only the raw data lookup; the feature-flag gate lives at
// the GetSpeciesType() call site (callers read GetConfig(FEATURE_NEW_TYPES)).

// If `species` has a fork type override, writes its type for `slot` (0 or 1)
// into *outType and returns TRUE; otherwise returns FALSE and leaves *outType
// untouched (the caller keeps the stock gSpeciesInfo type). Pure data lookup: it
// does not consider battle state and does not check the feature flag.
bool32 GetSpeciesTypeOverride(enum Species species, u8 slot, enum Type *outType);

#endif // GUARD_NEW_TYPES_H
