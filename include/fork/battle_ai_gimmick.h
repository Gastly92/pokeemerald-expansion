#ifndef GUARD_FORK_BATTLE_AI_GIMMICK_H
#define GUARD_FORK_BATTLE_AI_GIMMICK_H

#include "battle.h"
#include "battle_gimmick.h"

// FORK: let AI trainers choose between the gimmicks a mon is eligible for.
//
// AssignUsableGimmicks seeds usableGimmick to the first candidate in enum order
// (MEGA, ULTRA_BURST, Z_MOVE, DYNAMAX, TERA) and nothing on the AI side ever revisits it -
// CycleGimmickSelection is driven by the player's Start press. That was harmless when a
// mon could only ever have one candidate, but with FEATURE_FREE_GIMMICKS almost every mon
// is Z-Move eligible, so Z-Move preempts Dynamax and Tera permanently and AI trainers
// never use them. DecideTerastal bails for the same reason.
//
// Selection is situational first, then falls back to this list: a candidate that turns
// the current turn into a KO is taken outright, and only when none does is the list
// consulted. The list itself is deliberately a plain, tunable ordering rather than a
// scoring function - ranking a one-shot burst against a multi-turn form change outside
// of a concrete KO is a balance decision, not something to infer per turn. Persistent
// gimmicks come first so the single-use Z-Move stays in reserve. Index 0 is picked first.
#define AI_GIMMICK_PREFERENCE_ORDER \
    GIMMICK_MEGA,                   \
    GIMMICK_ULTRA_BURST,            \
    GIMMICK_DYNAMAX,                \
    GIMMICK_TERA,                   \
    GIMMICK_Z_MOVE

// FORK: when the persistent gimmicks fire. Z-Move and Tera already wait for a good moment
// (ShouldUseZMove, DecideTerastal); Mega and Dynamax had no such check, so the lead fired
// whichever it was assigned on turn 1, every battle. Each turn a picked Mega or Dynamax now
// commits only on a roll, unless it is forced: the gimmick turns this turn into a KO the
// plain moves could not get (GimmickSecuresKO), it saves the mon from a KO this turn
// (GimmickSavesFromKO), or no teammate is left to use it later.
//
// Mega is bound to its species, so there is nobody to save it for - it only gets a short,
// fixed hesitation. Dynamax is shared by the whole team, so, like upstream's
// AI_CONSERVE_TERA_CHANCE_PER_MON, it is held more often the more teammates could still
// use it: with five in reserve it commits 25% of the time, with one 85%.
#define AI_FREE_MEGA_COMMIT_CHANCE          60 // % per turn to Mega Evolve when not forced
#define AI_FREE_DYNAMAX_HOLD_CHANCE_PER_MON 15 // % held per healthy teammate that could Dynamax instead

void AI_SelectGimmicksForTurn(void);

// FORK: the move the engine will really execute for `move`, given battlerAtk's *active*
// gimmick. Returns `move` unchanged when nothing converts. See the comment on the
// definition for why the AI needs it.
enum Move AI_GetGimmickExecutedMove(enum BattlerId battlerAtk, enum Move move);

#endif // GUARD_FORK_BATTLE_AI_GIMMICK_H
