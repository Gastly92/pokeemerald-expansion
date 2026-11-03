#ifndef GUARD_FORK_FRONTIER_POKEDEX_H
#define GUARD_FORK_FRONTIER_POKEDEX_H

// Frontier battles fill in the Pokédex (FRONTIER_MARKS_DEX_SEEN, config/fork.h).
//
// Upstream's end-of-battle "mark seen" loop in HandleEndTurn_FinishBattle
// (src/battle_main.c) excludes BATTLE_TYPE_FRONTIER, and skips the player's own side.
// In this hack every battle is a Frontier battle and the player's team is usually rented,
// so without this the Pokédex would never fill. MarkFrontierBattleMonsSeen runs right
// after that loop and marks every mon that was sent out, on both sides, as SEEN (never
// CAUGHT: rentals and opponents are not the player's).

// TRUE for a Frontier battle the player actually fought: not a link battle and not a
// recorded-battle playback (a record mixed in from another save would otherwise mark
// species the player never faced).
bool32 ShouldFrontierBattleMarkDexSeen(u32 battleTypeFlags);

// Marks SEEN every mon in `party` whose `states` entry has sentOut set.
void MarkSentOutMonsDexSeen(struct Pokemon *party, const struct PartyState *states);

// The hook: does both of the above for every trainer in the battle that just ended.
void MarkFrontierBattleMonsSeen(void);

#endif // GUARD_FORK_FRONTIER_POKEDEX_H
