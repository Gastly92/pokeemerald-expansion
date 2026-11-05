#ifndef GUARD_FORK_STATUS_PREVIEW_H
#define GUARD_FORK_STATUS_PREVIEW_H

// B_MOVE_STATUS_PREVIEW (include/config/fork.h) -- the move menu's "<Type> Fails" readout for a
// status move the player can already tell will not work on the foe.
// See fork-docs/DAMAGE_PREVIEW.md ("Status moves") and src/fork/status_preview.c.

// TRUE when `move`, used by `battlerAtk` on `battlerDef`, is certain to fail on what the player
// can see: the foe's types and innates, its *revealed* ability and held item, its status and
// volatiles (Substitute, Leech Seed, confusion, Yawn), the field (Safeguard, terrain, weather)
// and the attacker's own ability. An unrevealed ability or item is assumed absent, so a hidden
// Safety Goggles or Insomnia never shows. FALSE when the move might work, and for any move this
// readout does not judge (damaging moves, moves that target the user, its side or the field).
// Leaves no battle state changed. Exposed for test/fork/status_preview.c.
bool32 IsStatusMoveKnownToFail(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// The move-menu hook, called from TryPrintMoveDamagePreview once the damage readout has declined:
// prints "<Type> Fails" on the type row and returns TRUE, or prints nothing and returns FALSE so
// the caller falls back to the stock "TYPE/<Type>" line.
bool32 TryPrintStatusMovePreview(enum BattlerId battler, enum Move move, enum Type type);

#endif // GUARD_FORK_STATUS_PREVIEW_H
