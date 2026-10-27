#ifndef GUARD_FORK_SILENT_ABILITY_POPUP_H
#define GUARD_FORK_SILENT_ABILITY_POPUP_H

// FEATURE_ANNOUNCE_SILENT_ABILITIES (config/feature.h): the first time an ability that only
// changes a hit's damage (Fluffy, Multiscale, Fur Coat, Thick Fat, ...) takes effect, show its
// pop-up before the hit animation. Stock play applies these with no pop-up at all, while the
// damage calc still records them as revealed for the INFO viewer and the damage preview.

// Called from CancelerClearFlags, the first canceler of every move: remembers which abilities
// were already revealed before this move's type matchup and damage calc ran.
void SnapshotAbilityRevealsBeforeMove(void);

// Called from the pre-animation activations, after the type matchup and damage calc. Queues the pop-up for the
// next ability the calc revealed silently and returns TRUE; FALSE once there is none left.
bool32 TryShowSilentAbilityPopUp(void);

#endif // GUARD_FORK_SILENT_ABILITY_POPUP_H
