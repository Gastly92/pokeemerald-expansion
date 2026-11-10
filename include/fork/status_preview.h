#ifndef GUARD_FORK_STATUS_PREVIEW_H
#define GUARD_FORK_STATUS_PREVIEW_H

// B_MOVE_STATUS_PREVIEW (include/config/fork.h) -- the move menu's "<Type> Fails" / "<Type> Bounces"
// readout for a status move the player can already tell will not work on the foe.
// See fork-docs/DAMAGE_PREVIEW.md ("Status moves") and src/fork/status_preview.c.

// TRUE when `move`, used by `battlerAtk` on `battlerDef`, is certain to fail on what the player
// can see: the foe's types and innates, its *revealed* ability and held item, its status and
// volatiles (Substitute, Leech Seed, confusion, Yawn), the field (Safeguard, terrain, weather)
// and the attacker's own ability. An unrevealed ability or item is assumed absent, so a hidden
// Safety Goggles or Insomnia never shows. FALSE when the move might work, and for any move this
// readout does not judge (damaging moves, moves that target the user, its side or the whole
// field). A move a visible Magic Bounce reflects counts as failing -- hazards on the foes' side
// included, where either foe's Magic Bounce sends it back.
// Leaves no battle state changed. Exposed for test/fork/status_preview.c.
bool32 IsStatusMoveKnownToFail(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

enum StatusPreviewResult
{
    STATUS_PREVIEW_NONE,    // might work: the stock "TYPE/<Type>" line
    STATUS_PREVIEW_FAILS,   // "<Type> Fails"
    STATUS_PREVIEW_BOUNCES, // "<Type> Bounces": a visible Magic Bounce reflects it onto the user
};

// What IsStatusMoveKnownToFail decides, split by why. Leaves no battle state changed.
enum StatusPreviewResult GetStatusMovePreviewResult(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);

// The move-menu hook, called from TryPrintMoveDamagePreview once the damage readout has declined:
// prints "<Type> Fails" or "<Type> Bounces" on the type row and returns TRUE, or prints nothing and returns FALSE so
// the caller falls back to the stock "TYPE/<Type>" line.
bool32 TryPrintStatusMovePreview(enum BattlerId battler, enum Move move, enum Type type);

#endif // GUARD_FORK_STATUS_PREVIEW_H
