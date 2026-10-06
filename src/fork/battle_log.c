// B_FRONTIER_BATTLE_INFO -- the Battle Log page's recorder. See include/fork/battle_log.h.
// Only ever called from real move execution (never from the AI's simulations, which do not run
// the move-resolution cancelers), so everything here is something the player watched happen.

#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "fork/battle_log.h"

static enum Species GetSeenSpecies(enum BattlerId battler)
{
    enum Species illusion = GetIllusionMonSpecies(battler);

    return (illusion != SPECIES_NONE) ? illusion : gBattleMons[battler].species;
}

static struct BattleLogEntry *AppendEntry(enum BattlerId attacker, enum Move move)
{
    struct BattleLog *log = &gBattleStruct->battleLog;
    struct BattleLogEntry *entry = &log->entries[log->next];

    memset(entry, 0, sizeof(*entry));
    entry->move = move;
    entry->attacker = attacker;
    entry->attackerOnPlayerSide = IsOnPlayerSide(attacker);
    entry->attackerSpecies = GetSeenSpecies(attacker);
    entry->turn = gBattleTurnCounter;

    if (++log->next >= BATTLE_LOG_CAPACITY)
        log->next = 0;
    if (log->count < BATTLE_LOG_CAPACITY)
        log->count++;
    return entry;
}

u32 BattleLogCount(void)
{
    return gBattleStruct->battleLog.count;
}

const struct BattleLogEntry *BattleLogGetNewest(u32 index)
{
    const struct BattleLog *log = &gBattleStruct->battleLog;

    if (index >= log->count)
        return NULL;
    return &log->entries[(log->next + BATTLE_LOG_CAPACITY - 1 - index) % BATTLE_LOG_CAPACITY];
}

u32 BattleLogEntryPercent(const struct BattleLogEntry *entry)
{
    u32 pct;

    if (entry->targetMaxHP == 0)
        return 0;
    pct = (entry->hpLost * 100 + entry->targetMaxHP / 2) / entry->targetMaxHP;
    if (pct == 0 && entry->hpLost != 0)
        pct = 1;
    return pct;
}

void BattleLogRecordMove(enum BattlerId attacker, enum Move move)
{
    AppendEntry(attacker, move);
}

void BattleLogRecordDamage(enum BattlerId attacker, enum BattlerId target, enum Move move, u32 hpLost, bool32 substitute)
{
    struct BattleLogEntry *entry = (struct BattleLogEntry *)BattleLogGetNewest(0);

    // A hit belongs to the move entry just recorded for it. Anything else -- the second target
    // of a spread move, a delayed hit like Future Sight, a Dancer copy (which prints no
    // "used" line of its own) -- opens a fresh entry for the same move.
    if (entry == NULL || entry->attacker != attacker || entry->move != move || entry->turn != gBattleTurnCounter
     || (entry->hasTarget && (entry->target != target || entry->substitute != substitute)))
        entry = AppendEntry(attacker, move);

    if (!entry->hasTarget)
    {
        entry->hasTarget = TRUE;
        entry->target = target;
        entry->targetSpecies = GetSeenSpecies(target);
        entry->targetMaxHP = gBattleMons[target].maxHP;
        entry->substitute = substitute;
    }
    if (entry->hpLost + hpLost > UINT16_MAX)
        entry->hpLost = UINT16_MAX;
    else
        entry->hpLost += hpLost;
    entry->ko = (gBattleMons[target].hp == 0);
}
