#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_gimmick.h"
#include "battle_util.h"
#include "fork/battle_ai_zmove.h"

// FORK: decide whether spending the one-per-battle Z-Move on this move is worth it.
//
// A Z-Move is a single-use resource, so the only reasons to spend it are that it changes
// the outcome of the turn, or that there is no later turn to save it for. Upstream's
// ShouldUseZMove only declines when the *plain* move already secures the KO, which means
// the AI reliably burns the Z-Move on turn one - frequently into a resisted move, and
// frequently for a few points of damage it did not need.
//
// Gated behind AI_FLAG_SMART_Z_MOVE so it is opt-in per trainer, exactly like
// AI_FLAG_SMART_TERA. Without the flag the AI keeps upstream's eager behaviour.
bool32 AI_ShouldSpendZMove(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move chosenMove)
{
    uq4_12_t effectiveness;
    struct SimulatedDamage plainDmg = AI_CalcDamageSaveBattlers(chosenMove, battlerAtk, battlerDef, &effectiveness, NO_GIMMICK, NO_GIMMICK);
    struct SimulatedDamage zDmg = AI_CalcDamageSaveBattlers(chosenMove, battlerAtk, battlerDef, &effectiveness, USE_GIMMICK, NO_GIMMICK);

    u32 plainHits = GetNoOfHitsToKOBattlerDmg(plainDmg.minimum, battlerDef);
    u32 zHits = GetNoOfHitsToKOBattlerDmg(zDmg.minimum, battlerDef);

    // The Z-Move secures a KO this turn that the plain move does not. Merely needing
    // fewer hits is not enough: a stronger move almost always lowers the raw hit count
    // (a 24-hit chip becomes a 10-hit chip), which would spend the Z-Move on turn one
    // against anything bulky. Only a KO actually changes the outcome of the turn.
    // GetNoOfHitsToKO returns 0 for "deals no damage", so plainHits == 0 never matches 1.
    if (zHits == 1 && plainHits != 1)
        return TRUE;

    // About to be KO'd, so there is no later turn to save it for.
    if (GetBestNoOfHitsToKO(battlerDef, battlerAtk, AI_DEFENDING) == 1)
        return TRUE;

    // Otherwise the Z-Move only adds damage that changes nothing this turn. Keep it.
    return FALSE;
}
