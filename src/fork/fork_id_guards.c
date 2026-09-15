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
// These three sit inside upstream's growth range: 314 and 317 are still upstream's
// ABILITY_314 / ABILITY_317 placeholders, and 320 is the next slot upstream will fill.
// They are expected to conflict on sync; resolve by renumbering OURS. The asserts below
// catch the case where a resolution accidentally aliases a fork ability onto an
// upstream one (which the compiler would otherwise accept silently, since a duplicated
// designated initializer in gAbilitiesInfo[] is legal C).
STATIC_ASSERT(ABILITY_HALO != ABILITY_PSYCHIC_AFFINITY, ForkAbilityIdsCollide_HaloPsychicAffinity);
STATIC_ASSERT(ABILITY_HALO != ABILITY_WATER_AFFINITY,   ForkAbilityIdsCollide_HaloWaterAffinity);
STATIC_ASSERT(ABILITY_PSYCHIC_AFFINITY != ABILITY_WATER_AFFINITY, ForkAbilityIdsCollide_Affinities);
STATIC_ASSERT(ABILITY_WATER_AFFINITY != ABILITY_AURA_GUARD, ForkAbilityIdCollidesWithUpstream_AuraGuard);
STATIC_ASSERT(ABILITY_HALO < ABILITIES_COUNT && ABILITY_WATER_AFFINITY < ABILITIES_COUNT, ForkAbilityIdOutOfRange);

// --- Bitfield words the fork steals padding bits from ------------------------------
// Upstream regularly widens fields in these structs and shrinks `padding` to match.
// When a fork bit also lives in the word, the padding number must be recomputed rather
// than taken from either side of the merge -- and when both sides happen to land on the
// same number, git merges it cleanly and the word overflows. These size asserts are the
// backstop for that (a GBA int is 4 bytes; each word below must not grow).
STATIC_ASSERT(sizeof(struct SpecialStatus) % 4 == 0, SpecialStatusBitfieldOverflowed);
STATIC_ASSERT(sizeof(struct PartyState) % 4 == 0, PartyStateBitfieldOverflowed);
