#ifndef GUARD_CONFIG_BUFF_H
#define GUARD_CONFIG_BUFF_H

// FORK: fork-owned config file. These BUFF_* flags are an ongoing fork project
// to rebalance items and other game functionality, usually as compensation for
// other changes we make (e.g. the DETERMINISTIC_* project trades random upsides
// away, so some items get buffed to keep battles balanced). Each flag improves
// one specific thing; FALSE = stock pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered
// into the runtime config system (BUFF_CONFIG_DEFINITIONS in
// constants/config_changes.h), so code reads them via GetConfig(BUFF_X) and
// battle tests can toggle them per-test with WITH_CONFIG(BUFF_X, TRUE/FALSE).
// The test baseline forces every flag off (see TestInitConfigData) so the
// inherited suite keeps exercising stock behavior; the dedicated
// test/battle/buff_*.c files opt in explicitly. To add a new flag: add a
// #define here and one line in BUFF_CONFIG_DEFINITIONS.

// When TRUE, Shell Bell recovery is buffed from the stock 1/8 of the damage dealt
// to 1/BUFF_SHELL_BELL_DENOMINATOR (1/4 by default). The toggle is registered in
// the runtime config system (BUFF_CONFIG_DEFINITIONS) so battle tests can flip it
// per-test; the magnitude lives in the plain compile-time constant below. This
// mirrors how DETERMINISTIC_DAMAGE pairs a registered toggle with unregistered
// BASE_PERCENT/TURN_INCREMENT tuning. See TryShellBell() in
// src/battle_hold_effects.c.
#define BUFF_SHELL_BELL TRUE

// Heal divisor used when BUFF_SHELL_BELL is on: HP recovered = damage dealt /
// BUFF_SHELL_BELL_DENOMINATOR (4 -> 1/4). Stock behavior (flag off) instead
// divides by the item's holdEffectParam (8 -> 1/8). Lower = more healing; must be
// nonzero. Ignored when BUFF_SHELL_BELL is FALSE.
#define BUFF_SHELL_BELL_DENOMINATOR 4

// When TRUE, Leech Seed is buffed in two ways:
//  - Multiple battlers can seed the same target ("stacking"). Each seeder drains
//    the victim independently every turn, so a doubly-seeded foe loses HP twice
//    per turn, one share going to each seeder.
//  - Using Leech Seed on a foe *you already seed* no longer just fails - it deals
//    an immediate drain (instead of wasting the turn). Seeding a foe that only
//    *another* battler has seeded stacks your seed on instead of failing.
// The per-tick (and immediate) drain fraction is 1/BUFF_LEECH_SEED_DENOMINATOR.
// The toggle is registered in the runtime config system (BUFF_CONFIG_DEFINITIONS)
// so battle tests can flip it per-test; the magnitude lives in the plain
// compile-time constant below. Stock behavior (flag off): a single seeder, and
// re-seeding an already-seeded foe fails. See Cmd_setseeded() in
// src/battle_script_commands.c and HandleEndTurnLeechSeed() in
// src/battle_end_turn.c.
#define BUFF_LEECH_SEED TRUE

// Drain divisor used for Leech Seed when BUFF_LEECH_SEED is on: HP drained per
// seeder per turn (and on an immediate re-drain) = victim's max HP /
// BUFF_LEECH_SEED_DENOMINATOR. Stock behavior (flag off) is always 1/8. Lower =
// more draining; must be nonzero. Kept at 8 so the per-tick rate matches vanilla
// by default - the buff is the stacking + immediate re-drain, not a bigger tick.
#define BUFF_LEECH_SEED_DENOMINATOR 8

// When TRUE, the two accuracy-boosting hold items get a purpose again under
// DETERMINISTIC_ACCURACY_EVASION. That flag removes accuracy as a hit/miss axis and
// replaces it with a PP economy, but it only carried the DEFENDER's half across:
// BrightPowder / Lax Incense still tax the attacker a PP (GetDeterministicMoveTargetPPTax),
// while Wide Lens and Zoom Lens kept multiplying an accuracy figure that nothing reads --
// DoesMoveMissTarget() returns FALSE before it ever calls GetTotalAccuracy(). Both items
// are therefore completely dead in a shipped build, exactly the hole Blunder Policy was in.
// This flag closes it from the attacker's side, mirroring how the fork repurposed the
// accuracy-boosting ABILITIES (Compound Eyes / Keen Eye / Illuminate) into evasion-ignore:
//   - Wide Lens (HOLD_EFFECT_WIDE_LENS): the holder's moves stop paying the flat evasion
//     taxes -- a target's BrightPowder / Lax Incense, Sand Veil in sand, Snow Cloak in
//     snow, Tangled Feet while confused, Wonder Skin against a status move.
//   - Zoom Lens (HOLD_EFFECT_ZOOM_LENS): the same, AND the whole stat-stage half of the
//     axis is ignored -- BOTH the target's evasion boosts and the holder's OWN accuracy
//     drops (e.g. from Sand Attack). That is the entire ignorePenalties treatment No Guard
//     and Micle Berry already get, reused wholesale rather than reimplemented, which is why
//     the accuracy-drop half comes along. Only on turns the holder moves second, its stock
//     condition, lifted verbatim from GetTotalAccuracy(). Stronger in a narrower window,
//     mirroring its bigger stock boost (holdEffectParam 20 vs Wide Lens's 10).
// PURE BOON, like every other accuracy source in the PP economy: these only ever cancel a
// penalty, never turn into a refund, so an accuracy item can reduce a move's cost to the
// base 1 PP but never below it. Against a target with no evasion trick at all they do
// nothing. See GetAccuracyItemRelief() in src/fork/deterministic_moves.c, and its two
// consumers -- CancelerPPDeduction() in src/battle_move_resolution.c and the move-info
// projection GetProjectedMovePPCost() in src/battle_util.c.
#define BUFF_ACCURACY_ITEMS TRUE

#endif // GUARD_CONFIG_BUFF_H
