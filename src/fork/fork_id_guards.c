// FORK: build-time guards for the ID spaces this fork claims out of upstream's enums
// and flag words. Upstream allocates these UPWARD as it implements things, so a fork ID
// parked just above upstream's high-water mark gets taken sooner or later. When our ID
// lives in a fork-owned header (the AI flags), that collision lands with no merge
// conflict at all -- which is how AI_FLAG_SMART_SPECIES_LOGIC silently became
// AI_FLAG_ABILITY_OMNISCIENCE in the 1.17.0 sync.
//
// These asserts make the next such collision a build error. See CLAUDE.md,
// "ID spaces the fork claims", for the allocation rule each one enforces.

#include "global.h"
#include "constants/battle_ai.h"
#include "constants/abilities.h"
#include "battle.h"
#include "fork/battle_ai_species_overrides.h"
#include "fork/battle_ai_zmove.h"

// --- AI flag bits -----------------------------------------------------------------
// The fork allocates downward from bit 59 (60-63 are upstream's "other" block).
// If upstream's upward allocation ever reaches a fork bit, these fire.
STATIC_ASSERT(AI_FLAG_SMART_SPECIES_LOGIC != AI_FLAG_MOVE_OMNISCIENCE, ForkAiFlagCollidesWithUpstream_SmartSpeciesLogic);
STATIC_ASSERT(AI_FLAG_SMART_Z_MOVE        != AI_FLAG_MOVE_OMNISCIENCE, ForkAiFlagCollidesWithUpstream_SmartZMove);
STATIC_ASSERT(AI_FLAG_SMART_SPECIES_LOGIC != AI_FLAG_SMART_Z_MOVE,     ForkAiFlagsCollideWithEachOther);
// Upstream's highest allocated bit is 36 (AI_FLAG_MOVE_OMNISCIENCE). Keep a wide gap:
// if upstream ever climbs past bit 55 this fires while there is still room to move.
STATIC_ASSERT(AI_FLAG_MOVE_OMNISCIENCE < ((u64)1 << 56), UpstreamAiFlagsAreClosingInOnTheForkBlock);
// The fork's flags must stay below upstream's "other" block at 60-63.
STATIC_ASSERT(AI_FLAG_SMART_SPECIES_LOGIC < AI_FLAG_DYNAMIC_FUNC, ForkAiFlagEnteredUpstreamOtherBlock);
STATIC_ASSERT(AI_FLAG_SMART_Z_MOVE        < AI_FLAG_DYNAMIC_FUNC, ForkAiFlagEnteredUpstreamOtherBlock2);

// --- Ability IDs ------------------------------------------------------------------
// The fork's abilities live in their own block at FORK_ABILITY_BASE and up, clear of
// upstream's growth path (upstream allocates upward and reached 320 in 1.17.0, when
// ABILITY_AURA_GUARD took 319 out from under the fork's old 314/317/320 numbering).
//
// The assert that matters is the first one: it fires the day upstream's own run grows
// far enough to reach our base, which is the only way this scheme can break. When that
// finally happens, raise FORK_ABILITY_BASE — never renumber into upstream's range.
STATIC_ASSERT(ABILITIES_COUNT_GEN9 <= FORK_ABILITY_BASE, UpstreamAbilitiesReachedForkBlock);
STATIC_ASSERT(ABILITY_HALO == FORK_ABILITY_BASE, ForkAbilityBlockNotAtBase);

// The fork block must stay contiguous and ordered, so ABILITIES_COUNT spans all of it.
STATIC_ASSERT(ABILITY_PSYCHIC_AFFINITY == ABILITY_HALO + 1, ForkAbilityBlockNotContiguous1);
STATIC_ASSERT(ABILITY_WATER_AFFINITY == ABILITY_PSYCHIC_AFFINITY + 1, ForkAbilityBlockNotContiguous2);
STATIC_ASSERT(ABILITY_WATER_AFFINITY < ABILITIES_COUNT, ForkAbilityIdOutOfRange);

// gAbilitiesInfo[] is now sparse between upstream's run and the fork block. Anything
// that ITERATES the ability space (rather than indexing a known ability) has to skip
// entries whose .description is NULL — see test/text.c.

// --- Bitfield words the fork steals padding bits from ------------------------------
// Upstream regularly widens fields in these structs and shrinks `padding` to match.
// When a fork bit also lives in the word, the padding number must be recomputed rather
// than taken from either side of the merge -- and when both sides happen to land on the
// same number, git merges it cleanly and the word overflows. These size asserts are the
// backstop for that (a GBA int is 4 bytes; each word below must not grow).
STATIC_ASSERT(sizeof(struct SpecialStatus) % 4 == 0, SpecialStatusBitfieldOverflowed);
STATIC_ASSERT(sizeof(struct PartyState) % 4 == 0, PartyStateBitfieldOverflowed);
