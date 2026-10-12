# Fork changes

This is a fork of [RHH's `pokeemerald-expansion`](https://github.com/rh-hideout/pokeemerald-expansion).
It tracks upstream and layers custom features on top to build a **standalone
single-player romhack** centered on quality-of-life improvements to the Battle
Frontier. See [`CLAUDE.md`](../CLAUDE.md) for the conventions and the upstream-sync
process.

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
| [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md) | Converting a facility to 6v6 + endless, per-facility status, the Factory and Tower as worked examples |
| [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md) | The extended roster, the species tier map, and the draft rules |
| [`HELD_ITEMS.md`](HELD_ITEMS.md) | Held-item usage across the roster and the audit behind each item's place |
| [`FREE_GIMMICKS.md`](FREE_GIMMICKS.md) | Item-free Mega/Z/Tera/Dynamax and the gimmick picker |
| [`BATTLE_INFO.md`](BATTLE_INFO.md) | The in-battle INFO viewer and its reveal-gating rules |
| [`DAMAGE_PREVIEW.md`](DAMAGE_PREVIEW.md) | The move menu's damage-range readout: what the range spans and what it hides |
| [`LINE_REVIEW.md`](LINE_REVIEW.md) | The per-species-line review playbook (innates, overrides, Factory sets) |

Legend: ✅ done · ⚠️ partial / has known limitations.

## New game & startup

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Boot straight to the main menu | `SKIP_TITLE_SEQUENCE` | `config/fork.h` | ✅ | Skips copyright/intro/title. RHH intro still plays if `EXPANSION_INTRO`. |
| Trim Prof. Birch's new-game intro | `SKIP_BIRCH_SPEECH` | `config/fork.h` | ✅ | Keeps look + name selection, drops the monologue. |
| Gender-neutral text | `GENDER_NEUTRAL_TEXT` | `config/fork.h` | ✅ | Neutral wording in the new-game look picker. Text only — selection unchanged. |
| Start a new game at the Battle Frontier | `START_AT_BATTLE_FRONTIER` | `config/fork.h`, `src/new_game.c` | ✅ | Warps to the Frontier dock as if first arriving, with the party menu reachable before any Pokémon is obtained. No effect on FRLG. |
| Battle Frontier facility guide | _(no flag; part of the Frontier-start intro)_ | `BattleFrontier_OutsideWest`/`ReceptionGate` scripts | ✅ | A greeter escorts the player to the Battle Factory door, then stays as a directions-giver for every facility. Exchange-shop directions not yet listed. |
| New-game option defaults | `NEW_GAME_TEXT_SPEED`, `NEW_GAME_BATTLE_STYLE` | `config/fork.h` | ✅ | Starts at Fast text and Set battle style; the player can still change them. |
| Start with battle-gimmick items | `START_WITH_BATTLE_GIMMICK_ITEMS` | `config/fork.h` | ⚠️ partial | Gives Mega Ring / Z-Power Ring / Dynamax Band / Tera Orb. Moot while `FEATURE_FREE_GIMMICKS` is on. |
| Debug menus always on | `DEBUG_OVERWORLD_MENU`, `DEBUG_BATTLE_MENU` | `config/debug.h` | ✅ | Overworld menu (hold R + Start), battle menu (Select). |

## Battle Frontier

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| 6v6 Battle Frontier | `B_FRONTIER_PARTY_SIZE_6V6` | `config/frontier.h` | ⚠️ partial | Full 6-mon teams in singles and doubles, Frontier-wide. **Battle Dome layout is not generalized to 6.** [Details](FRONTIER_ENDLESS.md) |
| Endless Frontier challenge | `B_FRONTIER_ENDLESS` (+ `FRONTIER_STAGES_PER_CHALLENGE`) | `config/frontier.h`, `constants/battle_frontier.h` | ⚠️ partial | Challenges never end: BP after every win, "Rest" saves with the streak live, and the Frontier Brain at the 50th/100th win. Factory and Tower only. [Details](FRONTIER_ENDLESS.md) |
| 6v6 + endless Battle Tower | `B_FRONTIER_ENDLESS`, `B_FRONTIER_TOWER_DISABLE_MULTI_LINK` | `config/frontier.h`, `src/battle_tower.c`, `src/fork/battle_tower_trainers.c` | ⚠️ partial | Singles + Doubles with Mega-carrying opponents, the Salon Maiden and gym-leader bosses; Multi/Link Multi disabled. [Details + limitations](FRONTIER_ENDLESS.md#the-battle-tower--the-worked-reference-conversion) |
| Frontier battles forced to Lv100 | `B_FRONTIER_FORCE_LVL_100` | `config/frontier.h` | ✅ | Factory and Tower force Open Level and hide the Lv50 options. See Known quirks. |
| Frontier max PP | `B_FRONTIER_MAX_PP` | `config/frontier.h` | ✅ | Maxes PP Ups on every facility mon, including Battle Tent and multi partners. |
| Frontier prefers Return | `B_FRONTIER_PREFER_RETURN` | `config/frontier.h` | ✅ | Facility mons carry Return instead of Frustration and keep max friendship, swapped-in Factory rentals included. |
| Frontier max IVs | `B_FRONTIER_MAX_IVS` | `config/frontier.h` | ⚠️ partial | 31 IVs in every stat for rentals, opponents and the Brain. Factory-only. |
| Frontier AI difficulty tiers | `B_FRONTIER_HARD_AI` (+ `…_HARD_AI_FLAGS`, `…_REGULAR_AI_FLAGS`) | `config/frontier.h`, `src/fork/frontier_ai.c` | ✅ | AI strength is picked by opponent role: Brains and Tower bosses get the strongest preset, regular opponents one tier below. `test/fork/frontier_ai_difficulty.c` |
| Factory swap: opponent summary | `B_FRONTIER_FACTORY_OPP_SUMMARY` | `config/frontier.h`, `src/battle_factory_screen.c` | ✅ | The rental-swap screen lets you open an opponent mon's summary before taking it. |
| Extended frontier roster | `B_FRONTIER_EXTENDED_MONS` | `config/frontier.h`, `src/fork/frontier_extended_mons.c` | ✅ | Modern competitive sets for Gens I–IX, drawn per National Dex number under a Species Clause and CI-gated for coverage and set sanity. **Saved rentals key on array index — append only.** [Details](FRONTIER_ROSTER.md) · [set authoring](LINE_REVIEW.md) |
| Species tier map | _(data; no flag)_ | `src/fork/species_tiers.c` | ✅ | This fork's per-forme tier groupings, which gate the frontier draft. [Details](FRONTIER_ROSTER.md) |
| All species legal in Frontier | `B_FRONTIER_ALL_SPECIES_LEGAL` | `config/frontier.h`, `include/fork/frontier_legality.h` | ✅ | Every species is Frontier-legal; `FALSE` restores the vanilla bans. |
| Disable Frontier battle recording | `B_FRONTIER_DISABLE_RECORD_BATTLE` | `config/frontier.h`, `src/frontier_util.c` | ✅ | No battle-recording offer, since replays desync on the expansion engine (an upstream fragility). The recording code is intact. |
| In-battle INFO viewer | `B_FRONTIER_BATTLE_INFO` | `config/frontier.h`, `src/fork/frontier_battle_info.c` | ⚠️ partial | The BAG slot opens a read-only, reveal-gated reference: speed tiers, field, stats, battle log, foe, base stats, innates. Foe page reads opponent A only. [Details](BATTLE_INFO.md) |
| Move damage preview | `B_MOVE_DAMAGE_PREVIEW` | `config/fork.h`, `src/fork/damage_preview.c` | ✅ | The move menu shows the selected move's damage as a % of the foe's HP and the type it is fired as, with a KO verdict, using only what the player has seen and an armed Mega form. [Details](DAMAGE_PREVIEW.md) · `test/fork/damage_preview.c` |

## Determinism (`DETERMINISTIC_*`)

Removes RNG one source at a time, replacing each random upside with something the
player can read off the board; the AI is taught each rule. Each flag lives in
`config/deterministic.h` (`FALSE` = stock), and **all ten are enabled.** Rationale
and mechanics: **[`DETERMINISM.md`](DETERMINISM.md)**.

| Feature | Flag(s) | Status | Notes |
|---|---|---|---|
| Deterministic critical hits | `DETERMINISTIC_CRITICAL_HITS` | ✅ | Crits land only when guaranteed. [Mechanics](DETERMINISM.md#deterministic_critical_hits) · `test/fork/deterministic_critical_hits.c` |
| Deterministic damage | `DETERMINISTIC_DAMAGE` (+ `…_BASE_PERCENT`, `…_TURN_INCREMENT`) | ✅ | A fixed damage multiplier that rises with the turn count replaces the 85–100% roll. [Mechanics](DETERMINISM.md#deterministic_damage) · `test/fork/deterministic_damage.c` |
| Deterministic flinch | `DETERMINISTIC_FLINCH` | ✅ | A foe that flinched last turn can't be flinched again (Fake Out exempt). [Mechanics](DETERMINISM.md#deterministic_flinch) · `test/fork/deterministic_flinch.c` |
| Deterministic additional effects | `DETERMINISTIC_ADDITIONAL_EFFECTS` | ✅ | Secondary effects land on a fixed condition (super-effective, or STAB for Normal) instead of a roll. [Mechanics](DETERMINISM.md#deterministic_additional_effects) · `test/fork/deterministic_additional_effects.c` |
| Deterministic paralysis | `DETERMINISTIC_PARALYSIS` (+ `…_PP_TAX`, `…_PRIORITY_TAX`) | ✅ | No full-paralysis miss or Speed cut; moves cost +1 PP and −1 priority instead. [Mechanics](DETERMINISM.md#deterministic_paralysis) · `test/fork/deterministic_paralysis.c` |
| Deterministic hold effects | `DETERMINISTIC_HOLD_EFFECTS` | ✅ | Chance-based items become guaranteed one-shot entry items. [Mechanics](DETERMINISM.md#deterministic_hold_effects) · `test/fork/deterministic_hold_effects.c` |
| Deterministic accuracy/evasion | `DETERMINISTIC_ACCURACY_EVASION` (+ `…_OHKO_MAX_HP_PERCENT`, `…_EXTRA_MISS_COST_PERCENT`) | ✅ | Moves always hit; accuracy and evasion become a PP economy. [Mechanics](DETERMINISM.md#deterministic_accuracy_evasion) · `test/fork/deterministic_accuracy_evasion.c` |
| Deterministic abilities | `DETERMINISTIC_ABILITIES` | ⚠️ partial | Chance-based battle abilities become always-on or state-based. **Overworld ability RNG is out of scope.** [Mechanics](DETERMINISM.md#deterministic_abilities) · `test/fork/deterministic_abilities.c` |
| Deterministic status | `DETERMINISTIC_STATUS` (+ `…_INFATUATION_TURNS`, `…_INFATUATION_DMG_PERCENT`, `…_SLEEP_TURNS`) | ✅ | Fixed-length sleep and infatuation, and confusion as one guaranteed self-hit. [Mechanics](DETERMINISM.md#deterministic_status) · `test/fork/deterministic_status.c` |
| Deterministic move results | `DETERMINISTIC_MOVE_RESULTS` (+ multi-hit / rampage / wrap / Present tuning) | ✅ | Fixed hit counts and durations, a fixed speed-tie ladder, and state-based random moves. [Mechanics](DETERMINISM.md#deterministic_move_results) · `test/fork/deterministic_move_results.c` |

## Balance / buffs (`BUFF_*`)

Rebalances items and mechanics, mostly to compensate for the random upsides
`DETERMINISTIC_*` removed. Each flag lives in `config/buff.h` (`FALSE` = stock), with
its full reasoning in the flag comment; all are enabled. The held-item audit and its
CI tracker are in [`HELD_ITEMS.md`](HELD_ITEMS.md).

| Feature | Flag(s) | Status | Notes |
|---|---|---|---|
| Shell Bell buff | `BUFF_SHELL_BELL` (+ `…_DENOMINATOR`) | ✅ | Heals 1/4 of damage dealt instead of 1/8. `test/fork/buff_shell_bell.c` |
| Leech Seed buff | `BUFF_LEECH_SEED` (+ `…_DENOMINATOR`) | ✅ | Seeds stack across seeders, and re-seeding drains immediately instead of failing. `test/fork/buff_leech_seed.c` |
| Accuracy items buff | `BUFF_ACCURACY_ITEMS` | ✅ | Wide Lens and Zoom Lens cancel accuracy PP taxes, since `DETERMINISTIC_ACCURACY_EVASION` had left them inert. `test/fork/buff_accuracy_items.c` |
| Accuracy items: reveal | `BUFF_ACCURACY_ITEMS_REVEAL` | ✅ | Wide Lens reveals every foe's item and Zoom Lens one foe's ability and moveset in the INFO viewer. [Mechanics](BATTLE_INFO.md#held-item-and-depth-reveals-from-a-lens-buff_accuracy_items_reveal) · `test/fork/buff_accuracy_items.c` |
| Type-boost items buff | `BUFF_TYPE_BOOST_ITEMS` (+ `…_PERCENT`) | ✅ | Type items and Plates boost +40% instead of +20%. `test/fork/buff_type_boost_items.c` |
| Gems buff | `BUFF_GEMS` (+ `…_PERCENT`) | ✅ | Gems boost +60% instead of +30%. `test/fork/buff_gems.c` |
| Flat-HP items buff | `BUFF_FLAT_HP_ITEMS` (+ `…_DENOMINATOR`) | ✅ | Oran Berry and Berry Juice heal 1/4 max HP, like Sitrus. [Reasoning](HELD_ITEMS.md#group-c--weak-in-stock-still-weak-here--shipped) · `test/fork/buff_flat_hp_items.c` |
| Signature type items | `BUFF_SIGNATURE_TYPE_ITEMS` | ✅ | Plates, Memories and Drives are locked to their own species and boost on the type-item scale, as do the signature orbs. [Reasoning](HELD_ITEMS.md#group-b--dominated-by-our-own-buffs--shipped) · `test/fork/buff_signature_type_items.c` |
| Confusion self-hit buff | `BUFF_CONFUSION_SELF_DAMAGE` (+ `…_POWER`) | ✅ | The confusion self-hit is 60 BP instead of 40. [Mechanics](DETERMINISM.md#deterministic_status) · `test/fork/buff_confusion.c` |

## Abilities, types & gimmicks

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Innate abilities | `FEATURE_INNATE_ABILITIES` | `config/feature.h`, `src/fork/innate_abilities.c` | ✅ | Some species gain always-on innate abilities on top of their chosen one; an innate is a pure boon. [Details](INNATE_ABILITIES.md) · `test/fork/innate_abilities.c` |
| Species ability overrides | `FEATURE_INNATE_ABILITIES` | `src/fork/species_ability_overrides.c` | ✅ | Replaces a species' ability slot without editing `gSpeciesInfo`, CI-gated against innate clashes. [Details](INNATE_ABILITIES.md#direction) |
| New types | `FEATURE_NEW_TYPES` | `config/feature.h`, `src/fork/new_types.c` | ✅ | Re-types selected species everywhere from one hook (first: Galarian Ponyta/Rapidash → Fire/Fairy). [Details](NEW_TYPES.md) · `test/fork/new_types.c` |
| Custom abilities (Affinity family) | _(data; no flag)_ | `src/fork/type_affinity.c` | ✅ | An Affinity gives its holder a latent third type in battle, weaknesses included. [Details](NEW_ABILITIES.md) |
| Custom abilities (Halo) | _(data; no flag)_ | `src/fork/halo.c` | ✅ | While its holder is out, no single hit can take more than 40% of any battler's max HP; the holder pays +1 PP per move. [Details](NEW_ABILITIES.md#case-study-halo) |
| Item-free battle gimmicks | `FEATURE_FREE_GIMMICKS` | `config/feature.h`, `src/battle_gimmick.c` | ✅ | Mega, Z-Moves, Tera and Dynamax need no items, with a per-mon picker among them. [Details](FREE_GIMMICKS.md) · `test/fork/free_gimmicks.c` |
| Sand Spit weather animation fix | _(fix; no new flag)_ | `src/battle_util.c`, `src/battle_script_commands.c` | ✅ | Sand Spit no longer plays a garbage animation that could hang the battle. `test/fork/weather_ability_animation.c` |
| Illusion survives a form-change gfx reload | _(fix; no new flag)_ | `src/battle_gfx_sfx_util.c` | ✅ | A disguised battler's sprite is no longer repainted as its true species when Dynamax ends. `test/fork/illusion_dynamax.c` |

## Battle AI

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Species-aware AI overrides | `AI_FLAG_SMART_SPECIES_LOGIC` | `src/fork/battle_ai_species_overrides.c` | ⚠️ partial | Per-species AI fixes (so far Palafin and Sharpedo); extend by adding cases. `test/fork/ai_smart_species_logic.c` |
| Deliberate AI Z-Move usage | `AI_FLAG_SMART_Z_MOVE` | `src/fork/battle_ai_zmove.c` | ✅ | The AI saves its Z-Move to secure a KO or before fainting, not on turn one. `test/fork/ai_zmove_selection.c` |
| Z-Move base power fix | _(no flag; always compiled)_ | `src/battle_z_move.c`, `src/battle_util.c` | ✅ | One function gives Z-Move power to both the menu and the damage calc, correct for signature Z-Moves. `test/fork/zmove_power.c` |
| AI gimmick type matchups | _(no flag; always compiled)_ | `src/fork/battle_ai_gimmick.c`, `src/battle_ai_util.c` | ✅ | The AI scores a Z-Move or Max Move's matchup without its base move's quirks (Freeze-Dry, Flying Press). `test/fork/ai_gimmick_effectiveness.c` |
| -ate abilities convert Max Moves | _(no flag; always compiled)_ | `src/battle_main.c`, `src/battle_controller_player.c` | ✅ | Pixilate & co. retype Max Moves as the games do, in the picker and when fired. `test/fork/ate_max_moves.c` |
| Max Move preview resolves per slot | _(no flag; always compiled)_ | `src/battle_dynamax.c`, `src/battle_controller_player.c` | ✅ | Each move slot previews its own Max Move and power. `test/fork/dynamax_move_preview.c` |

## UI & accessibility

| Feature | Flag(s) | Where | Status | Notes |
|---|---|---|---|---|
| Color-blind HP bar | `COLOR_BLIND` | `config/accessibility.h` | ✅ | The battle HP bar's healthy band is blue instead of green. Party/summary bars unchanged. |
| Clean battle healthboxes | `B_CLEAN_HEALTHBOX` | `config/fork.h`, `src/battle_interface.c` | ✅ | Removes the healthbox corner tail and the singles player EXP bar. |
| Illusion-safe effectiveness readout | `B_SHOW_EFFECTIVENESS` (fix; no new flag) | `src/battle_controller_player.c` | ✅ | The effectiveness icon reads a disguised foe's apparent typing, so it can't leak an Illusion. Type chart only while disguised. |

## The fork-flag test harness

The `DETERMINISTIC_*`, `BUFF_*` and `FEATURE_*` flags all ride one mechanism, and
this is the note the other docs point back to.

Each flag is registered into the runtime config system (`DETERMINISTIC_CONFIG_DEFINITIONS`
/ `BUFF_CONFIG_DEFINITIONS` / `FEATURE_CONFIG_DEFINITIONS` in
`include/constants/config_changes.h`), so engine code reads it via `GetConfig(X)` and
battle tests toggle it per-test with `WITH_CONFIG(X, TRUE)`. The `#define`s are the
production defaults, while **the per-test baseline forces every flag off**
(`TestInitConfigData`), so the inherited upstream suite runs against stock behavior and
only `test/fork/*.c` opts in.

Adding a flag costs one `#define`, one `*_CONFIG_DEFINITIONS` line, and `GetConfig` at
its use sites. A magnitude (`BUFF_SHELL_BELL_DENOMINATOR`) stays a plain compile-time
constant beside its registered toggle. The `B_FRONTIER_*` flags are the exception:
plain `#if` compile-time flags, not registered.

## Known quirks / future work

- **Only the Factory and Tower are converted.** Per-facility status and the conversion
  pattern are in [`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md).
- **`FRONTIER_STAGES_PER_CHALLENGE` (7 → 10) is global**, so unconverted facilities with
  fixed 7-entry tables (Pyramid, floor names) are off until their own conversion.
- **Battle Dome at 6v6:** its 3-mon layout tables are stubbed so it builds, but its
  layout is wrong.
- **Forced Lv100 leftovers:** the other facilities' record windows still show a Lv50
  block, and the save still holds Lv50 records that can no longer change.
- **Dead "won challenge" lobby path** under `B_FRONTIER_ENDLESS`, kept for old saves.
- **Substitute message mis-targeting (upstream #10630)** is fixed here
  (`test/fork/substitute_message.c`). Two sibling upstream defects remain:
  `STRINGID_SUBSTITUTEDAMAGED` names `gBattlerTarget` rather than the Substitute's owner
  (wrong foe on a doubles spread move), and `CancelerSubstitute()`'s loop tests
  `cv->battlerDef` instead of its own `battler`.

## CI / infrastructure

- **Hardened `Install binutils` step:** a fork-owned composite action caches the
  toolchain and retries stalled installs.
  [`.github/actions/install-binutils`](../.github/actions/install-binutils/action.yml)
  carries the details and the sync rule.
- **`test/battle/front_anim.c` is quarantined (`TO_DO`)** because a latent upstream
  out-of-bounds write hangs the suite under our memory layout. The `FORK:` note in that
  file has the diagnosis and when to restore it.

## Conventions

- Intentional divergences from upstream inside upstream-owned files are tagged `FORK:`
  (greppable: `grep -rn "FORK:" src include .github`).
- Changes worth contributing back are tagged `UPSTREAM:`.
- Prefer a config flag over patching core logic; see `CLAUDE.md` for the full rationale.
