#ifndef GUARD_BATTLE_UTIL_H
#define GUARD_BATTLE_UTIL_H

#include "move.h"
#include "constants/battle_string_ids.h"
#include "constants/hold_effects.h"
#include "constants/battle_stat_change.h"

#define MOVE_LIMITATION_ZEROMOVE                (1 << 0)
#define MOVE_LIMITATION_PP                      (1 << 1)
#define MOVE_LIMITATION_DISABLED                (1 << 2)
#define MOVE_LIMITATION_TORMENTED               (1 << 3)
#define MOVE_LIMITATION_TAUNT                   (1 << 4)
#define MOVE_LIMITATION_IMPRISON                (1 << 5)
#define MOVE_LIMITATION_ENCORE                  (1 << 6)
#define MOVE_LIMITATION_CHOICE_ITEM             (1 << 7)
#define MOVE_LIMITATION_ASSAULT_VEST            (1 << 8)
#define MOVE_LIMITATION_GRAVITY                 (1 << 9)
#define MOVE_LIMITATION_HEAL_BLOCK              (1 << 10)
#define MOVE_LIMITATION_BELCH                   (1 << 11)
#define MOVE_LIMITATION_THROAT_CHOP             (1 << 12)
#define MOVE_LIMITATION_STUFF_CHEEKS            (1 << 13)
#define MOVE_LIMITATION_CANT_USE_TWICE          (1 << 14)

#define MOVE_LIMITATION_PLACEHOLDER             (1 << 15)
#define MOVE_LIMITATIONS_ALL                    0xFFFF

// Switches between simulated battle calc and actual battle combat
enum ResultOption
{
    CHECK_TRIGGER, // Check the function without running scripts / setting any flags.
    AI_CHECK,  // Check the function without running scripts / setting any flags. Same as CHECK_TRIGGER but only used when additional data has to be fetched during ai calcs
    RUN_SCRIPT,
};

enum FieldEffectCases
{
    FIELD_EFFECT_TRAINER_STATUSES,
    FIELD_EFFECT_OVERWORLD_TERRAIN,
    FIELD_EFFECT_OVERWORLD_WEATHER,
};

enum AbilityEffect
{
    ABILITYEFFECT_ENDTURN,
    ABILITYEFFECT_MOVE_END_ATTACKER,
    ABILITYEFFECT_COLOR_CHANGE, // Color Change / Berserk / Anger Shell
    ABILITYEFFECT_MOVE_END,
    ABILITYEFFECT_IMMUNITY,
    ABILITYEFFECT_FORM_CHANGE_ON_HIT,
    ABILITYEFFECT_DANCER,
    ABILITYEFFECT_MOVE_END_FOES_FAINTED, // Moxie-like abilities / Battle Bond / Magician

    ABILITYEFFECT_ON_FORM_CHANGE,

    // On Switch in
    ABILITYEFFECT_TERA_SHIFT,
    ABILITYEFFECT_NEUTRALIZINGGAS,
    ABILITYEFFECT_UNNERVE,
    ABILITYEFFECT_ON_SWITCHIN,
    ABILITYEFFECT_SWITCH_IN_FORM_CHANGE,
    ABILITYEFFECT_DEPENDS_ON_ALLY, // Commander / Hospitality / Costar
    ABILITYEFFECT_ON_WEATHER,
    ABILITYEFFECT_ON_TERRAIN,
    ABILITYEFFECT_OPPORTUNIST,
};

enum ItemEffect
{
    ITEM_NO_EFFECT,
    ITEM_STATUS_CHANGE,
    ITEM_EFFECT_OTHER,
    ITEM_PP_CHANGE,
    ITEM_HP_CHANGE,
    ITEM_STATS_CHANGE,
};

#define IS_WHOLE_SIDE_ALIVE(battler)    ((IsBattlerAlive(battler) && IsBattlerAlive(GetPartnerBattler(battler))))
#define IS_ALIVE_AND_PRESENT(battler)   (IsBattlerAlive(battler) && IsBattlerSpritePresent(battler))

// Lowest and highest percentages used for damage roll calculations
#define DMG_ROLL_PERCENT_LO 85
#define DMG_ROLL_PERCENT_HI 100

// FORK: DETERMINISTIC_DAMAGE replaces the random 85%-100% damage roll with this
// fixed, turn-scaling percentage. gBattleTurnCounter is 0 on the first turn, so
// this is DETERMINISTIC_DAMAGE_BASE_PERCENT (92% by default) then climbs by
// DETERMINISTIC_DAMAGE_TURN_INCREMENT per elapsed turn. Uncapped on purpose (see
// the flag comment in config/deterministic.h). Shared by the real damage calc
// and the AI's damage prediction so the two agree.
#define DETERMINISTIC_DAMAGE_PERCENT \
    (DETERMINISTIC_DAMAGE_BASE_PERCENT + DETERMINISTIC_DAMAGE_TURN_INCREMENT * gBattleTurnCounter)

// Crit chance exceptions
#define CRITICAL_HIT_BLOCKED -1
#define CRITICAL_HIT_ALWAYS  -2

enum ImmunityHealStatusOutcome
{
    IMMUNITY_NO_EFFECT,
    IMMUNITY_STATUS_CLEARED,
    IMMUNITY_CONFUSION_CLEARED,
    IMMUNITY_INFATUATION_CLEARED,
    IMMUNITY_TAUNT_CLEARED,
};

struct DamageContext
{
    enum BattlerId battlerAtk:3;
    enum BattlerId battlerDef:3;
    u32 fixedBasePower:8;
    u32 weather:16;
    u32 unused:2;
    u32 fieldStatuses;

    enum Move move;
    enum Move chosenMove; // For Trump Card and Me First
    enum Move baseMove; // For z-moves and dynamax-moves
    enum Type moveType;

    uq4_12_t typeEffectivenessModifier;
    enum Ability abilities[MAX_BATTLERS_COUNT];
    enum HoldEffect holdEffects[MAX_BATTLERS_COUNT];

    // Flags
    u32 isCrit:1;
    u32 randomFactor:1;
    u32 updateFlags:1;
    u32 isAnticipation:1;
    u32 isSelfInflicted:1;
    u32 aiCalc:1;
    u32 aiCheckBerryModifier:1; // Flags that KOing through a berry should be checked
    u32 airBalloonBlocked:1;
    u32 abilityBlocked:1;
    u32 runScript:1;  // Used during actual combat where scripts have to be run / flags need to be set
    u32 innatesEnabled:1; // FORK: cached GetConfig(FEATURE_INNATE_ABILITIES), set once in DoMoveDamageCalcVars
    u32 haloOnField:1; // FORK: cached "a Halo holder is on the field", set once in DoMoveDamageCalcVars (see include/fork/halo.h)
    u32 padding:20;
};

// Helper struct to keep the arg list small and prevent constant recalculations of abilities/hold effects.
struct BattleCalcValues
{
    enum BattlerId battlerAtk:3;
    enum BattlerId battlerDef:3;
    enum Move move:16;
    enum BattleMoveEffects moveEffect:10;
    enum Ability abilities[MAX_BATTLERS_COUNT];
    enum HoldEffect holdEffects[MAX_BATTLERS_COUNT];
};

enum SleepClauseBlock
{
    NOT_BLOCKED_BY_SLEEP_CLAUSE,
    BLOCKED_BY_SLEEP_CLAUSE,
};

enum EjectPackTiming
{
    START_OF_TURN,
    END_TURN,
    OTHER,
};

enum SubCheck
{
    EXCLUDING_SUBSTITUTES,
    INCLUDING_SUBSTITUTES
};

void HandleAction_ThrowBall(void);
u32 GetCurrentBattleWeather(void);
bool32 EndOrContinueWeather(void);
enum DamageCategory GetReflectDamageMoveDamageCategory(enum BattlerId battler, enum Move move);
bool32 ShouldTeraShellDistortTypeMatchups(struct DamageContext *ctx);
bool32 IsUnnerveBlocked(enum BattlerId battler, enum Item itemId);
bool32 IsAffectedByFollowMe(enum BattlerId battlerAtk, enum BattleSide defSide, enum Move move);
void HandleAction_UseMove(void);
void HandleAction_Switch(void);
void HandleAction_UseItem(void);
bool32 TryRunFromBattle(enum BattlerId battler);
void HandleAction_Run(void);
void HandleAction_WatchesCarefully(void);
void HandleAction_SafariZoneBallThrow(void);
void HandleAction_ThrowPokeblock(void);
void HandleAction_GoNear(void);
void HandleAction_SafariZoneRun(void);
void HandleAction_WallyBallThrow(void);
void HandleAction_TryFinish(void);
void HandleAction_NothingIsFainted(void);
void HandleAction_ActionFinished(void);
enum BattlerId GetBattlerForBattleScript(u8 caseId);
bool32 IsBattlerMarkedForControllerExec(enum BattlerId battler);
void MarkBattlerForControllerExec(enum BattlerId battler);
void MarkBattlerReceivedLinkData(enum BattlerId battler);
void CancelMultiTurnMoves(enum BattlerId battler);
bool32 IsLastMonToMove(enum BattlerId battler);
void PrepareStringBattle(enum StringID stringId, enum BattlerId battler);
void ResetSentPokesToOpponentValue(void);
void OpponentSwitchInResetSentPokesToOpponentValue(enum BattlerId battler);
void UpdateSentPokesToOpponentValue(enum BattlerId battler);
void BattleScriptPush(const u8 *bsPtr);
void BattleScriptPushCursor(void);
void BattleScriptCall(const u8 *bsPtr);
void BattleScriptPop(void);
u32 TrySetCantSelectMoveBattleScript(enum BattlerId battler);
u32 CheckMoveLimitations(enum BattlerId battler, u8 unusableMoves, u16 check);
bool32 AreAllMovesUnusable(enum BattlerId battler);
u8 GetImprisonedMovesCount(enum BattlerId battler, enum Move move);
s32 GetDrainedBigRootHp(enum BattlerId battler, s32 hp);
// FORK: BUFF_LEECH_SEED. Which branch a Leech Seed drain takes, returned by SetUpLeechSeedDrain.
enum LeechSeedDrainKind
{
    LEECH_SEED_DRAIN_RECOVERY,    // victim loses HP, seeder heals
    LEECH_SEED_DRAIN_LIQUID_OOZE, // victim loses HP, seeder takes recoil (victim has Liquid Ooze)
    LEECH_SEED_DRAIN_HEAL_BLOCK,  // victim loses HP, seeder heals nothing (seeder under Heal Block)
};
enum LeechSeedDrainKind SetUpLeechSeedDrain(enum BattlerId victim, enum BattlerId seeder);
bool32 IsAbilityAndRecord(enum BattlerId battler, enum Ability battlerAbility, enum Ability abilityToCheck);
// FORK: FEATURE_INNATE_ABILITIES. Innate-aware drop-in for IsAbilityAndRecord: TRUE if the chosen
// ability matches (recorded, exactly as upstream) OR an active innate matches (NOT recorded — the
// chosen slot stays the mon's identity). Used at chip-damage / indirect-damage gates so an innate
// holder is spared like the real ability (e.g. Magic Guard's many end-turn/hazard/recoil sites).
bool32 IsAbilityOrInnateAndRecord(enum BattlerId battler, enum Ability battlerAbility, enum Ability abilityToCheck);
bool32 HandleFaintedMonActions(void);
bool32 HasNoMonsToSwitch(enum BattlerId battler, u8 partyIdBattlerOn1, u8 partyIdBattlerOn2);
bool32 TryChangeBattleWeather(enum BattlerId battler, u32 battleWeatherId, enum Ability ability);
bool32 TryChangeBattleTerrain(enum BattlerId battler, u32 statusFlag);
bool32 IsPowderMoveBlocked(struct DamageContext *ctx);
bool32 CanTargetBlockPranksterMove(struct DamageContext *ctx, s32 movePriority);
bool32 CanPsychicTerrainProtectTarget(struct DamageContext *ctx, s32 movePriority);
bool32 CanMoveBeBlockedByTarget(struct DamageContext *ctx, s32 movePriority);
bool32 CanAbilityAbsorbMove(struct DamageContext *ctx);
bool32 TryFieldEffects(enum FieldEffectCases caseId);
u32 AbilityBattleEffects(enum AbilityEffect caseID, enum BattlerId battler, enum Ability ability, enum Move move, bool32 shouldAbilityTrigger);
bool32 TryPrimalReversion(enum BattlerId battler);
bool32 IsNeutralizingGasOnField(void);
bool32 IsMoldBreakerTypeAbility(enum BattlerId battler, enum Ability ability);
enum Ability GetBattlerAbilityIgnoreMoldBreaker(enum BattlerId battler);
enum Ability GetBattlerAbilityNoAbilityShield(enum BattlerId battler);
enum Ability GetBattlerAbilityInternal(enum BattlerId battler, bool32 ignoreMoldBreaker, bool32 noAbilityShield);
enum Ability GetBattlerAbility(enum BattlerId battler);
// FORK: FEATURE_INNATE_ABILITIES. "Does this battler have ability X?" trait
// predicate: TRUE for the primary (chosen) ability or an active innate. Use this
// for trait checks; keep GetBattlerAbility() for identity/copy/swap/display.
bool32 BattlerHasAbility(enum BattlerId battler, enum Ability ability);
// FORK: FEATURE_INNATE_ABILITIES. TRUE if `battler`'s species declares `ability` as an
// innate AND it is currently active (same suppression gates as the chosen slot). Unlike
// BattlerHasAbility(), this does NOT also match the chosen ability — use it at innate-only
// effect sites that must not credit (or leak) the chosen slot, e.g. GetBattleMovePriority's
// innate Prankster check. No-op (FALSE) when the feature flag is off. See src/battle_util.c.
bool32 IsInnateActive(enum BattlerId battler, enum Ability ability);
u32 IsAbilityOnSide(enum BattlerId battler, enum Ability ability);
u32 IsAbilityOnOpposingSide(enum BattlerId battler, enum Ability ability);
u32 IsInnateOnSide(enum BattlerId battler, enum Ability ability); // FORK: FEATURE_INNATE_ABILITIES
u32 IsAbilityOnField(enum Ability ability);
u32 IsAbilityOnFieldExcept(enum BattlerId battler, enum Ability ability);
u32 IsAbilityPreventingEscape(enum BattlerId battler);
enum Ability GetBattlerEscapePreventionAbility(enum BattlerId battler, enum BattlerId trapper); // FORK: FEATURE_INNATE_ABILITIES — the trapping ability (chosen or innate) shown in the escape/switch message
bool32 IsBattlerProtected(struct BattleCalcValues *cv);
enum ProtectType GetProtectType(enum ProtectMethod method);
bool32 CanBattlerEscape(enum BattlerId battler); // no ability check
void BattleScriptExecute(const u8 *BS_ptr);
void BattleScriptPushCursorAndCallback(const u8 *BS_ptr);
void ClearVariousBattlerFlags(enum BattlerId battler);
void HandleAction_RunBattleScript(void);
u32 SetRandomTarget(enum BattlerId battlerAtk);
u32 GetBattleMoveTarget(enum Move move, enum MoveTarget moveTarget);
enum Obedience GetAttackerObedienceForAction(void);
enum HoldEffect GetBattlerHoldEffect(enum BattlerId battler);
enum HoldEffect GetBattlerHoldEffectIgnoreAbility(enum BattlerId battler);
enum HoldEffect GetBattlerHoldEffectIgnoreNegation(enum BattlerId battler);
enum HoldEffect GetBattlerHoldEffectInternal(enum BattlerId battler, enum Ability ability);
u32 GetBattlerHoldEffectParam(enum BattlerId battler);
bool32 CanBattlerAvoidContactEffects(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum HoldEffect holdEffectAtk, enum Move move);
bool32 IsMoveMakingContact(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum HoldEffect holdEffectAtk, enum Move move);
bool32 IsBattlerGrounded(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect);
bool32 IsBattlerGroundedForBenefit(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect); // FORK: grounded, or floating only by an innate Levitate (terrain / Toxic Spikes boon)
u32 GetMoveSlot(u16 *moves, enum Move move);
u32 GetBattlerWeight(enum BattlerId battler);
s32 CalcCritChanceStage(struct DamageContext *ctx);
s32 CalcCritChanceStageGen1(struct DamageContext *ctx);
bool32 IsGuaranteedCriticalHit(s32 critChance); // FORK: shared "is a crit certain" predicate
s32 CalculateMoveDamage(struct DamageContext *ctx);
s32 CalculateMoveDamageVars(struct DamageContext *ctx);
s32 DoFixedDamageMoveCalc(struct DamageContext *ctx);
s32 ApplyModifiersAfterDmgRoll(struct DamageContext *ctx, s32 dmg);
uq4_12_t CalcTypeEffectivenessMultiplier(struct DamageContext *ctx);
uq4_12_t CalcPartyMonTypeEffectivenessMultiplier(enum Move move, enum Species speciesDef, enum Ability abilityDef);
uq4_12_t GetTypeModifier(enum Type atkType, enum Type defType);
uq4_12_t GetOverworldTypeEffectiveness(struct Pokemon *mon, enum Type moveType);
void UpdateMoveResultFlags(uq4_12_t modifier, u32 *resultFlags);
s32 GetStealthHazardDamage(enum TypeSideHazard hazardType, enum BattlerId battler);
s32 GetStealthHazardDamageByTypesAndHP(enum TypeSideHazard hazardType, enum Type type1, enum Type type2, u32 maxHp);
bool32 CanMegaEvolve(enum BattlerId battler);
bool32 CanUltraBurst(enum BattlerId battler);
void ActivateMegaEvolution(enum BattlerId battler);
void ActivateUltraBurst(enum BattlerId battler);
bool32 IsBattlerMegaEvolved(enum BattlerId battler);
bool32 IsBattlerPrimalReverted(enum BattlerId battler);
bool32 IsBattlerUltraBursted(enum BattlerId battler);
enum Species GetBattleFormChangeTargetSpecies(enum BattlerId battler, enum FormChanges method, enum Ability ability);
bool32 TryRevertPartyMonFormChange(u32 partyIndex);
bool32 TryBattleFormChange(enum BattlerId battler, enum FormChanges method, enum Ability ability);
bool32 DoBattlersShareType(enum BattlerId battler1, enum BattlerId battler2);
bool32 CanBattlerGetOrLoseItem(enum BattlerId fromBattler, enum BattlerId battler, enum Item itemId);
enum Species GetBattlerVisualSpecies(enum BattlerId battler);
bool32 TryClearIllusion(enum BattlerId battler, enum Ability ability);
enum Species GetIllusionMonSpecies(enum BattlerId battler);
struct Pokemon *GetIllusionMonPtr(enum BattlerId battler);
void ClearIllusionMon(enum BattlerId battler);
u32 GetIllusionMonPartyId(struct Pokemon *party, struct Pokemon *mon, struct Pokemon *partnerMon, enum BattlerId battler);
void SetIllusionMon(struct Pokemon *mon, enum BattlerId battler);
enum ImmunityHealStatusOutcome TryImmunityAbilityHealStatus(enum BattlerId battler);
bool32 ShouldGetStatBadgeBoost(u16 flagId, enum BattlerId battler);
uq4_12_t GetBadgeBoostModifier(void);
enum DamageCategory GetBattleMoveCategory(enum Move move);
void SetDynamicMoveCategory(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
bool32 CanFling(enum BattlerId battlerAtk, enum Ability abilityAtk);
bool32 IsTelekinesisBannedSpecies(enum Species species);
bool32 IsHealBlockPreventingMove(enum BattlerId battler, enum Move move);
bool32 IsGravityPreventingMove(enum Move move);
bool32 IsBelchPreventingMove(enum BattlerId battler, enum Move move);
bool32 HasEnoughHpToEatBerry(enum BattlerId battler, enum Ability ability, u32 hpFraction, enum Item itemId);
bool32 IsPartnerMonFromSameTrainer(enum BattlerId battler);
enum DamageCategory GetCategoryBasedOnStats(enum BattlerId battler);
void SetShellSideArmCategory(void);
bool32 MoveIsAffectedBySheerForce(enum Move move);
bool32 IsSheerForceAffected(enum Move move, enum Ability ability);
void TryRestoreHeldItems(void);
bool32 CanStealItem(enum BattlerId battlerStealing, enum BattlerId battlerItem, enum Item item);
void TrySaveExchangedItem(enum BattlerId battler, enum Item stolenItem);
bool32 IsBattlerAffectedByHazards(enum BattlerId battler, enum HoldEffect holdEffect, bool32 toxicSpikes);
void SortBattlersByRawSpeed(u8 battlers[]);
void SortBattlersBySpeed(enum BattlerId *battlers, bool32 slowToFast);
bool32 BlocksPrankster(enum Move move, enum BattlerId battlerPrankster, enum BattlerId battlerDef, bool32 checkTarget);
bool32 PickupHasValidTarget(enum BattlerId battler);
bool32 CantPickupItem(u32 battler);
u32 GetWeather(void);
u32 GetAttackerWeather(enum BattlerId battler, enum HoldEffect holdEffect, enum Ability ability, u32 weather);
bool32 IsBattlerWeatherAffected(enum HoldEffect holdEffect, u32 weather, u32 weatherFlags);
enum MoveTarget GetBattlerMoveTargetType(enum BattlerId battler, enum Move move);
bool32 CanTargetBattler(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move);
u32 GetNextTarget(u32 moveTarget, bool32 excludeCurrent);
void CopyMonLevelAndBaseStatsToBattleMon(enum BattlerId battler, struct Pokemon *mon);
void CopyMonAbilityAndTypesToBattleMon(enum BattlerId battler, struct Pokemon *mon);
void RecalcBattlerStats(enum BattlerId battler, struct Pokemon *mon, bool32 isDynamaxing);
bool32 IsGen6ExpShareEnabled(void);
bool32 MoveHasAdditionalEffect(enum Move move, enum MoveEffect moveEffect);
bool32 MoveHasAdditionalOnSideEffect(enum Move move);
bool32 MoveHasAdditionalEffectWithChance(enum Move move, enum MoveEffect moveEffect, u32 chance);
bool32 MoveHasAdditionalEffectSelf(enum Move move, enum MoveEffect moveEffect);
bool32 IsMoveEffectRemoveSpeciesType(enum Move move, enum MoveEffect moveEffect, u32 argument);
bool32 MoveHasChargeTurnAdditionalEffect(enum Move move);
bool32 CanTargetPartner(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 IsBattlerUnaffectedByMove(enum BattlerId battler);
bool32 MoodyCantRaiseStat(u32 stat);
bool32 MoodyCantLowerStat(u32 stat);
bool32 IsPsychicTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses);
bool32 IsMistyTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses);
bool32 IsGrassyTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses);
bool32 IsElectricTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses);
bool32 IsAnyTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses);
bool32 IsBattlerTerrainAffected(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect, u32 fieldStatuses, u32 terrainFlag);
enum Stat GetHighestStatId(enum BattlerId battler);
enum Stat GetParadoxHighestStatId(enum BattlerId battler);
u32 GetStatValueWithStages(enum BattlerId battler, enum Stat stat); // FORK: DETERMINISTIC_HOLD_EFFECTS (Starf Berry highest-stat pick)
enum Stat GetParadoxBoostedStatId(enum BattlerId battler);

bool32 CanBeSlept(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef, enum SleepClauseBlock isBlockedBySleepClause);
bool32 CanBePoisoned(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef);
bool32 CanBeBurned(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability ability);
bool32 CanBeParalyzed(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 CanBeFrozen(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 CanGetFrostbite(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityDef);
bool32 CanSetNonVolatileStatus(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum MoveEffect secondaryMoveEffect, enum ResultOption option);
bool32 CanBeConfused(enum BattlerId battlerAtk, enum BattlerId effectBattler);
bool32 IsSafeguardProtected(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk);
u32 GetBattlerAffectionHearts(enum BattlerId battler);
void TryToRevertMimicryAndFlags(void);
u32 CountBattlerStatIncreases(enum BattlerId battler, bool32 countEvasionAcc);
bool32 BattlerHasCopyableChanges(enum BattlerId battler);
bool32 ChangeTypeBasedOnTerrain(enum BattlerId battler);
void RemoveConfusionStatus(enum BattlerId battler);
u32 GetBattlerGender(enum BattlerId battler);
bool32 AreBattlersOfOppositeGender(enum BattlerId battler1, enum BattlerId battler2);
bool32 AreBattlersOfSameGender(enum BattlerId battler1, enum BattlerId battler2);
u32 CalcSecondaryEffectChance(enum BattlerId battler, enum Ability battlerAbility, const struct AdditionalEffect *additionalEffect);
bool32 MoveEffectIsGuaranteed(enum BattlerId battler, enum Ability battlerAbility, const struct AdditionalEffect *additionalEffect);
void GetBattlerTypes(enum BattlerId battler, bool32 ignoreTera, enum Type types[static 3]);
enum Type GetBattlerType(enum BattlerId battler, u32 typeIndex, bool32 ignoreTera);
bool32 CanMonParticipateInSkyBattle(struct Pokemon *mon);
void RemoveBattlerType(enum BattlerId battler, enum Type type);
enum Type GetBattleMoveType(enum Move move);
void TryActivateSleepClause(enum BattlerId battler, u32 indexInParty);
void TryDeactivateSleepClause(enum BattlerId battler, u32 indexInParty);
bool32 IsSleepClauseActiveForSide(enum BattleSide battlerSide);
bool32 IsSleepClauseEnabled(void);
bool32 AreMultiPartiesFullTeams(void);
void ClearDamageCalcResults(void);
u32 DoesDestinyBondFail(enum BattlerId battler);
bool32 IsMoveEffectBlockedByTarget(enum Ability ability);
bool32 SetTargetToNextPursuiter(enum BattlerId battlerDef);
bool32 IsPursuitTargetSet(void);
void ClearPursuitValuesIfSet(enum BattlerId battler);
void ClearPursuitValues(void);
bool32 HasWeatherEffect(void);
bool32 HadMoreThanHalfHpNowDoesnt(enum BattlerId battler);
void ChooseStatBoostAnimation(enum BattlerId battler);
bool32 TrySwitchInEjectPack(enum EjectPackTiming timing);
bool32 EmergencyExitCanBeTriggered(enum BattlerId battler, enum Ability ability);
bool32 TryTriggerSymbiosis(enum BattlerId battler, u32 ally);
bool32 TrySymbiosis(enum BattlerId battler, enum Item itemId, const u8 *nextInstr);
void BestowItem(enum BattlerId battlerAtk, enum BattlerId battlerDef);
ARM_FUNC u32 GetBattlerVolatile(enum BattlerId battler, enum Volatile _volatile);
void SetMonVolatile(enum BattlerId battler, enum Volatile _volatile, u32 newValue);
bool32 ItemHealMonVolatile(enum BattlerId battler, enum Item itemId);
void PushHazardTypeToQueue(enum BattleSide side, enum Hazards hazardType);
bool32 IsHazardOnSide(enum BattleSide side, enum Hazards hazardType);
bool32 AreAnyHazardsOnSide(enum BattleSide side);
void RemoveAllHazardsFromField(enum BattleSide side);
bool32 IsHazardOnSideAndClear(enum BattleSide side, enum Hazards hazardType);
void RemoveHazardFromField(enum BattleSide side, enum Hazards hazardType);
bool32 CanMoveSkipAccuracyCalc(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum Move move, enum ResultOption option);
u32 GetTotalAccuracy(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability atkAbility, enum Ability defAbility, enum HoldEffect atkHoldEffect, enum HoldEffect defHoldEffect);
s32 GetAccEvasionStageDelta(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability atkAbility, enum Ability defAbility, bool32 ignorePenalties); // FORK: DETERMINISTIC_ACCURACY_EVASION PP economy
u32 GetDeterministicMoveTargetPPTax(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability defAbility, enum HoldEffect defHoldEffect); // FORK: DETERMINISTIC_ACCURACY_EVASION PP economy
s32 GetProjectedMovePPCost(enum BattlerId battlerAtk, enum Move move); // FORK: DETERMINISTIC_ACCURACY_EVASION move-info PP cost
bool32 DoesOHKOMoveMissTarget(struct BattleCalcValues *cv);
bool32 DoesMoveMissTarget(struct BattleCalcValues *cv);
bool32 IsSemiInvulnerable(enum BattlerId battler, enum SemiInvulnerableExclusion excludeCommander);
bool32 CanBreakThroughSemiInvulnerablity(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum Move move);
bool32 BreaksThroughSemiInvulnerableState(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Ability abilityAtk, enum Ability abilityDef, enum Move move, enum SemiInvulnerableState state);
bool32 IsBattlerOnAir(enum BattlerId battler);
bool32 HasPartnerTrainer(enum BattlerId battler);
bool32 IsAffectedByPowderMove(enum BattlerId battler, enum Ability ability, enum HoldEffect holdEffect);
enum Move GetNaturePowerMove(void);
void RemoveAbilityFlags(enum BattlerId battler);
void RemoveRuinAbilityFlags(enum BattlerId battler);
void CheckSetUnburden(enum BattlerId battler);
bool32 IsDazzlingAbility(enum Ability ability);
enum Ability GetBattlerDazzlingAbility(enum BattlerId battler, enum Ability chosenAbility); // FORK: innate-aware (FEATURE_INNATE_ABILITIES)
bool32 IsAllowedToUseBag(void);
bool32 IsAnyTargetTurnDamaged(enum BattlerId battlerAtk, enum SubCheck subCheck);
bool32 IsAnyTargetAffected(void);
bool32 IsMimikyuDisguised(enum BattlerId battler);
bool32 IsDoubleSpreadMove(void);
bool32 IsBattlerInvalidForSpreadMove(enum BattlerId battlerAtk, enum BattlerId battlerDef);
void SetStartingStatus(enum StartingStatus status);
void ResetStartingStatuses(void);
bool32 IsUsableWhileAsleepEffect(enum BattleMoveEffects effect);
void SetWrapTurns(enum BattlerId battler, enum HoldEffect holdEffect);
bool32 ChangeOrderTargetAfterAttacker(void);
void TryUpdateEvolutionTracker(enum EvolutionConditions evolutionCondition, u32 upAmount, enum Move usedMove);
bool32 CanUseMoveConsecutively(enum BattlerId battler);
void TryResetConsecutiveUseCounter(enum BattlerId battler);
void SetOrClearRageVolatile(void);
enum BattlerId GetTargetBySlot(enum BattlerId battlerAtk, enum BattlerId battlerDef);
enum BattlerId GetTargetFromSlotId(enum BattlerId battlerAtk, enum BattlerId battlerDef);
bool32 IsNaturalEnemy(enum Species speciesAttacker, enum Species speciesTarget);
enum Stat GetDownloadStat(enum BattlerId battler);
bool32 BattlerJustSwitchedIn(enum BattlerId battler);
bool32 IsBattlersFirstTurn(enum BattlerId battler);
bool32 IsBattlersEntryTurn(enum BattlerId battler); // FORK: DETERMINISTIC_HOLD_EFFECTS — true only on a battler's field-entry turn (lead or switch-in)
struct PartyState *GetBattlerPartyState(enum BattlerId battler);
void SetValuesOnFaint(enum BattlerId battler);

#endif // GUARD_BATTLE_UTIL_H
