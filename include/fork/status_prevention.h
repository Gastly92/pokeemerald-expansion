#ifndef GUARD_FORK_STATUS_PREVENTION_H
#define GUARD_FORK_STATUS_PREVENTION_H

// FEATURE_ANNOUNCE_STATUS_PREVENTION (config/feature.h): announce when an ability or item stops
// a damaging move's status side effect (Body Slam's paralysis, Scald's burn, ...), which stock
// play swallows silently. Only a status that would actually have landed is announced: the
// chance roll already succeeded, and nothing else (a type immunity, an existing status,
// Safeguard, Misty Terrain, a Substitute, Sheer Force) would have stopped it anyway.

#include "battle_set_effect.h"

// The per-status messages BattleScript_StatusPreventedBy* prints, chosen by MULTISTRING_CHOOSER.
extern const u16 gStatusPreventedStringIds[];

// Called from IsNonVolatileStatusBlocked (src/battle_util.c) every time it blocks a status, so
// the announcement can name the ability that blocked it without redoing the check. `ability`
// is ABILITY_NONE when nothing ability-related blocked it.
void NoteNonVolatileStatusBlocker(enum BattlerId battler, enum Ability ability);

// The blocker the last NoteNonVolatileStatusBlocker call recorded (ABILITY_NONE if no ability).
enum Ability GetNonVolatileStatusBlocker(enum BattlerId *battler);

// Shield Dust / Covert Cloak: called from SetMoveEffect just before its blocked-effect branch
// drops the effect. Redirects se->script through the announcement when it fires.
void TryAnnounceBlockedStatusEffect(struct BattleCalcValues *cv, struct SetEffect *se, bool32 affectsUser);

// Status immunity abilities (Limber, Immunity, Water Veil, Insomnia, ...): called from the
// non-volatile status handler after CanSetNonVolatileStatus refused, once it has set
// gBattlescriptCurrInstr. Redirects it through the announcement when an ability was the reason.
void TryAnnounceStatusImmunity(struct BattleCalcValues *cv, struct SetEffect *se);

#endif // GUARD_FORK_STATUS_PREVENTION_H
