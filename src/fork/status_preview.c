// B_MOVE_STATUS_PREVIEW -- the move menu's "<Type> Fails" readout for status moves.
// See include/fork/status_preview.h and fork-docs/DAMAGE_PREVIEW.md ("Status moves").
//
// The damage preview's sister readout. A status move has no range to show, but the player can
// often tell it will not work: Thunder Wave into a Ground type, Toxic into a Steel type, Spore
// into an innate Insomnia, Sleep Powder into a Safety Goggles the foe has already shown. This
// file answers "is it certain to fail?" by asking the engine's own checks (the same ones the
// move runs when it is used), with the foe rewritten into what the player knows: its unrevealed
// ability and held item blanked, and a foe under Illusion read as its disguise. Everything is put
// back before returning -- like the damage preview, it runs on every cursor move.
//
// Only "certain to fail" is reported. Anything that hinges on what the player cannot see (a
// hidden item, a hidden ability, turn order, accuracy) leaves the stock line in place.

#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"
#include "battle_gimmick.h"
#include "battle_message.h"
#include "battle_script_commands.h"
#include "battle_util.h"
#include "move.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "fork/buff_status_reapply.h"
#include "fork/damage_preview.h"
#include "fork/innate_abilities.h"
#include "fork/status_preview.h"
#include "constants/battle.h"
#include "constants/characters.h"

// The battle state the knowledge view rewrites. EWRAM, not the stack: the move menu's call depth
// is shallow, but the battler structs are large and the damage preview's convention is the same.
static EWRAM_DATA enum Ability sSavedAbility[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA enum Item sSavedItem[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA enum Species sSavedDefSpecies = 0;
static EWRAM_DATA enum Type sSavedDefTypes[3] = {0};
static EWRAM_DATA enum Ability sSavedAiAbilityAtk = 0;
static EWRAM_DATA enum BattlerId sSavedAttacker = 0;
static EWRAM_DATA bool8 sSavedMoldBreaker = 0;
static EWRAM_DATA enum Type sSavedDynamicMoveType = 0;

// Moves aimed at a foe. Moves on the user, its ally, its own side or the whole field never "fail
// on the foe" in a way this readout could name.
static bool32 IsFoeTargetingMove(enum BattlerId battlerAtk, enum Move move)
{
    switch (GetBattlerMoveTargetType(battlerAtk, move))
    {
    case TARGET_SELECTED:
    case TARGET_SMART:
    case TARGET_OPPONENT:
    case TARGET_RANDOM:
    case TARGET_BOTH:
    case TARGET_FOES_AND_ALLY:
        return TRUE;
    default:
        return FALSE;
    }
}

// Mold Breaker, Teravolt, Turboblaze (chosen or innate) and Mycelium Might on a status move, the
// way IsMoldBreakerTypeAbility decides it at move start -- minus its RecordAbilityBattle, which
// would hand the AI the player's ability for merely opening the menu.
static bool32 IsPreviewMoldBreakerActive(enum BattlerId battlerAtk, enum Ability abilityAtk, enum Move move)
{
    if (MoveIgnoresTargetAbility(move))
        return TRUE;
    if (gBattleMons[battlerAtk].volatiles.gastroAcid)
        return FALSE;
    if (abilityAtk == ABILITY_MOLD_BREAKER || abilityAtk == ABILITY_TERAVOLT || abilityAtk == ABILITY_TURBOBLAZE
     || (abilityAtk == ABILITY_MYCELIUM_MIGHT && IsBattleMoveStatus(move)))
        return TRUE;
    return IsInnateActive(battlerAtk, ABILITY_MOLD_BREAKER)
        || IsInnateActive(battlerAtk, ABILITY_TERAVOLT)
        || IsInnateActive(battlerAtk, ABILITY_TURBOBLAZE);
}

// Rewrites the battle into what the player knows: the foe side's unrevealed abilities and items
// blanked (both foes, since Sweet Veil / Flower Veil / Pastel Veil guard the whole side), and a
// foe under Illusion turned into its disguise. Innates are species data, so they follow the
// disguise and always apply -- the player can look them up.
static void BeginStatusPreviewView(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Species illusion = GetIllusionMonSpecies(battlerDef);

    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        sSavedAbility[b] = gBattleMons[b].ability;
        sSavedItem[b] = gBattleMons[b].item;
    }
    sSavedDefSpecies = gBattleMons[battlerDef].species;
    sSavedDefTypes[0] = gBattleMons[battlerDef].types[0];
    sSavedDefTypes[1] = gBattleMons[battlerDef].types[1];
    sSavedDefTypes[2] = gBattleMons[battlerDef].types[2];
    sSavedAiAbilityAtk = gAiLogicData->abilities[battlerAtk];
    sSavedAttacker = gBattlerAttacker;
    sSavedMoldBreaker = gBattleStruct->moldBreakerActive;
    sSavedDynamicMoveType = gBattleStruct->dynamicMoveType;

    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        if (IsOnPlayerSide(b))
            continue;
        if (!IsPreviewFoeAbilityKnown(b))
            gBattleMons[b].ability = ABILITY_NONE;
        if (!IsPreviewFoeItemKnown(b))
            gBattleMons[b].item = ITEM_NONE;
    }
    if (illusion != SPECIES_NONE)
    {
        gBattleMons[battlerDef].species = illusion;
        gBattleMons[battlerDef].types[0] = GetSpeciesType(illusion, 0);
        gBattleMons[battlerDef].types[1] = GetSpeciesType(illusion, 1);
        gBattleMons[battlerDef].types[2] = TYPE_MYSTERY;
    }

    // As the move would run: the player's mon is the attacker, its Mold Breaker active. The
    // buff re-apply checks read the attacker's ability from the AI's knowledge model, which may
    // not have seen it yet; the player always knows their own.
    gBattlerAttacker = battlerAtk;
    gBattleStruct->moldBreakerActive = IsPreviewMoldBreakerActive(battlerAtk, GetBattlerAbility(battlerAtk), move);
    gAiLogicData->abilities[battlerAtk] = GetBattlerAbility(battlerAtk);
}

static void EndStatusPreviewView(enum BattlerId battlerAtk, enum BattlerId battlerDef)
{
    for (enum BattlerId b = 0; b < gBattlersCount; b++)
    {
        gBattleMons[b].ability = sSavedAbility[b];
        gBattleMons[b].item = sSavedItem[b];
    }
    gBattleMons[battlerDef].species = sSavedDefSpecies;
    gBattleMons[battlerDef].types[0] = sSavedDefTypes[0];
    gBattleMons[battlerDef].types[1] = sSavedDefTypes[1];
    gBattleMons[battlerDef].types[2] = sSavedDefTypes[2];
    gAiLogicData->abilities[battlerAtk] = sSavedAiAbilityAtk;
    gBattlerAttacker = sSavedAttacker;
    gBattleStruct->moldBreakerActive = sSavedMoldBreaker;
    gBattleStruct->dynamicMoveType = sSavedDynamicMoveType;
}

// Checks every foe-aimed status move goes through before its effect: the type chart (only
// Thunder Wave reads it), an absorbing or blocking ability (Volt Absorb, Sap Sipper, Soundproof,
// Good as Gold...), a powder immunity, a reflecting Magic Bounce, and priority blocks (a chosen
// Prankster into a Dark type, Dazzling and co., Psychic Terrain).
static bool32 IsBlockedBeforeEffect(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability abilityAtk, enum Ability abilityDef)
{
    struct DamageContext ctx = {0};
    s32 priority = GetMovePriority(move);

    ctx.battlerAtk = battlerAtk;
    ctx.battlerDef = battlerDef;
    ctx.move = ctx.chosenMove = ctx.baseMove = move;
    ctx.moveType = GetDamagePreviewMoveType(battlerAtk, move, GIMMICK_NONE);
    ctx.updateFlags = FALSE;
    ctx.runScript = FALSE;
    ctx.weather = gBattleWeather;
    ctx.terrain = gFieldTimers.terrain;
    ctx.abilities[battlerAtk] = abilityAtk;
    ctx.abilities[battlerDef] = abilityDef;
    ctx.holdEffects[battlerAtk] = GetBattlerHoldEffect(battlerAtk);
    ctx.holdEffects[battlerDef] = GetBattlerHoldEffect(battlerDef);

    if (CalcTypeEffectivenessMultiplier(&ctx) == UQ_4_12(0.0))
        return TRUE;
    if (CanAbilityAbsorbMove(&ctx) || IsPowderMoveBlocked(&ctx))
        return TRUE;
    if (MoveCanBeBouncedBack(move) && (abilityDef == ABILITY_MAGIC_BOUNCE || IsInnateActive(battlerDef, ABILITY_MAGIC_BOUNCE)))
        return TRUE;

    // GetBattleMovePriority is not asked: it records the attacker's ability for the AI and sets
    // the Prankster latch. Only a chosen Prankster makes the move fail on a Dark type (an innate
    // one is a pure boon, see GetBattleMovePriority); either kind lifts its priority.
    if (abilityAtk == ABILITY_PRANKSTER || IsInnateActive(battlerAtk, ABILITY_PRANKSTER))
        priority++;
    if (abilityAtk == ABILITY_PRANKSTER
     && GetConfig(B_PRANKSTER_DARK_TYPES) >= GEN_7
     && GetBattlerMoveTargetType(battlerAtk, move) != TARGET_DEPENDS
     && IS_BATTLER_OF_TYPE(battlerDef, TYPE_DARK)
     && !IsSemiInvulnerable(battlerDef, CHECK_ALL))
        return TRUE;
    if (priority > 0)
    {
        if (GetBattlerDazzlingAbility(battlerDef, abilityDef) != ABILITY_NONE)
            return TRUE;
        if (IsDoubleBattle() && IsBattlerAlive(GetPartnerBattler(battlerDef))
         && GetBattlerDazzlingAbility(GetPartnerBattler(battlerDef), GetBattlerAbility(GetPartnerBattler(battlerDef))) != ABILITY_NONE)
            return TRUE;
        if (IsPsychicTerrainAffected(battlerDef, abilityDef, ctx.holdEffects[battlerDef], gFieldTimers.terrain))
            return TRUE;
    }
    return FALSE;
}

// The move's own effect, mirroring the command that applies it.
static bool32 IsEffectBlocked(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, enum Ability abilityAtk, enum Ability abilityDef)
{
    enum MoveEffect status = GetMoveNonVolatileStatus(move);

    switch (GetMoveEffect(move))
    {
    case EFFECT_NON_VOLATILE_STATUS:
    case EFFECT_YAWN:
        if (status == MOVE_EFFECT_NONE)
            return FALSE;
        if (GetMoveEffect(move) == EFFECT_YAWN && gBattleMons[battlerDef].volatiles.yawn)
            return TRUE;
        // Cmd_trynonvolatilestatus: the fork's re-apply buffs turn "already burned / poisoned /
        // paralyzed" into a tick or a PP drain, so those are not failures.
        if (AI_CanReburn(battlerAtk, battlerDef, abilityDef, move)
         || AI_CanRepoison(battlerAtk, battlerDef, abilityDef, move)
         || AI_CanReparalyze(battlerAtk, battlerDef, abilityDef, move))
            return FALSE;
        if (!CanSetNonVolatileStatus(battlerAtk, battlerDef, abilityAtk, abilityDef, status, CHECK_TRIGGER))
            return TRUE;
        return DoesSubstituteBlockMove(battlerAtk, battlerDef, move);
    case EFFECT_CONFUSE:
        return !CanBeConfused(battlerAtk, battlerDef) || DoesSubstituteBlockMove(battlerAtk, battlerDef, move);
    case EFFECT_LEECH_SEED:
        // Cmd_setseeded. Under BUFF_LEECH_SEED re-seeding drains instead of failing.
        if (IS_BATTLER_OF_TYPE(battlerDef, TYPE_GRASS))
            return TRUE;
        if (!GetConfig(BUFF_LEECH_SEED) && gBattleMons[battlerDef].volatiles.leechSeed)
            return TRUE;
        return DoesSubstituteBlockMove(battlerAtk, battlerDef, move);
    default:
        return FALSE;
    }
}

bool32 IsStatusMoveKnownToFail(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move)
{
    enum Ability abilityAtk, abilityDef;
    bool32 fails;

    if (move == MOVE_NONE || !IsBattleMoveStatus(move) || !IsBattlerAlive(battlerDef)
     || IsBattlerAlly(battlerAtk, battlerDef) || !IsFoeTargetingMove(battlerAtk, move))
        return FALSE;
    // A Dynamaxed mon's status moves are Max Guard.
    if (GetActiveGimmick(battlerAtk) == GIMMICK_DYNAMAX)
        return FALSE;

    BeginStatusPreviewView(battlerAtk, battlerDef, move);
    abilityAtk = GetBattlerAbility(battlerAtk);
    abilityDef = GetBattlerAbility(battlerDef); // Mold Breaker already applied
    fails = IsBlockedBeforeEffect(battlerAtk, battlerDef, move, abilityAtk, abilityDef)
         || IsEffectBlocked(battlerAtk, battlerDef, move, abilityAtk, abilityDef);
    EndStatusPreviewView(battlerAtk, battlerDef);
    return fails;
}

bool32 TryPrintStatusMovePreview(enum BattlerId battler, enum Move move, enum Type type)
{
    u8 *end;

    if (!B_MOVE_STATUS_PREVIEW)
        return FALSE;
    // An armed Dynamax turns the move into Max Guard; an armed Z-Move has its own view.
    switch (GetArmedGimmick(battler))
    {
    case GIMMICK_DYNAMAX:
    case GIMMICK_Z_MOVE:
        return FALSE;
    default:
        break;
    }
    if (!IsStatusMoveKnownToFail(battler, GetDamagePreviewTarget(battler), move))
        return FALSE;

    end = StringCopy(gDisplayedStringBattle, gTypesInfo[type].name);
    *end++ = CHAR_SPACE;
    end = StringCopy(end, COMPOUND_STRING("Fails"));
    PrependFontIdToFit(gDisplayedStringBattle, end, FONT_NARROW, WindowWidthPx(B_WIN_MOVE_TYPE));
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MOVE_TYPE);
    return TRUE;
}
