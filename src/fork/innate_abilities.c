#include "global.h"
#include "battle.h"
#include "battle_util.h"
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
//   - ABILITY_PRESSURE — makes the holder's foes spend 1 extra PP per move used against it, handled at
//     the two PP-deduction sites that read ABILITY_PRESSURE: the real deduction in CancelerPPDeduction
//     (src/battle_move_resolution.c) and the fork-owned deterministic PP-refund mirror in src/battle_util.c
//     (both the spread-move loop and the single-target branch swap GetBattlerAbility(x) == ABILITY_PRESSURE
//     for BattlerHasAbility(x, ABILITY_PRESSURE)). A pure passive trait checked at a single kind of site:
//     no script / pop-up / driver, and the innate is NOT recorded as identity (the cosmetic "exerting its
//     Pressure!" switch-in message still fires only for the chosen ability, like all innate announcements).
//     NO pure-boon divergence: Pressure only ever costs the FOE extra PP, so it never hurts its holder —
//     the innate is a 1:1 copy of the real ability. Suppression parity holds via BattlerHasAbility() ->
//     IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field); Pressure is not breakable, so Mold
//     Breaker never touches it, same as the real ability. AI needs no wiring: nothing in src/battle_ai_*.c
//     reads ABILITY_PRESSURE for an effect (the PP tax isn't modeled in the AI's damage/turn calcs), so an
//     innate Pressure is exactly as (in)visible to the AI as a real one. Canon-only (no flavor picks — the
//     "exerts pressure" theme is hard to attribute beyond its real users, and the +1 PP tax is a potent
//     stall tool): every species whose ability data carries Pressure in any slot, so the signature survives
//     whichever slot a build picks (Aerodactyl/Aggron-style slot-2/HA Pressure included). Forms are listed
//     only where the form's ability data still carries Pressure (Giratina-Origin/Dialga-Origin/Palkia-Origin
//     keep it; the Galarian birds, the Mega/Kyurem-B/W and Mewtwo-Mega-Y forms swap to a different signature
//     and are omitted). Mewtwo (innate Levitate), Ho-Oh (innate Regenerator), Dusclops, the Deoxys formes
//     and Giratina-Altered (all innate Levitate) already carry an innate, so they take a combined
//     INNATES(...) list with Pressure added.
//   - ABILITY_STENCH — on a damaging hit, a 10% chance to make the target flinch (under
//     DETERMINISTIC_ABILITIES: a guaranteed flinch on the holder's first turn out, like a King's
//     Rock entry flinch). Handled at the single on-hit site in src/battle_util.c
//     (ABILITYEFFECT_MOVE_END_ATTACKER): the chosen-ability switch keys off gLastUsedAbility, so an
//     innate Stench whose chosen ability differs is run additively in a pre-check beside the switch
//     (TryStenchFlinch, guarded `chosen != ABILITY_STENCH` so a real Stench never flinches twice).
//     No script/pop-up/driver — the flinch flows through SetMoveEffect(MOVE_EFFECT_FLINCH), and the
//     innate is NOT recorded as identity (no ability pop-up, exactly like the real Stench, which has
//     none either). NO pure-boon divergence: Stench only ever flinches the FOE, so it never hurts its
//     holder — the innate is a 1:1 copy of the real ability. It still doesn't stack with a King's Rock
//     flinch — TryKingsRock (src/battle_hold_effects.c) pre-empts the holder's own flinch item via a
//     BattlerHasAbility(ATK, STENCH) guard, made innate-aware so an innate holder's item bows out (and
//     isn't consumed) exactly like a chosen Stench's. The flinch is also still blocked by Shield Dust /
//     Covert Cloak, same as the real ability (those checks live in the shared flinch path, not the
//     ability dispatch). Because Stench sets the flinch via SetMoveEffect (not TryTriggerAdditionalEffect),
//     it bypasses DETERMINISTIC_FLINCH's anti-lock cap exactly like the real ability / King's Rock / Fake
//     Out — no flinchedLastTurn check. Suppression parity holds via
//     IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field); Stench is not breakable, so Mold
//     Breaker never touches it, same as the real ability. AI needs no wiring: nothing in src/battle_ai_*.c
//     reads ABILITY_STENCH (the AI doesn't model the flinch chance), so an innate Stench is exactly as
//     (in)visible to the AI as a real one. Two species groups: the canon Stench users (Grimer/Muk,
//     Koffing/Weezing's HA, Stunky/Skuntank, the Trubbish/Garbodor line incl. Gmax, and Gloom's HA — each
//     keeps the signature flinch no matter which slot the build picks; Galarian Weezing swaps its HA to
//     Misty Surge and is omitted), plus a tight foul-odor flavor set lacking the real ability (Oddish and
//     Vileplume completing the canon Gloom line — the "smells atrocious" weed line — and the Gulpin line's
//     poison-gas bags). No frontier roster sets hardcoded Stench, so none needed freeing.
//   - ABILITY_BATTLE_ARMOR / ABILITY_SHELL_ARMOR — the two crit-immunity abilities (identical effect:
//     attacks landed on the holder are never critical hits), handled at the two crit-calc sites in
//     src/battle_util.c (CalcCritChanceStage and the Gen-1 CalcCritChanceStageGen1): each gains a clause
//     beside the cached chosen-ability test that forces critChance = CRITICAL_HIT_BLOCKED for an innate
//     holder too. A pure passive immunity checked at a single kind of site (like Sturdy): no script /
//     pop-up / driver. NO pure-boon divergence: crit immunity is a clean upside that never hurts its
//     holder, so each innate is a 1:1 copy of the real ability. The innate is NOT recorded as identity —
//     only the chosen-ability path calls RecordAbilityBattle (the innate blocks silently, like Filter),
//     so Trace/Skill Swap/the ability pop-up still read the chosen slot. Suppression parity holds via
//     IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field); neither is breakable, so Mold
//     Breaker never touches them, same as the real ability. AI is innate-aware: nothing in the shared
//     damage calc needs it (the calc's crit result already runs through CalcCritChanceStage keyed off the
//     real battler, so a predicted crit against an innate holder is correctly blocked for FREE), and the
//     one dedicated AI read — "don't waste Laser Focus on a crit-immune target" (EFFECT_LASER_FOCUS in
//     src/battle_ai_main.c) — now credits an innate holder via BattlerHasAbility(). Canon-only (no flavor
//     picks — crit immunity is hard to theme beyond an actual armored shell, and the set is already
//     large): every species whose ability data carries Battle Armor or Shell Armor in any slot, so the
//     signature survives whichever slot a build picks. Forms are listed only where the form's ability
//     data still carries it: Slowbro-Mega and Scolipede-Mega gain Shell Armor (their Mega ability data is
//     Shell Armor, replacing the base's), the Hisuian Sliggoo/Goodra carry Shell Armor where the base
//     forms carry Hydration instead (so only the Hisui rows get it), and the Drednaw-Gmax / Kingler-Gmax /
//     Lapras-Gmax forms keep theirs. Many species already carry other innates (Kabuto/Kabutops/Omastar/
//     Anorith/Armaldo's Swift Swim, the Turtwig and Oshawott lines' Overgrow/Torrent, Shellder/Cloyster/
//     Dwebble/Crustle's Sturdy, Slowbro-Mega's Regenerator, Escavalier's Swarm, Chewtle/Drednaw's Swift
//     Swim, Klawf's Regenerator), so they take a combined INNATES(...) list with the armor added. NOTE:
//     base Slowbro/Samurott-Hisui are intentionally NOT given the armor — their data carries Regenerator /
//     Sharpness there, not the armor, so only the form whose data actually carries it gets the innate.
//     Frontier roster sets that hardcoded an armor are freed (Step 3.5): Omastar/Kabutops keep their real
//     Weak Armor, Drapion its Sniper, Goodra-Hisui its Sap Sipper, Drednaw its Strong Jaw, while the
//     all-real-abilities-now-innate species take a fork-owned chosen override (Armaldo/Samurott → Water
//     Absorb, Torterra → Sand Stream, Turtonator → Flame Body) in species_ability_overrides.c.
//   - ABILITY_SPEED_BOOST — raises the holder's Speed by 1 stage at the end of every turn. This is the
//     fork's first ACTIVE, scripted end-turn innate, so unlike the passive abilities above it needs an
//     end-turn driver: TryActivateInnateEndTurnEffects (below in this file) is hooked from the
//     THIRD_EVENT_BLOCK_ABILITIES_INNATE step of the end-turn loop (src/battle_end_turn.c), right after
//     the chosen-ability end-turn block. The driver delegates to the upstream end-turn handler with the
//     innate ability passed explicitly — AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, ABILITY_SPEED_BOOST, ...)
//     — so the +1 Speed, the stat-change script (BattleScript_AbilityStatChange) and the pop-up are
//     identical to the real ability; the pop-up is overridden to show Speed Boost (not the chosen
//     ability) at the effect site in src/battle_util.c, but ONLY when the chosen ability differs, so a
//     real Speed Boost stays byte-for-byte unchanged (Sturdy/Levitate precedent). The driver skips an
//     innate that equals the chosen ability so a real Speed Boost never boosts twice. The driver is
//     RE-ENTRANT (a per-battler cursor in gBattleStruct->eventState, see TryActivateInnateEndTurnEffects),
//     so a battler can carry several active end-turn innates and fire each in turn; Speed Boost is just
//     the only one on the allowlist today. NO pure-boon
//     divergence: Speed Boost is a clean upside that never hurts its holder, so the innate is a 1:1 copy.
//     Suppression parity holds via IsInnateActive() (Gastro Acid / Neutralizing Gas / not-on-field);
//     Speed Boost is not breakable, so Mold Breaker never touches it, same as the real ability. AI is
//     innate-aware: the boost isn't in any shared damage calc, but the two AI reads that key off a foe's
//     Speed Boost — "don't bother lowering an innate Speed Boost foe's Speed" in CanLowerStat
//     (src/battle_ai_util.c) and CanStatChange (src/battle_stat_change.c) — now credit an innate holder
//     via IsInnateActive(); the ability-transfer scoring (BattlerBenefitsFromAbilityScore) is left alone
//     since innates are never transferable. Canon-only (no flavor picks — a +1 Speed-per-turn snowball is
//     potent, like the pinch / weather-doubler / Pressure abilities): every species whose ability data
//     carries Speed Boost in any slot, so the signature survives whichever slot a build picks. Forms are
//     listed only where the form's ability data still carries it (Blaziken-Mega's ability data IS Speed
//     Boost, so it gets the innate; Sharpedo-Mega → Strong Jaw and Scolipede-Mega → Shell Armor are
//     omitted for Speed Boost). The Torchic line (innate Blaze) and the Venipede line (innate Swarm)
//     already carry an innate, so they take a combined INNATES(...) list with Speed Boost added.
//     Frontier roster sets that hardcoded Speed Boost are freed (Step 3.5): Ninjask → Infiltrator,
//     Sharpedo → Rough Skin, Yanmega → Tinted Lens, Scolipede → Poison Point, Espathra → Opportunist
//     (each a real, complementary slot), while Blaziken — whose only real abilities (Blaze, Speed Boost)
//     are now BOTH innate — takes a fork-owned chosen Sheer Force override in species_ability_overrides.c.
//   - ABILITY_LIMBER — the holder cannot be paralyzed, handled at the paralysis-immunity sites in
//     src/battle_util.c: the block site in CanSetNonVolatileStatus (the MOVE_EFFECT_PARALYSIS branch
//     gains an IsInnateActive() clause beside the chosen-ability test, and when an innate Limber — chosen
//     ability differs — blocks the paralysis it reassigns abilityDef to LIMBER and overwrites the pop-up
//     so the "protected by Limber" message/record shows Limber, the Levitate/Sturdy pop-up precedent), and
//     the switch-in cure site in TryImmunityAbilityHealStatus (an innate Limber cures pre-existing paralysis
//     on switch-in like the real ability, again with the pop-up overwritten to Limber). Also mirrored at the
//     out-of-battle Battle Pike status room (DoesAbilityPreventStatus, src/battle_pike.c) so an innate-Limber
//     party mon shrugs off the Pike's paralysis room exactly like a real Limber. A pure passive immunity
//     checked at a single kind of site (like Sturdy/Filter): no driver. NO pure-boon divergence: paralysis
//     immunity is a clean upside that never hurts its holder, so the innate is a 1:1 copy of the real ability.
//     Suppression parity holds via IsInnateActive(): Limber is breakable, so an attacker's Mold Breaker
//     pierces an innate Limber exactly as it would the real ability. AI is innate-aware FOR FREE: the AI's
//     paralysis reasoning runs through CanBeParalyzed()/AI_CanParalyze() -> CanSetNonVolatileStatus(), whose
//     fork clause reads IsInnateActive(battlerDef, ABILITY_LIMBER) keyed off the real on-field battler (not
//     the passed-in abilityDef), so the AI correctly never tries to paralyze an innate-Limber foe; nothing in
//     src/battle_ai_*.c reads ABILITY_LIMBER directly. Two species groups: the canon Limber users (the agile
//     cats Persian/Glameow, the boneless contortionists Hitmonlee/Hawlucha/Clobbopus/Graploct, the flexible
//     rabbits Buneary/Lopunny, the formless Ditto, and Stunfisk — each keeps the para-immunity no matter which
//     slot the build picks; Mega Lopunny → Scrappy and Galarian Stunfisk → Mimicry swap their data and are
//     omitted), plus a tight supple-serpent flavor set lacking the real ability (the coiling snakes Ekans/Arbok
//     and Seviper, whose limber bodies fit the theme). Purrloin/Liepard already carry innate Prankster and
//     Mareanie/Toxapex already carry innate Regenerator, so they take a combined INNATES(...) list with Limber
//     added. Frontier roster sets that hardcoded Limber are freed (Step 3.5): Persian → Technician, Lopunny →
//     Cute Charm, Liepard → Unburden, Toxapex → Merciless, Graploct → Technician (each a real, complementary
//     slot the now-innate Limber freed).
//   - ABILITY_CUTE_CHARM — when the holder is hit by a contact move, a 30% chance to infatuate the
//     attacker if they are of opposite genders (under DETERMINISTIC_ABILITIES, a guaranteed infatuation
//     regardless of gender). Wired innate-aware at the ABILITYEFFECT_MOVE_END on-hit site in
//     src/battle_util.c: the chosen-ability dispatch keys off the target's gLastUsedAbility, so an innate
//     Cute Charm whose chosen ability differs is run additively in a pre-check beside the switch
//     (TryCuteCharmInfatuate, guarded `!= ABILITY_CUTE_CHARM` so a real Cute Charm never infatuates twice).
//     The effect runs the same BattleScript_CuteCharmActivates (pop-up + infatuation), so the one extra
//     step a pop-up'd innate needs is forcing gBattleScripting.abilityPopupOverwrite = ABILITY_CUTE_CHARM
//     when the chosen ability differs (the Limber/Speed Boost pop-up precedent), so the pop-up shows Cute
//     Charm and not the chosen ability; a real Cute Charm stays byte-for-byte unchanged. NO pure-boon
//     divergence: Cute Charm only ever infatuates the FOE, so it never hurts its holder — the innate is a
//     1:1 copy of the real ability. Suppression parity holds via IsInnateActive() (Gastro Acid /
//     Neutralizing Gas / not-on-field); Cute Charm is not breakable, so Mold Breaker never touches it, same
//     as the real ability. AI is innate-aware: the only AI read of ABILITY_CUTE_CHARM is the
//     DETERMINISTIC_ABILITIES contact-punish predictor AI_DeterministicContactAbilityPunishes
//     (src/battle_ai_util.c), which now also credits an innate Cute Charm on the defender via
//     BattlerHasAbility() so the AI treats contact as a downside even when the chosen ability differs;
//     under non-deterministic play neither a real nor an innate Cute Charm is modeled, so parity holds.
//     Canon-only (no flavor picks — infatuation can fully disable a foe for a turn, a potent disruption,
//     so like Prankster / the pinch abilities the set stays to species whose ability data carries Cute
//     Charm in any slot): the signature survives whichever slot a build picks (Milotic's HA Cute Charm,
//     Stufful's HA, Skitty/Delcatty/Minccino/Cinccino/Lopunny/Sylveon/Enamorus, the Clefairy and
//     Jigglypuff lines). Forms are listed only where the form's ability data still carries Cute Charm:
//     Buneary (Run Away/Klutz/Limber), Bewear (Fluffy/Klutz/Unnerve), Enamorus-Therian (Overcoat) and
//     Clefable-Mega lack it in their data and are omitted; only base Lopunny / Stufful's own line member /
//     Enamorus-Incarnate carry it. Clefable already carries innate Unaware and Lopunny innate Limber, so
//     they take a combined INNATES(...) list with Cute Charm added. Frontier roster sets that hardcoded a
//     chosen Cute Charm are freed (Step 3.5): Enamorus → Contrary (its real HA), while Lopunny — whose only
//     real non-drawback abilities (Cute Charm, Limber) are now BOTH innate — takes a fork-owned chosen
//     Sheer Force override in species_ability_overrides.c (its slot-2 Limber, now innate-redundant). Audino
//     is intentionally NOT given the innate: its ability data lacks Cute Charm (Healer/Regenerator/Klutz);
//     it only runs Cute Charm as a fork-chosen ability via the override table, so its frontier set is left
//     to keep that chosen Cute Charm and needs no freeing.
//   - ABILITY_OBLIVIOUS — the holder cannot be infatuated or Taunted (B_OBLIVIOUS_TAUNT >= GEN_6) and
//     is unaffected by Intimidate (B_UPDATED_INTIMIDATE >= GEN_8). A passive trait checked at several
//     scattered immunity sites; no script/pop-up driver. Wired innate-aware at: the Attract infatuation
//     block (Cmd_setdrowsy/Attract in src/battle_script_commands.c), the Taunt block (Cmd_settaunt), the
//     generic infatuation setter (BS_TrySetInfatuation), the Captivate stat-drop immunity (EFFECT_CAPTIVATE
//     in src/battle_stat_change.c), the Intimidate immunity (IsIntimidateBlocked in src/battle_stat_change.c),
//     the Cute-Charm self-infatuation check on the contacting attacker (src/battle_util.c), and the switch-in
//     cure of pre-existing infatuation/Taunt (TryImmunityAbilityHealStatus in src/battle_util.c). Each pairs
//     the chosen-ability test with IsInnateActive()/BattlerHasAbility(); the visible blocks (Attract, Taunt,
//     Captivate, Intimidate, switch-in cure) overwrite the pop-up to Oblivious when the chosen ability differs
//     (the Limber/Cute Charm pop-up precedent), so a real Oblivious stays byte-for-byte unchanged. NO pure-boon
//     divergence: Oblivious is a clean upside that never hurts its holder, so the innate is a 1:1 copy.
//     Suppression parity holds via IsInnateActive(): Oblivious is breakable, so an attacker's Mold Breaker
//     pierces an innate Oblivious exactly as it would the real ability. AI is innate-aware: the foe-side reads
//     are credited via IsInnateActive()/BattlerHasAbility() — AI_CanBeInfatuated (don't Attract an innate-Oblivious
//     foe), CanIntimidateLowerOpponentAtk (don't switch in an Intimidator against one, in src/battle_ai_switch.c),
//     and the Cute-Charm contact-punish predictor's attacker-Oblivious check (src/battle_ai_util.c). The AI's Taunt
//     scoring does not model Oblivious immunity even for the real ability, so it needs no innate wiring (parity).
//     Canon-only (no flavor picks): the canon roster — the perpetually-dazed Slowpoke/Numel/Spheal lines, the
//     clueless Smoochum/Jynx, the spaced-out Swinub line, etc. — already embodies the "oblivious" theme, so no
//     extra flavor picks are warranted (keeping the set tight). Every species whose ability data carries Oblivious
//     in any slot gets it, so the immunity survives whichever slot a build picks; forms are listed only where the
//     form's data still carries it (the Slowpoke line's Galarian/Mega forms swap to Own Tempo/Shell Armor and are
//     omitted; Tsareena loses Oblivious on evolving, so only Bounsweet/Steenee get it). Many species already carry
//     other innates (the Slowpoke line's Regenerator, Illumise's Prankster, the Wailmer line's Pressure, Numel's and
//     Dondozo's Unaware, Feebas's Swift Swim), so they take a combined INNATES(...) list with Oblivious added.
//     Frontier roster sets that hardcoded a chosen Oblivious are freed (Step 3.5): Whiscash → Hydration (its real HA),
//     while Dondozo — whose only non-Water-Veil real abilities (Unaware, Oblivious) are now BOTH innate — takes its
//     real Water Veil slot (burn immunity), no override needed.
//   - ABILITY_SAND_VEIL / ABILITY_SNOW_CLOAK — the two weather evasion abilities: +25% evasion (a 0.8
//     accuracy modifier on incoming moves) while their weather is up — sandstorm for Sand Veil, hail/snow
//     for Snow Cloak — and immunity to that weather's chip damage (sandstorm / hail), exactly like the
//     real abilities. Wired as passive calc-modifiers (like Filter / Unaware): the evasion lives at the
//     accuracy site GetTotalAccuracy (src/battle_util.c), applied additively beside the chosen-ability
//     switch (guarded `defAbility != ABILITY_X` so a real holder never applies the 0.8 twice), plus the
//     deterministic PP-tax mirror GetDeterministicMoveTargetPPTax in the same file. The weather-damage
//     immunity is mirrored at the end-turn chip sites (src/battle_end_turn.c), exactly as the innate Sand
//     Rush's sandstorm immunity already is. No script / pop-up / driver, and the innate is NOT recorded as
//     identity. NO pure-boon divergence: both are clean upsides that never hurt their holder, so each
//     innate is a 1:1 copy of the real ability. Suppression parity holds via IsInnateActive(): both are
//     breakable, so an attacker's Mold Breaker pierces an innate Sand Veil / Snow Cloak exactly as it
//     would the real ability (and the move then ignores the evasion). AI is innate-aware: on-field
//     accuracy prediction is correct for FREE (GetTotalAccuracy runs keyed off the real battler), and the
//     dedicated weather-damage / weather-setting reads credit the innate too — the sandstorm/hail chip
//     predictors (DoesBattlerTakeSandstormDamage / DoesBattlerTakeHailDamage in src/battle_ai_util.c, the
//     switch-in GetSwitchinWeatherImpact in src/battle_ai_switch.c) and the weather-setting heuristic
//     (DoesInnateBenefitFromWeather in src/battle_ai_field_statuses.c, so the AI values setting the matching
//     weather to enable the evasion). The off-field accuracy prediction is left unwired (the Unaware/Filter
//     scope call — a 25% evasion boost is not a KO-flipping immunity). The overworld wild-encounter-rate
//     halving (src/wild_encounter.c) reads the party lead's chosen ability only and is deliberately left
//     alone: innates are a battle-only feature (no battle state out of battle). Canon-only (no flavor
//     picks — evasion is a contentious, can-be-frustrating mechanic, so like Prankster / the potent
//     abilities the set stays tight to species whose ability data carries it in any slot): the signature
//     survives whichever slot a build picks (Garchomp's slot-1 Sand Veil, Gliscor's, Donphan's HA, etc.).
//     Forms are listed only where the form's ability data still carries it (Garchomp-Mega-Z keeps Sand Veil,
//     but the regular Garchomp-Mega swaps to Sand Force and is omitted; Sandaconda-Gmax keeps Sand Veil;
//     Vanilluxe / Cetitan / Tyranitar lose it on evolving and are omitted, so only the pre-evos get it).
//     Many species already carry other innates (Sandshrew/Sandslash's Sand Rush, the Geodude line and
//     Donphan's Sturdy, Stunfisk's Limber, the Swinub line's Oblivious, Articuno's Pressure, the Vanillite
//     line and Froslass's Levitate, the Sandshrew-Alola line / Cubchoo / Beartic's Slush Rush), so they take
//     a combined INNATES(...) list with the evasion ability added. Frontier roster sets that hardcoded a
//     chosen Sand Veil / Snow Cloak are freed (Step 3.5): Glaceon → Ice Body, Froslass → Cursed Body and
//     Wugtrio → Gooey each take a real complementary slot, while the species whose ALL relevant real
//     abilities are now innate take a fork-owned chosen override (species_ability_overrides.c) — Sandslash
//     and Donphan → Sand Stream, Sandslash-Alola / Articuno / Beartic → Snow Warning — each a stable :x:
//     weather-setter that also turns on the mon's own evasion innate.
// Do NOT give a species an innate that is not on this list: nothing would honor it
// (no effect site activates it), so it would silently do nothing.
//
// FORMS ARE KEYED EXACTLY (no base-species fallback): the lookup matches the exact battle species,
// so a Mega / Gigantamax / regional / forme variant gets innates ONLY if it has its own row. After
// a form change gBattleMons[].species becomes the form constant, so the form must be listed to keep
// any innate. Mega forms are populated as a PURE BOON: each Mega whose BASE creature has innates has
// its own row mirroring the base's list, so e.g. Mega Venusaur keeps Overgrow / Chlorophyll / etc.
// even though its real ability is Thick Fat — the innate models the base creature's trait persisting
// through the Mega, not the Mega's own ability data. (This SUPERSEDES the older per-ability notes
// below that say a Mega "swaps to <ability> and is omitted".) DELIBERATE EXCEPTIONS — grounded Megas
// must not float: Mega Gengar has NO row (Levitate was its only inheritable innate), and Mega Mewtwo X
// keeps only the non-floating boon (Pressure), dropping base Mewtwo's Levitate.

struct SpeciesInnates
{
    u16 species;
    const enum Ability *innates; // ABILITY_NONE-terminated
};

// A species with SEVERAL innates lists them inline at its row with INNATES(...) instead of needing a
// named combination array per pairing (which doesn't scale as the allowlist grows). The compound
// literal has static storage at file scope; the terminator is appended automatically.
#define INNATES(...) (const enum Ability[]){ __VA_ARGS__, ABILITY_NONE }

static const struct SpeciesInnates sSpeciesInnates[] =
{
    // Sorted by National Pokédex number (shown in each row's `{ // NNNN` comment); a distinct forme
    // that needs innates follows its base's number. There is no base-species fallback, so each form
    // (incl. Megas) that should carry innates needs its OWN row (see the FORMS note in the file
    // header above). List a forme ONLY when its species id differs from the base: a DEFAULT-form
    // alias (e.g. SPECIES_CASTFORM_NORMAL == SPECIES_CASTFORM, SPECIES_HOOPA_CONFINED == SPECIES_HOOPA)
    // is the same id as the bare base, so a row for it is dead duplicate data — list the bare base
    // only. The "no species appears more than once" integrity test (test/fork/innate_abilities.c)
    // enforces this. Every row
    // lists its innates inline with INNATES(...), one ability per line, sorted alphabetically. The
    // per-ability rationale (canon vs flavor picks) is documented in the file header above.

    // ----- Gen 1 -----
    { // 0001
        SPECIES_BULBASAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0002
        SPECIES_IVYSAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0003
        SPECIES_VENUSAUR_GMAX,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_FILTER,
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW,
            ABILITY_REGENERATOR
        )
    },
    { // 0004
        SPECIES_CHARMANDER,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0005
        SPECIES_CHARMELEON,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_X,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD_MEGA_Y,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0006
        SPECIES_CHARIZARD_GMAX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0007
        SPECIES_SQUIRTLE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0008
        SPECIES_WARTORTLE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_MEGA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0009
        SPECIES_BLASTOISE_GMAX,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0015
        SPECIES_BEEDRILL,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0015
        SPECIES_BEEDRILL_MEGA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0023
        SPECIES_EKANS,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0024
        SPECIES_ARBOK,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0027
        SPECIES_SANDSHREW,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0027
        SPECIES_SANDSHREW_ALOLA,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0028
        SPECIES_SANDSLASH,
        INNATES(
            ABILITY_SAND_RUSH,
            ABILITY_SAND_VEIL
        )
    },
    { // 0028
        SPECIES_SANDSLASH_ALOLA,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0035
        SPECIES_CLEFAIRY,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0036
        SPECIES_CLEFABLE,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_UNAWARE
        )
    },
    { // 0036
        SPECIES_CLEFABLE_MEGA,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0037
        SPECIES_VULPIX_ALOLA,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0038
        SPECIES_NINETALES_ALOLA,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0039
        SPECIES_JIGGLYPUFF,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0040
        SPECIES_WIGGLYTUFF,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0043
        SPECIES_ODDISH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0044
        SPECIES_GLOOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0045
        SPECIES_VILEPLUME,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_STENCH
        )
    },
    { // 0050
        SPECIES_DIGLETT,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0050
        SPECIES_DIGLETT_ALOLA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0051
        SPECIES_DUGTRIO,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0051
        SPECIES_DUGTRIO_ALOLA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0053
        SPECIES_PERSIAN,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0054
        SPECIES_PSYDUCK,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_UNAWARE
        )
    },
    { // 0055
        SPECIES_GOLDUCK,
        INNATES(
            ABILITY_SWIFT_SWIM,
            ABILITY_UNAWARE
        )
    },
    { // 0060
        SPECIES_POLIWAG,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0061
        SPECIES_POLIWHIRL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0062
        SPECIES_POLIWRATH,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0069
        SPECIES_BELLSPROUT,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0070
        SPECIES_WEEPINBELL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0071
        SPECIES_VICTREEBEL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0071
        SPECIES_VICTREEBEL_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0074
        SPECIES_GEODUDE,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0074
        SPECIES_GEODUDE_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0075
        SPECIES_GRAVELER_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0076
        SPECIES_GOLEM_ALOLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0079
        SPECIES_SLOWPOKE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0079
        SPECIES_SLOWPOKE_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_MEGA,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0080
        SPECIES_SLOWBRO_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0081
        SPECIES_MAGNEMITE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0082
        SPECIES_MAGNETON,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0088
        SPECIES_GRIMER,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0089
        SPECIES_MUK,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0090
        SPECIES_SHELLDER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0091
        SPECIES_CLOYSTER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0092
        SPECIES_GASTLY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0093
        SPECIES_HAUNTER,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0094
        SPECIES_GENGAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0094
        SPECIES_GENGAR_GMAX,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0095
        SPECIES_ONIX,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0098
        SPECIES_KRABBY,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0099
        SPECIES_KINGLER,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0099
        SPECIES_KINGLER_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0102
        SPECIES_EXEGGCUTE,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0103
        SPECIES_EXEGGUTOR,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0104
        SPECIES_CUBONE,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0105
        SPECIES_MAROWAK,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0106
        SPECIES_HITMONLEE,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0108
        SPECIES_LICKITUNG,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0109
        SPECIES_KOFFING,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STENCH
        )
    },
    { // 0110
        SPECIES_WEEZING_GALAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0113
        SPECIES_CHANSEY,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0114
        SPECIES_TANGELA,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_REGENERATOR
        )
    },
    { // 0116
        SPECIES_HORSEA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0118
        SPECIES_GOLDEEN,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0119
        SPECIES_SEAKING,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0120
        SPECIES_STARYU,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0121
        SPECIES_STARMIE_MEGA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0122
        SPECIES_MR_MIME,
        INNATES(
            ABILITY_FILTER
        )
    },
    { // 0123
        SPECIES_SCYTHER,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0124
        SPECIES_JYNX,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0129
        SPECIES_MAGIKARP,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0131
        SPECIES_LAPRAS,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0131
        SPECIES_LAPRAS_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0132
        SPECIES_DITTO,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0137
        SPECIES_PORYGON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0138
        SPECIES_OMANYTE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0139
        SPECIES_OMASTAR,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0140
        SPECIES_KABUTO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0141
        SPECIES_KABUTOPS,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0142
        SPECIES_AERODACTYL,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0142
        SPECIES_AERODACTYL_MEGA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0143
        SPECIES_SNORLAX,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0143
        SPECIES_SNORLAX_GMAX,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0144
        SPECIES_ARTICUNO,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0145
        SPECIES_ZAPDOS,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0146
        SPECIES_MOLTRES,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0150
        SPECIES_MEWTWO,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_X,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0150
        SPECIES_MEWTWO_MEGA_Y,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0151
        SPECIES_MEW,
        INNATES(
            ABILITY_LEVITATE
        )
    },

    // ----- Gen 2 -----
    { // 0152
        SPECIES_CHIKORITA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0153
        SPECIES_BAYLEEF,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0154
        SPECIES_MEGANIUM,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0154
        SPECIES_MEGANIUM_MEGA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_OVERGROW
        )
    },
    { // 0155
        SPECIES_CYNDAQUIL,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0156
        SPECIES_QUILAVA,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0157
        SPECIES_TYPHLOSION_HISUI,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0158
        SPECIES_TOTODILE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0159
        SPECIES_CROCONAW,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0160
        SPECIES_FERALIGATR_MEGA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0165
        SPECIES_LEDYBA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0166
        SPECIES_LEDIAN,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0167
        SPECIES_SPINARAK,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0168
        SPECIES_ARIADOS,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0173
        SPECIES_CLEFFA,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0174
        SPECIES_IGGLYBUFF,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0182
        SPECIES_BELLOSSOM,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0185
        SPECIES_SUDOWOODO,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0187
        SPECIES_HOPPIP,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0188
        SPECIES_SKIPLOOM,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0189
        SPECIES_JUMPLUFF,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0190
        SPECIES_AIPOM,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0191
        SPECIES_SUNKERN,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0192
        SPECIES_SUNFLORA,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0193
        SPECIES_YANMA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0194
        SPECIES_WOOPER,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0194
        SPECIES_WOOPER_PALDEA,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0195
        SPECIES_QUAGSIRE,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0198
        SPECIES_MURKROW,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0199
        SPECIES_SLOWKING,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_REGENERATOR
        )
    },
    { // 0199
        SPECIES_SLOWKING_GALAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0200
        SPECIES_MISDREAVUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_B,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_C,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_D,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_E,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_F,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_G,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_H,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_I,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_J,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_K,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_L,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_M,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_N,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_O,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_P,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Q,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_R,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_S,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_T,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_U,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_V,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_W,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_X,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Y,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_Z,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_EXCLAMATION,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0201
        SPECIES_UNOWN_QUESTION,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0204
        SPECIES_PINECO,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0205
        SPECIES_FORRETRESS,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0207
        SPECIES_GLIGAR,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0208
        SPECIES_STEELIX,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0208
        SPECIES_STEELIX_MEGA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0211
        SPECIES_QWILFISH,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0211
        SPECIES_QWILFISH_HISUI,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0212
        SPECIES_SCIZOR,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0212
        SPECIES_SCIZOR_MEGA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0213
        SPECIES_SHUCKLE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0214
        SPECIES_HERACROSS,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0214
        SPECIES_HERACROSS_MEGA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0220
        SPECIES_SWINUB,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0221
        SPECIES_PILOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0222
        SPECIES_CORSOLA,
        INNATES(
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0226
        SPECIES_MANTINE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0227
        SPECIES_SKARMORY,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0227
        SPECIES_SKARMORY_MEGA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0230
        SPECIES_KINGDRA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0231
        SPECIES_PHANPY,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0232
        SPECIES_DONPHAN,
        INNATES(
            ABILITY_SAND_VEIL,
            ABILITY_STURDY
        )
    },
    { // 0233
        SPECIES_PORYGON2,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0238
        SPECIES_SMOOCHUM,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0242
        SPECIES_BLISSEY,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0243
        SPECIES_RAIKOU,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0244
        SPECIES_ENTEI,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0245
        SPECIES_SUICUNE,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0246
        SPECIES_LARVITAR,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0249
        SPECIES_LUGIA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0250
        SPECIES_HO_OH,
        INNATES(
            ABILITY_PRESSURE,
            ABILITY_REGENERATOR
        )
    },
    { // 0251
        SPECIES_CELEBI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE,
            ABILITY_REGENERATOR
        )
    },

    // ----- Gen 3 -----
    { // 0252
        SPECIES_TREECKO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0253
        SPECIES_GROVYLE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0254
        SPECIES_SCEPTILE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0254
        SPECIES_SCEPTILE_MEGA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0255
        SPECIES_TORCHIC,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0256
        SPECIES_COMBUSKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_SPEED_BOOST
        )
    },
    { // 0257
        SPECIES_BLAZIKEN_MEGA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0258
        SPECIES_MUDKIP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0259
        SPECIES_MARSHTOMP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0260
        SPECIES_SWAMPERT_MEGA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0267
        SPECIES_BEAUTIFLY,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0270
        SPECIES_LOTAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0271
        SPECIES_LOMBRE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0272
        SPECIES_LUDICOLO,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0273
        SPECIES_SEEDOT,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0274
        SPECIES_NUZLEAF,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0275
        SPECIES_SHIFTRY,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0283
        SPECIES_SURSKIT,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0291
        SPECIES_NINJASK,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0292
        SPECIES_SHEDINJA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0299
        SPECIES_NOSEPASS,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0300
        SPECIES_SKITTY,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0301
        SPECIES_DELCATTY,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0302
        SPECIES_SABLEYE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0302
        SPECIES_SABLEYE_MEGA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0304
        SPECIES_ARON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0305
        SPECIES_LAIRON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0306
        SPECIES_AGGRON_MEGA,
        INNATES(
            ABILITY_FILTER,
            ABILITY_STURDY
        )
    },
    { // 0313
        SPECIES_VOLBEAT,
        INNATES(
            ABILITY_PRANKSTER,
            ABILITY_SWARM
        )
    },
    { // 0314
        SPECIES_ILLUMISE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRANKSTER
        )
    },
    { // 0315
        SPECIES_ROSELIA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0316
        SPECIES_GULPIN,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0317
        SPECIES_SWALOT,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0318
        SPECIES_CARVANHA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0319
        SPECIES_SHARPEDO,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0319
        SPECIES_SHARPEDO_MEGA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0320
        SPECIES_WAILMER,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE
        )
    },
    { // 0321
        SPECIES_WAILORD,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_PRESSURE
        )
    },
    { // 0322
        SPECIES_NUMEL,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0323
        SPECIES_CAMERUPT_MEGA,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0324
        SPECIES_TORKOAL,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0329
        SPECIES_VIBRAVA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0330
        SPECIES_FLYGON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0331
        SPECIES_CACNEA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0332
        SPECIES_CACTURNE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0333
        SPECIES_SWABLU,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0334
        SPECIES_ALTARIA_MEGA,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0336
        SPECIES_SEVIPER,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0337
        SPECIES_LUNATONE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0338
        SPECIES_SOLROCK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0339
        SPECIES_BARBOACH,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0340
        SPECIES_WHISCASH,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0341
        SPECIES_CORPHISH,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0342
        SPECIES_CRAWDAUNT,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0343
        SPECIES_BALTOY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0344
        SPECIES_CLAYDOL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0347
        SPECIES_ANORITH,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0348
        SPECIES_ARMALDO,
        INNATES(
            ABILITY_BATTLE_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0349
        SPECIES_FEEBAS,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0350
        SPECIES_MILOTIC,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0351
        SPECIES_CASTFORM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_SUNNY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_RAINY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0351
        SPECIES_CASTFORM_SNOWY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0353
        SPECIES_SHUPPET,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0354
        SPECIES_BANETTE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0355
        SPECIES_DUSKULL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0356
        SPECIES_DUSCLOPS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0357
        SPECIES_TROPIUS,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0358
        SPECIES_CHIMECHO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0358
        SPECIES_CHIMECHO_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0359
        SPECIES_ABSOL,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0359
        SPECIES_ABSOL_MEGA_Z,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0362
        SPECIES_GLALIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0362
        SPECIES_GLALIE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0363
        SPECIES_SPHEAL,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0364
        SPECIES_SEALEO,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0365
        SPECIES_WALREIN,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0366
        SPECIES_CLAMPERL,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0367
        SPECIES_HUNTAIL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0368
        SPECIES_GOREBYSS,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0369
        SPECIES_RELICANTH,
        INNATES(
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0370
        SPECIES_LUVDISC,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0377
        SPECIES_REGIROCK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0380
        SPECIES_LATIAS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0380
        SPECIES_LATIAS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0381
        SPECIES_LATIOS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0381
        SPECIES_LATIOS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0385
        SPECIES_JIRACHI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0386
        SPECIES_DEOXYS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_ATTACK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_DEFENSE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0386
        SPECIES_DEOXYS_SPEED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },

    // ----- Gen 4 -----
    { // 0387
        SPECIES_TURTWIG,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0388
        SPECIES_GROTLE,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0389
        SPECIES_TORTERRA,
        INNATES(
            ABILITY_OVERGROW,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0390
        SPECIES_CHIMCHAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0391
        SPECIES_MONFERNO,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0392
        SPECIES_INFERNAPE,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0393
        SPECIES_PIPLUP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0394
        SPECIES_PRINPLUP,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0395
        SPECIES_EMPOLEON,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0399
        SPECIES_BIDOOF,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0400
        SPECIES_BIBAREL,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0402
        SPECIES_KRICKETUNE,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0406
        SPECIES_BUDEW,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0407
        SPECIES_ROSERADE,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0410
        SPECIES_SHIELDON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0411
        SPECIES_BASTIODON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0416
        SPECIES_VESPIQUEN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0418
        SPECIES_BUIZEL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0419
        SPECIES_FLOATZEL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0420
        SPECIES_CHERUBI,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0424
        SPECIES_AMBIPOM,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0427
        SPECIES_BUNEARY,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER
        )
    },
    { // 0428
        SPECIES_LOPUNNY_MEGA,
        INNATES(
            ABILITY_CUTE_CHARM,
            ABILITY_LIMBER
        )
    },
    { // 0429
        SPECIES_MISMAGIUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0431
        SPECIES_GLAMEOW,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0433
        SPECIES_CHINGLING,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0434
        SPECIES_STUNKY,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0435
        SPECIES_SKUNTANK,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0436
        SPECIES_BRONZOR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0437
        SPECIES_BRONZONG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0438
        SPECIES_BONSLY,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0439
        SPECIES_MIME_JR,
        INNATES(
            ABILITY_FILTER
        )
    },
    { // 0440
        SPECIES_HAPPINY,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0442
        SPECIES_SPIRITOMB,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0443
        SPECIES_GIBLE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0444
        SPECIES_GABITE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0445
        SPECIES_GARCHOMP_MEGA_Z,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0446
        SPECIES_MUNCHLAX,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0447
        SPECIES_RIOLU,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0455
        SPECIES_CARNIVINE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0456
        SPECIES_FINNEON,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0457
        SPECIES_LUMINEON,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0458
        SPECIES_MANTYKE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0461
        SPECIES_WEAVILE,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0462
        SPECIES_MAGNEZONE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0463
        SPECIES_LICKILICKY,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0465
        SPECIES_TANGROWTH,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_REGENERATOR
        )
    },
    { // 0469
        SPECIES_YANMEGA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0470
        SPECIES_LEAFEON,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0471
        SPECIES_GLACEON,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0472
        SPECIES_GLISCOR,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0473
        SPECIES_MAMOSWINE,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0474
        SPECIES_PORYGON_Z,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0476
        SPECIES_PROBOPASS,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0477
        SPECIES_DUSKNOIR,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0478
        SPECIES_FROSLASS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0478
        SPECIES_FROSLASS_MEGA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0479
        SPECIES_ROTOM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_HEAT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_WASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FROST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_FAN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0479
        SPECIES_ROTOM_MOW,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0480
        SPECIES_UXIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0481
        SPECIES_MESPRIT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0482
        SPECIES_AZELF,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0483
        SPECIES_DIALGA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0483
        SPECIES_DIALGA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0484
        SPECIES_PALKIA,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0484
        SPECIES_PALKIA_ORIGIN,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0487
        SPECIES_GIRATINA_ALTERED,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRESSURE
        )
    },
    { // 0487
        SPECIES_GIRATINA_ORIGIN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0488
        SPECIES_CRESSELIA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0491
        SPECIES_DARKRAI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0491
        SPECIES_DARKRAI_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0492
        SPECIES_SHAYMIN_LAND,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },

    // ----- Gen 5 -----
    { // 0495
        SPECIES_SNIVY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0496
        SPECIES_SERVINE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0497
        SPECIES_SERPERIOR,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0498
        SPECIES_TEPIG,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0499
        SPECIES_PIGNITE,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0500
        SPECIES_EMBOAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0500
        SPECIES_EMBOAR_MEGA,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0501
        SPECIES_OSHAWOTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0502
        SPECIES_DEWOTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_TORRENT
        )
    },
    { // 0503
        SPECIES_SAMUROTT_HISUI,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0507
        SPECIES_HERDIER,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0508
        SPECIES_STOUTLAND,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0509
        SPECIES_PURRLOIN,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER
        )
    },
    { // 0510
        SPECIES_LIEPARD,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_PRANKSTER
        )
    },
    { // 0511
        SPECIES_PANSAGE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0512
        SPECIES_SIMISAGE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0513
        SPECIES_PANSEAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0514
        SPECIES_SIMISEAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0515
        SPECIES_PANPOUR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0516
        SPECIES_SIMIPOUR,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0517
        SPECIES_MUNNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0518
        SPECIES_MUSHARNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0524
        SPECIES_ROGGENROLA,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0525
        SPECIES_BOLDORE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0526
        SPECIES_GIGALITH,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0527
        SPECIES_WOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0528
        SPECIES_SWOOBAT,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0529
        SPECIES_DRILBUR,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0530
        SPECIES_EXCADRILL_MEGA,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0531
        SPECIES_AUDINO,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0531
        SPECIES_AUDINO_MEGA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0535
        SPECIES_TYMPOLE,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0536
        SPECIES_PALPITOAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0537
        SPECIES_SEISMITOAD,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0539
        SPECIES_SAWK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0540
        SPECIES_SEWADDLE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SWARM
        )
    },
    { // 0541
        SPECIES_SWADLOON,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0542
        SPECIES_LEAVANNY,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_SWARM
        )
    },
    { // 0543
        SPECIES_VENIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0544
        SPECIES_WHIRLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE,
        INNATES(
            ABILITY_SPEED_BOOST,
            ABILITY_SWARM
        )
    },
    { // 0545
        SPECIES_SCOLIPEDE_MEGA,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0546
        SPECIES_COTTONEE,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0547
        SPECIES_WHIMSICOTT,
        INNATES(
            ABILITY_CHLOROPHYLL,
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0548
        SPECIES_PETILIL,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0549
        SPECIES_LILLIGANT,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0549
        SPECIES_LILLIGANT_HISUI,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0556
        SPECIES_MARACTUS,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0557
        SPECIES_DWEBBLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0558
        SPECIES_CRUSTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_STURDY
        )
    },
    { // 0562
        SPECIES_YAMASK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0562
        SPECIES_YAMASK_GALAR,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0563
        SPECIES_COFAGRIGUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0564
        SPECIES_TIRTOUGA,
        INNATES(
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0565
        SPECIES_CARRACOSTA,
        INNATES(
            ABILITY_STURDY,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0568
        SPECIES_TRUBBISH,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0569
        SPECIES_GARBODOR,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0569
        SPECIES_GARBODOR_GMAX,
        INNATES(
            ABILITY_STENCH
        )
    },
    { // 0570
        SPECIES_ZORUA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0571
        SPECIES_ZOROARK,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0572
        SPECIES_MINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0573
        SPECIES_CINCCINO,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0577
        SPECIES_SOLOSIS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0578
        SPECIES_DUOSION,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0579
        SPECIES_REUNICLUS,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_REGENERATOR
        )
    },
    { // 0582
        SPECIES_VANILLITE,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0583
        SPECIES_VANILLISH,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0584
        SPECIES_VANILLUXE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0585
        SPECIES_DEERLING_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0585
        SPECIES_DEERLING_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0585
        SPECIES_DEERLING_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0585
        SPECIES_DEERLING_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SPRING,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_SUMMER,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_AUTUMN,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0586
        SPECIES_SAWSBUCK_WINTER,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0588
        SPECIES_KARRABLAST,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0589
        SPECIES_ESCAVALIER,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWARM
        )
    },
    { // 0590
        SPECIES_FOONGUS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0591
        SPECIES_AMOONGUSS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0592
        SPECIES_FRILLISH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0593
        SPECIES_JELLICENT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0594
        SPECIES_ALOMOMOLA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0595
        SPECIES_JOLTIK,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0596
        SPECIES_GALVANTULA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0599
        SPECIES_KLINK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0600
        SPECIES_KLANG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0601
        SPECIES_KLINKLANG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0602
        SPECIES_TYNAMO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0603
        SPECIES_EELEKTRIK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0604
        SPECIES_EELEKTROSS_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0605
        SPECIES_ELGYEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0606
        SPECIES_BEHEEYEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0607
        SPECIES_LITWICK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0608
        SPECIES_LAMPENT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0609
        SPECIES_CHANDELURE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0609
        SPECIES_CHANDELURE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0613
        SPECIES_CUBCHOO,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0614
        SPECIES_BEARTIC,
        INNATES(
            ABILITY_SLUSH_RUSH,
            ABILITY_SNOW_CLOAK,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0615
        SPECIES_CRYOGONAL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0616
        SPECIES_SHELMET,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0618
        SPECIES_STUNFISK,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_SAND_VEIL
        )
    },
    { // 0619
        SPECIES_MIENFOO,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0620
        SPECIES_MIENSHAO,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0624
        SPECIES_PAWNIARD,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0625
        SPECIES_BISHARP,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0632
        SPECIES_DURANT,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0635
        SPECIES_HYDREIGON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0637
        SPECIES_VOLCARONA,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0641
        SPECIES_TORNADUS_INCARNATE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0641
        SPECIES_TORNADUS_THERIAN,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0642
        SPECIES_THUNDURUS_INCARNATE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0646
        SPECIES_KYUREM,
        INNATES(
            ABILITY_PRESSURE
        )
    },

    // ----- Gen 6 -----
    { // 0650
        SPECIES_CHESPIN,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0651
        SPECIES_QUILLADIN,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0652
        SPECIES_CHESNAUGHT_MEGA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0653
        SPECIES_FENNEKIN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0654
        SPECIES_BRAIXEN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0655
        SPECIES_DELPHOX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0655
        SPECIES_DELPHOX_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0656
        SPECIES_FROAKIE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0657
        SPECIES_FROGADIER,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0658
        SPECIES_GRENINJA_MEGA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0678
        SPECIES_MEOWSTIC_M_MEGA,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0679
        SPECIES_HONEDGE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0680
        SPECIES_DOUBLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0681
        SPECIES_AEGISLASH_BLADE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0686
        SPECIES_INKAY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0694
        SPECIES_HELIOPTILE,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0695
        SPECIES_HELIOLISK,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0696
        SPECIES_TYRUNT,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0700
        SPECIES_SYLVEON,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0701
        SPECIES_HAWLUCHA,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0701
        SPECIES_HAWLUCHA_MEGA,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0703
        SPECIES_CARBINK,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_STURDY
        )
    },
    { // 0705
        SPECIES_SLIGGOO_HISUI,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0706
        SPECIES_GOODRA_HISUI,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0707
        SPECIES_KLEFKI,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0708
        SPECIES_PHANTUMP,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0709
        SPECIES_TREVENANT,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SMALL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_LARGE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0710
        SPECIES_PUMPKABOO_SUPER,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SMALL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_LARGE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0711
        SPECIES_GOURGEIST_SUPER,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0712
        SPECIES_BERGMITE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0713
        SPECIES_AVALUGG_HISUI,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0718
        SPECIES_ZYGARDE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_MEGA,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_10,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0718
        SPECIES_ZYGARDE_COMPLETE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0719
        SPECIES_DIANCIE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0719
        SPECIES_DIANCIE_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0720
        SPECIES_HOOPA,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },
    { // 0720
        SPECIES_HOOPA_UNBOUND,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_PRANKSTER
        )
    },

    // ----- Gen 7 -----
    { // 0722
        SPECIES_ROWLET,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0723
        SPECIES_DARTRIX,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0724
        SPECIES_DECIDUEYE_HISUI,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0725
        SPECIES_LITTEN,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0726
        SPECIES_TORRACAT,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0727
        SPECIES_INCINEROAR,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0728
        SPECIES_POPPLIO,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0729
        SPECIES_BRIONNE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0730
        SPECIES_PRIMARINA,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0736
        SPECIES_GRUBBIN,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0738
        SPECIES_VIKAVOLT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0738
        SPECIES_VIKAVOLT_TOTEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0745
        SPECIES_LYCANROC_MIDDAY,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0747
        SPECIES_MAREANIE,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_REGENERATOR
        )
    },
    { // 0748
        SPECIES_TOXAPEX,
        INNATES(
            ABILITY_LIMBER,
            ABILITY_REGENERATOR
        )
    },
    { // 0757
        SPECIES_SALANDIT,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0758
        SPECIES_SALAZZLE,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0759
        SPECIES_STUFFUL,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },
    { // 0761
        SPECIES_BOUNSWEET,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0762
        SPECIES_STEENEE,
        INNATES(
            ABILITY_OBLIVIOUS
        )
    },
    { // 0764
        SPECIES_COMFEY,
        INNATES(
            ABILITY_LEVITATE,
            ABILITY_NATURAL_CURE
        )
    },
    { // 0769
        SPECIES_SANDYGAST,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0770
        SPECIES_PALOSSAND,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0771
        SPECIES_PYUKUMUKU,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0772
        SPECIES_TYPE_NULL,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0775
        SPECIES_KOMALA,
        INNATES(
            ABILITY_UNAWARE
        )
    },
    { // 0776
        SPECIES_TURTONATOR,
        INNATES(
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0777
        SPECIES_TOGEDEMARU,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0781
        SPECIES_DHELMISE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0785
        SPECIES_TAPU_KOKO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0786
        SPECIES_TAPU_LELE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0787
        SPECIES_TAPU_BULU,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0788
        SPECIES_TAPU_FINI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0789
        SPECIES_COSMOG,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0790
        SPECIES_COSMOEM,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0792
        SPECIES_LUNALA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0793
        SPECIES_NIHILEGO,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0796
        SPECIES_XURKITREE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0798
        SPECIES_KARTANA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_DUSK_MANE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_DAWN_WINGS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0800
        SPECIES_NECROZMA_ULTRA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0801
        SPECIES_MAGEARNA_ORIGINAL_MEGA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0803
        SPECIES_POIPOLE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0806
        SPECIES_BLACEPHALON,
        INNATES(
            ABILITY_LEVITATE
        )
    },

    // ----- Gen 8 -----
    { // 0810
        SPECIES_GROOKEY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0811
        SPECIES_THWACKEY,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0812
        SPECIES_RILLABOOM,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0812
        SPECIES_RILLABOOM_GMAX,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0813
        SPECIES_SCORBUNNY,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0814
        SPECIES_RABOOT,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0815
        SPECIES_CINDERACE,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0815
        SPECIES_CINDERACE_GMAX,
        INNATES(
            ABILITY_BLAZE
        )
    },
    { // 0816
        SPECIES_SOBBLE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0817
        SPECIES_DRIZZILE,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0818
        SPECIES_INTELEON_GMAX,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0823
        SPECIES_CORVIKNIGHT_GMAX,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0824
        SPECIES_BLIPBUG,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0825
        SPECIES_DOTTLER,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0826
        SPECIES_ORBEETLE,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0826
        SPECIES_ORBEETLE_GMAX,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0829
        SPECIES_GOSSIFLEUR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0830
        SPECIES_ELDEGOSS,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0833
        SPECIES_CHEWTLE,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0834
        SPECIES_DREDNAW_GMAX,
        INNATES(
            ABILITY_SHELL_ARMOR,
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0843
        SPECIES_SILICOBRA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0844
        SPECIES_SANDACONDA,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0844
        SPECIES_SANDACONDA_GMAX,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0846
        SPECIES_ARROKUDA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0847
        SPECIES_BARRASKEWDA,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0852
        SPECIES_CLOBBOPUS,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0853
        SPECIES_GRAPPLOCT,
        INNATES(
            ABILITY_LIMBER
        )
    },
    { // 0854
        SPECIES_SINISTEA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0854
        SPECIES_SINISTEA_ANTIQUE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0855
        SPECIES_POLTEAGEIST_ANTIQUE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0859
        SPECIES_IMPIDIMP,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0860
        SPECIES_MORGREM,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0861
        SPECIES_GRIMMSNARL_GMAX,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0863
        SPECIES_PERRSERKER,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0867
        SPECIES_RUNERIGUS,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0870
        SPECIES_FALINKS,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0870
        SPECIES_FALINKS_MEGA,
        INNATES(
            ABILITY_BATTLE_ARMOR
        )
    },
    { // 0880
        SPECIES_DRACOZOLT,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0881
        SPECIES_ARCTOZOLT,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0882
        SPECIES_DRACOVISH,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0883
        SPECIES_ARCTOVISH,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0885
        SPECIES_DREEPY,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0886
        SPECIES_DRAKLOAK,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0887
        SPECIES_DRAGAPULT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0890
        SPECIES_ETERNATUS,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0890
        SPECIES_ETERNATUS_ETERNAMAX,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0894
        SPECIES_REGIELEKI,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0900
        SPECIES_KLEAVOR,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0902
        SPECIES_BASCULEGION_M,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0902
        SPECIES_BASCULEGION_F,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0903
        SPECIES_SNEASLER,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0904
        SPECIES_OVERQWIL,
        INNATES(
            ABILITY_SWIFT_SWIM
        )
    },
    { // 0905
        SPECIES_ENAMORUS_INCARNATE,
        INNATES(
            ABILITY_CUTE_CHARM
        )
    },

    // ----- Gen 9 -----
    { // 0906
        SPECIES_SPRIGATITO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0907
        SPECIES_FLORAGATO,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0908
        SPECIES_MEOWSCARADA,
        INNATES(
            ABILITY_OVERGROW
        )
    },
    { // 0909
        SPECIES_FUECOCO,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0910
        SPECIES_CROCALOR,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0911
        SPECIES_SKELEDIRGE,
        INNATES(
            ABILITY_BLAZE,
            ABILITY_UNAWARE
        )
    },
    { // 0912
        SPECIES_QUAXLY,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0913
        SPECIES_QUAXWELL,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0914
        SPECIES_QUAQUAVAL,
        INNATES(
            ABILITY_TORRENT
        )
    },
    { // 0919
        SPECIES_NYMBLE,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0920
        SPECIES_LOKIX,
        INNATES(
            ABILITY_SWARM
        )
    },
    { // 0921
        SPECIES_PAWMI,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0922
        SPECIES_PAWMO,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0923
        SPECIES_PAWMOT,
        INNATES(
            ABILITY_NATURAL_CURE
        )
    },
    { // 0932
        SPECIES_NACLI,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0933
        SPECIES_NACLSTACK,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0934
        SPECIES_GARGANACL,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 0944
        SPECIES_SHROODLE,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0945
        SPECIES_GRAFAIAI,
        INNATES(
            ABILITY_PRANKSTER
        )
    },
    { // 0950
        SPECIES_KLAWF,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_SHELL_ARMOR
        )
    },
    { // 0951
        SPECIES_CAPSAKID,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0952
        SPECIES_SCOVILLAIN_MEGA,
        INNATES(
            ABILITY_CHLOROPHYLL
        )
    },
    { // 0955
        SPECIES_FLITTLE,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0956
        SPECIES_ESPATHRA,
        INNATES(
            ABILITY_SPEED_BOOST
        )
    },
    { // 0960
        SPECIES_WIGLETT,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0961
        SPECIES_WUGTRIO,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0965
        SPECIES_REVAVROOM,
        INNATES(
            ABILITY_FILTER
        )
    },
    { // 0967
        SPECIES_CYCLIZAR,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 0968
        SPECIES_ORTHWORM,
        INNATES(
            ABILITY_SAND_VEIL
        )
    },
    { // 0972
        SPECIES_HOUNDSTONE,
        INNATES(
            ABILITY_SAND_RUSH
        )
    },
    { // 0974
        SPECIES_CETODDLE,
        INNATES(
            ABILITY_SNOW_CLOAK
        )
    },
    { // 0975
        SPECIES_CETITAN,
        INNATES(
            ABILITY_SLUSH_RUSH
        )
    },
    { // 0977
        SPECIES_DONDOZO,
        INNATES(
            ABILITY_OBLIVIOUS,
            ABILITY_UNAWARE
        )
    },
    { // 0980
        SPECIES_CLODSIRE,
        INNATES(
            ABILITY_REGENERATOR,
            ABILITY_UNAWARE
        )
    },
    { // 0983
        SPECIES_KINGAMBIT,
        INNATES(
            ABILITY_PRESSURE
        )
    },
    { // 0987
        SPECIES_FLUTTER_MANE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0994
        SPECIES_IRON_MOTH,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 0999
        SPECIES_GIMMIGHOUL_ROAMING,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1008
        SPECIES_MIRAIDON,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1012
        SPECIES_POLTCHAGEIST_ARTISAN,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1013
        SPECIES_SINISTCHA_MASTERPIECE,
        INNATES(
            ABILITY_LEVITATE
        )
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 1018
        SPECIES_ARCHALUDON,
        INNATES(
            ABILITY_STURDY
        )
    },
    { // 1019
        SPECIES_HYDRAPPLE,
        INNATES(
            ABILITY_REGENERATOR
        )
    },
    { // 1025
        SPECIES_PECHARUNT,
        INNATES(
            ABILITY_LEVITATE
        )
    },
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

// FORK: raw-table accessors for table-integrity tests (test/fork/innate_abilities.c).
// These walk the sSpeciesInnates rows directly (NOT keyed by species), so a duplicate
// species row — invisible to GetSpeciesInnateList, which returns the first match — is
// still observable. Not for battle logic: use SpeciesHasInnate / GetSpeciesInnate there.
u32 GetSpeciesInnatesEntryCount(void)
{
    return ARRAY_COUNT(sSpeciesInnates);
}

const enum Ability *GetSpeciesInnatesEntry(u32 row, u16 *outSpecies)
{
    if (row >= ARRAY_COUNT(sSpeciesInnates))
        return NULL;
    if (outSpecies != NULL)
        *outSpecies = sSpeciesInnates[row].species;
    return sSpeciesInnates[row].innates;
}

// Active, scripted innate abilities that fire at the end of every turn. Today only
// Speed Boost (raises Speed +1). Add a future end-turn active here; the driver
// (TryActivateInnateEndTurnEffects) is already re-entrant, so a battler may carry
// more than one and each fires in turn.
static bool32 IsActiveEndTurnInnate(enum Ability ability)
{
    return ability == ABILITY_SPEED_BOOST;
}

// FORK: end-turn innate driver (FEATURE_INNATE_ABILITIES). Fires the holder's active,
// scripted end-turn innates (today only Speed Boost), hooked from the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step of the end-turn loop (src/battle_end_turn.c)
// right after the chosen-ability end-turn block.
//
// RE-ENTRANT: a battle script fires one at a time, so this resumes from a per-battler
// cursor. *index is the next innate-list slot to consider; the end-turn loop holds the
// THIRD_EVENT_BLOCK_ABILITIES_INNATE step (keeping the cursor) while this returns TRUE,
// and only advances the block once it returns FALSE (list exhausted). The caller resets
// the cursor to 0 for the next battler. Each fired effect leaves *index pointing past it,
// so a battler with several active end-turn innates fires them across successive turns of
// the loop. Returns TRUE if an effect fired this call.
//
// The effect is delegated to the upstream end-turn ability handler with the innate
// passed explicitly: AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, ...)
// sets gLastUsedAbility = innate and runs that ability's existing case, so the stat
// change / script / pop-up match the real ability exactly (the pop-up is overridden to
// show the innate at the Speed Boost effect site in src/battle_util.c, but only when the
// chosen ability differs). An innate equal to the chosen ability is skipped so the
// chosen-ability block (which already ran it) never boosts twice; IsInnateActive() applies
// the usual suppression (feature flag, Gastro Acid, Neutralizing Gas, not-on-field). An
// eligible innate that does nothing this turn (e.g. Speed already maxed) is stepped over
// without firing, so the scan continues to the battler's next end-turn innate.
bool32 TryActivateInnateEndTurnEffects(enum BattlerId battler, u32 *index)
{
    enum Ability innate;

    while ((innate = GetSpeciesInnate(gBattleMons[battler].species, *index)) != ABILITY_NONE)
    {
        (*index)++; // step past this slot now, so a fired effect resumes at the next one
        if (!IsActiveEndTurnInnate(innate))
            continue;
        if (GetBattlerAbility(battler) == innate) // chosen-ability end-turn block already ran it
            continue;
        if (!IsInnateActive(battler, innate))
            continue;
        if (AbilityBattleEffects(ABILITYEFFECT_ENDTURN, battler, innate, MOVE_NONE, TRUE))
            return TRUE;
    }

    return FALSE;
}
