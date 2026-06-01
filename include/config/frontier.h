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
#define B_FRONTIER_PARTY_SIZE_6V6   TRUE

// If TRUE, Frontier battles are locked to level 100. For the Battle Factory
// this is done by forcing the challenge into Open Level mode, which already
// creates both teams at FRONTIER_MAX_LEVEL_OPEN (== MAX_LEVEL).
#define B_FRONTIER_FORCE_LVL_100    TRUE

// Frontier "endless challenge". When TRUE, a challenge no longer ends after
// FRONTIER_STAGES_PER_CHALLENGE wins — the player keeps battling indefinitely.
// A "set" is FRONTIER_STAGES_PER_CHALLENGE (10) wins and is now purely a
// pacing/reward unit: Battle Points are awarded after EVERY win and scale up
// each set (2 BP/win in the first set, 4 in the next, 6 in the next, ...),
// "rest" lets the player step out and resume later instead of rebooting, and the
// Frontier Brain appears at the 50th and 100th wins. When FALSE the vanilla
// once-per-challenge flow is kept.
//
// Like B_FRONTIER_FORCE_LVL_100 this gates both C (#if) and map-script (.if)
// paths; the vanilla branch is preserved in each .else so upstream syncs stay
// clean. Only the Battle Factory honors this flag for now; the other facilities
// (Battle Tower, etc.) will be converted in later passes.
#define B_FRONTIER_ENDLESS  TRUE

#endif // GUARD_CONFIG_FRONTIER_H
