#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_gimmick.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "config_changes.h"
#include "fork/dynamax_sleep.h"

// Dynamax resists sleep under DETERMINISTIC_STATUS -- see include/fork/dynamax_sleep.h.

bool32 IsDynamaxSleepImmune(enum BattlerId battler)
{
    return GetConfig(DETERMINISTIC_STATUS) && GetActiveGimmick(battler) == GIMMICK_DYNAMAX;
}

void TryDynamaxWakeUp(enum BattlerId battler)
{
    if (!GetConfig(DETERMINISTIC_STATUS))
        return;

    gBattleMons[battler].volatiles.yawn = 0;
    if (!(gBattleMons[battler].status1 & STATUS1_SLEEP))
        return;

    // The same wake-up as Uproar's in CancelerAsleepOrFrozen; ActivateDynamax runs with
    // gBattlerAttacker == battler, which BattleScript_MoveUsedWokeUp prints and updates.
    TryDeactivateSleepClause(battler, gBattlerPartyIndexes[battler]);
    gBattleMons[battler].status1 &= ~STATUS1_SLEEP;
    gBattleMons[battler].volatiles.nightmare = FALSE;
    BtlController_EmitSetMonData(battler, B_COMM_TO_CONTROLLER, REQUEST_STATUS_BATTLE, 0, 4, &gBattleMons[battler].status1);
    MarkBattlerForControllerExec(battler);
    gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_WOKE_UP;
    BattleScriptCall(BattleScript_MoveUsedWokeUp);
}
