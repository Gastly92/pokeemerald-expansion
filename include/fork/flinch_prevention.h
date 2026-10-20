#ifndef GUARD_FORK_FLINCH_PREVENTION_H
#define GUARD_FORK_FLINCH_PREVENTION_H

// FEATURE_ANNOUNCE_FLINCH_PREVENTION (config/feature.h): announce a flinch that Shield Dust,
// Inner Focus or Covert Cloak blocked, which stock play swallows silently. Only a flinch that
// would actually have landed is announced, so a target that already moved this turn, is
// already flinching, is Dynamaxed, or sits behind a Substitute stays quiet.

#include "battle_set_effect.h"

// Shield Dust / Covert Cloak: called from SetMoveEffect just before its blocked-effect branch
// drops the flinch. Redirects se->script through the announcement when it fires.
void TryAnnounceBlockedFlinch(struct BattleCalcValues *cv, struct SetEffect *se, bool32 affectsUser);

// Inner Focus: called from the flinch handler's Inner Focus branch once it has set
// gBattlescriptCurrInstr, and redirects it through the announcement when it fires.
void TryAnnounceInnerFocusFlinch(struct BattleCalcValues *cv, struct SetEffect *se);

#endif // GUARD_FORK_FLINCH_PREVENTION_H
