#ifndef GUARD_FORK_DAMAGE_PREVIEW_H
#define GUARD_FORK_DAMAGE_PREVIEW_H

// B_MOVE_DAMAGE_PREVIEW (include/config/fork.h) -- the move menu's damage range.
// See fork-docs/DAMAGE_PREVIEW.md and src/fork/damage_preview.c.

// The % of the defender's max HP a move could deal, as the player can bound it from public
// information only. The defender's spread is unknown, so the range runs from the bulkiest
// legal spread (31 IV / 252 EV / boosting nature in HP and both defences) at the bottom to
// the frailest (0 / 0 / hindering) at the top, widened by the damage roll. Anything the
// player has not seen -- the foe's chosen ability, its held item, the real mon behind an
// Illusion -- is left out of the calc. A damaging move against a foe the player knows is immune
// (type, or a revealed ability such as Bulletproof) is a 0-0 range. Returns FALSE for a move that
// has no range to show (status moves, or a move that fails for any other reason). Leaves no
// battle state changed.
// `ko` says whether the hit KOs from the defender's *current* HP: at every corner of the range,
// at some, or not at all (also when a known Sturdy / Focus Sash / Disguise would hold on).
enum DamagePreviewKO
{
    DAMAGE_PREVIEW_NO_KO,
    DAMAGE_PREVIEW_KO_MAYBE,
    DAMAGE_PREVIEW_KO_ALWAYS,
};

bool32 GetDamagePreviewRange(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Gimmick gimmick, u32 *loPct, u32 *hiPct, enum DamagePreviewKO *ko);

// The readout after the type name: "KO" (KOs at every corner), "lo%-KO" (at some), else
// "lo-hi%" capped at 100. Returns the new end of `dst`. Exposed for test/fork/damage_preview.c.
u8 *FormatDamagePreviewAmount(u8 *dst, u32 lo, u32 hi, enum DamagePreviewKO ko);

// The move-menu hook: prints "<Type> lo-hi%" on the type row for the battler's selected
// move against the foe it would hit. Returns FALSE (having printed nothing) when there is no
// range to show, so the caller falls back to the stock "TYPE/<Type>" line.
bool32 TryPrintMoveDamagePreview(enum BattlerId battler, enum Move move, enum Type type);

// The same for the Z-Move view, which prints its own type row: the range of `zMove` as fired
// from the move under the cursor. FALSE for a status Z-Move, so the caller keeps its stock line.
bool32 TryPrintZMoveDamagePreview(enum BattlerId battler, enum Move zMove, enum Type zMoveType);

#endif // GUARD_FORK_DAMAGE_PREVIEW_H
