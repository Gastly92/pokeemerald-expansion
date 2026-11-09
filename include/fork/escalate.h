#ifndef GUARD_FORK_ESCALATE_H
#define GUARD_FORK_ESCALATE_H

#include "battle.h"

// "Escalate" -- a fork ability for the override table (see fork-docs/NEW_ABILITIES.md).
// It makes the battle heat up faster: at the end of each turn, every Escalate holder on the
// field adds ESCALATE_BONUS_PERCENT to the DETERMINISTIC_DAMAGE multiplier, on top of that
// flag's own DETERMINISTIC_DAMAGE_TURN_INCREMENT. Two holders stack.
//
// - Field-wide: the multiplier is shared, so it raises the damage of every battler, foes
//   included. That symmetry is why it needs no separate downside, and why it is never an
//   innate (the global-field bucket in fork-docs/INNATE_ABILITIES.md).
// - A running total (gBattleStruct->escalateDamageBonus), not turn count x rate: a holder that
//   enters late does not back-date the bonus, and what it added stays after it leaves.
// - Fires at end of turn only for a holder that is still standing and did not switch in this
//   turn (Speed Boost's rule), so a holder knocked out mid-turn adds nothing that turn.
// - Inert unless DETERMINISTIC_DAMAGE is on, since there is no multiplier to raise otherwise.
// - Every reader of the multiplier (damage calc, AI damage prediction, the Battle Info screen)
//   goes through DETERMINISTIC_DAMAGE_PERCENT, so all three see the bonus.

#define ESCALATE_BONUS_PERCENT 2

// End-turn hook (ABILITYEFFECT_ENDTURN). Raises the bonus and queues the pop-up and message.
bool32 TryActivateEscalate(enum BattlerId battler);

#endif // GUARD_FORK_ESCALATE_H
