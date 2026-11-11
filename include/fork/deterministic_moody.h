#ifndef GUARD_FORK_DETERMINISTIC_MOODY_H
#define GUARD_FORK_DETERMINISTIC_MOODY_H

// DETERMINISTIC_ABILITIES Moody: instead of a random +2/-1, every end of turn
// Moody *sets* all seven stat stages (accuracy and evasion included) to the next
// value in a fixed swing: -1, +1, -2, +2, -3, +3, -6, +6, then -6/+6 forever.
// The step is kept in the moodyPhase volatile, so it restarts on switch-in.

#define MOODY_PHASE_COUNT 8

extern const u8 BattleScript_DeterministicMoody[];
extern const u16 gMoodyOscillationStringIds[MOODY_PHASE_COUNT];

s32 GetDeterministicMoodyStage(u32 phase);
void ApplyDeterministicMoody(enum BattlerId battler);

#endif // GUARD_FORK_DETERMINISTIC_MOODY_H
