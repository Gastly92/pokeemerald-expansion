// FEATURE_FOREWARN_MATCHUP -- Forewarn reveals the foe's move that threatens the Forewarn user
// most, rather than the one with the highest base power. Also the INFO viewer's record of the
// moves Forewarn has revealed, which applies with or without the feature.
//
// The ranking is built only from what the Forewarn user's side could work out for itself, so the
// pick reveals one move name and nothing else: the attacker's unknown spread is bounded at its
// strongest (31 IV / 252 EV / boosting nature, as the damage preview's top end does for a
// defender), and its unrevealed ability and held item are treated as absent. The Forewarn user is
// priced as it really is, since its own side knows it. The calc is the AI's damage simulation, as
// the damage preview uses, so every byte it rewrites is put back.

#include "global.h"
#include "battle.h"
#include "battle_ai_record.h"
#include "battle_ai_util.h"
#include "battle_dynamax.h"
#include "battle_main.h"
#include "battle_message.h"
#include "battle_util.h"
#include "pokemon.h"
#include "fork/damage_preview.h"
#include "fork/forewarn.h"

// Counter / Mirror Coat / Metal Burst return what the user takes, which nothing can know at
// switch-in, and the AI calc prices them at their placeholder power of 1. Scored as half the
// Forewarn user's max HP: a real threat, but below a strong attack.
#define FOREWARN_REFLECT_DAMAGE_PERCENT 50

// The attacker as its foe can bound it: the strongest legal offences (Defense too, for Body
// Press), Speed at whichever end the move prefers, and the health bar's fraction of the bounded
// max HP (Eruption, Water Spout, Final Gambit). A transformed attacker copied stats its foe can
// see, so it keeps them.
static void ApplyAttackerBound(struct BattlePokemon *mon, enum Species species, u32 level, u32 hpScaleNum, u32 hpScaleDen, enum Move move)
{
    bool32 slow = (GetMoveEffect(move) == EFFECT_GYRO_BALL);
    u32 maxHP = DamagePreviewHpBound(species, level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS) * hpScaleNum / hpScaleDen;
    u32 hp;

    if (mon->volatiles.transformed)
        return;
    if (maxHP == 0)
        maxHP = 1;
    hp = maxHP * mon->hp / mon->maxHP;
    mon->maxHP = maxHP;
    mon->hp = (hp == 0) ? 1 : hp;

    mon->attack    = DamagePreviewStatBound(GetSpeciesBaseAttack(species),    level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS, NATURE_BOOSTING);
    mon->spAttack  = DamagePreviewStatBound(GetSpeciesBaseSpAttack(species),  level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS, NATURE_BOOSTING);
    mon->defense   = DamagePreviewStatBound(GetSpeciesBaseDefense(species),   level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS, NATURE_BOOSTING);
    mon->spDefense = DamagePreviewStatBound(GetSpeciesBaseSpDefense(species), level, MAX_PER_STAT_IVS, MAX_PER_STAT_EVS, NATURE_BOOSTING);
    mon->speed     = DamagePreviewStatBound(GetSpeciesBaseSpeed(species),     level,
                                            slow ? 0 : MAX_PER_STAT_IVS, slow ? 0 : MAX_PER_STAT_EVS, slow ? NATURE_HINDERING : NATURE_BOOSTING);
}

u32 GetForewarnThreat(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    struct BattlePokemon realAtk;
    enum Ability savedAbility[MAX_BATTLERS_COUNT];
    enum Item savedItem[MAX_BATTLERS_COUNT];
    enum Ability savedAiAbilities[MAX_BATTLERS_COUNT];
    enum Item savedAiItems[MAX_BATTLERS_COUNT];
    enum HoldEffect savedAiHoldEffects[MAX_BATTLERS_COUNT];
    u32 savedAiSpeeds[MAX_BATTLERS_COUNT];
    u32 savedDynamicMoveType = gBattleStruct->dynamicMoveType;
    u32 savedDynamicMoveCategory = gBattleStruct->dynamicMoveCategory;
    enum BattleSide viewerSide = GetBattlerSide(battlerDef);
    enum Species species;
    u32 level, hpScaleNum, hpScaleDen, threat;
    struct SimulatedDamage dmg;

    if (move == MOVE_NONE || IsBattleMoveStatus(move) || !IsBattlerAlive(battlerAtk) || !IsBattlerAlive(battlerDef))
        return 0;

    realAtk = gBattleMons[battlerAtk];
    species = realAtk.species;
    level = realAtk.level;
    // Under Illusion the Forewarn user is facing the disguise, as far as it knows: its species,
    // typing, base stats and innates. The move is still the real mon's, which is the reveal.
    if (GetIllusionMonSpecies(battlerAtk) != SPECIES_NONE)
    {
        species = GetIllusionMonSpecies(battlerAtk);
        level = GetMonData(GetIllusionMonPtr(battlerAtk), MON_DATA_LEVEL, NULL);
    }
    // A Dynamaxed attacker's bar is its multiplied HP, so the fraction is read against that.
    hpScaleNum = realAtk.maxHP;
    hpScaleDen = GetNonDynamaxMaxHP(battlerAtk);
    if (hpScaleDen == 0)
        hpScaleDen = hpScaleNum = 1;

    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        savedAbility[b] = gBattleMons[b].ability;
        savedItem[b] = gBattleMons[b].item;
        savedAiAbilities[b] = gAiLogicData->abilities[b];
        savedAiItems[b] = gAiLogicData->items[b];
        savedAiHoldEffects[b] = gAiLogicData->holdEffects[b];
        savedAiSpeeds[b] = gAiLogicData->speedStats[b];
    }

    // The Forewarn user's side in full, the other side only as revealed (the INFO viewer's
    // reveal bits, which the damage preview reads too).
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        bool32 sameSide = (GetBattlerSide(b) == viewerSide);
        bool32 abilityKnown = sameSide || IsPreviewFoeAbilityKnown(b);
        bool32 itemKnown = sameSide || IsPreviewFoeItemKnown(b);

        gAiLogicData->abilities[b] = abilityKnown ? GetBattlerAbility(b) : ABILITY_NONE;
        gAiLogicData->items[b] = itemKnown ? gBattleMons[b].item : ITEM_NONE;
        gAiLogicData->holdEffects[b] = itemKnown ? GetBattlerHoldEffect(b) : HOLD_EFFECT_NONE;
    }
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        if (gAiLogicData->abilities[b] == ABILITY_NONE)
            gBattleMons[b].ability = ABILITY_NONE;
        if (gAiLogicData->items[b] == ITEM_NONE)
            gBattleMons[b].item = ITEM_NONE;
    }

    if (species != realAtk.species)
    {
        gBattleMons[battlerAtk].species = species;
        gBattleMons[battlerAtk].types[0] = GetSpeciesType(species, 0);
        gBattleMons[battlerAtk].types[1] = GetSpeciesType(species, 1);
        gBattleMons[battlerAtk].types[2] = TYPE_MYSTERY;
    }
    ApplyAttackerBound(&gBattleMons[battlerAtk], species, level, hpScaleNum, hpScaleDen, move);
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
        gAiLogicData->speedStats[b] = GetBattlerTotalSpeedStat(b, gAiLogicData->abilities[b], gAiLogicData->holdEffects[b]);

    // Start the type latches clean (see GetDamagePreviewRange), and put back what was there.
    gBattleStruct->dynamicMoveType = TYPE_NONE;
    gBattleStruct->dynamicMoveCategory = DAMAGE_CATEGORY_NONE;
    {
        struct AiCalcValues aiCalc = {
            .move = move,
            .isDamagePreview = TRUE,
            .gimmickAtk = GIMMICK_NONE,
            .gimmickDef = GIMMICK_NONE,
            .weather = AI_GetWeather(),
            .terrain = gFieldTimers.terrain,
        };
        dmg = AI_CalcDamage(&aiCalc, battlerAtk, battlerDef);
    }

    gBattleMons[battlerAtk] = realAtk;
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        gBattleMons[b].ability = savedAbility[b];
        gBattleMons[b].item = savedItem[b];
        gAiLogicData->abilities[b] = savedAiAbilities[b];
        gAiLogicData->items[b] = savedAiItems[b];
        gAiLogicData->holdEffects[b] = savedAiHoldEffects[b];
        gAiLogicData->speedStats[b] = savedAiSpeeds[b];
    }
    gBattleStruct->dynamicMoveType = savedDynamicMoveType;
    gBattleStruct->dynamicMoveCategory = savedDynamicMoveCategory;

    threat = dmg.maximum;
    if (threat != 0 && GetMoveEffect(move) == EFFECT_REFLECT_DAMAGE)
    {
        threat = gBattleMons[battlerDef].maxHP * FOREWARN_REFLECT_DAMAGE_PERCENT / 100;
        if (threat == 0)
            threat = 1;
    }
    return threat;
}

void ForewarnChooseMoveByMatchup(enum BattlerId battler)
{
    enum BattlerId bestBattler = MAX_BATTLERS_COUNT;
    enum Move bestMove = MOVE_NONE;
    u32 bestThreat = 0;

    // Opponents in battler order, moves in slot order. Only a strictly higher threat replaces the
    // pick, so ties go to the first slot of the first foe, and no RNG is ever drawn. A move that
    // cannot damage the Forewarn user never beats the first move seen, which is the fallback.
    for (enum BattlerId foe = 0; foe < gBattlersCount; foe++)
    {
        if (!IsBattlerAlive(foe) || IsBattlerAlly(foe, battler))
            continue;
        for (u32 j = 0; j < MAX_MON_MOVES; j++)
        {
            enum Move move = gBattleMons[foe].moves[j];
            u32 threat;

            if (move == MOVE_NONE)
                continue;
            threat = GetForewarnThreat(foe, battler, move);
            if (bestMove == MOVE_NONE || threat > bestThreat)
            {
                bestBattler = foe;
                bestMove = move;
                bestThreat = threat;
            }
        }
    }

    if (bestMove == MOVE_NONE)
        return;

    gEffectBattler = bestBattler;
    PREPARE_MOVE_BUFFER(gBattleTextBuff1, bestMove)
    RecordKnownMove(bestBattler, bestMove);
    RecordForewarnedMove(bestBattler, bestMove);
}

void RecordForewarnedMove(enum BattlerId battler, enum Move move)
{
    for (u32 j = 0; j < MAX_MON_MOVES; j++)
    {
        if (gBattleMons[battler].moves[j] == move)
        {
            gBattleStruct->infoForewarnedMoves[GetBattlerSide(battler)][gBattlerPartyIndexes[battler]] |= 1u << j;
            return;
        }
    }
}
