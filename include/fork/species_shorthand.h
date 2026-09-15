#ifndef GUARD_FORK_SPECIES_SHORTHAND_H
#define GUARD_FORK_SPECIES_SHORTHAND_H

#include "constants/species.h"

// FORK: short aliases for upstream species constants whose names are long enough
// to be a nuisance in the fork's hand-edited data tables (species_tiers.c,
// frontier_extended_mons.c, innate_abilities.c). Those tables are edited from a
// phone, where a 34-character token is both painful to type and — being longer
// than every other entry — the one line that breaks a table's comment alignment.
//
// These are aliases, not new ids: each expands to the upstream constant, so the
// two spellings are interchangeable and upstream's name stays the real id. The
// SPECIES_ prefix is kept so the data tables stay visually uniform.
//
// ADD A ROW only when a name is both (a) over ~30 characters and (b) used in a
// hand-edited fork table. A second spelling is a second thing to grep for, so
// this is a short list on purpose — and once a species has an alias here, use it
// at every fork call site rather than mixing the two spellings.
//
// Upstream owns the SPECIES_ namespace, so an alias could in principle collide
// with a constant upstream adds later. That collision is a loud build error (a
// macro rewriting an enum declaration), never silent drift, which is why plain
// #defines are enough here.
//
// PC = Power Construct. Zygarde's Power Construct formes are the competitive ones
// (plain SPECIES_ZYGARDE_50 / _10 carry Aura Break), and "Zygarde-50-PC" /
// "Zygarde-10-PC" is how the competitive scene writes them.
#define SPECIES_ZYGARDE_50_PC SPECIES_ZYGARDE_50_POWER_CONSTRUCT
#define SPECIES_ZYGARDE_10_PC SPECIES_ZYGARDE_10_POWER_CONSTRUCT

#endif // GUARD_FORK_SPECIES_SHORTHAND_H
