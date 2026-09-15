#include "global.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "constants/battle_string_ids.h"
#include "config/buff.h"
#include "fork/innate_abilities.h" // FORK: BattlerHasAbility / IsAbilityOrInnateAndRecord
#include "fork/buff_leech_seed.h"

// FORK: BUFF_LEECH_SEED. Leech Seed's drain, and the "re-drain" rule that lets a seeded
// battler keep draining after the seeder switches. Moved out of src/battle_util.c and
// src/battle_end_turn.c -- both files upstream rewrites heavily, and upstream's 1.17.0
// rewrite of the drain scripts already broke this feature once by relocating the hook.
// See fork-docs/FORK.md.

// FORK: BUFF_LEECH_SEED. TRUE when a use of Leech Seed on `victim` is the immediate
// re-drain (this `seeder` already seeds it) rather than a fresh seed, AND that drain can
// actually land. Callers use it both to run the re-drain (Cmd_setseeded) and to let it
// pierce the victim's Substitute (Cmd_jumpifsubstituteblocks) - the seed is attached to
// the mon itself, so like the end-turn tick the drain ignores a Substitute put up after
// the seed. A fresh seed is unaffected and still fails against a Substitute, as in vanilla.
bool32 CanLeechSeedReDrain(enum BattlerId seeder, enum BattlerId victim)
{
    return GetConfig(BUFF_LEECH_SEED)
        && (gBattleMons[victim].volatiles.leechSeededBy & LEECH_SEED_BIT(seeder))
        && IsBattlerPresent(victim)
        && !BattlerHasAbility(victim, ABILITY_MAGIC_GUARD); // FORK: innate-aware Magic Guard (FEATURE_INNATE_ABILITIES)
}

// FORK: BUFF_LEECH_SEED. Computes a single Leech Seed drain of `victim` by
// `seeder`: stores the passive HP deltas (victim loses, seeder gains - or takes
// recoil under Liquid Ooze) and the drain message, then returns which drain
// branch applies. Shared by the end-turn tick (HandleEndTurnLeechSeed) and the
// immediate re-drain when re-seeding an already-seeded foe (Cmd_setseeded). The
// caller is responsible for the Magic Guard / battler-present gating and for
// selecting the matching battle script for the returned branch.
enum LeechSeedDrainKind SetUpLeechSeedDrain(enum BattlerId victim, enum BattlerId seeder)
{
    s32 drainAmount = GetNonDynamaxMaxHP(victim) / BUFF_LEECH_SEED_DENOMINATOR;
    s32 healAmount = GetDrainedBigRootHp(seeder, drainAmount);

    SetPassiveDamageAmount(victim, drainAmount);
    if (BattlerHasAbility(victim, ABILITY_LIQUID_OOZE)) // FORK: innate-aware Liquid Ooze (FEATURE_INNATE_ABILITIES)
    {
        // FORK: show the innate in the pop-up, not the chosen ability, only when they
        // differ (Speed Boost precedent — CreateAbilityPopUp reads the primary slot).
        if (GetBattlerAbility(victim) != ABILITY_LIQUID_OOZE)
            gBattleScripting.abilityPopupOverwrite = ABILITY_LIQUID_OOZE;
        SetPassiveDamageAmount(seeder, healAmount); // seeder takes recoil instead of healing
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_LEECH_SEED_OOZE;
        return LEECH_SEED_DRAIN_LIQUID_OOZE;
    }
    else if (gBattleMons[seeder].volatiles.healBlockTimer)
    {
        return LEECH_SEED_DRAIN_HEAL_BLOCK;
    }
    else
    {
        SetHealAmount(seeder, healAmount);
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_LEECH_SEED_DRAIN;
        return LEECH_SEED_DRAIN_RECOVERY;
    }
}

// FORK: BUFF_LEECH_SEED - queues the end-turn drain script matching the branch
// chosen by SetUpLeechSeedDrain().
void CallLeechSeedTurnDrainScript(enum LeechSeedDrainKind kind)
{
    switch (kind)
    {
    case LEECH_SEED_DRAIN_LIQUID_OOZE:
        BattleScriptCall(BattleScript_LeechSeedTurnDrainLiquidOoze);
        break;
    case LEECH_SEED_DRAIN_HEAL_BLOCK:
        BattleScriptCall(BattleScript_LeechSeedTurnDrainHealBlock);
        break;
    case LEECH_SEED_DRAIN_RECOVERY:
    default:
        BattleScriptCall(BattleScript_LeechSeedTurnDrainRecovery);
        break;
    }
}
