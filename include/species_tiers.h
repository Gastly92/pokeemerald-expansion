#ifndef GUARD_SPECIES_TIERS_H
#define GUARD_SPECIES_TIERS_H

// FORK: fork-owned species -> "tier" classification map (src/species_tiers.c).
//
// A small data table tagging each legendary / mythical / pseudo-legendary species
// with a tier, so facility logic can control *what appears where* (e.g. keep the
// strongest restricted legendaries out of a low-stakes pool, or build a
// legendary-only challenge). Kept in a fork-owned file rather than in gSpeciesInfo
// so upstream syncs never touch it and the upstream species data stays untouched.
//
// The tiers use this fork's own definitions (NOT the official Game Freak
// categories — e.g. Groudon is grouped with Arceus as TIER_MYTHICAL here):
//   - TIER_MYTHICAL  : the strongest "restricted" legends — box/cover legendaries
//                      (Groudon, Kyogre, Dialga, Zacian, Koraidon, ...), Mewtwo,
//                      Lugia/Ho-Oh, plus the official event Mythicals (Mew, Celebi,
//                      Jirachi, Arceus, Diancie, Magearna, Pecharunt, ...).
//   - TIER_LEGENDARY : the sub-legendaries — legendary birds/beasts, the lake trio,
//                      Regis, the genies (+Therian), musketeers, Tapus, Heatran,
//                      Cresselia, Urshifu, Ogerpon, the Loyal Three, ...
//   - TIER_PSEUDO    : non-legendary "almost-legendary" power tier — the 600-BST
//                      pseudo-legendaries (Dragonite, Garchomp, Dragapult, ...),
//                      the Ultra Beasts, the Paradox Pokemon, and the Treasures of
//                      Ruin.
//   - TIER_NORMAL    : everything else (the default; not stored in the table).
//
// The table is keyed by EXACT species id, so each forme is classified on its own
// merits rather than inheriting a single tier from its base species' Pokedex
// number. This lets a powerful forme outrank its base — Shaymin-Sky is
// TIER_LEGENDARY while ordinary Shaymin is TIER_NORMAL (simply absent from the
// table) — and a weak base sit below its formes — base Calyrex is TIER_NORMAL
// while its Ice/Shadow riders are TIER_MYTHICAL. List every forme you want
// classified; anything not listed is TIER_NORMAL.
//
// SCOPE: the table currently covers the species/formes used by the extended
// frontier roster (src/frontier_extended_mons.c). Species not listed return
// TIER_NORMAL. Add a row to extend coverage.

enum SpeciesTier
{
    TIER_NORMAL = 0,
    TIER_LEGENDARY,
    TIER_MYTHICAL,
    TIER_PSEUDO,
};

// Returns the tier of `species` (resolving its forme to the base species'
// National Dex number), or TIER_NORMAL if the species is not classified.
enum SpeciesTier GetSpeciesTier(u16 species);

// Convenience predicate: TRUE if `species` is classified as exactly `tier`.
bool32 SpeciesIsTier(u16 species, enum SpeciesTier tier);

#endif // GUARD_SPECIES_TIERS_H
