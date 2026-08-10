#include "global.h"
#include "fork/halo.h"
#include "battle.h"
#include "battle_util.h"
#include "constants/abilities.h"

// FORK: the Halo ability (see include/fork/halo.h and fork-docs/NEW_ABILITIES.md). Halo is an
// aura, so the cap keys off "is a Halo holder anywhere on the field", not "is the target the
// holder" -- everyone standing in the light is protected, including the holder's foes.
//
// This runs inside ApplyModifiersAfterDmgRoll(), which the AI re-runs many times per turn while
// simulating (its thinking time is bounded by test/battle/ai/ai_thinking_time.c), so the
// no-Halo case has to be cheap: the loop below reads the raw chosen-ability struct field and
// only pays for the suppression-aware GetBattlerAbility() lookup once a raw match is found.

static bool32 IsHaloOnField(void)
{
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        if (gBattleMons[battler].ability != ABILITY_HALO || !IsBattlerAlive(battler))
            continue;
        // Confirmed Halo holder (rare). Resolve suppression-aware so Gastro Acid / Neutralizing
        // Gas put the aura out.
        if (GetBattlerAbility(battler) == ABILITY_HALO)
            return TRUE;
    }
    return FALSE;
}

s32 ApplyHaloDamageCap(u32 battlerDef, s32 dmg)
{
    s32 cap;

    if (dmg <= 0 || !IsHaloOnField())
        return dmg;

    cap = gBattleMons[battlerDef].maxHP * HALO_DAMAGE_CAP_PERCENT / 100;
    if (cap < 1) // a 1-2 HP battler must still be damageable; the caller re-floors to 1 anyway
        cap = 1;

    return (dmg > cap) ? cap : dmg;
}
