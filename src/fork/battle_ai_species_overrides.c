#include "global.h"
#include "battle.h"
#include "battle_ai_main.h"   // ADJUST_SCORE, AI helpers
#include "battle_ai_util.h"   // SetAIUsingGimmick, gimmick helpers
#include "battle_ai_switch.h" // struct SwitchAiContext
#include "fork/battle_ai_species_overrides.h"
#include "battle_gimmick.h"
#include "battle_util.h"      // GetBattleFormChangeTargetSpecies
#include "pokemon.h"          // GetSpeciesAbility
#include "move.h"             // GetMoveEffect
#include "constants/abilities.h"
#include "constants/battle_move_effects.h"
#include "constants/form_change_types.h"
#include "constants/pokemon.h" // DEFAULT_STAT_STAGE, STAT_SPEED
#include "constants/species.h"

// FORK: see include/battle_ai_species_overrides.h for the design rationale. Every
// public function here is reached from a single, additive hook in upstream AI
// code, all guarded by AI_FLAG_SMART_SPECIES_LOGIC at the call site.

// Score bump large enough to make the chosen move win the turn outright over a
// KO move (FAST_KILL etc. are single digits, base score is AI_SCORE_DEFAULT).
#define SPECIES_OVERRIDE_FORCE_MOVE 30

// ---------------------------------------------------------------------------
// Palafin: keep the Hero form in
// ---------------------------------------------------------------------------
// Zero to Hero is a one-time transformation; once Palafin is in its Hero form it
// is a premier sweeper, but the generic switch logic still happily pivots it out
// on a so-so matchup, throwing away the power spike. Veto voluntary switches of
// the Hero form unless staying in is clearly fatal (gets OHKO'd and can't win the
// 1v1 anyway), in which case we let the normal logic bail as usual.
bool32 AI_ShouldKeepTransformedFormIn(struct SwitchAiContext *switchContext)
{
    enum BattlerId battler = switchContext->battler;

    if (gBattleMons[battler].species != SPECIES_PALAFIN_HERO)
        return FALSE;

    if (switchContext->battlerGetsOHKOd && !switchContext->canBattlerWin1v1)
        return FALSE;

    return TRUE;
}

// ---------------------------------------------------------------------------
// Sharpedo: bank a Speed Boost before Mega Evolving
// ---------------------------------------------------------------------------
// A pre-Mega Speed Boost mon (e.g. Sharpedo) wants to Protect on its first turn
// so Speed Boost ticks for +1 Speed, then Mega Evolve the next turn keeping that
// boost. The vanilla AI assumes it Megas every turn it can, so it never sets up
// the play. We only do this when the Mega form does NOT keep Speed Boost — Mega
// Blaziken stays Speed Boost, so for it banking is pointless and we Mega at once.
static bool32 MegaFormKeepsSpeedBoost(enum BattlerId battler, enum Ability ability)
{
    enum Species megaSpecies = GetBattleFormChangeTargetSpecies(battler, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM, ability);

    if (megaSpecies == SPECIES_NONE)
        megaSpecies = GetBattleFormChangeTargetSpecies(battler, FORM_CHANGE_BATTLE_MEGA_EVOLUTION_MOVE, ability);

    if (megaSpecies == SPECIES_NONE)
        return TRUE; // can't resolve the Mega form; assume no benefit and Mega normally

    return GetSpeciesAbility(megaSpecies, 0) == ABILITY_SPEED_BOOST;
}

bool32 AI_ShouldBankSpeedBoostBeforeMega(enum BattlerId battler)
{
    enum Ability ability = gAiLogicData->abilities[battler];

    if (ability != ABILITY_SPEED_BOOST)
        return FALSE;
    if (gBattleStruct->gimmick.usableGimmick[battler] != GIMMICK_MEGA)
        return FALSE;
    // Only worth banking while Speed Boost hasn't ticked yet (Speed still at its
    // default stage). After the first boost there's nothing left to bank, so we
    // let the mon Mega Evolve normally. This is also why we don't Protect-loop.
    if (gBattleMons[battler].statStages[STAT_SPEED] != DEFAULT_STAT_STAGE)
        return FALSE;
    if (MegaFormKeepsSpeedBoost(battler, ability))
        return FALSE;

    return TRUE;
}

s32 AI_GetSpeciesOverrideMoveScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 score)
{
    if (GetMoveEffect(move) == EFFECT_PROTECT && AI_ShouldBankSpeedBoostBeforeMega(battlerAtk))
        ADJUST_SCORE(SPECIES_OVERRIDE_FORCE_MOVE);

    return score;
}
