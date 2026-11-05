#include "global.h"
#include "fork/mythical.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "config/feature.h"
#include "fork/innate_abilities.h"
#include "pokemon.h"

// The Mythical and Legendary abilities -- see include/fork/mythical.h and fork-docs/NEW_ABILITIES.md.
// Both are the same HP boost with a different multiplier, written as a fraction: Mythical x2/1, Legendary x3/2.
// A boosted value is floor(x * num / den), and ceil(boosted * den / num) recovers x exactly for both fractions,
// so a boost can always be undone without remembering the value it started from.

#define MYTHICAL_HP_NUM  2
#define MYTHICAL_HP_DEN  1
#define LEGENDARY_HP_NUM 3
#define LEGENDARY_HP_DEN 2

// Battler bitmasks for the AI's switch-in simulations: which battlers currently hold a simulated candidate, which
// of those candidates would be boosted, and which of the boosted ones by Legendary's x1.5 rather than x2.
static EWRAM_DATA u8 sSimulatedBattlers = 0;
static EWRAM_DATA u8 sSimulatedBoosted = 0;
static EWRAM_DATA u8 sSimulatedLegendary = 0;

static bool32 IsPartySlotBoosted(enum BattleTrainer trainer, u32 partyIndex)
{
    return (gBattleStruct->mythicalHPBoosted[trainer] >> partyIndex) & 1;
}

static bool32 IsPartySlotLegendary(enum BattleTrainer trainer, u32 partyIndex)
{
    return (gBattleStruct->legendaryHPBoosted[trainer] >> partyIndex) & 1;
}

static u32 BoostHP(u32 hp, bool32 legendary)
{
    if (legendary)
        return hp * LEGENDARY_HP_NUM / LEGENDARY_HP_DEN;
    return hp * MYTHICAL_HP_NUM / MYTHICAL_HP_DEN;
}

// The exact inverse of BoostHP for a max HP, and a rounded-up division for any other HP value.
static u32 UnboostHP(u32 hp, bool32 legendary)
{
    if (legendary)
        return (hp * LEGENDARY_HP_DEN + LEGENDARY_HP_NUM - 1) / LEGENDARY_HP_NUM;
    return (hp * MYTHICAL_HP_DEN + MYTHICAL_HP_NUM - 1) / MYTHICAL_HP_NUM;
}

bool32 IsMythicalHPBoosted(enum BattlerId battler)
{
    return IsPartySlotBoosted(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]);
}

bool32 TryActivateMythical(enum BattlerId battler, enum Ability ability)
{
    struct Pokemon *mon = GetBattlerMon(battler);
    bool32 legendary = (ability == ABILITY_LEGENDARY);
    u32 hp, maxHP;

    // The presence belongs to the species itself, so a Ditto that copies a Mythical never gets it.
    if (IsMythicalHPBoosted(battler) || gBattleMons[battler].volatiles.transformed || !IsBattlerAlive(battler))
        return FALSE;

    hp = BoostHP(GetMonData(mon, MON_DATA_HP), legendary);
    maxHP = BoostHP(GetMonData(mon, MON_DATA_MAX_HP), legendary);
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
    gBattleMons[battler].hp = hp;
    gBattleMons[battler].maxHP = maxHP;
    gBattleStruct->mythicalHPBoosted[GetBattlerTrainer(battler)] |= 1u << gBattlerPartyIndexes[battler];
    if (legendary)
        gBattleStruct->legendaryHPBoosted[GetBattlerTrainer(battler)] |= 1u << gBattlerPartyIndexes[battler];
    UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], mon, HEALTHBOX_ALL);

    // An innate Mythical / Legendary shows itself in the pop-up, not the chosen ability (the Pressure precedent).
    if (GetBattlerAbility(battler) != ability)
        gBattleScripting.abilityPopupOverwrite = ability;
    gBattleScripting.battler = gBattlerAbility = battler;
    BattleScriptCall(legendary ? BattleScript_LegendaryActivates : BattleScript_MythicalActivates);
    return TRUE;
}

// Divides the boost out of the slot's HP and max HP, rounding up so a living mon stays alive. Every recalculation
// re-boosts the max HP (ReapplyMythicalHPBoost), so UnboostHP recovers it exactly.
static void UndoPartySlot(enum BattleTrainer trainer, u32 partyIndex)
{
    struct Pokemon *mon = &gParties[trainer][partyIndex];
    bool32 legendary = IsPartySlotLegendary(trainer, partyIndex);
    u32 hp = UnboostHP(GetMonData(mon, MON_DATA_HP), legendary);
    u32 maxHP = UnboostHP(GetMonData(mon, MON_DATA_MAX_HP), legendary);

    // The same conversion as the max HP, so an HP the boost produced and nothing changed comes back exactly (a
    // proportional rescale against Legendary's uneven x1.5 max would drift by a point on odd values).
    SetMonData(mon, MON_DATA_MAX_HP, &maxHP);
    if (hp > maxHP)
        hp = maxHP;
    SetMonData(mon, MON_DATA_HP, &hp);
    gBattleStruct->mythicalHPBoosted[trainer] &= ~(1u << partyIndex);
    gBattleStruct->legendaryHPBoosted[trainer] &= ~(1u << partyIndex);
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

    // CalculateMonStats' own HP rule, against the boosted max: a fainted mon stays fainted, a rise in max HP is
    // added to the current HP, and the HP never exceeds the max.
    maxHP = BoostHP(GetMonData(mon, MON_DATA_MAX_HP), IsPartySlotLegendary(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]));
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
    bool32 boosted, legendary;

    if ((sSimulatedBattlers >> battler) & 1)
    {
        boosted = (sSimulatedBoosted >> battler) & 1;
        legendary = (sSimulatedLegendary >> battler) & 1;
    }
    else
    {
        boosted = IsMythicalHPBoosted(battler);
        legendary = IsPartySlotLegendary(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]);
    }
    if (!boosted)
        return hp;
    return UnboostHP(hp, legendary);
}

void SimulateMythicalSwitchIn(enum BattlerId battler)
{
    struct BattlePokemon *battleMon = &gBattleMons[battler];
    bool32 innates = GetConfig(FEATURE_INNATE_ABILITIES);
    bool32 mythical = battleMon->ability == ABILITY_MYTHICAL
                   || (innates && SpeciesHasInnate(battleMon->species, ABILITY_MYTHICAL));
    bool32 legendary = battleMon->ability == ABILITY_LEGENDARY
                    || (innates && SpeciesHasInnate(battleMon->species, ABILITY_LEGENDARY));

    sSimulatedBattlers |= 1u << battler;
    sSimulatedBoosted &= ~(1u << battler);
    sSimulatedLegendary &= ~(1u << battler);
    if (battleMon->hp == 0 || (!mythical && !legendary))
        return;

    // No species carries both; should a chosen Mythical meet an innate Legendary, the chosen ability fires first.
    if (battleMon->ability == ABILITY_MYTHICAL)
        legendary = FALSE;
    battleMon->hp = BoostHP(battleMon->hp, legendary);
    battleMon->maxHP = BoostHP(battleMon->maxHP, legendary);
    sSimulatedBoosted |= 1u << battler;
    if (legendary)
        sSimulatedLegendary |= 1u << battler;
}

void ClearSimulatedMythicalSwitchIns(void)
{
    sSimulatedBattlers = 0;
    sSimulatedBoosted = 0;
    sSimulatedLegendary = 0;
}
