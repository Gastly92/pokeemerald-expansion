#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_util.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "fork/buff_micle_berry.h"
#include "fork/innate_abilities.h"

// BUFF_MICLE_BERRY (config/buff.h). The Berry's trigger lives in TrySetMicleBerry()
// (src/battle_hold_effects.c) and the revive itself in BS_MicleBerryRevive()
// (src/battle_script_commands.c, which needs that file's NATIVE_ARGS). This file holds the
// parts both of them share: the faint order and the revive amount.

void MicleBerry_RecordFaint(enum BattlerId battler)
{
    // A u8 sequence: 255 faints in one battle is not reachable, but stop counting rather
    // than wrap so the order can never invert.
    if (gBattleStruct->micleFaintSequence < 255)
        gBattleStruct->micleFaintSequence++;
    gBattleStruct->micleFaintOrder[GetBattlerTrainer(battler)][gBattlerPartyIndexes[battler]] = gBattleStruct->micleFaintSequence;
}

u32 MicleBerry_GetReviveTarget(enum BattlerId holder)
{
    struct Pokemon *party = GetBattlerParty(holder);
    enum BattleTrainer trainer = GetBattlerTrainer(holder);
    u32 end, target = PARTY_SIZE;
    s32 bestOrder = -1;

    // The same party range Revival Blessing searches (GetFirstFaintedPartyIndex): only the
    // holder's own trainer's half when the side has two separate trainers.
    if (BattleSideHasTwoTrainers(holder & BIT_SIDE) && !AreMultiPartiesFullTeams())
        end = PARTY_SIZE / 2;
    else
        end = PARTY_SIZE;

    for (u32 i = 0; i < end; i++)
    {
        enum Species species = GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG);
        if (species == SPECIES_NONE || species == SPECIES_EGG || GetMonData(&party[i], MON_DATA_HP) != 0)
            continue;
        // Most recent faint wins. A mon that was already fainted when the battle began has
        // order 0 and is only picked when nothing fainted during the battle.
        s32 order = gBattleStruct->micleFaintOrder[trainer][i];
        if (order > bestOrder)
        {
            bestOrder = order;
            target = i;
        }
    }

    return target;
}

u32 MicleBerry_GetReviveHp(enum BattlerId holder, u32 partyIndex)
{
    struct Pokemon *mon = &GetBattlerParty(holder)[partyIndex];
    u32 maxHP = GetMonData(mon, MON_DATA_MAX_HP);
    u32 percent = BUFF_MICLE_BERRY_REVIVE_PERCENT;

    if (GetBattlerAbility(holder) == ABILITY_RIPEN || IsInnateActive(holder, ABILITY_RIPEN))
        percent *= 2;
    if (percent > 100)
        percent = 100;

    u32 hp = maxHP * percent / 100;
    return hp == 0 ? 1 : hp;
}
