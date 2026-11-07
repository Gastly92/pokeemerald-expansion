#ifndef GUARD_FORK_PSYBUBBLE_H
#define GUARD_FORK_PSYBUBBLE_H

#include "battle.h"

// "Psybubble" -- designed to be an innate (a pure boon with no downside; see
// fork-docs/NEW_ABILITIES.md). On the holder's entry turn it survives one lethal hit at 1 HP,
// from ANY HP. It is Sturdy's endure with the deterministic Focus Band's entry-turn window in place
// of Sturdy's full-HP requirement.
//
// - Window: IsBattlersEntryTurn() (src/battle_util.c), the same "lead / switch-in turn, and not
//   the turn after" window DETERMINISTIC_HOLD_EFFECTS gives Focus Band. That window is tracked
//   unconditionally, so this ability does not depend on the config.
// - Once per entry: enduring closes the window (facedFoeAction), so a multi-hit move's next strike,
//   or a second attacker in doubles, still KOs. It also closes a held Focus Band's window: one save
//   per entry, never two.
// - Breakable (Mold Breaker pierces it, like Sturdy); suppressed by Gastro Acid / Neutralizing Gas.
// - Message: rides Sturdy's MOVE_RESULT_STURDIED flow, so the pop-up shows Psybubble and
//   "<mon> endured the hit!" prints.

// TRUE when `battler` has the ability live (chosen slot `chosenAbility`, already Mold-Breaker
// resolved by the caller, or an active innate) and its entry window is still open.
bool32 HasPsybubbleProtection(enum BattlerId battler, enum Ability chosenAbility);

// Applies the endure from GetAdjustedDamage(). `updateFlags` is FALSE for the AI's simulations,
// which must not close the window.
void ActivatePsybubble(enum BattlerId battler, enum Ability chosenAbility, bool32 updateFlags);

// AI: would Psybubble save battlerDef from this single hit of `move` by battlerAtk?
bool32 AI_PsybubbleEndures(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// AI switch-in simulation: does this (not yet on-field) candidate carry the ability?
bool32 SwitchinHasPsybubble(enum BattlerId battler, enum Ability chosenAbility);

#endif // GUARD_FORK_PSYBUBBLE_H
