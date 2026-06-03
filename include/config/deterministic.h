#ifndef GUARD_CONFIG_DETERMINISTIC_H
#define GUARD_CONFIG_DETERMINISTIC_H

// FORK: fork-owned config file. These DETERMINISTIC_* flags are an ongoing
// fork project to strip random chance out of the game piece by piece, so that
// outcomes are decided by player choices and state rather than RNG. Each flag
// removes one specific source of randomness; FALSE = stock
// pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered
// into the runtime config system (DETERMINISTIC_CONFIG_DEFINITIONS in
// constants/config_changes.h), so code reads them via GetConfig(DETERMINISTIC_X)
// and battle tests can toggle them per-test with WITH_CONFIG(DETERMINISTIC_X,
// TRUE/FALSE). The test baseline forces every flag off (see TestInitConfigData)
// so the inherited suite keeps exercising stock behavior; the dedicated
// test/battle/deterministic_*.c files opt in explicitly. To add a new flag: add
// a #define here and one line in DETERMINISTIC_CONFIG_DEFINITIONS.

// When TRUE, critical hits no longer occur from the random crit-chance roll.
// A hit still crits when it is *guaranteed* by a non-random source: moves that
// always crit (e.g. Frost Breath, Surging Strikes), Laser Focus, the Merciless
// ability vs. a poisoned target, and crit-stage stacks high enough to reach 1/1
// odds. Crit-blocking (Battle Armor, Shell Armor, Lucky Chant) is unaffected.
// See IsCriticalHit() in src/battle_util.c.
#define DETERMINISTIC_CRITICAL_HITS TRUE

// When TRUE, the post-calc damage roll no longer multiplies damage by a random
// 85%-100%. Instead the multiplier is fixed and scales with the battle's turn
// count: it is DETERMINISTIC_DAMAGE_BASE_PERCENT on the first turn and grows by
// DETERMINISTIC_DAMAGE_TURN_INCREMENT for every turn that has passed thereafter.
// With the defaults below that is 92% on turn 1, 93% on turn 2, 94% on turn 3,
// and so on. It is intentionally *uncapped*, so from turn 9 on it exceeds 100%
// and a move can deal more than its stock maximum. The AI's damage prediction is
// taught the same value, so min/median/max/random rolls all collapse to this
// single deterministic figure. See DoMoveDamageCalcVars in src/battle_util.c and
// the roll helpers in src/battle_ai_util.c.
#define DETERMINISTIC_DAMAGE TRUE

// Tuning for DETERMINISTIC_DAMAGE (ignored when it is FALSE). BASE_PERCENT is the
// first-turn multiplier as a percentage; TURN_INCREMENT is how many percentage
// points it climbs for each turn that has since elapsed. Set TURN_INCREMENT to 0
// for a flat multiplier with no per-turn ramp.
#define DETERMINISTIC_DAMAGE_BASE_PERCENT 92
#define DETERMINISTIC_DAMAGE_TURN_INCREMENT 1

// When TRUE, paralysis stops being a coin-flip and becomes a flat, predictable
// tax. It no longer randomly costs the battler its turn (the 25% full-paralysis
// roll in CancelerParalyzed) and no longer cuts Speed (the drop in
// GetBattlerTotalSpeedStat). In their place every move the paralyzed battler uses
// pays two deterministic penalties: it costs DETERMINISTIC_PARALYSIS_PP_TAX extra
// PP (CancelerPPDeduction) and its priority is lowered by
// DETERMINISTIC_PARALYSIS_PRIORITY_TAX (GetBattleMovePriority) — so a paralyzed
// mon moves later in its priority bracket and burns PP faster, but never freezes
// up and keeps its full Speed. Quick Feet, whose niche is shrugging off the
// paralysis Speed drop, is exempt from both taxes. See
// src/battle_move_resolution.c and src/battle_main.c.
#define DETERMINISTIC_PARALYSIS TRUE

// Tuning for DETERMINISTIC_PARALYSIS (ignored when it is FALSE). PP_TAX is how
// many extra PP each move costs while paralyzed; PRIORITY_TAX is how many points
// of priority each move loses. Set either to 0 to drop that one penalty (e.g.
// PRIORITY_TAX 0 keeps only the PP tax). Mirrors DETERMINISTIC_DAMAGE's
// toggle + compile-time tuning split.
#define DETERMINISTIC_PARALYSIS_PP_TAX 1
#define DETERMINISTIC_PARALYSIS_PRIORITY_TAX 1

// When TRUE, a flinch additional effect can never be applied to a target that was
// already flinched on the previous turn. This is an anti-lock CAP layered on top
// of whatever decides the flinch in the first place — with DETERMINISTIC_ADDITIONAL_EFFECTS
// also on (the shipped config), flinch is first gated on super effective / STAB
// like any other effect (see that flag), and this rule then prevents a gated
// flinch from chaining turn after turn into a stunlock: each flinch "uses up" the
// next turn's. Guaranteed flinches (chance >= 100%) and Fake Out (and any
// first-turn-only flincher, which can't be used on consecutive turns anyway) are
// exempt and always flinch. Inner Focus / Shield Dust / Covert Cloak immunity is
// unchanged. Required by DETERMINISTIC_ADDITIONAL_EFFECTS so its super-effective/
// STAB flinches can't lock. See TryTriggerAdditionalEffect() in src/battle_util.c.
#define DETERMINISTIC_FLINCH TRUE

// When TRUE, a move's chance-based additional effect (burn, paralysis, a stat
// drop, etc.) stops being a random roll and instead lands on a fixed, state-based
// condition, so a move's "secondary" is decided by the matchup rather than luck:
//   - If the move's type CAN be super effective against something (every type but
//     Normal in the stock chart), the effect lands only when the hit was actually
//     super effective. So Fire Punch only burns when it hits a Fire-weak target.
//   - If the move's type can NEVER be super effective (Normal), the effect lands
//     only when the move is STAB (the user shares the move's type). So Body Slam
//     only paralyzes when used by a Normal-type user.
// Flinch obeys this same super-effective/STAB gate (Iron Head / Rock Slide only
// flinch on a super effective hit; Stomp only flinches from a Normal user), and
// DETERMINISTIC_FLINCH (which this flag requires) then adds an anti-lock cap so a
// gated flinch can't chain into a stunlock. Guaranteed effects (chance >= 100%)
// always land, unchanged. The stock secondary-chance boosters — Serene Grace and
// the Pledge Rainbow, which normally just double the odds — instead make the
// effect certain: the holder bypasses the gate and always lands it. This includes
// flinch, which still keeps DETERMINISTIC_FLINCH's anti-lock cap — so a boosted
// flinch lands even on a neutral/resisted hit, but still can't be re-applied the
// next turn, so the boosters can't restore flinch-lock. The AI's valuation is taught the
// same conditions so it credits an effect exactly when it will actually happen.
// See TryTriggerAdditionalEffect() and DeterministicAdditionalEffectApplies() in
// src/battle_util.c.
#define DETERMINISTIC_ADDITIONAL_EFFECTS TRUE

// When TRUE, hold items whose effect is a random chance to trigger stop rolling
// and instead become guaranteed one-shot "entry" items: the effect always
// activates, but only on the first turn the holder is on the field (its entry
// turn, whether it leads or switches in mid-battle — see IsBattlersFirstTurn),
// and the item is then consumed. Covered items (by hold effect, so shared
// effects come along):
//   - Focus Band (HOLD_EFFECT_FOCUS_BAND): like a Focus Sash that works from ANY
//     HP, but only on the entry turn; survives one lethal hit at 1 HP, then is
//     consumed. As with Focus Sash, a multi-hit move gets around it (the first
//     strike consumes the band, the next KOs). See GetAdjustedDamage().
//   - Quick Claw (HOLD_EFFECT_QUICK_CLAW): always moves first within its priority
//     bracket on the entry turn, regardless of move, then is consumed. See
//     TryChangingTurnOrderEffects() in battle_main.c.
//   - Crit items — Scope Lens / Razor Claw (HOLD_EFFECT_SCOPE_LENS), Lucky Punch
//     (Chansey), Leek/Stick (Farfetch'd): the holder's FIRST landed attack is a
//     guaranteed critical hit, then the item is consumed (so it applies to the first
//     attack, not a whole turn). This composes with DETERMINISTIC_CRITICAL_HITS (which
//     otherwise removes random crits), giving the crit-item class a deterministic
//     purpose again. See IsCriticalHit().
//   - Lansat Berry (HOLD_EFFECT_CRITICAL_UP): once its HP threshold is reached the
//     berry is consumed and the holder's next attack is a guaranteed critical hit
//     (reusing Laser Focus's volatile), instead of a crit-stage boost that the
//     deterministic-crit regime can never cash in. See CriticalHitRatioUp().
//   - Flinch items — King's Rock / Razor Fang (HOLD_EFFECT_FLINCH): the holder's
//     FIRST attack whose move doesn't already flinch flinches the target, then the
//     item is consumed. Like Fake Out, this flinch is set via SetMoveEffect (not the
//     additional-effect path), so it bypasses DETERMINISTIC_FLINCH's anti-lock cap
//     and flinches even a target that flinched last turn. See TryKingsRock() in
//     battle_hold_effects.c.
// Crit and flinch items are consumed at move end via MOVEEND_DETERMINISTIC_HOLD_CONSUME.
// Separately, this flag makes Starf Berry's random +2 stat deterministic: it
// raises the holder's currently-highest stat instead of a random one
// (RandomStatRaiseBerry). Evasion items (BrightPowder / Lax Incense) are left to
// a future DETERMINISTIC_ACCURACY_EVASION flag.
#define DETERMINISTIC_HOLD_EFFECTS TRUE

#endif // GUARD_CONFIG_DETERMINISTIC_H
