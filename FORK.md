# Fork changes

This is a fork of [RHH's `pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).
It tracks upstream and layers a set of custom features on top to build a
**standalone single-player romhack** centered on quality-of-life improvements to
the Battle Frontier facilities, growing into a fuller game over time. See
[`CLAUDE.md`](CLAUDE.md) for the conventions and the upstream-sync process.

This file is an **index**, not a spec: each feature is gated by a config flag
whose comment in `include/config/*.h` is the source of truth for its exact
behavior. The table records what the flag comment can't — status, known
limitations, and where to look.

## Features

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Boot straight to the main menu | `SKIP_TITLE_SEQUENCE` | `config/general.h` | ✅ | Skips copyright/intro/title. RHH intro still plays if `EXPANSION_INTRO`. |
| Trim Prof. Birch's new-game intro | `SKIP_BIRCH_SPEECH` | `config/general.h` | ✅ | Keeps look + name selection, drops the monologue. |
| Gender-neutral text | `GENDER_NEUTRAL_TEXT` | `config/general.h` | ✅ | Replaces gendered wording with neutral text. Currently the new-game look picker; the flag can extend to other gendered strings later. Text-only; selection unchanged. |
| Start a new game at the Battle Frontier | `START_AT_BATTLE_FRONTIER` | `config/general.h` | ✅ | Warps to the Frontier dock as if first arriving. See `SetBattleFrontierFirstArrivalState()` in `src/new_game.c`. No effect on FRLG. |
| Debug menus always on | `DEBUG_OVERWORLD_MENU`, `DEBUG_BATTLE_MENU` | `config/debug.h` | ✅ | Overworld menu (hold R + Start), battle menu (Select). |
| 6v6 Battle Factory | `B_FRONTIER_PARTY_SIZE_6V6` | `config/frontier.h` | ⚠️ partial | Frontier singles use full 6-mon teams; Factory auto-rents the team (rental select skipped, always max IVs). **Battle Dome is not yet generalized to 6** — its coordinate tables are stubbed so it builds, but the 6-mon layout is incorrect. |
| Frontier battles forced to Lv100 | `B_FRONTIER_FORCE_LVL_100` | `config/frontier.h` | ✅ | Factory forces Open Level mode (both teams at `MAX_LEVEL`). See known quirks below. |

Legend: ✅ done · ⚠️ partial / has known limitations.

## Known quirks / future work

- **Forced Lv100 leaves dual record tracking (`B_FRONTIER_FORCE_LVL_100`).** With
  only Open Level in use, facility record TVs still display separate "Lv 50" and
  "Open Level" entries; these could later collapse into a single "records" view.
  The save block also still persists Lv50 records that can no longer change —
  candidate to drop for space if needed.
- **Battle Dome at 6v6.** See the 6v6 Factory row: the Dome's fixed 3-mon
  coordinate tables aren't generalized yet, so its layout is wrong at 6 mons.

## Conventions

- Intentional divergences from upstream inside upstream-owned files are tagged
  `FORK:` (greppable: `grep -rn "FORK:" src include`).
- Prefer adding behavior behind a config flag in `include/config/*.h` over
  patching core logic, so changes land in files we own and survive upstream
  syncs cleanly. See `CLAUDE.md` for the full rationale.
