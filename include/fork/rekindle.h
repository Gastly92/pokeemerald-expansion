#ifndef GUARD_FORK_REKINDLE_H
#define GUARD_FORK_REKINDLE_H

// "Rekindle" -- the first custom ability designed to be an innate (a pure boon with no
// downside; see fork-docs/NEW_ABILITIES.md). Each battler the holder knocks out with a move
// restores REKINDLE_HEAL_FRACTION of the holder's max HP, so a double KO heals twice as much.
//
// It rides Moxie's trigger: the ABILITYEFFECT_MOVE_END_FOES_FAINTED case in src/battle_util.c
// calls TryActivateRekindle with NumFaintedBattlersByAttacker(), which counts every battler the
// move knocked out -- allies included, exactly like Moxie and Beast Boost. As an innate it is
// reached through the attacker-side on-hit driver (IsActiveOnHitAttackerInnate in
// src/fork/innate_abilities.c), which delegates to that same case.

// Each KO heals max HP / REKINDLE_HEAL_FRACTION.
#define REKINDLE_HEAL_FRACTION 4

// Heals `battler` for `numFainted` KOs and queues BattleScript_AbilityHpHeal. Returns TRUE
// when it fired. `ability` is the Rekindle being processed (chosen or innate).
bool32 TryActivateRekindle(enum BattlerId battler, enum Ability ability, u32 numFainted);

#endif // GUARD_FORK_REKINDLE_H
