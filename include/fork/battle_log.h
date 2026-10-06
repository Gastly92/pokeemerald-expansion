#ifndef GUARD_FORK_BATTLE_LOG_H
#define GUARD_FORK_BATTLE_LOG_H

// FORK: B_FRONTIER_BATTLE_INFO -- the INFO viewer's Battle Log page: every move used this
// battle and the damage it dealt, as a % of the target's max HP so the log never states a
// foe's HP outright. Recorded by src/fork/battle_log.c from two hooks in
// src/battle_move_resolution.c; drawn by src/fork/frontier_battle_info.c.
// See fork-docs/BATTLE_INFO.md, "The Battle Log page".

// Older entries drop off the end once a battle runs past this; the page shows newest first.
#define BATTLE_LOG_CAPACITY 40

struct BattleLogEntry
{
    u16 move;
    u16 attackerSpecies; // the species the player SAW (the disguise under an active Illusion)
    u16 targetSpecies;   // likewise; only meaningful when hasTarget
    u16 turn;            // gBattleTurnCounter, 0-based
    u16 hpLost;          // summed over every hit this move landed on this target
    u16 targetMaxHP;     // at the first hit, so the % reads against the bar the player saw
    u8 attacker:2;
    u8 target:2;
    u8 attackerOnPlayerSide:1;
    u8 hasTarget:1;      // FALSE for a status move, a miss, or a hit absorbed by Disguise
    u8 ko:1;
    u8 substitute:1;     // the damage went into a Substitute, not the mon
};

// Lives in gBattleStruct (zero-allocated per battle), so a new battle starts with an empty log.
struct BattleLog
{
    struct BattleLogEntry entries[BATTLE_LOG_CAPACITY];
    u8 next;  // ring-buffer write index
    u8 count; // entries held, up to BATTLE_LOG_CAPACITY
};

// A move resolving (the point its "X used Y!" is printed).
void BattleLogRecordMove(enum BattlerId attacker, enum Move move);
// A move's damage landing. Folds into the move's own entry -- several hits of a multi-hit move
// sum, and a spread move gets one entry per target.
void BattleLogRecordDamage(enum BattlerId attacker, enum BattlerId target, enum Move move, u32 hpLost, bool32 substitute);

u32 BattleLogCount(void);
// index 0 is the newest entry.
const struct BattleLogEntry *BattleLogGetNewest(u32 index);
// The entry's damage as a whole % of the target's max HP (at least 1 for any damage at all).
u32 BattleLogEntryPercent(const struct BattleLogEntry *entry);

#endif // GUARD_FORK_BATTLE_LOG_H
