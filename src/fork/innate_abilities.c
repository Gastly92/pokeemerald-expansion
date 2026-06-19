#include "global.h"
#include "fork/innate_abilities.h"
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species->innate table (FEATURE_INNATE_ABILITIES). Kept here
// instead of in gSpeciesInfo so upstream syncs never touch it and the upstream
// species data stays untouched. Each row maps a species to an ABILITY_NONE-
// terminated list of innate abilities that are always active on top of that
// species' normal chosen ability. The list is variable-length (no fixed cap): add
// or remove entries freely, just keep the terminating ABILITY_NONE.
//
// ALLOWLIST — only abilities whose innate behavior has been deliberately wired in
// may appear here (see innate_abilities.h "SCOPE"). The fork grows this set one
// ability at a time. Supported innate abilities:
//   - ABILITY_LEVITATE — Ground-immunity / ungrounding, handled in src/battle_util.c
//     (IsBattlerUngroundedByAbilityItemOrEffect and the type-effectiveness calc credit an
//     innate Levitate exactly like the real one). DELIBERATE DIVERGENCE: an innate Levitate is
//     a *pure boon*, NOT identical to a real Levitate. It still floats above Ground moves and
//     entry-hazard damage, but the fork also treats it as grounded for the *beneficial* ground
//     interactions — field terrain and Toxic Spikes absorption (via IsBattlerGroundedForBenefit,
//     src/battle_util.c). So an innate-Levitate mon reaps the terrain it/an ally sets and a
//     Poison-type still clears Toxic Spikes — things a real Levitate forgoes. This is why
//     terrain summoners (the Tapus, Miraidon) and Poison floaters happily carry the innate.
//   - ABILITY_REGENERATOR — heals 1/3 max HP on switch-out, handled at the single switch-out
//     site in src/battle_script_commands.c (Cmd_switchoutabilities), additively alongside the
//     real Regenerator so a mon carrying it as an innate heals exactly like the real ability.
//     The heal is silent (no script/pop-up), so no driver is needed. Suppression parity holds:
//     the innate is gated by IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field),
//     same as the real ability's GetBattlerAbility() path. AI is innate-aware: the heal isn't in
//     any shared calc the AI runs, so the AI's dedicated Regenerator switch/pivot reads
//     (ShouldSwitchIfAbilityBenefit, the bad-odds and hazard-switchin checks in src/battle_ai_switch.c,
//     and ShouldPivot in src/battle_ai_util.c) credit an innate Regenerator via BattlerHasAbility(),
//     so the AI values an innate-only Regenerator's pivot heal. This populates the canon Regenerator
//     users so they keep their signature pivot heal regardless of which ability slot the build
//     picks, plus a few flavor regenerators (Staryu/Starmie's regrowing core, the axolotl Wooper
//     line, Zygarde's reassembling cells).
//   - ABILITY_UNAWARE — ignores the foe's stat-stage changes in the damage and accuracy calcs,
//     handled in src/battle_util.c (the four calc sites that read ABILITY_UNAWARE — offensive and
//     defensive stat stages in the damage calc, plus evasion/accuracy in GetTotalAccuracy and
//     GetAccEvasionStageDelta — route the innate through InnateUnawareBoonStage() next to the
//     chosen-ability test). A pure calc-modifier passive like Levitate: no script / pop-up / driver.
//     Suppression parity holds via IsInnateActive() — Unaware is breakable, so an attacker's Mold
//     Breaker ignores an innate Unaware on the defender exactly as it would the real ability.
//     AI: on-field damage prediction is correct for free (the stat-ignore lives in the shared
//     damage calc, keyed off the real battler via IsInnateActive). The AI's off-field *setup*
//     heuristics — "don't bother boosting against an Unaware foe" (ShouldRaiseAnyStat and the Belly
//     Drum/half-HP-cost score in battle_ai_main.c), the doubles ally-stat-change score
//     (GetAllyStatChangeScore), and the Yawn evasion-dodge stay-in check (battle_ai_switch.c) —
//     read the chosen ability, so each now also credits an innate Unaware (AI_IsInnateOnSide beside
//     the AI_IsAbilityOnSide reads; IsInnateActive at the switch site). All are about the AI's own
//     *boosts* being ignored, which an innate Unaware (boost-ignoring) does just like the real one.
//     DELIBERATE DIVERGENCE: an innate Unaware is a *pure boon*, NOT identical to a real Unaware. A
//     real Unaware blanks the foe's stat stage in both directions (so it ignores a foe's *drop* too,
//     and takes more damage / deals less for it); the innate ignores only the foe's *boosts* and
//     keeps the foe's *drops* (always the favorable half — see InnateUnawareBoonStage, battle_util.c).
//     This populates the canon Unaware users so they keep the stat-ignore no matter which slot the
//     build picks, plus flavor picks too dull/dazed/asleep to notice the foe's buffs (Numel's
//     "doesn't notice being hit", the dazed Psyduck line, the ever-sleeping Komala, the unbothered
//     Snorlax line).
//   - ABILITY_STURDY — endures a lethal hit at full HP (B_STURDY >= GEN_5) and is immune to OHKO
//     moves, handled at the two effect sites in src/battle_util.c (the GetAdjustedDamage endure and
//     the OHKO-move accuracy gate, each gets an IsInnateActive() clause beside the cached chosen-
//     ability test). NO pure-boon divergence: Sturdy is a clean upside that never hurts its holder,
//     so the innate is a 1:1 copy of the real ability. No driver/pop-up wiring is needed — the
//     "endured / Sturdy" messages and the ability pop-up flow from the existing MOVE_RESULT_STURDIED
//     / MOVE_RESULT_ONE_HIT_KO_STURDY flags. Suppression parity holds via IsInnateActive(): Sturdy is
//     breakable, so an attacker's Mold Breaker pierces an innate Sturdy exactly as it would the real
//     ability. AI is innate-aware too: Sturdy's survival reasoning lives in DEDICATED AI helpers, NOT
//     the shared damage calc (so unlike Unaware it is NOT automatic and had to be wired) — the endure/KO
//     predictor (CanEndureHit), the OHKO-move avoidance, BattlerHasMaxHPProtection (src/battle_ai_util.c),
//     and the switch-in KO simulation (src/battle_ai_switch.c) each credit an innate Sturdy via
//     BattlerHasAbility()/SpeciesHasInnate(), so the AI doesn't blunder a hit it can't actually KO. This
//     populates the canon Sturdy users so they keep
//     the signature endure no matter which slot the build picks (Mega/regional/form constants are
//     listed so the innate survives a mid-battle form change), plus an "impenetrable shell" flavor line
//     (Shellder/Cloyster, whose shell "even a missile can't break") that lacks the real ability. Species
//     whose ONLY ability is Sturdy are omitted as redundant when unused by the frontier roster (Cosmoem,
//     Togedemaru-Totem). Ogerpon-Cornerstone is the exception — it is also sole-Sturdy, but because it IS a
//     frontier set it instead takes the innate AND a fork-owned chosen Defiant (species_ability_overrides.c),
//     so its frontier slot isn't spent on the now-innate Sturdy.
//   - ABILITY_NATURAL_CURE — silently cures the holder's status1 on switch-out, handled at the
//     same single switch-out site as Regenerator in src/battle_script_commands.c
//     (Cmd_switchoutabilities), additively alongside the chosen Natural Cure path so a mon
//     carrying it as an innate self-cleanses exactly like the real ability. Like the innate
//     Regenerator there, it writes the party mon's status DIRECTLY (mirroring the controller's
//     REQUEST_STATUS_BATTLE) rather than a second BtlController_EmitSetMonData, so it can't
//     clobber the single bufferA slot a chosen ability (e.g. Slowking's Regenerator) may have
//     queued this switch-out. Silent (no script/pop-up), so no driver is needed. NO pure-boon
//     divergence: Natural Cure is a clean upside that never hurts its holder, so the innate is a
//     1:1 copy. Suppression parity holds via IsInnateActive()/BattlerHasAbility() (Gastro Acid /
//     Neutralizing Gas / not-on-field), same as the real ability's GetBattlerAbility() path. AI is
//     innate-aware: the cure isn't in any shared calc, so the AI's dedicated Natural Cure switch
//     reads credit an innate one via BattlerHasAbility() — the switch-to-cure heuristic
//     (ShouldSwitchIfAbilityBenefit, factored into ShouldSwitchForNaturalCure like Regenerator),
//     the Yawn anti-sleep switch (ShouldSwitchIfBadlyStatused), and the burned/frostbitten
//     force-switch move scoring (src/battle_ai_main.c). This populates the canon Natural Cure users
//     so they keep the signature self-cure no matter which slot the build picks, plus herbal/aromatic
//     healer flavor (the Chikorita line's restorative aroma, Bellossom's revitalizing dance).
//   - ABILITY_PRANKSTER — gives the holder's status moves +1 priority, handled at the single
//     effect site in src/battle_main.c (GetBattleMovePriority): an IsInnateActive() clause sits
//     beside the chosen-ability IsAbilityAndRecord() test, so the boost applies for an innate
//     Prankster too. No script/pop-up/driver — priority is a pure turn-order calc. The AI gets it
//     for FREE: its turn-order prediction (AI_WhoStrikesFirst -> GetBattleMovePriority) runs the
//     same calc keyed off the real battler, so the AI both threatens and respects an innate
//     Prankster's priority. Suppression parity holds via IsInnateActive() (Gastro Acid /
//     Neutralizing Gas / not-on-field); Prankster is not breakable, so Mold Breaker never touches
//     it, same as the real ability. DELIBERATE DIVERGENCE: an innate Prankster is a *pure boon*,
//     NOT identical to a real Prankster. A real Prankster sets gProtectStructs.pranksterElevated,
//     which makes its boosted status moves FAIL against Dark-types (B_PRANKSTER_DARK_TYPES >= GEN_7);
//     the innate keeps the +1 priority but never sets that flag, so its status moves still land on
//     Dark-types — the favorable half, dropping the real ability's only cost. (Because the innate
//     never sets pranksterElevated, the AI's Dark-type avoidance check in src/battle_ai_main.c
//     correctly leaves an innate Prankster's status moves unpenalized — no wiring needed there.)
//     The doubles Psychic-Terrain heuristic in src/battle_ai_field_statuses.c IS made innate-aware
//     (Psychic Terrain blanks priority moves regardless of source): beside its chosen-only
//     AI_IsAbilityOnSide(ABILITY_PRANKSTER) reads, the fork helper AI_IsInnateOnSide() also credits
//     an innate Prankster, so the AI values/avoids the terrain for an innate-Prankster side too.
//     Two species groups: the canon Prankster users (the trickster lines keep the signature priority
//     no matter which slot the build picks; Mega/regional/Gmax forms are listed only where the form's
//     ability data ALSO carries Prankster — Grimmsnarl-Gmax yes; Banette/Sableye/Meowstic Megas and
//     the Therian formes have a DIFFERENT signature ability, so they are omitted like the Natural Cure
//     rule), plus a deliberately small, on-theme flavor set lacking the real ability (Hoopa the
//     "Mischief Pokémon," the playful Aipom line, the illusion-trickster Unovan Zorua line — the
//     flavor set is narrower than other abilities' because Prankster's +1 priority is potent).
//     Cottonee/Whimsicott, Klefki and Hoopa are also innate-Levitate floaters, so they take the
//     combined INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER) list below.
//   - ABILITY_OVERGROW / ABILITY_BLAZE / ABILITY_TORRENT / ABILITY_SWARM — the "pinch" abilities:
//     +50% to Grass/Fire/Water/Bug moves respectively while the holder is low on HP. Handled by an
//     additive block in CalcAttackStat (src/battle_util.c), beside (not inside) the chosen-ability
//     switch. DELIBERATE DIVERGENCE: an innate pinch ability is a *pure boon* that LATCHES. A real
//     pinch ability only boosts while the holder is *currently* <=1/3 HP, so healing back up (notably
//     an innate Regenerator's switch-out heal, but also Leftovers / a Berry) strips the boost; the
//     innate instead sets gBattleStruct's per-mon reachedPinchHp flag the first time the holder hits
//     <=1/3 HP (latched each end-of-turn in src/battle_end_turn.c) and keeps the boost for the rest of
//     the battle. The chosen-ability switch case is left untouched (a real pinch ability stays vanilla),
//     and the block's `chosen != ABILITY_X` guard means a starter running its real pinch ability never
//     double-applies. Suppression parity holds via IsInnateActive() (feature flag + Gastro Acid /
//     Neutralizing Gas / not-on-field); pinch abilities aren't breakable, so Mold Breaker never touches
//     them, same as the real ability. AI is correct for FREE: the boost lives in the shared damage calc
//     (CalcAttackStat), which the AI runs keyed off the real battler via IsInnateActive(), so it both
//     threatens and respects an innate pinch boost. Canon-only (no flavor picks): every species whose
//     ability data carries the pinch ability in any slot, so the signature survives whichever slot a
//     build picks (a Chlorophyll Venusaur / Solar Power Charizard / Protean Greninja keeps its boost);
//     forms are listed only where the form's ability data still carries it (Megas swap to Thick Fat /
//     Tough Claws / Drought / Mega Launcher / etc. and are omitted; Gigantamax forms and the Hisuian
//     starters keep theirs). The Bulbasaur, Chikorita and Fuecoco lines and Volbeat already carry other
//     innates, so they take a combined INNATES(...) list with the pinch ability added.
//   - ABILITY_SWIFT_SWIM / ABILITY_CHLOROPHYLL / ABILITY_SAND_RUSH / ABILITY_SLUSH_RUSH — the weather
//     speed-doublers: x2 Speed in rain / harsh sun / sandstorm / snow respectively (Sand Rush also
//     shrugs off sandstorm chip damage, like the real ability). Handled at the single speed-calc site
//     GetBattlerTotalSpeedStat (src/battle_main.c): each `ability == ABILITY_X` test gains an
//     `|| IsInnateActive(battler, ABILITY_X)` clause, so an innate holder doubles exactly like the real
//     ability. Sand Rush's sandstorm-damage immunity is mirrored at the end-turn damage site
//     (src/battle_end_turn.c) and the AI's two sandstorm-damage predictors (DoesBattlerTakeSandstormDamage
//     in src/battle_ai_util.c, GetSwitchinWeatherImpact in src/battle_ai_switch.c). NO pure-boon divergence:
//     a weather speed-doubler is a clean upside that never hurts its holder, so each innate is a 1:1 copy.
//     Suppression parity holds via IsInnateActive() (none of the four is breakable, so Mold Breaker never
//     touches them — same as the real ability). AI is innate-aware: turn-order prediction runs the same
//     GetBattlerTotalSpeedStat keyed off the real battler, so the AI both threatens and respects an innate
//     doubler's speed for FREE (innates are species-derived, so this never leaks a hidden chosen ability);
//     the AI's weather-SETTING heuristics (DoesAbilityBenefitFromWeather in src/battle_ai_field_statuses.c,
//     DoesAbilityBenefitFromSunOrRain in src/battle_ai_main.c) also credit an innate doubler so the AI sets
//     the matching weather to enable it. Canon-only (no flavor picks — a x2-Speed weather sweeper is potent,
//     so like the pinch abilities the set stays to species whose ability data carries it in any slot): the
//     signature survives whichever slot a build picks (a Rain Dish Ludicolo / Sand Force Excadrill keeps its
//     doubling), and forms are listed only where the form's ability data still carries it (Mega Swampert,
//     Gigantamax Drednaw/Venusaur, Hisuian Qwilfish/Lilligant/Overqwil, the seasonal Deerling/Sawsbuck).
//     Beartic carries BOTH Swift Swim (primary) and Slush Rush (HA), so it takes the combined pair. Many
//     species already carry other innates (the Bulbasaur/Tangela/Bellossom/Cottonee/Psyduck/Relicanth/...
//     lines), so they take a combined INNATES(...) list with the speed-doubler added.
//   - ABILITY_FILTER — reduces the damage the holder takes from supereffective moves by 25%, handled
//     at the single defensive calc site GetDefenderAbilitiesModifier (src/battle_util.c): an
//     IsInnateActive() clause beside the chosen-ability Filter / Solid Rock / Prism Armor switch case
//     applies the 0.75 modifier (guarded against those three so it never double-applies, and stacking
//     correctly with any other defender-ability modifier). A pure calc-modifier passive like Unaware:
//     no script / pop-up / driver, and the innate is NOT recorded as identity. NO pure-boon divergence:
//     Filter is a clean upside that never hurts its holder, so the innate is a 1:1 copy of the real
//     ability. Suppression parity holds via IsInnateActive(): Filter is breakable, so an attacker's Mold
//     Breaker pierces an innate Filter exactly as it would the real ability. AI is correct for FREE: the
//     reduction lives in the shared damage calc the AI runs keyed off the real battler (like Unaware's
//     stat-ignore), so the AI both threatens and respects an innate Filter on-field; the off-field
//     switch-in damage prediction is left unwired (the Unaware scope call — a 25% reduction is not a
//     KO-flipping immunity like Levitate/Sturdy). Canon-only (no flavor picks — the Filter theme is hard
//     to attribute beyond its real users): every species whose ability data carries Filter in any slot
//     (Mr. Mime and Mime Jr.'s slot-1 Filter, Revavroom's HA, Mega Aggron whose Mega ability data is
//     Filter), so the signature survives whichever slot a build picks. Mega Aggron already carries innate
//     Sturdy (persisting from base Aggron), so it takes the combined INNATES(STURDY, FILTER) list.
// Do NOT give a species an innate that is not on this list: nothing would honor it
// (no effect site activates it), so it would silently do nothing.

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// Most species have a single innate, so they share one of these ABILITY_NONE-terminated arrays.
static const enum Ability sInnateLevitate[] = { ABILITY_LEVITATE, ABILITY_NONE };
static const enum Ability sInnateRegenerator[] = { ABILITY_REGENERATOR, ABILITY_NONE };
static const enum Ability sInnateUnaware[] = { ABILITY_UNAWARE, ABILITY_NONE };
static const enum Ability sInnateSturdy[] = { ABILITY_STURDY, ABILITY_NONE };
static const enum Ability sInnateNaturalCure[] = { ABILITY_NATURAL_CURE, ABILITY_NONE };
static const enum Ability sInnatePrankster[] = { ABILITY_PRANKSTER, ABILITY_NONE };
static const enum Ability sInnateOvergrow[] = { ABILITY_OVERGROW, ABILITY_NONE };
static const enum Ability sInnateBlaze[] = { ABILITY_BLAZE, ABILITY_NONE };
static const enum Ability sInnateTorrent[] = { ABILITY_TORRENT, ABILITY_NONE };
static const enum Ability sInnateSwarm[] = { ABILITY_SWARM, ABILITY_NONE };
static const enum Ability sInnateSwiftSwim[] = { ABILITY_SWIFT_SWIM, ABILITY_NONE };
static const enum Ability sInnateChlorophyll[] = { ABILITY_CHLOROPHYLL, ABILITY_NONE };
static const enum Ability sInnateSandRush[] = { ABILITY_SAND_RUSH, ABILITY_NONE };
static const enum Ability sInnateSlushRush[] = { ABILITY_SLUSH_RUSH, ABILITY_NONE };
static const enum Ability sInnateFilter[] = { ABILITY_FILTER, ABILITY_NONE };

// A species with SEVERAL innates lists them inline at its row with INNATES(...) instead of needing a
// named combination array per pairing (which doesn't scale as the allowlist grows). The compound
// literal has static storage at file scope; the terminator is appended automatically.
#define INNATES(...) (const enum Ability[]){ __VA_ARGS__, ABILITY_NONE }

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // Sorted by National Pokédex number (shown in each row's trailing comment); formes share
    // their base's number and follow it. A base-form constant (e.g. SPECIES_CASTFORM) is listed
    // alongside its forme constants so the lookup matches whichever value is queried. Most rows use
    // a shared single-innate array (sInnate*); a species with several innates uses INNATES(...).
    // The per-ability rationale (canon vs flavor picks, included/omitted formes) is documented in
    // the file header above; per-row notes record the species-specific reasoning.

    // ----- Gen 1 -----
    { SPECIES_BULBASAUR,                INNATES(ABILITY_NATURAL_CURE, ABILITY_REGENERATOR, ABILITY_OVERGROW, ABILITY_CHLOROPHYLL) }, // 1 canon Overgrow + canon Chlorophyll (HA)
    { SPECIES_IVYSAUR,                  INNATES(ABILITY_NATURAL_CURE, ABILITY_REGENERATOR, ABILITY_OVERGROW, ABILITY_CHLOROPHYLL) }, // 2
    { SPECIES_VENUSAUR,                 INNATES(ABILITY_NATURAL_CURE, ABILITY_REGENERATOR, ABILITY_OVERGROW, ABILITY_CHLOROPHYLL) }, // 3 both real abilities now innate -> chosen Thick Fat via override
    { SPECIES_VENUSAUR_GMAX,            INNATES(ABILITY_OVERGROW, ABILITY_CHLOROPHYLL) }, // 3 Gmax keeps both
    { SPECIES_CHARMANDER,               sInnateBlaze }, // 4
    { SPECIES_CHARMELEON,               sInnateBlaze }, // 5
    { SPECIES_CHARIZARD,                sInnateBlaze }, // 6 Mega-X is Tough Claws, Mega-Y is Drought; both omitted
    { SPECIES_CHARIZARD_GMAX,           sInnateBlaze }, // 6
    { SPECIES_SQUIRTLE,                 sInnateTorrent }, // 7
    { SPECIES_WARTORTLE,                sInnateTorrent }, // 8
    { SPECIES_BLASTOISE,                sInnateTorrent }, // 9 Mega is Mega Launcher, omitted
    { SPECIES_BLASTOISE_GMAX,           sInnateTorrent }, // 9
    { SPECIES_BEEDRILL,                 sInnateSwarm }, // 15 Mega is Adaptability, omitted
    { SPECIES_SANDSHREW,                sInnateSandRush }, // 27 Sand Rush is the HA
    { SPECIES_SANDSHREW_ALOLA,          sInnateSlushRush }, // 27 Alolan line's HA is Slush Rush
    { SPECIES_SANDSLASH,                sInnateSandRush }, // 28
    { SPECIES_SANDSLASH_ALOLA,          sInnateSlushRush }, // 28
    { SPECIES_CLEFABLE,                 sInnateUnaware }, // 36 Unaware is the HA
    { SPECIES_CLEFABLE_MEGA,            sInnateUnaware }, // 36 innate persists through the Mega
    { SPECIES_ODDISH,                   sInnateChlorophyll }, // 43 Chlorophyll is the primary
    { SPECIES_GLOOM,                    sInnateChlorophyll }, // 44
    { SPECIES_VILEPLUME,                sInnateChlorophyll }, // 45
    { SPECIES_PSYDUCK,                  INNATES(ABILITY_UNAWARE, ABILITY_SWIFT_SWIM) }, // 54 dazed (flavor Unaware) + canon Swift Swim (HA)
    { SPECIES_GOLDUCK,                  INNATES(ABILITY_UNAWARE, ABILITY_SWIFT_SWIM) }, // 55 flavor Unaware + canon Swift Swim (HA)
    { SPECIES_POLIWAG,                  sInnateSwiftSwim }, // 60 Swift Swim is the HA
    { SPECIES_POLIWHIRL,                sInnateSwiftSwim }, // 61
    { SPECIES_POLIWRATH,                sInnateSwiftSwim }, // 62
    { SPECIES_BELLSPROUT,               sInnateChlorophyll }, // 69 Chlorophyll is the primary
    { SPECIES_WEEPINBELL,               sInnateChlorophyll }, // 70
    { SPECIES_VICTREEBEL,               sInnateChlorophyll }, // 71
    { SPECIES_GEODUDE,                  sInnateSturdy }, // 74
    { SPECIES_GEODUDE_ALOLA,            sInnateSturdy }, // 74
    { SPECIES_GRAVELER,                 sInnateSturdy }, // 75
    { SPECIES_GRAVELER_ALOLA,           sInnateSturdy }, // 75
    { SPECIES_GOLEM,                    sInnateSturdy }, // 76
    { SPECIES_GOLEM_ALOLA,              sInnateSturdy }, // 76
    { SPECIES_SLOWPOKE,                 sInnateRegenerator }, // 79
    { SPECIES_SLOWPOKE_GALAR,           sInnateRegenerator }, // 79
    { SPECIES_SLOWBRO,                  sInnateRegenerator }, // 80
    { SPECIES_SLOWBRO_MEGA,             sInnateRegenerator }, // 80 canon Mega is Shell Armor; innate persists through the Mega
    { SPECIES_SLOWBRO_GALAR,            sInnateRegenerator }, // 80
    { SPECIES_MAGNEMITE,                INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // 81 flavor Levitate (hovers) + canon Sturdy (slot 1)
    { SPECIES_MAGNETON,                 INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // 82 flavor Levitate + canon Sturdy
    { SPECIES_SHELLDER,                 sInnateSturdy }, // 90 its shell "even a missile can't break"
    { SPECIES_CLOYSTER,                 sInnateSturdy }, // 91 "Its shell is harder than diamond"
    { SPECIES_GASTLY,                   sInnateLevitate }, // 92
    { SPECIES_HAUNTER,                  sInnateLevitate }, // 93
    { SPECIES_GENGAR,                   sInnateLevitate }, // 94 floats; primary is Cursed Body at GEN_LATEST, so observable
    { SPECIES_GENGAR_GMAX,              sInnateLevitate }, // 94 a Gigantamaxed Gengar still floats (NOT Gengar-Mega, which is grounded)
    { SPECIES_ONIX,                     sInnateSturdy }, // 95
    { SPECIES_EXEGGCUTE,                sInnateChlorophyll }, // 102 Chlorophyll is the primary
    { SPECIES_EXEGGUTOR,                sInnateChlorophyll }, // 103 Kanto form (Alolan is Frisk/Harvest, omitted)
    { SPECIES_KOFFING,                  sInnateLevitate }, // 109
    { SPECIES_WEEZING,                  sInnateLevitate }, // 110
    { SPECIES_WEEZING_GALAR,            sInnateLevitate }, // 110 Misty Surge (HA) build floats AND reaps its terrain; Poison-type builds clear Toxic Spikes
    { SPECIES_CHANSEY,                  sInnateNaturalCure }, // 113
    { SPECIES_TANGELA,                  INNATES(ABILITY_REGENERATOR, ABILITY_CHLOROPHYLL) }, // 114 canon Regen (HA) + canon Chlorophyll (primary)
    { SPECIES_HORSEA,                   sInnateSwiftSwim }, // 116 Swift Swim is the HA
    { SPECIES_GOLDEEN,                  sInnateSwiftSwim }, // 118 Swift Swim is the primary
    { SPECIES_SEAKING,                  sInnateSwiftSwim }, // 119
    { SPECIES_STARYU,                   INNATES(ABILITY_REGENERATOR, ABILITY_NATURAL_CURE) }, // 120 flavor Regen (regrows from its core) + canon Natural Cure
    { SPECIES_STARMIE,                  INNATES(ABILITY_REGENERATOR, ABILITY_NATURAL_CURE) }, // 121 flavor Regen + canon Natural Cure
    { SPECIES_MR_MIME,                  sInnateFilter }, // 122 Filter is the secondary ability (Kanto form; Galarian Mr. Mime lacks it)
    { SPECIES_SCYTHER,                  sInnateSwarm }, // 123
    { SPECIES_MAGIKARP,                 sInnateSwiftSwim }, // 129 Swift Swim is the primary
    { SPECIES_PORYGON,                  sInnateLevitate }, // 137
    { SPECIES_OMANYTE,                  sInnateSwiftSwim }, // 138 Swift Swim is the HA
    { SPECIES_OMASTAR,                  sInnateSwiftSwim }, // 139
    { SPECIES_KABUTO,                   sInnateSwiftSwim }, // 140 Swift Swim is the primary
    { SPECIES_KABUTOPS,                 sInnateSwiftSwim }, // 141
    { SPECIES_SNORLAX,                  sInnateUnaware }, // 143 too busy eating/sleeping to be bothered
    { SPECIES_SNORLAX_GMAX,             sInnateUnaware }, // 143 flavor
    { SPECIES_MEWTWO,                   sInnateLevitate }, // 150
    { SPECIES_MEWTWO_MEGA_Y,            sInnateLevitate }, // 150 Mega-X is a grounded bruiser, omitted
    { SPECIES_MEW,                      sInnateLevitate }, // 151

    // ----- Gen 2 -----
    { SPECIES_CHIKORITA,                INNATES(ABILITY_NATURAL_CURE, ABILITY_OVERGROW) }, // 152 flavor Natural Cure + canon Overgrow
    { SPECIES_BAYLEEF,                  INNATES(ABILITY_NATURAL_CURE, ABILITY_OVERGROW) }, // 153
    { SPECIES_MEGANIUM,                 INNATES(ABILITY_NATURAL_CURE, ABILITY_OVERGROW) }, // 154
    { SPECIES_CYNDAQUIL,                sInnateBlaze }, // 155
    { SPECIES_QUILAVA,                  sInnateBlaze }, // 156
    { SPECIES_TYPHLOSION,               sInnateBlaze }, // 157
    { SPECIES_TYPHLOSION_HISUI,         sInnateBlaze }, // 157
    { SPECIES_TOTODILE,                 sInnateTorrent }, // 158
    { SPECIES_CROCONAW,                 sInnateTorrent }, // 159
    { SPECIES_FERALIGATR,               sInnateTorrent }, // 160
    { SPECIES_LEDYBA,                   sInnateSwarm }, // 165
    { SPECIES_LEDIAN,                   sInnateSwarm }, // 166
    { SPECIES_SPINARAK,                 sInnateSwarm }, // 167
    { SPECIES_ARIADOS,                  sInnateSwarm }, // 168
    { SPECIES_BELLOSSOM,                INNATES(ABILITY_NATURAL_CURE, ABILITY_CHLOROPHYLL) }, // 182 flavor Natural Cure (healing dance) + canon Chlorophyll (primary)
    { SPECIES_SUDOWOODO,                sInnateSturdy }, // 185
    { SPECIES_HOPPIP,                   sInnateChlorophyll }, // 187 Chlorophyll is the primary
    { SPECIES_SKIPLOOM,                 sInnateChlorophyll }, // 188
    { SPECIES_JUMPLUFF,                 sInnateChlorophyll }, // 189
    { SPECIES_AIPOM,                    sInnatePrankster }, // 190 playful, mischievous tail-pranking monkey
    { SPECIES_SUNKERN,                  sInnateChlorophyll }, // 191 Chlorophyll is the primary
    { SPECIES_SUNFLORA,                 sInnateChlorophyll }, // 192
    { SPECIES_WOOPER,                   INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // 194 flavor Regen (axolotl limb regrowth) + canon Unaware (HA)
    { SPECIES_WOOPER_PALDEA,            INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // 194 flavor Regen (Paldean axolotl) + canon Unaware (HA)
    { SPECIES_QUAGSIRE,                 INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // 195 flavor Regen + canon Unaware (HA)
    { SPECIES_MURKROW,                  sInnatePrankster }, // 198 Prankster is the HA (Honchkrow loses it, so it is not listed)
    { SPECIES_SLOWKING,                 sInnateRegenerator }, // 199
    { SPECIES_SLOWKING_GALAR,           sInnateRegenerator }, // 199
    { SPECIES_MISDREAVUS,               sInnateLevitate }, // 200
    { SPECIES_UNOWN,                    sInnateLevitate }, // 201
    { SPECIES_UNOWN_B,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_C,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_D,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_E,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_F,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_G,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_H,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_I,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_J,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_K,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_L,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_M,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_N,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_O,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_P,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_Q,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_R,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_S,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_T,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_U,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_V,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_W,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_X,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_Y,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_Z,                  sInnateLevitate }, // 201
    { SPECIES_UNOWN_EXCLAMATION,        sInnateLevitate }, // 201
    { SPECIES_UNOWN_QUESTION,           sInnateLevitate }, // 201
    { SPECIES_PINECO,                   sInnateSturdy }, // 204
    { SPECIES_FORRETRESS,               sInnateSturdy }, // 205
    { SPECIES_STEELIX,                  sInnateSturdy }, // 208
    { SPECIES_STEELIX_MEGA,             sInnateSturdy }, // 208 canon Mega is Sand Force; innate persists through the Mega
    { SPECIES_QWILFISH,                 sInnateSwiftSwim }, // 211 Swift Swim is the HA (Kanto/Johto form)
    { SPECIES_QWILFISH_HISUI,           sInnateSwiftSwim }, // 211 Hisuian form keeps Swift Swim (HA)
    { SPECIES_SCIZOR,                   sInnateSwarm }, // 212 Mega is Technician, omitted
    { SPECIES_SHUCKLE,                  sInnateSturdy }, // 213
    { SPECIES_HERACROSS,                sInnateSwarm }, // 214 Mega is Skill Link, omitted
    { SPECIES_CORSOLA,                  INNATES(ABILITY_REGENERATOR, ABILITY_NATURAL_CURE) }, // 222 canon Regen (HA) + canon Natural Cure
    { SPECIES_MANTINE,                  sInnateSwiftSwim }, // 226 Swift Swim is the HA
    { SPECIES_SKARMORY,                 sInnateSturdy }, // 227
    { SPECIES_SKARMORY_MEGA,            sInnateSturdy }, // 227 innate persists through the Mega
    { SPECIES_KINGDRA,                  sInnateSwiftSwim }, // 230 Swift Swim is the primary
    { SPECIES_DONPHAN,                  sInnateSturdy }, // 232
    { SPECIES_PORYGON2,                 sInnateLevitate }, // 233
    { SPECIES_BLISSEY,                  sInnateNaturalCure }, // 242
    { SPECIES_HO_OH,                    sInnateRegenerator }, // 250
    { SPECIES_CELEBI,                   INNATES(ABILITY_LEVITATE, ABILITY_NATURAL_CURE) }, // 251 floats + canon Natural Cure (its sole ability)

    // ----- Gen 3 -----
    { SPECIES_TREECKO,                  sInnateOvergrow }, // 252
    { SPECIES_GROVYLE,                  sInnateOvergrow }, // 253
    { SPECIES_SCEPTILE,                 sInnateOvergrow }, // 254 Mega is Lightning Rod, omitted
    { SPECIES_TORCHIC,                  sInnateBlaze }, // 255
    { SPECIES_COMBUSKEN,                sInnateBlaze }, // 256
    { SPECIES_BLAZIKEN,                 sInnateBlaze }, // 257 Mega is Speed Boost, omitted
    { SPECIES_MUDKIP,                   sInnateTorrent }, // 258
    { SPECIES_MARSHTOMP,                sInnateTorrent }, // 259
    { SPECIES_SWAMPERT,                 sInnateTorrent }, // 260 base is Torrent; Mega's ability data is Swift Swim (next row)
    { SPECIES_SWAMPERT_MEGA,            sInnateSwiftSwim }, // 260 Mega Swampert's ability is Swift Swim
    { SPECIES_BEAUTIFLY,                sInnateSwarm }, // 267
    { SPECIES_LOTAD,                    sInnateSwiftSwim }, // 270 Swift Swim is the primary
    { SPECIES_LOMBRE,                   sInnateSwiftSwim }, // 271
    { SPECIES_LUDICOLO,                 sInnateSwiftSwim }, // 272
    { SPECIES_SEEDOT,                   sInnateChlorophyll }, // 273 Chlorophyll is the primary
    { SPECIES_NUZLEAF,                  sInnateChlorophyll }, // 274
    { SPECIES_SHIFTRY,                  sInnateChlorophyll }, // 275
    { SPECIES_SURSKIT,                  sInnateSwiftSwim }, // 283 Swift Swim is the primary
    { SPECIES_SHEDINJA,                 sInnateLevitate }, // 292
    { SPECIES_NOSEPASS,                 sInnateSturdy }, // 299
    { SPECIES_SABLEYE,                  sInnatePrankster }, // 302 Prankster is the HA (Mega Sableye is Magic Bounce, omitted)
    { SPECIES_ARON,                     sInnateSturdy }, // 304
    { SPECIES_LAIRON,                   sInnateSturdy }, // 305
    { SPECIES_AGGRON,                   sInnateSturdy }, // 306
    { SPECIES_AGGRON_MEGA,              INNATES(ABILITY_STURDY, ABILITY_FILTER) }, // 306 innate Sturdy persists from base; Mega's ability data is Filter (canon)
    { SPECIES_VOLBEAT,                  INNATES(ABILITY_PRANKSTER, ABILITY_SWARM) }, // 313 Prankster is the HA, Swarm the secondary
    { SPECIES_ILLUMISE,                 sInnatePrankster }, // 314 Prankster is the HA
    { SPECIES_ROSELIA,                  sInnateNaturalCure }, // 315
    { SPECIES_NUMEL,                    sInnateUnaware }, // 322 Pokédex: so dull-witted it doesn't notice being hit
    { SPECIES_CAMERUPT,                 sInnateUnaware }, // 323 flavor
    { SPECIES_CAMERUPT_MEGA,            sInnateUnaware }, // 323 flavor; innate persists through the Mega
    { SPECIES_VIBRAVA,                  sInnateLevitate }, // 329
    { SPECIES_FLYGON,                   sInnateLevitate }, // 330
    { SPECIES_SWABLU,                   sInnateNaturalCure }, // 333
    { SPECIES_ALTARIA,                  sInnateNaturalCure }, // 334 Mega is Pixilate, so the innate is base-form only
    { SPECIES_LUNATONE,                 sInnateLevitate }, // 337
    { SPECIES_SOLROCK,                  sInnateLevitate }, // 338
    { SPECIES_BALTOY,                   sInnateLevitate }, // 343
    { SPECIES_CLAYDOL,                  sInnateLevitate }, // 344
    { SPECIES_ANORITH,                  sInnateSwiftSwim }, // 347 Swift Swim is the HA
    { SPECIES_ARMALDO,                  sInnateSwiftSwim }, // 348
    { SPECIES_FEEBAS,                   sInnateSwiftSwim }, // 349 Swift Swim is the primary
    { SPECIES_CASTFORM,                 sInnateLevitate }, // 351
    { SPECIES_CASTFORM_NORMAL,          sInnateLevitate }, // 351
    { SPECIES_CASTFORM_SUNNY,           sInnateLevitate }, // 351
    { SPECIES_CASTFORM_RAINY,           sInnateLevitate }, // 351
    { SPECIES_CASTFORM_SNOWY,           sInnateLevitate }, // 351
    { SPECIES_SHUPPET,                  sInnateLevitate }, // 353
    { SPECIES_BANETTE,                  sInnateLevitate }, // 354
    { SPECIES_BANETTE_MEGA,             sInnateLevitate }, // 354
    { SPECIES_DUSKULL,                  sInnateLevitate }, // 355
    { SPECIES_DUSCLOPS,                 sInnateLevitate }, // 356
    { SPECIES_TROPIUS,                  sInnateChlorophyll }, // 357 Chlorophyll is the primary
    { SPECIES_CHIMECHO,                 sInnateLevitate }, // 358
    { SPECIES_CHIMECHO_MEGA,            sInnateLevitate }, // 358
    { SPECIES_GLALIE,                   sInnateLevitate }, // 362
    { SPECIES_GLALIE_MEGA,              sInnateLevitate }, // 362
    { SPECIES_HUNTAIL,                  sInnateSwiftSwim }, // 367 Swift Swim is the primary
    { SPECIES_GOREBYSS,                 sInnateSwiftSwim }, // 368
    { SPECIES_RELICANTH,                INNATES(ABILITY_STURDY, ABILITY_SWIFT_SWIM) }, // 369 canon Sturdy (HA) + canon Swift Swim (primary)
    { SPECIES_LUVDISC,                  sInnateSwiftSwim }, // 370 Swift Swim is the primary
    { SPECIES_REGIROCK,                 sInnateSturdy }, // 377
    { SPECIES_LATIAS,                   sInnateLevitate }, // 380
    { SPECIES_LATIAS_MEGA,              sInnateLevitate }, // 380
    { SPECIES_LATIOS,                   sInnateLevitate }, // 381
    { SPECIES_LATIOS_MEGA,              sInnateLevitate }, // 381
    { SPECIES_JIRACHI,                  sInnateLevitate }, // 385
    { SPECIES_DEOXYS,                   sInnateLevitate }, // 386
    { SPECIES_DEOXYS_NORMAL,            sInnateLevitate }, // 386
    { SPECIES_DEOXYS_ATTACK,            sInnateLevitate }, // 386
    { SPECIES_DEOXYS_DEFENSE,           sInnateLevitate }, // 386
    { SPECIES_DEOXYS_SPEED,             sInnateLevitate }, // 386

    // ----- Gen 4 -----
    { SPECIES_TURTWIG,                  sInnateOvergrow }, // 387
    { SPECIES_GROTLE,                   sInnateOvergrow }, // 388
    { SPECIES_TORTERRA,                 sInnateOvergrow }, // 389
    { SPECIES_CHIMCHAR,                 sInnateBlaze }, // 390
    { SPECIES_MONFERNO,                 sInnateBlaze }, // 391
    { SPECIES_INFERNAPE,                sInnateBlaze }, // 392
    { SPECIES_PIPLUP,                   sInnateTorrent }, // 393
    { SPECIES_PRINPLUP,                 sInnateTorrent }, // 394
    { SPECIES_EMPOLEON,                 sInnateTorrent }, // 395
    { SPECIES_BIDOOF,                   sInnateUnaware }, // 399
    { SPECIES_BIBAREL,                  sInnateUnaware }, // 400
    { SPECIES_KRICKETUNE,               sInnateSwarm }, // 402
    { SPECIES_BUDEW,                    sInnateNaturalCure }, // 406
    { SPECIES_ROSERADE,                 sInnateNaturalCure }, // 407
    { SPECIES_SHIELDON,                 sInnateSturdy }, // 410
    { SPECIES_BASTIODON,                sInnateSturdy }, // 411
    { SPECIES_BUIZEL,                   sInnateSwiftSwim }, // 418 Swift Swim is the primary
    { SPECIES_FLOATZEL,                 sInnateSwiftSwim }, // 419
    { SPECIES_CHERUBI,                  sInnateChlorophyll }, // 420 Chlorophyll is the primary
    { SPECIES_AMBIPOM,                  sInnatePrankster }, // 424 nimble and just as mischievous as Aipom
    { SPECIES_MISMAGIUS,                sInnateLevitate }, // 429
    { SPECIES_CHINGLING,                sInnateLevitate }, // 433
    { SPECIES_BRONZOR,                  sInnateLevitate }, // 436
    { SPECIES_BRONZONG,                 sInnateLevitate }, // 437
    { SPECIES_BONSLY,                   sInnateSturdy }, // 438
    { SPECIES_MIME_JR,                  sInnateFilter }, // 439 Filter is the secondary ability
    { SPECIES_HAPPINY,                  sInnateNaturalCure }, // 440
    { SPECIES_MUNCHLAX,                 sInnateUnaware }, // 446 flavor: only ever thinks about food
    { SPECIES_RIOLU,                    sInnatePrankster }, // 447 Prankster is the HA (Lucario loses it, so it is not listed)
    { SPECIES_CARNIVINE,                sInnateLevitate }, // 455
    { SPECIES_FINNEON,                  sInnateSwiftSwim }, // 456 Swift Swim is the primary
    { SPECIES_LUMINEON,                 sInnateSwiftSwim }, // 457
    { SPECIES_MANTYKE,                  sInnateSwiftSwim }, // 458 Swift Swim is the HA
    { SPECIES_MAGNEZONE,                INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // 462 flavor Levitate + canon Sturdy
    { SPECIES_TANGROWTH,                INNATES(ABILITY_REGENERATOR, ABILITY_CHLOROPHYLL) }, // 465 canon Regen (HA) + canon Chlorophyll (primary)
    { SPECIES_LEAFEON,                  sInnateChlorophyll }, // 470 Chlorophyll is the HA
    { SPECIES_PORYGON_Z,                sInnateLevitate }, // 474
    { SPECIES_PROBOPASS,                sInnateSturdy }, // 476
    { SPECIES_FROSLASS,                 sInnateLevitate }, // 478
    { SPECIES_FROSLASS_MEGA,            sInnateLevitate }, // 478
    { SPECIES_ROTOM,                    sInnateLevitate }, // 479
    { SPECIES_ROTOM_HEAT,               sInnateLevitate }, // 479
    { SPECIES_ROTOM_WASH,               sInnateLevitate }, // 479
    { SPECIES_ROTOM_FROST,              sInnateLevitate }, // 479
    { SPECIES_ROTOM_FAN,                sInnateLevitate }, // 479
    { SPECIES_ROTOM_MOW,                sInnateLevitate }, // 479
    { SPECIES_UXIE,                     sInnateLevitate }, // 480
    { SPECIES_MESPRIT,                  sInnateLevitate }, // 481
    { SPECIES_AZELF,                    sInnateLevitate }, // 482
    { SPECIES_GIRATINA_ALTERED,         sInnateLevitate }, // 487 Origin form is already covered above
    { SPECIES_GIRATINA_ORIGIN,          sInnateLevitate }, // 487
    { SPECIES_CRESSELIA,                sInnateLevitate }, // 488
    { SPECIES_DARKRAI,                  sInnateLevitate }, // 491
    { SPECIES_DARKRAI_MEGA,             sInnateLevitate }, // 491
    { SPECIES_SHAYMIN_LAND,             sInnateNaturalCure }, // 492 Sky forme is Serene Grace (SPECIES_SHAYMIN aliases Land)

    // ----- Gen 5 -----
    { SPECIES_SNIVY,                    sInnateOvergrow }, // 495
    { SPECIES_SERVINE,                  sInnateOvergrow }, // 496
    { SPECIES_SERPERIOR,                sInnateOvergrow }, // 497
    { SPECIES_TEPIG,                    sInnateBlaze }, // 498
    { SPECIES_PIGNITE,                  sInnateBlaze }, // 499
    { SPECIES_EMBOAR,                   sInnateBlaze }, // 500
    { SPECIES_OSHAWOTT,                 sInnateTorrent }, // 501
    { SPECIES_DEWOTT,                   sInnateTorrent }, // 502
    { SPECIES_SAMUROTT,                 sInnateTorrent }, // 503
    { SPECIES_SAMUROTT_HISUI,           sInnateTorrent }, // 503
    { SPECIES_HERDIER,                  sInnateSandRush }, // 507 Sand Rush is the HA
    { SPECIES_STOUTLAND,                sInnateSandRush }, // 508
    { SPECIES_PURRLOIN,                 sInnatePrankster }, // 509 Prankster is the HA across the line
    { SPECIES_LIEPARD,                  sInnatePrankster }, // 510
    { SPECIES_PANSAGE,                  sInnateOvergrow }, // 511 Overgrow is the HA
    { SPECIES_SIMISAGE,                 sInnateOvergrow }, // 512
    { SPECIES_PANSEAR,                  sInnateBlaze }, // 513 Blaze is the HA
    { SPECIES_SIMISEAR,                 sInnateBlaze }, // 514
    { SPECIES_PANPOUR,                  sInnateTorrent }, // 515 Torrent is the HA
    { SPECIES_SIMIPOUR,                 sInnateTorrent }, // 516
    { SPECIES_MUNNA,                    sInnateLevitate }, // 517
    { SPECIES_MUSHARNA,                 sInnateLevitate }, // 518
    { SPECIES_ROGGENROLA,               sInnateSturdy }, // 524
    { SPECIES_BOLDORE,                  sInnateSturdy }, // 525
    { SPECIES_GIGALITH,                 sInnateSturdy }, // 526
    { SPECIES_WOOBAT,                   sInnateUnaware }, // 527 Unaware is the primary ability
    { SPECIES_SWOOBAT,                  sInnateUnaware }, // 528
    { SPECIES_DRILBUR,                  sInnateSandRush }, // 529 Sand Rush is the primary
    { SPECIES_EXCADRILL,                sInnateSandRush }, // 530
    { SPECIES_AUDINO,                   sInnateRegenerator }, // 531
    { SPECIES_AUDINO_MEGA,              sInnateRegenerator }, // 531 canon Mega is Healer; innate persists through the Mega
    { SPECIES_TYMPOLE,                  sInnateSwiftSwim }, // 535 Swift Swim is the primary
    { SPECIES_PALPITOAD,                sInnateSwiftSwim }, // 536
    { SPECIES_SEISMITOAD,               sInnateSwiftSwim }, // 537
    { SPECIES_SAWK,                     sInnateSturdy }, // 539
    { SPECIES_SEWADDLE,                 INNATES(ABILITY_SWARM, ABILITY_CHLOROPHYLL) }, // 540 canon Swarm (primary) + canon Chlorophyll (secondary)
    { SPECIES_SWADLOON,                 sInnateChlorophyll }, // 541 Chlorophyll is the secondary (loses Swarm)
    { SPECIES_LEAVANNY,                 INNATES(ABILITY_SWARM, ABILITY_CHLOROPHYLL) }, // 542 canon Swarm (primary) + canon Chlorophyll (secondary)
    { SPECIES_VENIPEDE,                 sInnateSwarm }, // 543
    { SPECIES_WHIRLIPEDE,               sInnateSwarm }, // 544
    { SPECIES_SCOLIPEDE,                sInnateSwarm }, // 545
    { SPECIES_COTTONEE,                 INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER, ABILITY_CHLOROPHYLL) }, // 546 floats + canon Prankster (primary) + canon Chlorophyll (HA)
    { SPECIES_WHIMSICOTT,               INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER, ABILITY_CHLOROPHYLL) }, // 547 floats + canon Prankster (primary) + canon Chlorophyll (HA)
    { SPECIES_PETILIL,                  sInnateChlorophyll }, // 548 Chlorophyll is the primary
    { SPECIES_LILLIGANT,                sInnateChlorophyll }, // 549 Unovan form (Hisuian also has Chlorophyll, next row)
    { SPECIES_LILLIGANT_HISUI,          sInnateChlorophyll }, // 549 Hisuian form keeps Chlorophyll (primary)
    { SPECIES_MARACTUS,                 sInnateChlorophyll }, // 556 Chlorophyll is the secondary
    { SPECIES_DWEBBLE,                  sInnateSturdy }, // 557
    { SPECIES_CRUSTLE,                  sInnateSturdy }, // 558
    { SPECIES_YAMASK,                   sInnateLevitate }, // 562
    { SPECIES_YAMASK_GALAR,             sInnateLevitate }, // 562
    { SPECIES_COFAGRIGUS,               sInnateLevitate }, // 563
    { SPECIES_TIRTOUGA,                 INNATES(ABILITY_STURDY, ABILITY_SWIFT_SWIM) }, // 564 canon Sturdy (primary) + canon Swift Swim (HA)
    { SPECIES_CARRACOSTA,               INNATES(ABILITY_STURDY, ABILITY_SWIFT_SWIM) }, // 565
    { SPECIES_ZORUA,                    sInnatePrankster }, // 570 illusion trickster that "delights in confusing people" (Unovan; Hisuian is vengeful, omitted)
    { SPECIES_ZOROARK,                  sInnatePrankster }, // 571 master of illusions and deception (Unovan)
    { SPECIES_SOLOSIS,                  INNATES(ABILITY_LEVITATE, ABILITY_REGENERATOR) }, // 577 floats + canon Regenerator
    { SPECIES_DUOSION,                  INNATES(ABILITY_LEVITATE, ABILITY_REGENERATOR) }, // 578 floats + canon Regenerator
    { SPECIES_REUNICLUS,                INNATES(ABILITY_LEVITATE, ABILITY_REGENERATOR) }, // 579 floats + canon Regenerator
    { SPECIES_VANILLITE,                sInnateLevitate }, // 582
    { SPECIES_VANILLISH,                sInnateLevitate }, // 583
    { SPECIES_VANILLUXE,                sInnateLevitate }, // 584
    { SPECIES_DEERLING_SPRING,          sInnateChlorophyll }, // 585 Chlorophyll is the primary (SPECIES_DEERLING aliases Spring)
    { SPECIES_DEERLING_SUMMER,          sInnateChlorophyll }, // 585
    { SPECIES_DEERLING_AUTUMN,          sInnateChlorophyll }, // 585
    { SPECIES_DEERLING_WINTER,          sInnateChlorophyll }, // 585
    { SPECIES_SAWSBUCK_SPRING,          sInnateChlorophyll }, // 586 Chlorophyll is the primary (SPECIES_SAWSBUCK aliases Spring)
    { SPECIES_SAWSBUCK_SUMMER,          sInnateChlorophyll }, // 586
    { SPECIES_SAWSBUCK_AUTUMN,          sInnateChlorophyll }, // 586
    { SPECIES_SAWSBUCK_WINTER,          sInnateChlorophyll }, // 586
    { SPECIES_KARRABLAST,               sInnateSwarm }, // 588
    { SPECIES_ESCAVALIER,               sInnateSwarm }, // 589
    { SPECIES_FOONGUS,                  sInnateRegenerator }, // 590
    { SPECIES_AMOONGUSS,                sInnateRegenerator }, // 591
    { SPECIES_FRILLISH,                 sInnateLevitate }, // 592
    { SPECIES_JELLICENT,                sInnateLevitate }, // 593
    { SPECIES_ALOMOMOLA,                sInnateRegenerator }, // 594
    { SPECIES_JOLTIK,                   sInnateSwarm }, // 595
    { SPECIES_GALVANTULA,               sInnateSwarm }, // 596
    { SPECIES_KLINK,                    sInnateLevitate }, // 599
    { SPECIES_KLANG,                    sInnateLevitate }, // 600
    { SPECIES_KLINKLANG,                sInnateLevitate }, // 601
    { SPECIES_TYNAMO,                   sInnateLevitate }, // 602
    { SPECIES_EELEKTRIK,                sInnateLevitate }, // 603
    { SPECIES_EELEKTROSS,               sInnateLevitate }, // 604
    { SPECIES_EELEKTROSS_MEGA,          sInnateLevitate }, // 604
    { SPECIES_ELGYEM,                   sInnateLevitate }, // 605
    { SPECIES_BEHEEYEM,                 sInnateLevitate }, // 606
    { SPECIES_LITWICK,                  sInnateLevitate }, // 607
    { SPECIES_LAMPENT,                  sInnateLevitate }, // 608
    { SPECIES_CHANDELURE,               sInnateLevitate }, // 609
    { SPECIES_CHANDELURE_MEGA,          sInnateLevitate }, // 609
    { SPECIES_CUBCHOO,                  sInnateSlushRush }, // 613 Slush Rush is the HA
    { SPECIES_BEARTIC,                  INNATES(ABILITY_SWIFT_SWIM, ABILITY_SLUSH_RUSH) }, // 614 canon Swift Swim (primary) + canon Slush Rush (HA)
    { SPECIES_CRYOGONAL,                sInnateLevitate }, // 615
    { SPECIES_MIENFOO,                  sInnateRegenerator }, // 619
    { SPECIES_MIENSHAO,                 sInnateRegenerator }, // 620
    { SPECIES_DURANT,                   sInnateSwarm }, // 632
    { SPECIES_HYDREIGON,                sInnateLevitate }, // 635
    { SPECIES_VOLCARONA,                sInnateSwarm }, // 637
    { SPECIES_TORNADUS_INCARNATE,       sInnatePrankster }, // 641 Incarnate forme only (Therian is Regenerator)
    { SPECIES_TORNADUS_THERIAN,         sInnateRegenerator }, // 641 only the Therian forme has Regenerator
    { SPECIES_THUNDURUS_INCARNATE,      sInnatePrankster }, // 642 Incarnate forme only (Therian is Volt Absorb)

    // ----- Gen 6 -----
    { SPECIES_CHESPIN,                  sInnateOvergrow }, // 650
    { SPECIES_QUILLADIN,                sInnateOvergrow }, // 651
    { SPECIES_CHESNAUGHT,               sInnateOvergrow }, // 652
    { SPECIES_FENNEKIN,                 sInnateBlaze }, // 653
    { SPECIES_BRAIXEN,                  sInnateBlaze }, // 654
    { SPECIES_DELPHOX,                  sInnateBlaze }, // 655
    { SPECIES_DELPHOX_MEGA,             sInnateLevitate }, // 655
    { SPECIES_FROAKIE,                  sInnateTorrent }, // 656
    { SPECIES_FROGADIER,                sInnateTorrent }, // 657
    { SPECIES_GRENINJA,                 sInnateTorrent }, // 658 Battle Bond / Protean builds keep the Torrent boost
    { SPECIES_MEOWSTIC_M,               sInnatePrankster }, // 678 male Meowstic's HA (female is Competitive; Megas are Trace)
    { SPECIES_HONEDGE,                  sInnateLevitate }, // 679
    { SPECIES_DOUBLADE,                 sInnateLevitate }, // 680
    { SPECIES_AEGISLASH,                sInnateLevitate }, // 681
    { SPECIES_AEGISLASH_SHIELD,         sInnateLevitate }, // 681
    { SPECIES_AEGISLASH_BLADE,          sInnateLevitate }, // 681
    { SPECIES_INKAY,                    sInnateLevitate }, // 686 Inkay floats; Malamar stands, so omitted
    { SPECIES_TYRUNT,                   sInnateSturdy }, // 696
    { SPECIES_CARBINK,                  INNATES(ABILITY_LEVITATE, ABILITY_STURDY) }, // 703 floats + canon Sturdy
    { SPECIES_KLEFKI,                   INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER) }, // 707 floats (key ring) + canon Prankster (primary)
    { SPECIES_PHANTUMP,                 sInnateNaturalCure }, // 708
    { SPECIES_TREVENANT,                sInnateNaturalCure }, // 709
    { SPECIES_PUMPKABOO,                sInnateLevitate }, // 710
    { SPECIES_PUMPKABOO_AVERAGE,        sInnateLevitate }, // 710
    { SPECIES_PUMPKABOO_SMALL,          sInnateLevitate }, // 710
    { SPECIES_PUMPKABOO_LARGE,          sInnateLevitate }, // 710
    { SPECIES_PUMPKABOO_SUPER,          sInnateLevitate }, // 710
    { SPECIES_GOURGEIST,                sInnateLevitate }, // 711
    { SPECIES_GOURGEIST_AVERAGE,        sInnateLevitate }, // 711
    { SPECIES_GOURGEIST_SMALL,          sInnateLevitate }, // 711
    { SPECIES_GOURGEIST_LARGE,          sInnateLevitate }, // 711
    { SPECIES_GOURGEIST_SUPER,          sInnateLevitate }, // 711
    { SPECIES_BERGMITE,                 sInnateSturdy }, // 712
    { SPECIES_AVALUGG,                  sInnateSturdy }, // 713
    { SPECIES_AVALUGG_HISUI,            sInnateSturdy }, // 713
    { SPECIES_ZYGARDE,                  sInnateRegenerator }, // 718 flavor: a colony of cells that reassemble
    { SPECIES_ZYGARDE_50,               sInnateRegenerator }, // 718 flavor
    { SPECIES_ZYGARDE_10,               sInnateRegenerator }, // 718 flavor
    { SPECIES_ZYGARDE_COMPLETE,         sInnateRegenerator }, // 718 flavor
    { SPECIES_DIANCIE,                  sInnateLevitate }, // 719
    { SPECIES_DIANCIE_MEGA,             sInnateLevitate }, // 719
    { SPECIES_HOOPA,                    INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER) }, // 720 floats + flavor Prankster (the "Mischief Pokémon," ring trickery)
    { SPECIES_HOOPA_CONFINED,           INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER) }, // 720 floats + flavor Prankster
    { SPECIES_HOOPA_UNBOUND,            INNATES(ABILITY_LEVITATE, ABILITY_PRANKSTER) }, // 720 floats + flavor Prankster

    // ----- Gen 7 -----
    { SPECIES_ROWLET,                   sInnateOvergrow }, // 722
    { SPECIES_DARTRIX,                  sInnateOvergrow }, // 723
    { SPECIES_DECIDUEYE,                sInnateOvergrow }, // 724
    { SPECIES_DECIDUEYE_HISUI,          sInnateOvergrow }, // 724
    { SPECIES_LITTEN,                   sInnateBlaze }, // 725
    { SPECIES_TORRACAT,                 sInnateBlaze }, // 726
    { SPECIES_INCINEROAR,               sInnateBlaze }, // 727
    { SPECIES_POPPLIO,                  sInnateTorrent }, // 728
    { SPECIES_BRIONNE,                  sInnateTorrent }, // 729
    { SPECIES_PRIMARINA,                sInnateTorrent }, // 730
    { SPECIES_GRUBBIN,                  sInnateSwarm }, // 736
    { SPECIES_VIKAVOLT,                 sInnateLevitate }, // 738
    { SPECIES_VIKAVOLT_TOTEM,           sInnateLevitate }, // 738
    { SPECIES_LYCANROC_MIDDAY,          sInnateSandRush }, // 745 Sand Rush is the HA (Midday form; SPECIES_LYCANROC aliases it. Midnight/Dusk lack it)
    { SPECIES_MAREANIE,                 sInnateRegenerator }, // 747
    { SPECIES_TOXAPEX,                  sInnateRegenerator }, // 748
    { SPECIES_COMFEY,                   INNATES(ABILITY_LEVITATE, ABILITY_NATURAL_CURE) }, // 764 floats (lei) + canon Natural Cure (HA)
    { SPECIES_PYUKUMUKU,                sInnateUnaware }, // 771 Unaware is the HA
    { SPECIES_KOMALA,                   sInnateUnaware }, // 775 sleeps its whole life, oblivious to its surroundings
    { SPECIES_TOGEDEMARU,               sInnateSturdy }, // 777 Totem form omitted (Sturdy is its sole ability — redundant)
    { SPECIES_DHELMISE,                 sInnateLevitate }, // 781
    { SPECIES_TAPU_KOKO,                sInnateLevitate }, // 785 Electric Surge
    { SPECIES_TAPU_LELE,                sInnateLevitate }, // 786 Psychic Surge
    { SPECIES_TAPU_BULU,                sInnateLevitate }, // 787 Grassy Surge: floats AND reaps its own terrain (innate Levitate is a pure boon)
    { SPECIES_TAPU_FINI,                sInnateLevitate }, // 788 Misty Surge
    { SPECIES_COSMOG,                   sInnateLevitate }, // 789
    { SPECIES_COSMOEM,                  sInnateLevitate }, // 790
    { SPECIES_LUNALA,                   sInnateLevitate }, // 792
    { SPECIES_NIHILEGO,                 sInnateLevitate }, // 793
    { SPECIES_XURKITREE,                sInnateLevitate }, // 796
    { SPECIES_KARTANA,                  sInnateLevitate }, // 798
    { SPECIES_NECROZMA,                 sInnateLevitate }, // 800
    { SPECIES_NECROZMA_DUSK_MANE,       sInnateLevitate }, // 800
    { SPECIES_NECROZMA_DAWN_WINGS,      sInnateLevitate }, // 800
    { SPECIES_NECROZMA_ULTRA,           sInnateLevitate }, // 800
    { SPECIES_MAGEARNA,                 sInnateLevitate }, // 801
    { SPECIES_MAGEARNA_ORIGINAL,        sInnateLevitate }, // 801
    { SPECIES_MAGEARNA_MEGA,            sInnateLevitate }, // 801
    { SPECIES_MAGEARNA_ORIGINAL_MEGA,   sInnateLevitate }, // 801
    { SPECIES_POIPOLE,                  sInnateLevitate }, // 803
    { SPECIES_BLACEPHALON,              sInnateLevitate }, // 806

    // ----- Gen 8 -----
    { SPECIES_GROOKEY,                  sInnateOvergrow }, // 810
    { SPECIES_THWACKEY,                 sInnateOvergrow }, // 811
    { SPECIES_RILLABOOM,                sInnateOvergrow }, // 812
    { SPECIES_RILLABOOM_GMAX,           sInnateOvergrow }, // 812
    { SPECIES_SCORBUNNY,                sInnateBlaze }, // 813
    { SPECIES_RABOOT,                   sInnateBlaze }, // 814
    { SPECIES_CINDERACE,                sInnateBlaze }, // 815
    { SPECIES_CINDERACE_GMAX,           sInnateBlaze }, // 815
    { SPECIES_SOBBLE,                   sInnateTorrent }, // 816
    { SPECIES_DRIZZILE,                 sInnateTorrent }, // 817
    { SPECIES_INTELEON,                 sInnateTorrent }, // 818
    { SPECIES_INTELEON_GMAX,            sInnateTorrent }, // 818
    { SPECIES_BLIPBUG,                  sInnateSwarm }, // 824
    { SPECIES_DOTTLER,                  sInnateSwarm }, // 825
    { SPECIES_ORBEETLE,                 sInnateSwarm }, // 826
    { SPECIES_ORBEETLE_GMAX,            sInnateSwarm }, // 826
    { SPECIES_GOSSIFLEUR,               sInnateRegenerator }, // 829
    { SPECIES_ELDEGOSS,                 sInnateRegenerator }, // 830
    { SPECIES_CHEWTLE,                  sInnateSwiftSwim }, // 833 Swift Swim is the HA
    { SPECIES_DREDNAW,                  sInnateSwiftSwim }, // 834
    { SPECIES_DREDNAW_GMAX,             sInnateSwiftSwim }, // 834 Gmax keeps Swift Swim (HA)
    { SPECIES_ARROKUDA,                 sInnateSwiftSwim }, // 846 Swift Swim is the primary
    { SPECIES_BARRASKEWDA,              sInnateSwiftSwim }, // 847
    { SPECIES_SINISTEA,                 sInnateLevitate }, // 854
    { SPECIES_SINISTEA_PHONY,           sInnateLevitate }, // 854
    { SPECIES_SINISTEA_ANTIQUE,         sInnateLevitate }, // 854
    { SPECIES_POLTEAGEIST,              sInnateLevitate }, // 855
    { SPECIES_POLTEAGEIST_PHONY,        sInnateLevitate }, // 855
    { SPECIES_POLTEAGEIST_ANTIQUE,      sInnateLevitate }, // 855
    { SPECIES_IMPIDIMP,                 sInnatePrankster }, // 859 Prankster is the primary across the line
    { SPECIES_MORGREM,                  sInnatePrankster }, // 860
    { SPECIES_GRIMMSNARL,               sInnatePrankster }, // 861
    { SPECIES_GRIMMSNARL_GMAX,          sInnatePrankster }, // 861 Gmax keeps Prankster, so the innate survives the transformation
    { SPECIES_RUNERIGUS,                sInnateLevitate }, // 867
    { SPECIES_DRACOZOLT,                sInnateSandRush }, // 880 Sand Rush is the HA
    { SPECIES_ARCTOZOLT,                sInnateSlushRush }, // 881 Slush Rush is the HA
    { SPECIES_DRACOVISH,                sInnateSandRush }, // 882 Sand Rush is the HA
    { SPECIES_ARCTOVISH,                sInnateSlushRush }, // 883 Slush Rush is the HA
    { SPECIES_DREEPY,                   sInnateLevitate }, // 885
    { SPECIES_DRAKLOAK,                 sInnateLevitate }, // 886
    { SPECIES_DRAGAPULT,                sInnateLevitate }, // 887
    { SPECIES_REGIELEKI,                sInnateLevitate }, // 894
    { SPECIES_KLEAVOR,                  sInnateSwarm }, // 900
    { SPECIES_BASCULEGION_M,            sInnateSwiftSwim }, // 902 Swift Swim is the primary
    { SPECIES_BASCULEGION_F,            sInnateSwiftSwim }, // 902 (SPECIES_BASCULEGION aliases the male form)
    { SPECIES_OVERQWIL,                 sInnateSwiftSwim }, // 904 Swift Swim is the HA

    // ----- Gen 9 -----
    { SPECIES_SPRIGATITO,               sInnateOvergrow }, // 906
    { SPECIES_FLORAGATO,                sInnateOvergrow }, // 907
    { SPECIES_MEOWSCARADA,              sInnateOvergrow }, // 908
    { SPECIES_FUECOCO,                  INNATES(ABILITY_UNAWARE, ABILITY_BLAZE) }, // 909 Unaware is the HA, Blaze the primary
    { SPECIES_CROCALOR,                 INNATES(ABILITY_UNAWARE, ABILITY_BLAZE) }, // 910
    { SPECIES_SKELEDIRGE,               INNATES(ABILITY_UNAWARE, ABILITY_BLAZE) }, // 911
    { SPECIES_QUAXLY,                   sInnateTorrent }, // 912
    { SPECIES_QUAXWELL,                 sInnateTorrent }, // 913
    { SPECIES_QUAQUAVAL,                sInnateTorrent }, // 914
    { SPECIES_NYMBLE,                   sInnateSwarm }, // 919
    { SPECIES_LOKIX,                    sInnateSwarm }, // 920
    { SPECIES_PAWMI,                    sInnateNaturalCure }, // 921 Natural Cure is the line's HA
    { SPECIES_PAWMO,                    sInnateNaturalCure }, // 922
    { SPECIES_PAWMOT,                   sInnateNaturalCure }, // 923
    { SPECIES_NACLI,                    sInnateSturdy }, // 932
    { SPECIES_NACLSTACK,                sInnateSturdy }, // 933
    { SPECIES_GARGANACL,                sInnateSturdy }, // 934
    { SPECIES_SHROODLE,                 sInnatePrankster }, // 944 Prankster is the HA across the line
    { SPECIES_GRAFAIAI,                 sInnatePrankster }, // 945
    { SPECIES_KLAWF,                    sInnateRegenerator }, // 950
    { SPECIES_CAPSAKID,                 sInnateChlorophyll }, // 951 Chlorophyll is the primary
    { SPECIES_SCOVILLAIN,               sInnateChlorophyll }, // 952
    { SPECIES_REVAVROOM,                sInnateFilter }, // 965 Filter is the HA
    { SPECIES_CYCLIZAR,                 sInnateRegenerator }, // 967
    { SPECIES_HOUNDSTONE,               sInnateSandRush }, // 972 Sand Rush is the primary
    { SPECIES_CETITAN,                  sInnateSlushRush }, // 975 Slush Rush is the secondary
    { SPECIES_DONDOZO,                  sInnateUnaware }, // 977 Unaware is the primary ability
    { SPECIES_CLODSIRE,                 INNATES(ABILITY_REGENERATOR, ABILITY_UNAWARE) }, // 980 flavor Regen + canon Unaware (HA)
    { SPECIES_FLUTTER_MANE,             sInnateLevitate }, // 987
    { SPECIES_IRON_MOTH,                sInnateLevitate }, // 994
    { SPECIES_GIMMIGHOUL_ROAMING,       sInnateLevitate }, // 999 Chest form sits on the ground, omitted
    { SPECIES_MIRAIDON,                 sInnateLevitate }, // 1008 Hadron Engine: floats AND reaps its own Electric Terrain
    { SPECIES_POLTCHAGEIST,             sInnateLevitate }, // 1012
    { SPECIES_POLTCHAGEIST_COUNTERFEIT, sInnateLevitate }, // 1012
    { SPECIES_POLTCHAGEIST_ARTISAN,     sInnateLevitate }, // 1012
    { SPECIES_SINISTCHA,                sInnateLevitate }, // 1013
    { SPECIES_SINISTCHA_UNREMARKABLE,   sInnateLevitate }, // 1013
    { SPECIES_SINISTCHA_MASTERPIECE,    sInnateLevitate }, // 1013
    { SPECIES_OGERPON_CORNERSTONE,      sInnateSturdy }, // 1017
    { SPECIES_ARCHALUDON,               sInnateSturdy }, // 1018
    { SPECIES_HYDRAPPLE,                sInnateRegenerator }, // 1019
    { SPECIES_PECHARUNT,                sInnateLevitate }, // 1025
};

static const enum Ability *GetSpeciesInnateList(u16 species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesInnates); i++)
    {
        if (sSpeciesInnates[i].species == species)
            return sSpeciesInnates[i].innates;
    }

    return NULL;
}

bool32 SpeciesHasInnate(u16 species, enum Ability ability)
{
    const enum Ability *list;
    u32 i;

    if (ability == ABILITY_NONE)
        return FALSE;

    list = GetSpeciesInnateList(species);
    if (list == NULL)
        return FALSE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (list[i] == ability)
            return TRUE;
    }

    return FALSE;
}

enum Ability GetSpeciesInnate(u16 species, u32 index)
{
    const enum Ability *list = GetSpeciesInnateList(species);
    u32 i;

    if (list == NULL)
        return ABILITY_NONE;

    for (i = 0; list[i] != ABILITY_NONE; i++)
    {
        if (i == index)
            return list[i];
    }

    return ABILITY_NONE;
}
