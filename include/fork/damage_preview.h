#ifndef GUARD_FORK_DAMAGE_PREVIEW_H
#define GUARD_FORK_DAMAGE_PREVIEW_H

// FORK: B_MOVE_DAMAGE_PREVIEW (include/config/fork.h) -- the move menu's damage range.
// See fork-docs/DAMAGE_PREVIEW.md and src/fork/damage_preview.c.

// The % of the defender's max HP a move could deal, as the player can bound it from public
// information only. The defender's spread is unknown, so the range runs from the bulkiest
// legal spread (31 IV / 252 EV / boosting nature in HP and both defences) at the bottom to
// the frailest (0 / 0 / hindering) at the top, widened by the damage roll. Anything the
// player has not seen -- the foe's chosen ability, its held item, the real mon behind an
// Illusion -- is left out of the calc. Returns FALSE for a move that has no range to show
// (status moves, or nothing the calc can damage). Leaves no battle state changed.
bool32 GetDamagePreviewRange(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Gimmick gimmick, u32 *loPct, u32 *hiPct);

// The move-menu hook: prints "<Type> lo-hi%" on the type row for the battler's selected
// move against the foe it would hit. Returns FALSE (having printed nothing) when there is no
// range to show, so the caller falls back to the stock "TYPE/<Type>" line.
bool32 TryPrintMoveDamagePreview(enum BattlerId battler, enum Move move, enum Type type);

#endif // GUARD_FORK_DAMAGE_PREVIEW_H
