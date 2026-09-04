#include "global.h"
#include "fork/frontier_ai.h"
#include "fork/battle_ai_species_overrides.h" // AI_FLAG_SMART_SPECIES_LOGIC for B_FRONTIER_HARD_AI_FLAGS
#include "fork/battle_ai_zmove.h"             // AI_FLAG_SMART_Z_MOVE for B_FRONTIER_HARD_AI_FLAGS
#include "fork/battle_tower_trainers.h"       // IsTowerBossTrainerId
#include "constants/battle_ai.h"
#include "constants/trainers.h"

// FORK: which AI preset a Battle Frontier opponent runs, chosen by opponent role
// rather than by facility or win streak.
//
// The fork used to hand B_FRONTIER_HARD_AI_FLAGS to every Battle Factory
// opponent alike, which made the strongest AI in the game the *baseline* for a
// run — a boss battle then played no better than the routine fight before it.
// Now the top preset is reserved for the fights meant to be a wall (the Frontier
// Brain, the Tower's gym-leader bosses) and everything else runs one tier down.
//
// The two presets themselves are B_FRONTIER_HARD_AI_FLAGS and
// B_FRONTIER_REGULAR_AI_FLAGS in config/frontier.h; tune difficulty there, and
// change *who counts as a boss* here.

bool32 IsFrontierBossTrainer(u16 trainerId)
{
    // The Frontier Brain id is shared by every facility's Brain (the Factory
    // Head, the Salon Maiden, ...); the Tower's gym-leader bosses sit in the
    // fork-owned id range above TRAINER_PLAYER.
    return trainerId == TRAINER_FRONTIER_BRAIN || IsTowerBossTrainerId(trainerId);
}

u64 GetFrontierAiFlags(u16 trainerId)
{
    if (IsFrontierBossTrainer(trainerId))
        return B_FRONTIER_HARD_AI_FLAGS;

    return B_FRONTIER_REGULAR_AI_FLAGS;
}
