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
// odds. Because the stripped random roll would otherwise leave the sub-1/1
// crit-ratio boosts doing nothing, two dedicated crit-enablers escalate to a
// guaranteed crit under this flag: Focus Energy and Dragon Cheer each arm a
// one-shot guaranteed crit on the affected battler's next attack (reusing Laser
// Focus's volatile; see Cmd_setfocusenergy), and Super Luck guarantees a crit on
// the user's first turn on the field (its +1 crit stage still stacks every turn),
// mirroring the fork's deterministic Stench. Crit-blocking (Battle Armor, Shell
// Armor, Lucky Chant) is unaffected. See IsCriticalHit()/CalcCritChanceStage()
// in src/battle_util.c.
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
// STAB flinches can't lock. See TryTriggerAdditionalEffect() in src/fork/deterministic_moves.c.
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
// src/fork/deterministic_moves.c.
#define DETERMINISTIC_ADDITIONAL_EFFECTS TRUE

// When TRUE, hold items whose effect is a random chance to trigger stop rolling
// and instead become guaranteed one-shot "entry" items: the effect always
// activates, but only on the holder's first turn (whether it leads or switches in
// mid-battle), and the item is then consumed. The exact "first turn" differs by
// item kind: defending/on-field items (Focus Band) anchor on the field-entry turn
// (IsBattlersEntryTurn — the lead/switch-in turn, and not the turn after), while
// attacking items (Quick Claw, crit items) anchor on the holder's first action
// (IsBattlersFirstTurn). Covered items (by hold effect, so shared effects come
// along):
//   - Focus Band (HOLD_EFFECT_FOCUS_BAND): like a Focus Sash that works from ANY
//     HP, but only on the entry turn (the lead/switch-in turn, never the turn
//     after); survives one lethal hit at 1 HP, then is consumed. As with Focus
//     Sash, a multi-hit move gets around it (the first strike consumes the band,
//     the next KOs). See GetAdjustedDamage() / IsBattlersEntryTurn().
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
//     FIRST attack that didn't itself flinch the target flinches it, then the item is
//     consumed. This includes an attack whose own flinch was gated out by
//     DETERMINISTIC_ADDITIONAL_EFFECTS (e.g. a non-super-effective Rock Slide): the rock
//     fills the gap rather than bowing out just because the move *can* flinch. Like Fake
//     Out, this flinch is set via SetMoveEffect (not the additional-effect path), so it
//     bypasses DETERMINISTIC_FLINCH's anti-lock cap and flinches even a target that
//     flinched last turn. See TryKingsRock() in battle_hold_effects.c.
// Crit and flinch items are consumed at move end via MOVEEND_DETERMINISTIC_HOLD_CONSUME,
// which announces the consumption ("The <item> was used up...") so the otherwise-silent
// crit/flinch is credited to the item.
// Separately, this flag makes Starf Berry's random +2 stat deterministic: it
// raises the holder's currently-highest stat instead of a random one
// (RandomStatRaiseBerry). Evasion items (BrightPowder / Lax Incense) are handled
// by DETERMINISTIC_ACCURACY_EVASION below.
#define DETERMINISTIC_HOLD_EFFECTS TRUE

// When TRUE, accuracy and evasion stop being a hit/miss coin flip and become a PP
// economy instead. Stat-stage accuracy/evasion no longer change whether a move
// lands — every move that reaches the accuracy roll always hits (semi-invulnerable
// targets, Protect, type/ability immunities etc. are resolved earlier and still
// avoid the move). What the accuracy/evasion axis does now:
//   - Max PP scaling: a move whose base accuracy is in the 50<acc<100 band has its
//     maximum PP scaled down by that accuracy, rounded down (a 5 PP / 80% move
//     becomes 4 PP; a 10 PP / 85% move becomes 8), so the holder pays for the misses
//     it no longer suffers — the same idea as
//     DETERMINISTIC_DAMAGE's fixed roll. 100%/0-accuracy moves and the specially
//     handled classes below are exempt. The 50<acc<100 band intentionally carries
//     no further penalty yet (a knob for that can be added later). See
//     CalculatePPWithBonus() in src/pokemon.c.
//   - Per-use PP economy (CancelerPPDeduction in src/battle_move_resolution.c) for a
//     move that targets an opposing mon, derived from the SAME net accuracy/evasion
//     stage the hit calc used (so Keen Eye, Unaware, Foresight/Miracle Eye, Minds
//     Eye, Compound Eyes and Victory Star — which all ignore the target's evasion —
//     carry over for free, satisfying "abilities that affected accuracy still do"):
//       * raising the user's accuracy recovers 1 PP per net stage;
//       * lowering it costs 1 PP per net stage;
//       * raising the target's evasion costs 1 PP per net stage;
//       * lowering it recovers 1 PP per net stage.
//     Accuracy and evasion still cancel (it is one signed net stage). In doubles the
//     opposing targets' stages are summed, so a +1/-1 split costs a normal 1 PP.
//     A move always consumes at least 1 PP; recovery is applied after and clamped to
//     max PP, and if only 1 PP is left when a move would cost more, the last PP is
//     spent. Flat, additive (uncapped) extra costs stack on top: BrightPowder/Lax
//     Incense, Sand Veil (sand) and Snow Cloak (hail/snow), Tangled Feet (confused)
//     each add 1 PP to OFFENSIVE moves targeting the holder; Wonder Skin adds 1 PP to
//     STATUS moves; Hustle adds 1 PP to the user's physical moves. Micle Berry (the old
//     accuracy berry) instead makes its next move ignore the user's accuracy drops and
//     the foe's evasion increases — so it is never taxed by them, though it still recovers
//     PP from boosts/evasion drops — and refunds a flat 1 PP.
//   - OHKO moves (Fissure etc.) become always-hitting attacks that deal
//     DETERMINISTIC_OHKO_MAX_HP_PERCENT of the target's max HP instead of a full KO, but
//     keep the OHKO immunities: Sturdy, Dynamax, a higher-level target and type immunity
//     all still block them entirely. See DoesOHKOMoveMissTarget and EFFECT_OHKO in
//     DoMoveDamageCalc (src/battle_util.c).
//   - Moves that were exactly 50% accurate (Zap Cannon, Inferno, ...) now require a
//     recharge turn like Hyper Beam (MOVEEND_DETERMINISTIC_RECHARGE in
//     src/battle_move_resolution.c).
// The AI is taught that every move always hits (Ai_SetMoveAccuracy), so it neither
// avoids low-accuracy moves nor values evasion as a miss chance.
#define DETERMINISTIC_ACCURACY_EVASION TRUE

// Tuning for DETERMINISTIC_ACCURACY_EVASION (ignored when it is FALSE): the
// percentage of the target's max HP that a (formerly one-hit-KO) OHKO move deals.
#define DETERMINISTIC_OHKO_MAX_HP_PERCENT 40

// When TRUE, abilities whose effect is a random chance (or a random choice) stop
// rolling and become guaranteed/state-based, so an ability pays off on the matchup
// and board state rather than luck. Covered abilities (all in src/battle_util.c
// unless noted):
//   - Contact status/effect abilities always *attempt* their effect (the usual
//     immunity/substitute/contact checks still gate it): Static (paralysis),
//     Poison Point and Poison Touch (poison), Flame Body (burn), Effect Spore
//     (always drowsiness/Yawn instead of the 3-way poison/paralysis/sleep pick),
//     Cute Charm (infatuation — and the opposite-gender requirement is dropped, so
//     it attempts regardless of gender), Toxic Chain (bad poison; the roll in
//     SetToxicChainPriority in src/battle_script_commands.c), Cursed Body (disable
//     the used move).
//   - Stench only attempts its flinch on the holder's first turn on the field
//     (IsBattlersFirstTurn), but then always does. It resolves at
//     MOVEEND_ABILITIES_ATTACKER, before attacker hold items, so it lands before a
//     King's Rock-style flinch item is consumed.
//   - End-of-turn: Shed Skin always cures status, Healer always cures the ally's
//     status, Harvest always recovers a used berry (not just in sun / on the 50%
//     roll) and, when it activates in the sun, also heals 1/16 max HP
//     (BattleScript_HarvestActivatesSunHeal).
//   - Quick Draw only activates on the user's first turn on the field
//     (IsBattlersFirstTurn), but then always moves it first in its bracket — the
//     attacker-ability mirror of DETERMINISTIC_HOLD_EFFECTS' Quick Claw. See
//     src/battle_main.c.
//   - Moody compares the *raw* current values of the five battle stats (Atk/Def/
//     SpA/SpD/Spe — accuracy/evasion are ALWAYS excluded under this flag, ignoring
//     B_MOODY_ACC_EVASION), raises the lowest-valued stat by +2 and lowers the
//     highest-valued by -1; on a tie every tied stat on that side is raised/lowered.
//     Raw values differ from turn 1 (base stats + nature), so there is no degenerate
//     all-equal case.
//   - Pickup recovers the first valid used item in a fixed preference order rather
//     than a random battler's: self, then partner, then the directly-opposing foe,
//     then the across foe (each gated by CantPickupItem).
//   - Trace, in doubles, copies the directly-opposing battler's ability (same flank,
//     opposite side) when traceable, otherwise the other foe's, instead of choosing
//     randomly between two valid foes.
//   - Forewarn breaks ties between equal-power moves by battler/move-slot order
//     (lowest slot, first opponent) instead of randomly (ForewarnChooseMove).
//   - Rivalry keys off shared type instead of gender: +25% damage when attacker and
//     target share a type, -25% when they share none (GetBattlerAbility damage mod).
// Out of scope (deferred): overworld ability RNG — post-battle Pickup item finds,
// Cute Charm/Synchronize encounter bias, Static/Magnet Pull/etc. encounter-type
// bias, Compound Eyes held-item odds, Hustle/Pressure/Vital Spirit encounter level.
// These will be revisited with the Battle Pyramid survival rework.
#define DETERMINISTIC_ABILITIES TRUE

// When TRUE, the non-volatile/volatile status conditions whose effect is a random
// roll become legible, state-based outcomes:
//   - Infatuation: the opposite-gender requirement is dropped (matching
//     DETERMINISTIC_ABILITIES' Cute Charm), it lasts a fixed
//     DETERMINISTIC_INFATUATION_TURNS turns (decremented each time the infatuated
//     battler acts, then cured with a message), and instead of the 50% "won't
//     attack" coin flip the infatuated battler always acts but its moves AGAINST
//     the loved target deal DETERMINISTIC_INFATUATION_DMG_PERCENT% damage. See
//     CancelerInfatuation in src/battle_move_resolution.c and the Attract/Cute
//     Charm application sites.
//   - Sleep always lasts DETERMINISTIC_SLEEP_TURNS turns (default 2, like Rest)
//     instead of the random 2-4/2-5/2-8 spread. See the MOVE_EFFECT_SLEEP and
//     end-of-turn Yawn/Effect-Spore sites.
//   - Confusion stops being a 2-5 turn chain of self-hit rolls. The confused
//     battler's next ATTACKING move makes it hit itself once (the usual 40-BP
//     typeless self-hit) and then confusion clears; a STATUS move shakes it off
//     for free (no self-hit). So confusion is a one-time tempo/damage tax that a
//     utility turn can play around. See CancelerConfused.
// The AI is taught each rule so it values attract/sleep/confusion by what will
// actually happen.
#define DETERMINISTIC_STATUS TRUE

// Tuning for DETERMINISTIC_STATUS (ignored when it is FALSE). INFATUATION_TURNS is
// how many of the infatuated battler's actions the infatuation lasts;
// INFATUATION_DMG_PERCENT is the damage its moves deal to the loved target while
// infatuated; SLEEP_TURNS is the fixed sleep duration.
#define DETERMINISTIC_INFATUATION_TURNS 2
#define DETERMINISTIC_INFATUATION_DMG_PERCENT 50
#define DETERMINISTIC_SLEEP_TURNS 2

// When TRUE, the random rolls baked into move outcomes become fixed or
// state-based, so a move's result is read off the board rather than diced:
//   - Multi-hit moves that roll 2-5 hits always hit DETERMINISTIC_MULTI_HIT_COUNT
//     times (default 3); Skill Link and Loaded Dice instead guarantee
//     DETERMINISTIC_MULTI_HIT_MAX_COUNT (default 5). See SetRandomMultiHitCounter.
//   - Population Bomb has a fixed 10 strikes but rolls accuracy per strike, so
//     DETERMINISTIC_ACCURACY_EVASION (which makes every strike land) would turn it
//     into a guaranteed 10-hit move. To keep it in check it is toned down to
//     DETERMINISTIC_POPULATION_BOMB_COUNT hits (default 5); Loaded Dice or Skill
//     Link guarantee the full DETERMINISTIC_POPULATION_BOMB_LOADED_DICE_COUNT
//     (default 10). The other fixed-count moves (Triple Kick, Beat Up, Dragon
//     Darts) are unaffected. See CancelerMultihitMoves in
//     src/battle_move_resolution.c.
//   - Protect-family moves used on consecutive turns always fail instead of
//     keeping a shrinking success chance. See CanUseMoveConsecutively in
//     src/battle_util.c.
//   - Rampage moves (Thrash/Outrage/Petal Dance) always last
//     DETERMINISTIC_RAMPAGE_TURNS turns (default 2). See MOVE_EFFECT_THRASH.
//   - Speed ties are broken by a fixed ladder instead of a coin flip: higher raw
//     base Speed, then lighter weight, then higher remaining-HP%, then (only if
//     still tied) the random permutation. Inverted under Trick Room. The AI's
//     turn-order prediction uses the same ladder. See GetWhichBattlerFaster /
//     DeterministicSpeedTieWins in src/battle_main.c and AI_WhoStrikesFirst.
//   - Binding moves (Wrap/Fire Spin/…) always last DETERMINISTIC_WRAP_TURNS turns
//     (default 4); a Grip Claw holder traps for DETERMINISTIC_WRAP_GRIP_CLAW_TURNS
//     (default 7). See SetWrapTurns.
//   - Tri Attack picks its status by the target's offenses: burn if its Attack is
//     higher, frostbite if its Sp. Atk is higher, paralysis on a tie. See the
//     MOVE_EFFECT_TRI_ATTACK site.
//   - Dire Claw picks by the target's stats: paralysis if its Speed beats either
//     defense, sleep on a three-way Speed/Def/SpD tie, else poison. See the
//     MOVE_EFFECT_DIRE_CLAW site.
//   - Magnitude's power is chosen by the attacker:target weight ratio (like Heavy
//     Slam) instead of a random tier. See CalculateMagnitudeDamage.
//   - Present always damages a foe (DETERMINISTIC_PRESENT_POWER base power) and
//     always heals an ally (1/DETERMINISTIC_PRESENT_HEAL_DENOMINATOR max HP),
//     keyed off the target's side. See the EFFECT_PRESENT site.
//   - Fickle Beam only doubles its power on a super-effective hit. See the
//     EFFECT_FICKLE_BEAM site.
//   - Shell Side Arm's physical/special pick is deterministic on a projected-damage
//     tie (defaults to physical) rather than a coin flip. See SetShellSideArmCategory.
//   - Roar/Whirlwind/Dragon Tail/Red Card drag out the next living party member in
//     slot order (wrapping) instead of a random one, so party order matters. See
//     the RNG_FORCE_RANDOM_SWITCH site.
// The AI is taught each rule so it values these moves by their real outcome.
#define DETERMINISTIC_MOVE_RESULTS TRUE

// Tuning for DETERMINISTIC_MOVE_RESULTS (ignored when it is FALSE).
#define DETERMINISTIC_MULTI_HIT_COUNT 3
#define DETERMINISTIC_MULTI_HIT_MAX_COUNT 5
#define DETERMINISTIC_POPULATION_BOMB_COUNT 5
#define DETERMINISTIC_POPULATION_BOMB_LOADED_DICE_COUNT 10
#define DETERMINISTIC_RAMPAGE_TURNS 2
#define DETERMINISTIC_WRAP_TURNS 4
#define DETERMINISTIC_WRAP_GRIP_CLAW_TURNS 7
#define DETERMINISTIC_PRESENT_POWER 80
#define DETERMINISTIC_PRESENT_HEAL_DENOMINATOR 4

#endif // GUARD_CONFIG_DETERMINISTIC_H
