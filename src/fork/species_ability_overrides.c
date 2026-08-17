#include "global.h"
#include "fork/species_ability_overrides.h"
#include "config_changes.h" // FORK: GetConfig(FEATURE_INNATE_ABILITIES) gates the override table
#include "constants/abilities.h"
#include "constants/species.h"

// FORK: fork-owned species ability overrides (sibling to src/innate_abilities.c).
// See include/species_ability_overrides.h for the full rationale. In short: this
// gives a small set of innate-Levitate/Regenerator species a real, selectable
// SECOND ability so a Battle Factory set can run it alongside the innate, without
// editing upstream gSpeciesInfo. GetSpeciesAbility() (src/pokemon.c) consults this
// table first; where no row matches, the upstream ability data is used unchanged.
//
// Each row replaces ability SLOT `slot` for `species` with `ability`. The replaced
// slot is usually either empty in the upstream data (a free normal slot on an
// ability-locked species) or holds an ability that is redundant because it is now
// granted innately (e.g. the Regenerator slot on Tangrowth/Audino/Alomomola). One
// row (Sceptile) instead replaces a real Hidden Ability — its Overgrow is innately
// latched, so a frontier slot is free, and its HA (Unburden) is dead weight on the
// roster's non-consumable-item sets, so the slot is repurposed for a flavorful pick.
// The roster test test/frontier_extended_roster.c verifies every set's chosen ability
// resolves to a real slot through this hook, so an out-of-place row fails CI.
//
// GATED BY FEATURE_INNATE_ABILITIES (see GetSpeciesAbilityOverride below). Like innates and
// the fork's other runtime features, the table is FORCE-DISABLED in tests by default
// (TestInitConfigData), so upstream battle-ability tests see VANILLA slots and a row can no
// longer perturb them; a fork test that wants an override opts in with WITH_CONFIG. This is
// what lets a row repurpose a slot that an *upstream* test pins via Ability(...) or a default
// read -- that test never sees the override. Only *fork* tests that opt into the flag observe
// a repurposed slot, so the "SLOT CHOICE MATTERS" audit below now only needs to clear
// flag-on fork tests (the innate test, the frontier roster), not the whole upstream suite.

struct SpeciesAbilityOverride
{
    u16 species;
    u8 slot;             // ability slot (0..NUM_ABILITY_SLOTS-1) this row replaces
    enum Ability ability;
};

// Sorted by National Pokédex number (shown in each row's trailing comment); formes share their
// base's number and follow it. Adding a row: drop it at its dex position with a trailing `// <dex>`.
//
// PICK A NEVER-AN-INNATE CHOSEN ABILITY — cross-reference it against sImplementedInnates[] in
// test/fork/innate_abilities.c, the single source of truth. The ability MUST be ABSENT from that
// array (never wired as an innate — e.g. Lightning Rod, Soundproof, Water Absorb, Sheer Force). An
// innate-CAPABLE ability (one ON the array) is NOT a legal pick, even when this species
// does not currently carry it: an innate-capable ability belongs in an INNATES(...) row, where it is
// always-on and costs nothing, so spending the one observable slot on it both wastes that slot's
// only purpose (a trait the species can express no other way) and leaves a latent duplicate that
// collapses the moment a line review gives the species that ability innately. Sceptile's
// LIGHTNING_ROD is the model. (Separately, the slot a row *frees* must already be redundant via an
// *implemented* :white_check_mark: innate — that's the row's whole premise.)
//
// MIGRATION COMPLETE — every row below conforms. The legacy rows that predated this rule (123 of
// them, roughly a third of the table) were converted in one sweep, and TEST("Innate abilities: no
// ability override or frontier set names an innate-capable ability") in
// test/fork/innate_abilities.c is no longer KNOWN_FAILING: it is a REAL CI GATE, so a row naming an
// innate-capable ability now fails the build. Run it after editing this table.
//
// SLOT CHOICE MATTERS — because the table is gated by FEATURE_INNATE_ABILITIES, a row REPLACES that
// slot's ability only where the flag is on: all real gameplay/frontier, plus any FORK test that opts
// in with WITH_CONFIG. Upstream tests run flag-off and see vanilla slots, so a row can now safely
// repurpose a slot that an upstream `Ability(ABILITY_X)` or default read pins. Filling an EMPTY slot
// (ABILITY_NONE) is always safe. Repurposing a REAL slot deletes that ability from the species in
// every flag-on context, so audit that no *fork* test with the flag on observes it: `Ability(ABILITY_X)`
// on the species in test/fork/ (chiefly the innate test) and the frontier roster. (Upstream test/battle/
// pins no longer matter for slot choice, but a repurposed slot still changes real gameplay, so keep the
// pick a redundant innate or empty slot as before.) The rows above that repurpose real slots (Sceptile,
// Lopunny, Bronzong, Mamoswine, Beartic, Carracosta, Scovillain, Sinistcha, Volbeat, Zangoose, Pidgeot,
// Chatot, Crawdaunt, Klinklang, Bombirdier) were each audited this way.
static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    { // 0003
        SPECIES_VENUSAUR, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 0,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 1,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_MEGA, 2,
        ABILITY_GRASSY_SURGE
    },
    {
        SPECIES_VENUSAUR_GMAX, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0006
        SPECIES_CHARIZARD, 1,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 0,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 1,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_MEGA_X, 2,
        ABILITY_FLASH_FIRE
    },
    {
        SPECIES_CHARIZARD_GMAX, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0009
        SPECIES_BLASTOISE, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 0,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 1,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_MEGA, 2,
        ABILITY_WATER_ABSORB
    },
    {
        SPECIES_BLASTOISE_GMAX, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0012
        SPECIES_BUTTERFREE, 1,
        ABILITY_PSYCHIC_AFFINITY
    },
    {
        SPECIES_BUTTERFREE_GMAX, 1,
        ABILITY_PSYCHIC_AFFINITY
    },
    { // 0015
        SPECIES_BEEDRILL, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 0,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 1,
        ABILITY_POISON_TOUCH
    },
    {
        SPECIES_BEEDRILL_MEGA, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0018
        SPECIES_PIDGEOT, 1,
        ABILITY_NO_GUARD
    },
    { // 0022
        SPECIES_FEAROW, 1,
        ABILITY_HUSTLE
    },
    { // 0024
        SPECIES_ARBOK, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0026
        SPECIES_RAICHU_ALOLA, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0028
        SPECIES_SANDSLASH, 1,
        ABILITY_SAND_STREAM
    },
    { // 0028
        SPECIES_SANDSLASH_ALOLA, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0036
        // Clefable: all its real abilities are now innate, so its innate-redundant slot-1 Magic Guard takes a
        // chosen Misty Surge -- :x: (never an innate -> stable): it sets Misty Terrain on entry, blocking
        // status on grounded mons and halving Dragon moves.
        SPECIES_CLEFABLE, 1,
        ABILITY_MISTY_SURGE
    },
    {
        SPECIES_CLEFABLE_MEGA, 0,
        ABILITY_HALO
    },
    {
        SPECIES_CLEFABLE_MEGA, 1,
        ABILITY_HALO
    },
    {
        SPECIES_CLEFABLE_MEGA, 2,
        ABILITY_HALO
    },
    { // 0040
        SPECIES_WIGGLYTUFF, 0,
        ABILITY_HALO
    },
    { // 0045
        // Vileplume's real abilities are Chlorophyll (now innate) and Effect Spore. Effect Spore is a poor
        // chosen slot here: under DETERMINISTIC_ABILITIES it always sleeps contact attackers, which is
        // redundant with the set's own Sleep Powder. Its EMPTY slot 1 instead takes Solar Power -- :x:
        // (never an innate -> stable) and self-synergistic with the innate Chlorophyll on a sun team.
        SPECIES_VILEPLUME, 1,
        ABILITY_SOLAR_POWER
    },
    { // 0049
        SPECIES_VENOMOTH, 2,
        ABILITY_PSYCHIC_AFFINITY
    },
    { // 0051
        SPECIES_DUGTRIO, 2,
        ABILITY_SAND_STREAM
    },
    { // 0051
        SPECIES_DUGTRIO_ALOLA, 0,
        ABILITY_EARTH_EATER
    },
    { // 0053
        // Persian: all its real abilities are now innate, so its innate-redundant slot-2 Unnerve takes a chosen
        // Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x
        // power.
        SPECIES_PERSIAN, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0053
        // Persian-Alola: all its real abilities are now innate, so its innate-redundant slot-2 Rattled takes a
        // chosen Dark Aura -- :x: (never an innate -> stable): every Dark-type move on the field gains 1.33x
        // power.
        SPECIES_PERSIAN_ALOLA, 2,
        ABILITY_DARK_AURA
    },
    { // 0065
        SPECIES_ALAKAZAM, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0071
        // Victreebel: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Poison Touch --
        // :x: (never an innate -> stable): its contact moves carry a poison chance.
        SPECIES_VICTREEBEL, 1,
        ABILITY_POISON_TOUCH
    },
    {
        // Victreebel-Mega: all its real abilities are now innate, so its innate-redundant slot-0 Innards Out
        // takes a chosen Poison Touch -- :x: (never an innate -> stable): its contact moves carry a poison
        // chance.
        SPECIES_VICTREEBEL_MEGA, 0,
        ABILITY_POISON_TOUCH
    },
    {
        // Victreebel-Mega: all its real abilities are now innate, so its innate-redundant slot-1 Innards Out
        // takes a chosen Poison Touch -- :x: (never an innate -> stable): its contact moves carry a poison
        // chance.
        SPECIES_VICTREEBEL_MEGA, 1,
        ABILITY_POISON_TOUCH
    },
    {
        // Victreebel-Mega: all its real abilities are now innate, so its innate-redundant slot-2 Innards Out
        // takes a chosen Poison Touch -- :x: (never an innate -> stable): its contact moves carry a poison
        // chance.
        SPECIES_VICTREEBEL_MEGA, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0073
        // Tentacruel's three real abilities (Clear Body, Liquid Ooze, Rain Dish) are ALL now innate; slots 0/1
        // (Clear Body, Liquid Ooze) are pinned by tests (ai_switching.c / liquid_ooze.c / the innate test), so its
        // innate-redundant slot-2 Rain Dish -- unpinned by any test (audited) -- takes Water Absorb. Water Absorb is
        // :x: (never an innate -> stable) and thematic for the jellyfish: it shrugs off Water moves and heals from
        // them, a clean boon for its bulky spinner sets on top of the innate Liquid Ooze / Clear Body. Same pick as
        // the other Water walls (Suicune / Samurott / Clawitzer).
        SPECIES_TENTACRUEL, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0076
        SPECIES_GOLEM, 2,
        ABILITY_SAND_STREAM
    },
    { // 0076
        SPECIES_GOLEM_ALOLA, 0,
        ABILITY_LIGHTNING_ROD
    },
    { // 0078
        // Rapidash-Galar: Pastel Veil, Anticipation are now innate, so its innate-redundant slot-1 Pastel Veil
        // takes a chosen Misty Surge -- :x: (never an innate -> stable): it sets Misty Terrain on entry,
        // blocking status on grounded mons and halving Dragon moves.
        SPECIES_RAPIDASH_GALAR, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0080
        // Slowbro: all its real abilities are now innate, so its innate-redundant slot-1 Own Tempo takes a
        // chosen Storm Drain -- :x: (never an innate -> stable): Water moves are drawn in and absorbed for +1
        // Sp. Atk. (Slot audited: no flag-on fork test pins it.)
        SPECIES_SLOWBRO, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0080
        // Galarian Slowbro's three real abilities (Quick Draw, Own Tempo, Regenerator) are ALL now innate;
        // slot-0 Quick Draw + slot-1 Own Tempo are test-pinned (quick_draw.c / deterministic_abilities.c / the
        // innate test), so its innate-redundant slot-2 Regenerator (audited: no Ability(ABILITY_REGENERATOR) on
        // SLOWBRO_GALAR in test/) takes Poison Touch -- :x: (never an innate -> stable) and thematic for the
        // Poison/Psychic mon: its Shell Side Arm contact can poison the target, observable alongside the innate
        // Own Tempo / Quick Draw. Same Poison-flavor pick as Swalot / Skuntank.
        SPECIES_SLOWBRO_GALAR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0083
        SPECIES_FARFETCHD, 1,
        ABILITY_HUSTLE
    },
    { // 0085
        // Dodrio: all real abilities now innate, so its innate-redundant slot-0 Run Away takes a chosen
        // Hustle so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DODRIO, 0,
        ABILITY_HUSTLE
    },
    { // 0087
        // Dewgong's three real abilities (Thick Fat, Hydration, Ice Body) are ALL now innate, and it is used by
        // no test, so its innate-redundant slot-2 Ice Body takes a chosen Snow Warning -- :x: (never an innate ->
        // stable) and thematic for the sea-lion of the icy seas: it summons snow, which its own innate Ice Body
        // (and an Aurora Veil set) then cash in, observable alongside the innate Thick Fat / Hydration.
        SPECIES_DEWGONG, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0091
        SPECIES_CLOYSTER, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0094
        SPECIES_GENGAR, 1,
        ABILITY_ILLUSION
    },
    {
        SPECIES_GENGAR_GMAX, 1,
        ABILITY_ILLUSION
    },
    { // 0097
        // Hypno: all its real abilities are now innate, so its innate-redundant slot-1 Forewarn takes a chosen
        // Synchronize -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is passed
        // straight back.
        SPECIES_HYPNO, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0103
        // Exeggutor's only real abilities (Chlorophyll, Harvest) are BOTH now innate, so its EMPTY slot 1 takes
        // Sap Sipper -- :x: (never an innate -> stable) and thematic for the walking palm: Grass-type moves miss
        // it and instead boost its Attack, observable alongside the innate Chlorophyll / Harvest.
        SPECIES_EXEGGUTOR, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0103
        // Alolan Exeggutor's only real abilities (Frisk, Harvest) are BOTH now innate, so its EMPTY slot 1 takes
        // Sap Sipper -- :x: (never an innate -> stable), the same Grass-immunity pick as its Kantonian sibling and
        // fitting for the towering Grass/Dragon palm; its innate Harvest still recycles the set's berry.
        SPECIES_EXEGGUTOR_ALOLA, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0106
        // Hitmonlee's three real abilities (Limber, Reckless, Unburden) are ALL now innate; slot-0 Limber is
        // pinned by tests (last_respects.c / check_bad_move.c + the innate test's default read), so its innate-
        // redundant slot-2 Unburden (audited: no Ability(ABILITY_UNBURDEN) on Hitmonlee in test/) takes No Guard.
        // No Guard is :x: (never an innate -> stable) and thematic for the Kicking Pokemon: its High Jump Kick /
        // Blaze Kick / Stone Edge never miss (and HJK never crashes on a miss), a clean offensive boon alongside
        // its innate Reckless (recoil/crash boost) and Unburden (Speed). Same pick as Pidgeot.
        SPECIES_HITMONLEE, 2,
        ABILITY_NO_GUARD
    },
    { // 0107
        // Hitmonchan's three real abilities (Keen Eye, Iron Fist, Inner Focus) are ALL now innate; slot-0 Keen Eye
        // is test-pinned (keen_eye.c PARAMETRIZE + the default read in the innate test), so its innate-redundant
        // slot-2 Inner Focus (audited: no Ability(ABILITY_INNER_FOCUS) on Hitmonchan in test/) takes No Guard --
        // :x: (never an innate -> stable) and thematic for the Punching Pokemon: its Mach Punch / Ice Punch /
        // Thunder Punch / Close Combat never miss, a clean offensive boon alongside its innate Iron Fist (punch
        // power) and Inner Focus (flinch immunity). Same pick as the sibling fighters Hitmonlee / Hitmontop.
        SPECIES_HITMONCHAN, 2,
        ABILITY_NO_GUARD
    },
    { // 0113
        // Chansey: all its real abilities are now innate, so its innate-redundant slot-1 Serene Grace takes a
        // chosen Fluffy -- :x: (never an innate -> stable): contact damage is halved (at the cost of doubled
        // Fire damage).
        SPECIES_CHANSEY, 1,
        ABILITY_FLUFFY
    },
    { // 0115
        // Kangaskhan: all real abilities (Early Bird, Scrappy, Inner Focus) now innate; slot-2 Inner Focus is
        // Ability()-pinned in the fork innate test, so its innate-redundant slot-1 Scrappy takes Sheer Force --
        // :x: (never an innate -> stable) and a clean physical boon: its coverage (Body Slam / Earthquake /
        // Sucker Punch) trades secondaries for +30% power, observable atop innate Scrappy (Normal hits Ghosts).
        SPECIES_KANGASKHAN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0121
        // Starmie: all real abilities now innate, so its innate-redundant slot-1 Natural Cure takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_STARMIE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0127
        // Pinsir: all its real abilities are now innate, so its innate-redundant slot-0 Hyper Cutter takes a
        // chosen Aerilate -- :x: (never an innate -> stable): its Normal-type moves turn Flying and gain 1.2x
        // power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_PINSIR, 0,
        ABILITY_AERILATE
    },
    { // 0128
        // Tauros-Paldea-Combat's three real abilities (Intimidate, Anger Point, Cud Chew) are ALL now innate;
        // slot-0 Intimidate + slot-2 Cud Chew are test-pinned (innate test / cud_chew.c), so its innate-redundant
        // slot-1 Anger Point takes a chosen Sheer Force -- :x: (never an innate -> stable) and the Tauros line's
        // signature (base Tauros keeps its real Sheer Force below): raw force powers the raging bull's Raging Bull /
        // Close Combat, observable alongside the innate Intimidate.
        SPECIES_TAUROS_PALDEA_COMBAT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0128
        // Tauros-Paldea-Blaze -- same all-innate case as its Combat sibling; innate-redundant slot-1 Anger Point
        // (unpinned, audited) takes chosen Sheer Force (:x: -> stable; Tauros-line flavor) for its frontier set.
        SPECIES_TAUROS_PALDEA_BLAZE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0128
        // Tauros-Paldea-Aqua -- same all-innate case; innate-redundant slot-1 Anger Point (unpinned, audited)
        // takes chosen Sheer Force (:x: -> stable; Tauros-line flavor) for its frontier set.
        SPECIES_TAUROS_PALDEA_AQUA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0130
        // Gyarados's real abilities (Intimidate, Moxie) are BOTH now innate, so its empty slot 1 takes a chosen
        // Motor Drive -- :x: (never an innate -> stable) and a clean boon for the Dragon Dance sweeper: it turns
        // Gyarados's 2x Electric weakness into an immunity plus a Speed boost, observable alongside the innate
        // Intimidate. (Its slot-2 Moxie frontier set is pinned by dynamax.c and stays a real Moxie.)
        SPECIES_GYARADOS, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0142
        // Aerodactyl: all its real abilities are now innate, so its innate-redundant slot-2 Unnerve takes a
        // chosen Hustle -- :x: (never an innate -> stable): its physical attacks gain 1.5x power for an
        // accuracy cost. (Slot audited: no flag-on fork test pins it.)
        SPECIES_AERODACTYL, 2,
        ABILITY_HUSTLE
    },
    { // 0143
        // Snorlax: all real abilities (Immunity, Thick Fat, Gluttony) now innate; slot-0 Immunity is the fork
        // innate test's default read and slot-2 Gluttony is Ability()-pinned there, so its innate-redundant
        // slot-1 Thick Fat takes Sap Sipper -- :x: (never an innate -> stable) and thematic for the Pokemon that
        // eats anything: Grass moves miss it and boost its Attack, observable atop innate Thick Fat.
        SPECIES_SNORLAX, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0144
        // Articuno's only real abilities (Pressure, Snow Cloak) are BOTH now innate, so its empty
        // slot 1 takes Snow Warning — :x: (never an innate -> stable) and flavorful (the legendary ice
        // bird heralds the blizzard), setting the snow that turns on its own innate Snow Cloak evasion.
        SPECIES_ARTICUNO, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0144
        // Articuno-Galar: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Psychic Surge -- :x: (never an innate ->
        // stable): it sets Psychic Terrain on entry, boosting Psychic moves 1.3x and blocking priority on
        // grounded mons.
        SPECIES_ARTICUNO_GALAR, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0145
        // Zapdos-Galar: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Hustle -- :x: (never an innate -> stable): its
        // physical attacks gain 1.5x power for an accuracy cost.
        SPECIES_ZAPDOS_GALAR, 1,
        ABILITY_HUSTLE
    },
    { // 0146
        // Moltres-Galar: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Dark Aura -- :x: (never an innate -> stable):
        // every Dark-type move on the field gains 1.33x power.
        SPECIES_MOLTRES_GALAR, 1,
        ABILITY_DARK_AURA
    },
    { // 0149
        // Dragonite: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Aerilate -- :x:
        // (never an innate -> stable): its Normal-type moves turn Flying and gain 1.2x power.
        SPECIES_DRAGONITE, 1,
        ABILITY_AERILATE
    },
    { // 0150
        // Mewtwo: all real abilities now innate, so its empty slot takes a chosen
        // Synchronize so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_MEWTWO, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0154
        // Meganium's only real abilities (Overgrow, Leaf Guard) are BOTH now innate, so its EMPTY slot 1 takes
        // Grassy Surge -- :x: (never an innate -> stable) and thematic for the Herb Pokemon: the terrain it
        // sets powers its Grass STAB and passively heals its bulky sets. Same pick as Venusaur/Celebi.
        SPECIES_MEGANIUM, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0157
        // Typhlosion Hisui: all real abilities now innate, so its empty slot takes a chosen
        // Flash Fire so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_TYPHLOSION_HISUI, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0162
        // Furret: all real abilities now innate, so its innate-redundant slot-0 Run Away takes a chosen
        // Hustle so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_FURRET, 0,
        ABILITY_HUSTLE
    },
    { // 0164
        // Noctowl: all its real abilities are now innate, so its innate-redundant slot-0 Insomnia takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power.
        SPECIES_NOCTOWL, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0166
        // Ledian: all its real abilities are now innate, so its innate-redundant slot-0 Swarm takes a chosen
        // Victory Star -- :x: (never an innate -> stable): it and its allies gain 1.1x accuracy.
        SPECIES_LEDIAN, 0,
        ABILITY_VICTORY_STAR
    },
    { // 0168
        // Ariados: all real abilities now innate, so its innate-redundant slot-1 Insomnia takes a chosen
        // Poison Point so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_ARIADOS, 1,
        ABILITY_POISON_POINT
    },
    { // 0169
        // Crobat: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Poison Touch -- :x:
        // (never an innate -> stable): its contact moves carry a poison chance.
        SPECIES_CROBAT, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0182
        // Bellossom's only real abilities (Chlorophyll, Healer) are BOTH now innate, so its EMPTY slot 1 takes
        // Solar Power -- :x: (never an innate -> stable) and self-synergistic with the innate Chlorophyll on a
        // sun team, boosting its special Quiver Dance set. (Effect Spore was moved off: under
        // DETERMINISTIC_ABILITIES it always sleeps contact attackers, redundant with the set's own Sleep Powder.)
        SPECIES_BELLOSSOM, 1,
        ABILITY_SOLAR_POWER
    },
    { // 0185
        // Sudowoodo: all its real abilities are now innate, so its innate-redundant slot-1 Rock Head takes a
        // chosen Illusion -- :x: (never an innate -> stable): it enters disguised as the party's last member
        // until it takes a direct hit. (Slot audited: no flag-on fork test pins it.)
        SPECIES_SUDOWOODO, 1,
        ABILITY_ILLUSION
    },
    { // 0189
        // Jumpluff: all its real abilities are now innate, so its innate-redundant slot-1 Leaf Guard takes a
        // chosen Effect Spore -- :x: (never an innate -> stable): contact attackers risk poison, paralysis or
        // sleep. (Slot audited: no flag-on fork test pins it.)
        SPECIES_JUMPLUFF, 1,
        ABILITY_EFFECT_SPORE
    },
    { // 0199
        // Slowking's three real abilities (Oblivious, Own Tempo, Regenerator) are ALL now innate; slot-0 Oblivious
        // is the default read (psyblade.c) and slot-1 Own Tempo is test-pinned (frontier_battle_info_reveal.c), so
        // its innate-redundant slot-2 Regenerator (audited: no Ability(ABILITY_REGENERATOR) on Slowking in test/)
        // takes Water Absorb -- :x: (never an innate -> stable) and thematic for the Water/Psychic wall: it shrugs
        // off Water moves and heals from them, a clean boon for its bulky Slack Off / Calm Mind sets alongside the
        // innate Regenerator / Own Tempo. Same pick as the other Water walls (Suicune / Tentacruel / Samurott).
        SPECIES_SLOWKING, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0202
        // Wobbuffet: all real abilities now innate, so its empty slot takes a chosen
        // Synchronize so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_WOBBUFFET, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0205
        // Forretress: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Bulletproof --
        // :x: (never an innate -> stable): ball, bomb and cannon moves (Focus Blast, Shadow Ball, Sludge Bomb)
        // cannot touch it.
        SPECIES_FORRETRESS, 1,
        ABILITY_BULLETPROOF
    },
    { // 0210
        // Granbull's three real abilities (Intimidate, Quick Feet, Rattled) are ALL now innate, so its innate-
        // redundant slot-2 Rattled (audited: no test references Granbull at all) takes a chosen Static -- :x:
        // (never an innate -> stable), pure-upside, and thematic for the charged-fur Fairy bulldog: contact
        // attackers risk paralysis, which pairs with its Thunder Wave / Play Rough pivot sets.
        SPECIES_GRANBULL, 2,
        ABILITY_STATIC
    },
    { // 0212
        // Scizor: all its real abilities are now innate, so its innate-redundant slot-1 Technician takes a
        // chosen Well-Baked Body -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for
        // +2 Defense. (Slot audited: no flag-on fork test pins it.)
        SPECIES_SCIZOR, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0214
        // Heracross: all real abilities now innate, so its innate-redundant slot-2 Moxie takes a chosen
        // No Guard so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_HERACROSS, 2,
        ABILITY_NO_GUARD
    },
    { // 0217
        // Ursaring: all real abilities now innate, so its innate-redundant slot-1 Quick Feet takes a chosen
        // Hustle so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_URSARING, 1,
        ABILITY_HUSTLE
    },
    { // 0227
        // Skarmory's only non-drawback real abilities (Keen Eye, Sturdy) are BOTH now innate (Weak Armor,
        // slot 2, is a drawback on a wall), so its slot-1 Sturdy — now innate-redundant — takes Bulletproof.
        // Bulletproof is :x: (never an innate -> stable) and a clean defensive boon: this Spikes/Defog wall
        // shrugs off Focus Blast, Sludge Bomb, Energy Ball and the other ball/bomb moves that threaten it.
        SPECIES_SKARMORY, 1,
        ABILITY_BULLETPROOF
    },
    { // 0232
        // Donphan's only real abilities (Sturdy, Sand Veil) are BOTH now innate, so its empty slot 1
        // takes Sand Stream — :x: (never an innate -> stable) and self-synergistic: the sandstorm it
        // sets turns on its own innate Sand Veil evasion. Same pick as base Sandslash above.
        SPECIES_DONPHAN, 1,
        ABILITY_SAND_STREAM
    },
    { // 0237
        // Hitmontop's three real abilities (Intimidate, Technician, Steadfast) are ALL now innate; slot-0
        // Intimidate is test-pinned (intimidate.c / ai_switching.c), so its innate-redundant slot-2 Steadfast
        // (audited: unpinned) takes a chosen No Guard -- :x: (never an innate -> stable) and thematic for the
        // spinning martial artist: its Triple Axel / Close Combat / Rapid Spin never miss. Same pick as Pidgeot /
        // Hitmonlee.
        SPECIES_HITMONTOP, 2,
        ABILITY_NO_GUARD
    },
    { // 0242
        // Blissey: all its real abilities are now innate, so its innate-redundant slot-2 Healer takes a chosen
        // Fluffy -- :x: (never an innate -> stable): contact damage is halved (at the cost of doubled Fire
        // damage).
        SPECIES_BLISSEY, 2,
        ABILITY_FLUFFY
    },
    { // 0243
        // Raikou's only real abilities (Pressure, Inner Focus) are BOTH now innate, so its EMPTY slot 1 takes
        // Lightning Rod -- :x: (never an innate -> stable) and thematic for the thunder beast: an Electric
        // immunity + Sp. Atk boost for its special attacker sets.
        SPECIES_RAIKOU, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0244
        // Entei's only real abilities (Pressure, Inner Focus) are BOTH now innate, so its EMPTY slot 1 takes
        // Flame Body -- :x: (never an innate -> stable) and thematic for the volcano beast: contact attackers
        // risk a burn. Same pick as Ho-Oh/Turtonator.
        SPECIES_ENTEI, 1,
        ABILITY_FLAME_BODY
    },
    { // 0245
        // Suicune's only real abilities (Pressure, Inner Focus) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the north wind beast: it shrugs off
        // Water moves and heals from them, a clean boon for its bulky Calm Mind sets.
        SPECIES_SUICUNE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0249
        // Lugia's only real abilities (Pressure, Multiscale) are BOTH now innate, so its empty slot 1 takes
        // a chosen Storm Drain — :x: (never an innate -> stable) and flavorful (the guardian of the seas draws
        // in Water), giving the Calm Mind set a Water immunity + Sp. Atk boost.
        SPECIES_LUGIA, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0250
        SPECIES_HO_OH, 1,
        ABILITY_FLAME_BODY
    },
    { // 0251
        SPECIES_CELEBI, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0254
        SPECIES_SCEPTILE, 2,
        ABILITY_LIGHTNING_ROD
    },
    { // 0257
        // Blaziken's only real abilities (Blaze, Speed Boost) are BOTH now innate, so its empty
        // slot 1 takes a flavorful chosen ability for the frontier sets. Sheer Force is :x:
        // (never an innate -> stable) and complements its snowballing physical wallbreaker sets
        // (Close Combat / Blaze Kick / Thunder Punch all gain the +30% and drop their secondaries).
        SPECIES_BLAZIKEN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0262
        // Mightyena's three real abilities (Intimidate, Quick Feet, Moxie) are ALL now innate; slot-0 Intimidate is
        // test-pinned (innate test / opportunist.c / contrary.c), so its innate-redundant slot-2 Moxie (audited: no
        // Ability(ABILITY_MOXIE) on Mightyena in test/) takes a chosen Sheer Force -- :x: (never an innate -> stable)
        // and a clean boon for the vicious biter: its Crunch / Play Rough / Fire Fang gain +30% and drop their
        // secondaries, observable alongside the innate Intimidate.
        SPECIES_MIGHTYENA, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0264
        // Linoone: all its real abilities are now innate, so its innate-redundant slot-1 Gluttony takes a
        // chosen Hustle -- :x: (never an innate -> stable): its physical attacks gain 1.5x power for an
        // accuracy cost. (Slot audited: no flag-on fork test pins it.)
        SPECIES_LINOONE, 1,
        ABILITY_HUSTLE
    },
    { // 0269
        // Dustox's only real abilities (Shield Dust, Compound Eyes) are BOTH now innate, so its EMPTY slot 1
        // takes Poison Point — :x: (never an innate -> stable) and flavorful: the toxic-dust moth poisons the
        // contact its Rocky Helmet wall set already punishes.
        SPECIES_DUSTOX, 1,
        ABILITY_POISON_POINT
    },
    { // 0272
        // Ludicolo: all real abilities (Swift Swim, Rain Dish, Own Tempo) now innate; slot-2 Own Tempo is
        // Ability()-pinned in the fork innate test, so its innate-redundant slot-1 Rain Dish takes Storm Drain --
        // :x: (never an innate -> stable) and thematic for the carefree dancer: it draws in Water moves for a
        // Sp. Atk boost + immunity, a clean boon for its rain special sweeper atop innate Swift Swim.
        SPECIES_LUDICOLO, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0277
        // Swellow: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Wind Rider -- :x:
        // (never an innate -> stable): wind moves and Tailwind give it +1 Attack instead of landing.
        SPECIES_SWELLOW, 1,
        ABILITY_WIND_RIDER
    },
    { // 0284
        // Masquerain's only real abilities (Intimidate, Unnerve) are BOTH now innate, so its EMPTY slot 1 takes a
        // chosen Storm Drain -- :x: (never an innate -> stable) and thematic for the pond-skating water strider: it
        // draws in Water moves for a Sp. Atk boost + immunity, a clean boon for its Quiver Dance special sweeper.
        SPECIES_MASQUERAIN, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0286
        // Breloom's real abilities are Effect Spore, Poison Heal and Technician; the latter two are now innate.
        // Its natural Effect Spore is a poor chosen slot: under DETERMINISTIC_ABILITIES it always sleeps contact
        // attackers, redundant with the set's own Spore. Its innate-redundant slot-1 Poison Heal (kept as an
        // innate, and unpinned by any test -- audited) takes Hustle instead -- :x: (never an innate -> stable):
        // +50% Attack for the punching mushroom, its accuracy cost softened to a small PP tax under
        // DETERMINISTIC_ACCURACY_EVASION. Slot 0 stays real Effect Spore (pinned by sleep_clause.c / safety_goggles.c).
        SPECIES_BRELOOM, 1,
        ABILITY_HUSTLE
    },
    { // 0291
        // Ninjask: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Hustle -- :x:
        // (never an innate -> stable): its physical attacks gain 1.5x power for an accuracy cost.
        SPECIES_NINJASK, 1,
        ABILITY_HUSTLE
    },
    { // 0302
        // Sableye: Keen Eye, Prankster are now innate, so its slot-1 Stall takes a chosen Wandering Spirit --
        // :x: (never an innate -> stable): contact attackers have their ability swapped with its own.
        SPECIES_SABLEYE, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0306
        // Aggron: all its real abilities are now innate, so its innate-redundant slot-1 Rock Head takes a
        // chosen Well-Baked Body -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for
        // +2 Defense. (Slot audited: no flag-on fork test pins it.)
        SPECIES_AGGRON, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0308
        // Medicham: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_MEDICHAM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0313
        // Volbeat's three real abilities (Illuminate, Swarm, Prankster) are ALL now innate, so its slot-1
        // Swarm — now innate-redundant — takes Victory Star. Victory Star is :x: (never an innate -> stable)
        // and flavorful for the firefly's guiding light: it boosts its doubles allies' accuracy. (Azelf also
        // hands out Victory Star.)
        SPECIES_VOLBEAT, 1,
        ABILITY_VICTORY_STAR
    },
    { // 0314
        // Illumise: all its real abilities are now innate, so its innate-redundant slot-0 Oblivious takes a
        // chosen Lingering Aroma -- :x: (never an innate -> stable): contact attackers have their own ability
        // replaced by Lingering Aroma.
        SPECIES_ILLUMISE, 0,
        ABILITY_LINGERING_AROMA
    },
    { // 0317
        // Swalot's three real abilities (Liquid Ooze, Sticky Hold, Gluttony) are ALL now innate, and NONE is
        // test-pinned (audited: no reference to Swalot in test/), so its innate-redundant slot-2 Gluttony takes
        // Poison Touch. Poison Touch is :x: (never an innate -> stable) and a clean boon for the Poison Bag's
        // Sticky Hold status tank: its Gunk Shot / Body Slam contact can poison the target, adding chip to its
        // Toxic / Encore disruption, on top of the innate Liquid Ooze (punishing drainers) and Sticky Hold.
        SPECIES_SWALOT, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0319
        // Sharpedo: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_SHARPEDO, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0321
        // Wailord: all real abilities now innate, so its innate-redundant slot-2 Pressure takes a chosen
        // Drizzle so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_WAILORD, 2,
        ABILITY_DRIZZLE
    },
    { // 0323
        // Camerupt: all real abilities (Magma Armor, Solid Rock, Anger Point) now innate; slot-0 Magma Armor is
        // the default read and slot-1 Solid Rock is Ability()-pinned in the fork innate test, so its innate-
        // redundant slot-2 Anger Point takes Sheer Force -- :x: (never an innate -> stable) and a clean special
        // boon: Earth Power / Lava Plume / Flamethrower trade secondaries for +30% power, atop innate Solid Rock.
        SPECIES_CAMERUPT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0326
        // Grumpig's three real abilities (Thick Fat, Own Tempo, Gluttony) are ALL now innate; slot-0 Thick Fat is the
        // default read and slot-2 Gluttony is the chosen-differs-from-innate exemplar in test/fork/innate_abilities.c,
        // so its innate-redundant slot-1 Own Tempo (audited: no Ability(ABILITY_OWN_TEMPO) on Grumpig in test/) takes
        // Synchronize -- :x: (never an innate -> stable) and thematic for the Psychic pig: burns / poison / paralysis
        // inflicted on it bounce back, a clean boon for its Thick Fat / Calm Mind special tank alongside the innate
        // Own Tempo (confusion immunity) and Gluttony.
        SPECIES_GRUMPIG, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0330
        SPECIES_FLYGON, 1,
        ABILITY_SAND_STREAM
    },
    { // 0335
        // Zangoose carries innate Toxic Boost (its frontier identity), so its slot-2 real Toxic Boost is
        // now redundant and its slot-0 Immunity would CONTRADICT it (Immunity blocks the poison Toxic
        // Boost needs). Its empty slot 1 therefore takes a chosen Sheer Force — :x: (never an innate ->
        // stable) and a clean offensive boon for the Cat Ferret's physical sets: coverage moves drop their
        // secondaries for +30% power (and a Life Orb set skips the recoil), stacking with innate Toxic Boost.
        SPECIES_ZANGOOSE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0336
        // Infiltrator now innate (slot-0 Shed Skin is pending), so its EMPTY slot 1 takes Poison Point -- :x:
        // (never an innate -> stable) and flavorful: the fanged snake poisons contact attackers.
        SPECIES_SEVIPER, 1,
        ABILITY_POISON_POINT
    },
    { // 0337
        SPECIES_LUNATONE, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0338
        SPECIES_SOLROCK, 1,
        ABILITY_DROUGHT
    },
    { // 0340
        // Whiscash's three real abilities (Oblivious, Anticipation, Hydration) are ALL now innate; slot-1 Anticipation
        // is the chosen-differs exemplar in the innate test (+ anticipation.c), so its unpinned innate-redundant slot-2
        // Hydration (audited: no test selects it) takes Storm Drain -- :x: (never an innate -> stable) and thematic for
        // the pond-dwelling catfish: it draws in Water moves for a Sp. Atk boost, observable alongside innate Oblivious.
        SPECIES_WHISCASH, 2,
        ABILITY_STORM_DRAIN
    },
    { // 0342
        // Crawdaunt: all its real abilities are now innate, so its innate-redundant slot-1 Shell Armor takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_CRAWDAUNT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0344
        SPECIES_CLAYDOL, 1, 
        ABILITY_SAND_STREAM
    },
    { // 0348
        SPECIES_ARMALDO, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0350
        // Milotic: all real abilities now innate, so its innate-redundant slot-2 Cute Charm takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_MILOTIC, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0354
        // Banette's three real abilities (Insomnia, Frisk, Cursed Body) are ALL now innate; slot-0 Insomnia is the
        // default read by terastal.c, so its innate-redundant slot-2 Cursed Body -- unpinned by any test (audited)
        // -- takes Wandering Spirit. Wandering Spirit is :x: (never an innate -> stable) and thematic for the
        // possessed Marionette Pokemon (the Mismagius precedent): contact attackers swap Abilities with the doll, a
        // disruptive boon on top of the innate Frisk / Cursed Body. (Both frontier sets, formerly on the now-innate
        // Frisk / Cursed Body, select this Wandering Spirit slot.)
        SPECIES_BANETTE, 2,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0356
        // Dusclops: all real abilities now innate, so its empty slot takes a chosen
        // Mummy so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DUSCLOPS, 1,
        ABILITY_MUMMY
    },
    { // 0358
        SPECIES_CHIMECHO, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0359
        // Absol: all its real abilities are now innate, so its innate-redundant slot-2 Justified takes a chosen
        // Dark Aura -- :x: (never an innate -> stable): every Dark-type move on the field gains 1.33x power.
        SPECIES_ABSOL, 2,
        ABILITY_DARK_AURA
    },
    { // 0365
        // Walrein's three real abilities (Thick Fat, Ice Body, Oblivious) are ALL now innate; slot-0 Thick Fat is the
        // chosen-differs exemplar in the innate test, so its unpinned innate-redundant slot-1 Ice Body (audited: no
        // test selects it) takes Water Absorb -- :x: (never an innate -> stable) and thematic for the blubbery Ice/Water
        // wall: it heals on the Water hits it invites, observable alongside the innate Thick Fat / Ice Body.
        SPECIES_WALREIN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0367
        // Huntail's only real abilities (Swift Swim, Water Veil) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the deep-sea fish: it heals on the
        // Water hits its Shell Smash sweeper invites.
        SPECIES_HUNTAIL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0368
        // Gorebyss's only real abilities (Swift Swim, Hydration) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the deep-sea siren: it heals on the Water
        // hits its Shell Smash sweeper invites, the same pick as its Huntail counterpart.
        SPECIES_GOREBYSS, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0369
        // Swift Swim, Rock Head and Sturdy are ALL now innate, so its innate-redundant slot-1 Rock Head
        // (audited: unpinned) takes Water Absorb -- :x: (never an innate -> stable) and thematic for the
        // deep-sea fossil: it heals on the Water hits a Rock Polish sweeper invites.
        SPECIES_RELICANTH, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0370
        // Luvdisc's only real abilities (Swift Swim, Hydration) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the Water fish: it heals on the Water
        // hits it invites, observable alongside the innate Swift Swim / Hydration.
        SPECIES_LUVDISC, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0373
        // Salamence's only real abilities (Intimidate, Moxie) are BOTH now innate, so its EMPTY slot 1 takes a chosen
        // Rivalry -- :x: (never an innate -> stable) and thematic for the ferocious, territorial dragon: it hits
        // same-gender foes 25% harder, observable on its Dragon Dance / Choice sweeper sets alongside the innate
        // Intimidate.
        SPECIES_SALAMENCE, 1,
        ABILITY_RIVALRY
    },
    { // 0376
        // Metagross: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_METAGROSS, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0377
        // Regirock: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sand Stream --
        // :x: (never an innate -> stable): it sets sandstorm on entry, chipping the field and boosting Rock-
        // type Sp. Def.
        SPECIES_REGIROCK, 1,
        ABILITY_SAND_STREAM
    },
    { // 0378
        // Regice: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Snow Warning -- :x:
        // (never an innate -> stable): it sets snow on entry, boosting Ice-type Defense.
        SPECIES_REGICE, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0379
        // Registeel's only real abilities are Clear Body (slot 0) and Light Metal (slot 2), BOTH now innate, so
        // its EMPTY slot 1 takes Bulletproof, :x: (never an innate -> stable) and thematic for the iron golem: it
        // deflects Focus Blast / Flash Cannon and the other ball/bomb moves. Same pick as Skarmory.
        SPECIES_REGISTEEL, 1,
        ABILITY_BULLETPROOF
    },
    { // 0380
        SPECIES_LATIAS, 1,
        ABILITY_ILLUSION
    },
    { // 0381
        SPECIES_LATIOS, 1,
        ABILITY_ILLUSION
    },
    { // 0385
        // Jirachi's only real ability (Serene Grace) is now innate, so its empty slot 1 takes a
        // flavorful chosen ability. Victory Star is :x: (never an innate -> stable) and on-theme: the
        // wishing star grants fortune, boosting the accuracy of itself and its doubles allies. (Azelf
        // and Volbeat also hand out Victory Star.)
        SPECIES_JIRACHI, 1,
        ABILITY_VICTORY_STAR
    },
    { // 0386
        SPECIES_DEOXYS_ATTACK, 1,
        ABILITY_TRACE
    },
    { // 0386
        SPECIES_DEOXYS_DEFENSE, 1,
        ABILITY_TRACE
    },
    { // 0386
        SPECIES_DEOXYS_SPEED, 1, 
        ABILITY_TRACE
    },
    { // 0389
        SPECIES_TORTERRA, 1,
        ABILITY_SAND_STREAM
    },
    { // 0392
        // Infernape: all real abilities now innate, so its empty slot takes a chosen
        // Flash Fire so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_INFERNAPE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0395
        // Empoleon: all real abilities now innate, so its empty slot takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_EMPOLEON, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0398
        // Staraptor's only real abilities (Intimidate, Reckless) are BOTH now innate, so its EMPTY slot 1 takes a
        // chosen Hustle -- :x: (never an innate -> stable) and thematic for the reckless raptor: +50% Attack powers
        // its Brave Bird / Double-Edge / Close Combat spam (the -20% accuracy is the brash predator's price),
        // stacking with the innate Reckless recoil boost.
        SPECIES_STARAPTOR, 1,
        ABILITY_HUSTLE
    },
    { // 0401
        // Kricketune's only real abilities (Swarm, Technician) are BOTH now innate, so its empty slot 1
        // takes a flavorful chosen ability for the frontier set. Sheer Force is :x: (never an innate ->
        // stable) and a pure boon for its WIDE_LENS Swords Dance sweeper: Pounce gains the +30% and drops
        // its Speed-lowering secondary, on top of the innate Technician Fury Cutter ramp.
        SPECIES_KRICKETUNE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0416
        // Vespiquen: all real abilities now innate, so its empty slot 1 takes Water Absorb -- :x: (never an
        // innate -> stable) and a clean defensive boon for the Roost / Defog / Toxic staller (Water immunity +
        // recovery). Effect Spore was moved off: under DETERMINISTIC_ABILITIES it always sleeps contact
        // attackers, which pre-empts the set's own Toxic for the single status slot -- actively fighting the plan.
        SPECIES_VESPIQUEN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0419
        // Floatzel's only real abilities (Swift Swim, Water Veil) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the sea weasel: it heals on the Water
        // moves its bulky-water switch-ins invite, alongside the innate Swift Swim speed.
        SPECIES_FLOATZEL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0424
        // Ambipom: all its real abilities are now innate, so its innate-redundant slot-1 Pickup takes a chosen
        // Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x
        // power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_AMBIPOM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0426
        // Drifblim: all its real abilities are now innate, so its innate-redundant slot-0 Aftermath takes a
        // chosen Cloud Nine -- :x: (never an innate -> stable): all weather effects are switched off while it
        // is on the field. (Slot audited: no flag-on fork test pins it.)
        SPECIES_DRIFBLIM, 0,
        ABILITY_CLOUD_NINE
    },
    { // 0428
        // Lopunny's only real non-drawback abilities (Cute Charm, Limber) are BOTH now innate
        // (Klutz, slot 1, is a drawback), so its slot-2 Limber — now innate-redundant — takes a
        // flavorful chosen ability for the frontier set. Sheer Force is :x: (never an innate ->
        // stable) and a pure boon for its offensive Fake Out / Ice Punch breaker set.
        SPECIES_LOPUNNY, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0429
        SPECIES_MISMAGIUS, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0430
        // Honchkrow: all its real abilities are now innate, so its innate-redundant slot-1 Super Luck takes a
        // chosen Dark Aura -- :x: (never an innate -> stable): every Dark-type move on the field gains 1.33x
        // power.
        SPECIES_HONCHKROW, 1,
        ABILITY_DARK_AURA
    },
    { // 0432
        // Purugly: all real abilities now innate, so its innate-redundant slot-0 Thick Fat takes a chosen
        // Hustle so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_PURUGLY, 0,
        ABILITY_HUSTLE
    },
    { // 0435
        // Skuntank's three real abilities (Stench, Aftermath, Keen Eye) are ALL now innate, so its innate-
        // redundant slot-1 Aftermath -- unpinned by any test (audited: aftermath.c uses Voltorb) -- takes
        // Poison Touch. Poison Touch is :x: (never an innate -> stable) and a clean boon for the skunk's
        // physical pivot: its Gunk Shot / Crunch contact can poison the target.
        SPECIES_SKUNTANK, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0437
        // Bronzong's Levitate and Heatproof are now innate, freeing its frontier slot; its remaining slot-2
        // Heavy Metal is dead weight (it sets no weight moves and only worsens Low Kick / Grass Knot), so the
        // slot is repurposed to Soundproof — :x: (never an innate -> stable) and a thematic clean boon: the
        // bell shrugs off Hyper Voice / Boomburst and other sound moves. (Sceptile-style dead-weight repurpose.)
        SPECIES_BRONZONG, 2,
        ABILITY_SOUNDPROOF
    },
    { // 0441
        // Chatot: all its real abilities are now innate, so its innate-redundant slot-1 Tangled Feet takes a
        // chosen Aerilate -- :x: (never an innate -> stable): its Normal-type moves turn Flying and gain 1.2x
        // power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_CHATOT, 1,
        ABILITY_AERILATE
    },
    { // 0442
        // Spiritomb: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Mummy -- :x:
        // (never an innate -> stable): contact attackers have their own ability replaced by Mummy.
        SPECIES_SPIRITOMB, 1,
        ABILITY_MUMMY
    },
    { // 0445
        // Garchomp's only real abilities (Sand Veil, Rough Skin) are BOTH now innate, so its EMPTY slot 1 takes
        // Sand Stream — :x: (never an innate -> stable) and self-synergistic for the desert dragon: the sandstorm
        // it kicks up turns on its own innate Sand Veil evasion (and chips non-Ground/Rock/Steel foes). Same pick
        // as the other Ground sweepers (Dugtrio / Donphan / Flygon / Torterra).
        SPECIES_GARCHOMP, 1,
        ABILITY_SAND_STREAM
    },
    { // 0448
        // Lucario: all real abilities now innate, so its innate-redundant slot-1 Inner Focus takes a chosen
        // No Guard so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_LUCARIO, 1,
        ABILITY_NO_GUARD
    },
    { // 0448
        // Lucario: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its innate-redundant slot-2 Justified takes a chosen Sheer Force -- :x: (never
        // an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_LUCARIO, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0452
        // Drapion: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its innate-redundant slot-1 Sniper takes a chosen Poison Touch -- :x: (never an
        // innate -> stable): its contact moves carry a poison chance.
        SPECIES_DRAPION, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0455
        // Carnivine: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Seed Sower --
        // :x: (never an innate -> stable): being hit sets Grassy Terrain.
        SPECIES_CARNIVINE, 1,
        ABILITY_SEED_SOWER
    },
    { // 0461
        // Weavile: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Dark Aura -- :x:
        // (never an innate -> stable): every Dark-type move on the field gains 1.33x power.
        SPECIES_WEAVILE, 1,
        ABILITY_DARK_AURA
    },
    { // 0462
        // Magnezone's three real abilities (Magnet Pull, Sturdy, Analytic) are ALL now innate; slot-0 Magnet Pull and
        // slot-1 Sturdy are pinned by tests, so its innate-redundant slot-2 Analytic (audited: no Ability(ABILITY_ANALYTIC)
        // on Magnezone) takes Lightning Rod — :x: (never an innate -> stable) and thematic for the magnet UFO: it draws in
        // Electric moves for immunity + a Sp. Atk boost. Same pick as Rotom/Eelektross.
        SPECIES_MAGNEZONE, 2,
        ABILITY_LIGHTNING_ROD
    },
    { // 0465
        SPECIES_TANGROWTH, 2,
        ABILITY_SAP_SIPPER
    },
    { // 0468
        // Togekiss: all real abilities now innate, so its innate-redundant slot-1 Serene Grace takes a chosen
        // Sheer Force so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_TOGEKISS, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0469
        // Yanmega: all real abilities now innate, so its innate-redundant slot-1 Tinted Lens takes a chosen
        // Sheer Force so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_YANMEGA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0470
        // Leafeon's real abilities (Leaf Guard in slots 0 AND 1, Chlorophyll in slot 2) are ALL now innate; slot-0
        // Leaf Guard is the chosen-differs-from-innate exemplar in test/fork/innate_abilities.c (and leaf_guard.c),
        // so its innate-redundant DUPLICATE slot-1 Leaf Guard (never selected -- Ability(ABILITY_LEAF_GUARD) resolves
        // to slot 0) takes Sap Sipper -- :x: (never an innate -> stable) and thematic for the Verdant Pokemon: Grass
        // moves give it an Attack boost + immunity, a clean boon for its Swords Dance physical sweeper alongside the
        // innate Chlorophyll (Speed in sun). Same pick as the other Grass mons (Tangrowth).
        SPECIES_LEAFEON, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0471
        // Glaceon's real abilities (Snow Cloak in slots 0 & 1, Ice Body) are ALL now innate; Snow Cloak resolves to
        // slot 0 (pinned by snow_cloak.c / pursuit.c / mega_sol.c + the default damage tests), so its unpinned innate-
        // redundant slot-2 Ice Body (audited) takes Snow Warning -- :x: (never an innate -> stable) and self-synergistic:
        // the snow the Fresh Snow fox heralds powers its own innate Snow Cloak evasion and Ice Body heal.
        SPECIES_GLACEON, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0472
        // Gliscor: all its real abilities are now innate, so its innate-redundant slot-2 Poison Heal takes a
        // chosen Poison Touch -- :x: (never an innate -> stable): its contact moves carry a poison chance.
        // (Slot audited: no flag-on fork test pins it.)
        SPECIES_GLISCOR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0473
        // Mamoswine's three real abilities (Oblivious, Snow Cloak, Thick Fat) are ALL now innate, so
        // its slot-2 Thick Fat — now innate-redundant — is repurposed to Snow Warning. Snow Warning is
        // :x: (never an innate -> stable) and self-synergistic: the snow the prehistoric mammoth heralds
        // turns on its own innate Snow Cloak evasion. Same pick as Beartic/Articuno above.
        SPECIES_MAMOSWINE, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0474
        // Porygon-Z's three real abilities (Adaptability, Download, Analytic) are ALL now innate. Slot 0
        // (Adaptability) is test-pinned (Ability(ABILITY_ADAPTABILITY) in test/fork/innate_abilities.c), so its
        // innate-redundant slot-2 Analytic (its Hidden Ability, the Sceptile model) is repurposed to Simple.
        // Simple is :x: (never an innate -> stable) and thematic for the glitched virtual Pokemon: it doubles
        // the Nasty Plot boost on its special nuke, so both innates + the chosen ability stay observable.
        SPECIES_PORYGON_Z, 2,
        ABILITY_SIMPLE
    },
    { // 0475
        // Gallade: all its real abilities are now innate, so its innate-redundant slot-0 Steadfast takes a
        // chosen Simple -- :x: (never an innate -> stable): every stat change it takes is doubled.
        SPECIES_GALLADE, 0,
        ABILITY_SIMPLE
    },
    { // 0476
        // Probopass's three real abilities (Sturdy, Magnet Pull, Sand Force) are ALL now innate, so its innate-redundant
        // slot-1 Magnet Pull (audited: no Ability(ABILITY_MAGNET_PULL) on Probopass) takes Lightning Rod — :x: (never an
        // innate -> stable) and thematic for the compass magnet: an Electric immunity + Sp. Atk boost for its special wall.
        SPECIES_PROBOPASS, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0477
        // Dusknoir: all real abilities now innate, so its empty slot takes a chosen
        // Mummy so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DUSKNOIR, 1,
        ABILITY_MUMMY
    },
    { // 0478
        // Froslass's only real abilities (Snow Cloak, Cursed Body) are BOTH now innate, so its EMPTY slot 1 takes
        // Snow Warning -- :x: (never an innate -> stable) and self-synergistic for the Snow Land Pokemon: the snow
        // it heralds turns on its own innate Snow Cloak evasion, a clean boon for its fast Spikes / Destiny Bond lead
        // on top of the innate Cursed Body. Same pick as the other ice mons (Articuno / Beartic / Cryogonal / Frosmoth).
        SPECIES_FROSLASS, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0479
        SPECIES_ROTOM, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_HEAT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_WASH, 1, 
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_FROST, 1, 
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_FAN, 1, 
        ABILITY_MOTOR_DRIVE
    },
    { // 0479
        SPECIES_ROTOM_MOW, 1, 
        ABILITY_MOTOR_DRIVE
    },
    { // 0480
        SPECIES_UXIE, 1,
        ABILITY_TRACE
    },
    { // 0481
        SPECIES_MESPRIT, 1,
        ABILITY_MOODY
    },
    { // 0482
        SPECIES_AZELF, 1,
        ABILITY_VICTORY_STAR
    },
    { // 0483
        // Dialga's only real abilities (Pressure, Telepathy) are BOTH now innate (Telepathy), so its
        // EMPTY slot 1 takes Bulletproof -- :x: (never an innate -> stable) and thematic for the armored Steel
        // legend: it deflects the Focus Blast / Sludge Bomb / Aura Sphere ball-and-bomb moves aimed at its
        // Fighting weakness, a clean defensive boon on its bulky Leftovers sets. Same pick as the Steel walls
        // Skarmory / Registeel / Corviknight.
        SPECIES_DIALGA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0483
        // Dialga-Origin shares base Dialga's now-all-innate real slots (Pressure, Telepathy), so its EMPTY slot 1
        // takes the same Bulletproof pick for the armored Steel legend.
        SPECIES_DIALGA_ORIGIN, 1,
        ABILITY_BULLETPROOF
    },
    { // 0484
        // Palkia's only real abilities (Pressure, Telepathy) are BOTH now innate (Telepathy), so its
        // EMPTY slot 1 takes Water Absorb -- :x: (never an innate -> stable) and thematic for the Water legend
        // of space: it shrugs off Water moves and heals from them, a clean switch-in boon for its fast special
        // sets. Same pick as the other Water legends / walls (Suicune / Clawitzer / Samurott).
        SPECIES_PALKIA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0484
        // Palkia-Origin shares base Palkia's now-all-innate real slots (Pressure, Telepathy), so its EMPTY slot 1
        // takes the same Water Absorb pick for the Water legend of space.
        SPECIES_PALKIA_ORIGIN, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0487
        // Giratina: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Wandering Spirit
        // -- :x: (never an innate -> stable): contact attackers have their ability swapped with its own.
        SPECIES_GIRATINA_ALTERED, 1,
        ABILITY_WANDERING_SPIRIT
    },
    { // 0487
        // Giratina-Origin: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer
        // Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_GIRATINA_ORIGIN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0488
        SPECIES_CRESSELIA, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0489
        // Phione's only real ability (Hydration) is now innate, so its EMPTY slot 1 takes Water Absorb -- :x:
        // (never an innate -> stable) and thematic for the sea-drifting mythical: it heals on the Water it swims
        // through. (Its slot-0 Hydration is pinned by ow_abilities.c, so the fill goes in slot 1.)
        SPECIES_PHIONE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0490
        // Manaphy's only real ability (Hydration) is now innate, so its EMPTY slot 1 takes Water Absorb -- :x:
        // (never an innate -> stable), the same sea-mythical pick as its offspring Phione.
        SPECIES_MANAPHY, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0491
        // Darkrai's only real ability (Bad Dreams) is now innate, so its EMPTY slot 1 takes Sheer Force -- :x:
        // (never an innate -> stable) and a clean boon for the Pitch-Black nightmare: its Dark Pulse / Sludge Bomb /
        // Focus Blast drop their added effect for +30% power, observable alongside the innate Bad Dreams.
        SPECIES_DARKRAI, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0492
        SPECIES_SHAYMIN, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0492
        SPECIES_SHAYMIN_SKY, 1,
        ABILITY_WIND_RIDER
    },
    { // 0500
        // Emboar: all real abilities now innate, so its empty slot takes a chosen
        // Flash Fire so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_EMBOAR, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0503
        SPECIES_SAMUROTT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0503
        // Samurott Hisui: all real abilities now innate, so its empty slot takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_SAMUROTT_HISUI, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0505
        // Watchog: all its real abilities are now innate, so its innate-redundant slot-1 Keen Eye takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power.
        SPECIES_WATCHOG, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0508
        // Stoutland's three real abilities (Intimidate, Sand Rush, Scrappy) are ALL now innate; slot-1 Sand Rush is
        // test-pinned (sand_rush.c), so its innate-redundant slot-2 Scrappy (audited: no Ability(ABILITY_SCRAPPY) on
        // Stoutland in test/) takes a chosen Sheer Force -- :x: (never an innate -> stable) and a boon for the loyal
        // gundog: its Crunch / Body Slam gain +30% and drop their secondaries, observable alongside the innate
        // Intimidate.
        SPECIES_STOUTLAND, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0510
        // Liepard: all its real abilities are now innate, so its innate-redundant slot-1 Unburden takes a
        // chosen Illusion -- :x: (never an innate -> stable): it enters disguised as the party's last member
        // until it takes a direct hit. (Slot audited: no flag-on fork test pins it.)
        SPECIES_LIEPARD, 1,
        ABILITY_ILLUSION
    },
    { // 0512
        // Simisage: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Grassy Surge --
        // :x: (never an innate -> stable): it sets Grassy Terrain on entry, healing grounded allies and
        // boosting Grass moves 1.3x.
        SPECIES_SIMISAGE, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0514
        // Simisear's only real abilities (Gluttony, Blaze) are BOTH now innate, so its EMPTY slot 1 takes
        // Flash Fire -- :x: (never an innate -> stable) and thematic for the fire monkey: it shrugs off Fire
        // moves for an immunity + a Fire-power boost on its Nasty Plot sets.
        SPECIES_SIMISEAR, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0516
        // Simipour's only real abilities (Gluttony, Torrent) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the water monkey: it heals on the
        // Water hits its bulky switch-ins invite.
        SPECIES_SIMIPOUR, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0530
        // Excadrill: all real abilities (Sand Rush, Sand Force, Mold Breaker) now innate; slot-0 Sand Rush and
        // slot-1 Sand Force are Ability()-pinned in the fork innate test, so its innate-redundant slot-2 Mold
        // Breaker takes Sand Stream -- :x: (never an innate -> stable) and self-synergistic: the sandstorm it sets
        // turns on its own innate Sand Rush (Speed) and Sand Force (power). Same pick as Sandslash / Dugtrio.
        SPECIES_EXCADRILL, 2,
        ABILITY_SAND_STREAM
    },
    { // 0531
        // Audino: Healer, Regenerator are now innate, so its innate-redundant slot-1 Regenerator takes a chosen
        // Synchronize -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is passed
        // straight back.
        SPECIES_AUDINO, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0538
        // Throh's three real abilities (Guts, Inner Focus, Mold Breaker) are ALL now innate, and none is test-pinned
        // (audited: no test selects Throh), so its innate-redundant slot-1 Inner Focus takes Simple -- :x: (never an
        // innate -> stable) and a perfect fit for its Bulk Up tank set: each Bulk Up counts double (+2 Atk / +2 Def),
        // snowballing the Judo Pokemon's Drain Punch / Body Press sweep, on top of the innate Guts. (Its karate
        // counterpart Sawk takes Sheer Force just below.)
        SPECIES_THROH, 1,
        ABILITY_SIMPLE
    },
    { // 0539
        // Sawk's three real abilities (Sturdy, Inner Focus, Mold Breaker) are ALL now innate (Mold Breaker,
        // Tier 5.5), so its innate-redundant slot-1 Inner Focus (audited: no test selects Sawk) takes Sheer
        // Force -- :x: (never an innate -> stable) and a clean boon for the karate breaker: Ice Punch / Poison
        // Jab gain +30% and drop their secondaries, stacking with the innate Mold Breaker ability-ignore.
        SPECIES_SAWK, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0542
        // Leavanny: all its real abilities are now innate, so its innate-redundant slot-2 Overcoat takes a
        // chosen Hustle -- :x: (never an innate -> stable): its physical attacks gain 1.5x power for an
        // accuracy cost. (Slot audited: no flag-on fork test pins it.)
        SPECIES_LEAVANNY, 2,
        ABILITY_HUSTLE
    },
    { // 0547
        // Whimsicott: all its real abilities are now innate, so its innate-redundant slot-2 Chlorophyll takes a
        // chosen Cotton Down -- :x: (never an innate -> stable): anything that hits it eats a -1 Speed drop.
        // (Slot audited: no flag-on fork test pins it.)
        SPECIES_WHIMSICOTT, 2,
        ABILITY_COTTON_DOWN
    },
    { // 0549
        // Lilligant's three real abilities (Chlorophyll, Own Tempo, Leaf Guard) are ALL now innate; slot-0 Chlorophyll
        // is the default read and slot-1 Own Tempo is the chosen-differs-from-innate exemplar in
        // test/fork/innate_abilities.c, so its innate-redundant slot-2 Leaf Guard (audited: no Ability(ABILITY_LEAF_GUARD)
        // on Lilligant in test/) takes Grassy Surge -- :x: (never an innate -> stable) and thematic for the Flowering
        // Pokemon: the terrain it sets powers its Grass STAB (Giga Drain / Petal Dance) and passively heals, a clean
        // boon for its Quiver Dance sweeper alongside the innate Chlorophyll (Speed in sun) and Dancer. Same pick as
        // the other Grass mons (Venusaur / Meganium / Celebi).
        SPECIES_LILLIGANT, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0553
        // Krookodile's three real abilities (Intimidate, Moxie, Anger Point) are ALL now innate; slot-0 Intimidate is
        // test-pinned (frontier_battle_info_reveal.c), so its innate-redundant slot-2 Anger Point (audited: no
        // Ability(ABILITY_ANGER_POINT) on Krookodile in test/) takes a chosen Sand Stream -- :x: (never an innate ->
        // stable) and thematic for the desert croc: it kicks up a sandstorm on entry (Ground-immune to the chip, a
        // clean team boon), observable alongside the innate Intimidate.
        SPECIES_KROOKODILE, 2,
        ABILITY_SAND_STREAM
    },
    { // 0560
        // Scrafty's three real abilities (Shed Skin, Moxie, Intimidate) are ALL now innate; slot-2 Intimidate is
        // test-pinned (primal_reversion.c) and slot-0 Shed Skin is the default read by ai_switching.c, so its
        // innate-redundant slot-1 Moxie (audited: no Ability(ABILITY_MOXIE) on Scrafty in test/) takes a chosen
        // Rivalry -- :x: (never an innate -> stable) and thematic for the gang-forming thug lizard: it hits
        // same-gender rivals 25% harder, observable on its Bulk Up sweeper alongside the innate Intimidate.
        SPECIES_SCRAFTY, 1,
        ABILITY_RIVALRY
    },
    { // 0561
        // Sigilyph's three real abilities (Wonder Skin, Magic Guard, Tinted Lens) are ALL now innate (Tier 5.4).
        // Slot-1 Magic Guard is pinned by the innate test, so its innate-redundant slot-2 Tinted Lens (audited: no
        // Ability(ABILITY_TINTED_LENS) on Sigilyph in test/) takes Simple -- :x: (never an innate -> stable) and
        // thematic for the Psychic bird's Cosmic Power stallbreaker: each Cosmic Power now gives +2 Def / +2 Sp. Def,
        // supercharging its Stored Power (the innate Magic Guard still voids the Life Orb recoil). Slots 0/1
        // (Wonder Skin / Magic Guard) stay intact -- the innate test pins Magic Guard and reads Wonder Skin.
        SPECIES_SIGILYPH, 2,
        ABILITY_SIMPLE
    },
    { // 0565
        // Carracosta's only real abilities (Solid Rock, Sturdy, Swift Swim) are ALL now innate, so its
        // innate-redundant slot-2 Swift Swim takes a chosen Water Absorb — :x: (never an innate -> stable)
        // and flavorful: the prehistoric shell turtle heals on the Water hits a Shell Smash sweeper invites.
        SPECIES_CARRACOSTA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0569
        // Garbodor's slot-0 Stench and slot-2 Aftermath are now innate; its only non-innate real slot is slot-1
        // Weak Armor, a drawback on a Rocky Helmet hazard wall. So its innate-redundant slot-2 Aftermath --
        // unpinned by any test (audited: aftermath.c uses Voltorb) -- takes Poison Touch instead. Poison Touch
        // is :x: (never an innate -> stable) and a clean boon for the trash heap: its Gunk Shot can poison.
        SPECIES_GARBODOR, 2,
        ABILITY_POISON_TOUCH
    },
    { // 0573
        // Cinccino: all its real abilities are now innate, so its innate-redundant slot-1 Technician takes a
        // chosen Hustle -- :x: (never an innate -> stable): its physical attacks gain 1.5x power for an
        // accuracy cost. (Slot audited: no flag-on fork test pins it.)
        SPECIES_CINCCINO, 1,
        ABILITY_HUSTLE
    },
    { // 0576
        // Gothitelle: all its real abilities are now innate, so its innate-redundant slot-2 Shadow Tag takes a
        // chosen Synchronize -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is
        // passed straight back. (Slot audited: no flag-on fork test pins it.)
        SPECIES_GOTHITELLE, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0579
        // Reuniclus's three real abilities (Overcoat, Magic Guard, Regenerator) are ALL now innate (Tier 5.4) and
        // NONE is test-pinned, so its innate-redundant slot-2 Regenerator (audited: no Ability() on Reuniclus in
        // test/) takes No Guard -- :x: (never an innate -> stable) and thematic for the pure-intellect Psychic: its
        // Focus Blast (run on both frontier sets) never misses. Slots 0/1 (Overcoat / Magic Guard) stay intact so
        // Magic Guard remains a visible chosen option, and the innate Magic Guard still voids Life Orb recoil.
        SPECIES_REUNICLUS, 2,
        ABILITY_NO_GUARD
    },
    { // 0581
        // Swanna's three real abilities (Keen Eye, Big Pecks, Hydration) are ALL now innate, and it is used by no
        // test, so its innate-redundant slot-2 Hydration takes Storm Drain -- :x: (never an innate -> stable) and
        // thematic for the graceful Water/Flying bird: it draws in Water moves for a Sp. Atk boost, observable
        // alongside the innate Keen Eye / Big Pecks.
        SPECIES_SWANNA, 2,
        ABILITY_STORM_DRAIN
    },
    { // 0589
        // Escavalier's three real abilities (Swarm, Shell Armor, Overcoat) are ALL now innate, and none is
        // test-pinned (audited: no test selects Escavalier), so its innate-redundant slot-2 Overcoat takes Sheer
        // Force -- :x: (never an innate -> stable) and a clean boon for the Cavalry Pokemon's Swords Dance sets: Iron
        // Head gains +30% and drops its flinch, stacking with the innate Shell Armor (no crits taken).
        SPECIES_ESCAVALIER, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0591
        // Amoonguss's real abilities are Effect Spore and Regenerator (now innate). Effect Spore is a poor chosen
        // slot: under DETERMINISTIC_ABILITIES it always sleeps contact attackers, which pre-empts the Singles set's
        // own Toxic (single status slot) and is redundant with the Doubles set's Spore. Its EMPTY slot 1 takes
        // Water Absorb instead -- :x: (never an innate -> stable) and a defensive boon for the fungus pivot.
        SPECIES_AMOONGUSS, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0594
        SPECIES_ALOMOMOLA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0596
        // Galvantula: all real abilities now innate, so its innate-redundant slot-1 Unnerve takes a chosen
        // Static so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_GALVANTULA, 1,
        ABILITY_STATIC
    },
    { // 0598
        // Ferrothorn: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Well-Baked Body
        // -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for +2 Defense.
        SPECIES_FERROTHORN, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0601
        // Klinklang's slot-2 Clear Body is now innate and its slots 0/1 are Plus/Minus (dead in singles); slot-0 Plus
        // is pinned by upstream doubles/anim tests, so its unpinned slot-1 Minus (audited) is repurposed to a chosen
        // Motor Drive, :x: (never an innate -> stable) and thematic for the gear Pokemon: an Electric immunity + Speed
        // boost that snowballs its Shift Gear sweeper.
        SPECIES_KLINKLANG, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0604
        SPECIES_EELEKTROSS, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0614
        // Beartic's three real abilities (Snow Cloak, Slush Rush, Swift Swim) are ALL now innate, so
        // its slot-1 Slush Rush — now innate-redundant — is repurposed to Snow Warning, :x: (never an
        // innate -> stable) and self-synergistic: the snow it sets turns on its own innate Slush Rush
        // (Speed) and Snow Cloak (evasion).
        SPECIES_BEARTIC, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0615
        SPECIES_CRYOGONAL, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0617
        // Accelgor: all its real abilities are now innate, so its innate-redundant slot-1 Sticky Hold takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_ACCELGOR, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0620
        // Mienshao's three real abilities (Inner Focus, Regenerator, Reckless) are ALL now innate; slot-0 Inner Focus
        // is the default read (upper_hand.c), so its innate-redundant slot-2 Reckless (audited: no Ability() on
        // Mienshao in test/) takes No Guard -- :x: (never an innate -> stable) and thematic for the martial artist:
        // its High Jump Kick never misses (and never crashes), a clean boon for the fast attacker on top of the
        // innate Reckless (recoil/crash power) and Regenerator pivot. Same pick as the other fighters (Hitmonlee).
        SPECIES_MIENSHAO, 2,
        ABILITY_NO_GUARD
    },
    { // 0630
        // Mandibuzz: Big Pecks, Overcoat are now innate, so its slot-2 Weak Armor takes a chosen Wind Rider --
        // :x: (never an innate -> stable): wind moves and Tailwind give it +1 Attack instead of landing.
        SPECIES_MANDIBUZZ, 2,
        ABILITY_WIND_RIDER
    },
    { // 0635
        SPECIES_HYDREIGON, 1, 
        ABILITY_SHEER_FORCE },
    { // 0638
        // Cobalion: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Bulletproof -- :x: (never an innate -> stable):
        // ball, bomb and cannon moves (Focus Blast, Shadow Ball, Sludge Bomb) cannot touch it.
        SPECIES_COBALION, 1,
        ABILITY_BULLETPROOF
    },
    { // 0639
        // Terrakion: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Sand Stream -- :x: (never an innate -> stable):
        // it sets sandstorm on entry, chipping the field and boosting Rock-type Sp. Def.
        SPECIES_TERRAKION, 1,
        ABILITY_SAND_STREAM
    },
    { // 0640
        // Virizion: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Sap Sipper -- :x: (never an innate -> stable):
        // Grass moves are absorbed for +1 Attack instead of landing.
        SPECIES_VIRIZION, 1,
        ABILITY_SAP_SIPPER
    },
    { // 0641
        // Tornadus-Therian: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Wind
        // Rider -- :x: (never an innate -> stable): wind moves and Tailwind give it +1 Attack instead of
        // landing.
        SPECIES_TORNADUS_THERIAN, 1,
        ABILITY_WIND_RIDER
    },
    { // 0641
        // Tornadus: all real abilities now innate, so its empty slot takes a chosen
        // Cloud Nine so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_TORNADUS, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0642
        // Thundurus: all real abilities now innate, so its empty slot takes a chosen
        // Volt Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_THUNDURUS, 1,
        ABILITY_VOLT_ABSORB
    },
    { // 0643
        // Reshiram's only real ability (Turboblaze) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) / Necrozma (Y3) / Solgaleo (Y4) it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Flash Fire is :x: (never an innate -> stable) and thematic for the Vast White dragon: it
        // shrugs off Fire moves for an immunity + a Fire-power boost on its Blue Flare special sets, stacking with the
        // innate Turboblaze ability-ignore. (Kyurem-White, its fusion, takes the same pick.)
        SPECIES_RESHIRAM, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0644
        // Zekrom's only real ability (Teravolt) is now innate, and it's a frontier set, so it takes the
        // innate AND a fork-owned chosen ability in its EMPTY slot 1. Motor Drive is :x: (never an innate -> stable)
        // and thematic for the Deep Black dragon: it draws in Electric moves for an immunity + a Speed boost that
        // snowballs its Dragon Dance sweeper, on top of the innate Teravolt ability-ignore. Same pick as Klinklang /
        // Vikavolt. (Kyurem-Black, its fusion, takes the same pick.)
        SPECIES_ZEKROM, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0645
        // Landorus-Therian's only real ability (Intimidate) is now innate, and it's a frontier set, so like
        // Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability in its empty slot 1. Sheer
        // Force is :x: (never an innate -> stable) and flavorful: it's the signature of Landorus's Incarnate
        // forme, so its frontier sets run Sheer Force offense on top of the innate Intimidate switch-in drop.
        SPECIES_LANDORUS_THERIAN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0646
        SPECIES_KYUREM, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0646
        // Kyurem-White's only real ability (Turboblaze) is now innate, and it's a frontier set, so it takes
        // the innate AND a fork-owned chosen ability in its EMPTY slot 1. Flash Fire is :x: (never an innate -> stable)
        // and thematic: fused with Reshiram, the Boundary dragon wields fire (Fusion Flare), so it shrugs off Fire for
        // an immunity + a Fire-power boost on its special sets. Same pick as Reshiram itself.
        SPECIES_KYUREM_WHITE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0646
        // Kyurem-Black's only real ability (Teravolt) is now innate, and it's a frontier set, so it takes
        // the innate AND a fork-owned chosen ability in its EMPTY slot 1. Motor Drive is :x: (never an innate ->
        // stable) and thematic: fused with Zekrom, the Boundary dragon wields lightning (Fusion Bolt), so it draws in
        // Electric moves for an immunity + a Speed boost that snowballs its Dragon Dance sweeper. Same pick as Zekrom.
        SPECIES_KYUREM_BLACK, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0647
        // Keldeo: every real ability it has is innate-capable, so its frontier sets had no legal chosen ability
        // left to name. Its EMPTY slot 1 takes a chosen Storm Drain -- :x: (never an innate -> stable): Water
        // moves are drawn in and absorbed for +1 Sp. Atk.
        SPECIES_KELDEO, 1,
        ABILITY_STORM_DRAIN
    },
    { // 0648
        // Meloetta: its only real ability (Serene Grace) is innate-capable, so its EMPTY slot 1 takes a chosen
        // Pixilate -- :x: (never an innate -> stable): its Normal-type moves turn Fairy and gain 1.2x power.
        SPECIES_MELOETTA, 1,
        ABILITY_PIXILATE
    },
    { // 0649
        // Genesect's only real ability (Download) is now innate, so its EMPTY slot 1 takes Sheer Force.
        // Sheer Force is :x: (never an innate -> stable) and thematic for the paleo-cyborg cannon: raw
        // mechanical force boosts its secondary-effect coverage (Ice Beam / Flamethrower / Thunderbolt /
        // Flash Cannon) by 30%, keeping the chosen ability observable alongside the innate Download.
        SPECIES_GENESECT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0655
        // Delphox's only real abilities (Blaze, Magician) are BOTH now innate; slot-0 Blaze and slot-2 Magician are
        // both pinned by tests (the innate test / magician.c / air_balloon.c), so its EMPTY slot 1 takes Flash Fire.
        // Flash Fire is :x: (never an innate -> stable) and thematic for the mystic fox: it shrugs off Fire moves for
        // an immunity + a Fire-power boost on its Choice Specs special breaker, on top of the innate Magician steal.
        // Same pick as the other Fire attackers (Simisear / Reshiram / Sinistcha).
        SPECIES_DELPHOX, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0660
        // Diggersby: all its real abilities are now innate, so its innate-redundant slot-2 Huge Power takes a
        // chosen Earth Eater -- :x: (never an innate -> stable): Ground-type moves heal it for 1/4 max HP
        // instead of damaging it. (Slot audited: no flag-on fork test pins it.)
        SPECIES_DIGGERSBY, 2,
        ABILITY_EARTH_EATER
    },
    { // 0675
        // Pangoro: all its real abilities are now innate, so its innate-redundant slot-0 Iron Fist takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_PANGORO, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0678
        // Meowstic-M: all its real abilities are now innate, so its innate-redundant slot-1 Infiltrator takes a
        // chosen Synchronize -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is
        // passed straight back. (Slot audited: no flag-on fork test pins it.)
        SPECIES_MEOWSTIC_M, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0678
        // Meowstic-F: all its real abilities are now innate, so its innate-redundant slot-0 Keen Eye takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power.
        SPECIES_MEOWSTIC_F, 0,
        ABILITY_SHEER_FORCE
    },
    { // 0683
        // Aromatisse's only real abilities (Healer, Aroma Veil) are BOTH now innate (Aroma Veil), so its
        // EMPTY slot 1 takes Misty Surge -- :x: (never an innate -> stable) and thematic for the Fragrance
        // Pokemon: on switch-in it blankets the field in Misty Terrain, protecting its team from status and
        // softening Dragon moves, a clean support boon for its Trick Room / Aromatherapy / Wish cleric set.
        // (Slot-2 Aroma Veil stays intact -- aroma_veil.c selects it -- so the innate Aroma Veil is still an
        // observable chosen option.)
        SPECIES_AROMATISSE, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0685
        // Slurpuff: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Well-Baked Body
        // -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for +2 Defense.
        SPECIES_SLURPUFF, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0689
        // Barbaracle: all real abilities now innate, so its innate-redundant slot-1 Sniper takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_BARBARACLE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0693
        // Clawitzer's only real ability (Mega Launcher) is now innate, so its empty slot 1 takes a
        // flavorful chosen ability. Water Absorb is :x: (never an innate -> stable) and on-theme for the
        // howitzer shrimp: it shrugs off Water moves and heals from them, a clean boon alongside its
        // innate Mega Launcher pulse boost.
        SPECIES_CLAWITZER, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0697
        // Tyrantrum: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_TYRANTRUM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0701
        // Hawlucha: all its real abilities are now innate, so its innate-redundant slot-1 Unburden takes a
        // chosen Hustle -- :x: (never an innate -> stable): its physical attacks gain 1.5x power for an
        // accuracy cost. (Slot audited: no flag-on fork test pins it.)
        SPECIES_HAWLUCHA, 1,
        ABILITY_HUSTLE
    },
    { // 0703
        // Carbink: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Bulletproof -- :x:
        // (never an innate -> stable): ball, bomb and cannon moves (Focus Blast, Shadow Ball, Sludge Bomb)
        // cannot touch it.
        SPECIES_CARBINK, 1,
        ABILITY_BULLETPROOF
    },
    { // 0707
        // Klefki's only real abilities (Prankster, Magician) are BOTH now innate, and NONE is test-pinned (audited:
        // no reference to Klefki in test/), so its EMPTY slot 1 takes Bulletproof. Bulletproof is :x: (never an
        // innate -> stable) and thematic for the Steel/Fairy Key Ring: it deflects the Focus Blast / Sludge Bomb /
        // Energy Ball ball-and-bomb moves aimed at its Prankster dual-screens + Spikes support core, on top of the
        // innate Prankster (priority status) and Magician steal. Same pick as the other Steel walls (Skarmory / Dialga).
        SPECIES_KLEFKI, 1,
        ABILITY_BULLETPROOF
    },
    { // 0709
        // Trevenant's three real abilities (Natural Cure, Frisk, Harvest) are ALL now innate; slot-0 Natural Cure is
        // the AI-test default read (powder.c / revelation_dance.c), so its unpinned innate-redundant slot-2 Harvest
        // (audited) takes Sap Sipper -- :x: (never an innate -> stable) and thematic for the Elder Tree: the old tree
        // drinks in Grass moves for an Attack boost, observable alongside the innate Harvest / Frisk.
        SPECIES_TREVENANT, 2,
        ABILITY_SAP_SIPPER
    },
    { // 0711
        // Gourgeist-Super: all its real abilities are now innate, so its innate-redundant slot-0 Pickup takes a
        // chosen Well-Baked Body -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for
        // +2 Defense.
        SPECIES_GOURGEIST_SUPER, 0,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0713
        // Avalugg: all its real abilities are now innate, so its innate-redundant slot-2 Sturdy takes a chosen
        // Water Absorb -- :x: (never an innate -> stable): Water moves heal it for 1/4 max HP instead of
        // damaging it. (Slot audited: no flag-on fork test pins it.)
        SPECIES_AVALUGG, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0713
        // Avalugg-Hisui: all its real abilities are now innate, so its innate-redundant slot-1 Ice Body takes a
        // chosen Water Absorb -- :x: (never an innate -> stable): Water moves heal it for 1/4 max HP instead of
        // damaging it.
        SPECIES_AVALUGG_HISUI, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0715
        // Noivern: all its real abilities are now innate, so its innate-redundant slot-2 Telepathy takes a
        // chosen Soundproof -- :x: (never an innate -> stable): sound moves (Boomburst, Hyper Voice, Snarl)
        // cannot touch it. (Slot audited: no flag-on fork test pins it.)
        SPECIES_NOIVERN, 2,
        ABILITY_SOUNDPROOF
    },
    { // 0719
        // Diancie: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Misty Surge -- :x:
        // (never an innate -> stable): it sets Misty Terrain on entry, blocking status on grounded mons and
        // halving Dragon moves.
        SPECIES_DIANCIE, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0720
        // Hoopa: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force -- :x:
        // (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_HOOPA, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0720
        // Hoopa-Unbound: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force
        // -- :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_HOOPA_UNBOUND, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0724
        // Decidueye-Hisui: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer
        // Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_DECIDUEYE_HISUI, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0724
        // Decidueye: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Soundproof --
        // :x: (never an innate -> stable): sound moves (Boomburst, Hyper Voice, Snarl) cannot touch it.
        SPECIES_DECIDUEYE, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0727
        // Incineroar: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Flame Body --
        // :x: (never an innate -> stable): contact attackers risk a burn.
        SPECIES_INCINEROAR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0735
        // Gumshoos: all its real abilities are now innate, so its innate-redundant slot-1 Strong Jaw takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power.
        SPECIES_GUMSHOOS, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0738
        SPECIES_VIKAVOLT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0740
        // Crabominable: all real abilities now innate, so its innate-redundant slot-1 Iron Fist takes a chosen
        // No Guard so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_CRABOMINABLE, 1,
        ABILITY_NO_GUARD
    },
    { // 0743
        // Ribombee: Shield Dust, Sweet Veil are now innate, so its slot-0 Honey Gather takes a chosen Effect
        // Spore -- :x: (never an innate -> stable): contact attackers risk poison, paralysis or sleep.
        SPECIES_RIBOMBEE, 0,
        ABILITY_EFFECT_SPORE
    },
    { // 0745
        // Lycanroc-Dusk: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sand Stream
        // -- :x: (never an innate -> stable): it sets sandstorm on entry, chipping the field and boosting Rock-
        // type Sp. Def.
        SPECIES_LYCANROC_DUSK, 1,
        ABILITY_SAND_STREAM
    },
    { // 0745
        // Lycanroc: all real abilities now innate, so its innate-redundant slot-1 Sand Rush takes a chosen
        // Sand Stream so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_LYCANROC, 1,
        ABILITY_SAND_STREAM
    },
    { // 0748
        // Toxapex: all real abilities now innate, so its innate-redundant slot-2 Regenerator takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_TOXAPEX, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0750
        // Mudsdale: all real abilities now innate, so its innate-redundant slot-1 Stamina takes a chosen
        // Earth Eater so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_MUDSDALE, 1,
        ABILITY_EARTH_EATER
    },
    { // 0758
        // Corrosion and Oblivious now innate, so its EMPTY slot 1 takes Flame Body -- :x: (never an innate ->
        // stable) and thematic for the toxic lizard: contact attackers risk a burn, alongside its
        // Toxic-spreading innate Corrosion.
        SPECIES_SALAZZLE, 1,
        ABILITY_FLAME_BODY
    },
    { // 0763
        // Tsareena's three real abilities (Leaf Guard, Queenly Majesty, Sweet Veil) are ALL now innate; slot-0 Leaf
        // Guard is the chosen-differs-from-innate exemplar (test/fork/innate_abilities.c) and slot-1 Queenly Majesty
        // is test-pinned (dazzling.c / ai.c), so its innate-redundant slot-2 Sweet Veil takes Grassy Surge -- :x:
        // (never an innate -> stable) and thematic for the Fruit Pokemon: the terrain it sets powers its Grass STAB
        // (Power Whip / Trop Kick) and passively heals, on top of the innate Queenly Majesty (priority block). Same
        // pick as the other Grass mons (Lilligant / Venusaur). Non-immunity pick avoids perturbing the ai.c /
        // dazzling.c priority-block tests that read Tsareena's ability set.
        SPECIES_TSAREENA, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0764
        // Comfey: all its real abilities are now innate, so its innate-redundant slot-2 Natural Cure takes a
        // chosen Grassy Surge -- :x: (never an innate -> stable): it sets Grassy Terrain on entry, healing
        // grounded allies and boosting Grass moves 1.3x. (Slot audited: no flag-on fork test pins it.)
        SPECIES_COMFEY, 2,
        ABILITY_GRASSY_SURGE
    },
    { // 0766
        // Passimian: all real abilities now innate, so its empty slot takes a chosen
        // Rivalry so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_PASSIMIAN, 1,
        ABILITY_RIVALRY
    },
    { // 0770
        // Palossand: all real abilities now innate, so its empty slot takes a chosen
        // Earth Eater so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_PALOSSAND, 1,
        ABILITY_EARTH_EATER
    },
    { // 0771
        // Pyukumuku's real abilities are Innards Out (slot 0, now innate AND pinned by innards_out.c) and Unaware
        // (slot 2, now innate), so its EMPTY slot 1 takes Water Absorb -- :x: (never an innate -> stable) and
        // thematic for the sea cucumber: it heals off the Water hits its Counter / Recover staller invites,
        // stacking with its innate Unaware. (Slot 0 Innards Out stays a real ability -- test-pinned.)
        SPECIES_PYUKUMUKU, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0775
        // Komala: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Hustle -- :x:
        // (never an innate -> stable): its physical attacks gain 1.5x power for an accuracy cost.
        SPECIES_KOMALA, 1,
        ABILITY_HUSTLE
    },
    { // 0776
        SPECIES_TURTONATOR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0741
        // Oricorio-Baile: its only real ability (Dancer) is innate-capable, so its EMPTY slot 1 takes a chosen
        // Flash Fire -- :x: (never an innate -> stable): Fire moves are absorbed for a 1.5x Fire-power boost
        // instead of landing.
        SPECIES_ORICORIO, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0741
        // Oricorio-Pa'u: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Synchronize
        // -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is passed straight back.
        SPECIES_ORICORIO_PAU, 1,
        ABILITY_SYNCHRONIZE
    },
    { // 0756
        // Shiinotic: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its innate-redundant slot-2 Rain Dish takes a chosen Mycelium Might -- :x:
        // (never an innate -> stable): its status moves ignore the target's ability, at the cost of always
        // moving last.
        SPECIES_SHIINOTIC, 2,
        ABILITY_MYCELIUM_MIGHT
    },
    { // 0779
        // Bruxish's three real abilities (Dazzling, Strong Jaw, Wonder Skin) are ALL now innate, so its
        // innate-redundant slot-1 Strong Jaw -- unpinned by any test (audited: only slot-0 Dazzling is
        // pinned, by dazzling.c / bide.c / last_resort.c) -- takes a chosen Sheer Force. Sheer Force is :x:
        // (never an innate -> stable) and a strong boon for the Gnash Teeth Pokemon's biting attacker sets:
        // Psychic Fangs / Crunch / Liquidation / Ice Fang all gain +30% and drop their secondaries (and a
        // Life Orb set skips its recoil), stacking with the innate Strong Jaw fang boost and Dazzling block.
        SPECIES_BRUXISH, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0781
        // Dhelmise: all real abilities now innate, so its empty slot takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DHELMISE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0791
        // Solgaleo: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Drought -- :x:
        // (never an innate -> stable): it sets harsh sunlight on entry.
        SPECIES_SOLGALEO, 1,
        ABILITY_DROUGHT
    },
    { // 0792
        // Lunala: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Psychic Surge --
        // :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting Psychic moves 1.3x and
        // blocking priority on grounded mons.
        SPECIES_LUNALA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        // Necrozma: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Psychic Surge --
        // :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting Psychic moves 1.3x and
        // blocking priority on grounded mons.
        SPECIES_NECROZMA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        // Necrozma-Dusk Mane: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Psychic
        // Surge -- :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting Psychic moves
        // 1.3x and blocking priority on grounded mons.
        SPECIES_NECROZMA_DUSK_MANE, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0800
        // Necrozma-Dawn Wings: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen
        // Psychic Surge -- :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting Psychic
        // moves 1.3x and blocking priority on grounded mons.
        SPECIES_NECROZMA_DAWN_WINGS, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0802
        // Marshadow's only real ability (Technician) is now innate, so its empty slot 1 takes a flavorful
        // chosen ability for the frontier sets. Illusion is :x: (never an innate -> stable) and on-theme:
        // the Gloomdweller lurks in shadows and mimics, so it enters disguised as the party's last mon.
        SPECIES_MARSHADOW, 1,
        ABILITY_ILLUSION
    },
    { // 0793
        // Nihilego: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Toxic Debris --
        // :x: (never an innate -> stable): a physical hit scatters Toxic Spikes on the attacker's side.
        SPECIES_NIHILEGO, 1,
        ABILITY_TOXIC_DEBRIS
    },
    { // 0794
        // Buzzwole: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Poison Touch --
        // :x: (never an innate -> stable): its contact moves carry a poison chance.
        SPECIES_BUZZWOLE, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0795
        // Pheromosa: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Lingering Aroma
        // -- :x: (never an innate -> stable): contact attackers have their own ability replaced by Lingering
        // Aroma.
        SPECIES_PHEROMOSA, 1,
        ABILITY_LINGERING_AROMA
    },
    { // 0796
        // Xurkitree's only real ability (Beast Boost) is now innate, and it's a frontier set, so its
        // EMPTY slot 1 takes Lightning Rod -- :x: (never an innate -> stable) and thematic for the living power
        // line (the Raichu-Alola / Regieleki precedent): it draws in Electric moves for immunity + a Sp. Atk boost
        // for its Tail Glow special sets, on top of the innate Beast Boost snowball (and innate Levitate).
        SPECIES_XURKITREE, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0797
        // Celesteela: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Well-Baked Body
        // -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for +2 Defense.
        SPECIES_CELESTEELA, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0798
        // Kartana: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Bulletproof -- :x:
        // (never an innate -> stable): ball, bomb and cannon moves (Focus Blast, Shadow Ball, Sludge Bomb)
        // cannot touch it.
        SPECIES_KARTANA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0799
        // Guzzlord: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Earth Eater --
        // :x: (never an innate -> stable): Ground-type moves heal it for 1/4 max HP instead of damaging it.
        SPECIES_GUZZLORD, 1,
        ABILITY_EARTH_EATER
    },
    { // 0801
        // Magearna: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Misty Surge -- :x: (never an innate -> stable):
        // it sets Misty Terrain on entry, blocking status on grounded mons and halving Dragon moves.
        SPECIES_MAGEARNA, 1,
        ABILITY_MISTY_SURGE
    },
    { // 0804
        // Naganadel's only real ability (Beast Boost) is now innate, and it's a frontier set, so its
        // EMPTY slot 1 takes Sheer Force -- :x: (never an innate -> stable) and a strong boon for the Poison Pin
        // Pokemon's Nasty Plot special sweeper: Sludge Wave / Fire Blast / Draco Meteor gain +30% and drop their
        // secondaries, stacking with the innate Beast Boost snowball.
        SPECIES_NAGANADEL, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0805
        // Stakataka: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Bulletproof --
        // :x: (never an innate -> stable): ball, bomb and cannon moves (Focus Blast, Shadow Ball, Sludge Bomb)
        // cannot touch it.
        SPECIES_STAKATAKA, 1,
        ABILITY_BULLETPROOF
    },
    { // 0806
        // Blacephalon: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Flash Fire --
        // :x: (never an innate -> stable): Fire moves are absorbed for a 1.5x Fire-power boost instead of
        // landing.
        SPECIES_BLACEPHALON, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0809
        // Melmetal: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Well-Baked Body
        // -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for +2 Defense.
        SPECIES_MELMETAL, 1,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0818
        // Inteleon: all real abilities now innate, so its empty slot takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_INTELEON, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0820
        // Greedent: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_GREEDENT, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0823
        // Corviknight's three real abilities (Pressure, Unnerve, Mirror Armor) are ALL now innate (Tier 5.7);
        // slot-2 Mirror Armor is pinned by tests (mirror_armor.c / pursuit.c / sticky_web.c / parting_shot.c /
        // ...), so its innate-redundant slot-1 Unnerve -- unpinned by any test (audited) and dead weight on a wall
        // -- takes Bulletproof. Bulletproof is :x: (never an innate -> stable) and thematic for the armored raven:
        // it deflects the Focus Blast / Sludge Bomb / Energy Ball ball-and-bomb moves that threaten its Steel/Flying
        // Defog wall, on top of the innate Mirror Armor and Pressure. Same pick as the Steel walls Skarmory / Registeel.
        SPECIES_CORVIKNIGHT, 1,
        ABILITY_BULLETPROOF
    },
    { // 0826
        // Orbeetle: all its real abilities are now innate, so its innate-redundant slot-2 Telepathy takes a
        // chosen Synchronize -- :x: (never an innate -> stable): any burn, poison or paralysis it takes is
        // passed straight back. (Slot audited: no flag-on fork test pins it.)
        SPECIES_ORBEETLE, 2,
        ABILITY_SYNCHRONIZE
    },
    { // 0834
        // Drednaw: all real abilities now innate, so its innate-redundant slot-1 Shell Armor takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DREDNAW, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0836
        // Boltund: all real abilities now innate, so its empty slot takes a chosen
        // Lightning Rod so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_BOLTUND, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0842
        // Appletun: all its real abilities are now innate, so its innate-redundant slot-2 Thick Fat takes a
        // chosen Well-Baked Body -- :x: (never an innate -> stable): Fire moves are shrugged off entirely for
        // +2 Defense. (Slot audited: no flag-on fork test pins it.)
        SPECIES_APPLETUN, 2,
        ABILITY_WELL_BAKED_BODY
    },
    { // 0847
        // Barraskewda's only real abilities (Swift Swim, Propeller Tail) are BOTH now innate, so its EMPTY
        // slot 1 takes Water Absorb — :x: (never an innate -> stable) and on-theme for the skewer fish: it
        // shrugs off Water moves and heals from them, a clean switch-in boon for its Choice attacker sets
        // alongside the innate Swift Swim (Speed) and Propeller Tail (redirection-proof). Same pick as the
        // other water mons in this table (Clawitzer / Grapploct / Carracosta).
        SPECIES_BARRASKEWDA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0849
        // Toxtricity (Amped): its innate traits are Punk Rock (slot 0) and Technician (slot 2); slot-1 Plus is a
        // real ability that is dead weight in singles, so it takes Volt Absorb -- :x: (never an innate -> stable)
        // and thematic for the Electric/Poison punk: Electric moves miss it and heal it instead, atop innate Punk
        // Rock. Same pick as its Low Key sibling below.
        SPECIES_TOXTRICITY, 1,
        ABILITY_VOLT_ABSORB
    },
    { // 0849
        // Toxtricity Low Key: all real abilities now innate, so its innate-redundant slot-0 Punk Rock takes a chosen
        // Volt Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_TOXTRICITY_LOW_KEY, 0,
        ABILITY_VOLT_ABSORB
    },
    { // 0853
        // Grapploct's only real abilities (Limber, Technician) are BOTH now innate, so its empty slot 1
        // takes a flavorful chosen ability for the frontier sets. Water Absorb is :x: (never an innate ->
        // stable) and on-theme for the octopus: it shrugs off Water moves and heals from them, a clean
        // boon for its bulky Octolock / Bulk Up setup sweeper.
        SPECIES_GRAPPLOCT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0858
        // Hatterene: all its real abilities are now innate, so its innate-redundant slot-1 Anticipation takes a
        // chosen Psychic Surge -- :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting
        // Psychic moves 1.3x and blocking priority on grounded mons. (Slot audited: no flag-on fork test pins
        // it.)
        SPECIES_HATTERENE, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0861
        // Grimmsnarl: all its real abilities are now innate, so its innate-redundant slot-2 Pickpocket takes a
        // chosen Illusion -- :x: (never an innate -> stable): it enters disguised as the party's last member
        // until it takes a direct hit. (Slot audited: no flag-on fork test pins it.)
        SPECIES_GRIMMSNARL, 2,
        ABILITY_ILLUSION
    },
    { // 0862
        // Obstagoon: all its real abilities are now innate, so its innate-redundant slot-0 Reckless takes a
        // chosen Dark Aura -- :x: (never an innate -> stable): every Dark-type move on the field gains 1.33x
        // power.
        SPECIES_OBSTAGOON, 0,
        ABILITY_DARK_AURA
    },
    { // 0863
        // Perrserker: all real abilities now innate, so its innate-redundant slot-1 Tough Claws takes a chosen
        // Bulletproof so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_PERRSERKER, 1,
        ABILITY_BULLETPROOF
    },
    { // 0865
        SPECIES_SIRFETCHD, 1,
        ABILITY_BULLETPROOF
    },
    { // 0870
        // Falinks: all real abilities now innate, so its empty slot takes a chosen
        // No Guard so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_FALINKS, 1,
        ABILITY_NO_GUARD
    },
    { // 0873
        // Frosmoth's only real abilities (Shield Dust, Ice Scales) are BOTH now innate, so its EMPTY slot 1
        // takes Snow Warning — :x: (never an innate -> stable) and flavorful: the frost moth heralds the snow
        // (Ice-type Def boost). Same pick as Articuno/Beartic/Cryogonal/Kyurem.
        SPECIES_FROSMOTH, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0874
        // Stonjourner: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sand Stream --
        // :x: (never an innate -> stable): it sets sandstorm on entry, chipping the field and boosting Rock-
        // type Sp. Def.
        SPECIES_STONJOURNER, 1,
        ABILITY_SAND_STREAM
    },
    { // 0887
        // Dragapult: all its real abilities are now innate, so its innate-redundant slot-2 Cursed Body takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_DRAGAPULT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0888
        // Zacian: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sword of Ruin --
        // :x: (never an innate -> stable): every other battler's Defense is cut to 3/4.
        SPECIES_ZACIAN, 1,
        ABILITY_SWORD_OF_RUIN
    },
    { // 0888
        // Zacian-Crowned: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sword of
        // Ruin -- :x: (never an innate -> stable): every other battler's Defense is cut to 3/4.
        SPECIES_ZACIAN_CROWNED, 1,
        ABILITY_SWORD_OF_RUIN
    },
    { // 0889
        // Zamazenta: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Tablets of Ruin
        // -- :x: (never an innate -> stable): every other battler's Attack is cut to 3/4.
        SPECIES_ZAMAZENTA, 1,
        ABILITY_TABLETS_OF_RUIN
    },
    { // 0889
        // Zamazenta-Crowned: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Tablets
        // of Ruin -- :x: (never an innate -> stable): every other battler's Attack is cut to 3/4.
        SPECIES_ZAMAZENTA_CROWNED, 1,
        ABILITY_TABLETS_OF_RUIN
    },
    { // 0890
        SPECIES_ETERNATUS, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0892
        // Urshifu: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Dark Aura -- :x:
        // (never an innate -> stable): every Dark-type move on the field gains 1.33x power.
        SPECIES_URSHIFU, 1,
        ABILITY_DARK_AURA
    },
    { // 0892
        // Urshifu-Rapid Strike: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Water
        // Absorb -- :x: (never an innate -> stable): Water moves heal it for 1/4 max HP instead of damaging it.
        SPECIES_URSHIFU_RAPID_STRIKE, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0893
        // Zarude: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Grassy Surge -- :x:
        // (never an innate -> stable): it sets Grassy Terrain on entry, healing grounded allies and boosting
        // Grass moves 1.3x.
        SPECIES_ZARUDE, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0894
        // Regieleki's only real ability is Transistor (now innate), and it's a frontier set, so like
        // Glastrier / Spectrier it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1.
        // Lightning Rod is :x: (never an innate -> stable) and on-theme for the Electron Pokemon (the
        // Raichu-Alola precedent): it draws in Electric moves for immunity + a Sp. Atk boost, a clean boon
        // for its fast special sets on top of the innate Transistor Electric-power boost (and innate Levitate).
        SPECIES_REGIELEKI, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0895
        // Regidrago: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_REGIDRAGO, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0896
        // Glastrier's only real ability is Chilling Neigh (now innate), and it's a frontier set, so like
        // Landorus-Therian / Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Snow Warning is :x: (never an innate -> stable) and the ice-legend standard (Articuno /
        // Kyurem / Beartic / Frosmoth / Baxcalibur): the snow the Wild Horse heralds boosts its own Ice-type
        // Defense (Body Press set) on top of the innate Chilling Neigh on-KO Attack snowball.
        SPECIES_GLASTRIER, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0897
        // Spectrier: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_SPECTRIER, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0898
        // Calyrex: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Grassy Surge -- :x: (never an innate ->
        // stable): it sets Grassy Terrain on entry, healing grounded allies and boosting Grass moves 1.3x.
        SPECIES_CALYREX, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0901
        // Ursaluna-Bloodmoon: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Earth
        // Eater -- :x: (never an innate -> stable): Ground-type moves heal it for 1/4 max HP instead of
        // damaging it.
        SPECIES_URSALUNA_BLOODMOON, 1,
        ABILITY_EARTH_EATER
    },
    { // 0902
        // Basculegion's three real abilities (Swift Swim, Adaptability, Mold Breaker) are ALL now innate (Mold
        // Breaker, Tier 5.5); slot-0 Swift Swim and slot-2 Mold Breaker are pinned by tests, so its innate-
        // redundant slot-1 Adaptability (audited: minds_eye.c selects only Swift Swim / Mold Breaker) takes Water
        // Absorb -- :x: (never an innate -> stable) and thematic for the deep-sea fish: it heals on the Water
        // moves its Wave Crash / Flip Turn switch-ins invite, alongside the innate Swift Swim + Adaptability.
        SPECIES_BASCULEGION, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0902
        // Basculegion-F shares the M form's all-innate slots (Swift Swim, Adaptability, Mold Breaker); its
        // innate-redundant slot-1 Adaptability takes the same chosen Water Absorb -- :x: (never an innate ->
        // stable) and thematic for the special breaker, healing on the Water its Hydro Pump / Flip Turn set draws.
        SPECIES_BASCULEGION_F, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0905
        // Enamorus-Therian's only real ability is Overcoat (now innate), so its EMPTY slot 1 takes Sheer Force --
        // :x: (never an innate -> stable) and a strong boon for the Choice Band physical breaker: Play Rough /
        // Springtide Storm gain +30% and drop their secondaries.
        SPECIES_ENAMORUS_THERIAN, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0911
        SPECIES_SKELEDIRGE, 1,
        ABILITY_MUMMY
    },
    { // 0914
        // Quaquaval: all real abilities now innate, so its empty slot takes a chosen
        // Water Absorb so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_QUAQUAVAL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0918
        // Spidops: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Toxic Debris --
        // :x: (never an innate -> stable): a physical hit scatters Toxic Spikes on the attacker's side.
        SPECIES_SPIDOPS, 1,
        ABILITY_TOXIC_DEBRIS
    },
    { // 0920
        // Lokix: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Dark Aura -- :x:
        // (never an innate -> stable): every Dark-type move on the field gains 1.33x power.
        SPECIES_LOKIX, 1,
        ABILITY_DARK_AURA
    },
    { // 0925
        // Maushold's only real abilities (Friend Guard, Technician) are BOTH now innate, so its empty slot 1
        // takes a chosen No Guard — :x: (never an innate -> stable) and synergistic: the whole family attacks
        // as one, so Population Bomb / Beat Up land their full multi-hit reliably.
        SPECIES_MAUSHOLD, 1,
        ABILITY_NO_GUARD
    },
    { // 0934
        // Garganacl: all its real abilities are now innate, so its innate-redundant slot-1 Sturdy takes a
        // chosen Earth Eater -- :x: (never an innate -> stable): Ground-type moves heal it for 1/4 max HP
        // instead of damaging it. (Slot audited: no flag-on fork test pins it.)
        SPECIES_GARGANACL, 1,
        ABILITY_EARTH_EATER
    },
    { // 0943
        // Mabosstiff: all its real abilities are now innate, so its innate-redundant slot-2 Stakeout takes a
        // chosen Sheer Force -- :x: (never an innate -> stable): moves with a secondary effect trade it for
        // 1.3x power. (Slot audited: no flag-on fork test pins it.)
        SPECIES_MABOSSTIFF, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0952
        // Scovillain's only non-drawback real abilities (Chlorophyll, Insomnia) are BOTH now innate (Klutz,
        // slot 2, is a drawback that disables its Life Orb), so its slot-2 Klutz is repurposed to Sheer Force.
        // Sheer Force is :x: (never an innate -> stable) and a strong boon for its mixed sun attacker set:
        // Flamethrower / Earth Power gain +30% and drop their secondaries (and skip Life Orb recoil too).
        SPECIES_SCOVILLAIN, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0956
        // Espathra: all its real abilities are now innate, so its innate-redundant slot-1 Frisk takes a chosen
        // Psychic Surge -- :x: (never an innate -> stable): it sets Psychic Terrain on entry, boosting Psychic
        // moves 1.3x and blocking priority on grounded mons. (Slot audited: no flag-on fork test pins it.)
        SPECIES_ESPATHRA, 1,
        ABILITY_PSYCHIC_SURGE
    },
    { // 0959
        // Tinkaton: all real abilities (Mold Breaker, Own Tempo, Pickpocket) now innate; slot-0 Mold Breaker is
        // Ability()-pinned in the fork innate test, so its innate-redundant slot-1 Own Tempo takes Sheer Force --
        // :x: (never an innate -> stable) and a clean physical boon: Play Rough trades its secondary for +30%
        // power, observable atop innate Mold Breaker (its Gigaton Hammer already ignoring abilities).
        SPECIES_TINKATON, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0961
        // Wugtrio's real abilities are Gooey (slot 0, now innate), Rattled (slot 1, still :white_large_square:
        // pending) and Sand Veil (slot 2, now innate). Its innate-redundant slot-0 Gooey -- unpinned by any test
        // (audited: no Ability(ABILITY_GOOEY) on Wugtrio) -- takes Water Absorb, :x: (never an innate -> stable) and
        // thematic for the Garden Eel: it heals on the Water moves its priority attacker set invites. (Dead-weight
        // real-slot repurpose, Sceptile-style.)
        SPECIES_WUGTRIO, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0962
        // Bombirdier: all its real abilities are now innate, so its innate-redundant slot-1 Keen Eye takes a
        // chosen Wind Rider -- :x: (never an innate -> stable): wind moves and Tailwind give it +1 Attack
        // instead of landing. (Slot audited: no flag-on fork test pins it.)
        SPECIES_BOMBIRDIER, 1,
        ABILITY_WIND_RIDER
    },
    { // 0966
        // Revavroom's only real abilities (Overcoat, Filter) are BOTH now innate, so its EMPTY slot 1 takes Sheer
        // Force -- :x: (never an innate -> stable) and a strong boon for the Poison/Steel hot rod: Gunk Shot /
        // Iron Head gain +30% and drop their secondaries.
        SPECIES_REVAVROOM, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0967
        // Cyclizar's only real abilities (Shed Skin, Regenerator) are BOTH now innate, so its EMPTY slot 1 takes
        // Motor Drive -- :x: (never an innate -> stable) and thematic for the ridable Mount Pokemon: the living
        // motorbike shrugs off Electric moves and revs its Speed +1, observable alongside the innate Regenerator.
        SPECIES_CYCLIZAR, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0976
        // Veluza's only real abilities are Mold Breaker (slot 0, now innate, Tier 5.5) and Sharpness (slot 2, now
        // innate), so its EMPTY slot 1 takes Water Absorb -- :x: (never an innate -> stable) and thematic for the
        // jettisoning fish: it heals on the Water moves its Fillet Away sweeper invites, alongside the innate
        // Sharpness slicing boost.
        SPECIES_VELUZA, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0977
        // Dondozo: all real abilities (Unaware, Oblivious, Water Veil) now innate; slot-1 Oblivious and slot-2
        // Water Veil are Ability()-pinned in the fork innate test, so its innate-redundant slot-0 Unaware takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the huge Water fish: it shrugs off Water
        // moves and heals, a clean boon for its bulky Rest / Order Up sets, atop its always-on innate Unaware.
        SPECIES_DONDOZO, 0,
        ABILITY_WATER_ABSORB
    },
    { // 0979
        // Annihilape: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its innate-redundant slot-1 Inner Focus takes a chosen Anger Shell -- :x:
        // (never an innate -> stable): dropping below half HP trades its defenses for +1 Atk/Sp. Atk/Speed.
        SPECIES_ANNIHILAPE, 1,
        ABILITY_ANGER_SHELL
    },
    { // 0982
        // Dudunsparce: all real abilities now innate, so its innate-redundant slot-0 Serene Grace takes a chosen
        // Simple so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_DUDUNSPARCE, 0,
        ABILITY_SIMPLE
    },
    { // 0983
        // Kingambit: all real abilities now innate, so its innate-redundant slot-2 Pressure takes a chosen
        // Sheer Force so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_KINGAMBIT, 2,
        ABILITY_SHEER_FORCE
    },
    { // 0998
        // Baxcalibur's only real abilities are Thermal Exchange (slot 0, now innate) and Ice Body (slot 2, now
        // innate), so its EMPTY slot 1 takes Snow Warning -- :x: (never an innate -> stable) and self-synergistic
        // for the glacial dragon: the snow it heralds turns on its own innate Ice Body end-turn heal (and boosts
        // Ice-type Def). Same pick as the other Ice legends/mons (Kyurem / Articuno / Beartic / Frosmoth).
        SPECIES_BAXCALIBUR, 1,
        ABILITY_SNOW_WARNING
    },
    { // 1000
        // Gholdengo: all its real abilities are now innate, so its EMPTY slot 1 takes a chosen Sheer Force --
        // :x: (never an innate -> stable): moves with a secondary effect trade it for 1.3x power.
        SPECIES_GHOLDENGO, 1,
        ABILITY_SHEER_FORCE
    },
    { // 1013
        // Sinistcha's Heatproof is now innate, freeing its frontier slot; its slot-0 Hospitality is dead in
        // singles (it heals an ally), so the innate-redundant slot-2 (Heatproof) is repurposed to a chosen
        // Flash Fire — :x: (never an innate -> stable) and thematic: the haunted hot tea turns its Fire
        // weakness into an immunity (stacking with the innate Heatproof's burn-damage halving).
        SPECIES_SINISTCHA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 1017
        // Ogerpon-Hearthflame's only ability (Mold Breaker, from the Hearthflame Mask) is now innate (Tier 5.5),
        // and it's a frontier set, so like Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability
        // in its EMPTY slot 1. Flash Fire is :x: (never an innate -> stable) and thematic for the fiery mask: it
        // shrugs off Fire moves for an immunity + a Fire-power boost on its Ivy Cudgel (Fire-type) Swords Dance
        // sweep, stacking with the innate Mold Breaker ability-ignore.
        SPECIES_OGERPON_HEARTHFLAME, 1,
        ABILITY_FLASH_FIRE
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE, 1,
        ABILITY_EARTH_EATER
    },
    { // 1017
        // Ogerpon: every real ability it has is innate-capable, so its frontier sets had no legal chosen
        // ability left to name. Its EMPTY slot 1 takes a chosen Seed Sower -- :x: (never an innate -> stable):
        // being hit sets Grassy Terrain.
        SPECIES_OGERPON, 1,
        ABILITY_SEED_SOWER
    },
    { // 1018
        // Archaludon: all real abilities now innate, so its innate-redundant slot-1 Sturdy takes a chosen
        // Bulletproof so the frontier chosen slot is a real, non-innate ability (not a redundant innate).
        SPECIES_ARCHALUDON, 1,
        ABILITY_BULLETPROOF
    },
    { // 1019
        // Regenerator and Sticky Hold now innate (slot-0 Supersweet Syrup is pinned by supersweet_syrup.c), so
        // its innate-redundant slot-2 Sticky Hold (audited: unpinned) takes Grassy Surge -- :x: (never an
        // innate -> stable) and thematic for the Apple Nectar dragon: its terrain powers Grass STAB and
        // passively heals. Same pick as Venusaur/Meganium.
        SPECIES_HYDRAPPLE, 2,
        ABILITY_GRASSY_SURGE
    },
};

// GetSpeciesAbility (src/pokemon.c) calls this on EVERY ability lookup, game-wide
// and in the AI's per-move hot path, but only the handful of species with a row
// above ever match — so a plain linear scan taxes every lookup for the whole
// roster (it was measurably inflating the AI's thinking time, and the frontier-slot sweep
// keeps growing the table). A one-time bitmap of "which species have any override
// row" lets the overwhelmingly common no-override case return in O(1); only a
// species that actually carries an override falls through to the short scan.
static u8 sSpeciesHasOverride[(NUM_SPECIES + 7) / 8];
static bool8 sSpeciesHasOverrideReady;

enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot)
{
    u32 i;
    bool32 foundSpecies = FALSE;

    // FORK: gate the override table behind FEATURE_INNATE_ABILITIES, exactly like innates
    // and the fork's other runtime features. TestInitConfigData() force-disables every fork
    // FEATURE flag by default, so upstream tests see VANILLA ability slots -- an override can
    // no longer rewrite a species' ability inside a test that does not opt in via WITH_CONFIG.
    // Real builds default the flag TRUE, so gameplay/frontier behavior is unchanged. Overrides
    // exist solely as the counterpart to innates (they hand a species a real chosen ability
    // precisely because innates made its real slots redundant), so they share the one flag.
    if (!GetConfig(FEATURE_INNATE_ABILITIES))
        return ABILITY_NONE;

    if (!sSpeciesHasOverrideReady)
    {
        for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
        {
            u16 s = sSpeciesAbilityOverrides[i].species;
            sSpeciesHasOverride[s / 8] |= 1 << (s % 8);
        }
        sSpeciesHasOverrideReady = TRUE;
    }

    if (species >= NUM_SPECIES
     || !(sSpeciesHasOverride[species / 8] & (1 << (species % 8))))
        return ABILITY_NONE;

    for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
    {
        if (sSpeciesAbilityOverrides[i].species != species)
        {
            // A species' rows are contiguous (the table is dex-sorted, with a species'
            // slots and formes kept together), so once we have stepped past this species'
            // block there is nothing left to find -- stop rather than scan the whole
            // table. Keeps this hot lookup cheap (it runs deep in AI evaluation) as the
            // override table grows.
            if (foundSpecies)
                break;
            continue;
        }
        foundSpecies = TRUE;
        if (sSpeciesAbilityOverrides[i].slot == slot)
            return sSpeciesAbilityOverrides[i].ability;
    }

    return ABILITY_NONE;
}
