#include "global.h"
#include "fork/halo.h"
#include "battle.h"
#include "battle_util.h"
#include "constants/abilities.h"

// FORK: the Halo ability (see include/fork/halo.h and fork-docs/NEW_ABILITIES.md). Halo is an
// aura, so IsHaloOnField() asks "is a Halo holder anywhere on the field", not "is the target
// the holder" -- everyone standing in the light is protected, including the holder's foes.
//
// Neither function belongs on a per-damage-roll path: IsHaloOnField() is called once per
// DamageContext to fill the cached ctx->haloOnField bit, and ApplyHaloDamageCap() only runs
// when that bit is set. See the performance note in the header before relocating either.

bool32 IsHaloOnField(void)
{
    for (u32 battler = 0; battler < gBattlersCount; battler++)
    {
        // Check the raw chosen-ability field first so the pricier suppression-aware lookup is
        // only paid by a battler that plainly has Halo.
        if (gBattleMons[battler].ability != ABILITY_HALO || !IsBattlerAlive(battler))
            continue;
        // Resolve suppression-aware so Gastro Acid / Neutralizing Gas put the aura out.
        if (GetBattlerAbility(battler) == ABILITY_HALO)
            return TRUE;
    }
    return FALSE;
}

s32 ApplyHaloDamageCap(u32 battlerDef, s32 dmg)
{
    s32 cap = gBattleMons[battlerDef].maxHP * HALO_DAMAGE_CAP_PERCENT / 100;

    if (cap < 1) // a 1-2 HP battler must still be damageable; the caller re-floors to 1 anyway
        cap = 1;

    return (dmg > cap) ? cap : dmg;
}
