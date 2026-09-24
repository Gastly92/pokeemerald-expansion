#include "global.h"
#include "battle.h"
#include "battle_ai_util.h"
#include "battle_controllers.h" // BattlerHasAi, GetBattlerTrainer
#include "battle_dynamax.h"
#include "battle_gimmick.h"
#include "battle_z_move.h"
#include "config/feature.h"
#include "constants/config_changes.h"
#include "random.h"
#include "test_runner.h"
#include "fork/battle_ai_gimmick.h"

static const enum Gimmick sAiGimmickPreference[] = { AI_GIMMICK_PREFERENCE_ORDER };

static bool32 IsGimmickAvailable(enum BattlerId battler, enum Gimmick gimmick)
{
    return (GetGimmickCandidates(battler) & (1u << gimmick))
        && !HasTrainerUsedGimmick(battler, gimmick);
}

// Whether any of this battler's moves would KO a foe outright with `gimmick` active.
// usableGimmick is what AI_CalcDamage reads to decide which gimmick to simulate, so the
// candidate is swapped in around the calc and restored afterwards.
static bool32 GimmickSecuresKO(enum BattlerId battler, enum Gimmick gimmick)
{
    enum Gimmick saved = gBattleStruct->gimmick.usableGimmick[battler];
    enum Move *moves = GetMovesArray(battler);
    bool32 securesKO = FALSE;

    gBattleStruct->gimmick.usableGimmick[battler] = gimmick;

    for (enum BattlerId target = 0; target < gBattlersCount && !securesKO; target++)
    {
        if (!IsBattlerAlive(target) || GetBattlerSide(target) == GetBattlerSide(battler))
            continue;

        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            uq4_12_t effectiveness;
            struct SimulatedDamage dmg;

            if (moves[moveIndex] == MOVE_NONE || GetMovePower(moves[moveIndex]) == 0)
                continue;

            dmg = AI_CalcDamageSaveBattlers(moves[moveIndex], battler, target, &effectiveness, gimmick, GIMMICK_NONE);
            if (dmg.minimum >= gBattleMons[target].hp)
            {
                securesKO = TRUE;
                break;
            }
        }
    }

    gBattleStruct->gimmick.usableGimmick[battler] = saved;
    return securesKO;
}

// Whether any foe could KO this battler this turn with a move the AI knows about.
static bool32 IsThreatenedWithKO(enum BattlerId battler)
{
    for (enum BattlerId foe = 0; foe < gBattlersCount; foe++)
    {
        enum Move *moves;

        if (!IsBattlerAlive(foe) || GetBattlerSide(foe) == GetBattlerSide(battler))
            continue;

        moves = GetMovesArray(foe);
        for (u32 moveIndex = 0; moveIndex < MAX_MON_MOVES; moveIndex++)
        {
            uq4_12_t effectiveness;

            if (moves[moveIndex] == MOVE_NONE || GetMovePower(moves[moveIndex]) == 0)
                continue;

            if (AI_CalcDamageSaveBattlers(moves[moveIndex], foe, battler, &effectiveness, GIMMICK_NONE, GIMMICK_NONE).median >= gBattleMons[battler].hp)
                return TRUE;
        }
    }

    return FALSE;
}

// Teammates that could still use a shared gimmick after this battler: the healthy reserves,
// plus a living partner in doubles.
static u32 CountLaterGimmickUsers(enum BattlerId battler)
{
    u32 count = CountUsablePartyMons(battler);

    if (IsDoubleBattle() && IsBattlerAlive(GetPartnerBattler(battler)))
        count++;

    return count;
}

// FORK: decide whether a picked Mega or Dynamax fires this turn or is held for a later one.
// See AI_FREE_MEGA_COMMIT_CHANCE for the rationale. The caller has already established
// whether the gimmick secures a KO, which always commits. The rolls are phrased as "commit"
// so a battle test that does not rig them sees the gimmick used, as before this existed.
static bool32 ShouldCommitGimmickNow(enum BattlerId battler, enum Gimmick gimmick, bool32 securesKO)
{
    u32 laterUsers, holdChance, commitChance;

    if (gimmick != GIMMICK_MEGA && gimmick != GIMMICK_DYNAMAX)
        return TRUE;
    if (securesKO)
        return TRUE;

    laterUsers = CountLaterGimmickUsers(battler);
    if (laterUsers == 0 || IsThreatenedWithKO(battler))
        return TRUE;

    if (gimmick == GIMMICK_MEGA)
        return RandomPercentage(RNG_AI_COMMIT_MEGA, AI_FREE_MEGA_COMMIT_CHANCE);

    // Computed into a local first: RandomPercentage does not parenthesise its argument.
    holdChance = laterUsers * AI_FREE_DYNAMAX_HOLD_CHANCE_PER_MON;
    if (holdChance >= 100)
        return FALSE;
    commitChance = 100 - holdChance;
    return RandomPercentage(RNG_AI_COMMIT_DYNAMAX, commitChance);
}

// Settle on `gimmick` for this turn, or on none if it is being held. Clearing usableGimmick
// (rather than only the aiUsingGimmick bit) matters: the move-damage pass that follows
// caches the turn's damage with usableGimmick forced on, so a held Dynamax would otherwise
// still be scored as Max Moves.
static void SetGimmickForTurn(enum BattlerId battler, enum Gimmick gimmick, bool32 securesKO)
{
    if (!ShouldCommitGimmickNow(battler, gimmick, securesKO))
        gimmick = GIMMICK_NONE;

    gBattleStruct->gimmick.usableGimmick[battler] = gimmick;
}

// FORK: pick this battler's gimmick from its full candidate set instead of leaving it on
// the enum-order default AssignUsableGimmicks seeded, then decide whether it fires this turn.
//
// Two passes, because the fork allows each gimmick type only once per trainer per battle
// (see fork-docs/FORK.md), which makes every one of them a resource worth placing well:
//   1. Any candidate that turns this turn into a KO wins - tempo is worth more than
//      holding the resource, and a KO now is the one benefit that cannot be deferred.
//   2. Otherwise fall back to AI_GIMMICK_PREFERENCE_ORDER, which puts the persistent
//      form changes ahead of the single-use Z-Move, so the Z-Move stays in reserve.
// A Mega or Dynamax picked by pass 2 may then be held for a later turn (ShouldCommitGimmickNow).
// Ultra Burst is left alone when available, because it is not a gimmick in its own right -
// it is the form change that unlocks Necrozma's Z-Move, so overriding it costs the mon both.
static void AI_SelectBestGimmick(enum BattlerId battler)
{
    u32 candidates = GetGimmickCandidates(battler);
    enum Gimmick current = gBattleStruct->gimmick.usableGimmick[battler];

    if (candidates == 0 || (candidates & (1u << GIMMICK_ULTRA_BURST)))
        return;

    #if TESTING
    // Battle tests fix *which* gimmick through the DSL; honour it over the preference, but
    // still decide *when* it fires.
    if (TestRunner_Battle_GetChosenGimmick(GetBattlerTrainer(battler), gBattlerPartyIndexes[battler]) != GIMMICK_NONE)
    {
        if (IsGimmickAvailable(battler, current))
            SetGimmickForTurn(battler, current, GimmickSecuresKO(battler, current));
        return;
    }
    #endif

    for (u32 i = 0; i < ARRAY_COUNT(sAiGimmickPreference); i++)
    {
        if (IsGimmickAvailable(battler, sAiGimmickPreference[i])
         && GimmickSecuresKO(battler, sAiGimmickPreference[i]))
        {
            SetGimmickForTurn(battler, sAiGimmickPreference[i], TRUE);
            return;
        }
    }

    for (u32 i = 0; i < ARRAY_COUNT(sAiGimmickPreference); i++)
    {
        if (IsGimmickAvailable(battler, sAiGimmickPreference[i]))
        {
            SetGimmickForTurn(battler, sAiGimmickPreference[i], FALSE);
            return;
        }
    }
}

// FORK: run the pick for every AI battler once per turn, from inside SetAiLogicDataForTurn
// between the battler-data pass and the move-damage pass. It needs the former (the KO check
// runs real damage calcs, which read gAiLogicData's abilities and hold effects) and must
// precede the latter, which caches a whole turn of simulated damage with the selected
// gimmick forced on - choosing afterwards would score every move against a gimmick the AI
// is no longer going to use.
void AI_SelectGimmicksForTurn(void)
{
    if (!GetConfig(FEATURE_FREE_GIMMICKS))
        return;

    for (enum BattlerId battler = 0; battler < gBattlersCount; ++battler)
    {
        if (IsBattlerAlive(battler) && BattlerHasAi(battler))
            AI_SelectBestGimmick(battler);
    }
}

// FORK: resolve a chosen move to the move the engine will actually execute for it, given
// the attacker's *active* gimmick. Mirrors the conversion HandleAction_UseMove performs
// right before the move runs (Z-Move / Max Move), and returns `move` untouched when
// nothing converts.
//
// The AI needs this because a converted move keeps the base move's *type* but none of its
// type-matchup quirks - Freeze-Dry's bonus against Water, Flying Press's second Flying
// pass, Thousand Arrows' grounding of Flying-types, Synchronoise's same-type-only rule.
// Subzero Slammer and Max Hailstorm are plain Ice moves, so a Freeze-Dry the AI reads as
// 2x is 0.5x once it upgrades: a 4x error in the wrong direction, which is exactly what
// made the AI upgrade into resists.
//
// Keyed off GetActiveGimmick rather than usableGimmick so it is also right for a battler
// that is already Dynamaxed - those use Max Moves whether or not this calc is simulating
// a gimmick.
enum Move AI_GetGimmickExecutedMove(enum BattlerId battlerAtk, enum Move move)
{
    switch (GetActiveGimmick(battlerAtk))
    {
    case GIMMICK_Z_MOVE:
        // Status moves are not converted; they run as themselves and apply a Z effect.
        if (!IsBattleMoveStatus(move) && !IsZMove(move))
        {
            enum Move zMove = GetUsableZMove(battlerAtk, move);
            if (zMove != MOVE_NONE)
                return zMove;
        }
        break;
    case GIMMICK_DYNAMAX:
    {
        enum Move maxMove = GetMaxMove(battlerAtk, move);
        if (maxMove != MOVE_NONE)
            return maxMove;
        break;
    }
    default:
        break;
    }

    return move;
}
