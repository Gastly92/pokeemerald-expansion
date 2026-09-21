#ifndef GUARD_FORK_FREE_GIMMICKS_H
#define GUARD_FORK_FREE_GIMMICKS_H

#include "pokemon.h"
#include "constants/form_change_types.h"

// FORK (FEATURE_FREE_GIMMICKS): Mega Evolution is item-free, so for a species with more
// than one Mega form there is no stone to say which one you get. We pick it from the mon's
// own offensive stats and return the Mega Stone that yields it, to feed into the held-item
// driven form change machinery. The chosen form is the physical one when the mon's
// Attack >= Sp. Atk, otherwise the special one; since the physical form always has the
// higher (or equal) Attack-minus-Sp.Atk of the pair, a stat tie defaults to it.
// Single-Mega species return their one stone; non-Mega species return ITEM_NONE.
//
// This used to be GetMegaStoneForBattler() in src/battle_util.c, reading gBattleMons
// directly. It lives here, taking the two stats as arguments, because the INFO viewer's
// Base Stats page has to answer the same question for a *benched* party mon, which has no
// battler and therefore no gBattleMons entry. `static inline` rather than a fork .c file so
// both callers keep inlining it (the pattern InnateUnawareBoonStage uses).
//
// Callers: GetBattleFormChangeTargetSpecies (src/battle_util.c, the live form change) and
// GetPartyMonProjectedForm (src/fork/frontier_battle_info.c, the viewer's projection). Both
// must agree, which is the whole reason this is one function.
static inline u16 FindMegaStoneForStats(enum Species species, u32 attack, u32 spAttack)
{
    const struct FormChange *formChanges = GetSpeciesFormChanges(species);
    bool32 preferPhysical = (attack >= spAttack);
    u16 bestItem = ITEM_NONE;
    s32 bestScore = 0;

    if (formChanges == NULL)
        return ITEM_NONE;

    for (u32 i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
    {
        if (formChanges[i].method != FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM)
            continue;

        enum Species target = formChanges[i].targetSpecies;
        s32 bias = (s32)gSpeciesInfo[target].baseAttack - (s32)gSpeciesInfo[target].baseSpAttack;
        s32 score = preferPhysical ? bias : -bias;

        if (bestItem == ITEM_NONE || score > bestScore)
        {
            bestItem = formChanges[i].param1;
            bestScore = score;
        }
    }

    return bestItem;
}

#endif // GUARD_FORK_FREE_GIMMICKS_H
