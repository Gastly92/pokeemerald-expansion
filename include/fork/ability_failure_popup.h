#ifndef GUARD_FORK_ABILITY_FAILURE_POPUP_H
#define GUARD_FORK_ABILITY_FAILURE_POPUP_H

// FEATURE_ANNOUNCE_ABILITY_FAILURES (config/feature.h): when a move fails because of an ability,
// show that ability's pop-up before the usual "But it failed!", so the player can see why. No new
// text: the stock fail script still runs, BattleScript_AbilityPopUp just plays first.

// Points the pop-up at battler's ability (naming an innate rather than the chosen slot). FALSE,
// touching nothing, when the feature is off.
bool32 TryPrepareAbilityFailurePopUp(enum BattlerId battler, enum Ability ability);

// Roar / Whirlwind (BS_JumpIfRoarFails): when Guard Dog, chosen or innate, blocks the switch, runs
// the pop-up and then failScript, and returns TRUE.
bool32 TryFailWithGuardDogPopUp(enum BattlerId battler, const u8 *failScript);

// Psycho Shift (BS_TryPsychoShift), once gBattlescriptCurrInstr is the fail script: puts the
// pop-up first when an ability is what stopped the status passing over.
void TryAnnouncePsychoShiftBlocker(void);

// Rest (BS_JumpIfAbilityPreventsRest), once gBattlescriptCurrInstr is the fail script: puts the
// pop-up of the Leaf Guard / Shields Down / Sweet Veil that blocked it first.
void TryAnnounceRestBlocker(enum BattlerId battler);

#endif // GUARD_FORK_ABILITY_FAILURE_POPUP_H
