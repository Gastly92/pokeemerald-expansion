#ifndef GUARD_BATTLE_AI_SPECIES_OVERRIDES_H
#define GUARD_BATTLE_AI_SPECIES_OVERRIDES_H

#include "battle_ai_switch.h" // struct SwitchAiContext

// FORK: Species-aware AI corrections. These patch a handful of spots where the
// generic AI misplays specific mons/abilities, gated behind one opt-in AI flag
// so non-smart trainers (and anything that doesn't set the flag) are untouched.
//
// AI flag bit 34: upstream uses bits 0-33 for its own flags and reserves 60-63
// for special battle conditions, leaving 34-59 free. The fork claims bit 34 for
// these overrides. Defined here (not in constants/battle_ai.h) so the definition
// stays in a fork-owned file and never conflicts on an upstream sync; if upstream
// ever assigns bit 34, move this to the next free bit.
#define AI_FLAG_SMART_SPECIES_LOGIC ((u64)1 << 34)

// Switch hook: TRUE means "do not voluntarily switch this mon out".
bool32 AI_ShouldKeepTransformedFormIn(struct SwitchAiContext *switchContext);

// Gimmick hook: TRUE means "skip Mega Evolution this turn to bank a Speed Boost".
bool32 AI_ShouldBankSpeedBoostBeforeMega(enum BattlerId battler);

// Move-scoring hook: registered as sBattleAiFuncTable[bit 34]. Signature must
// match the other entries in that table exactly.
s32 AI_GetSpeciesOverrideMoveScore(enum BattlerId battlerAtk, enum BattlerId battlerDef, enum Move move, s32 score);

#endif // GUARD_BATTLE_AI_SPECIES_OVERRIDES_H
