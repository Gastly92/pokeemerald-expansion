#include "global.h"
#include "battle.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "battle_stat_change.h"
#include "battle_ai_record.h"
#include "battle_anim.h"
#include "battle_controllers.h"
#include "battle_ai_util.h"
#include "item.h"
#include "move.h"

// Stat change
static enum StatChangeResult CanDecreaseStat(struct BattleCalcValues *cv, struct StatChange *st);
static enum StatChangeResult DecreaseStat(struct BattleCalcValues *cv, struct StatChange *st);
static enum StatChangeResult IncreaseStat(struct BattleCalcValues *cv, struct StatChange *st);
static void StatChanged(struct BattleCalcValues *cv, struct StatChange *st, bool32 isMaxStage);
static void TryPlayStatChangeAnimation(struct BattleCalcValues *cv, struct StatChange *st);

// Failure handling
static bool32 IsSubstituteBlocked(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsMistProtected(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsFlowerVeilBlocked(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsClearAmuletBlocked(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsIntimidateBlocked(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsAbilityBlocked(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 IsMirrorArmorReflected(struct BattleCalcValues *cv, struct StatChange *st);

// Utitily
static void AdjustStatStage(struct BattleCalcValues *cv, struct StatChange *st);
static bool32 CanAbilityPreventStatLoss(enum Ability ability);
static bool32 AbilityPreventsSpecificStatDrop(u32 ability, u32 stat);
static enum Ability GetInnateStatDropProtector(enum BattlerId battler, u32 stat, bool32 *fullProtection); // FORK: innate-aware stat-drop protection
static u32 GetNumPositiveStats(struct StatChange *st);
static u32 GetNumNegativeStats(struct StatChange *st);
static void SetAdditionalEffectsOnStatChange(struct BattleCalcValues *cv, struct StatChange *st);
static void MarkStatsAsDone(struct StatChange *st, u32 stat);

u32 const sAccurateStatOrder[NUM_BATTLE_STATS] =
{
    STAT_HP,
    STAT_ATK,
    STAT_DEF,
    STAT_SPATK,
    STAT_SPDEF,
    STAT_SPEED,
    STAT_ACC,
    STAT_EVASION,
};

static void SetStrengthSapHealing(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Stat stat)
{
    u32 healAmount = 0;
    switch (stat)
    {
    case STAT_ATK:
        healAmount = gBattleMons[battlerDef].attack;
        break;
    case STAT_DEF:
        healAmount = gBattleMons[battlerDef].defense;
        break;
    case STAT_SPATK:
        healAmount = gBattleMons[battlerDef].spAttack;
        break;
    case STAT_SPDEF:
        healAmount = gBattleMons[battlerDef].spDefense;
        break;
    case STAT_SPEED:
        healAmount = gBattleMons[battlerDef].speed;
        break;
    default:
        errorf("Illegal stat requested");
        return;
    }

    healAmount *= gStatStageRatios[gBattleMons[battlerDef].statStages[stat]][0];
    healAmount /= gStatStageRatios[gBattleMons[battlerDef].statStages[stat]][1];
    gBattleStruct->passiveHpUpdate[battlerAtk] = healAmount;
}

// Not the most elegant implementation but it guarantees that everything works for single and multi targets
static bool32 CheckSpecificMoveCondition(struct BattleCalcValues *cv, struct StatChange *st)
{
    switch (cv->moveEffect)
    {
    case EFFECT_CAPTIVATE:
        // FORK: innate Oblivious blocks Captivate like the real ability (IsInnateActive supplies suppression parity).
        if (cv->abilities[cv->battlerDef] == ABILITY_OBLIVIOUS || IsInnateActive(cv->battlerDef, ABILITY_OBLIVIOUS))
        {
            if (!st->onlyChecking)
            {
                st->script = BattleScript_AbilityProtectedTarget;
                gBattlerAbility = gBattleScripting.battler = cv->battlerDef;
                gLastUsedAbility = ABILITY_OBLIVIOUS;
                RecordAbilityBattle(cv->battlerDef, ABILITY_OBLIVIOUS);
                // FORK: innate — the pop-up reads the primary slot, so show Oblivious; real path untouched.
                if (cv->abilities[cv->battlerDef] != ABILITY_OBLIVIOUS)
                    gBattleScripting.abilityPopupOverwrite = ABILITY_OBLIVIOUS;
            }
            return TRUE;
        }
        break;
    case EFFECT_STRENGTH_SAP:
        if (CompareStat(cv->battlerDef, STAT_ATK, MIN_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
        {
            if (!st->onlyChecking)
            {
                PREPARE_STAT_BUFFER(gBattleTextBuff1, STAT_ATK);
                gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_WONT_CHANGE;
                st->script = BattleScript_DecreaseStatChangeMessage;
                gBattleScripting.battler = cv->battlerDef;
            }
            return TRUE;
        }
        else
        {
            SetStrengthSapHealing(cv->battlerAtk, cv->battlerDef, STAT_ATK);
            st->additionalEffectTriggers = TRUE;
        }
        break;
    case EFFECT_ROTOTILLER:
        if (!IsBattlerGrounded(cv->battlerDef, cv->abilities[cv->battlerDef], cv->holdEffects[cv->battlerDef])
         || !IS_BATTLER_OF_TYPE(cv->battlerDef, TYPE_GRASS))
        {
            if (!st->onlyChecking)
            {
                st->script = BattleScript_ItDoesntAffectScrTarget;
                gBattleScripting.battler = cv->battlerDef;
            }
            return TRUE;
        }
        break;
    case EFFECT_FLOWER_SHIELD:
        if (!IS_BATTLER_OF_TYPE(cv->battlerDef, TYPE_GRASS))
        {
            if (!st->onlyChecking)
            {
                st->script = BattleScript_ItDoesntAffectScrTarget;
                gBattleScripting.battler = cv->battlerDef;
            }
            return TRUE;
        }
        break;
    case EFFECT_TOXIC_THREAD:
        if (CanBePoisoned(cv->battlerAtk, cv->battlerDef, cv->abilities[cv->battlerAtk], cv->abilities[cv->battlerDef]))
        {
            st->additionalEffectTriggers = TRUE;
            if (!st->onlyChecking)
                st->moveScript = BattleScript_ToxicThread;
        }
        break;
    case EFFECT_SWAGGER:
        if (cv->abilities[cv->battlerDef] == ABILITY_OWN_TEMPO)
        {
            if (!st->onlyChecking)
            {
                st->moveScript = BattleScript_OwnTempoPrevents;
                gBattlerAbility = cv->battlerDef;
                gLastUsedAbility = ABILITY_OWN_TEMPO;
                RecordAbilityBattle(cv->battlerDef, ABILITY_OWN_TEMPO);
            }
        }
        else if (CanBeConfused(cv->battlerAtk, cv->battlerDef))
        {
            st->additionalEffectTriggers = TRUE;
            if (!st->onlyChecking)
                st->moveScript = BattleScript_SwaggerConfusion;
        }
        break;
    case EFFECT_TAR_SHOT:
        if (!gBattleMons[cv->battlerDef].volatiles.tarShot && GetActiveGimmick(cv->battlerDef) != GIMMICK_TERA)
        {
            st->additionalEffectTriggers = TRUE;
            if (!st->onlyChecking)
            {
                gBattleMons[cv->battlerDef].volatiles.tarShot = TRUE;
                st->moveScript = BattleScript_TarShotMessage;
            }
        }
        break;
    default:
        break;
    }

    return FALSE;

}

bool32 CanAnyStatChange(struct BattleCalcValues *cv, struct StatChange *st)
{
    u32 numAdditionalEffects = GetMoveAdditionalEffectCount(cv->move);
    bool32 canAnyStatChange = FALSE;
    bool32 statChangeBlockedOnBattler = FALSE;

    if (CheckSpecificMoveCondition(cv, st) || IsSubstituteBlocked(cv, st))
        statChangeBlockedOnBattler = TRUE;

    for (u32 i = 0; i < numAdditionalEffects; i++)
    {
        const struct AdditionalEffect *additionalEffect = GetMoveAdditionalEffectById(cv->move, i);

        for (enum Stat j = STAT_ATK; j < NUM_BATTLE_STATS; j++)
        {
            st->stat = sAccurateStatOrder[j];
            st->stage = GetStatStage(st->stat, additionalEffect);

            if (st->stage == 0)
                continue;

            if (additionalEffect->moveEffect == STAT_CHANGE_EFFECT_MINUS)
                st->stage = -1 * st->stage;

            AdjustStatStage(cv, st);
            SetStatChange(cv->battlerDef, st->stat, st->stage);

            if (statChangeBlockedOnBattler) // Still need to collect stats for proper failure
                continue;

            // Workaround for contrary
            if (cv->moveEffect == EFFECT_BELLY_DRUM && !CompareStat(cv->battlerDef, st->stat, MAX_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
            {
                canAnyStatChange = TRUE;
                continue;
            }

            if (st->stage < 0)
            {
                if (CompareStat(cv->battlerDef, st->stat, MIN_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
                    continue;
            }
            else
            {
                if (CompareStat(cv->battlerDef, st->stat, MAX_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
                    continue;
            }

            if (st->stage < 0 && CanDecreaseStat(cv, st) == STAT_CHANGE_DIDNT_WORK)
                continue;

            canAnyStatChange = TRUE;
        }
    }

    return canAnyStatChange;
}

enum StatChangeResult TryStatChange(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (CheckSpecificMoveCondition(cv, st) || IsSubstituteBlocked(cv, st))
    {
        st->nextBattler = TRUE;
        return STAT_CHANGE_BLOCKED_BY_TARGET;
    }

    enum StatChangeResult result = STAT_CHANGE_DIDNT_WORK;
    for (u32 i = 0; i < st->statStageAmount; i++)
    {
        if (i + 1 == st->statStageAmount) // Avoids redundant looping
            st->nextBattler = TRUE;

        if (st->statStageQueue[i].done)
            continue;

        st->stat = st->statStageQueue[i].stat;
        st->stage = st->statStageQueue[i].stage;
        st->statStageQueue[i].done = TRUE;

        if (cv->move == MOVE_NONE)
            AdjustStatStage(cv, st);

        if (st->stage < 0)
        {
            if (CanDecreaseStat(cv, st) == STAT_CHANGE_DIDNT_WORK)
            {
                if (st->silentFailure)
                    continue;
                result = STAT_CHANGE_BLOCKED_BY_TARGET;
                break;
            }

            if (DecreaseStat(cv, st) == STAT_CHANGE_WORKED)
            {
                result = STAT_CHANGE_WORKED;
                break;
            }
        }
        else if (IncreaseStat(cv, st) == STAT_CHANGE_WORKED)
        {
            result = STAT_CHANGE_WORKED;
            break;
        }
    }

    if (st->nextBattler && !st->onlyChecking) // Set volatiles after all stats are done
        SetAdditionalEffectsOnStatChange(cv, st);

    return result;
}

enum StatChangeResult TrySingleStatChange(struct BattleCalcValues *cv, struct StatChange *st)
{
    AdjustStatStage(cv, st);

    if (st->stage < 0)
    {
        if (CanDecreaseStat(cv, st) == STAT_CHANGE_DIDNT_WORK)
            return STAT_CHANGE_DIDNT_WORK;

        if (DecreaseStat(cv, st) == STAT_CHANGE_WORKED)
            return STAT_CHANGE_WORKED;
    }
    else if (IncreaseStat(cv, st) == STAT_CHANGE_WORKED)
    {
        return STAT_CHANGE_WORKED;
    }

    return STAT_CHANGE_DIDNT_WORK;
}

static enum StatChangeResult CanDecreaseStat(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (IsMistProtected(cv, st)
     || IsIntimidateBlocked(cv, st)
     || IsFlowerVeilBlocked(cv, st)
     || IsClearAmuletBlocked(cv, st)
     || IsAbilityBlocked(cv, st)
     || IsMirrorArmorReflected(cv, st))
        return STAT_CHANGE_DIDNT_WORK;
    return STAT_CHANGE_WORKED;
}

static enum StatChangeResult DecreaseStat(struct BattleCalcValues *cv, struct StatChange *st)
{
    u32 currStage = gBattleMons[cv->battlerDef].statStages[st->stat];

    PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);

    if (currStage == (MIN_STAT_STAGE + 1))
        st->stage = -1;
    else if (currStage == 2 && st->stage < -2)
        st->stage = -2;

    if (st->stage == -2)
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_STATHARSHLY);
    }
    else if (st->stage <= -3)
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_SEVERELY);
    }
    else
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_EMPTYSTRING3);
    }

    if (currStage == MIN_STAT_STAGE)
    {
        if (st->onlyChecking)
            return STAT_CHANGE_DIDNT_WORK;

        if (cv->moveEffect == EFFECT_BELLY_DRUM)
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_CHANGED_BELLY_DRUM;
        else
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_WONT_CHANGE;

        gBattleScripting.battler = cv->battlerDef;
        st->script = BattleScript_DecreaseStatChangeMessageMinStat;
        return STAT_CHANGE_WORKED; // Handle failure
    }
    else if (!st->onlyChecking)
    {
        gBattleMons[cv->battlerDef].volatiles.tryEjectPack = TRUE;
        gProtectStructs[cv->battlerDef].lashOutAffected = TRUE;
    }

    if (!st->onlyChecking)
    {
        if (!st->stickyWeb)
        {
            if (st->certain || (cv->battlerAtk != cv->battlerDef && IsBattlerAlly(cv->battlerAtk, cv->battlerDef)))
                gBattleStruct->ignoreDefiant = TRUE;
        }

        StatChanged(cv, st, FALSE);
        st->script = BattleScript_DecreaseStatChangeMessage;
        TryPlayStatChangeAnimation(cv, st);
    }

    return STAT_CHANGE_WORKED;
}

static enum StatChangeResult IncreaseStat(struct BattleCalcValues *cv, struct StatChange *st)
{
    u32 currStage = gBattleMons[cv->battlerDef].statStages[st->stat];
    bool32 isMaxStage = st->stage >= 12;

    PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);

    if (currStage == MAX_STAT_STAGE - 1)
        st->stage = 1;
    else if (currStage == MAX_STAT_STAGE - 2 && st->stage > 2)
        st->stage = 2;

    if (st->stage == 2)
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_STATSHARPLY);
    }
    else if (st->stage >= 3)
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_DRASTICALLY);
    }
    else
    {
        PREPARE_STRING_BUFFER(gBattleTextBuff2, STRINGID_EMPTYSTRING3);
    }

    if (gBattleMons[cv->battlerDef].statStages[st->stat] == MAX_STAT_STAGE)
    {
        if (st->onlyChecking)
            return STAT_CHANGE_DIDNT_WORK;

        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_WONT_CHANGE;
        st->script = BattleScript_StatDidntChangeMessagePause;
        gBattleScripting.battler = cv->battlerDef;
        return STAT_CHANGE_WORKED; // Handle failure
    }
    else if (!st->onlyChecking)
    {
        u32 stageIncrease = st->stage;

        if ((st->stage + gBattleMons[cv->battlerDef].statStages[st->stat]) > MAX_STAT_STAGE)
            stageIncrease  = MAX_STAT_STAGE - gBattleMons[cv->battlerDef].statStages[st->stat];

        if (stageIncrease > 0)
        {
            // Check Mirror Herb / Opportunist
            for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
            {
                if (IsBattlerAlly(battler, cv->battlerDef))
                    continue; // Only triggers on opposing side

                if (CompareStat(battler, st->stat, MAX_STAT_STAGE, CMP_EQUAL, cv->abilities[battler]))
                    continue;

                if (cv->abilities[battler] == ABILITY_OPPORTUNIST && !st->opportunistActivation)
                    gProtectStructs[battler].activateOpportunist = TRUE;

                if (cv->holdEffects[battler] == HOLD_EFFECT_MIRROR_HERB && !st->mirrorHerbActivation)
                    gProtectStructs[battler].eatMirrorHerb = TRUE;

                if (gProtectStructs[battler].activateOpportunist || gProtectStructs[battler].eatMirrorHerb)
                {
                    gQueuedStatBoosts[battler].stats |= (1 << (st->stat - 1)); // -1 to start at atk
                    gQueuedStatBoosts[battler].statChanges[st->stat - 1] += stageIncrease;
                }
            }
        }
    }

    if (!st->onlyChecking)
    {
        gProtectStructs[cv->battlerDef].statRaised = TRUE;
        StatChanged(cv, st, isMaxStage);
        st->script = BattleScript_IncreaseStatChangeMessage;
        TryPlayStatChangeAnimation(cv, st);
    }

    return STAT_CHANGE_WORKED;
}

static void StatChanged(struct BattleCalcValues *cv, struct StatChange *st, bool32 isMaxStage)
{
    gBattleStruct->moveResultFlags[cv->battlerDef] |= MOVE_RESULT_STAT_CHANGED;
    gBattleScripting.battler = cv->battlerDef;
    gBattleMons[cv->battlerDef].statStages[st->stat] += st->stage;

    if (st->stage > 0)
    {
        if (gBattleMons[cv->battlerDef].statStages[st->stat] > MAX_STAT_STAGE)
            gBattleMons[cv->battlerDef].statStages[st->stat] = MAX_STAT_STAGE;
    }
    else
    {
        if (gBattleMons[cv->battlerDef].statStages[st->stat] < MIN_STAT_STAGE)
            gBattleMons[cv->battlerDef].statStages[st->stat] = MIN_STAT_STAGE;
    }

    if (cv->moveEffect == EFFECT_STOCKPILE && st->stage > 0)
    {
        switch (st->stat)
        {
        case STAT_DEF:
            gBattleMons[cv->battlerDef].volatiles.stockpileDef++;
            break;
        case STAT_SPDEF:
            gBattleMons[cv->battlerDef].volatiles.stockpileSpDef++;
            break;
        default:
            break;
        }
    }

    if (cv->moveEffect == EFFECT_BELLY_DRUM)
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_CHANGED_BELLY_DRUM;
    }
    else if (isMaxStage)
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_MAXED;
    }
    else if (st->itemMessage)
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_CHANGED_ITEM;
    }
    else
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_STAT_CHANGED;
    }
}

static void TryPlayStatChangeAnimation(struct BattleCalcValues *cv, struct StatChange *st)
{
    u32 statAnimId = st->stat;

    if (st->stage <= -1) // goes down
    {
        if (gBattleStruct->negativeAnimPlayed)
            return;

        u32 numNegativeStats = 0;
        bool32 isStatChangeByTwo = abs(st->stage) > 1;

        gBattleStruct->negativeAnimPlayed = TRUE;
        numNegativeStats = GetNumNegativeStats(st);

        statAnimId += isStatChangeByTwo ? STAT_ANIM_MINUS2 : STAT_ANIM_MINUS1;

        // FORK: also suppress the down-arrow when an innate Defiant / Competitive will re-raise the stat
        // (FEATURE_INNATE_ABILITIES), matching the chosen-ability behavior on the same line.
        if (ShouldDefiantCompetitiveActivate(cv->battlerDef, cv->abilities[cv->battlerDef])
         || (IsInnateActive(cv->battlerDef, ABILITY_DEFIANT) && ShouldDefiantCompetitiveActivate(cv->battlerDef, ABILITY_DEFIANT))
         || (IsInnateActive(cv->battlerDef, ABILITY_COMPETITIVE) && ShouldDefiantCompetitiveActivate(cv->battlerDef, ABILITY_COMPETITIVE)))
            numNegativeStats = 0;

        if (numNegativeStats > 1) // more than one stat, so the color is gray
            statAnimId = isStatChangeByTwo ? STAT_ANIM_MULTIPLE_MINUS2 : STAT_ANIM_MULTIPLE_MINUS1;
    }
    else // goes up
    {
        if (gBattleStruct->positiveAnimPlayed)
            return;

        u32 numPositiveStats = 0;
        bool32 isStatChangeByTwo = st->stage > 1;

        gBattleStruct->positiveAnimPlayed = TRUE;
        numPositiveStats = GetNumPositiveStats(st);

        statAnimId += isStatChangeByTwo ? STAT_ANIM_PLUS2 : STAT_ANIM_PLUS1;

        if (numPositiveStats > 1)
            statAnimId = isStatChangeByTwo ? STAT_ANIM_MULTIPLE_PLUS2 : STAT_ANIM_MULTIPLE_PLUS1;
    }

    BtlController_EmitBattleAnimation(cv->battlerDef, B_COMM_TO_CONTROLLER, B_ANIM_STATS_CHANGE, statAnimId);
    MarkBattlerForControllerExec(cv->battlerDef);
}

static bool32 IsSubstituteBlocked(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (st->ignoreCertainFailure)
        return FALSE;

    if (st->certain || GetBattlerMoveTargetType(cv->battlerAtk, cv->move) == TARGET_ALLY)
        return FALSE;

    if (!IsSubstituteProtected(cv->battlerAtk, cv->battlerDef, cv->abilities[cv->battlerAtk], cv->move))
        return FALSE;

    if (!st->onlyChecking)
    {
        gBattleScripting.battler = cv->battlerDef;
        st->script = BattleScript_ButItFailedRet;
    }

    return TRUE;
}

static bool32 IsMistProtected(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (gSideTimers[GetBattlerSide(cv->battlerDef)].mistTimer == 0)
        return FALSE;

    if (st->certain)
        return FALSE;

    // FORK: innate-aware Infiltrator ignores the foe's Mist (FEATURE_INNATE_ABILITIES)
    if (!IsBattlerAlly(cv->battlerDef, cv->battlerAtk)
     && (cv->abilities[cv->battlerAtk] == ABILITY_INFILTRATOR || IsInnateActive(cv->battlerAtk, ABILITY_INFILTRATOR)))
        return FALSE;

    if (!st->onlyChecking)
    {
        MarkStatsAsDone(st, NUM_BATTLE_STATS);
        gBattleScripting.battler = cv->battlerDef;
        st->script = BattleScript_MistProtected;
    }

    return TRUE;
}

static enum BattlerId StatChange_IsFlowerVeilProtected(struct BattleCalcValues *cv)
{
    if (!IS_BATTLER_OF_TYPE(cv->battlerDef, TYPE_GRASS))
        return MAX_BATTLERS_COUNT;

    for (enum BattlerId battler = B_BATTLER_0; battler < gBattlersCount; battler++)
    {
        if (!IsBattlerAlly(cv->battlerDef, battler))
            continue;
        // FORK: an innate Flower Veil ally protects a Grass battler's stats like the real ability (Batch U).
        if (cv->abilities[battler] == ABILITY_FLOWER_VEIL || IsInnateActive(battler, ABILITY_FLOWER_VEIL))
            return battler;
    }

    return MAX_BATTLERS_COUNT;
}

static bool32 IsFlowerVeilBlocked(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (st->certain)
        return FALSE;

    enum BattlerId flowerVeilBattler = StatChange_IsFlowerVeilProtected(cv);

    if (flowerVeilBattler == MAX_BATTLERS_COUNT)
        return FALSE;

    if (!st->onlyChecking)
    {
        st->script = BattleScript_FlowerVeilProtectsRet;
        gBattleScripting.battler = cv->battlerDef;
        gBattlerAbility = flowerVeilBattler;
        gLastUsedAbility = ABILITY_FLOWER_VEIL;
        // FORK: when an innate Flower Veil (the protector's chosen ability differs) blocks the drop,
        // overwrite the pop-up so it shows Flower Veil, not the chosen ability (Batch U).
        if (GetBattlerAbility(flowerVeilBattler) != ABILITY_FLOWER_VEIL)
            gBattleScripting.abilityPopupOverwrite = ABILITY_FLOWER_VEIL;
        MarkStatsAsDone(st, NUM_BATTLE_STATS);
        RecordAbilityBattle(gBattlerAbility, ABILITY_FLOWER_VEIL);
    }

    return TRUE;
}

static bool32 IsClearAmuletBlocked(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (st->certain)
        return FALSE;

    if (cv->holdEffects[cv->battlerDef] != HOLD_EFFECT_CLEAR_AMULET)
        return FALSE;

    if (!st->onlyChecking)
    {
        st->script = BattleScript_ItemNoStatLoss;
        gBattleScripting.battler = cv->battlerDef;
        gLastUsedItem = gBattleMons[cv->battlerDef].item;
        MarkStatsAsDone(st, NUM_BATTLE_STATS);
        RecordItemEffectBattle(cv->battlerDef, HOLD_EFFECT_CLEAR_AMULET);
    }

    return TRUE;
}

static bool32 IsIntimidateBlocked(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (!st->intimidate)
        return FALSE;

    // FORK: an innate Guard Dog (the chosen ability differs, so the switch below would miss it) is immune
    // to Intimidate's Attack drop and instead boosts its own Attack by 1 stage, exactly like the real
    // ability (Batch V — the switch-in Intimidate driver Batch L built now lets the reaction fire on an
    // innate holder, completing the half Batch S deferred). Mirrors the ABILITY_GUARD_DOG switch case
    // (Flower Veil ordering, min-stage guard, DefiantActivates script) and overwrites the pop-up/record to
    // the innate (CreateAbilityPopUp reads the primary slot). Only fires when the chosen ability isn't
    // itself Guard Dog (that case is handled by the switch). IsInnateActive supplies suppression parity.
    if (cv->abilities[cv->battlerDef] != ABILITY_GUARD_DOG
     && IsInnateActive(cv->battlerDef, ABILITY_GUARD_DOG))
    {
        enum BattlerId flowerVeilBattler = StatChange_IsFlowerVeilProtected(cv);

        if (flowerVeilBattler != MAX_BATTLERS_COUNT
         && GetBattlerRawSpeedOrder(flowerVeilBattler) < GetBattlerRawSpeedOrder(cv->battlerDef))
            return FALSE;

        if (!CompareStat(cv->battlerDef, STAT_ATK, MIN_STAT_STAGE, CMP_GREATER_THAN, ABILITY_GUARD_DOG))
            return FALSE;

        SetStatChange2(cv->battlerDef, st->stat, -1 * st->stage);
        st->script = BattleScript_DefiantActivates;
        gEffectBattler = cv->battlerDef;
        gLastUsedAbility = ABILITY_GUARD_DOG;
        gBattlerAbility = cv->battlerDef;
        gBattleScripting.battler = cv->battlerDef;
        gBattleScripting.abilityPopupOverwrite = ABILITY_GUARD_DOG;
        MarkStatsAsDone(st, st->stat);
        RecordAbilityBattle(cv->battlerDef, ABILITY_GUARD_DOG);
        return TRUE;
    }

    // FORK: an innate Oblivious / Scrappy / Inner Focus / Own Tempo (the chosen ability differs, so the switch
    // below would miss it) is unaffected by Intimidate exactly like the real ability (GEN_8+). Mirror the switch
    // cases and overwrite the pop-up/record to the innate (CreateAbilityPopUp reads the primary slot). Only fires
    // when the chosen ability isn't itself an Intimidate-immune one (those are handled by the switch).
    // IsInnateActive supplies suppression parity. An innate Guard Dog is handled by the reactive block above,
    // so it stays out of the pure-immunity list here (and keeps the != guard in its chosen-only switch case).
    if (cv->abilities[cv->battlerDef] != ABILITY_OBLIVIOUS
     && cv->abilities[cv->battlerDef] != ABILITY_INNER_FOCUS
     && cv->abilities[cv->battlerDef] != ABILITY_SCRAPPY
     && cv->abilities[cv->battlerDef] != ABILITY_OWN_TEMPO
     && cv->abilities[cv->battlerDef] != ABILITY_GUARD_DOG)
    {
        enum Ability innateImmunity = ABILITY_NONE;
        if (IsInnateActive(cv->battlerDef, ABILITY_OBLIVIOUS))
            innateImmunity = ABILITY_OBLIVIOUS;
        else if (IsInnateActive(cv->battlerDef, ABILITY_SCRAPPY))
            innateImmunity = ABILITY_SCRAPPY;
        else if (IsInnateActive(cv->battlerDef, ABILITY_INNER_FOCUS))
            innateImmunity = ABILITY_INNER_FOCUS;
        else if (IsInnateActive(cv->battlerDef, ABILITY_OWN_TEMPO))
            innateImmunity = ABILITY_OWN_TEMPO;
        if (innateImmunity != ABILITY_NONE)
        {
            if (GetConfig(B_UPDATED_INTIMIDATE) < GEN_8)
                return FALSE;
            PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);
            st->script = BattleScript_AbilityNoSpecificStatLoss;
            gLastUsedAbility = innateImmunity;
            gBattlerAbility = cv->battlerDef;
            gBattleScripting.battler = cv->battlerDef;
            gBattleScripting.abilityPopupOverwrite = innateImmunity;
            MarkStatsAsDone(st, st->stat);
            RecordAbilityBattle(cv->battlerDef, innateImmunity);
            return TRUE;
        }
    }

    switch (cv->abilities[cv->battlerDef])
    {
    case ABILITY_INNER_FOCUS:
    case ABILITY_SCRAPPY:
    case ABILITY_OWN_TEMPO:
    case ABILITY_OBLIVIOUS:
        if (GetConfig(B_UPDATED_INTIMIDATE) < GEN_8)
            return FALSE;
        PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);
        st->script = BattleScript_AbilityNoSpecificStatLoss;
        break;
    case ABILITY_GUARD_DOG:
    {
        enum BattlerId flowerVeilBattler = StatChange_IsFlowerVeilProtected(cv);

        if (flowerVeilBattler != MAX_BATTLERS_COUNT
         && GetBattlerRawSpeedOrder(flowerVeilBattler) < GetBattlerRawSpeedOrder(cv->battlerDef))
            return FALSE;

        if (!CompareStat(cv->battlerDef, STAT_ATK, MIN_STAT_STAGE, CMP_GREATER_THAN, cv->abilities[cv->battlerDef]))
            return FALSE;

        SetStatChange2(cv->battlerDef, st->stat, -1 * st->stage);
        st->script = BattleScript_DefiantActivates;
        gEffectBattler = cv->battlerDef;
        break;
    }
    default:
        return FALSE;
    }

    gLastUsedAbility = cv->abilities[cv->battlerDef];
    gBattlerAbility = cv->battlerDef;
    gBattleScripting.battler = cv->battlerDef;
    MarkStatsAsDone(st, st->stat);
    RecordAbilityBattle(cv->battlerDef, cv->abilities[cv->battlerDef]);
    return TRUE;
}

static bool32 IsAbilityBlocked(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (st->certain)
        return FALSE;

    // FORK: an innate Clear Body / White Smoke / Full Metal Body (any stat), Hyper Cutter (Atk), Big Pecks
    // (Def), Keen Eye / Illuminate / Mind's Eye (Acc) prevents the holder's stat from being lowered exactly
    // like the real ability (pure boon, 1:1 copy). The chosen-ability paths below miss it when the chosen
    // ability differs, so handle it here when the chosen ability doesn't already block the drop, overwriting
    // the pop-up/record to the innate (CreateAbilityPopUp reads the primary slot; the overwrite also stops
    // the reveal leaking the chosen ability). IsInnateActive supplies suppression parity (all are breakable,
    // so an attacker's Mold Breaker pierces them — except Full Metal Body, .breakable = FALSE, immune for
    // free). Full protectors (Clear Body / White Smoke / Full Metal Body) use the whole-stat script;
    // single-stat protectors the specific one. Mirrors the innate-Oblivious block in IsIntimidateBlocked.
    if (!CanAbilityPreventStatLoss(cv->abilities[cv->battlerDef])
     && !AbilityPreventsSpecificStatDrop(cv->abilities[cv->battlerDef], st->stat))
    {
        bool32 fullProtection = FALSE;
        enum Ability innate = GetInnateStatDropProtector(cv->battlerDef, st->stat, &fullProtection);

        if (innate != ABILITY_NONE)
        {
            if (!st->onlyChecking)
            {
                if (fullProtection)
                {
                    MarkStatsAsDone(st, NUM_BATTLE_STATS);
                    st->script = BattleScript_AbilityNoStatLoss;
                }
                else
                {
                    PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);
                    MarkStatsAsDone(st, st->stat);
                    st->script = BattleScript_AbilityNoSpecificStatLoss;
                }
                gLastUsedAbility = innate;
                gBattlerAbility = cv->battlerDef;
                gBattleScripting.battler = cv->battlerDef;
                gBattleScripting.abilityPopupOverwrite = innate;
                RecordAbilityBattle(cv->battlerDef, innate);
            }
            return TRUE;
        }
    }

    if (CanAbilityPreventStatLoss(cv->abilities[cv->battlerDef]))
    {
        if (!st->onlyChecking)
        {
            MarkStatsAsDone(st, NUM_BATTLE_STATS);
            st->script = BattleScript_AbilityNoStatLoss;
        }
    }
    else if (AbilityPreventsSpecificStatDrop(cv->abilities[cv->battlerDef], st->stat))
    {
        if (!st->onlyChecking)
        {
            MarkStatsAsDone(st, st->stat);
            PREPARE_STAT_BUFFER(gBattleTextBuff1, st->stat);
            st->script = BattleScript_AbilityNoSpecificStatLoss;
        }
    }
    else
    {
        return FALSE;
    }

    if (!st->onlyChecking)
    {
        gBattleScripting.battler = cv->battlerDef;
        gBattlerAbility = cv->battlerDef;
        gLastUsedAbility = cv->abilities[cv->battlerDef];
        RecordAbilityBattle(cv->battlerDef, gLastUsedAbility);
    }

    return TRUE;
}

static bool32 IsMirrorArmorReflected(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (cv->abilities[cv->battlerDef] != ABILITY_MIRROR_ARMOR
     || st->ignoreMirrorArmored
     || st->certain)
        return FALSE;

    if (st->onlyChecking && !st->ignoreCertainFailure)
        return TRUE;

    if (gBattleStruct->moveResultFlags[cv->battlerDef] & MOVE_RESULT_MIRROR_ARMOR_PENDING || !st->ignoreCertainFailure)
    {
        st->script = BattleScript_MirrorArmorReflect;
        gBattlerAbility = cv->battlerDef;
        RecordAbilityBattle(cv->battlerDef, cv->abilities[cv->battlerDef]);

        if (st->stickyWeb)
        {
            if (gSideTimers[GetBattlerSide(cv->battlerDef)].stickyWebBattlerId != 0xFF)
            {
                gBattleScripting.battler = gSideTimers[GetBattlerSide(cv->battlerDef)].stickyWebBattlerId;
            }
            else
            {
                st->script = BattleScript_AbilityPopUp;
                return TRUE;
            }
        }
        else
        {
            if (cv->battlerAtk == cv->battlerDef)
                gBattleScripting.battler = cv->battlerDef;
            else
                gBattleScripting.battler = cv->battlerAtk;

            gBattleStruct->allowPartingShot = TRUE;
        }

        for (u32 i = 0; i < st->statStageAmount; i++)
        {
            enum Stat stat = st->statStageQueue[i].stat;
            s32 stage = st->statStageQueue[i].stage;
            if (stage < 0)
            {
                st->statStageQueue[i].done = TRUE;
                SetStatChange2(gBattleScripting.battler, stat, stage);
            }
        }

        return TRUE;
    }

    return FALSE;
}

// There is a similar function AI_GetAdjustedStatStage that needs to be updated if things are changed here
static void AdjustStatStage(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (cv->moveEffect == EFFECT_GROWTH && GetAttackerWeather(cv->battlerDef, cv->holdEffects[cv->battlerDef], cv->abilities[cv->battlerDef], GetWeather()) & B_WEATHER_SUN)
        st->stage = 2;

    if (st->stage == STAT_CHANGE_FORCE_MAX)
        st->stage = 12;

    switch (cv->abilities[cv->battlerDef])
    {
    case ABILITY_CONTRARY:
        st->stage = -1 * st->stage;
        if (!st->onlyChecking)
            RecordAbilityBattle(cv->battlerDef, cv->abilities[cv->battlerDef]);
        break;
    case ABILITY_SIMPLE:
        st->stage = 2 * st->stage;
        if (!st->onlyChecking)
            RecordAbilityBattle(cv->battlerDef, cv->abilities[cv->battlerDef]);
        break;
    default:
        break;
    }
}

static bool32 CanAbilityPreventStatLoss(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_CLEAR_BODY:
    case ABILITY_FULL_METAL_BODY:
    case ABILITY_WHITE_SMOKE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 AbilityPreventsSpecificStatDrop(u32 ability, u32 stat)
{
    switch (ability)
    {
    case ABILITY_ILLUMINATE:
        if (B_ILLUMINATE_EFFECT < GEN_9)
            return FALSE;
    case ABILITY_KEEN_EYE:
    case ABILITY_MINDS_EYE:
        return stat == STAT_ACC;
    case ABILITY_HYPER_CUTTER:
        return stat == STAT_ATK;
    case ABILITY_BIG_PECKS:
        return stat == STAT_DEF;
    default:
        return FALSE;
    }
}

// FORK: returns the innate stat-drop-protecting ability active on `battler` for a drop of `stat`
// (Clear Body / White Smoke / Full Metal Body block any stat, Hyper Cutter Atk, Big Pecks Def,
// Keen Eye / Illuminate / Mind's Eye Acc), or ABILITY_NONE if none. Sets *fullProtection TRUE for the
// whole-stat protectors so the caller can pick the right message/script. Suppression parity via
// IsInnateActive (all are breakable EXCEPT Full Metal Body, whose .breakable = FALSE makes it immune to
// Mold Breaker for free). Illuminate only protects accuracy in Gen 9+. See IsAbilityBlocked for how this
// is applied when the chosen ability doesn't already block the drop.
static enum Ability GetInnateStatDropProtector(enum BattlerId battler, u32 stat, bool32 *fullProtection)
{
    *fullProtection = FALSE;
    if (IsInnateActive(battler, ABILITY_CLEAR_BODY))
    {
        *fullProtection = TRUE;
        return ABILITY_CLEAR_BODY;
    }
    // FORK: Full Metal Body is the unbreakable clone of Clear Body (Batch Y4). It protects any stat
    // exactly like Clear Body; IsInnateActive reads its own .breakable = FALSE, so unlike Clear Body /
    // White Smoke an attacker's Mold Breaker cannot pierce it — the canon split, for free.
    if (IsInnateActive(battler, ABILITY_FULL_METAL_BODY))
    {
        *fullProtection = TRUE;
        return ABILITY_FULL_METAL_BODY;
    }
    if (IsInnateActive(battler, ABILITY_WHITE_SMOKE))
    {
        *fullProtection = TRUE;
        return ABILITY_WHITE_SMOKE;
    }
    if (stat == STAT_ATK && IsInnateActive(battler, ABILITY_HYPER_CUTTER))
        return ABILITY_HYPER_CUTTER;
    if (stat == STAT_DEF && IsInnateActive(battler, ABILITY_BIG_PECKS))
        return ABILITY_BIG_PECKS;
    if (stat == STAT_ACC && IsInnateActive(battler, ABILITY_KEEN_EYE))
        return ABILITY_KEEN_EYE;
    if (stat == STAT_ACC && IsInnateActive(battler, ABILITY_MINDS_EYE)) // FORK: Mind's Eye keeps accuracy from being lowered (Keen Eye clone, Batch Y4)
        return ABILITY_MINDS_EYE;
    if (stat == STAT_ACC && GetConfig(B_ILLUMINATE_EFFECT) >= GEN_9 && IsInnateActive(battler, ABILITY_ILLUMINATE))
        return ABILITY_ILLUMINATE;
    return ABILITY_NONE;
}

u32 GetStatStage(u32 stat, const struct AdditionalEffect *additionalEffect)
{
    switch (stat)
    {
    case STAT_ATK:     return additionalEffect->attack;
    case STAT_DEF:     return additionalEffect->defense;
    case STAT_SPEED:   return additionalEffect->speed;
    case STAT_SPATK:   return additionalEffect->spAtk;
    case STAT_SPDEF:   return additionalEffect->spDef;
    case STAT_ACC:     return additionalEffect->accuracy;
    case STAT_EVASION: return additionalEffect->evasion;
    }

    return 0;
}

static u32 GetNumPositiveStats(struct StatChange *st)
{
    u32 num = 0;
    for (u32 i = 0; i < st->statStageAmount; i++)
    {
        if (st->statStageQueue[i].stage > 0)
            num++;
    }
    return num;
}

static u32 GetNumNegativeStats(struct StatChange *st)
{
    u32 num = 0;
    for (u32 i = 0; i < st->statStageAmount; i++)
    {
        if (st->statStageQueue[i].stage < 0)
            num++;
    }
    return num;
}

void SetStatChange(enum BattlerId battler, enum Stat stat, s32 stage)
{
    gSpecialStatuses[battler].statStageQueue[gSpecialStatuses[battler].statStageAmount].stat = stat;
    gSpecialStatuses[battler].statStageQueue[gSpecialStatuses[battler].statStageAmount].stage = stage;
    gSpecialStatuses[battler].statStageAmount++;
}

// Used for stat change responses like Defiant and Mirror Armor
void SetStatChange2(enum BattlerId battler, enum Stat stat, s32 stage)
{
    gSpecialStatuses[battler].statStageQueue2[gSpecialStatuses[battler].statStageAmount2].stat = stat;
    gSpecialStatuses[battler].statStageQueue2[gSpecialStatuses[battler].statStageAmount2].stage = stage;
    gSpecialStatuses[battler].statStageAmount2++;
}

void ClearStatChangeValues(void)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        memset(gSpecialStatuses[battler].statStageQueue, 0, sizeof(gSpecialStatuses[battler].statStageQueue));
        gSpecialStatuses[battler].statStageAmount = 0;
    }
    gBattleStruct->negativeAnimPlayed = 0;
    gBattleStruct->positiveAnimPlayed = 0;
    gBattleStruct->statChangeBattler  = 0;
}

void ClearOtherStatChangeValues(enum BattlerId battler)
{
    memset(gSpecialStatuses[battler].statStageQueue2, 0, sizeof(gSpecialStatuses[battler].statStageQueue2));
    gSpecialStatuses[battler].statStageAmount2 = 0;
    gBattleStruct->negativeAnimPlayed = 0;
    gBattleStruct->positiveAnimPlayed = 0;
}

void ClearBothStatChangeQueues(void)
{
    for (enum BattlerId battler = 0; battler < gBattlersCount; battler++)
    {
        memset(gSpecialStatuses[battler].statStageQueue2, 0, sizeof(gSpecialStatuses[battler].statStageQueue2));
        gSpecialStatuses[battler].statStageAmount2 = 0;
        memset(gSpecialStatuses[battler].statStageQueue, 0, sizeof(gSpecialStatuses[battler].statStageQueue));
        gSpecialStatuses[battler].statStageAmount = 0;
    }
    gBattleStruct->negativeAnimPlayed = 0;
    gBattleStruct->positiveAnimPlayed = 0;
    gBattleStruct->statChangeBattler  = 0;
}

bool32 CompareStat(enum BattlerId battler, enum Stat statId, u32 cmpTo, u32 cmpKind, enum Ability ability)
{
    bool32 ret = FALSE;
    u32 statValue = gBattleMons[battler].statStages[statId];

    // Because this command is used as a way of checking if a stat can be lowered/raised,
    // we need to do some modification at run-time.
    if (ability == ABILITY_CONTRARY)
    {
        if (cmpKind == CMP_GREATER_THAN)
            cmpKind = CMP_LESS_THAN;
        else if (cmpKind == CMP_LESS_THAN)
            cmpKind = CMP_GREATER_THAN;

        if (cmpTo == MIN_STAT_STAGE)
            cmpTo = MAX_STAT_STAGE;
        else if (cmpTo == MAX_STAT_STAGE)
            cmpTo = MIN_STAT_STAGE;
    }

    switch (cmpKind)
    {
    case CMP_EQUAL:
        if (statValue == cmpTo)
            ret = TRUE;
        break;
    case CMP_NOT_EQUAL:
        if (statValue != cmpTo)
            ret = TRUE;
        break;
    case CMP_GREATER_THAN:
        if (statValue > cmpTo)
            ret = TRUE;
        break;
    case CMP_LESS_THAN:
        if (statValue < cmpTo)
            ret = TRUE;
        break;
    case CMP_COMMON_BITS:
        if (statValue & cmpTo)
            ret = TRUE;
        break;
    case CMP_NO_COMMON_BITS:
        if (!(statValue & cmpTo))
            ret = TRUE;
        break;
    }

    return ret;
}

static void SetAdditionalEffectsOnStatChange(struct BattleCalcValues *cv, struct StatChange *st)
{
    switch (cv->moveEffect)
    {
    case EFFECT_DEFENSE_CURL:
        gBattleMons[cv->battlerDef].volatiles.defenseCurl = TRUE;
        break;
    case EFFECT_MINIMIZE: // Write a tests that fails on max stat
        if (gBattleStruct->moveResultFlags[cv->battlerDef] & MOVE_RESULT_STAT_CHANGED)
            gBattleMons[cv->battlerDef].volatiles.minimize = TRUE;
        break;
    case EFFECT_NO_RETREAT:
        if (!gBattleMons[cv->battlerDef].volatiles.escapePrevention)
        {
            gBattleMons[cv->battlerDef].volatiles.noRetreat = TRUE;
            gBattleMons[cv->battlerDef].volatiles.escapePrevention = TRUE;
            st->moveScript = BattleScript_NoRetreatMessage;
        }
        break;
    case EFFECT_AUTOTOMIZE:
        if (gBattleStruct->moveResultFlags[cv->battlerDef] & MOVE_RESULT_STAT_CHANGED
         && GetBattlerWeight(cv->battlerDef) > 1)
        {
            gBattleMons[cv->battlerDef].volatiles.autotomizeCount++;
            st->moveScript = BattleScript_AutotomizeMessage;
        }
        break;
    default:
        break;
    }
}

/*
  There might be a better solution without this function
  The problems this fixes:
  1. Multiply failure pop ups
  2. Since we don't mark battlers as doesn't affect foe, they still get a stat drop
*/
static void MarkStatsAsDone(struct StatChange *st, u32 stat)
{
    for (u32 i = 0; i < st->statStageAmount; i++)
    {
        if (st->statStageQueue[i].stat != stat && stat != NUM_BATTLE_STATS)
            continue;

        if (st->statStageQueue[i].stage < 0)
            st->statStageQueue[i].done = TRUE;
    }
}

// The speed boost case should be removed from here but would currently create a regression. Usage now only limited to ai
bool32 CanStatChange(struct BattleCalcValues *cv, struct StatChange *st)
{
    if (st->stage < 0)
    {
        // Special Case for speed boost since shouldn't try to lower opposing stats on speed boost
        // Also for user it might make sense to lower the stat. Regardless this whole check is better suited for CheckViability since the move wouldn't fail in this case
        // FORK: innate-aware — also credit a foe's innate Speed Boost here.
        if (cv->battlerAtk != cv->battlerDef && st->stat == STAT_SPEED && st->stage < 0
         && (cv->abilities[cv->battlerDef] == ABILITY_SPEED_BOOST || IsInnateActive(cv->battlerDef, ABILITY_SPEED_BOOST)))
            return FALSE;

        if (CompareStat(cv->battlerDef, st->stat, MIN_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
            return FALSE;

        if (st->stage < 0 && CanDecreaseStat(cv, st) == STAT_CHANGE_DIDNT_WORK)
            return FALSE;
    }
    else
    {
        if (CompareStat(cv->battlerDef, st->stat, MAX_STAT_STAGE, CMP_EQUAL, ABILITY_NONE))
            return FALSE;
    }

    return TRUE;
}

bool32 IsStatChangeStatusMove(enum Move move, bool32 (*isStatChange)(const struct AdditionalEffect *effect))
{
    u32 numAdditionalEffects = GetMoveAdditionalEffectCount(move);
    for (u32 i = 0; i < numAdditionalEffects; i++)
    {
        const struct AdditionalEffect *effect = GetMoveAdditionalEffectById(move, i);
        if (isStatChange(effect))
            return TRUE;
    }
    return FALSE;
}

bool32 IsAtkStatUpMove(const struct AdditionalEffect *effect)
{
    if (effect->moveEffect != STAT_CHANGE_EFFECT_PLUS)
        return FALSE;

    return effect->attack;
}

bool32 IsAtkSpAtkStatUpMove(const struct AdditionalEffect *effect)
{
    if (effect->moveEffect != STAT_CHANGE_EFFECT_PLUS)
        return FALSE;

    return effect->attack || effect->spAtk;
}

bool32 IsDefSpDefStatUpMove(const struct AdditionalEffect *effect)
{
    if (effect->moveEffect != STAT_CHANGE_EFFECT_PLUS)
        return FALSE;

    return effect->defense || effect->spDef;
}

bool32 IsAccDownEvasionUpStatChangeMove(const struct AdditionalEffect *effect)
{
    switch (effect->moveEffect)
    {
    case STAT_CHANGE_EFFECT_PLUS:
        return effect->evasion;
    case STAT_CHANGE_EFFECT_MINUS:
        return effect->accuracy;
    default:
        break;
    }

    return FALSE;
}
