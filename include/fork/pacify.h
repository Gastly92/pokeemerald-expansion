#ifndef GUARD_FORK_PACIFY_H
#define GUARD_FORK_PACIFY_H

// "Pacify" -- Intimidate's Sp. Atk twin, designed to be an innate (a pure boon with no downside;
// see fork-docs/NEW_ABILITIES.md). When the holder enters battle, every opposing battler's Sp. Atk
// drops by 1 stage. It touches foes only, so the holder's partner in doubles is never affected.
//
// It is an ordinary stat drop, not an Intimidate: Clear Body, Clear Amulet, Mirror Armor, Defiant,
// Competitive and Contrary react to it as to any other drop, but the reactions upstream keys on
// Intimidate by name (Rattled, Guard Dog, Adrenaline Orb, the Gen 8 Inner Focus / Own Tempo /
// Oblivious / Scrappy immunity) do not.
//
// The ABILITYEFFECT_ON_SWITCHIN case in src/battle_util.c calls TryActivatePacify. As an innate it is
// reached through the switch-in driver (SwitchInInnateAbilityEffect in src/fork/innate_abilities.c),
// which delegates to that same case, exactly like an innate Intimidate.

// Queues the Sp. Atk drop on every opposing battler and BattleScript_AbilityStatChange. Returns TRUE
// when it fired. `ability` is the Pacify being processed (chosen or innate).
bool32 TryActivatePacify(enum BattlerId battler, enum Ability ability);

#endif // GUARD_FORK_PACIFY_H
