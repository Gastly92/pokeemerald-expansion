#ifndef GUARD_CONFIG_FRONTIER_H
#define GUARD_CONFIG_FRONTIER_H

// FORK: Battle Frontier sandbox configuration. These flags are our own
// divergence from upstream; on a sync conflict, keep the flags and re-apply
// the gated behavior rather than reverting to the vanilla 3-mon / Lv50 format.

// If TRUE, Frontier singles use full 6-mon teams instead of the vanilla 3, by
// gating FRONTIER_PARTY_SIZE (constants/global.h). Currently exercised by the
// 6v6 Battle Factory sandbox, which auto-rents the full team of 6 (the rental
// select screen is skipped). NOTE: other facilities (e.g. Battle Dome) have
// fixed 3-mon coordinate tables that aren't yet generalized to 6 — they're
// stubbed to build but not visually correct at 6v6.
//
// IMPORTANT: this is defined as 1/0, NOT TRUE/FALSE, on purpose. Unlike the other
// B_FRONTIER_* flags this one feeds a numeric constant (FRONTIER_PARTY_SIZE /
// FRONTIER_DOUBLES_PARTY_SIZE) computed with `#if` in constants/global.h that
// MAP SCRIPTS read as a value (e.g. `setvar VAR_0x8005, FRONTIER_PARTY_SIZE` in
// the Battle Tower lobby). The script cpp pass does not define TRUE, so `#if
// B_FRONTIER_PARTY_SIZE_6V6` would expand to `#if TRUE` → undefined → `#if 0` and
// scripts would silently see the vanilla 3/4 while C saw 6 (the party picker then
// only lets you choose 3). Using a literal 1 makes `#if` evaluate the same in the
// C and script cpp passes; `.if B_FRONTIER_PARTY_SIZE_6V6` still works too (it
// becomes `.if 1`). See CLAUDE.md, "Using config flags in scripts".
#define B_FRONTIER_PARTY_SIZE_6V6   1

// If TRUE, Frontier battles are locked to level 100. For the Battle Factory
// this is done by forcing the challenge into Open Level mode, which already
// creates both teams at FRONTIER_MAX_LEVEL_OPEN (== MAX_LEVEL).
#define B_FRONTIER_FORCE_LVL_100    TRUE

// Frontier "endless challenge". When TRUE, a challenge no longer ends after
// FRONTIER_STAGES_PER_CHALLENGE wins — the player keeps battling indefinitely.
// A "set" is FRONTIER_STAGES_PER_CHALLENGE wins and is now purely a pacing/reward
// unit: Battle Points are awarded after EVERY win and scale up each set (2 BP/win
// in the first set, 4 in the next, 6 in the next, ...), "rest" lets the player
// step out and resume later instead of rebooting, and the Frontier Brain appears
// at the 50th and 100th wins. When FALSE the vanilla once-per-challenge flow is
// kept.
//
// Like B_FRONTIER_FORCE_LVL_100 this gates both C (#if) and map-script (.if)
// paths; the vanilla branch is preserved in each .else so upstream syncs stay
// clean. The Battle Factory is the first facility wired up to it; the others
// (Battle Tower, etc.) follow in later passes.
#define B_FRONTIER_ENDLESS  TRUE

// If TRUE, facility mons have every move's PP maxed out (full PP Up bonus on all
// four slots), instead of the vanilla base PP. Applied inside CreateFacilityMon,
// the shared builder for every facility roster, so it covers all of them: the
// Battle Factory (rentals — initial draft and 6v6 auto-rent — opponents, and the
// Frontier Brain), the Battle Tower/Dome/Palace/Arena/Pyramid trainers, the
// Battle Tent, and multi-battle partners.
#define B_FRONTIER_MAX_PP   TRUE

// If TRUE, Frontier mons are generated with max IVs (31) in every stat instead
// of the vanilla per-challenge IV ramp. Gated in GetFactoryMonFixedIV (the one
// Battle-Factory IV source), so it covers the player's rentals, the opposing
// trainers, and the Frontier Brain alike. The 6v6 auto-rent already forces max
// IVs unconditionally; this also brings the *opponents* up to 31. Factory-only
// for now (the IV ramp for other facilities lives elsewhere).
#define B_FRONTIER_MAX_IVS  TRUE

// If TRUE, Frontier opponents run the fork's two-tier AI presets instead of the
// vanilla per-challenge scaling (where the first rounds run with little or no
// AI) and instead of upstream's flat basic preset for the non-Factory
// facilities. Boss opponents — the Frontier Brain and the Battle Tower's
// gym-leader bosses — get the strongest preset (B_FRONTIER_HARD_AI_FLAGS);
// every regular opponent gets one tier below it (B_FRONTIER_REGULAR_AI_FLAGS).
// The tier choice lives in src/fork/frontier_ai.c, reached from
// GetAiScriptsInBattleFactory (Factory) and GetAiFlags (every other facility).
// The Battle Tent (FRONTIER_LVL_TENT) is untouched and keeps its vanilla AI.
#define B_FRONTIER_HARD_AI  TRUE

// The BOSS tier used when B_FRONTIER_HARD_AI is TRUE: the Frontier Brain and the
// Battle Tower's gym-leader bosses. The expansion's strongest *standard* preset:
// basic AI + OMNISCIENT (knows the player's moves/abilities/items) + smart
// switching/mon choices + PP-stall prevention + smart Tera + the fork's
// species-aware overrides + smart Z-Move (spends the one-per-battle Z-Move only
// when it secures a KO, instead of burning it turn one). Tweak here to taste —
// e.g. add AI_FLAG_PREDICTION for a meaner AI. Only expanded at the use site
// (src/fork/frontier_ai.c, which includes constants/battle_ai.h and the two fork
// AI headers), so no extra include is needed here.
#define B_FRONTIER_HARD_AI_FLAGS    (AI_FLAG_SMART_TRAINER | AI_FLAG_SMART_SPECIES_LOGIC | AI_FLAG_SMART_Z_MOVE)

// The REGULAR tier used when B_FRONTIER_HARD_AI is TRUE: every non-boss Frontier
// opponent. Deliberately one notch below B_FRONTIER_HARD_AI_FLAGS — never
// stronger in any respect — so a boss battle is always the harder fight. What it
// gives up:
//   - AI_FLAG_OMNISCIENT, the single biggest lever. Instead of reading the
//     player's whole team, it gets AI_FLAG_ASSUMPTIONS — upstream's restricted
//     knowledge set (STAB moves, a chance at certain status moves, ability
//     prediction weighted by aiRating), so it can still be scouted and baited.
//     These are the only bits the regular tier holds that the boss tier doesn't,
//     and they only ever *fill in* unknown player data (RecordMovesBasedOnStab /
//     RecordStatusMoves), which omniscience already knows outright.
//   - AI_FLAG_PP_STALL_PREVENTION, so immunity-stalling it is viable again.
//   - AI_FLAG_SMART_SPECIES_LOGIC, the fork's per-species misplay patches.
// It keeps the play-competence flags (smart switching/mon choices, randomized
// switch-ins, smart Tera, smart Z-Move) so regular opponents still play their
// teams sensibly and don't throw their one gimmick away on turn one.
#define B_FRONTIER_REGULAR_AI_FLAGS (AI_FLAG_BASIC_TRAINER | AI_FLAG_SMART_SWITCHING | AI_FLAG_SMART_MON_CHOICES \
                                     | AI_FLAG_RANDOMIZE_SWITCHIN | AI_FLAG_SMART_TERA | AI_FLAG_SMART_Z_MOVE \
                                     | AI_FLAG_ASSUMPTIONS)

// If TRUE, the post-battle Battle Factory rental-swap screen lets the player
// open a Pokémon summary for the *opponent's* mons too, not just their own.
// Vanilla only offers Summary/Swap/Rechoose when a player mon is selected; the
// opponent's mons jump straight to the "Accept this Pokémon?" prompt with no way
// to inspect their moves/stats first. With this on, selecting an opponent mon
// shows the same popup menu (Summary/Swap/Rechoose), where "Swap" accepts the
// mon. Gated in src/battle_factory_screen.c (Swap_* screen). Factory-only.
#define B_FRONTIER_FACTORY_OPP_SUMMARY  TRUE

// If TRUE, the Battle Factory draws its rental/opponent/Brain Pokémon from a
// fork-defined roster of modern competitive sets (spanning all nine generations
// and tuned around this fork's DETERMINISTIC_* changes) instead of the vanilla
// gBattleFrontierMons list. Selection is uniform across the whole roster (every
// mon is equally drawable from the start; the vanilla per-challenge tier ramp in
// sInitialRentalMonRanges is bypassed). A generated team may hold at most one
// Mega Stone and at most one Z-Crystal, since only one Mega Evolution / Z-Move is
// usable per battle (the player can still end up with extras by swapping after a
// win — that's fine, the per-battle limit still applies). The roster lives in a
// fork-owned file (src/frontier_extended_mons.c) so upstream syncs never touch
// it; only the Battle Factory's code paths swap rosters (Tower/Dome keep the
// vanilla gBattleFrontierMons for now). Gated in src/battle_factory.c and
// src/battle_factory_screen.c. The roster carries several distinct builds per
// species (so the opponent's exact set can't be read off the species) and tags
// each set FORMAT_SINGLES/DOUBLES/BOTH so format-inappropriate sets never appear.
// Generations I through IX are all built out.
#define B_FRONTIER_EXTENDED_MONS TRUE

// If TRUE, in Frontier facilities where the bag is disabled (Tower, Dome,
// Palace, Arena, Factory, Pike — everything except the Pyramid, which keeps a
// working bag), the BAG action slot is replaced by an INFO option. Selecting it
// opens a read-only screen showing the current battle state the player can know
// about: weather/terrain, entry hazards and side screens for both sides, and the
// foe's *revealed-only* data (mons seen, plus the moves/PP/ability/held item that
// have actually been shown). It reuses the B_ACTION_DEBUG controller path, so the
// turn is not consumed. Implemented in src/frontier_battle_info.c.
#define B_FRONTIER_BATTLE_INFO  TRUE

// If TRUE, the Battle Tower's Multi and Link Multi battle modes are disabled:
// talking to those two attendants shows a brief "not available" message instead
// of starting a challenge, and the Singles/Doubles modes are the only ones that
// get the 6v6 / endless treatment. These modes need a partner or a second player
// and don't fit the endless single-player loop. The Multi/Link code and scripts
// are left intact (gated by .if in the Tower lobby script), so flipping this back
// to FALSE restores them. Tower-only.
#define B_FRONTIER_TOWER_DISABLE_MULTI_LINK TRUE

// If TRUE, the per-species "banned in the Battle Frontier" flag
// (gSpeciesInfo[species].isFrontierBanned, set on legendaries/mythicals/etc.) is
// ignored, so EVERY species is legal in the Battle Frontier facilities (Battle
// Tower, Factory, etc.). Reads of the banned flag are funnelled through
// IsSpeciesFrontierBanned() (include/frontier_legality.h), which returns FALSE
// under this flag; the party-menu eligibility check, the AppendIfValid /
// CheckPartyIneligibility logic, and the "caught banned species" list viewer all
// go through it. When FALSE the vanilla per-species ban is honored.
#define B_FRONTIER_ALL_SPECIES_LEGAL    TRUE

// If TRUE, the Battle Frontier never offers to record a battle onto the Frontier
// Pass. Recorded-battle "playback" isn't a video: it re-simulates the battle from
// a saved RNG seed + the players' per-turn inputs, so it only looks right if the
// engine reproduces the original run byte-for-byte. The expansion's battle engine
// is far too stateful for that to hold reliably (this is a long-standing upstream
// fragility, not something this fork introduced — the recording engine files are
// stock upstream), so the replay desyncs into a glitched battle. Rather than ship
// a broken feature, we force FRONTIER_DATA_RECORD_DISABLED on: every facility
// attendant's "Record" menu option falls through to its existing no-record menu
// (the scripts already branch on this), and the Frontier Pass "Record" area has
// nothing to play. The recording/playback code is left untouched (so an upstream
// fix can revive it by flipping this back to FALSE). Gated in GetFrontierData()
// (FRONTIER_DATA_RECORD_DISABLED) in src/frontier_util.c.
#define B_FRONTIER_DISABLE_RECORD_BATTLE    TRUE

#endif // GUARD_CONFIG_FRONTIER_H
