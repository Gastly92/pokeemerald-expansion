#ifndef GUARD_CONFIG_FEATURE_H
#define GUARD_CONFIG_FEATURE_H

// FORK: fork-owned config file. The FEATURE_* flags here gate standalone
// gameplay features this fork layers on top of upstream (innate abilities, new
// types, etc.). FALSE = stock pokeemerald-expansion behavior.
//
// These #defines are the *production* defaults. The flags are also registered
// into the runtime config system (FEATURE_CONFIG_DEFINITIONS in
// constants/config_changes.h), so code reads them via GetConfig(FEATURE_X) and
// battle tests can toggle them per-test with WITH_CONFIG(FEATURE_X, TRUE/FALSE).
// The test baseline forces every flag off (see TestInitConfigData) so the
// inherited suite keeps exercising stock behavior; the dedicated test files opt
// in explicitly. To add a new flag: add a #define here and one line in
// FEATURE_CONFIG_DEFINITIONS.

// When TRUE, some species gain one or more *innate* abilities that are always
// active in addition to their single chosen ability (e.g. Flygon always has an
// innate Levitate on top of its normal ability). Innates are additive passives:
// they are NOT copied/swapped/suppressed as the battler's identity (Trace, Skill
// Swap, Role Play, the ability pop-up, etc. all still see only the primary
// ability in gBattleMons[battler].ability), but they DO satisfy "does this
// battler have ability X?" trait checks via BattlerHasAbility(). Innates honor
// the same suppression rules as a real ability of the same name (Gastro Acid,
// Neutralizing Gas, Mold Breaker on breakable abilities, Ability Shield). An
// innate's *effect* may diverge from the real ability by design: an innate
// Levitate is a deliberate pure boon — still immune to Ground moves and entry
// hazards, but unlike a real Levitate it stays "grounded" for the beneficial
// ground interactions (field terrain, Toxic Spikes absorption) via
// IsBattlerGroundedForBenefit(). The species->innate table lives in
// src/innate_abilities.c; the predicate lives in
// BattlerHasAbility()/IsInnateActive() (src/battle_util.c).
#define FEATURE_INNATE_ABILITIES TRUE

// When TRUE, some species' types are overwritten with a fork-defined typing,
// replacing the stock gSpeciesInfo types for those species everywhere the
// canonical accessor GetSpeciesType() is consulted (battle type matchups/STAB,
// the type icons, the Pokédex, the summary screen, IsSpeciesOfType, etc.). The
// species->types override table lives in a fork-owned file (src/new_types.c)
// rather than in gSpeciesInfo, so it never conflicts on upstream sync and leaves
// the upstream species data untouched. The first entry re-types Galarian Ponyta
// and Galarian Rapidash from Psychic(/Fairy) to Fire/Fairy. The override is
// applied inside GetSpeciesType() (src/pokemon.c); see fork-docs/NEW_TYPES.md
// for how to add a species.
#define FEATURE_NEW_TYPES TRUE

// When TRUE, the held-item requirement for the battle transformation gimmicks is
// dropped and a single mon may have MORE than one gimmick available at once,
// chosen from a picker. Specifically:
//   - Mega Evolution no longer needs a Mega Stone (any species with a Mega form
//     can evolve); for species with X/Y Megas (Charizard, Mewtwo) the form is
//     chosen by the battler's RESOLVED in-battle stats -- physical (X) when its
//     current Attack >= Sp. Atk, else special (Y), tie -> X -- via
//     GetMegaStoneForBattler() (src/battle_util.c) instead of by which stone is
//     held. Resolved stats (EVs/nature/IVs), not base, so a physical-EV Charizard
//     reaches Mega X while a special-EV one reaches Mega Y.
//   - Z-Moves no longer need a Z-Crystal (any damaging move becomes its type's
//     Z-Move; signature Z-Moves still require the specific base move).
//   - Terastallization and Dynamax no longer need the Tera Orb / Dynamax Band in
//     the bag nor their B_FLAG_* charge/enable flags (those upstream config flags
//     are left untouched; the checks are simply skipped for the player).
//   - The player presses Start during move selection to cycle a per-mon picker
//     through every gimmick that mon can currently use, then off.
// The once-per-mon and once-per-type-per-trainer restrictions are unchanged (a
// mon that has used any gimmick can't use another; each gimmick type is still
// usable once per trainer per battle), so a trainer may e.g. use one Z-Move and
// one Mega Evolution across a battle, but a single mon picks exactly one.
// AI trainers use the best gimmick any of their eligible mons can, which makes
// existing trainer battles noticeably tougher; this is intentional. FALSE =
// stock pokeemerald-expansion behavior (item-gated, single Start toggle). See
// fork-docs/FORK.md.
#define FEATURE_FREE_GIMMICKS TRUE

#endif // GUARD_CONFIG_FEATURE_H
