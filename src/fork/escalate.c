#include "global.h"
#include "fork/escalate.h"
#include "battle.h"
#include "battle_ai_record.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "config_changes.h"

// The Escalate ability -- see include/fork/escalate.h and fork-docs/NEW_ABILITIES.md.

bool32 TryActivateEscalate(enum BattlerId battler)
{
    if (!GetConfig(DETERMINISTIC_DAMAGE) || BattlerJustSwitchedIn(battler))
        return FALSE;

    gBattleStruct->escalateDamageBonus += ESCALATE_BONUS_PERCENT;
    RecordAbilityBattle(battler, ABILITY_ESCALATE);
    gBattleScripting.battler = gBattlerAbility = battler;
    BattleScriptCall(BattleScript_EscalateActivates);
    return TRUE;
}
