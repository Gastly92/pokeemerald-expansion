#ifndef GUARD_FORK_BUFF_MICLE_BERRY_H
#define GUARD_FORK_BUFF_MICLE_BERRY_H

// BUFF_MICLE_BERRY (config/buff.h): Micle Berry revives the holder's most recently
// fainted ally instead of boosting accuracy. See src/fork/buff_micle_berry.c.

// Notes the order a mon fainted in. Called from SetValuesOnFaint() for every faint, flag on
// or off (it only writes gBattleStruct, which is zeroed per battle).
void MicleBerry_RecordFaint(enum BattlerId battler);

// The party index of the holder's most recently fainted ally, or PARTY_SIZE if none has
// fainted. The holder's trigger waits on this being something other than PARTY_SIZE.
u32 MicleBerry_GetReviveTarget(enum BattlerId holder);

// HP the revived ally comes back with: BUFF_MICLE_BERRY_REVIVE_PERCENT of its max HP,
// doubled by the holder's Ripen (ability or innate), capped at full and never below 1.
u32 MicleBerry_GetReviveHp(enum BattlerId holder, u32 partyIndex);

#endif // GUARD_FORK_BUFF_MICLE_BERRY_H
