#ifndef GUARD_FRONTIER_LEGALITY_H
#define GUARD_FRONTIER_LEGALITY_H

#include "pokemon.h"

// FORK: single chokepoint for the per-species "banned in the Battle Frontier"
// check. Upstream reads gSpeciesInfo[species].isFrontierBanned directly at every
// frontier-eligibility site; we funnel those reads through this helper so the
// B_FRONTIER_ALL_SPECIES_LEGAL flag (config/frontier.h) can ignore the banned
// flag in one place. When the flag is TRUE (the fork default) every species is
// legal in the Battle Frontier facilities, so this always returns FALSE; when
// FALSE it falls back to the vanilla per-species data.
//
// Pass whatever species id the call site was indexing gSpeciesInfo with (a base
// species, GET_BASE_SPECIES_ID(...), etc.) — this helper does no resolution of
// its own.
static inline bool32 IsSpeciesFrontierBanned(u32 species)
{
#if B_FRONTIER_ALL_SPECIES_LEGAL
    return FALSE;
#else
    return gSpeciesInfo[species].isFrontierBanned;
#endif
}

#endif // GUARD_FRONTIER_LEGALITY_H
