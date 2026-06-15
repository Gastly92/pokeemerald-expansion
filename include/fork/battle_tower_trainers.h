#ifndef GUARD_BATTLE_TOWER_TRAINERS_H
#define GUARD_BATTLE_TOWER_TRAINERS_H

#include "constants/trainers.h"

// FORK: the Battle Tower's fork-owned "special" opponents and their fixed,
// hand-authored competitive teams: the Salon Maiden (Frontier Brain) and the
// gym-leader boss roster. The data lives here (not in the upstream frontier files
// frontier_util.c / battle_frontier.c) so it carries no merge-conflict surface;
// small hooks in those files route the tower brain / boss ids to the accessors
// below. See src/battle_tower_trainers.c. Active under B_FRONTIER_EXTENDED_MONS.

// --- Frontier Brain (Anabel) ---
// Build the tower brain's party from its static competitive team into the
// opponent's party. symbol 0 = Silver (50th win), 1 = Gold (100th win).
void FillTowerBrainParty(u32 symbol);

// --- Gym-leader bosses ---
// Boss opponent ids occupy a fork-owned range just above the frontier specials
// (TRAINER_PLAYER), staying in the "Battle Frontier only" id space (< 0xFF00) so
// they never collide with regular gTrainers[] ids.
#define TOWER_BOSS_TRAINER_FIRST   (TRAINER_PLAYER + 1)

u32 GetTowerBossCount(void);
bool32 IsTowerBossTrainerId(u16 trainerId);
// Pick a random boss id (avoiding an immediate repeat of the last one).
u16 ChooseTowerBossTrainerId(void);
// The boss's overworld object-event graphics id (gym-leader sprite).
u8 GetTowerBossObjEventGfx(u16 trainerId);
// The boss's in-battle trainer pic and class (the linked gym leader's).
u8 GetTowerBossTrainerPic(u16 trainerId);
enum TrainerClassID GetTowerBossTrainerClass(u16 trainerId);
// Copy the boss's display name (the linked gym leader's name) into dst.
void GetTowerBossTrainerName(u8 *dst, u16 trainerId);
// Build the boss's fixed competitive party into the opponent's party.
void FillTowerBossParty(u16 trainerId, u8 monCount);
// Buffer the boss's pre-/post-battle line (FRONTIER_*_TEXT) into gStringVar4.
void BufferTowerBossBattleText(u8 whichText, u16 trainerId);

#endif // GUARD_BATTLE_TOWER_TRAINERS_H
