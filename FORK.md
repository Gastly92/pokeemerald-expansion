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
| New-game option defaults | `NEW_GAME_TEXT_SPEED`, `NEW_GAME_BATTLE_STYLE` | `config/general.h` | ✅ | Sets the initial text speed (default Fast) and battle style (default Set) for a fresh save in `SetDefaultOptions()`. Value-defines using `OPTIONS_*` constants; the player can still change them in Options. |
| Start with battle-gimmick items | `START_WITH_BATTLE_GIMMICK_ITEMS` | `config/general.h` | ⚠️ partial | New games give Mega Ring / Z-Power Ring / Dynamax Band / Tera Orb in the bag (`GiveStartingBattleGimmickItems()` in `src/new_game.c`). Mega/Z-Move/Ultra Burst work right away. **Dynamax & Tera are intentionally left disabled** (`B_FLAG_DYNAMAX_BATTLE` / `B_FLAG_TERA_ORB_*` in `config/battle.h` stay `0`) — the player holds the items but can't use those gimmicks until a planned player-facing toggle wires those flags to real flags and sets them. |
| Debug menus always on | `DEBUG_OVERWORLD_MENU`, `DEBUG_BATTLE_MENU` | `config/debug.h` | ✅ | Overworld menu (hold R + Start), battle menu (Select). |
| 6v6 Battle Factory | `B_FRONTIER_PARTY_SIZE_6V6` | `config/frontier.h` | ⚠️ partial | Frontier singles use full 6-mon teams; Factory auto-rents the team (rental select skipped, always max IVs). **Battle Dome is not yet generalized to 6** — its coordinate tables are stubbed so it builds, but the 6-mon layout is incorrect. |
| Frontier battles forced to Lv100 | `B_FRONTIER_FORCE_LVL_100` | `config/frontier.h` | ✅ | Factory forces Open Level mode (both teams at `MAX_LEVEL`). See known quirks below. |
| Endless Frontier challenge | `B_FRONTIER_ENDLESS` (+ `FRONTIER_STAGES_PER_CHALLENGE`) | `config/frontier.h`, `constants/battle_frontier.h` | ⚠️ partial | A challenge no longer ends after a set number of wins — the player keeps battling indefinitely. A "set" is `FRONTIER_STAGES_PER_CHALLENGE` (bumped 7 → 10) wins. **BP awarded after every win** (by the attendant in the pre-battle room), scaling per set (2/win for the first set, 4 for the next, 6 for the next, …). **"Rest"** now saves + restores the party + returns to the lobby (no reboot), keeping the streak active; the attendant offers to resume. In-challenge / resume detection uses the win streak, not `curChallengeBattleNum`/`challengePaused`. **Frontier Brain** moved to the 50th win (silver) / 100th win (gold), awarding BP = the current win number (50 / 100). Gated by `B_FRONTIER_ENDLESS` (C `#if` + script `.if`, vanilla preserved in each `.else`). The Battle Factory is the first facility wired up; the rest follow in later passes. |

| Frontier max PP | `B_FRONTIER_MAX_PP` | `config/frontier.h` | ✅ | Maxes PP Up bonus on all four move slots in `CreateFacilityMon`, the shared facility mon builder — so it covers every facility: Battle Factory (rentals, opponents, Brain), Tower/Dome/Palace/Arena/Pyramid trainers, Battle Tent, and multi-battle partners. |
| Frontier max IVs | `B_FRONTIER_MAX_IVS` | `config/frontier.h` | ⚠️ partial | Forces 31 IVs in every stat via `GetFactoryMonFixedIV` (the one Factory IV source), so rentals, opponents, and the Frontier Brain are all maxed instead of the vanilla per-challenge ramp. Factory-only for now; other facilities' IV ramps live elsewhere. |
| Frontier hard AI | `B_FRONTIER_HARD_AI` (+ `B_FRONTIER_HARD_AI_FLAGS`) | `config/frontier.h` | ⚠️ partial | Every Factory opponent and the Frontier Brain use the strongest AI preset (default `AI_FLAG_SMART_TRAINER` — basic AI + OMNISCIENT + smart switching/mon choices + PP-stall prevention + smart Tera) instead of the vanilla per-challenge scaling. Gated in `GetAiScriptsInBattleFactory`; Battle Tent keeps no AI. Tune via `B_FRONTIER_HARD_AI_FLAGS`. Factory-only for now. |

Legend: ✅ done · ⚠️ partial / has known limitations.

## Known quirks / future work

- **Forced Lv100 leaves dual record tracking (`B_FRONTIER_FORCE_LVL_100`).** With
  only Open Level in use, facility record TVs still display separate "Lv 50" and
  "Open Level" entries; these could later collapse into a single "records" view.
  The save block also still persists Lv50 records that can no longer change —
  candidate to drop for space if needed.
- **Battle Dome at 6v6.** See the 6v6 Factory row: the Dome's fixed 3-mon
  coordinate tables aren't generalized yet, so its layout is wrong at 6 mons.
- **Only the Battle Factory is wired up so far.** `B_FRONTIER_ENDLESS` is a
  Frontier-wide flag, but only the Battle Factory currently implements the
  endless flow (per-win BP scaling, "rest to leave / resume", Brain-at-50/100);
  the other facilities still run their vanilla once-per-challenge flow until they
  are converted. Planned next: extend it to the Battle Tower and the rest, and
  boss battles every 10 challenges (the Frontier Brain is the current stand-in).
- **Bumped `FRONTIER_STAGES_PER_CHALLENGE` (7 → 10) is global.** Per review, the
  shared constant was changed rather than adding a Factory-specific one, so the
  other facilities now see 10-stage challenges too. Their fixed-size layout
  tables (e.g. Battle Pyramid floor/pickup offsets, floor-name arrays) still have
  7 entries, so those facilities are visually/functionally off until generalized
  — accepted for now, to be fixed alongside their endless conversion.
- **Dead "won challenge" lobby path.** With `B_FRONTIER_ENDLESS` on, the lobby's
  `CHALLENGE_STATUS_WON` handler is no longer reached by new runs (it only ever
  fires for a save left mid-WON by a pre-update build, or with the flag off).
  Kept for that compatibility; a later cleanup can remove it.

## Conventions

- Intentional divergences from upstream inside upstream-owned files are tagged
  `FORK:` (greppable: `grep -rn "FORK:" src include`).
- Prefer adding behavior behind a config flag in `include/config/*.h` over
  patching core logic, so changes land in files we own and survive upstream
  syncs cleanly. See `CLAUDE.md` for the full rationale.
