#ifndef GUARD_FRONTIER_BATTLE_INFO_H
#define GUARD_FRONTIER_BATTLE_INFO_H

// read-only "battle info" viewer that replaces the (disabled) BAG action
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

// B_FRONTIER_BATTLE_INFO -- how many Mega/Primal rows the viewer's Base Stats page can
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

// BUFF_ACCURACY_ITEMS_REVEAL -- writes the reveal bits the player's held Wide Lens
// (every seen foe's item) and Zoom Lens (the ability and full moveset of a foe that has used
// a move) can see. Called when the viewer is opened; exposed so test/fork/buff_accuracy_items.c
// can assert the bits directly rather than driving the UI.
void ApplyAccuracyItemReveals(void);

// The Foe page's "HP n%": the foe party slot's current HP as a % of its max, ROUNDED UP --
// the opposite of the move menu's rounded-down damage range, so "damage >= HP" on screen always
// means a real KO -- but capped at 99 below full HP, so 100 still means untouched. An
// on-field mon is read from its battler (a Dynamaxed foe's bar is its multiplied HP), a benched
// one from the party. Exposed for test/fork/frontier_battle_info_hp.c.
u32 GetFoeHpPercent(u32 foeIndex);
// The same rounded-up figure for one of the player's on-field battlers (Overview page "You:" rows):
// a 1/4-max-HP cost like Substitute is affordable iff it reads 26 or more.
u32 GetBattlerHpPercent(u32 battler);
// The Overview page's Speed range for a seen foe party slot: 0 IV / 0 EV / hindering nature up to
// 31 / 252 / boosting, then scaled by every modifier the player can see (stat stages, paralysis,
// Tailwind, weather, revealed item and ability, innates). A benched foe is priced as if it
// switched in now. `tie` is how a Speed tie with your fastest active mon would settle under
// DETERMINISTIC_MOVE_RESULTS' tie ladder, read from the same view: 1 you win, -1 the foe wins,
// 0 unsettled (config off, or every rung tied). On the number axis, i.e. not flipped by Trick Room.
// Exposed for test/fork/frontier_battle_info_speed.c.
void GetFoeSpeedRange(u32 foeIndex, u32 *lo, u32 *hi, s32 *tie);
// The foe row's glyph: CHAR_DOWN_ARROW when your fastest active mon outspeeds the whole range,
// CHAR_UP_ARROW when the whole range outspeeds you, else 0. A tie at the edge of the range
// counts the way the tie ladder settles it.
u32 GetFoeSpeedGlyph(u32 foeIndex);
// The type icons on a seen foe's Overview row, written to `types`; returns how many (1 or 2).
// The displayed (Illusion-aware) species' types, or the Tera type alone once the mon has
// Terastallized (Stellar keeps the species' types). Exposed for test/fork/frontier_battle_info_types.c.
u32 GetFoeOverviewTypes(u32 foeIndex, enum Type types[2]);

#endif // GUARD_FRONTIER_BATTLE_INFO_H
