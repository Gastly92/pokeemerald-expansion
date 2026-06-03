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
| Endless Frontier challenge | `B_FRONTIER_ENDLESS` (+ `FRONTIER_STAGES_PER_CHALLENGE`) | `config/frontier.h`, `constants/battle_frontier.h` | ⚠️ partial | A challenge no longer ends after a set number of wins — the player keeps battling indefinitely. A "set" is `FRONTIER_STAGES_PER_CHALLENGE` (bumped 7 → 10) wins. **BP awarded after every win** (by the attendant in the pre-battle room), scaling per set (2/win for the first set, 4 for the next, 6 for the next, …). **"Rest"** now saves + restores the party + returns to the lobby (no reboot), keeping the streak active; the attendant offers to resume. In-challenge / resume detection uses the win streak, not `curChallengeBattleNum`/`challengePaused`. Because a singles and a doubles challenge can both be rested at once, the rental team (the shared `rentalMons`) is snapshotted per battle mode into `frontier.factoryRentedMonsBackup` on rest and restored on resume, so the two modes keep separate teams. **Frontier Brain** moved to the 50th win (silver) / 100th win (gold), awarding BP = the current win number (50 / 100). Gated by `B_FRONTIER_ENDLESS` (C `#if` + script `.if`, vanilla preserved in each `.else`). The Battle Factory is the first facility wired up; the rest follow in later passes. |

| Frontier max PP | `B_FRONTIER_MAX_PP` | `config/frontier.h` | ✅ | Maxes PP Up bonus on all four move slots in `CreateFacilityMon`, the shared facility mon builder — so it covers every facility: Battle Factory (rentals, opponents, Brain), Tower/Dome/Palace/Arena/Pyramid trainers, Battle Tent, and multi-battle partners. |
| Frontier max IVs | `B_FRONTIER_MAX_IVS` | `config/frontier.h` | ⚠️ partial | Forces 31 IVs in every stat via `GetFactoryMonFixedIV` (the one Factory IV source), so rentals, opponents, and the Frontier Brain are all maxed instead of the vanilla per-challenge ramp. Factory-only for now; other facilities' IV ramps live elsewhere. |
| Frontier hard AI | `B_FRONTIER_HARD_AI` (+ `B_FRONTIER_HARD_AI_FLAGS`) | `config/frontier.h` | ⚠️ partial | Every Factory opponent and the Frontier Brain use the strongest AI preset (default `AI_FLAG_SMART_TRAINER` — basic AI + OMNISCIENT + smart switching/mon choices + PP-stall prevention + smart Tera) instead of the vanilla per-challenge scaling. Gated in `GetAiScriptsInBattleFactory`; Battle Tent keeps no AI. Tune via `B_FRONTIER_HARD_AI_FLAGS`. Factory-only for now. |
| Factory swap: opponent summary | `B_FRONTIER_FACTORY_OPP_SUMMARY` | `config/frontier.h` | ✅ | On the post-battle rental-swap screen, selecting an *opponent* mon now opens the same Summary/Swap/Rechoose popup the player's mons get (vanilla jumps straight to "Accept this Pokémon?" with no way to inspect moves/stats first); "Swap" accepts the mon. Gated in `src/battle_factory_screen.c` (`Swap_*`). The duplicate-species guard still runs before the menu. |
| In-battle INFO viewer | `B_FRONTIER_BATTLE_INFO` | `config/frontier.h` | ⚠️ partial | In facilities where the bag is disabled (`BATTLE_TYPE_FRONTIER_NO_PYRAMID` — Tower, Dome, Palace, Arena, Factory, Pike; **not** the Pyramid, which keeps a working bag), the BAG action slot becomes **INFO**, opening a read-only screen (`src/frontier_battle_info.c`, `CB2_FrontierBattleInfo`). Four pages, R/L cycle: **Field** (weather/terrain + entry hazards & side screens both sides), **Conditions** (each on-field battler's primary status + notable volatiles — confusion, leech seed, taunt, etc. — both sides), **Stat Changes** (each on-field battler's non-default stat stages, e.g. `Atk+2 Spe-1`, both sides), **Foe** (the foe's **revealed-only** party data; `<>` cycles mons — species/gender/level, `FNT` when fainted, plus moves/PP/ability/held-item). "Seen" comes from `gBattleStruct->partyState[...].sentOut`; species/gender/level/fainted read from the party once seen. Everything else is gated on genuine reveal, because `gAiPartyData` is the AI's *knowledge* model (pre-filled under `AI_FLAG_OMNISCIENT` and padded with STAB/status assumptions), not "what the player saw": used moves are tracked in `gBattleStruct->infoUsedMoves` (set at the move-use site), and ability/held-item in `gBattleStruct->infoAbilityRevealed`/`infoItemRevealed` (set in `RecordAbilityBattle`/`RecordItemEffectBattle`). Reuses the `B_ACTION_DEBUG` controller path so the turn isn't consumed; touches only the player controller, those two record fns, and the move-use site (`FORK:` tags) + a menu-label string. **Foe page reads opponent A only** (single-trainer doubles / singles); two-opponent multi battles show just the first foe trainer's party for now. |

| Deterministic critical hits | `DETERMINISTIC_CRITICAL_HITS` | `config/deterministic.h` | ✅ | First of the `DETERMINISTIC_*` flags (a project to strip RNG from the game). Removes the random crit-chance roll in `IsCriticalHit()` (`src/battle_util.c`): crits still land only when *guaranteed* — always-crit moves, Laser Focus, Merciless vs. a poisoned target, or crit-stage stacks that already reach 1/1 odds. Crit-blocking (Battle Armor, Shell Armor, Lucky Chant) unchanged. The AI is made config-aware: its damage prediction (`ShouldCalcCritDamage` in `src/battle_ai_util.c`) no longer assumes partial-chance crits that can't land — counting a crit only when guaranteed, via the shared `IsGuaranteedCriticalHit()` predicate (also the source of truth in `IsCriticalHit`) — and its crit-*chance* move valuations (`src/battle_ai_main.c`: the Risky high-crit-move bonus, Focus Energy, and Dragon Cheer) stop rewarding chance it can no longer cash in (Laser Focus and always-crit synergies still count, since those crits are guaranteed). **Enabled (`TRUE`) in this fork.** Covered by `test/battle/deterministic_critical_hits.c` (opt-in via `WITH_CONFIG`); see the determinism test harness note below. |
| Deterministic damage | `DETERMINISTIC_DAMAGE` (+ `DETERMINISTIC_DAMAGE_BASE_PERCENT`, `DETERMINISTIC_DAMAGE_TURN_INCREMENT`) | `config/deterministic.h` | ✅ | Replaces the random 85%–100% damage roll with a fixed multiplier that scales with the battle's turn count: `BASE_PERCENT` on turn 1, `+TURN_INCREMENT` per elapsed turn (`DETERMINISTIC_DAMAGE_PERCENT` = `BASE_PERCENT + TURN_INCREMENT * gBattleTurnCounter`). Defaults 92% / +1% → 92% turn 1, 93% turn 2, … **Intentionally uncapped** — from turn 9 on it exceeds 100%, so prolonged battles deal more than the stock maximum (set `TURN_INCREMENT` to 0 for a flat multiplier). The AI's damage prediction (`src/battle_ai_util.c` roll helpers) is fed the same figure, so its min/median/max/random rolls collapse to the deterministic value and it never mis-predicts. **Enabled (`TRUE`) in this fork.** Covered by `test/battle/deterministic_damage.c` (opt-in via `WITH_CONFIG`); see the determinism test harness note below. |
| Deterministic flinch | `DETERMINISTIC_FLINCH` | `config/deterministic.h` | ✅ | An **anti flinch-lock cap** that *composes* with `DETERMINISTIC_ADDITIONAL_EFFECTS`: a flinch effect is first gated on super effective / STAB like any other effect (see that flag — Iron Head/Rock Slide only flinch on a super effective hit, Stomp only from a Normal user), and this flag then prevents a *gated* flinch from being re-applied to a foe that was flinched the previous turn, so a fast flincher can't chain it into an inescapable lock (each flinch "uses up" the next turn's). Implemented in `TryTriggerAdditionalEffect()` (`src/battle_util.c`); the "flinched last turn" bit is snapshotted per battler in `HandleEndTurn_ContinueBattle` (`src/battle_main.c`, `BattlerState.flinchedLastTurn`) just before the volatile is cleared. Guaranteed flinches (chance ≥ 100%) and **Fake Out** / any first-turn-only flincher are exempt (they bypass both the gate and the cap). Inner Focus / Shield Dust / Covert Cloak immunity unchanged. The AI's flinch valuation (`IsFlinchGuaranteed`, `AI_CalcAdditionalEffectScore` via `AI_IsAdditionalEffectReliable`) is taught the composed rule. Required by `DETERMINISTIC_ADDITIONAL_EFFECTS` so its gated flinches can't lock. **Enabled (`TRUE`).** Covered by `test/battle/deterministic_flinch.c`. |
| Deterministic additional effects | `DETERMINISTIC_ADDITIONAL_EFFECTS` | `config/deterministic.h` | ✅ | A move's chance-based additional effect (burn, paralysis, a stat drop, …) stops being a random roll and lands on a fixed, state-based condition: if the move's type **can** be super effective (every type but Normal in the stock chart), the effect lands only on a super effective hit (e.g. Fire Punch burns only a Fire-weak target); if the type **never** can (Normal), it lands only when the move is **STAB** (e.g. Body Slam paralyzes only from a Normal user). **Only applies to sub-100% effects** — guaranteed (≥ 100%) or chance-less (primary) effects always land, unchanged. **Flinch obeys the same gate** (Iron Head/Rock Slide only flinch on a super effective hit, Stomp only from a Normal user); `DETERMINISTIC_FLINCH` (required) then adds an anti-lock cap on top so a gated flinch can't chain into a stunlock. The stock secondary-chance boosters (**Serene Grace** and the **Pledge Rainbow**), which would otherwise be near-useless under this flag, instead make a holder's effect *certain* — bypassing the gate and always landing (detected as "computed chance > base chance" so both sources and their quirks fall out for free). This **includes flinch**: a boosted flinch lands even on a neutral/resisted hit, but still keeps the anti-lock cap (it just can't be re-applied next turn), so the boosters can't restore flinch-lock. Implemented in `TryTriggerAdditionalEffect()` / `DeterministicAdditionalEffectApplies()` (`src/battle_util.c`), called from `Cmd_setadditionaleffects`; the AI is taught the same condition via `AI_IsAdditionalEffectReliable` (`src/battle_ai_util.c`, used by `AI_CalcAdditionalEffectScore`). **Enabled (`TRUE`).** Covered by `test/battle/deterministic_additional_effects.c`. (The super-effective branch relies on a recorded damage result; in practice every one of the game's chance-based additional effects is on a damaging move, so this is always available.) |
| Deterministic paralysis | `DETERMINISTIC_PARALYSIS` (+ `DETERMINISTIC_PARALYSIS_PP_TAX`, `DETERMINISTIC_PARALYSIS_PRIORITY_TAX`) | `config/deterministic.h` | ✅ | Turns paralysis from a coin-flip into a flat, predictable tax. Removes the random 25% full-paralysis miss (`CancelerParalyzed`, `src/battle_move_resolution.c`) and the Speed cut (`GetBattlerTotalSpeedStat`, `src/battle_main.c`); in their place every move the paralyzed battler uses costs **+`DETERMINISTIC_PARALYSIS_PP_TAX` PP** (`CancelerPPDeduction`) and has its **priority lowered by `DETERMINISTIC_PARALYSIS_PRIORITY_TAX`** (`GetBattleMovePriority`), defaults 1/1, so it acts later in its bracket and burns PP faster while keeping full Speed (set either tax to 0 to drop it). **Quick Feet** (which already ignores the Speed drop) is exempt from both taxes. The full-paralysis roll still consults `RNG_PARALYSIS` (guaranteed pass via `RandomChance(1, 1)`) so `PASSES_RANDOMLY` tests stay valid. Because turn order (Speed + priority) is read through the shared engine functions, the AI's turn-order *prediction* tracks this automatically; the AI's paralysis *valuation* (`IncreaseParalyzeScore`) is also made config-aware so it values the priority-bracket demotion instead of the stale Speed-halving check. **Enabled (`TRUE`) in this fork.** Covered by `test/battle/deterministic_paralysis.c` (opt-in via `WITH_CONFIG`); see the determinism test harness note below. |
| Deterministic hold effects | `DETERMINISTIC_HOLD_EFFECTS` | `config/deterministic.h` | ✅ | Turns chance-to-trigger hold items into guaranteed one-shot **entry items**: the effect always fires, but only on the first turn the holder is on the field (its entry turn — `IsBattlersFirstTurn`, so leads *and* mid-battle switch-ins each get one window), then the item is consumed. **Focus Band** becomes a Focus Sash that works from **any** HP (not just full) but only on the entry turn — survives one lethal hit at 1 HP then is consumed (`GetAdjustedDamage` in `src/battle_util.c`, consumed via `BattleScript_HangedOnMsg`); like Sash, a multi-hit move still gets around it. **Quick Claw / Razor Fang share-effect aside**, **Quick Claw** always moves first in-bracket on the entry turn regardless of move, then is consumed (`TryChangingTurnOrderEffects`, `src/battle_main.c`; consumed in `BattleScript_QuickClawActivation`). **Crit items** — Scope Lens / Razor Claw / Lucky Punch (Chansey) / Leek (Farfetch'd) — make the holder's attacks a guaranteed crit on the entry turn (honoring crit-blockers), then are consumed; this *composes* with `DETERMINISTIC_CRITICAL_HITS` to give the crit-item class a purpose again (`IsCriticalHit`). **Flinch items** — King's Rock / Razor Fang — flinch the target on the entry turn (unless the move already flinches), regardless of `flinchedLastTurn`, then are consumed (`TryKingsRock`, `src/battle_hold_effects.c`). Crit/flinch items are consumed at move end via the new `MOVEEND_DETERMINISTIC_HOLD_CONSUME` (`src/battle_move_resolution.c`). Separately, **Starf Berry** raises the holder's currently-highest stat instead of a random one (`RandomStatRaiseBerry`). The AI's OHKO reasoning (`ShouldTryOHKO`, `src/battle_ai_util.c`) and crit/damage prediction are taught the new behavior. **Evasion items (BrightPowder / Lax Incense) are intentionally left alone** here — reserved for a future `DETERMINISTIC_ACCURACY_EVASION`. **Enabled (`TRUE`) in this fork.** Covered by `test/battle/deterministic_hold_effects.c` (opt-in via `WITH_CONFIG`). |
| Shell Bell buff | `BUFF_SHELL_BELL` (+ `BUFF_SHELL_BELL_DENOMINATOR`) | `config/buff.h` | ✅ | First of the `BUFF_*` flags (a project to rebalance items/functionality). Buffs Shell Bell recovery from the stock 1/8 of damage dealt to 1/`BUFF_SHELL_BELL_DENOMINATOR` (1/4 by default) in `TryShellBell()` (`src/battle_hold_effects.c`). Registered boolean toggle (test-flippable) + plain compile-time denominator, mirroring `DETERMINISTIC_DAMAGE`'s toggle+tuning split. **Enabled (`TRUE`) in this fork.** Covered by `test/battle/buff_shell_bell.c` (opt-in via `WITH_CONFIG`); see the fork-flag test harness note below. |
| Color-blind HP bar | `COLOR_BLIND` | `config/accessibility.h` | ✅ | First of a planned accessibility set. Recolors the in-battle HP bar's healthy (>50%) band from green to the EXP bar's blue, so the bands no longer put the confusable colors at the extremes for red-green color blindness (stock ramp is green→yellow→red; new is blue→yellow→red). 2-entry palette swap on `TAG_HEALTHBAR_PAL` entries 10–11 in `ApplyHealthbarColorBlindPalette()` (`src/battle_gfx_sfx_util.c`); the yellow/red bands and the EXP bar are untouched, no new graphics. **Enabled (`TRUE`) in this fork.** Battle bar only for now; the party-menu/summary HP bars use a different (palette-by-index) mechanism and still follow stock colors. |

Legend: ✅ done · ⚠️ partial / has known limitations.

### Determinism (`DETERMINISTIC_*`)

An ongoing project to remove as much RNG from the game as possible, one source
at a time, so outcomes follow from player choices and battle state rather than
luck. Each source gets its own flag in `config/deterministic.h` (`FALSE` = stock
behavior); the fork enables them as they mature. As random upsides are removed,
the plan is to introduce compensating systems so battles stay balanced rather
than simply harder or easier.

**Design intent — this is a single-player game.** These changes are *not* a
competitive-meta rebalance; there's no human opponent, so multiplayer concerns
(how a move like Scald shifts a metagame, EV-neutral tweaks, etc.) are out of
scope. What's being protected is the **single player's experience against the
AI**, above all in the Battle Frontier facilities, where the draw is sustaining a
**long win streak on skill and Pokémon knowledge**. In stock play a strong run
can be cut short by pure hax with no counterplay — a surprise crit, a Quick Claw
flip into a Sheer Cold OHKO, a flinch you never act through. Determinism removes
the unlucky *and* the lucky variance from that loop, so a streak comes down to
decisions rather than dice; the `BUFF_*` line then compensates for the lucky
breaks the player gives up.

Flags so far: `DETERMINISTIC_CRITICAL_HITS`,
`DETERMINISTIC_DAMAGE`, `DETERMINISTIC_PARALYSIS`, `DETERMINISTIC_FLINCH`,
`DETERMINISTIC_ADDITIONAL_EFFECTS`, and `DETERMINISTIC_HOLD_EFFECTS` (all enabled
— see table). The general shape
is that a *random upside* (a lucky crit/burn/flinch) is swapped for something the
player can read off the board: a guaranteed-or-impossible outcome, or one gated
on a legible condition (super effective, STAB, "did they flinch last turn"). The
battle AI is taught each new rule so it keeps valuing moves by what will actually
happen.

**Test harness.** Each `DETERMINISTIC_*` flag is registered into the existing
runtime config system (`DETERMINISTIC_CONFIG_DEFINITIONS` in
`include/constants/config_changes.h`), so engine code reads it via
`GetConfig(DETERMINISTIC_X)` and battle tests toggle it per-test with
`WITH_CONFIG(DETERMINISTIC_X, TRUE)`. The `#define`s in `config/deterministic.h`
remain the production defaults (on in the shipped ROM); the per-test baseline
forces every flag **off** (`TestInitConfigData`), so the entire inherited test
suite keeps running against stock behavior untouched, and only the dedicated
`test/battle/deterministic_*.c` files opt in. This means **adding a new
determinism flag costs one `#define` + one line in
`DETERMINISTIC_CONFIG_DEFINITIONS` + swapping its consumption site(s) to
`GetConfig` — no scattering `#if` guards across the upstream suite.**

### Balance / buffs (`BUFF_*`)

An ongoing project to rebalance items and other game functionality, usually as
compensation for other changes we make (e.g. the `DETERMINISTIC_*` project trades
random upsides away, so some items get buffed to keep battles balanced). Each
tweak gets its own flag in `config/buff.h` (`FALSE` = stock behavior); the fork
enables them as they mature. Flags so far: `BUFF_SHELL_BELL` (enabled — see
table).

`BUFF_*` flags ride the **same runtime config system and test harness** as the
`DETERMINISTIC_*` flags above (`BUFF_CONFIG_DEFINITIONS` in
`include/constants/config_changes.h`): engine code reads each toggle via
`GetConfig(BUFF_X)`, battle tests opt in with `WITH_CONFIG(BUFF_X, TRUE)`, and the
per-test baseline forces every flag **off** (`TestInitConfigData`) so the
inherited suite keeps asserting stock behavior. Where a buff has a *magnitude*
(not just on/off), the toggle is registered but the magnitude is a plain
compile-time constant alongside it — e.g. `BUFF_SHELL_BELL` (registered toggle) +
`BUFF_SHELL_BELL_DENOMINATOR` (tuning), mirroring how `DETERMINISTIC_DAMAGE` pairs
its toggle with `BASE_PERCENT`/`TURN_INCREMENT`. This keeps the boolean test
baseline uniform (`force off` = stock) while leaving the numbers tunable in one
fork-owned file.

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
