#ifndef GUARD_FORK_FREE_GIMMICK_STRUGGLE_H
#define GUARD_FORK_FREE_GIMMICK_STRUGGLE_H

// FEATURE_FREE_GIMMICKS: when every move is blocked (e.g. choice-locked into a move
// Cursed Body disabled), upstream sends FIGHT straight to Struggle, so the gimmick
// picker in the move menu is never reached. Z-Moves and Dynamax lift most of those
// blocks, so for a player-controlled battler that still has one of them available the
// move menu opens instead.

// Called after AreAllMovesUnusable() returned TRUE for a FIGHT action. TRUE (and the
// battler's noValidMoves cleared) when the move menu should open anyway.
bool32 ShouldOpenMoveMenuForGimmick(enum BattlerId battler);

// Called on the move chosen from that menu. TRUE when no Z-Move / Dynamax was armed
// and every move is still blocked: the battler Struggles, exactly as it would have
// without the menu. Sets up the Struggle selection script as a side effect.
bool32 ShouldStruggleFromGimmickMoveMenu(enum BattlerId battler);

#endif // GUARD_FORK_FREE_GIMMICK_STRUGGLE_H
