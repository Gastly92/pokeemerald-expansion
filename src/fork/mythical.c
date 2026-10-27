#include "global.h"
#include "fork/mythical.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "pokemon.h"

// The Mythical ability -- see include/fork/mythical.h and fork-docs/NEW_ABILITIES.md.

#define MYTHICAL_HP_MULTIPLIER 2

static bool32 IsPartySlotBoosted(enum BattleTrainer trainer, u32 partyIndex)
{
    return (gBattleStruct->mythicalHPBoosted[trainer] >> partyIndex) & 1;
}

bool32 IsMythicalHPBoosted(enum BattlerId battler)
{
    return IsPartySlotBoosted(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]);
}

bool32 TryActivateMythical(enum BattlerId battler, enum Ability ability)
{
    struct Pokemon *mon = GetBattlerMon(battler);
    u32 hp, maxHP;

    // The presence belongs to the species itself, so a Ditto that copies a Mythical never gets it.
    if (IsMythicalHPBoosted(battler) || gBattleMons[battler].volatiles.transformed || !IsBattlerAlive(battler))
        return FALSE;

    hp = GetMonData(mon, MON_DATA_HP) * MYTHICAL_HP_MULTIPLIER;
    maxHP = GetMonData(mon, MON_DATA_MAX_HP) * MYTHICAL_HP_MULTIPLIER;
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
    gBattleMons[battler].hp = hp;
    gBattleMons[battler].maxHP = maxHP;
    gBattleStruct->mythicalHPBoosted[GetBattlerTrainer(battler)] |= 1u << gBattlerPartyIndexes[battler];
    UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], mon, HEALTHBOX_ALL);

    // An innate Mythical shows itself in the pop-up, not the chosen ability (the Pressure precedent).
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gBattleScripting.battler = gBattlerAbility = battler;
    BattleScriptCall(BattleScript_MythicalActivates);
    return TRUE;
}

// Halves the slot's max HP back and scales its HP onto it, rounding up so a living mon stays alive. Every
// recalculation re-doubles a boosted max HP (ReapplyMythicalHPBoost), so halving it is exact.
static void UndoPartySlot(enum BattleTrainer trainer, u32 partyIndex)
{
    struct Pokemon *mon = &gParties[trainer][partyIndex];
    u32 hp = GetMonData(mon, MON_DATA_HP);
    u32 boostedMaxHP = GetMonData(mon, MON_DATA_MAX_HP);
    u32 maxHP = boostedMaxHP / MYTHICAL_HP_MULTIPLIER;

    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
    if (hp != 0 && boostedMaxHP != 0)
        hp = (hp * maxHP + boostedMaxHP - 1) / boostedMaxHP;
    if (hp > maxHP)
        hp = maxHP;
    SetMonData(mon, MON_DATA_HP, &hp);
    gBattleStruct->mythicalHPBoosted[trainer] &= ~(1u << partyIndex);
}

void UndoMythicalHPBoost(enum BattlerId battler)
{
    struct Pokemon *mon = GetBattlerMon(battler);

    if (!IsMythicalHPBoosted(battler))
        return;

    UndoPartySlot(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]);
    gBattleMons[battler].hp = GetMonData(mon, MON_DATA_HP);
    gBattleMons[battler].maxHP = GetMonData(mon, MON_DATA_MAX_HP);
}

void UndoAllMythicalHPBoosts(void)
{
    for (enum BattleTrainer trainer = 0; trainer < MAX_BATTLE_TRAINERS; trainer++)
    {
        for (u32 i = 0; i < PARTY_SIZE; i++)
        {
            if (IsPartySlotBoosted(trainer, i))
                UndoPartySlot(trainer, i);
        }
    }
}

void ReapplyMythicalHPBoost(enum BattlerId battler, struct Pokemon *mon, u32 hpBefore, u32 maxHPBefore)
{
    u32 hp = hpBefore;
    u32 maxHP;

    if (!IsMythicalHPBoosted(battler))
        return;

    // CalculateMonStats' own HP rule, against the doubled max: a fainted mon stays fainted, a rise in max HP is
    // added to the current HP, and the HP never exceeds the max.
    maxHP = GetMonData(mon, MON_DATA_MAX_HP) * MYTHICAL_HP_MULTIPLIER;
    if (hp != 0 || maxHPBefore == 0)
    {
        if (maxHP > maxHPBefore)
            hp += maxHP - maxHPBefore;
        if (hp > maxHP)
            hp = maxHP;
    }
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
    SetMonData(mon, MON_DATA_HP, &hp);
}

u32 RemoveMythicalHPBoost(enum BattlerId battler, u32 hp)
{
    if (!IsMythicalHPBoosted(battler))
        return hp;
    return (hp + MYTHICAL_HP_MULTIPLIER - 1) / MYTHICAL_HP_MULTIPLIER;
}
