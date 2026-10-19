#ifndef GUARD_FORK_WILLPOWER_H
#define GUARD_FORK_WILLPOWER_H

// "Willpower" -- Stamina's Sp. Def twin, designed to be an innate (a pure boon with no
// downside; see fork-docs/NEW_ABILITIES.md). Whenever the holder is hit by a move, its Sp. Def
// rises by 1 stage. Stamina toughens the body, Willpower the mind.
//
// It rides Stamina's trigger: the ABILITYEFFECT_MOVE_END case in src/battle_util.c calls
// TryActivateWillpower right beside ABILITY_STAMINA, with the same conditions. As an innate it is
// reached through the on-hit driver (IsActiveOnHitInnate in src/fork/innate_abilities.c), which
// delegates to that same case.

// Raises `battler`'s Sp. Def by 1 and queues BattleScript_AbilityStatChange. Returns TRUE when it
// fired. `ability` is the Willpower being processed (chosen or innate).
bool32 TryActivateWillpower(enum BattlerId battler, enum Ability ability);

#endif // GUARD_FORK_WILLPOWER_H
