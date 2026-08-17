# Converting a Battle Frontier facility to 6v6 + endless

This is the **playbook** for giving a Battle Frontier facility the fork's
"sandbox" treatment: full **6-mon teams**, an **endless** challenge loop, **forced
Lv100**, and **competitive-roster opponents**. It generalizes the work already
done for the **Battle Factory** (first) and the **Battle Tower** (second) so the
**Battle Palace, Battle Arena, Battle Dome, Battle Pyramid, Battle Pike** can
follow the same path with far less rediscovery.

It is the how-to *and* the reference for how these facilities actually behave; the
flag comments in `include/config/frontier.h` remain the source of truth for each
flag's exact rule, and [`FORK.md`](FORK.md) is only the one-line index. Read this top
to bottom before converting a facility, then work the checklist.

## Status

| Facility | 6v6 | Endless | Forced Lv100 | Roster opponents | Notes |
|---|---|---|---|---|---|
| Factory | ✅ | ✅ | ✅ | ✅ | First conversion; rents a team (see "rental vs own-party"). |
| Tower | ✅ | ✅ | ✅ | ✅ | Second; own-party. Static competitive Brain teams (silver/gold) + random gym-leader bosses every 10 wins (`battle_tower_trainers.c`). General opponents are tier-gated. See "The Battle Tower" below. |
| Palace | ⛔ | ⛔ | ⛔ | ⛔ | Pending. Own-party, AI-driven mons, singles+doubles. |
| Arena | ⛔ | ⛔ | ⛔ | ⛔ | Pending. Own-party, judged 3-turn bouts, **singles only** (`arenaWinStreaks[lvlMode]`, no battleMode index). |
| Dome | ⛔ | ⛔ | ⛔ | ⛔ | Pending. Tournament bracket — fixed 3-mon coordinate tables not generalized to 6. |
| Pyramid | ⛔ | ⛔ | ⛔ | ⛔ | Pending. Keeps a working bag; survival/floor structure differs most. |
| Pike | ⛔ | ⛔ | ⛔ | ⛔ | Pending. Room-based, not a straight battle streak. |

## The flags

All live in fork-owned headers (`include/config/frontier.h`), gated with `#if` in
C and `.if` in scripts. They are facility-agnostic; converting a facility means
*honoring* them on that facility's code/script paths.

- `B_FRONTIER_PARTY_SIZE_6V6` — full 6-mon teams. **Defined as `1`/`0`, NOT
  `TRUE`/`FALSE`** (see Footgun 1).
- `B_FRONTIER_ENDLESS` — endless loop, per-win BP, rest/resume, Brain at 50/100.
- `B_FRONTIER_FORCE_LVL_100` — Open Level only.
- `B_FRONTIER_EXTENDED_MONS` — competitive roster (`src/fork/frontier_extended_mons.c`).
- Per-facility "disable a sub-mode" flag if needed, e.g.
  `B_FRONTIER_TOWER_DISABLE_MULTI_LINK`. Add a new one per facility/sub-mode.

## Shared infrastructure (reuse — do not reinvent)

These already exist and are facility-agnostic; a conversion *extends* them:

- **Brain cadence** — `gFrontierBrainInfo[facility].streakAppearances`
  (`src/frontier_util.c`). Set `{50, 100, 50, 1}` under `#if B_FRONTIER_ENDLESS`
  (Brain on the 50th win = silver, 100th = gold, then every 50).
- **Per-win BP** — the `#if B_FRONTIER_ENDLESS` block at the top of
  `GiveBattlePoints` (`src/frontier_util.c`). Extend its
  `if (facility == FRONTIER_FACILITY_FACTORY || facility == FRONTIER_FACILITY_TOWER)`
  to include the new facility, reading **that facility's** win-streak array
  (`<fac>WinStreaks[battleMode][lvlMode]`, or `[lvlMode]` for Arena/Pike/Pyramid).
  Scaling is 2 BP/win in set 1, 4 in set 2, …; Brain win = the win number.
- **Roster draw** — `GetRandomFrontierExtendedMonId()` (`src/fork/frontier_extended_mons.c`,
  declared in `include/fork/frontier_extended_mons.h`): one uniform, format-aware pull
  from the competitive roster. **Uniform = no difficulty scaling by streak.**
- **Draft rules** — `TeamHasGimmickItemConflict` / `TierRejectsCandidate` /
  `ReserveForcedTierSlot` (fork-owned `src/fork/frontier_draft.c`, declared in
  `include/fork/frontier_draft.h`): the at-most-one-Mega/Z guard, the per-slot tier
  quota, and the random forced-tier-slot reservation. Shared by the Factory and
  the Tower; reuse them, don't re-implement.
- **Static special teams** — the Tower's Salon Maiden and gym-leader bosses live
  in the fork-owned `src/fork/battle_tower_trainers.c` (authored `struct TrainerMon`
  arrays built via `CreateFacilityMon`). A facility that wants hand-authored boss
  teams instead of (or beside) a draft can follow the same id-range + hook pattern
  (see the boss notes there and the hook list under "The Battle Tower" below).
  *Known issue (glide):* gym-leader bosses **glide** to the battle spot instead of
  walking — their overworld sheets are 3-frame (one pose per facing, no step
  cycle), unlike the Salon Maiden's 9-frame walk sheet. Cosmetic, sprite-art
  limitation (~6 new authored frames per leader, ×8); see the boss gfx hook in
  `src/frontier_util.c` (`SetBattleFacilityTrainerGfxId`).
- **Opponent party builder** — `FillTrainerParty()` (`src/battle_frontier.c`)
  already has a `useExtendedRoster = (VarGet(VAR_FRONTIER_FACILITY) == FRONTIER_FACILITY_TOWER)`
  branch. Facilities that build opponents through `FillTrainerParty`
  (Tower/Dome/Palace/Arena/Pyramid — everything except the Factory, which has its
  own builder) just **add their facility to that condition**.
- **6v6 party picker** — already supports 6 (`sFrontierPartyOrderDescIds[]` +
  `gText_Fifth_PM`/`gText_Sixth_PM`, `src/party_menu.c`). Nothing to do per
  facility except the "select N Pokémon" lobby text.

## Step-by-step checklist

Numbers in brackets point at the equivalent change in the Tower commit history; see
"The Battle Tower" below for what that conversion ended up looking like.

### C side (`src/frontier_util.c`, the facility's `src/battle_<fac>.c`)

1. **[Brain cadence]** Set `gFrontierBrainInfo[FRONTIER_FACILITY_<FAC>].streakAppearances`
   to `{50, 100, 50, 1}` under `#if B_FRONTIER_ENDLESS` (keep the vanilla values in
   the `#else`).
2. **[Per-win BP]** Add `FRONTIER_FACILITY_<FAC>` to the `GiveBattlePoints` endless
   block and read its win-streak array.
3. **[Opponent gen from streak]** In the facility's `SetNext<Fac>Opponent`
   (or equivalent), under `#if B_FRONTIER_ENDLESS` derive the within-set position
   from the **per-mode win streak**, not the shared `curChallengeBattleNum`:
   `battleNum = winStreak % FRONTIER_STAGES_PER_CHALLENGE`. Use `battleNum` for the
   trainer-id draw and the `trainerIds[]` dedup window/storage. This is purely the
   **independence mechanism** that keeps a rested singles run and a rested doubles
   run separate — it is *not* a difficulty ramp (opponents are a uniform roster
   draw). Keep `curChallengeBattleNum` synced in `Set<Fac>BattleWon` for incidental
   readers (`FRONTIER_DATA_BATTLE_NUM`, the record-save check).
4. **[Roster opponents]** Add `FRONTIER_FACILITY_<FAC>` to the `useExtendedRoster`
   condition in `FillTrainerParty` (`src/battle_frontier.c`). It already forces max
   IVs and bypasses the per-trainer `monSet`/high-tier gate.
5. **[Results window]** In `Show<Fac>ResultsWindow` (`src/frontier_util.c`), under
   `#if B_FRONTIER_FORCE_LVL_100` drop the always-empty Lv50 block and show only
   the Open Level streak (mirror `ShowFactoryResultsWindow`/`ShowTowerResultsWindow`).

### Script side (`data/maps/BattleFrontier_<Fac>Lobby/scripts.inc`, battle/pre-battle rooms)

Remember: use `.if FLAG` (assembler), never cpp `#if`, for config flags in scripts
(see CLAUDE.md). The vanilla branch goes in `.else` so syncs stay clean.

6. **[Lobby: forced Lv100]** Skip the Lv50/Open `multichoice` under
   `.if B_FRONTIER_FORCE_LVL_100`; force `setvar VAR_RESULT, FRONTIER_LVL_OPEN`
   before the eligibility check.
7. **[Lobby: drop auto-resume]** Under `.if B_FRONTIER_ENDLESS`, remove the
   `CHALLENGE_STATUS_PAUSED → ResumeChallenge` `map_script_2` from the lobby
   `OnFrame` (the player resumes via the attendant instead).
8. **[Lobby: resume skips intro]** At the attendant, after `SavePlayerParty`, under
   `.if B_FRONTIER_ENDLESS` set the battle mode and `tower_get`/`<fac>_get
   <FAC>_DATA_WIN_STREAK_ACTIVE`; if active, `goto` straight into the team-pick /
   resume flow (skip the welcome + "take the challenge?" prompt). Show a brief
   "Welcome back! Your win streak of N…" message before team select.
9. **[Lobby: disable sub-modes]** If the facility has modes that don't fit
   single-player endless (multi/link), gate those attendants to a "not available"
   message behind a `B_FRONTIER_<FAC>_DISABLE_*` flag; keep the code intact in the
   `.else`/after the `.endif`.
10. **[Battle room: per-win BP after the attendant walks over]** On a win, under
    `.if B_FRONTIER_ENDLESS` drop the fixed "case 7 → end" and instead, *after the
    attendant approaches the player*, `frontier_givepoints` + a parens-free
    "Battle Points" message (the endless award is always ≥ 2, so use a
    `<Fac>_Text_AwardedBattlePoints` without the "(s)"), then heal and loop.
11. **[Battle room: opponent number]** Replace the fixed "2nd/3rd/…/7th opponent"
    line with a win-number message (`<fac>_get <FAC>_DATA_WIN_STREAK; addvar
    VAR_RESULT, 1; buffernumberstring`).
12. **[Battle room: Rest → lobby, no reboot]** Make "Rest" save (`<fac>_save 0`),
    restore the player's real party (own-party facilities) or back up the rented
    team (rental facilities), then warp to the lobby — **do not** `frontier_reset`
    / reboot. Keep the win-streak active flag set so the lobby attendant offers to
    resume. Reword the prompt to "save and take a break from your challenge?".
13. **[Battle room: Brain continues]** Route the Brain-victory path so that, under
    `.if B_FRONTIER_ENDLESS`, after the symbol is awarded it grants BP and resumes
    the streak instead of warping to "won". The Brain win usually does **not** call
    `Set<Fac>BattleWon`, so sync `curChallengeBattleNum` there if your opponent gen
    relies on it.
14. **[Text]** Update every reachable lobby/NPC/rules-board string that names the
    old counts ("three"/"four"/"two") or the old loop ("seven TRAINERS in a row",
    "save and quit") under the relevant flag.

## Rental vs own-party facilities

The biggest per-facility fork is **where the team comes from on resume**:

- **Rental (Factory):** the player rents a team. Resume must **restore the same
  rented team** from a per-mode backup (`frontier.factoryRentedMonsBackup`).
  Back the team up **at rent time** (`AutoRentFullParty`), not only on Rest, so an
  active-but-not-yet-rested challenge still has a valid team to restore (otherwise
  resume restores an all-zero backup = a full team of roster index 0; see
  Footgun 5).
- **Own-party (Tower, and Palace/Arena/Pyramid):** the player brings their own
  mons. Resume **re-picks a fresh team** — no rental backup. `<fac>_init`
  (`InitTowerChallenge`-style) preserves an active mode's win streak (it only
  zeroes `<fac>WinStreaks[mode][lvl]` when that mode's active flag is unset), so
  the streak continues across the re-pick automatically. Singles and doubles stay
  independent because the streak + active flag are per `[battleMode][lvlMode]`.

## The Battle Tower — the worked reference conversion

The Tower is the second conversion and the fullest one; read it as the model for the
rest. It covers Singles and Doubles, with the player's own party.

### Opponent draw is keyed to the win streak — this is the independence mechanism

`SetNextTowerOpponent` / `SetTowerBattleWon` derive
`battleNum = winStreak % FRONTIER_STAGES_PER_CHALLENGE` — **never**
`curChallengeBattleNum`.

> This is what lets a rested **Singles** run and a rested **Doubles** run continue
> **independently**, each keeping its own streak and active flag. It is **not** a
> difficulty ramp.

There is **no scaling** at all: opponents' mons are drawn uniformly from the
competitive roster via the shared `GetRandomFrontierExtendedMonId()` wired into
`FillTrainerParty`, always at max IVs. The vanilla per-trainer themed `monSet` and
high-tier gate are bypassed for the Tower only.

### Rest and resume

"Rest" saves and returns to the lobby with **no reboot**, keeping the player's real
party and leaving the streak active. The lobby no longer auto-resumes a paused
challenge — the player can leave, come back, talk to the attendant and **choose a
fresh team** to continue (`tower_init` preserves an active streak; see "Rental vs
own-party" above).

### Every general opponent brings a Mega

The player hand-picks their own team, so opponents always carry one:
`FillTrainerParty` reserves a random slot for a guaranteed Mega Stone holder, forbids
Mega Stones elsewhere, and applies the shared gimmick rule from `frontier_draft.c`
(which also caps Z-Crystals at one).

General opponents are **tier-gated like the Factory** — no legendaries or mythicals,
at most one pseudo-legendary per team (`TierRejectsCandidate`, every slot
`TIER_NORMAL`). Bosses and the Brain return earlier, so their fixed legendaries are
unaffected.

### Two kinds of special battle

**The Salon Maiden (Anabel)** appears at the 50th and 100th win
(`streakAppearances` `{50, 100, 50, 1}`). She fields a fork-owned **static
competitive team** modelled on her vanilla set — psychic specialist plus the
legendary beasts and the Lati twins — at full six, with separate **Silver** (50th)
and tougher **Gold** (100th) variants (`src/fork/battle_tower_trainers.c`, routed
through `CreateFrontierBrainPokemon`). Beating her awards BP equal to the win number
and the streak **continues** instead of warping out.

**Gym-leader bosses** take every *other* set-end win — the 10th, 20th, … that isn't a
50/100 Brain mark. Each is a real Hoenn gym leader with their own overworld sprite,
battle pic, name and trainer class, fielding an authored competitive team with one
legendary-tier mon and one Mega. They are chosen at **random** from `sTowerBosses[]`
(8 Hoenn leaders), which is deliberately extensible — Elite Four and Champions later.

A fork `BOSS_TRAINER` id range above `TRAINER_PLAYER` marks them, recognized by
additive branches in seven functions:

`FillTrainerParty` · `SetBattleFacilityTrainerGfxId` · `GetFrontierTrainerName` ·
`GetFrontierTrainerFrontSpriteId` · `GetFrontierOpponentClass` ·
`GetOpponentIntroSpeech` · `CopyFrontierTrainerText`

Copy that id-range + branch pattern for any facility that wants named bosses.

### Multi and Link Multi are disabled

Under `B_FRONTIER_TOWER_DISABLE_MULTI_LINK`, those two attendants show a "not
available" message. The code and scripts are kept fully intact behind the flag.

### Tower-specific known limitations

- **Bosses are pure-random** — no immediate-repeat avoidance. A mutable static would
  land in `.data`, which the linker script doesn't place for fork files.
- **Boss and Brain battle music** is the default frontier track.
- **Resting on exactly a Brain-threshold win** (49 or 99) can skip that Brain
  appearance.
- **The shared `trainerIds` dedup list** can briefly hold the other mode's trainers
  after a Singles↔Doubles switch.

## Footguns (each cost real debugging — read these)

1. **A `TRUE`/`FALSE` flag that feeds a numeric `#if`-computed constant scripts
   read as a value silently breaks.** `B_FRONTIER_PARTY_SIZE_6V6` gates
   `FRONTIER_PARTY_SIZE` (6 vs 3) in `constants/global.h`, and the lobby does
   `setvar VAR_0x8005, FRONTIER_PARTY_SIZE`. The script cpp pass leaves `TRUE`
   undefined, so `#if TRUE` → `#if 0` → scripts get 3 while C gets 6 (the picker
   then only lets you choose 3). **Define such flags as a literal `1`/`0`.** See
   CLAUDE.md, "Using config flags in scripts". Flags only ever read via `.if` or as
   operands can stay `TRUE`/`FALSE`.
2. **Use `.if`, not cpp `#if`, for config flags inside scripts.** cpp runs before
   the assembler and doesn't define `TRUE`/`FALSE` for the script pass.
3. **No non-charmap characters in `.string`.** An em-dash (`—`, U+2014) is **not**
   in the charmap; use `--` or rephrase. (`…` U+2026 is fine.)
4. **Build `make release`, not just `make -O all`, before pushing.** The release
   build is stricter and caught the em-dash (charmap) error the debug build let
   through. CI's on-demand ROM build runs `make release`.
5. **Rental facilities: back up the team at rent time.** See "Rental vs own-party".
6. **The Brain win often skips `Set<Fac>BattleWon`.** If opponent gen reads
   `curChallengeBattleNum`, sync it (or derive from the streak) on the Brain path.
7. **Resting exactly on a Brain-threshold win (49/99) can skip that Brain.** The
   first post-resume battle bypasses the mid-loop Brain check. Known minor
   limitation; documented per facility.
8. **A mutable `static`/global in a fork-added `.c` breaks linking.** pokeemerald's
   ld script wildcards `.text`/`.rodata` (so all-const fork files like
   `frontier_extended_mons.c` / `frontier_draft.c` / `battle_tower_trainers.c` link
   fine) but lists `.data`/`.bss` object files **explicitly** — a new file's `.data`
   section is discarded ("defined in discarded section `.data`"). Keep fork files
   `const`-only, or you must add the object to the upstream ld script (which the
   fork avoids). This is why the Tower bosses are picked with no mutable
   "last boss" state.

## Known gaps shared by every conversion so far

- **Hard AI** (`B_FRONTIER_HARD_AI`) is Factory-only. (Tower general opponents are
  tier-gated like the Factory — no legendaries/mythicals, ≤1 pseudo, via
  `frontier_draft.c`'s `TierRejectsCandidate`; the Tower uses **fixed gym-leader
  boss teams** on the set-end marks, each carrying one legendary, rather than a
  drafted one.)
- **Boss polish** (Tower): bosses are pure-random (no immediate-repeat avoidance),
  and use the default frontier battle music.

When you finish a facility, update the Status table above, add/refresh its
`FORK.md` row, and tick the shared gaps if you closed any.
