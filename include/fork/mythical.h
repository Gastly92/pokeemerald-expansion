#ifndef GUARD_FORK_MYTHICAL_H
#define GUARD_FORK_MYTHICAL_H

// "Mythical" -- an innate-only ability carried by every TIER_MYTHICAL species (include/fork/species_tiers.h,
// enforced by test/fork/mythical.c). When the holder enters battle its presence is announced and its max HP
// doubles, the way Dynamax raises it: the HP fraction is kept, and the boost is undone when the holder
// leaves the field (switch, forced switch, faint, end of battle), then re-applied on its next entry. It
// stacks with Dynamax. The ability is cantBeSuppressed and not breakable, so Gastro Acid, Neutralizing Gas
// and Mold Breaker never touch it, and a transformed battler (Ditto, Imposter) never gets the boost.
//
// Like Dynamax, effects that upstream scales off GetNonDynamaxHP / GetNonDynamaxMaxHP (residual damage,
// Leftovers-style healing, Super Fang, Endeavor, ...) divide the boost back out.
//
// The boost is tracked per party slot in gBattleStruct->mythicalHPBoosted. Hook points, each a single
// FORK-tagged line in an upstream file:
//   - ABILITYEFFECT_ON_SWITCHIN case in src/battle_util.c         -> TryActivateMythical
//   - Cmd_getswitchedmondata (every switch funnels through it)    -> UndoMythicalHPBoost
//   - FaintClearSetData / the end-of-battle stat recalculation    -> UndoMythicalHPBoost / UndoAllMythicalHPBoosts
//   - RecalcBattlerStats and UndoDynamax, after CalculateMonStats -> ReapplyMythicalHPBoost
//   - GetNonDynamaxHP / GetNonDynamaxMaxHP                        -> RemoveMythicalHPBoost

// TRUE if `battler`'s mon currently has its HP doubled by Mythical.
bool32 IsMythicalHPBoosted(enum BattlerId battler);

// Doubles the battler's HP and max HP and queues the announcement. Returns TRUE when it fired. `ability` is the
// Mythical being processed (chosen or innate), for the pop-up.
bool32 TryActivateMythical(enum BattlerId battler, enum Ability ability);

// Reverts the battler's boost, keeping its HP fraction (rounded up, so a living mon never drops to 0).
// A no-op for a battler that isn't boosted.
void UndoMythicalHPBoost(enum BattlerId battler);

// Reverts every boost still standing, for the end of battle.
void UndoAllMythicalHPBoosts(void);

// Call right after CalculateMonStats has rebuilt a boosted mon's max HP from scratch (form change, Dynamax
// ending). `hpBefore` / `maxHPBefore` are the mon's HP and max HP before that call. Re-doubles the max HP
// and restores the HP the way CalculateMonStats would have, then syncs gBattleMons.
void ReapplyMythicalHPBoost(enum BattlerId battler, struct Pokemon *mon, u32 hpBefore, u32 maxHPBefore);

// Divides the boost back out of an HP value, for GetNonDynamaxHP / GetNonDynamaxMaxHP.
u32 RemoveMythicalHPBoost(enum BattlerId battler, u32 hp);

#endif // GUARD_FORK_MYTHICAL_H
