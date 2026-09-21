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

// FEATURE_INNATE_ABILITIES -- how many innates the viewer's dedicated innates page can
// list for one foe (its row budget, two columns deep). A species declaring more than this would
// be counted on the Foe page's "+N innates" hint but silently truncated on the page itself, so
// test/fork/innate_abilities.c guards the table against it.
#define INFO_MAX_DISPLAYED_INNATES 14

// FORK: B_FRONTIER_BATTLE_INFO -- how many Mega/Primal rows the viewer's Base Stats page can
// list beneath the foe's own spread. It is both a display budget and a layout one: the page
// draws your active mon(s) above the foe, and a STATIC_ASSERT in src/fork/frontier_battle_info.c
// proves the worst case (doubles, both of your mons projecting a form) still fits. A species
// declaring more reachable forms than this would have one silently dropped, so the sweep in
// test/fork/frontier_battle_info_reveal.c guards the form-change tables against it -- if it
// ever fires, the page layout needs reworking, not just a bigger number here.
#define INFO_MAX_DISPLAYED_ALT_FORMS 2

void CB2_FrontierBattleInfo(void);

// Opens the viewer and returns to returnCallback when closed. Used from the
// in-battle action menu (INFO slot) and the in-battle party menu (SELECT).
void OpenFrontierBattleInfo(void (*returnCallback)(void));

// FORK: BUFF_ACCURACY_ITEMS_REVEAL -- writes the reveal bits the player's held Wide Lens
// (every seen foe's item) and Zoom Lens (the ability and full moveset of a foe that has used
// a move) can see. Called when the viewer is opened; exposed so test/fork/buff_accuracy_items.c
// can assert the bits directly rather than driving the UI.
void ApplyAccuracyItemReveals(void);

#endif // GUARD_FRONTIER_BATTLE_INFO_H
