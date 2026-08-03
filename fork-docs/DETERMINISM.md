# Determinism: removing RNG

A central goal of this fork is to **strip as much random chance out of the game
as possible**, so that what happens in a battle follows from the player's
choices and the state on the field rather than from luck. This is rolled out
gradually through a family of **`DETERMINISTIC_*`** flags (in
[`include/config/deterministic.h`](../include/config/deterministic.h)), each of
which removes one specific source of randomness. A flag set to `FALSE` is stock
`pokeemerald-expansion` behavior; this fork enables them as features mature.

## Why, given this is a single-player game

This isn't a competitive-meta rebalance — there's no human opponent to keep
things fair for, so the usual multiplayer concerns (how a move like Scald shifts
a metagame, etc.) don't apply here. The thing being protected is the *single
player's* experience against the AI, especially in the Battle Frontier
facilities, where the appeal is **building a long win streak through your own
skill and Pokémon knowledge**. In stock play a great run can be ended by pure hax
with no counterplay — a surprise critical hit, a Quick Claw turn flip into a
Sheer Cold OHKO, a flinch you never get to act through. Determinism cuts both the
lucky *and* the unlucky variance out of the loop, so a streak is won or lost on
decisions, not dice. (The flip side — the player also loses their own lucky
breaks — is intentional, and where the compensating `BUFF_*` systems come in.)

The idea is that a random *upside* (a lucky crit, a lucky burn, a lucky full
paralysis) is replaced by something that the player can read off the board and
plan around.

## Flags shipped so far

All of the `DETERMINISTIC_*` flags below are **enabled** in the shipped ROM. Each
is registered into the runtime config system and read via `GetConfig(...)`, so
battle tests opt in per-test with `WITH_CONFIG(...)` while the suite baseline
forces every flag **off** (see the **Test harness** note in
[`FORK.md`](FORK.md)). The `#define`s in
[`config/deterministic.h`](../include/config/deterministic.h) are the production
defaults and the source of truth for exact tuning values; the battle AI is taught
each rule so it keeps valuing moves by what will actually happen. Each flag has a
dedicated `test/fork/deterministic_*.c`. This section is the per-flag reference;
[`FORK.md`](FORK.md) carries a one-line index entry for each.

### `DETERMINISTIC_CRITICAL_HITS`

Removes the random crit-chance roll in `IsCriticalHit()` (`src/battle_util.c`):
crits still land only when *guaranteed* — always-crit moves, Laser Focus,
Merciless vs. a poisoned target, or crit-stage stacks that already reach 1/1 odds.
Crit-blocking (Battle Armor, Shell Armor, Lucky Chant) unchanged. AI: its damage
prediction (`ShouldCalcCritDamage` in `src/battle_ai_util.c`) no longer assumes
partial-chance crits that can't land — counting a crit only when guaranteed, via
the shared `IsGuaranteedCriticalHit()` predicate (also the source of truth in
`IsCriticalHit`) — and its crit-*chance* move valuations (`src/battle_ai_main.c`:
the Risky high-crit-move bonus, Focus Energy, and Dragon Cheer) stop rewarding
chance it can no longer cash in (Laser Focus and always-crit synergies still
count, since those crits are guaranteed). Test: `deterministic_critical_hits.c`.

### `DETERMINISTIC_DAMAGE`

Tuning: `DETERMINISTIC_DAMAGE_BASE_PERCENT`, `DETERMINISTIC_DAMAGE_TURN_INCREMENT`.

Replaces the random 85%–100% damage roll with a fixed multiplier that scales with
the battle's turn count: `BASE_PERCENT` on turn 1, `+TURN_INCREMENT` per elapsed
turn (`DETERMINISTIC_DAMAGE_PERCENT` = `BASE_PERCENT + TURN_INCREMENT *
gBattleTurnCounter`). Defaults 92% / +1% → 92% turn 1, 93% turn 2, …
**Intentionally uncapped** — from turn 9 on it exceeds 100%, so prolonged battles
deal more than the stock maximum (set `TURN_INCREMENT` to 0 for a flat
multiplier). The AI's damage prediction (`src/battle_ai_util.c` roll helpers) is
fed the same figure, so its min/median/max/random rolls collapse to the
deterministic value and it never mis-predicts. Test: `deterministic_damage.c`.

### `DETERMINISTIC_FLINCH`

An **anti flinch-lock cap** that *composes* with `DETERMINISTIC_ADDITIONAL_EFFECTS`:
a flinch effect is first gated on super effective / STAB like any other effect
(see that flag — Iron Head/Rock Slide only flinch on a super effective hit, Stomp
only from a Normal user), and this flag then prevents a *gated* flinch from being
re-applied to a foe that was flinched the previous turn, so a fast flincher can't
chain it into an inescapable lock (each flinch "uses up" the next turn's).
Implemented in `TryTriggerAdditionalEffect()` (`src/fork/deterministic_moves.c`); the "flinched
last turn" bit is snapshotted per battler in `HandleEndTurn_ContinueBattle`
(`src/battle_main.c`, `BattlerState.flinchedLastTurn`) just before the volatile is
cleared. Guaranteed flinches (chance ≥ 100%) and **Fake Out** / any first-turn-only
flincher are exempt (they bypass both the gate and the cap). Inner Focus / Shield
Dust / Covert Cloak immunity unchanged. The AI's flinch valuation
(`IsFlinchGuaranteed`, `AI_CalcAdditionalEffectScore` via
`AI_IsAdditionalEffectReliable`) is taught the composed rule. Required by
`DETERMINISTIC_ADDITIONAL_EFFECTS` so its gated flinches can't lock. Test:
`deterministic_flinch.c`.

### `DETERMINISTIC_ADDITIONAL_EFFECTS`

A move's chance-based additional effect (burn, paralysis, a stat drop, …) stops
being a random roll and lands on a fixed, state-based condition: if the move's
type **can** be super effective (every type but Normal in the stock chart), the
effect lands only on a super effective hit (e.g. Fire Punch burns only a Fire-weak
target); if the type **never** can (Normal), it lands only when the move is
**STAB** (e.g. Body Slam paralyzes only from a Normal user). **Only applies to
sub-100% effects** — guaranteed (≥ 100%) or chance-less (primary) effects always
land, unchanged. **Flinch obeys the same gate** (Iron Head/Rock Slide only flinch
on a super effective hit, Stomp only from a Normal user); `DETERMINISTIC_FLINCH`
(required) then adds an anti-lock cap on top so a gated flinch can't chain into a
stunlock. The stock secondary-chance boosters (**Serene Grace** and the **Pledge
Rainbow**), which would otherwise be near-useless under this flag, instead make a
holder's effect *certain* — bypassing the gate and always landing (detected as
"computed chance > base chance" so both sources and their quirks fall out for
free). This **includes flinch**: a boosted flinch lands even on a neutral/resisted
hit, but still keeps the anti-lock cap (it just can't be re-applied next turn), so
the boosters can't restore flinch-lock. Implemented in
`TryTriggerAdditionalEffect()` / `DeterministicAdditionalEffectApplies()`
(`src/fork/deterministic_moves.c`), called from `Cmd_setadditionaleffects`; the AI is taught
the same condition via `AI_IsAdditionalEffectReliable` (`src/battle_ai_util.c`,
used by `AI_CalcAdditionalEffectScore`). (The super-effective branch relies on a
recorded damage result; in practice every one of the game's chance-based
additional effects is on a damaging move, so this is always available.) Test:
`deterministic_additional_effects.c`.

### `DETERMINISTIC_PARALYSIS`

Tuning: `DETERMINISTIC_PARALYSIS_PP_TAX`, `DETERMINISTIC_PARALYSIS_PRIORITY_TAX`.

Turns paralysis from a coin-flip into a flat, predictable tax. Removes the random
25% full-paralysis miss (`CancelerParalyzed`, `src/battle_move_resolution.c`) and
the Speed cut (`GetBattlerTotalSpeedStat`, `src/battle_main.c`); in their place
every move the paralyzed battler uses costs **+`DETERMINISTIC_PARALYSIS_PP_TAX`
PP** (`CancelerPPDeduction`) and has its **priority lowered by
`DETERMINISTIC_PARALYSIS_PRIORITY_TAX`** (`GetBattleMovePriority`), defaults 1/1,
so it acts later in its bracket and burns PP faster while keeping full Speed (set
either tax to 0 to drop it). **Quick Feet** (which already ignores the Speed drop)
is exempt from both taxes. The full-paralysis roll still consults `RNG_PARALYSIS`
(guaranteed pass via `RandomChance(1, 1)`) so `PASSES_RANDOMLY` tests stay valid.
Because turn order (Speed + priority) is read through the shared engine functions,
the AI's turn-order *prediction* tracks this automatically; the AI's paralysis
*valuation* (`IncreaseParalyzeScore`) is also made config-aware so it values the
priority-bracket demotion instead of the stale Speed-halving check. Test:
`deterministic_paralysis.c`.

### `DETERMINISTIC_HOLD_EFFECTS`

Turns chance-to-trigger hold items into guaranteed one-shot **entry items**: the
effect always fires, but only on the holder's first turn, then the item is
consumed. The exact "first turn" differs by item kind. *Attacking* items (Quick
Claw, crit/flinch items) fire on the holder's first action — `IsBattlersFirstTurn`,
so leads *and* mid-battle switch-ins each get one window. *Defending* items (Focus
Band) instead fire on the holder's first turn as a live **target** —
`IsBattlersEntryTurn`, which is the lead/chosen-switch turn the holder actually
faces an attack, and **not** the turn after. (`IsBattlersFirstTurn` alone is true
for *two* turns after a mid-battle switch — `isFirstTurn == 2` then `1` — which
used to let a switched-in Focus Band holder survive twice; the entry window is now
closed by `facedFoeAction`, a per-battler flag cleared on switch-in and set at move
end once a foe has acted while the holder was on the field. A faint replacement,
sent in *after* the foe already acted, therefore still gets its window on its first
*playable* turn, like a lead.) **Focus Band** becomes a Focus Sash that
works from **any** HP (not just full) but only on that entry turn — survives one
lethal hit at 1 HP then is consumed (`GetAdjustedDamage` in `src/battle_util.c`,
consumed via `BattleScript_HangedOnMsg`); like Sash, a multi-hit move still gets
around it. **Quick Claw** always moves first in-bracket on the entry turn
regardless of move, then is consumed (`TryChangingTurnOrderEffects`,
`src/battle_main.c`; consumed in `BattleScript_QuickClawActivation`, which
announces "The *item* was used up…" like the other consumed items). **Crit
items** — Scope Lens / Razor Claw / Lucky Punch (Chansey) / Leek (Farfetch'd) —
make the holder's **first landed attack** a guaranteed crit (honoring
crit-blockers), then are consumed; this *composes* with
`DETERMINISTIC_CRITICAL_HITS` to give the crit-item class a purpose again
(`IsCriticalHit`). **Lansat Berry** behaves the same way once its HP threshold is
reached: the berry is consumed and the holder's next attack is a guaranteed crit,
reusing Laser Focus's volatile instead of a (dead-under-determinism) crit-stage
boost (`CriticalHitRatioUp`). **Flinch items** — King's Rock / Razor Fang — flinch
the target on the holder's **first attack that didn't itself flinch the target**,
then are consumed; this includes an attack whose own flinch was **gated out by
`DETERMINISTIC_ADDITIONAL_EFFECTS`** (e.g. a non-super-effective Rock Slide) — the
rock fills the gap rather than bowing out just because the move *can* flinch
(decided by the `!volatiles.flinched` check after the move's own effects resolve,
so it never double-flinches). Like Fake Out the flinch is set via `SetMoveEffect`
(not the additional-effect path), so it bypasses `DETERMINISTIC_FLINCH`'s
anti-lock cap and flinches even a target that flinched last turn (`TryKingsRock`,
`src/battle_hold_effects.c`). Crit/flinch items are consumed at move end via the
new `MOVEEND_DETERMINISTIC_HOLD_CONSUME` (`src/battle_move_resolution.c`), which
announces the consumption ("The *item* was used up…",
`BattleScript_DeterministicHoldEffectConsume`) so the otherwise-silent crit/flinch
is credited to the item. Separately, **Starf Berry** raises the holder's
currently-highest stat instead of a random one (`RandomStatRaiseBerry`). The AI's
OHKO reasoning (`ShouldTryOHKO`, `src/battle_ai_util.c`) and crit/damage prediction
are taught the new behavior. **Evasion items (BrightPowder / Lax Incense) are
intentionally left alone** here — handled by `DETERMINISTIC_ACCURACY_EVASION`.
Test: `deterministic_hold_effects.c`.

### `DETERMINISTIC_ACCURACY_EVASION`

Tuning: `DETERMINISTIC_OHKO_MAX_HP_PERCENT`, `DETERMINISTIC_EXTRA_MISS_COST_PERCENT`.

Accuracy/evasion stop being a hit/miss coin flip and become a **PP economy**.
Every move that reaches the accuracy roll always hits (`DoesMoveMissTarget`,
`src/battle_util.c`); semi-invulnerability, Protect and immunities are resolved
earlier (`ResolveMoveEffects`) and still avoid the move. A sub-100% move's **max
PP is scaled down by its base accuracy, rounded down** (a 5 PP/80% move → 4 PP; a
10 PP/85% move → 8) to pay for the misses it no longer suffers — applied in
`CalculatePPWithBonus` (`src/pokemon.c`, so the overworld PP counter shows it) and
clamped onto battle PP in `REQUEST_ALL_BATTLE` (`src/battle_controllers.c`) so
stored/effective PP can't diverge when the flag is toggled; only the 50<acc<100
band is scaled (100%/0-acc exempt). Two effects are **priced below their real
accuracy** (`DeterministicEffectiveAccuracy`, `src/fork/deterministic_moves.c`),
at `DETERMINISTIC_EXTRA_MISS_COST_PERCENT` (72%) of it, because their miss cost
more than the wasted turn and the flag deletes that extra cost for free:
`EFFECT_TRIPLE_KICK` (Triple Kick, Triple Axel) rolled accuracy **once per
strike**, so a nominally 90% Triple Axel really landed its full combo 73% of the
time and delivered 78% of its max damage — it now always lands all three strikes
for full escalating power, the guarantee Skill Link/Loaded Dice used to be the only
way to buy; and `EFFECT_RECOIL_IF_MISS` (Jump Kick, High Jump Kick, Axe Kick,
Supercell Slam) staked **half the user's max HP** on the roll. Both drop from 10
max PP to 6 (Supercell Slam 15 → 10). The crash still fires wherever the target is
genuinely unaffected — Protect (`MOVE_RESULT_PROTECTED`, part of
`MOVE_RESULT_NO_EFFECT`) or a type immunity (`B_CRASH_IF_TARGET_IMMUNE`) — so those
moves keep their counterplay; only the whiff case is gone. 72 is a **tuned** value,
not a derivation: it puts Triple Axel's lifetime-output cut (−28%) level with what
the plain scaling already does to Focus Blast (−29%). `EFFECT_POPULATION_BOMB` rolls
per strike too but is deliberately **not** discounted — it already pays on the
strike-count axis (see `DETERMINISTIC_MOVE_RESULTS`), so it keeps the ordinary 90%
scaling (10 → 9 PP). A **per-use PP economy**
(`CancelerPPDeduction`, `src/battle_move_resolution.c`) for moves targeting
opponents, derived from the same net accuracy/evasion stage as the old hit calc
(`GetAccEvasionStageDelta`, `src/battle_util.c` — so Keen Eye/Unaware/Foresight/Minds
Eye and the repurposed Compound Eyes/Victory Star carry over): +acc/−evasion
recover 1 PP per net stage, −acc/+evasion cost 1, accuracy and evasion cancel,
doubles sums both foes. Always spends ≥1 PP (last PP spent if it can't pay in
full), recovery clamped to max PP. Flat additive uncapped taxes
(`GetDeterministicMoveTargetPPTax`): BrightPowder/Lax Incense, Sand Veil, Snow
Cloak, Tangled Feet add 1 PP to offensive moves; Wonder Skin adds 1 to status
moves; Hustle adds 1 to the user's physical moves; Micle Berry (the old accuracy
berry) makes its next move ignore the user's accuracy drops and the foe's evasion
increases (never taxed by them, still recovers from boosts/drops) plus a flat 1
PP. **OHKO moves** become always-hitting attacks dealing
`DETERMINISTIC_OHKO_MAX_HP_PERCENT` (40%) of max HP instead of a full KO, but
**keep the OHKO immunities** — Sturdy, Dynamax, a higher-level target, and type
immunity still block them (`DoesOHKOMoveMissTarget` + `EFFECT_OHKO` in
`DoMoveDamageCalc`). The AI treats every move as always-hitting
(`Ai_SetMoveAccuracy`, `src/battle_ai_main.c`) so it neither avoids low-accuracy
moves nor fears evasion. **Moves that were exactly 50% accurate** (Zap Cannon,
Inferno, DynamicPunch) now require a recharge turn like Hyper Beam, set at move end
via the new `MOVEEND_DETERMINISTIC_RECHARGE` reusing Hyper Beam's
`rechargeTimer`/`gLockedMoves` state. **AI awareness:** the shared
`MoveGainsDeterministicRecharge` predicate (`src/fork/deterministic_moves.c`) drives both the
move-end hook and the AI, so the AI treats a 50% move as a recharge move
(`AI_IsMoveEffectInMinus` downside + Instruct avoidance,
`src/battle_ai_util.c`/`battle_ai_main.c`). **Move-info display:** because accuracy is now meaningless, the in-battle
move-info submenu (press L on move select, `MoveSelectionDisplayMoveDescription`,
`src/battle_controller_player.c`) replaces its `ACC` field with the move's
**projected net PP cost** this turn — `GetProjectedMovePPCost`
(`src/battle_util.c`) mirrors `CancelerPPDeduction`'s deduction (base, Pressure,
paralysis tax, the accuracy/evasion stage economy and flat item/ability taxes,
minus accuracy/Micle refunds), projecting single-target moves against the first
live foe. Test: `deterministic_accuracy_evasion.c`.

### `DETERMINISTIC_ABILITIES`

⚠️ Partial. Strips RNG (and a random choice or two) out of abilities so they pay
off on board state, not luck. All gated on `GetConfig(DETERMINISTIC_ABILITIES)` in
`src/battle_util.c` unless noted. **Contact status/effects always *attempt* their
effect** (immunity/substitute/contact checks still gate): Static (paralysis),
Poison Point & Poison Touch (poison), Flame Body (burn), Cute Charm (infatuation —
and the opposite-gender requirement is dropped, attempts regardless of gender),
Toxic Chain (bad poison; roll in `SetToxicChainPriority`,
`src/battle_script_commands.c`), Cursed Body (disable the used move). **Effect
Spore** drops the trigger roll *and* the 3-way poison/paralysis/sleep pick and
always attempts drowsiness/Yawn (`BattleScript_EffectSporeDrowsy`, new
`STRINGID_EFFECTSPOREDROWSY` keyed to the attacker). **Stench** only attempts its
flinch on the holder's first turn out (`IsBattlersFirstTurn`) but then always
does, resolving at `MOVEEND_ABILITIES_ATTACKER` (before attacker hold items, so it
precedes a King's Rock flinch). **End-of-turn:** Shed Skin always cures status,
Healer always cures the ally's status, Harvest always recovers a used berry and,
in sun, also heals 1/16 max HP (`BattleScript_HarvestActivatesSunHeal` + new
`STRINGID_HARVESTHPGAIN`). **Quick Draw** only activates on the user's first turn
out (`IsBattlersFirstTurn`, `src/battle_main.c`) — the attacker-ability mirror of
Quick Claw. **Moody** compares the *raw* current values of the five battle stats
(acc/evasion **always** excluded, ignoring `B_MOODY_ACC_EVASION`), raises the
lowest-valued by +2 and lowers the highest-valued by −1, ties affecting every tied
stat (`GetMoodyStatValue`). **Pickup** recovers the first valid used item in a
fixed order — self → partner → directly-opposing foe → across foe — instead of a
random battler. **Trace** (doubles) copies the directly-opposing foe (same flank)
when traceable, else the other. **Forewarn** tie-breaks equal-power moves by
battler/move-slot order instead of randomly. **Rivalry** keys off shared type
instead of gender (+25% sharing a type, −25% sharing none). **AI awareness:** the
Rivalry damage mod rides the shared `GetBattlerAbility` path so the AI's damage
prediction tracks it for free; and `AI_DeterministicContactAbilityPunishes`
(`src/battle_ai_util.c`) teaches the AI to treat a contact move into a known
Static/Flame Body/Poison Point/Effect Spore/Cute Charm holder as a guaranteed
downside (when its attacker can actually receive the status), so among
equal-damage moves it prefers a non-contact one (via
`AI_IsMoveEffectInMinus`/`CompareMoveEffects`, mirroring the Rough Skin/Iron Barbs
handling). `AI_WhoStrikesFirst` (`src/battle_ai_util.c`) also models Quick Draw's
guaranteed entry-turn first-strike, so the AI's turn-order prediction matches the
engine. And `AI_DeterministicAbilityGuaranteesStatus` teaches the AI that its
*own* Poison Touch (on a contact hit) / Toxic Chain (on any damaging hit) now
guarantees poison, scored as a plus via `AI_IsMoveEffectInPlus` (reusing
`AI_CanPoison`), so e.g. a Poison Touch user prefers an equal-damage contact move.
And `StatusWillBeCuredDeterministically` (`src/battle_ai_util.c`) teaches the AI
the flip side — that a non-volatile status inflicted on a target whose **known**
ability cures it every end-of-turn is wasted, so it stops re-applying it: **Shed
Skin** (unconditional) and **Hydration** (only while the target is being rained
on, mirroring the engine's weather check via the AI's `AI_GetWeather()` view) are
OR'd into all five status gates
(`AI_CanPutToSleep`/`AI_CanPoison`/`AI_CanParalyze`/`AI_CanBurn`/`AI_CanGiveFrostbite`),
so a doomed status scores the same −10 as a do-nothing move. **Healer is
deliberately not modelled here** — it cures the *partner*, not the holder, so it
isn't keyed on the target's own ability (it would need a doubles-only
partner-ability lookup, plus an end-of-turn-liveness assumption for the Healer
mon). **Overworld ability RNG is out of scope** (post-battle Pickup finds, Cute
Charm/Synchronize & encounter-type/level/held-item biases) — deferred to the
Battle Pyramid survival rework. Test: `deterministic_abilities.c`.

### `DETERMINISTIC_STATUS`

Tuning: `DETERMINISTIC_INFATUATION_TURNS`, `DETERMINISTIC_INFATUATION_DMG_PERCENT`,
`DETERMINISTIC_SLEEP_TURNS`.

Makes the status conditions whose effect is a roll into legible, state-based
outcomes. **Infatuation**: the opposite-gender requirement is dropped (matching
`DETERMINISTIC_ABILITIES`' Cute Charm) at all three application sites (Attract
`Cmd_*`, `BS_TrySetInfatuation`, Cute Charm); instead of the 50% "won't attack"
coin flip the battler always acts, and its moves **against the loved target deal
`DETERMINISTIC_INFATUATION_DMG_PERCENT`% damage** (a final-damage modifier in
`GetOtherModifiers`, `src/battle_util.c`, so the AI's shared damage prediction
tracks it); the infatuation lasts a fixed `DETERMINISTIC_INFATUATION_TURNS` of the
battler's actions (new `VOLATILE_INFATUATION_TIMER`, decremented in
`CancelerInfatuation`, then cured via new `BattleScript_DeterministicInfatuationEnds`).
**Sleep** always lasts `DETERMINISTIC_SLEEP_TURNS` turns (default 2, like Rest) at
both the move (`MOVE_EFFECT_SLEEP`) and end-of-turn (Yawn/Effect Spore) sites.
**Confusion** stops being a 2-5 turn chain of self-hit rolls (`CancelerConfused`):
on its first confused action the battler takes **one** guaranteed 40-BP typeless
self-hit but **still carries out its chosen move that turn** (the move is never
denied); the volatile then **lingers until the battler's next action, when it
snaps out**. Because the battler is never robbed of its action, a faster foe can't
chain confusion into an action lock — and because the volatile persists in the
interim, the foe can't *refresh* it either (a confused target can't be
re-confused, and `AI_CanBeConfused` already declines one), so the self-hit costs
at most once per spell of confusion. Implemented via a `confusionTurns == 1`
snap-out sentinel and `BattleScript_DeterministicConfusionSelfDmg` (self-damages
on `BS_ATTACKER` then returns so the move continues — it does not repoint
`gBattlerTarget`, so the continuing move keeps its real target; a self-hit that
would KO falls back to ending the move). Infinite confusion never snaps out (it
self-hits each action). The AI's confusion *valuation* (`IncreaseConfusionScore`)
is lowered to a light-chip `WEAK_EFFECT` to match (it no longer disables the foe).
Freeze is out of scope (handled by the frostbite-over-freeze config). The AI's
risk heuristics that treat a confused foe as possibly-incapacitated (e.g. Focus
Punch / Counter safety) are not retuned. Test: `deterministic_status.c`.

### `DETERMINISTIC_MOVE_RESULTS`

Tuning: `DETERMINISTIC_MULTI_HIT_COUNT`, `DETERMINISTIC_MULTI_HIT_MAX_COUNT`,
`DETERMINISTIC_POPULATION_BOMB_COUNT`,
`DETERMINISTIC_POPULATION_BOMB_LOADED_DICE_COUNT`, `DETERMINISTIC_RAMPAGE_TURNS`,
`DETERMINISTIC_WRAP_TURNS`, `DETERMINISTIC_WRAP_GRIP_CLAW_TURNS`,
`DETERMINISTIC_PRESENT_POWER`, `DETERMINISTIC_PRESENT_HEAL_DENOMINATOR`.

Fixes or makes state-based the rolls baked into move outcomes. **Multi-hit** 2-5
moves always hit `DETERMINISTIC_MULTI_HIT_COUNT` (3); Skill Link and Loaded Dice
guarantee `DETERMINISTIC_MULTI_HIT_MAX_COUNT` (5). **Population Bomb** (nominal 10
strikes, each rolling accuracy) would land all 10 once `DETERMINISTIC_ACCURACY_EVASION`
makes every strike hit, so it is toned down to `DETERMINISTIC_POPULATION_BOMB_COUNT`
(5); Loaded Dice or Skill Link restore the full
`DETERMINISTIC_POPULATION_BOMB_LOADED_DICE_COUNT` (10). The other fixed-count moves
(Triple Kick, Beat Up, Dragon Darts) are untouched
(`SetRandomMultiHitCounter`/`CancelerMultihitMoves`). **Protect**-family moves used
on consecutive turns always fail (`CanUseMoveConsecutively`, `src/battle_util.c`).
**Rampage** (Thrash/Outrage/Petal Dance) always lasts `DETERMINISTIC_RAMPAGE_TURNS`
(2). **Speed ties** are broken by a fixed ladder — higher raw base Speed → lighter
weight → higher remaining-HP% → (only then) the random permutation, inverted under
Trick Room — via the shared `DeterministicSpeedTieWins` (`src/battle_main.c`), also
taught to the AI's `AI_WhoStrikesFirst`. **Binding** moves last
`DETERMINISTIC_WRAP_TURNS` (4), or `DETERMINISTIC_WRAP_GRIP_CLAW_TURNS` (7) with
Grip Claw (`SetWrapTurns`). **Tri Attack** burns a target whose Attack is higher,
frostbites one whose Sp. Atk is higher, paralyzes on a tie. **Dire Claw**
paralyzes if the target's Speed beats either defense, sleeps on a three-way
Speed/Def/SpD tie, else poisons. **Magnitude**'s power is chosen by the
attacker:target weight ratio (like Heavy Slam), keeping Magnitude's 10-150 range
(`CalculateMagnitudeDamage`). **Present** always damages a foe
(`DETERMINISTIC_PRESENT_POWER`, 80 BP) and always heals an ally
(1/`DETERMINISTIC_PRESENT_HEAL_DENOMINATOR`, 1/4 HP), keyed off the target's side.
**Fickle Beam** only doubles on a super-effective hit. **Shell Side Arm**'s
physical/special pick defaults to physical on a projected-damage tie (its poison is
already super-effective-gated by `DETERMINISTIC_ADDITIONAL_EFFECTS`).
**Roar/Whirlwind/Dragon Tail/Red Card** drag out the next living party member in
slot order (wrapping), so party order matters. **AI awareness:** multi-hit damage
prediction uses the fixed counts (`src/battle_ai_util.c`); turn-order prediction
uses the speed-tie ladder; the weight/effectiveness-based powers ride the shared
damage calc. Test: `deterministic_move_results.c`.

More `DETERMINISTIC_*` flags will follow. As random upsides are removed, the plan
is to add balancing systems alongside them — the **`BUFF_*`** flags (see
[`FORK.md`](FORK.md)) — so play stays fair rather than just easier or harder. The
AI is taught each flag's new rules so it still plays to the actual odds.
