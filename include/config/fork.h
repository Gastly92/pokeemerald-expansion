#ifndef GUARD_CONFIG_FORK_H
#define GUARD_CONFIG_FORK_H

// FORK: fork-owned config file. Compile-time flags this fork layers on top of
// upstream that gate boot-time / new-game behavior. Unlike the FEATURE_* and
// DETERMINISTIC_* flags (which are registered into the runtime config system so
// battle tests can toggle them per-test), these are plain compile-time #defines
// read directly via #if / value substitution at their use sites.
//
// They live here, in a file we own, rather than inline in upstream's
// config/general.h: keeping them out of an upstream-maintained header means an
// upstream edit near these lines can never conflict with them. This header is
// pulled in once from include/global.h, right after config/general.h, so the
// flags are visible everywhere general.h is. To add a new compile-time fork
// flag, add a #define here; nothing else is needed.

// Boot sequence.
#define SKIP_TITLE_SEQUENCE          TRUE    // If TRUE, skips the title sequence on boot (the copyright screen, the intro cinematic, and the title screen), going straight to the main menu. The RHH intro still plays if EXPANSION_INTRO is TRUE. Essential boot init (save loading, etc.) still runs.

// New-game intro.
#define SKIP_BIRCH_SPEECH            TRUE    // If TRUE, skips Prof. Birch's new-game monologue (the Pokémon/Lotad speech and the closing "you're ready" remarks), keeping only the character look and name selection before the game starts.
#define GENDER_NEUTRAL_TEXT          TRUE    // If TRUE, gendered new-game wording is replaced with neutral text. Currently applies to the player-look picker ("Choose your appearance." / TYPE 1 / TYPE 2 instead of "Are you a boy? Or are you a girl?" / BOY / GIRL); can be reused for other gendered strings later. Only displayed text changes; the underlying selection is unaffected.
#define START_AT_BATTLE_FRONTIER     TRUE    // If TRUE, a new game skips the moving-truck arrival animation and Littleroot intro, warping the player straight to the Battle Frontier ferry dock with the flags/vars set as if they had just arrived there for the first time. Has no effect on FRLG. See SetBattleFrontierFirstArrivalState() in src/new_game.c.
#define START_WITH_BATTLE_GIMMICK_ITEMS TRUE  // If TRUE, a new game gives the player the four battle-transformation key items (Mega Ring, Z-Power Ring, Dynamax Band, Tera Orb) in the bag. Mega Evolution, Z-Moves and Ultra Burst work immediately (item is all they need); Dynamax and Tera stay disabled until their B_FLAG_DYNAMAX_BATTLE / B_FLAG_TERA_ORB_* flags (config/battle.h) are enabled, left for a later player-facing toggle. See GiveStartingBattleGimmickItems() in src/new_game.c.

// New game option defaults. These set the initial value of the corresponding option for a fresh save (SetDefaultOptions() in src/new_game.c); the player can still change them in the Options menu afterwards. Use the OPTIONS_* constants from include/constants/global.h.
#define NEW_GAME_TEXT_SPEED          OPTIONS_TEXT_SPEED_FAST   // Default text speed for a new game. Vanilla: OPTIONS_TEXT_SPEED_MID. Options: OPTIONS_TEXT_SPEED_{SLOW,MID,FAST}.
#define NEW_GAME_BATTLE_STYLE        OPTIONS_BATTLE_STYLE_SET  // Default battle style for a new game. Vanilla: OPTIONS_BATTLE_STYLE_SHIFT. Options: OPTIONS_BATTLE_STYLE_{SHIFT,SET}.

#endif // GUARD_CONFIG_FORK_H
