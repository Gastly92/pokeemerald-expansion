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
// The tiers are this fork's own power bands, NOT the official Game Freak
// categories — official Mythicals are split across both legend tiers by strength,
// and the official pseudo-legendaries are not all TIER_PSEUDO:
//   - TIER_MYTHICAL  : the restricted box/cover legends (Mewtwo, Lugia/Ho-Oh, the
//                      weather trio, Dialga/Palkia/Giratina, Reshiram/Zekrom,
//                      Zacian/Zamazenta, Koraidon/Miraidon, ...), their fused or
//                      Origin/Crowned formes, plus the strongest Mythicals
//                      (Arceus, Darkrai, Shaymin-Sky, Deoxys' battle formes).
//   - TIER_LEGENDARY : everything else legend-grade — the sub-legendaries (birds,
//                      beasts, lake trio, Regis, genies, musketeers, Tapus,
//                      Treasures of Ruin, Loyal Three, Ogerpon, ...), the other
//                      Mythicals (Mew, Celebi, Jirachi, Diancie, Magearna,
//                      Pecharunt, ...), the Ultra Beasts, the Paradox Pokemon, and
//                      a few non-legends strong enough to share the quota
//                      (Dragapult, Baxcalibur, Bloodmoon Ursaluna).
//   - TIER_PSEUDO    : non-legendary standouts capped at one per team — some
//                      600-BST pseudo-legendaries (Dragonite, Garchomp, Metagross,
//                      ...) alongside other top picks (Gengar, Lucario, the fossil
//                      quartet, Gholdengo, Archaludon, ...).
//   - TIER_NORMAL    : everything else (the default; not stored in the table).
//
// The table is keyed by EXACT species id, so each forme is classified on its own
// merits rather than inheriting a single tier from its base species' Pokedex
// number. This lets a powerful forme outrank its base — Shaymin-Sky is
// TIER_MYTHICAL while ordinary Shaymin is TIER_LEGENDARY — and a weak base sit
// below its formes — base Calyrex is TIER_LEGENDARY while its Ice/Shadow riders
// are TIER_MYTHICAL. List every forme you want classified; anything not listed
// is TIER_NORMAL.
//
// CAVEAT — an omitted forme is not "inherits its base's tier", it is TIER_NORMAL,
// i.e. draftable into any party slot with no quota. Every forme of a restricted
// species that the extended roster can draft must therefore appear here in its own
// right: leaving the 17 Arceus plate formes out let a TIER_MYTHICAL mon into
// ordinary battles. Deliberate divergences only ever make a forme MORE restricted
// than its base, so test/fork/species_tiers.c walks the roster and fails on any
// forme that is less restricted than its base forme.
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

#if TESTING
// Test-only: the per-tier arrays are static, so GetSpeciesTier's first-match
// priority (Mythical > Legendary > Pseudo) would silently mask a species
// accidentally listed twice (within one array or across two). Returns TRUE
// and writes the offending species to *outSpecies if any duplicate exists.
bool32 SpeciesTierListsOverlap(u16 *outSpecies);

// Test-only: the tier arrays are kept in ascending National Dex order so a new
// row has one obvious home and a duplicate is visible by eye, but nothing in
// the data itself enforces that. Returns TRUE and writes the first row that
// sits below its predecessor (plus that predecessor) to *outSpecies /
// *outPrevSpecies. Sibling formes share a dex number, so ties pass.
// NOTE: this checks the real dex number, not the `// 0901`-style comment on
// each row -- a comment that disagrees with the species it labels still passes
// as long as the row itself is in the right place.
bool32 SpeciesTierListIsUnsorted(u16 *outSpecies, u16 *outPrevSpecies);
#endif

#endif // GUARD_SPECIES_TIERS_H
