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

// An armed Mega Evolution / Ultra Burst, previewed as the form it becomes. The move menu shows
// the battler before the turn starts, but the form change happens before the move, so every
// readout about that move -- the damage range, its type, the effectiveness icon -- is asked of
// the projected form.
// The gimmick the player has armed for `battler` in the move menu, else GIMMICK_NONE.
enum Gimmick GetArmedGimmick(enum BattlerId battler);
// Rewrites gBattleMons[battler] into the form `gimmick` turns it into (its species, stats,
// ability and types), saving the real entry. FALSE, changing nothing, when the gimmick changes
// no form. There is one save slot: pair every TRUE with EndArmedFormPreview before the next.
bool32 BeginArmedFormPreview(enum BattlerId battler, enum Gimmick gimmick);
void EndArmedFormPreview(enum BattlerId battler);

// The type `move` is fired as, which is what the readout names in place of the stock label's
// base type: Weather Ball in sun reads Fire, Pixilate's Hyper Voice reads Fairy, and an armed
// gimmick or Mega form is in place first (an armed Tera's Tera Blast, a Mega-to-be's Pixilate).
// Exposed for test/fork/damage_preview.c.
enum Type GetDamagePreviewMoveType(enum BattlerId battler, enum Move move, enum Gimmick gimmick);

// The move that really runs when `battler` picks `move` with its Z-Move or Dynamax armed (or
// already active): the Z-Move or Max Move it turns into, else `move` itself. The effectiveness
// icon judges this move, since a converted move keeps the base move's type but none of its
// matchup quirks -- Freeze-Dry's Subzero Slammer / Max Hailstorm is resisted by Water.
enum Move GetDamagePreviewExecutedMove(enum BattlerId battler, enum Move move);

struct DamageContext;

// CalcTypeEffectivenessMultiplier against the disguise a defender under Illusion shows: its
// typing and species, with the real mon's ability and item left out. Move-specific matchups
// (Freeze-Dry, Flying Press, Thousand Arrows) still apply; the real typing never leaks.
// Leaves the defender and `ctx` as it found them. Used by the effectiveness icon.
uq4_12_t CalcDisguisedTypeEffectiveness(struct DamageContext *ctx, enum Species illusionSpecies);

// Whether the hit the preview prices is a crit, asked by AI_CalcDamage once `ctx` holds the type
// effectiveness of the move that really runs. A crit counts only when it is certain, as the real
// hit's IsCriticalHit decides it: innates included (an innate Super Luck's first-turn crit, an
// innate Battle Armor's block) and the DETERMINISTIC_CRITICAL_HITS high-crit-ratio gate, which
// the AI's own per-eval crit guess leaves out to stay inside its thinking-time budget.
bool32 IsDamagePreviewCrit(struct DamageContext *ctx);

#endif // GUARD_FORK_DAMAGE_PREVIEW_H
