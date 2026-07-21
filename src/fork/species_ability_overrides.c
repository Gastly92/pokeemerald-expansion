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
    { // 0024
        // Arbok's three real abilities (Intimidate, Shed Skin, Unnerve) are ALL now innate, so its innate-
        // redundant slot-2 Unnerve -- unpinned by any test (audited) -- takes a chosen Poison Point, :x: (never
        // an innate -> stable) and on-theme for the venomous Cobra Pokemon: contact attackers risk poison,
        // observable alongside the innate Intimidate. (Its slot-1 Shed Skin frontier set is pinned by shed_skin.c
        // and stays a real Shed Skin.)
        SPECIES_ARBOK, 2,
        ABILITY_POISON_POINT
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
        // Rock Head, Sturdy and Sand Veil are ALL now innate, so its innate-redundant slot-2 Sand Veil
        // (audited: unpinned) takes Sand Stream -- :x: (never an innate -> stable) and self-synergistic: the
        // sandstorm turns on its own innate Sand Veil evasion. Same pick as Golem's Rock/Ground kin.
        SPECIES_GOLEM, 2,
        ABILITY_SAND_STREAM
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
    { // 0091
        // Shell Armor, Skill Link and Overcoat are ALL now innate, so its innate-redundant slot-2 Overcoat
        // (audited: unpinned) takes Sniper, an implemented :white_check_mark: innate (stable) that pays off its
        // Skill Link Icicle Spear crits.
        SPECIES_CLOYSTER, 2,
        ABILITY_SNIPER
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
    { // 0205
        // Forretress's only real abilities (Sturdy, Overcoat) are BOTH now innate, so its EMPTY slot 1 takes
        // Filter, an already-implemented :white_check_mark: innate (stable) it does not carry innately: it blunts
        // the supereffective Fire hit its Spikes/Rapid Spin wall most fears.
        SPECIES_FORRETRESS, 1,
        ABILITY_FILTER
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
    { // 0237
        // Hitmontop's three real abilities (Intimidate, Technician, Steadfast) are ALL now innate; slot-0
        // Intimidate is test-pinned (intimidate.c / ai_switching.c), so its innate-redundant slot-2 Steadfast
        // (audited: unpinned) takes a chosen No Guard -- :x: (never an innate -> stable) and thematic for the
        // spinning martial artist: its Triple Axel / Close Combat / Rapid Spin never miss. Same pick as Pidgeot /
        // Hitmonlee.
        SPECIES_HITMONTOP, 2,
        ABILITY_NO_GUARD
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
        // Linoone's three real abilities (Pickup, Gluttony, Quick Feet) are ALL now innate; slot-0 Pickup is the
        // default read by tests (fury_cutter.c / pursuit.c / ai.c / ai_switching.c), so its innate-redundant slot-1
        // Gluttony (audited: no Ability(ABILITY_GLUTTONY) on Linoone in test/) takes Scrappy -- an already-implemented
        // :white_check_mark: innate (stable, like Diggersby's) it does not carry innately: its Normal STAB (Extreme
        // Speed / Facade / Body Slam) then hits Ghosts, a clean coverage boon for its Belly Drum priority sweeper
        // alongside the innate Quick Feet (Speed) / Gluttony.
        SPECIES_LINOONE, 1,
        ABILITY_SCRAPPY
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
    { // 0284
        // Masquerain's only real abilities (Intimidate, Unnerve) are BOTH now innate, so its EMPTY slot 1 takes a
        // chosen Storm Drain -- :x: (never an innate -> stable) and thematic for the pond-skating water strider: it
        // draws in Water moves for a Sp. Atk boost + immunity, a clean boon for its Quiver Dance special sweeper.
        SPECIES_MASQUERAIN, 1,
        ABILITY_STORM_DRAIN
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
        // Sharpedo's only real abilities (Rough Skin, Speed Boost) are BOTH now innate, so its EMPTY slot 1
        // takes Strong Jaw, an already-implemented :white_check_mark: innate (stable) and its Mega's ability:
        // it powers the Brutal Pokemon's biting STAB (Crunch / Psychic Fangs / Ice Fang) alongside the innate
        // Speed Boost and Rough Skin.
        SPECIES_SHARPEDO, 1,
        ABILITY_STRONG_JAW
    },
    // NOTE: Wailord (0321) and Camerupt (0323) are deliberately NOT given a chosen override — they are Batch W9
    // EXCLUSIONS. Both are used in AI tests that are sensitive to the species' whole ability SET, not just the
    // chosen slot: Wailord in test/battle/ai/ai.c ("best OHKO move" — a move-absorbing ability like Water Absorb in
    // any slot makes the AI treat Water Spout as possibly-nullified and pick Thunder instead) and Camerupt in
    // test/battle/ai/ai_thinking_time.c (the "Steven multi" node-count ceiling is already at its limit, so an extra
    // AI-evaluation branch from an immunity ability like Flash Fire tips it over). So both keep their now-innate
    // chosen ability (Water Veil / Magma Armor) — redundant-but-correct — like the other AI/test-pinned exclusions
    // (Slowbro / Snorlax / Clefable).
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
    { // 0373
        // Salamence's only real abilities (Intimidate, Moxie) are BOTH now innate, so its EMPTY slot 1 takes a chosen
        // Rivalry -- :x: (never an innate -> stable) and thematic for the ferocious, territorial dragon: it hits
        // same-gender foes 25% harder, observable on its Dragon Dance / Choice sweeper sets alongside the innate
        // Intimidate.
        SPECIES_SALAMENCE, 1,
        ABILITY_RIVALRY
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
    { // 0461
        // Weavile's only real abilities (Pressure, Pickpocket) are BOTH now innate, so its EMPTY slot 1 takes Tough
        // Claws, an already-implemented :white_check_mark: innate (stable) it does not carry innately and perfectly
        // thematic for the Sharp Claw Pokemon: its entirely-contact kit (Triple Axel / Icicle Crash / Knock Off /
        // Ice Shard / Low Kick) gains +30%, a broad boon for its fast Life Orb physical attacker on top of the innate
        // Pickpocket steal.
        SPECIES_WEAVILE, 1,
        ABILITY_TOUGH_CLAWS
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
    { // 0476
        // Probopass's three real abilities (Sturdy, Magnet Pull, Sand Force) are ALL now innate, so its innate-redundant
        // slot-1 Magnet Pull (audited: no Ability(ABILITY_MAGNET_PULL) on Probopass) takes Lightning Rod — :x: (never an
        // innate -> stable) and thematic for the compass magnet: an Electric immunity + Sp. Atk boost for its special wall.
        SPECIES_PROBOPASS, 1,
        ABILITY_LIGHTNING_ROD
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
    { // 0483
        // Dialga's only real abilities (Pressure, Telepathy) are BOTH now innate (Telepathy, Batch U), so its
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
        // Palkia's only real abilities (Pressure, Telepathy) are BOTH now innate (Telepathy, Batch U), so its
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
        // Base Giratina (Altered)'s only real abilities (Pressure, Telepathy) are BOTH now innate (Telepathy,
        // Batch U -- Levitate is an Origin-forme innate it also carries as flavor), so its EMPTY slot 1 takes
        // Unaware -- an already-implemented :white_check_mark: innate (stable, like Spiritomb's) it does not carry
        // innately and a pure boon for the Renegade's bulky Will-O / Dragon Tail / Defog wall: it ignores the
        // foe's stat boosts. (Origin forme takes its own Dragon's Maw pick just below.)
        SPECIES_GIRATINA_ALTERED, 1,
        ABILITY_UNAWARE
    },
    { // 0487
        // Giratina-Origin's only real ability (Levitate) is now innate, so its EMPTY slot 1 takes Dragon's Maw --
        // an already-implemented :white_check_mark: innate (Batch Y2, stable) it does NOT carry innately (its sole
        // innate is Levitate), so the pick stays observable and never needs re-pointing: the Renegade forme's
        // draconic might powers its Draco Meteor / Dragon Claw / Shadow Force nuke.
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
        // Liepard's three real abilities (Limber, Unburden, Prankster) are ALL now innate; slot-0 Limber is the
        // default read by revelation_dance.c, so its innate-redundant slot-1 Unburden (audited: no
        // Ability(ABILITY_UNBURDEN) on Liepard in test/) takes Infiltrator -- an already-implemented
        // :white_check_mark: innate (stable) it does not carry innately and thematic for the sneaky cat: its Foul
        // Play / Knock Off / Encore ignore the target's Substitute and screens, a clean disruption boon alongside
        // its innate Prankster (priority status) and Unburden (Speed once Sitrus is eaten).
        SPECIES_LIEPARD, 1,
        ABILITY_INFILTRATOR
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
        // Leavanny's three real abilities (Swarm, Chlorophyll, Overcoat) are ALL now innate, and none is test-pinned
        // (audited: no test selects Leavanny), so its innate-redundant slot-2 Overcoat takes Sharpness -- an
        // already-implemented :white_check_mark: innate (stable) it does not carry innately and perfectly thematic for
        // the Nurturing Pokemon's scythe arms: its slicing STAB (Leaf Blade / X-Scissor) gains +50%, a clean boon for
        // its Swords Dance / Sticky Web sets alongside the innate Chlorophyll (Speed in sun).
        SPECIES_LEAVANNY, 2,
        ABILITY_SHARPNESS
    },
    { // 0547
        // Prankster, Infiltrator and Chlorophyll are ALL now innate, so its innate-redundant slot-2 Chlorophyll
        // (audited: unpinned) takes Sweet Veil, an implemented :white_check_mark: innate (stable) and thematic
        // for the cotton fairy: its team can't be put to sleep.
        SPECIES_WHIMSICOTT, 2,
        ABILITY_SWEET_VEIL
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
    { // 0579
        // Reuniclus's three real abilities (Overcoat, Magic Guard, Regenerator) are ALL now innate (Tier 5.4) and
        // NONE is test-pinned, so its innate-redundant slot-2 Regenerator (audited: no Ability() on Reuniclus in
        // test/) takes No Guard -- :x: (never an innate -> stable) and thematic for the pure-intellect Psychic: its
        // Focus Blast (run on both frontier sets) never misses. Slots 0/1 (Overcoat / Magic Guard) stay intact so
        // Magic Guard remains a visible chosen option, and the innate Magic Guard still voids Life Orb recoil.
        SPECIES_REUNICLUS, 2,
        ABILITY_NO_GUARD
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
    { // 0643
        // Reshiram's only real ability (Turboblaze) is now innate (Batch Y8), and it's a frontier set, so like the
        // Regi legends (Y2) / Necrozma (Y3) / Solgaleo (Y4) it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Flash Fire is :x: (never an innate -> stable) and thematic for the Vast White dragon: it
        // shrugs off Fire moves for an immunity + a Fire-power boost on its Blue Flare special sets, stacking with the
        // innate Turboblaze ability-ignore. (Kyurem-White, its fusion, takes the same pick.)
        SPECIES_RESHIRAM, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0644
        // Zekrom's only real ability (Teravolt) is now innate (Batch Y8), and it's a frontier set, so it takes the
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
        // Kyurem-White's only real ability (Turboblaze) is now innate (Batch Y8), and it's a frontier set, so it takes
        // the innate AND a fork-owned chosen ability in its EMPTY slot 1. Flash Fire is :x: (never an innate -> stable)
        // and thematic: fused with Reshiram, the Boundary dragon wields fire (Fusion Flare), so it shrugs off Fire for
        // an immunity + a Fire-power boost on its special sets. Same pick as Reshiram itself.
        SPECIES_KYUREM_WHITE, 1,
        ABILITY_FLASH_FIRE
    },
    { // 0646
        // Kyurem-Black's only real ability (Teravolt) is now innate (Batch Y8), and it's a frontier set, so it takes
        // the innate AND a fork-owned chosen ability in its EMPTY slot 1. Motor Drive is :x: (never an innate ->
        // stable) and thematic: fused with Zekrom, the Boundary dragon wields lightning (Fusion Bolt), so it draws in
        // Electric moves for an immunity + a Speed boost that snowballs its Dragon Dance sweeper. Same pick as Zekrom.
        SPECIES_KYUREM_BLACK, 1,
        ABILITY_MOTOR_DRIVE
    },
    { // 0648
        // Meloetta's only real ability (Serene Grace) is now innate, so its empty slot 1 takes Punk
        // Rock — an already-implemented :white_check_mark: innate (stable, like Slurpuff's Unaware) and
        // on-theme: the Melody Pokemon's powerful voice boosts its sound-based Hyper Voice / Relic Song
        // (and softens incoming sound moves).
        SPECIES_MELOETTA, 1,
        ABILITY_PUNK_ROCK
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
        // Diggersby's three real abilities (Pickup, Cheek Pouch, Huge Power) are ALL now innate, so its slot-2
        // Huge Power — now innate-redundant and unpinned by any test (audited: no Ability(ABILITY_HUGE_POWER)
        // on Diggersby in test/battle) — is repurposed to Scrappy, an already-implemented :white_check_mark:
        // innate (stable, like Lokix's Tough Claws) it does not carry innately: its Normal STAB (Return / Quick
        // Attack) then hits Ghosts, a clean coverage boon alongside the innate Huge Power. (Dead-weight real-slot
        // repurpose, Sceptile-style.)
        SPECIES_DIGGERSBY, 2,
        ABILITY_SCRAPPY
    },
    { // 0675
        // Pangoro's three real abilities (Iron Fist, Mold Breaker, Scrappy) are ALL now innate (Mold Breaker,
        // Tier 5.5); slots 1/2 (Mold Breaker, Scrappy) are pinned by ai_doubles.c, so its innate-redundant
        // slot-0 Iron Fist (audited: unpinned) takes Tough Claws, an implemented :white_check_mark: innate
        // (stable) it does not carry innately: every move on its sets (Knock Off / Close Combat / Gunk Shot /
        // Drain Punch / Sucker Punch) makes contact, so the +30% is a broad boon alongside innate Iron Fist.
        SPECIES_PANGORO, 0,
        ABILITY_TOUGH_CLAWS
    },
    { // 0678
        // Keen Eye, Infiltrator and Prankster are ALL now innate, so its innate-redundant slot-1 Infiltrator
        // (audited: unpinned) takes Own Tempo, an implemented :white_check_mark: innate (stable) and thematic:
        // the Constraint cat keeps its own tempo, immune to confusion.
        SPECIES_MEOWSTIC_M, 1,
        ABILITY_OWN_TEMPO
    },
    { // 0683
        // Aromatisse's only real abilities (Healer, Aroma Veil) are BOTH now innate (Aroma Veil, Batch U), so its
        // EMPTY slot 1 takes Misty Surge -- :x: (never an innate -> stable) and thematic for the Fragrance
        // Pokemon: on switch-in it blankets the field in Misty Terrain, protecting its team from status and
        // softening Dragon moves, a clean support boon for its Trick Room / Aromatherapy / Wish cleric set.
        // (Slot-2 Aroma Veil stays intact -- aroma_veil.c selects it -- so the innate Aroma Veil is still an
        // observable chosen option.)
        SPECIES_AROMATISSE, 1,
        ABILITY_MISTY_SURGE
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
    { // 0701
        // Hawlucha's three real abilities (Limber, Unburden, Mold Breaker) are ALL now innate (Mold Breaker,
        // Tier 5.5); slot-2 Mold Breaker is pinned by the innate test, so its innate-redundant slot-1 Unburden
        // (audited: no Ability(ABILITY_UNBURDEN) on Hawlucha in test/) takes Tough Claws, an implemented
        // :white_check_mark: innate (stable) it does not carry innately: all its STAB (Acrobatics / Close Combat
        // / Thunder Punch / Stone Edge) makes contact, gaining +30% alongside the innate Unburden speed.
        SPECIES_HAWLUCHA, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0703
        // Carbink's only real abilities (Clear Body, Sturdy) are BOTH now innate, so its EMPTY slot 1 takes Solid
        // Rock, an already-implemented :white_check_mark: innate (stable) and thematic for the jewel: it blunts the
        // many supereffective hits its dual-screens wall fears.
        SPECIES_CARBINK, 1,
        ABILITY_SOLID_ROCK
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
    { // 0720
        // Hoopa's only real ability (Magician) is now innate, so its EMPTY slot 1 takes Tinted Lens, an already-
        // implemented :white_check_mark: innate (stable, like Accelgor) the Mischief Pokemon does not carry innately
        // and thematic for the ringed djinn that warps space: its Psychic / Shadow Ball / Focus Blast special breaker
        // punches through resists for full damage, on top of the innate Magician steal.
        SPECIES_HOOPA, 1,
        ABILITY_TINTED_LENS
    },
    { // 0720
        // Hoopa-Unbound's only real ability (Magician) is now innate, so its EMPTY slot 1 takes Tough Claws, an
        // already-implemented :white_check_mark: innate (stable) the Unbound djinn does not carry innately and
        // thematic for its six massive arms: its physical contact kit (Hyperspace Fury / Drain Punch / Fire Punch /
        // Zen Headbutt) gains +30%, a strong boon for its Life Orb mixed wallbreaker on top of the innate Magician steal.
        SPECIES_HOOPA_UNBOUND, 1,
        ABILITY_TOUGH_CLAWS
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
    { // 0727
        // Incineroar's only real abilities (Blaze, Intimidate) are BOTH now innate, so its EMPTY slot 1 takes
        // Tough Claws -- an already-implemented :white_check_mark: innate (stable, like Weavile/Metagross) it does
        // not carry innately and perfectly thematic for the Heel Pokemon: its entirely-contact kit (Fake Out /
        // Flare Blitz / Darkest Lariat / Knock Off / U-turn) gains +30%, a clean offensive boon on its bulky
        // Intimidate pivot sets, observable alongside the innate Intimidate.
        SPECIES_INCINEROAR, 1,
        ABILITY_TOUGH_CLAWS
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
    { // 0775
        // Komala's only real ability (Comatose) is now innate, and it's a frontier set, so like the Y-batch
        // sole-ability legends / Gholdengo it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1.
        // Sticky Hold is an already-implemented :white_check_mark: innate (stable, the same pick as Gholdengo) that
        // the Drowsing Pokemon does not carry innately and is perfectly thematic: Komala clings to its log and never
        // lets go, so its held item can't be knocked off or stolen -- a clean boon on top of its innate Comatose
        // (full status immunity) and innate Unaware.
        SPECIES_KOMALA, 1,
        ABILITY_STICKY_HOLD
    },
    { // 0776
        SPECIES_TURTONATOR, 1,
        ABILITY_FLAME_BODY
    },
    { // 0741
        // Oricorio's sole real ability (Dancer) is now innate (Tier 5.9), and it's a frontier set, so like the
        // other sole-ability innate carriers its EMPTY slot 1 takes a chosen ability. Tinted Lens is an already-
        // implemented :white_check_mark: innate (stable, like Slurpuff's Unaware) the Baile dancer does not carry
        // innately: its Life Orb Revelation Dance / Hurricane special sweeper punches through resists for full
        // damage, a clean offensive boon on top of the innate Dancer copy.
        SPECIES_ORICORIO, 1,
        ABILITY_TINTED_LENS
    },
    { // 0741
        // Oricorio-Pau shares base Oricorio's sole Dancer (now innate); its EMPTY slot 1 takes the same chosen
        // Tinted Lens so the ability is consistent across the forms and lets its Hurricane / Revelation Dance
        // pivot hit resists for full, on top of the innate Dancer.
        SPECIES_ORICORIO_PAU, 1,
        ABILITY_TINTED_LENS
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
    { // 0791
        // Solgaleo's only real ability (Full Metal Body) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) / Necrozma / Lunala (Y3) it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Tough Claws is an already-implemented :white_check_mark: innate (stable) the Sunne
        // Pokemon does not carry innately and powers its contact STAB (Sunsteel Strike / Close Combat / Flare
        // Blitz) on both physical sets, stacking with the innate Full Metal Body stat-drop lock. Same pick as
        // its fellow Steel bruiser Metagross.
        SPECIES_SOLGALEO, 1,
        ABILITY_TOUGH_CLAWS
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
    { // 0793
        // Nihilego's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so like
        // the Regi legends (Y2) / Necrozma / the Zacian line (Y6) it takes the innate AND a fork-owned chosen
        // ability in its EMPTY slot 1. Merciless is an already-implemented :white_check_mark: innate (stable) the
        // Parasite Pokemon does not carry innately and is self-synergistic: its Toxic Spikes / Sludge Wave poison
        // the foe, then Merciless auto-crits the poisoned target, on top of the innate Beast Boost snowball (and
        // innate Levitate).
        SPECIES_NIHILEGO, 1,
        ABILITY_MERCILESS
    },
    { // 0794
        // Buzzwole's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Iron Fist, an already-implemented :white_check_mark: innate (stable) the Swollen
        // Pokemon does not carry innately and powers its all-punch kit (Ice Punch / Thunder Punch / Drain Punch),
        // stacking with the innate Beast Boost on-KO snowball.
        SPECIES_BUZZWOLE, 1,
        ABILITY_IRON_FIST
    },
    { // 0795
        // Pheromosa's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Tough Claws, an already-implemented :white_check_mark: innate (stable) the Lissome
        // Pokemon does not carry innately and powers its contact STAB (Close Combat / Triple Axel / U-turn / Rapid
        // Spin), stacking with the innate Beast Boost snowball.
        SPECIES_PHEROMOSA, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0796
        // Xurkitree's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Lightning Rod -- :x: (never an innate -> stable) and thematic for the living power
        // line (the Raichu-Alola / Regieleki precedent): it draws in Electric moves for immunity + a Sp. Atk boost
        // for its Tail Glow special sets, on top of the innate Beast Boost snowball (and innate Levitate).
        SPECIES_XURKITREE, 1,
        ABILITY_LIGHTNING_ROD
    },
    { // 0797
        // Celesteela's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Filter, an already-implemented :white_check_mark: innate (stable, like Melmetal /
        // Zamazenta) the Launch Pokemon does not carry innately: it blunts the supereffective Fire / Electric hits
        // its bulky Leech Seed / Autotomize sets otherwise fear, stacking with the innate Beast Boost snowball.
        SPECIES_CELESTEELA, 1,
        ABILITY_FILTER
    },
    { // 0798
        // Kartana's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Sharpness, an already-implemented :white_check_mark: innate (stable) the Drawn Sword
        // Pokemon does not carry innately and is perfectly thematic: the origami blade's slicing STAB (Leaf Blade /
        // Sacred Sword) gets +50%, stacking with the innate Beast Boost on-KO snowball (and innate Levitate).
        SPECIES_KARTANA, 1,
        ABILITY_SHARPNESS
    },
    { // 0799
        // Guzzlord's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Filter, an already-implemented :white_check_mark: innate (stable, like Celesteela)
        // the Junkivore Pokemon does not carry innately: it blunts the supereffective hits (notably its 4x Fairy
        // weakness) its enormous-HP mixed tank sets fear, stacking with the innate Beast Boost snowball.
        SPECIES_GUZZLORD, 1,
        ABILITY_FILTER
    },
    { // 0804
        // Naganadel's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Sheer Force -- :x: (never an innate -> stable) and a strong boon for the Poison Pin
        // Pokemon's Nasty Plot special sweeper: Sludge Wave / Fire Blast / Draco Meteor gain +30% and drop their
        // secondaries, stacking with the innate Beast Boost snowball.
        SPECIES_NAGANADEL, 1,
        ABILITY_SHEER_FORCE
    },
    { // 0805
        // Stakataka's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Solid Rock, an already-implemented :white_check_mark: innate (stable, like Regirock /
        // Carbink / Stonjourner) the Rampart Pokemon does not carry innately: it blunts the supereffective Fighting
        // / Ground / Water / Steel / Grass hits its Trick Room wall otherwise fears, stacking with the innate Beast
        // Boost snowball.
        SPECIES_STAKATAKA, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0806
        // Blacephalon's only real ability (Beast Boost) is now innate (Batch Y7), and it's a frontier set, so its
        // EMPTY slot 1 takes Infiltrator, an already-implemented :white_check_mark: innate (stable, like Spectrier)
        // the Fireworks Pokemon does not carry innately and is thematic for a ghostly clown: its Shadow Ball /
        // Fire Blast / Focus Blast special sweeper ignores the foe's screens and Substitute, on top of the innate
        // Beast Boost snowball (and innate Levitate).
        SPECIES_BLACEPHALON, 1,
        ABILITY_INFILTRATOR
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
        // Orbeetle's three real abilities (Swarm, Frisk, Telepathy) are ALL now innate (Frisk Batch L,
        // Telepathy Batch U), and NONE is test-pinned (audited: no reference to Orbeetle in test/), so its
        // innate-redundant slot-2 Telepathy is repurposed to Unaware -- an already-implemented :white_check_mark:
        // innate (stable, like Spiritomb's) it does not carry innately and a pure boon for the Seven Spot's bulky
        // Calm Mind sweeper / dual-screens pivot: it ignores the foe's stat boosts. (Both frontier sets, formerly
        // on the now-innate Telepathy / Frisk, select this Unaware slot.)
        SPECIES_ORBEETLE, 2,
        ABILITY_UNAWARE
    },
    { // 0842
        // Appletun's three real abilities (Ripen, Gluttony, Thick Fat) are ALL now innate; slot-0 Ripen is the
        // default read by sleep_clause.c / dynamax.c, so its innate-redundant slot-2 Thick Fat (audited: no
        // Ability(ABILITY_THICK_FAT) on Appletun in test/) takes Filter -- an already-implemented :white_check_mark:
        // innate (stable) it does not carry innately: it blunts the crippling 4x Ice weakness its Grass/Dragon
        // typing suffers, a clean defensive boon for its bulky special tank alongside the innate Thick Fat. Same
        // "blunt the big supereffective weakness" pick as the Steel/Grass walls Forretress / Ferrothorn / Aggron.
        SPECIES_APPLETUN, 2,
        ABILITY_FILTER
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
    { // 0858
        // Hatterene's three real abilities (Healer, Anticipation, Magic Bounce) are ALL now innate (Tier 5.8), so
        // its innate-redundant slot-1 Anticipation -- unpinned by any test (audited: only dynamax.c references
        // Hatterene, and never pins Anticipation) -- takes Unaware. Unaware is an already-implemented
        // :white_check_mark: innate (stable, like Slurpuff / Gothitelle / Ursaluna-Bloodmoon) the serene witch
        // does not carry innately and a pure boon for its bulky Calm Mind sets: it ignores the foe's stat boosts,
        // on top of the innate Magic Bounce reflection. (Slot-2 Magic Bounce stays a real slot -- its identity.)
        SPECIES_HATTERENE, 1,
        ABILITY_UNAWARE
    },
    { // 0861
        // Grimmsnarl's three real abilities (Prankster, Frisk, Pickpocket) are ALL now innate; slot-0 Prankster is
        // the default read by lash_out.c, so its innate-redundant slot-2 Pickpocket -- unpinned by any test (audited)
        // -- takes Infiltrator. Infiltrator is an already-implemented :white_check_mark: innate (stable, like Liepard)
        // the Bulk Up imp does not carry innately and thematic for the sneaky hair-tendril attacker: its Foul Play /
        // Spirit Break / Darkest Lariat ignore the foe's screens and Substitute, a clean boon for its Prankster
        // dual-screens lead and Bulk Up sweeper on top of the innate Prankster / Frisk. (All three frontier sets,
        // formerly on the now-innate Pickpocket, select this Infiltrator slot.)
        SPECIES_GRIMMSNARL, 2,
        ABILITY_INFILTRATOR
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
    { // 0888
        // Zacian's only real ability (Intrepid Sword) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) / Necrozma / Lunala (Y3) it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Tough Claws is an already-implemented :white_check_mark: innate (stable) the Warrior
        // Pokemon does not carry innately and powers its entirely-contact kit (Behemoth Blade / Play Rough /
        // Close Combat / Crunch / Wild Charge), stacking with the innate Intrepid Sword switch-in Attack boost.
        // Same pick as its fellow physical contact bruisers Solgaleo / Zarude.
        SPECIES_ZACIAN, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0888
        // Zacian-Crowned shares base Zacian's sole Intrepid Sword (now innate); its EMPTY slot 1 takes the same
        // chosen Tough Claws so the ability is consistent across the Hero <-> Crowned form change and powers its
        // contact STAB (Behemoth Blade / Play Rough / Close Combat), stacking with the innate Intrepid Sword.
        SPECIES_ZACIAN_CROWNED, 1,
        ABILITY_TOUGH_CLAWS
    },
    { // 0889
        // Zamazenta's only real ability (Dauntless Shield) is now innate, and it's a frontier set, so like the
        // Regi legends (Y2) it takes the innate AND a fork-owned chosen ability in its EMPTY slot 1. Filter is an
        // already-implemented :white_check_mark: innate (stable, like Melmetal / Stonjourner) the Warrior Pokemon
        // does not carry innately and is thematic for the "Shield" defender: it blunts the supereffective hits its
        // Body Press / Iron Defense wall otherwise fears, stacking with the innate Dauntless Shield switch-in Defense
        // boost.
        SPECIES_ZAMAZENTA, 1,
        ABILITY_FILTER
    },
    { // 0889
        // Zamazenta-Crowned shares base Zamazenta's sole Dauntless Shield (now innate); its EMPTY slot 1 takes the
        // same chosen Filter so the ability is consistent across the Hero <-> Crowned form change and blunts the
        // supereffective Fire / Fighting / Ground hits its Fighting/Steel Body Press wall fears, on top of the innate
        // Dauntless Shield.
        SPECIES_ZAMAZENTA_CROWNED, 1,
        ABILITY_FILTER
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
    { // 0901
        // Ursaluna-Bloodmoon's only real ability is Mind's Eye (now innate), and it's a frontier set, so like
        // Landorus-Therian / Ogerpon-Cornerstone it takes the innate AND a fork-owned chosen ability in its
        // EMPTY slot 1. Unaware is an already-implemented :white_check_mark: innate (stable, like Slurpuff /
        // Spiritomb / Gothitelle) the Peat Pokemon does not carry innately and is a pure boon for its bulky
        // Calm Mind special tank set: it ignores the foe's stat boosts, stacking with the innate Mind's Eye
        // evasion-ignore + Ghost coverage on Hyper Voice.
        SPECIES_URSALUNA_BLOODMOON, 1,
        ABILITY_UNAWARE
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
    { // 0934
        // Garganacl's three real abilities (Purifying Salt, Sturdy, Clear Body) are ALL now innate; slot-0
        // Purifying Salt is pinned by purifying_salt.c, so its innate-redundant slot-1 Sturdy -- unpinned by any
        // test (audited) -- takes Solid Rock. Solid Rock is an already-implemented :white_check_mark: innate
        // (stable, like Regirock / Carbink / Stonjourner / Stakataka) the rock-salt golem does not carry innately
        // and blunts the supereffective Water / Grass / Ground / Steel / Fighting hits its Salt Cure wall fears,
        // on top of the innate Purifying Salt status immunity.
        SPECIES_GARGANACL, 1,
        ABILITY_SOLID_ROCK
    },
    { // 0943
        // Mabosstiff's three real abilities (Intimidate, Guard Dog, Stakeout) are ALL now innate, so its
        // innate-redundant slot-2 Stakeout (audited: unpinned) takes Strong Jaw, an implemented
        // :white_check_mark: innate (stable) that powers the Boss mastiff's Crunch / Jaw Lock / Psychic Fangs
        // bites, observable alongside the innate Intimidate. (Both frontier sets select this Strong Jaw slot.)
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
    { // 0956
        // Espathra's three real abilities (Opportunist, Frisk, Speed Boost) are ALL now innate; slot-0
        // Opportunist is pinned by opportunist.c, so its innate-redundant slot-1 Frisk -- unpinned by any test
        // (audited) -- takes Competitive. Competitive is an already-implemented :white_check_mark: innate (stable)
        // the Ostrich Pokemon does not carry innately and is self-synergistic with its Calm Mind / Stored Power
        // sweeper: a foe's stat drop instead sharply raises its Sp. Atk (feeding Stored Power), on top of the
        // innate Opportunist boost-copy and Speed Boost.
        SPECIES_ESPATHRA, 1,
        ABILITY_COMPETITIVE
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
    { // 0976
        // Veluza's only real abilities are Mold Breaker (slot 0, now innate, Tier 5.5) and Sharpness (slot 2, now
        // innate), so its EMPTY slot 1 takes Water Absorb -- :x: (never an innate -> stable) and thematic for the
        // jettisoning fish: it heals on the Water moves its Fillet Away sweeper invites, alongside the innate
        // Sharpness slicing boost.
        SPECIES_VELUZA, 1,
        ABILITY_WATER_ABSORB
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
        // Gholdengo's ONLY real ability (Good as Gold) is now innate, so its EMPTY slot 1 takes a chosen ability
        // for the frontier sets. Sticky Hold is an already-implemented :white_check_mark: innate Gholdengo does
        // NOT itself carry (stable, like Carnivine's Chlorophyll) and thematic + low-impact: the hoard of 1000
        // coins won't let its treasure be stolen. The innate Good as Gold (still blocking status moves) stays
        // observable behind the different chosen ability.
        SPECIES_GHOLDENGO, 1,
        ABILITY_STICKY_HOLD
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
// roster (it was measurably inflating the AI's thinking time, and the Batch W sweep
// keeps growing the table). A one-time bitmap of "which species have any override
// row" lets the overwhelmingly common no-override case return in O(1); only a
// species that actually carries an override falls through to the short scan.
static u8 sSpeciesHasOverride[(NUM_SPECIES + 7) / 8];
static bool8 sSpeciesHasOverrideReady;

enum Ability GetSpeciesAbilityOverride(u16 species, u8 slot)
{
    u32 i;

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
        if (sSpeciesAbilityOverrides[i].species == species
         && sSpeciesAbilityOverrides[i].slot == slot)
            return sSpeciesAbilityOverrides[i].ability;
    }

    return ABILITY_NONE;
}
