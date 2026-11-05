#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "fork/guard_dog_phasing.h"
#include "fork/innate_abilities.h"

// Announcing Guard Dog's forced-switch block -- see include/fork/guard_dog_phasing.h.
//
// Hooked from BS_JumpIfRoarFails (Roar / Whirlwind, before BattleScript_EffectRoar's own
// jumpifability Guard Dog line) and from the EFFECT_HIT_SWITCH_TARGET moveend case (Dragon Tail /
// Circle Throw). With the feature off both fall through to the stock silent paths.

bool32 TryPrepareGuardDogPhasingPopUp(enum BattlerId battler)
{
    enum Ability chosen;

    if (!GetConfig(FEATURE_ANNOUNCE_GUARD_DOG))
        return FALSE;

    chosen = GetBattlerAbility(battler);
    if (chosen != ABILITY_GUARD_DOG && !IsInnateActive(battler, ABILITY_GUARD_DOG))
        return FALSE;

    gBattlerAbility = battler;
    gEffectBattler = battler; // the message names B_EFF_NAME
    // CreateAbilityPopUp reads the chosen slot, so name the innate when that is what acted.
    if (chosen != ABILITY_GUARD_DOG)
        gBattleScripting.abilityPopupOverwrite = ABILITY_GUARD_DOG;
    return TRUE;
}
