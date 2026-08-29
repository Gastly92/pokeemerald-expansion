# Fork changes

This is a fork of [RHH's `pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).
It tracks upstream and layers a set of custom features on top to build a
**standalone single-player romhack** centered on quality-of-life improvements to
the Battle Frontier facilities, growing into a fuller game over time. See
[`CLAUDE.md`](../CLAUDE.md) for the conventions and the upstream-sync process.

**This file is an index, not a spec.** Each row is one line: what the feature does,
its flag, and where to read more. The source of truth for a flag's exact behavior is
its comment in `include/config/*.h`; the source of truth for a subsystem is its own
doc below. A row records only what neither of those can — status, and the pointer.

> **Keep rows to one or two sentences.** If a row needs a paragraph, the detail
> belongs in the relevant doc (or a new one), with the row pointing at it. This file
> is read on a phone.

| Doc | Covers |
|---|---|
| [`DETERMINISM.md`](DETERMINISM.md) | The `DETERMINISTIC_*` project — rationale + per-flag mechanics |
| [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) | Innate abilities, the species ability-override table, per-ability wiring, how to add one |
| [`NEW_ABILITIES.md`](NEW_ABILITIES.md) | Custom abilities (Affinity, Halo) and how to add one |
| [`NEW_TYPES.md`](NEW_TYPES.md) | Re-typing a species |
| [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md) | Converting a facility to 6v6 + endless; the Factory and Tower as worked examples |
| [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md) | The extended roster, the species tier map, and the draft rules |
| [`FREE_GIMMICKS.md`](FREE_GIMMICKS.md) | Item-free Mega/Z/Tera/Dynamax and the gimmick picker |
| [`BATTLE_INFO.md`](BATTLE_INFO.md) | The in-battle INFO viewer and its reveal-gating rules |
| [`LINE_REVIEW.md`](LINE_REVIEW.md) | The per-species-line review playbook (innates, overrides, Factory sets), for a single line or a batch given as a dex-number range |

Legend: ✅ done · ⚠️ partial / has known limitations.

## New game & startup

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Boot straight to the main menu | `SKIP_TITLE_SEQUENCE` | `config/fork.h` | ✅ | Skips copyright/intro/title. RHH intro still plays if `EXPANSION_INTRO`. |
| Trim Prof. Birch's new-game intro | `SKIP_BIRCH_SPEECH` | `config/fork.h` | ✅ | Keeps look + name selection, drops the monologue. |
| Gender-neutral text | `GENDER_NEUTRAL_TEXT` | `config/fork.h` | ✅ | Neutral wording in the new-game look picker; extensible to other gendered strings. Text only — selection unchanged. |
| Start a new game at the Battle Frontier | `START_AT_BATTLE_FRONTIER` | `config/fork.h`, `src/new_game.c` | ✅ | Warps to the Frontier dock as if first arriving (`SetBattleFrontierFirstArrivalState()`); no effect on FRLG. Also sets `FLAG_SYS_POKEMON_GET` so the party menu is reachable with no starter. |
| Battle Frontier facility guide | _(no flag; part of the Frontier-start intro)_ | `BattleFrontier_OutsideWest`/`ReceptionGate` scripts | ✅ | Continues the redirected Scott intro: a greeter escorts the player from the Reception Gate to the Battle Factory door, then becomes a permanent directions-giver with a two-level multichoice (`MULTI_FRONTIER_GUIDE_AREAS`/`_WEST`/`_EAST`). Hand-authored BFS paths avoid NPC home tiles; `lockall` freezes wanderers. Exchange-shop directions not yet listed. |
| New-game option defaults | `NEW_GAME_TEXT_SPEED`, `NEW_GAME_BATTLE_STYLE` | `config/fork.h` | ✅ | Initial text speed (Fast) and battle style (Set) in `SetDefaultOptions()`; the player can still change them. |
| Start with battle-gimmick items | `START_WITH_BATTLE_GIMMICK_ITEMS` | `config/fork.h` | ⚠️ partial | Gives Mega Ring / Z-Power Ring / Dynamax Band / Tera Orb (`GiveStartingBattleGimmickItems()`). Largely moot: `FEATURE_FREE_GIMMICKS` enables all four gimmicks without them, so these matter only in a free-gimmicks-off build. |
| Debug menus always on | `DEBUG_OVERWORLD_MENU`, `DEBUG_BATTLE_MENU` | `config/debug.h` | ✅ | Overworld menu (hold R + Start), battle menu (Select). |

## Battle Frontier

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| 6v6 Battle Frontier | `B_FRONTIER_PARTY_SIZE_6V6` | `config/frontier.h` | ⚠️ partial | Frontier-wide full 6-mon teams in singles **and** doubles; party picker extended to 6 picks; lobby and rules-board text updated. Flag is deliberately `1`/`0`, not `TRUE`/`FALSE` — see [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md) Footgun 1. **Battle Dome layout is not generalized to 6.** |
| Endless Frontier challenge | `B_FRONTIER_ENDLESS` (+ `FRONTIER_STAGES_PER_CHALLENGE`) | `config/frontier.h`, `constants/battle_frontier.h` | ⚠️ partial | Challenges never end: a "set" is `FRONTIER_STAGES_PER_CHALLENGE` (7 → 10) wins, BP is awarded after every win scaling per set, "Rest" saves and returns to the lobby with the streak live, and the Frontier Brain moves to the 50th/100th win. Factory and Tower wired up. Details: [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md). |
| 6v6 + endless Battle Tower | `B_FRONTIER_ENDLESS`, `B_FRONTIER_TOWER_DISABLE_MULTI_LINK` | `config/frontier.h`, `src/battle_tower.c`, `src/fork/battle_tower_trainers.c` | ⚠️ partial | The second facility conversion, Singles + Doubles: roster opponents each bringing a guaranteed Mega, the Salon Maiden at the 50th/100th win, random Hoenn gym-leader bosses on the other set-end wins, and independent per-mode streaks. Multi/Link Multi disabled behind the flag. Details + limitations: [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md) "The Battle Tower". |
| Frontier battles forced to Lv100 | `B_FRONTIER_FORCE_LVL_100` | `config/frontier.h` | ✅ | Factory and Tower force Open Level (both teams at `MAX_LEVEL`): the lobby skips the Lv50/Open multichoice, and the Reception Gate rules guide hides the now-irrelevant level-mode entries (`MULTI_FRONTIER_RULES_LV100`). See Known quirks. |
| Frontier max PP | `B_FRONTIER_MAX_PP` | `config/frontier.h` | ✅ | Maxes the PP Up bonus on all four move slots in `CreateFacilityMon`, the shared facility mon builder — so it covers every facility, plus Battle Tent and multi-battle partners. |
| Frontier max IVs | `B_FRONTIER_MAX_IVS` | `config/frontier.h` | ⚠️ partial | Forces 31 IVs in every stat via `GetFactoryMonFixedIV`, replacing the vanilla per-challenge ramp for rentals, opponents and the Brain. Factory-only; other facilities' IV ramps live elsewhere. |
| Frontier hard AI | `B_FRONTIER_HARD_AI` (+ `B_FRONTIER_HARD_AI_FLAGS`) | `config/frontier.h` | ⚠️ partial | Every Factory opponent and the Brain use the strongest AI preset (default `AI_FLAG_SMART_TRAINER`) instead of the vanilla per-challenge scaling, gated in `GetAiScriptsInBattleFactory`. Battle Tent keeps no AI. Factory-only. |
| Factory swap: opponent summary | `B_FRONTIER_FACTORY_OPP_SUMMARY` | `config/frontier.h`, `src/battle_factory_screen.c` | ✅ | On the post-battle rental-swap screen, selecting an *opponent* mon opens the same Summary/Swap/Rechoose popup the player's mons get, instead of jumping straight to "Accept this Pokémon?" with no way to inspect it. The duplicate-species guard still runs first. |
| Extended frontier roster | `B_FRONTIER_EXTENDED_MONS` | `config/frontier.h`, `src/fork/frontier_extended_mons.c` | ✅ | Fork-owned roster of modern competitive sets replacing `gBattleFrontierMons`, drawn uniformly, with several format-tagged builds per species. Gens I–IX are comprehensively built out, including all regional forms and notable formes, and a coverage test fails if a fully-evolved species has neither a set nor a reasoned exception row. Seven further gates, established by the /line-review sweep and now covering the whole dex: innate-row coverage, pre-evolution coverage, a legal observable slot per drafted species, no ally-hitting spread move on a doubles set, no status move on a Choice set, no damaging move on the stat a set dumped, and no two sets on one species that are the same set. Factory + Tower. **Saved rentals key on array index — appending is save-safe, mid-list edits are not.** Details: [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md); set authoring: [`LINE_REVIEW.md`](LINE_REVIEW.md). |
| Species tier map | _(data; no flag)_ | `src/fork/species_tiers.c` | ✅ | Fork-owned species→tier table (`TIER_MYTHICAL`/`LEGENDARY`/`PSEUDO`/`NORMAL` — this fork's own groupings, not Game Freak's), keyed per forme so a forme is classified on its own merits. Gates the frontier draft. Details: [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md). |
| All species legal in Frontier | `B_FRONTIER_ALL_SPECIES_LEGAL` | `config/frontier.h`, `include/fork/frontier_legality.h` | ✅ | Ignores the per-species `isFrontierBanned` flag, so every species is legal and the "ineligible Pokémon" list is always empty. The upstream reads are funnelled through the `IsSpeciesFrontierBanned()` chokepoint; `gSpeciesInfo` is untouched, so `FALSE` restores the vanilla bans. |
| Disable Frontier battle recording | `B_FRONTIER_DISABLE_RECORD_BATTLE` | `config/frontier.h`, `src/frontier_util.c` | ✅ | No attendant offers to record a battle. Playback re-simulates from a saved seed + inputs, which the expansion's far more stateful battle engine doesn't reproduce reliably (a long-standing **upstream** fragility — those files are stock), so replays desync into a glitched battle. Forces `FRONTIER_DATA_RECORD_DISABLED`; the recording code is left intact, so flipping this back revives it. |
| In-battle INFO viewer | `B_FRONTIER_BATTLE_INFO` | `config/frontier.h`, `src/fork/frontier_battle_info.c` | ⚠️ partial | In facilities where the bag is disabled, the BAG slot (and SELECT in the battle party menu) opens a read-only five-page reference: Speed Tiers, Field, Conditions, Stat Changes, Foe. Strictly reveal-gated and Illusion-safe; stock assets only. Details: [`BATTLE_INFO.md`](BATTLE_INFO.md). Foe page reads opponent A only. |

## Determinism (`DETERMINISTIC_*`)

An ongoing project to remove as much RNG from the game as possible, one source at a
time, so outcomes follow from player choices and battle state rather than luck. The
general shape is that a *random upside* (a lucky crit/burn/flinch) is swapped for
something the player can read off the board: a guaranteed-or-impossible outcome, or
one gated on a legible condition (super effective, STAB, "did they flinch last
turn"). The AI is taught each new rule. As random upsides are removed, the `BUFF_*`
line compensates so battles stay balanced rather than simply harder.

Each source gets its own flag in `config/deterministic.h` (`FALSE` = stock). **All
ten below are enabled.** Full rationale and per-flag mechanics:
**[`DETERMINISM.md`](DETERMINISM.md)**.

| Feature | Flag(s) | Status | Notes |
|---|---|---|---|
| Deterministic critical hits | `DETERMINISTIC_CRITICAL_HITS` | ✅ | Removes the random crit-chance roll (`IsCriticalHit`) — crits land only when *guaranteed* (always-crit moves, Laser Focus, Merciless vs. a poisoned target, 1/1 crit stage); crit-blockers unchanged, AI crit valuations made config-aware. [Mechanics](DETERMINISM.md#deterministic_critical_hits) · `test/fork/deterministic_critical_hits.c` |
| Deterministic damage | `DETERMINISTIC_DAMAGE` (+ `…_BASE_PERCENT`, `…_TURN_INCREMENT`) | ✅ | Replaces the random 85%–100% damage roll with a fixed multiplier scaling with the turn count (default 92%, +1%/turn, **intentionally uncapped**); the AI is fed the same figure. [Mechanics](DETERMINISM.md#deterministic_damage) · `test/fork/deterministic_damage.c` |
| Deterministic flinch | `DETERMINISTIC_FLINCH` | ✅ | Anti flinch-lock cap: a gated flinch can't be re-applied to a foe that flinched last turn (Fake Out / guaranteed flinches exempt). Required by the additional-effects flag so its gated flinches can't lock. [Mechanics](DETERMINISM.md#deterministic_flinch) · `test/fork/deterministic_flinch.c` |
| Deterministic additional effects | `DETERMINISTIC_ADDITIONAL_EFFECTS` | ✅ | A move's sub-100% secondary effect lands on a fixed condition instead of a roll: a super-effective hit (types that *can* be SE) or a STAB user (Normal). Serene Grace / Pledge Rainbow make it *certain*. [Mechanics](DETERMINISM.md#deterministic_additional_effects) · `test/fork/deterministic_additional_effects.c` |
| Deterministic paralysis | `DETERMINISTIC_PARALYSIS` (+ `…_PP_TAX`, `…_PRIORITY_TAX`) | ✅ | Drops the 25% full-paralysis miss and the Speed cut; instead every move costs +1 PP and −1 priority, keeping full Speed (Quick Feet exempt). AI turn-order prediction tracks it for free. [Mechanics](DETERMINISM.md#deterministic_paralysis) · `test/fork/deterministic_paralysis.c` |
| Deterministic hold effects | `DETERMINISTIC_HOLD_EFFECTS` | ✅ | Chance-to-trigger hold items become guaranteed one-shot **entry items** (Focus Band, Quick Claw, crit items, King's Rock, Starf Berry). Blunder Policy instead arms on any **blunder** — Protect, semi-invulnerability, a type/ability/Air Balloon immunity — since the accuracy flag makes its stock miss trigger unreachable. [Mechanics](DETERMINISM.md#deterministic_hold_effects) · `test/fork/deterministic_hold_effects.c` |
| Deterministic accuracy/evasion | `DETERMINISTIC_ACCURACY_EVASION` (+ `…_OHKO_MAX_HP_PERCENT`, `…_EXTRA_MISS_COST_PERCENT`) | ✅ | Accuracy/evasion become a **PP economy** rather than a coin flip: moves always hit, sub-100% moves get reduced max PP plus a per-use stage tax/refund and flat item/ability taxes (No Guard exempt). OHKO moves deal 40% max HP; sub-100% sleep moves cause drowsiness; 50%-accuracy moves gain a recharge turn. [Mechanics](DETERMINISM.md#deterministic_accuracy_evasion) · `test/fork/deterministic_accuracy_evasion.c` |
| Deterministic abilities | `DETERMINISTIC_ABILITIES` | ⚠️ partial | Contact statuses (Static, Flame Body, Poison Point/Touch, Cute Charm, Toxic Chain, Cursed Body) always attempt; Effect Spore → −1 accuracy on the contact attacker (PP tax, not a status); Stench / Quick Draw fire first-turn-only; Moody, Pickup, Trace, Forewarn, Rivalry made state-based. **Overworld ability RNG is out of scope** (deferred to the Pyramid rework). [Mechanics](DETERMINISM.md#deterministic_abilities) · `test/fork/deterministic_abilities.c` |
| Deterministic status | `DETERMINISTIC_STATUS` (+ `…_INFATUATION_TURNS`, `…_INFATUATION_DMG_PERCENT`, `…_SLEEP_TURNS`) | ✅ | Infatuation drops the gender requirement and the coin flip (reduced damage for a fixed duration instead); sleep lasts a fixed 3 turns, counted only on acting turns, costing the target two actions; confusion becomes one guaranteed 40-BP self-hit that doesn't deny the action. [Mechanics](DETERMINISM.md#deterministic_status) · `test/fork/deterministic_status.c` |
| Deterministic move results | `DETERMINISTIC_MOVE_RESULTS` (+ multi-hit / rampage / wrap / Present tuning) | ✅ | Multi-hit → 3 (5 with Skill Link/Loaded Dice), Population Bomb → 5/10, rampage 2 turns, binding 4/7, speed ties by a fixed Speed→weight→HP% ladder, consecutive Protect always fails; Tri Attack, Dire Claw, Magnitude, Present, Fickle Beam, Shell Side Arm and phazing made state-based. [Mechanics](DETERMINISM.md#deterministic_move_results) · `test/fork/deterministic_move_results.c` |

## Balance / buffs (`BUFF_*`)

An ongoing project to rebalance items and other functionality, usually as
compensation for other changes — the `DETERMINISTIC_*` project trades random upsides
away, so some items get buffed to keep battles balanced. Each tweak gets its own flag
in `config/buff.h` (`FALSE` = stock). Both below are enabled.

| Feature | Flag(s) | Status | Notes |
|---|---|---|---|
| Shell Bell buff | `BUFF_SHELL_BELL` (+ `…_DENOMINATOR`) | ✅ | Recovery from 1/8 of damage dealt to 1/`BUFF_SHELL_BELL_DENOMINATOR` (1/4 by default) in `TryShellBell()`. `test/fork/buff_shell_bell.c` |
| Leech Seed buff | `BUFF_LEECH_SEED` (+ `…_DENOMINATOR`) | ✅ | Two changes: seeds **stack** (several battlers can seed one target, each draining independently), and re-using Leech Seed on a foe you already seed deals an **immediate 1/8 drain** instead of failing. The tick fraction stays at vanilla 1/8 — the buff is the stacking, not a bigger tick. Storage is additive: upstream's single-seeder `leechSeed` volatile stays the "primary", with a fork `leechSeededBy` bitmask holding the full set. `test/fork/buff_leech_seed.c` |
| Accuracy items buff | `BUFF_ACCURACY_ITEMS` | ✅ | `DETERMINISTIC_ACCURACY_EVASION` left Wide Lens and Zoom Lens completely inert — they only scale an accuracy figure `DoesMoveMissTarget()` no longer reads — while the defender's BrightPowder half kept working. They now cancel the attacker's accuracy-axis PP penalty instead: Wide Lens the flat evasion taxes, Zoom Lens those plus the whole stat-stage half (the target's evasion boosts **and** the holder's own accuracy drops, since it reuses the No Guard / Micle `ignorePenalties` path) while it moves second. Pure boon — never refunds below the base 1 PP. `GetAccuracyItemRelief()` in `src/fork/deterministic_moves.c`; `test/fork/buff_accuracy_items.c`, plus a roster test that fails if a set holds a lens its own ability makes redundant. |
| Accuracy items: reveal | `BUFF_ACCURACY_ITEMS_REVEAL` | ✅ | The lenses also act as instruments for *seeing*, feeding the INFO viewer's reveal bits: Wide Lens is **breadth** (every seen foe's held item), Zoom Lens is **depth** (one foe's ability and full moveset, once it has been watched using a move). [Mechanics](BATTLE_INFO.md#held-item-and-depth-reveals-from-a-lens-buff_accuracy_items_reveal). `test/fork/buff_accuracy_items.c` |

## Abilities, types & gimmicks

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Innate abilities | `FEATURE_INNATE_ABILITIES` | `config/feature.h`, `src/fork/innate_abilities.c` | ✅ | Some species gain one or more **innate** abilities, always active *in addition to* their chosen ability, from a fork-owned table (never `gSpeciesInfo`). An innate answers `BattlerHasAbility()` but is never the battler's copyable/swappable identity. **Design principle: an innate is a pure boon** — where the real ability has a downside, the innate drops it. Allowlist-gated, grown one ability at a time; picks are keyed per form. Shown on the summary "Innates" page and in the INFO viewer. Details: [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md). `test/fork/innate_abilities.c` |
| Species ability overrides | `FEATURE_INNATE_ABILITIES` | `src/fork/species_ability_overrides.c` | ✅ | Fork-owned `{species, slot, ability}` table consulted by `GetSpeciesAbility()` — the accessor every ability read funnels through — so an override behaves like a real ability everywhere **without editing `gSpeciesInfo`**. It exists to give a real slot to species whose only real abilities are now innate. A chosen ability must be a **never-an-innate** pick, and must not be one of the **reserved** abilities welded to a single line (Illusion, Zorua/Zoroark); both rules are CI gates, not conventions. Details: [`INNATE_ABILITIES.md#direction`](INNATE_ABILITIES.md). |
| New types | `FEATURE_NEW_TYPES` | `config/feature.h`, `src/fork/new_types.c` | ✅ | Overwrites selected species' types from a fork-owned table (never `gSpeciesInfo`), applied at the single `GetSpeciesType()` accessor so the re-typing flows to matchups, STAB, type icons, the Pokédex and the summary from one hook. Type identity only — movepool/abilities/learnset unchanged. First entry: Galarian Ponyta and Rapidash → Fire/Fairy. Details: [`NEW_TYPES.md`](NEW_TYPES.md). `test/fork/new_types.c` |
| Custom abilities (Affinity family) | _(data; no flag)_ | `src/fork/type_affinity.c` | ✅ | Fork abilities in the reserved post-Gen-9 id block. An **Affinity** grants its holder a **latent third type in battle** — that type's STAB and resistances *plus* its weaknesses — injected into an empty third slot at the `GetBattlerTypes()` chokepoint, so the AI predicts it for free. The built-in downside is why it's a chosen ability and never an innate. `Psychic Affinity` on Butterfree and Venomoth (plus the paired G-Max row). Details: [`NEW_ABILITIES.md`](NEW_ABILITIES.md). |
| Custom abilities (Halo) | _(data; no flag)_ | `src/fork/halo.c` | ✅ | An **aura**, not a personal trait: while its holder is on the field, no single hit may take more than `HALO_DAMAGE_CAP_PERCENT` (40) of **any** battler's max HP — so nothing on the field dies in fewer than three hits, the holder's own attacks included. The holder alone pays: `HALO_PP_TAX` (+1 PP per move). Fixed-damage moves and multi-hit moves are deliberate counterplay. On Mega Clefable (all three slots) and Wigglytuff. Details: [`NEW_ABILITIES.md`](NEW_ABILITIES.md). |
| Item-free battle gimmicks | `FEATURE_FREE_GIMMICKS` | `config/feature.h`, `src/battle_gimmick.c` | ✅ | Drops the held-item, key-item and charge requirements for Mega, Z-Moves, Tera and Dynamax, and gives each mon a Start-key **picker** among the gimmicks it can currently use. One gimmick per mon and once per type per trainer still apply, so a team's mons *compete* for the slot. X/Y Megas are chosen by Attack vs Sp. Atk. The AI uses everything and picks among its candidates. Details: [`FREE_GIMMICKS.md`](FREE_GIMMICKS.md); set-building consequences: [`LINE_REVIEW.md`](LINE_REVIEW.md). `test/fork/free_gimmicks.c` |

## Battle AI

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Species-aware AI overrides | `AI_FLAG_SMART_SPECIES_LOGIC` | `src/fork/battle_ai_species_overrides.c` | ⚠️ partial | Opt-in AI flag (bit 34) patching spots where the generic AI misplays specific mons; OR'd into `B_FRONTIER_HARD_AI_FLAGS`. All logic lives in the fork module, reached from three small gated hooks (a switch veto in `ShouldSwitch`, a move-score entry, a Mega-defer in `ReconsiderGimmick`). Covers **Palafin** (don't voluntarily pivot out a transformed Hero form) and **Sharpedo** (Protect turn one to bank a Speed Boost, then Mega — only when the Mega form *loses* the ability). Extend by adding cases to the module. `test/fork/ai_smart_species_logic.c` |
| Deliberate AI Z-Move usage | `AI_FLAG_SMART_Z_MOVE` | `src/fork/battle_ai_zmove.c` | ✅ | Opt-in per trainer, mirroring upstream's `AI_FLAG_SMART_TERA`. Upstream's `ShouldUseZMove` says yes to any damaging move the plain move wouldn't KO with, so the AI burns its one Z-Move on turn one. With the flag, `AI_ShouldSpendZMove` spends it only to secure a KO this turn, or when the user is about to faint anyway — merely needing fewer hits isn't enough. Also fixes two unconditional fall-throughs in the status-move branch (which spent the Z-Move after judging the boost not worth it); those apply with or without the flag. Not part of `AI_FLAG_SMART_TRAINER`. `test/fork/ai_zmove_selection.c` |
| Z-Move base power fix | _(no flag; always compiled)_ | `src/battle_z_move.c`, `src/battle_util.c` | ✅ | `GetZMoveBasePower()` is the single source of truth for Z-Move power, shared by the move-selection preview and the damage calc so the number the menu advertises can't drift from the number dealt. Upstream's `ctx->baseMove` (added in #10616) now covers the ordinary path, but it still derives *every* Z-Move's power from the base move via the tier table — wrong for signature Z-Moves, which carry their own power — and it holds the Z-Move rather than the base move on the called-move path, so the fork keeps its `zmove.baseMoves[]` recovery there. `test/fork/zmove_power.c` |
| AI gimmick type matchups | _(no flag; always compiled)_ | `src/fork/battle_ai_gimmick.c`, `src/battle_ai_util.c` | ✅ | The AI now scores a gimmick move's type matchup off the move the engine will really execute, since a Z-Move or Max Move keeps its base move's type but none of its matchup quirks — Freeze-Dry read as 2x against Water while Subzero Slammer / Max Hailstorm really land for 0.5x, a 4x error that made the AI upgrade into resists. Also covers Flying Press, Thousand Arrows and Synchronoise. `test/fork/ai_gimmick_effectiveness.c` |
| -ate abilities convert Max Moves | _(no flag; always compiled)_ | `src/battle_main.c`, `src/battle_controller_player.c` | ✅ | Pixilate/Refrigerate/Aerilate/Galvanize/Dragonize now retype a Max Move as the games do, so Pixilate Sylveon's Hyper Voice is Max Starfall both in the picker and when it fires — upstream skipped the -ate branch while Dynamax was active, and the menu previews with the gimmick only *armed*, so the two disagreed. The 20% -ate boost stays off under Dynamax, matching the Normalize branch beside it. The move-selection effectiveness readout also follows the armed Z-Move's (unconverted) type. `test/fork/ate_max_moves.c` |

## UI & accessibility

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Color-blind HP bar | `COLOR_BLIND` | `config/accessibility.h` | ✅ | First of a planned accessibility set. Recolors the in-battle HP bar's healthy (>50%) band from green to the EXP bar's blue, so the ramp is blue→yellow→red rather than putting the confusable colors at the extremes. A 2-entry palette swap, no new graphics. Battle bar only — the party-menu/summary bars use a different mechanism and still follow stock colors. |
| Clean battle healthboxes | `B_CLEAN_HEALTHBOX` | `config/battle.h`, `src/battle_interface.c` | ✅ | Declutters the healthboxes two ways: **(1)** removes the corner pointer/tail that overlaps the on-healthbox type icons from `B_SHOW_TYPES` — it's baked into the box sprite art, so the flag swaps in fork-owned `*_notail.png` variants; **(2)** removes the singles player box's EXP bar and its wrap-around frame. HP bar, level/HP text, status and nick untouched. Vanilla art/behavior fully preserved at `FALSE`. |
| Illusion-safe effectiveness readout | `B_SHOW_EFFECTIVENESS` (fix; no new flag) | `src/battle_controller_player.c` | ✅ | Stops the move-select effectiveness indicator from **leaking an Illusion disguise** — `CheckTypeEffectiveness` called the real damage calc and surfaced e.g. Zoroark's Dark weaknesses through the disguise. Now computed from the *apparent* species' base types via `CalcIllusionTypeEffectiveness`, falling back to the true calc once the Illusion breaks. **Known limitation:** pure type-chart only, so it deliberately ignores the hidden mon's ability/item and move-specific overrides (Freeze-Dry, Flying Press). `FORK:`-tagged. |

## The fork-flag test harness

The `DETERMINISTIC_*`, `BUFF_*` and `FEATURE_*` flags all ride one mechanism, and
this is the note the other docs point back to.

Each flag is registered into the runtime config system (`DETERMINISTIC_CONFIG_DEFINITIONS`
/ `BUFF_CONFIG_DEFINITIONS` / `FEATURE_CONFIG_DEFINITIONS` in
`include/constants/config_changes.h`), so engine code reads it via `GetConfig(X)` and
battle tests toggle it per-test with `WITH_CONFIG(X, TRUE)`.

The `#define`s in `config/*.h` remain the production defaults (on in the shipped ROM),
while **the per-test baseline forces every flag off** (`TestInitConfigData`). That is
what keeps the entire inherited upstream test suite running against stock behavior
untouched — only the dedicated `test/fork/*.c` files opt in. It is also what lets the
species ability-override table repurpose a slot an upstream test pins: that test never
sees the override.

So **adding a new flag costs one `#define` + one line in the relevant
`*_CONFIG_DEFINITIONS` + swapping its consumption site(s) to `GetConfig`** — no
scattering `#if` guards across the upstream suite.

Where a flag has a *magnitude* rather than just on/off, the toggle is registered but
the magnitude is a plain compile-time constant alongside it — e.g. `BUFF_SHELL_BELL`
(registered) + `BUFF_SHELL_BELL_DENOMINATOR` (tuning), mirroring how
`DETERMINISTIC_DAMAGE` pairs its toggle with `BASE_PERCENT`/`TURN_INCREMENT`. This
keeps the boolean test baseline uniform (force off = stock) while leaving the numbers
tunable in one fork-owned file.

The `B_FRONTIER_*` flags are the exception: they are plain `#if` compile-time flags,
not registered.

## Known quirks / future work

- **Forced Lv100 leaves dual record tracking (`B_FRONTIER_FORCE_LVL_100`).** With only
  Open Level in use, the always-empty "Lv 50" block is dropped from the **Factory** and
  **Tower** results windows (`src/frontier_util.c`). The remaining facilities' record
  windows still show both blocks. The save block also still persists Lv50 records that
  can no longer change — a candidate to drop for space if needed.
- **Battle Dome at 6v6.** Its fixed 3-mon coordinate tables aren't generalized, so its
  layout is wrong at 6 mons. The tables are stubbed so it still builds.
- **Bumped `FRONTIER_STAGES_PER_CHALLENGE` (7 → 10) is global.** Per review, the shared
  constant was changed rather than adding a Factory-specific one, so every facility now
  sees 10-stage challenges. Their fixed-size layout tables (Battle Pyramid floor/pickup
  offsets, floor-name arrays) still have 7 entries, so those facilities are off until
  generalized — accepted for now, to be fixed alongside their endless conversion.
- **Dead "won challenge" lobby path.** With `B_FRONTIER_ENDLESS` on, the lobby's
  `CHALLENGE_STATUS_WON` handler is unreachable for new runs (it only fires for a save
  left mid-WON by a pre-update build, or with the flag off). Kept for compatibility; a
  later cleanup can remove it.
- **Only the Factory and Tower are converted.** Per-facility status, what's left, and
  the repeatable conversion pattern are in [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md)
  — read it before converting Palace/Arena/Dome/Pyramid/Pike.

## CI / infrastructure

- **Hardened `Install binutils` CI step.** Upstream installs the GBA toolchain with an
  inline `apt-get` step copy-pasted into each job, which hung intermittently on
  dpkg-lock/mirror stalls. We extracted it into a fork-owned composite action,
  [`.github/actions/install-binutils`](../.github/actions/install-binutils/action.yml),
  that **caches the ~107 MB of toolchain `.deb`s** (the actual root cause — jobs now
  install from GitHub's cache rather than the slow Ubuntu mirror) and wraps each apt
  invocation in a per-attempt `timeout` with backoff retries, so a stall is killed and
  **auto-retried in-job**, failing only after 3 attempts. A `timeout-minutes` backstop
  on each call site is the outer net. Every job in `build.yml` and `rom-artifact.yml`
  calls it via `uses:`. The call sites in `build.yml` are `FORK:`-tagged — on conflict,
  keep our `uses:` + `timeout-minutes` and port any upstream apt-package change into
  the action's package list rather than re-inlining the step.
- **`test/battle/front_anim.c` "Front anims work" is quarantined (`TO_DO`).** Upstream's
  test plays every species' front-pic animation against a shiny wild opponent under the
  headless runner. Something in that path writes out of bounds — upstream's EWRAM layout
  absorbs it, but ours puts `gHeap` there, so it corrupts a malloc block header and hangs
  the whole suite on an illegal-opcode loop (CI's `test` job then times out after ~1h).
  Diagnosed with a temporary heap-walk in `malloc.c`'s `FreeInternal`; skipping just the
  animation callback did **not** fix it (the OOB is in the test's setup), so the whole
  test is stubbed. This is a latent **upstream** bug worth reporting. On a sync conflict,
  keep the `TO_DO_BATTLE_TEST` stub; restore the real test only once upstream fixes the
  OOB (original body is in git history at the sync merge).

## Conventions

- Intentional divergences from upstream inside upstream-owned files are tagged `FORK:`
  (greppable: `grep -rn "FORK:" src include .github`).
- Behavior-preserving cleanups worth contributing back are tagged `UPSTREAM:`.
- Prefer adding behavior behind a config flag in `include/config/*.h` over patching core
  logic, so changes land in files we own and survive upstream syncs cleanly. See
  `CLAUDE.md` for the full rationale.
