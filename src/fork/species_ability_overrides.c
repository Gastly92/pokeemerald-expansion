#include "global.h"
#include "fork/species_ability_overrides.h"
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

struct SpeciesAbilityOverride
{
    u16 species;
    u8 slot;             // ability slot (0..NUM_ABILITY_SLOTS-1) this row replaces
    enum Ability ability;
};

// Sorted by National Pokédex number (shown in each row's trailing comment); formes share their
// base's number and follow it. Adding a row: drop it at its dex position with a trailing `// <dex>`.
//
// PICK A STABLE CHOSEN ABILITY — cross-reference it against fork-docs/INNATE_ABILITIES_PROGRESS.md.
// Prefer an ability that will NEVER be wired as an innate (marked :x: there — e.g. Lightning Rod,
// Soundproof, Water Absorb, Sheer Force) over one still PENDING (:white_large_square:). A pending
// ability is on track to become an innate, and the moment it is, the Step 3.5 sweep
// (INNATE_ABILITIES.md) has to revisit every set and override that hands it out — so a
// :white_large_square: pick is future churn baked in, while a :x: pick is stable for good. Sceptile's
// LIGHTNING_ROD is the model. (Separately, the slot a row *frees* must already be redundant via an
// *implemented* :white_check_mark: innate — that's the row's whole premise; noted in each comment.)
// The table below was audited on this rule: every row hands out a :x: (never-an-innate) ability,
// or an already-*implemented* :white_check_mark: innate the species does NOT itself carry (Carnivine's
// Chlorophyll, Tornadus-Therian's Prankster, Swellow's Quick Feet, ...), which is likewise stable.
//
// SLOT CHOICE MATTERS — this table is consulted UNCONDITIONALLY by GetSpeciesAbility (it is NOT gated
// by FEATURE_INNATE_ABILITIES), so a row REPLACES that slot's ability game-wide even with innates off.
// Filling an EMPTY slot (ABILITY_NONE) is always safe: nothing is deleted and no upstream test can
// select an empty slot. Repurposing a REAL slot deletes that ability from the species everywhere —
// only do it when nothing observes the slot (audit `Ability(ABILITY_X)` uses in test/battle/ first;
// e.g. Kangaskhan's Scrappy and Vivillon's Shield Dust slots are pinned by upstream tests and must
// stay). The rows above that repurpose real slots (Sceptile, Lopunny, Bronzong, Mamoswine, Beartic,
// Carracosta, Scovillain, Sinistcha, Volbeat, Zangoose, Pidgeot, Chatot, Crawdaunt, Klinklang,
// Bombirdier) were each audited this way.
static const struct SpeciesAbilityOverride sSpeciesAbilityOverrides[] =
{
    { // 0003
        SPECIES_VENUSAUR, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0012
        // Butterfree's only real abilities (Compound Eyes, Tinted Lens) are BOTH now innate, so its EMPTY
        // slot 1 takes Effect Spore — :x: (never an innate -> stable) and flavorful: the powder-scattering
        // butterfly may poison/sleep/paralyze contact attackers, guarding its Quiver Dance setup.
        SPECIES_BUTTERFREE, 1,
        ABILITY_EFFECT_SPORE
    },
    { // 0018
        // Pidgeot's three real abilities (Keen Eye, Tangled Feet, Big Pecks) are ALL now innate, so its innate-
        // redundant slot-1 Tangled Feet -- unpinned by any test (audited) -- takes a chosen No Guard, :x: (never an
        // innate -> stable) and its Mega's signature: Hurricane / Focus Blast never miss.
        SPECIES_PIDGEOT, 1,
        ABILITY_NO_GUARD
    },
    { // 0026
        // Raichu-Alola's only real ability (Surge Surfer) is now innate, so its empty slot 1 takes a
        // flavorful chosen ability for the frontier sets. Lightning Rod is :x: (never an innate -> stable)
        // and on-theme for the Electric mouse: it draws in Electric moves for immunity + a Sp. Atk boost,
        // a clean offensive boon for its special-attacker sets alongside the innate Surge Surfer speed.
        SPECIES_RAICHU_ALOLA, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0028
        // Sandslash's only real abilities (Sand Veil, Sand Rush) are BOTH now innate, so its empty
        // slot 1 takes a flavorful chosen ability. Sand Stream is :x: (never an innate -> stable) and
        // self-synergistic: it sets the sandstorm that turns on Sandslash's own innate Sand Rush (Speed)
        // and Sand Veil (evasion). Precedent: Flygon/Claydol/Torterra also hand out Sand Stream.
        SPECIES_SANDSLASH, 1,
        ABILITY_SAND_STREAM
    },
    { // 0028
        // Sandslash-Alola's only real abilities (Snow Cloak, Slush Rush) are BOTH now innate, so its
        // empty slot 1 takes Snow Warning — :x: (never an innate -> stable) and self-synergistic: the
        // snow it sets turns on its own innate Slush Rush (Speed) and Snow Cloak (evasion). Snowy
        // counterpart to base Sandslash's Sand Stream above.
        SPECIES_SANDSLASH_ALOLA, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0051
        // Dugtrio's three real abilities (Sand Veil, Arena Trap, Sand Force) are ALL now innate. Slot-1 Arena
        // Trap is pinned by tests, so its innate-redundant slot-2 Sand Force (audited: no Ability(ABILITY_SAND_FORCE)
        // on Dugtrio) takes Sand Stream — :x: (never an innate -> stable) and self-synergistic: the sandstorm it
        // kicks up turns on its own innate Sand Veil (evasion) and Sand Force (Rock/Ground/Steel power + sandstorm
        // immunity). Same pick as Sandslash/Donphan/Flygon.
        SPECIES_DUGTRIO, 2,
        ABILITY_SAND_STREAM
    },
    { // 0071
        // Victreebel's only real abilities (Chlorophyll, Gluttony) are BOTH now innate, so its EMPTY slot 1
        // takes Effect Spore -- :x: (never an innate -> stable) and flavorful for the carnivorous pitcher
        // plant: contact attackers risk poison/sleep/paralysis, punishing the hits its Sleep Powder set invites.
        SPECIES_VICTREEBEL, 1,
        ABILITY_EFFECT_SPORE
    },
    { // 0076
        // Rock Head, Sturdy and Sand Veil are ALL now innate, so its innate-redundant slot-2 Sand Veil
        // (audited: unpinned) takes Sand Stream -- :x: (never an innate -> stable) and self-synergistic: the
        // sandstorm turns on its own innate Sand Veil evasion. Same pick as Golem's Rock/Ground kin.
        SPECIES_GOLEM, 2,
        ABILITY_SAND_STREAM
    },
    { // 0091
        // Shell Armor, Skill Link and Overcoat are ALL now innate, so its innate-redundant slot-2 Overcoat
        // (audited: unpinned) takes Sniper, an implemented :white_check_mark: innate (stable) that pays off its
        // Skill Link Icicle Spear crits.
        SPECIES_CLOYSTER, 2,
        ABILITY_SNIPER
    },
    { // 0142
        // Rock Head + Pressure now innate; its pending slot-2 Unnerve (audited: unpinned) takes Tough Claws, an
        // implemented :white_check_mark: innate (stable) and its Mega's ability, powering its contact STAB
        // (Stone Edge/Crunch/Aqua Tail).
        SPECIES_AERODACTYL, 2,
        ABILITY_TOUGH_CLAWS
    },
    { // 0144
        // Articuno's only real abilities (Pressure, Snow Cloak) are BOTH now innate, so its empty
        // slot 1 takes Snow Warning — :x: (never an innate -> stable) and flavorful (the legendary ice
        // bird heralds the blizzard), setting the snow that turns on its own innate Snow Cloak evasion.
        SPECIES_ARTICUNO, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0149
        // Dragonite's only real abilities (Inner Focus, Multiscale) are BOTH now innate, so its EMPTY slot 1
        // takes Reckless, an already-implemented :white_check_mark: innate (stable) it does not carry innately:
        // its recoil STAB (Brave Bird / Double-Edge) is powered up, stacking with the innate Multiscale bulk.
        SPECIES_DRAGONITE, 1,
        ABILITY_RECKLESS
    },
    { // 0154
        // Meganium's only real abilities (Overgrow, Leaf Guard) are BOTH now innate, so its EMPTY slot 1 takes
        // Grassy Surge -- :x: (never an innate -> stable) and thematic for the Herb Pokemon: the terrain it
        // sets powers its Grass STAB and passively heals its bulky sets. Same pick as Venusaur/Celebi.
        SPECIES_MEGANIUM, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0169
        // Inner Focus and Infiltrator now innate, so its EMPTY slot 1 takes Reckless, an implemented
        // :white_check_mark: innate (stable) that powers the fast bat's Brave Bird recoil STAB.
        SPECIES_CROBAT, 1,
        ABILITY_RECKLESS
    },
    { // 0185
        // Sturdy and Rock Head now innate (slot-2 Rattled is pinned by rattled.c), so its innate-redundant
        // slot-1 Rock Head takes Solid Rock, an implemented :white_check_mark: innate (stable) that blunts the
        // Rock mimic's many supereffective hits.
        SPECIES_SUDOWOODO, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0189
        // Chlorophyll, Leaf Guard and Infiltrator are ALL now innate, so its innate-redundant slot-1 Leaf Guard
        // (audited: unpinned) takes Prankster, an implemented :white_check_mark: innate (stable) for the cotton
        // weed's Sleep Powder / Leech Seed support kit.
        SPECIES_JUMPLUFF, 1,
        ABILITY_PRANKSTER
    },
    { // 0205
        // Forretress's only real abilities (Sturdy, Overcoat) are BOTH now innate, so its EMPTY slot 1 takes
        // Filter, an already-implemented :white_check_mark: innate (stable) it does not carry innately: it blunts
        // the supereffective Fire hit its Spikes/Rapid Spin wall most fears.
        SPECIES_FORRETRESS, 1,
        ABILITY_FILTER
    },
    { // 0212
        // Swarm, Technician and Light Metal are ALL now innate (slot-2 Light Metal is pinned by light_metal.c),
        // so its innate-redundant slot-1 Technician (audited: unpinned) takes Tough Claws, an implemented
        // :white_check_mark: innate (stable) that powers Bullet Punch / U-turn contact.
        SPECIES_SCIZOR, 1,
        ABILITY_TOUGH_CLAWS
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
    { // 0269
        // Dustox's only real abilities (Shield Dust, Compound Eyes) are BOTH now innate, so its EMPTY slot 1
        // takes Poison Point — :x: (never an innate -> stable) and flavorful: the toxic-dust moth poisons the
        // contact its Rocky Helmet wall set already punishes.
        SPECIES_DUSTOX, 1,
        ABILITY_POISON_POINT
    },
    { // 0277
        // Swellow's only real abilities (Guts, Scrappy) are BOTH now innate, so its EMPTY slot 1 takes Quick
        // Feet — an already-implemented :white_check_mark: innate (stable, like Slurpuff's Unaware) that Swellow
        // does not carry innately: its Toxic Orb Facade set also gains +50% Speed (and ignores paralysis) once
        // statused, stacking with the innate Guts.
        SPECIES_SWELLOW, 1,
        ABILITY_QUICK_FEET
    },
    { // 0291
        // Speed Boost and Infiltrator now innate, so its EMPTY slot 1 takes Tough Claws, an implemented
        // :white_check_mark: innate (stable) that powers the fastest bug's U-turn / Leech Life / Aerial Ace
        // contact.
        SPECIES_NINJASK, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0306
        // Sturdy, Rock Head and Heavy Metal are ALL now innate (slot-2 Heavy Metal pinned by heavy_metal.c), so
        // its innate-redundant slot-1 Rock Head (audited: unpinned) takes Filter, an implemented
        // :white_check_mark: innate (stable) and its Mega's ability, blunting its 4x Fighting/Ground
        // weaknesses.
        SPECIES_AGGRON, 1,
        ABILITY_FILTER
    },
    { // 0308
        // Medicham's only real abilities are Pure Power (slot 0, now innate) and Telepathy (slot 2, dead in
        // frontier singles), so its EMPTY slot 1 takes Reckless — an already-implemented :white_check_mark:
        // innate (stable, like Slurpuff's Unaware) that Medicham does not carry innately and is a pure boon
        // for the martial artist: it powers up its High Jump Kick STAB (crash move) by 20%, stacking with the
        // innate Pure Power. (Slot 2 Telepathy is left intact for any future doubles set.)
        SPECIES_MEDICHAM, 1,
        ABILITY_RECKLESS
    },
    { // 0313
        // Volbeat's three real abilities (Illuminate, Swarm, Prankster) are ALL now innate, so its slot-1
        // Swarm — now innate-redundant — takes Victory Star. Victory Star is :x: (never an innate -> stable)
        // and flavorful for the firefly's guiding light: it boosts its doubles allies' accuracy. (Azelf also
        // hands out Victory Star.)
        SPECIES_VOLBEAT, 1,
        ABILITY_VICTORY_STAR
    },
    { // 0319
        // Sharpedo's only real abilities (Rough Skin, Speed Boost) are BOTH now innate, so its EMPTY slot 1
        // takes Strong Jaw, an already-implemented :white_check_mark: innate (stable) and its Mega's ability:
        // it powers the Brutal Pokemon's biting STAB (Crunch / Psychic Fangs / Ice Fang) alongside the innate
        // Speed Boost and Rough Skin.
        SPECIES_SHARPEDO, 1,
        ABILITY_STRONG_JAW
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
    { // 0342
        // Crawdaunt's three real abilities (Hyper Cutter, Shell Armor, Adaptability) are ALL now innate; its slot-2
        // Adaptability is pinned by adaptability.c, so its unpinned innate-redundant slot-1 Shell Armor (audited)
        // takes Sniper, an already-implemented :white_check_mark: innate (stable, like Decidueye-Hisui) that pays off
        // the Rogue Pokemon's high-crit Crabhammer.
        SPECIES_CRAWDAUNT, 1,
        ABILITY_SNIPER
    },
    { // 0344
        SPECIES_CLAYDOL, 1, 
        ABILITY_SAND_STREAM
    },
    { // 0348
        SPECIES_ARMALDO, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0358
        SPECIES_CHIMECHO, 1,
        ABILITY_SOUNDPROOF
    },
    { // 0367
        // Huntail's only real abilities (Swift Swim, Water Veil) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the deep-sea fish: it heals on the
        // Water hits its Shell Smash sweeper invites.
        SPECIES_HUNTAIL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0369
        // Swift Swim, Rock Head and Sturdy are ALL now innate, so its innate-redundant slot-1 Rock Head
        // (audited: unpinned) takes Water Absorb -- :x: (never an innate -> stable) and thematic for the
        // deep-sea fossil: it heals on the Water hits a Rock Polish sweeper invites.
        SPECIES_RELICANTH, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0376
        // Metagross's only real abilities are Clear Body (slot 0) and Light Metal (slot 2), BOTH now innate, so
        // its EMPTY slot 1 takes Tough Claws, an already-implemented :white_check_mark: innate (stable) and its Mega's
        // ability: it powers up its contact STAB (Meteor Mash / Bullet Punch / Zen Headbutt).
        SPECIES_METAGROSS, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0377
        // Regirock's only real abilities (Clear Body, Sturdy) are BOTH now innate, so its EMPTY slot 1 takes Solid
        // Rock, an already-implemented :white_check_mark: innate (stable) and thematic for the rock golem: it blunts
        // the supereffective Water/Grass/Ground/Steel/Fighting hits its defensive sets fear.
        SPECIES_REGIROCK, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0378
        // Regice's only real abilities are Clear Body (slot 0, now innate) and Ice Body (slot 2, pending), so its
        // EMPTY slot 1 takes Ice Scales, an already-implemented :white_check_mark: innate (stable) that halves the
        // special damage its Assault Vest special wall already invites.
        SPECIES_REGICE, 1,
        ABILITY_ICE_SCALES
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
    { // 0401
        // Kricketune's only real abilities (Swarm, Technician) are BOTH now innate, so its empty slot 1
        // takes a flavorful chosen ability for the frontier set. Sheer Force is :x: (never an innate ->
        // stable) and a pure boon for its WIDE_LENS Swords Dance sweeper: Pounce gains the +30% and drops
        // its Speed-lowering secondary, on top of the innate Technician Fury Cutter ramp.
        SPECIES_KRICKETUNE, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0419
        // Floatzel's only real abilities (Swift Swim, Water Veil) are BOTH now innate, so its EMPTY slot 1 takes
        // Water Absorb -- :x: (never an innate -> stable) and thematic for the sea weasel: it heals on the Water
        // moves its bulky-water switch-ins invite, alongside the innate Swift Swim speed.
        SPECIES_FLOATZEL, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0424
        // Technician, Skill Link and Prankster are ALL now innate, so its pending slot-1 Pickup (audited:
        // unpinned) takes Tough Claws, an implemented :white_check_mark: innate (stable) that powers the Long
        // Tail monkey's contact kit (Fake Out / Double Hit / Return).
        SPECIES_AMBIPOM, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0426
        // Drifblim's three real abilities (Aftermath, Unburden, Flare Boost) are ALL now innate. Slots 1/2
        // (Unburden, Flare Boost) are pinned by tests (unburden.c / flare_boost.c), so its unpinned slot-0
        // Aftermath (audited: aftermath.c uses Voltorb, not Drifblim) takes Unaware — an already-implemented
        // :white_check_mark: innate (stable, like Slurpuff's) that the balloon does not carry innately and pays
        // off its bulky Calm Mind / Strength Sap staller by ignoring the foe's stat boosts.
        SPECIES_DRIFBLIM, 0,
        ABILITY_UNAWARE
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
        // Chatot's three real abilities (Keen Eye, Tangled Feet, Big Pecks) are ALL now innate, so its innate-
        // redundant slot-1 Tangled Feet -- unpinned by any test (audited) -- takes Punk Rock, an already-implemented
        // :white_check_mark: innate (stable, like Meloetta) that powers the Music Note Pokemon's sound STAB (Boomburst
        // / Hyper Voice / Chatter).
        SPECIES_CHATOT, 1,
        ABILITY_PUNK_ROCK
    },
    { // 0442
        // Pressure and Infiltrator now innate, so its EMPTY slot 1 takes Unaware, an implemented
        // :white_check_mark: innate (stable, like Slurpuff) that pays off the Forbidden Pokemon's Calm Mind /
        // WoW staller by ignoring the foe's boosts.
        SPECIES_SPIRITOMB, 1,
        ABILITY_UNAWARE
    },
    { // 0445
        // Garchomp's only real abilities (Sand Veil, Rough Skin) are BOTH now innate, so its EMPTY slot 1 takes
        // Sand Stream — :x: (never an innate -> stable) and self-synergistic for the desert dragon: the sandstorm
        // it kicks up turns on its own innate Sand Veil evasion (and chips non-Ground/Rock/Steel foes). Same pick
        // as the other Ground sweepers (Dugtrio / Donphan / Flygon / Torterra).
        SPECIES_GARCHOMP, 1,
        ABILITY_SAND_STREAM
    },
    { // 0455
        SPECIES_CARNIVINE, 1,
        ABILITY_CHLOROPHYLL
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
    { // 0473
        // Mamoswine's three real abilities (Oblivious, Snow Cloak, Thick Fat) are ALL now innate, so
        // its slot-2 Thick Fat — now innate-redundant — is repurposed to Snow Warning. Snow Warning is
        // :x: (never an innate -> stable) and self-synergistic: the snow the prehistoric mammoth heralds
        // turns on its own innate Snow Cloak evasion. Same pick as Beartic/Articuno above.
        SPECIES_MAMOSWINE, 2,
        ABILITY_SNOW_WARNING
    },
    { // 0476
        // Probopass's three real abilities (Sturdy, Magnet Pull, Sand Force) are ALL now innate, so its innate-redundant
        // slot-1 Magnet Pull (audited: no Ability(ABILITY_MAGNET_PULL) on Probopass) takes Lightning Rod — :x: (never an
        // innate -> stable) and thematic for the compass magnet: an Electric immunity + Sp. Atk boost for its special wall.
        SPECIES_PROBOPASS, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_HEAT, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_WASH, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_FROST, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_FAN, 1, 
        ABILITY_LIGHTNING_ROD
    },
    { // 0479
        SPECIES_ROTOM_MOW, 1, 
        ABILITY_LIGHTNING_ROD
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
    { // 0487
        SPECIES_GIRATINA_ORIGIN, 1, 
        ABILITY_DRAGONS_MAW
    },
    { // 0488
        SPECIES_CRESSELIA, 1,
        ABILITY_CLOUD_NINE
    },
    { // 0492
        SPECIES_SHAYMIN, 1,
        ABILITY_GRASSY_SURGE
    },
    { // 0492
        SPECIES_SHAYMIN_SKY, 1,
        ABILITY_WIND_RIDER
    },
    { // 0503
        SPECIES_SAMUROTT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0512
        // Simisage's only real abilities (Gluttony, Overgrow) are BOTH now innate, so its EMPTY slot 1 takes
        // Chlorophyll -- an already-implemented :white_check_mark: innate (stable, like Slurpuff's Unaware) it
        // does not carry innately -- doubling the grass monkey's Speed in sun for its offensive sets.
        SPECIES_SIMISAGE, 1,
        ABILITY_CHLOROPHYLL
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
    { // 0531
        SPECIES_AUDINO, 1,
        ABILITY_CUTE_CHARM
    },
    { // 0547
        // Prankster, Infiltrator and Chlorophyll are ALL now innate, so its innate-redundant slot-2 Chlorophyll
        // (audited: unpinned) takes Sweet Veil, an implemented :white_check_mark: innate (stable) and thematic
        // for the cotton fairy: its team can't be put to sleep.
        SPECIES_WHIMSICOTT, 2,
        ABILITY_SWEET_VEIL
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
        // Cute Charm, Technician and Skill Link are ALL now innate, so its innate-redundant slot-1 Technician
        // (audited: unpinned) takes Tough Claws, an implemented :white_check_mark: innate (stable) for the
        // Scarf Chinchilla's Tail Slap / Bullet Seed multi-hit contact.
        SPECIES_CINCCINO, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0576
        // Gothitelle's slot-2 Shadow Tag is now innate; slots 0/1 (Frisk, Competitive) are both still PENDING innates, so
        // rather than hand out a pending pick (future churn) its innate-redundant slot-2 Shadow Tag (audited: no
        // Ability(ABILITY_SHADOW_TAG) on Gothitelle) takes Unaware — an already-implemented :white_check_mark: innate
        // (stable, like Slurpuff's) that pays off its Calm Mind sweeper by ignoring the foe's boosts. (Slots 0/1 kept
        // intact for the future Frisk/Competitive innates.)
        SPECIES_GOTHITELLE, 2,
        ABILITY_UNAWARE
    },
    { // 0594
        SPECIES_ALOMOMOLA, 2,
        ABILITY_WATER_ABSORB
    },
    { // 0598
        // Ferrothorn's only non-pending real abilities are Iron Barbs (slot 0, now innate) and Anticipation (slot 2,
        // still :white_large_square: pending), so its EMPTY slot 1 takes Filter, an already-implemented
        // :white_check_mark: innate (stable) it does not carry innately: the Barb Wire wall blunts the supereffective
        // Fire hit its Spikes / Leech Seed sets most fear. Same pick as the Steel spiker Forretress.
        SPECIES_FERROTHORN, 1,
        ABILITY_FILTER
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
        // Hydration, Sticky Hold and Unburden are ALL now innate, so its innate-redundant slot-1 Sticky Hold
        // (audited: unpinned) takes Tinted Lens, an implemented :white_check_mark: innate (stable) that lets the
        // glass-cannon ninja's Bug Buzz / Focus Blast hit resists for full. (Its former Unburden roster set now
        // rides the innate Unburden and also selects this Tinted Lens slot.)
        SPECIES_ACCELGOR, 1,
        ABILITY_TINTED_LENS
    },
    { // 0635
        SPECIES_HYDREIGON, 1, 
        ABILITY_SHEER_FORCE },
    { // 0641
        SPECIES_TORNADUS_THERIAN, 1,
        ABILITY_PRANKSTER
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
    { // 0648
        // Meloetta's only real ability (Serene Grace) is now innate, so its empty slot 1 takes Punk
        // Rock — an already-implemented :white_check_mark: innate (stable, like Slurpuff's Unaware) and
        // on-theme: the Melody Pokemon's powerful voice boosts its sound-based Hyper Voice / Relic Song
        // (and softens incoming sound moves).
        SPECIES_MELOETTA, 1,
        ABILITY_PUNK_ROCK
    },
    { // 0660
        // Diggersby's three real abilities (Pickup, Cheek Pouch, Huge Power) are ALL now innate, so its slot-2
        // Huge Power — now innate-redundant and unpinned by any test (audited: no Ability(ABILITY_HUGE_POWER)
        // on Diggersby in test/battle) — is repurposed to Scrappy, an already-implemented :white_check_mark:
        // innate (stable, like Lokix's Tough Claws) it does not carry innately: its Normal STAB (Return / Quick
        // Attack) then hits Ghosts, a clean coverage boon alongside the innate Huge Power. (Dead-weight real-slot
        // repurpose, Sceptile-style.)
        SPECIES_DIGGERSBY, 2,
        ABILITY_SCRAPPY
    },
    { // 0678
        // Keen Eye, Infiltrator and Prankster are ALL now innate, so its innate-redundant slot-1 Infiltrator
        // (audited: unpinned) takes Own Tempo, an implemented :white_check_mark: innate (stable) and thematic:
        // the Constraint cat keeps its own tempo, immune to confusion.
        SPECIES_MEOWSTIC_M, 1,
        ABILITY_OWN_TEMPO
    },
    { // 0685
        // Slurpuff's only real abilities (Sweet Veil, Unburden) are BOTH now innate, so its empty slot 1 takes
        // Unaware — an already-implemented :white_check_mark: innate (stable, like Carnivine/Tornadus-Therian)
        // and a pure boon for its Calm Mind wall: it ignores the foe's stat boosts. (Its Belly Drum sweeper set
        // now rides the innate Unburden — still doubling Speed once the Sitrus is eaten — and selects this Unaware slot.)
        SPECIES_SLURPUFF, 1,
        ABILITY_UNAWARE
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
        // Strong Jaw and Rock Head now innate, so its EMPTY slot 1 takes Reckless, an implemented
        // :white_check_mark: innate (stable) that powers the Despot's Head Smash (its innate Rock Head already
        // voids the recoil).
        SPECIES_TYRANTRUM, 1,
        ABILITY_RECKLESS
    },
    { // 0703
        // Carbink's only real abilities (Clear Body, Sturdy) are BOTH now innate, so its EMPTY slot 1 takes Solid
        // Rock, an already-implemented :white_check_mark: innate (stable) and thematic for the jewel: it blunts the
        // many supereffective hits its dual-screens wall fears.
        SPECIES_CARBINK, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0715
        // Infiltrator now innate (slots 0/2 Frisk/Telepathy are pending), so its pending slot-2 Telepathy
        // (audited: unpinned) takes Punk Rock, an implemented :white_check_mark: innate (stable) that powers
        // the Sound Wave dragon's Boomburst / Hyper Voice STAB (and softens incoming sound).
        SPECIES_NOIVERN, 2,
        ABILITY_PUNK_ROCK
    },
    { // 0719
        // Diancie's only real ability (Clear Body) is now innate, so its EMPTY slot 1 takes Solid Rock, an already-
        // implemented :white_check_mark: innate (stable) and thematic for the Jewel Pokemon: it blunts the
        // supereffective Steel/Ground/Water/Grass hits its Life Orb / Leftovers sets fear.
        SPECIES_DIANCIE, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0724
        // Decidueye-Hisui's only real abilities (Overgrow, Scrappy) are BOTH now innate, so its EMPTY slot 1
        // takes Sniper, an already-implemented :white_check_mark: innate (stable) that it does not carry innately:
        // the archer's precision pays off Triple Arrows' boosted crit rate on both roster sets.
        SPECIES_DECIDUEYE_HISUI, 1,
        ABILITY_SNIPER
    },
    { // 0724
        // Overgrow and Long Reach now innate, so its EMPTY slot 1 takes Sniper, an implemented
        // :white_check_mark: innate (stable) -- same pick as Decidueye-Hisui -- paying off Spirit Shackle /
        // Triple Arrows precision.
        SPECIES_DECIDUEYE, 1,
        ABILITY_SNIPER
    },
    { // 0738
        SPECIES_VIKAVOLT, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0745
        // Lycanroc-Dusk's only real ability (Tough Claws) is now innate, so its empty slot 1 takes a
        // flavorful chosen ability. Sand Rush is an already-implemented :white_check_mark: innate (stable,
        // like Carnivine/Tornadus-Therian) and on-theme — it is the signature ability of Lycanroc's other
        // forms, doubling this wolf's Speed in the sand.
        SPECIES_LYCANROC_DUSK, 1,
        ABILITY_SAND_RUSH
    },
    { // 0758
        // Corrosion and Oblivious now innate, so its EMPTY slot 1 takes Flame Body -- :x: (never an innate ->
        // stable) and thematic for the toxic lizard: contact attackers risk a burn, alongside its
        // Toxic-spreading innate Corrosion.
        SPECIES_SALAZZLE, 1,
        ABILITY_FLAME_BODY
    },
    { // 0764
        // Comfey's Triage is now innate (and its Natural Cure was already innate), so its slot-2 Natural
        // Cure — now innate-redundant and unpinned by any test — takes Sweet Veil, an already-implemented
        // :white_check_mark: innate (stable, like Slurpuff's Unaware) that Comfey does not carry innately and
        // is thematic for the flower-lei Pokemon: its soothing aroma keeps the doubles team from being put
        // to sleep. (Slot 1 Triage stays intact — upstream upper_hand/ai_doubles tests pin it.)
        SPECIES_COMFEY, 2,
        ABILITY_SWEET_VEIL
    },
    { // 0771
        // Pyukumuku's real abilities are Innards Out (slot 0, now innate AND pinned by innards_out.c) and Unaware
        // (slot 2, now innate), so its EMPTY slot 1 takes Water Absorb -- :x: (never an innate -> stable) and
        // thematic for the sea cucumber: it heals off the Water hits its Counter / Recover staller invites,
        // stacking with its innate Unaware. (Slot 0 Innards Out stays a real ability -- test-pinned.)
        SPECIES_PYUKUMUKU, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0776
        SPECIES_TURTONATOR, 1,
        ABILITY_FLAME_BODY
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
    { // 0792
        // Lunala's only real ability (Shadow Shield) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1.
        // Adaptability is an already-implemented :white_check_mark: innate (stable) the Moone Pokemon does
        // not carry innately and is self-synergistic: its Moongeist Beam / Shadow Ball get 2x STAB on top of
        // the innate Shadow Shield full-HP bulk (and innate Levitate), a devastating Calm Mind special sweeper.
        SPECIES_LUNALA, 1,
        ABILITY_ADAPTABILITY
    },
    { // 0800
        // Necrozma's only real ability (Prism Armor) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) it takes the innate AND a fork-owned chosen Adaptability in its EMPTY slot 1
        // (stable :white_check_mark: innate it does not carry): 2x STAB on its Photon Geyser on top of the
        // innate Prism Armor supereffective-damage cut (and innate Levitate).
        SPECIES_NECROZMA, 1,
        ABILITY_ADAPTABILITY
    },
    { // 0800
        // Necrozma-Dusk-Mane's only real ability (Prism Armor) is now innate, and it's a frontier set, so
        // like the Regi legends (Y2) its EMPTY slot 1 takes a chosen Adaptability (stable :white_check_mark:
        // innate it does not carry): 2x STAB on Sunsteel Strike / Photon Geyser stacks with the innate Prism
        // Armor supereffective cut, a fearsome Swords Dance physical sweeper.
        SPECIES_NECROZMA_DUSK_MANE, 1,
        ABILITY_ADAPTABILITY
    },
    { // 0800
        // Necrozma-Dawn-Wings' only real ability (Prism Armor) is now innate, and it's a frontier set, so
        // like the Regi legends (Y2) its EMPTY slot 1 takes a chosen Adaptability (stable :white_check_mark:
        // innate it does not carry): 2x STAB on Moongeist Beam / Photon Geyser stacks with the innate Prism
        // Armor supereffective cut, a strong Calm Mind special sweeper.
        SPECIES_NECROZMA_DAWN_WINGS, 1,
        ABILITY_ADAPTABILITY
    },
    { // 0802
        // Marshadow's only real ability (Technician) is now innate, so its empty slot 1 takes a flavorful
        // chosen ability for the frontier sets. Illusion is :x: (never an innate -> stable) and on-theme:
        // the Gloomdweller lurks in shadows and mimics, so it enters disguised as the party's last mon.
        SPECIES_MARSHADOW, 1,
        ABILITY_ILLUSION
    },
    { // 0809
        // Melmetal's only real ability (Iron Fist) is now innate, so its empty slot 1 takes a flavorful
        // chosen ability. Filter is an already-implemented :white_check_mark: innate (stable, like
        // Slurpuff's Unaware) and a clean defensive boon for the colossal steel titan: it blunts the
        // supereffective Fire/Fighting/Ground hits its bulky Iron Fist sets otherwise fear.
        SPECIES_MELMETAL, 1,
        ABILITY_FILTER
    },
    { // 0820
        // Greedent's only real abilities (Cheek Pouch, Gluttony) are BOTH now innate, so its EMPTY slot 1 takes
        // Pickup -- an already-implemented :white_check_mark: innate (stable) it does not carry innately and
        // flavorful for the hoarding squirrel: at end of turn it grabs an item consumed on the field, on top of
        // the innate Cheek Pouch heal loop.
        SPECIES_GREEDENT, 1,
        ABILITY_PICKUP
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
    { // 0853
        // Grapploct's only real abilities (Limber, Technician) are BOTH now innate, so its empty slot 1
        // takes a flavorful chosen ability for the frontier sets. Water Absorb is :x: (never an innate ->
        // stable) and on-theme for the octopus: it shrugs off Water moves and heals from them, a clean
        // boon for its bulky Octolock / Bulk Up setup sweeper.
        SPECIES_GRAPPLOCT, 1,
        ABILITY_WATER_ABSORB
    },
    { // 0865
        // Sirfetch'd's Scrappy is now innate and its remaining slot-0 Steadfast is weak (and pending), so its
        // EMPTY slot 1 takes Super Luck, an already-implemented :white_check_mark: innate (stable) that it does
        // not carry innately: the duelist's precision stacks with the Leek for guaranteed crits.
        SPECIES_SIRFETCHD, 1,
        ABILITY_SUPER_LUCK
    },
    { // 0873
        // Frosmoth's only real abilities (Shield Dust, Ice Scales) are BOTH now innate, so its EMPTY slot 1
        // takes Snow Warning — :x: (never an innate -> stable) and flavorful: the frost moth heralds the snow
        // (Ice-type Def boost). Same pick as Articuno/Beartic/Cryogonal/Kyurem.
        SPECIES_FROSMOTH, 1,
        ABILITY_SNOW_WARNING
    },
    { // 0874
        // Stonjourner's only real ability (Power Spot) is now innate, and it's a frontier set, so like
        // Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1.
        // Power Spot is doubles-only (it boosts allies), so a singles set needs a real ability: Solid Rock
        // is an already-implemented :white_check_mark: innate (stable) and thematic for the giant megalith,
        // blunting the supereffective Water/Grass/Fighting/Ground/Steel hits its bulk otherwise fears. Same
        // pick as the other lone rocks (Regirock / Carbink / Diancie).
        SPECIES_STONJOURNER, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0887
        // Clear Body and Infiltrator now innate (slot-2 Cursed Body is pending), so its pending slot-2 Cursed
        // Body (audited: unpinned) takes Dragon's Maw -- :x: (never an innate -> stable) and thematic: the
        // Stealth dragon's Draco Meteor / Dragon Darts hit harder. Same pick as Giratina-Origin.
        SPECIES_DRAGAPULT, 2,
        ABILITY_DRAGONS_MAW
    },
    { // 0890
        SPECIES_ETERNATUS, 1,
        ABILITY_POISON_TOUCH
    },
    { // 0892
        // Its ONLY real ability is Unseen Fist (now innate) -- like Ogerpon-Cornerstone its EMPTY slot 1 takes
        // Sniper, an implemented :white_check_mark: innate (stable) that pays off Single Strike's always-crit
        // Wicked Blow.
        SPECIES_URSHIFU, 1,
        ABILITY_SNIPER
    },
    { // 0892
        // Its ONLY real ability is Unseen Fist (now innate), so its EMPTY slot 1 takes Sniper, an implemented
        // :white_check_mark: innate (stable) that pays off Rapid Strike's always-crit Surging Strikes.
        SPECIES_URSHIFU_RAPID_STRIKE, 1,
        ABILITY_SNIPER
    },
    { // 0893
        // Zarude's only real ability is Leaf Guard (now innate), so -- like Ogerpon-Cornerstone -- its EMPTY slot 1
        // takes Tough Claws, an already-implemented :white_check_mark: innate (stable) it does not carry innately:
        // the Rogue Monkey's kit (Power Whip / Darkest Lariat / Knock Off / U-turn) is all contact.
        SPECIES_ZARUDE, 1,
        ABILITY_TOUGH_CLAWS
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
        // Regidrago's only real ability is Dragon's Maw (now innate), and it's a frontier set, so like
        // Glastrier / Spectrier it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1.
        // Adaptability is an already-implemented :white_check_mark: innate (stable, like Spectrier's
        // Infiltrator) the Dragon Orb Pokemon does not carry innately, and is self-synergistic: its Draco
        // Meteor / Outrage / Dragon Claw get 2x STAB on top of the innate Dragon's Maw 1.5x, a devastating
        // Choice Dragon breaker.
        SPECIES_REGIDRAGO, 1,
        ABILITY_ADAPTABILITY
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
        // Spectrier's only real ability is Grim Neigh (now innate), and it's a frontier set, so like
        // Landorus-Therian / Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Infiltrator is an already-implemented :white_check_mark: innate (stable, like
        // Slurpuff's Unaware) that the Swift Horse does not carry innately and is thematic for a phasing
        // phantom: its Nasty Plot / Substitute special sweeper ignores the foe's screens and Substitute,
        // on top of the innate Grim Neigh on-KO Sp. Atk snowball.
        SPECIES_SPECTRIER, 1,
        ABILITY_INFILTRATOR
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
    { // 0920
        // Lokix's only real abilities (Swarm, Tinted Lens) are BOTH now innate, so its EMPTY slot 1 takes
        // Tough Claws, an already-implemented :white_check_mark: innate (stable) that it does not carry innately:
        // the kickboxing grasshopper's kit (First Impression / Sucker Punch / Leech Life / Throat Chop) is
        // all contact.
        SPECIES_LOKIX, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0925
        // Maushold's only real abilities (Friend Guard, Technician) are BOTH now innate, so its empty slot 1
        // takes a chosen No Guard — :x: (never an innate -> stable) and synergistic: the whole family attacks
        // as one, so Population Bomb / Beat Up land their full multi-hit reliably.
        SPECIES_MAUSHOLD, 1,
        ABILITY_NO_GUARD
    },
    { // 0943
        // Guard Dog and Stakeout now innate (slot-0 Intimidate is pending), so its innate-redundant slot-2
        // Stakeout (audited: unpinned) takes Strong Jaw, an implemented :white_check_mark: innate (stable) that
        // powers the Boss mastiff's Crunch / Jaw Lock / Psychic Fangs bites.
        SPECIES_MABOSSTIFF, 2,
        ABILITY_STRONG_JAW
    },
    { // 0952
        // Scovillain's only non-drawback real abilities (Chlorophyll, Insomnia) are BOTH now innate (Klutz,
        // slot 2, is a drawback that disables its Life Orb), so its slot-2 Klutz is repurposed to Sheer Force.
        // Sheer Force is :x: (never an innate -> stable) and a strong boon for its mixed sun attacker set:
        // Flamethrower / Earth Power gain +30% and drop their secondaries (and skip Life Orb recoil too).
        SPECIES_SCOVILLAIN, 2,
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
        // Bombirdier's three real abilities (Big Pecks, Keen Eye, Rocky Payload) are ALL now innate, so its innate-
        // redundant slot-1 Keen Eye -- unpinned by any test (audited) -- takes Reckless, an already-implemented
        // :white_check_mark: innate (stable, like Medicham) that powers the vulture's Brave Bird recoil dive.
        SPECIES_BOMBIRDIER, 1,
        ABILITY_RECKLESS
    },
    { // 0966
        // Revavroom's only real abilities (Overcoat, Filter) are BOTH now innate, so its EMPTY slot 1 takes Sheer
        // Force -- :x: (never an innate -> stable) and a strong boon for the Poison/Steel hot rod: Gunk Shot /
        // Iron Head gain +30% and drop their secondaries.
        SPECIES_REVAVROOM, 1,
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
    { // 1013
        // Sinistcha's Heatproof is now innate, freeing its frontier slot; its slot-0 Hospitality is dead in
        // singles (it heals an ally), so the innate-redundant slot-2 (Heatproof) is repurposed to a chosen
        // Flash Fire — :x: (never an innate -> stable) and thematic: the haunted hot tea turns its Fire
        // weakness into an immunity (stacking with the innate Heatproof's burn-damage halving).
        SPECIES_SINISTCHA, 2,
        ABILITY_FLASH_FIRE
    },
    { // 1017
        SPECIES_OGERPON_CORNERSTONE, 1,
        ABILITY_EARTH_EATER
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

enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSpeciesAbilityOverrides); i++)
    {
        if (sSpeciesAbilityOverrides[i].species == species
         && sSpeciesAbilityOverrides[i].slot == slot)
            return sSpeciesAbilityOverrides[i].ability;
    }

    return ABILITY_NONE;
}
