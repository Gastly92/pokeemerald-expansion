#ifndef GUARD_FORK_DYNAMAX_SLEEP_H
#define GUARD_FORK_DYNAMAX_SLEEP_H

// DETERMINISTIC_STATUS (config/deterministic.h): a fixed 3-turn sleep would eat two of a
// Dynamax's three turns, so under the flag a Dynamaxed battler cannot be put to sleep, and
// Dynamaxing wakes a sleeping (or drowsy) battler.

// Whether a Dynamaxed battler is shielded from new sleep. Checked from CanSetNonVolatileStatus
// (sleep moves, Yawn, secondary sleep, and every AI CanBeSlept query) and from the end-turn
// Yawn handler.
bool32 IsDynamaxSleepImmune(enum BattlerId battler);

// Called from ActivateDynamax once BattleScript_DynamaxBegins has been pushed: wakes a sleeping
// battler, clears Yawn's drowsiness, and calls "X woke up!" ahead of the Dynamax script.
void TryDynamaxWakeUp(enum BattlerId battler);

#endif // GUARD_FORK_DYNAMAX_SLEEP_H
