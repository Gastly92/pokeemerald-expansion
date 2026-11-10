#ifndef GUARD_FORK_FOREWARN_H
#define GUARD_FORK_FOREWARN_H

// FEATURE_FOREWARN_MATCHUP (include/config/feature.h) and the INFO viewer's Forewarn reveal.
// See src/fork/forewarn.c and fork-docs/FORK.md.

// Forewarn's pick under FEATURE_FOREWARN_MATCHUP: the opposing move with the highest top-end
// damage against the Forewarn user, priced only from what that user's side could know. Falls back
// to the first move of the first foe when nothing can damage it. Sets gEffectBattler and
// gBattleTextBuff1 for STRINGID_FOREWARNACTIVATES, and records the reveal.
void ForewarnChooseMoveByMatchup(enum BattlerId battler);

// The top of the damage `move` could deal from `battlerAtk` to `battlerDef`, with the attacker's
// unknown spread at its strongest and its unrevealed ability / held item treated as absent. 0 for
// a status move or a move that cannot damage the target. Leaves no battle state changed.
// Exposed for test/fork/forewarn.c.
u32 GetForewarnThreat(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// Marks `move` as revealed by Forewarn on `battler`'s party slot, for the INFO viewer's move list.
// Kept apart from infoUsedMoves, which the Zoom Lens reads as "this foe has acted".
void RecordForewarnedMove(enum BattlerId battler, enum Move move);

#endif // GUARD_FORK_FOREWARN_H
