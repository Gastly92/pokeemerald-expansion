#ifndef GUARD_FRONTIER_BATTLE_INFO_H
#define GUARD_FRONTIER_BATTLE_INFO_H

// FORK: read-only "battle info" viewer that replaces the (disabled) BAG action
// in Frontier facilities. Gated by B_FRONTIER_BATTLE_INFO (config/frontier.h).
// Piggybacks on the existing B_ACTION_DEBUG controller plumbing: when the player
// chooses the INFO slot we set gFrontierBattleInfoActive and emit B_ACTION_DEBUG,
// and PlayerHandleBattleDebug opens CB2_FrontierBattleInfo instead of the debug
// menu. See src/frontier_battle_info.c.

extern bool8 gFrontierBattleInfoActive;

// TRUE when the BAG action slot should be shown/treated as INFO this battle
// (flag on + a facility where the bag is disabled). Always FALSE when the flag
// is off, so callers compile down to vanilla behavior.
bool32 ShouldReplaceBagWithInfo(void);

void CB2_FrontierBattleInfo(void);

// Opens the viewer and returns to returnCallback when closed. Used from the
// in-battle action menu (INFO slot) and the in-battle party menu (SELECT).
void OpenFrontierBattleInfo(void (*returnCallback)(void));

#endif // GUARD_FRONTIER_BATTLE_INFO_H
