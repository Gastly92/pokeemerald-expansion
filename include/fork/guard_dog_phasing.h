#ifndef GUARD_FORK_GUARD_DOG_PHASING_H
#define GUARD_FORK_GUARD_DOG_PHASING_H

// FEATURE_ANNOUNCE_GUARD_DOG (config/feature.h): when Guard Dog (chosen ability or innate) stops
// Roar, Whirlwind, Dragon Tail or Circle Throw from forcing its holder out, show the Guard Dog
// pop-up and "<mon> refuses to be forced out!" instead of a bare "But it failed!" (Roar /
// Whirlwind) or nothing at all (Dragon Tail / Circle Throw). Mirrors how Suction Cups already
// announces itself in the same two places.

// TRUE when battler's Guard Dog blocks a forced switch and the feature is on. Points
// gBattlerAbility / gEffectBattler at the holder, and the pop-up at Guard Dog for an innate holder,
// ready for BattleScript_GuardDogPreventsPhasingOut[Ret].
bool32 TryPrepareGuardDogPhasingPopUp(enum BattlerId battler);

#endif // GUARD_FORK_GUARD_DOG_PHASING_H
