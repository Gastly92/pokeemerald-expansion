# The extended frontier roster and the draft

Two related pieces of fork-owned data drive what Pokémon actually appear in the Battle
Frontier:

- **The roster** (`src/fork/frontier_extended_mons.c`) — modern competitive sets that
  replace upstream's `gBattleFrontierMons`, tuned around the `DETERMINISTIC_*` changes.
- **The draft** (`src/fork/frontier_draft.c`) — the rules that pick from it: uniqueness,
  tier gating, gimmick limits, and per-slot placement constraints.

Plus the **species tier map** (`src/fork/species_tiers.c`) that the draft gates on.

> **Authoring or auditing sets?** The creative process — flavor rules, what the
> `DETERMINISTIC_*` regime does to set-building, the free-gimmicks caveats, the
> `EVS()` / `NATURE()` helpers — is [`LINE_REVIEW.md`](LINE_REVIEW.md) Step 3. This doc
> is the rules the engine enforces around those sets.

- **Flag:** `B_FRONTIER_EXTENDED_MONS` (`include/config/frontier.h`). Plain `#if`-gated,
  like the other `B_FRONTIER_*` flags — not the runtime-config/`WITH_CONFIG` system.
- **Tests:** `test/fork/frontier_extended_roster.c`, `test/fork/frontier_draft.c`

## The roster

Under the flag, `GetFactoryMonsTable()` / `GetFactoryMonsCount()` swap in the fork
roster on the Battle Factory's code paths, and the vanilla tier ramp in
`sInitialRentalMonRanges` / `GetFactoryMonId` is bypassed — sets are drawn
**uniformly**. The vanilla `FRONTIER_MONS_HIGH_TIER` (849) Level-50 gate is disabled
too, since the roster has no tier ordering, so there is no 849-entry ceiling.

The list is ordered by National Pokédex number, and carries **several distinct builds
per species**, so an opponent's exact set can't be read off its species. Each set is
tagged `FORMAT_SINGLES` / `FORMAT_DOUBLES` / `FORMAT_BOTH`, and `GetFactoryMonId` only
draws sets valid for the current battle format.

It is named "extended" rather than "factory" or "competitive" because the other
frontier facilities will adopt the same list — the Battle Tower already has (see
[`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md)).

### Coverage

**Generations I–IX are comprehensively built out — every fully-evolved species has at
least one build**: 1–2 flavor/utility sets for niche mons, ~2–4 for stronger ones, Mega
and non-Mega. There is a deliberate **offense/defense balance** — Eviolite / Assault
Vest / Rocky Helmet / Heavy-Duty Boots tanks and hazard/Defog/cleric/Trick-Room support
sets alongside the sweepers.

**All regional forms** (Alolan, Galarian, Paldean, Hisuian) and their cross-gen
evolutions appear as their own entries, slotted at their base species' National Dex
number — alongside Paradox Pokémon (with Booster Energy + Protosynthesis/Quark Drive),
Ultra Beasts, and notable formes (Therian, Necrozma, Origin Dialga/Palkia, Urshifu,
Calyrex riders, Ogerpon masks, Bloodmoon Ursaluna, the appliance Rotoms).

**Held items are deliberately varied** — Leftovers, berries, Assault Vest, Rocky Helmet,
Heavy-Duty Boots, type-boost items and screens/utility items are favored over
defaulting everything to Choice items and Life Orb.

### Roster rules

- **Only fully-evolved Pokémon appear**, except NFEs with a genuine niche their
  evolution doesn't dominate (Eviolite Chansey, Light Ball Pikachu).
- **Cross-gen evolutions replace their pre-evos** — Magnezone / Electivire / Magmortar /
  Rhyperior / Tangrowth stand in for Magneton / Electabuzz / Magmar / Rhydon / Tangela.
- **Movesets are not restricted to the species' learnset.** Flavorful and powerful
  coverage is allowed; see `LINE_REVIEW.md`.
- **Innate abilities are respected.** A species with an innate Levitate
  (`src/fork/innate_abilities.c`) is never given a redundant Air Balloon, and a set's
  `.ability` carries a **complementary** ability that runs alongside the innate — e.g.
  Slowbro `OWN_TEMPO` + innate Regenerator, Rotom `LIGHTNING_ROD` + innate Levitate,
  Clefable `MAGIC_GUARD` + innate Unaware.
- **No set uses `ABILITY_NONE`** — banned by the roster test. For *ability-locked*
  species whose only real ability is now granted innately, the complementary ability is
  made selectable through the fork-owned **species ability overrides** table
  (`src/fork/species_ability_overrides.c`), never by editing upstream species data. See
  [`INNATE_ABILITIES.md`](INNATE_ABILITIES.md) for that table's rules.

Every set's `.ability` is verified legal through that hook by
`test/fork/frontier_extended_roster.c`, so a typo can't silently downgrade to slot 0.

### Save compatibility ⚠️

Saved rentals reference the roster by **array index** — there is no stable-ID layer. So:

> **Appending entries is save-safe. Inserting or removing mid-list invalidates an
> in-progress rented team in an existing save.**

## The species tier map

`src/fork/species_tiers.c` + `include/fork/species_tiers.h` provide a species→tier
table with `GetSpeciesTier(species)` / `SpeciesIsTier(species, tier)` accessors, so
facility logic can control what appears where. Data lives in a fork-owned file, never in
`gSpeciesInfo` — modeled on `src/fork/innate_abilities.c`.

These are **this fork's own groupings, not the official Game Freak categories**:

| Tier | Contents |
|---|---|
| `TIER_MYTHICAL` | The strongest "restricted" legends — box/cover legendaries (Groudon, Kyogre, Dialga, Zacian, Koraidon…), Mewtwo, Lugia/Ho-Oh — **plus** the official event Mythicals (Mew, Celebi, Jirachi, Arceus, Diancie, Magearna, Pecharunt…) |
| `TIER_LEGENDARY` | The sub-legendaries — birds, beasts, lake trio, Regis, genies + Therian, musketeers, Tapus, Heatran, Cresselia, Urshifu, Ogerpon, the Loyal Three… |
| `TIER_PSEUDO` | The 600-BST pseudo-legendaries (Dragonite, Garchomp, Dragapult…), Ultra Beasts, Paradox Pokémon, and Treasures of Ruin |
| `TIER_NORMAL` | Everything else — species simply absent from the table |

Derived from the upstream `isRestrictedLegendary` / `isSubLegendary` / `isMythical` /
`isUltraBeast` / `isParadox` flags with those groupings layered on top.

**Keyed by exact species id, so formes are classified on their own merits** rather than
inheriting one tier from the base's dex number. A powerful forme can outrank its base —
**Shaymin-Sky is `TIER_LEGENDARY`** (its fork-boosted Serene Grace) while ordinary
Shaymin is `TIER_NORMAL` — and a weak base can sit below its formes — **base Calyrex is
`TIER_NORMAL`** while its Ice/Shadow riders are `TIER_MYTHICAL`.

Currently covers the special species and formes the roster uses; anything unlisted
returns `TIER_NORMAL`.

## Draft rules

Applied to player rentals, ordinary opponents and the Frontier Brain alike.

### Uniqueness

- **Unique species** — the vanilla `currSpecies` one-dup quirk is closed under the flag.
- **Unique held item.**

### Gimmick items

**At most one Mega Stone and one Z-Crystal per generated team**
(`TeamHasGimmickItemConflict`). Post-win swaps can exceed this, which is fine — the
per-battle Mega/Z limit still applies. Neutralized under `FEATURE_FREE_GIMMICKS` (see
[`FREE_GIMMICKS.md`](FREE_GIMMICKS.md)).

### Tier gating

Via `TierRejectsCandidate`, using the tier map above:

| Team | Rule |
|---|---|
| Player's initial team, ordinary opponents | No legendaries or mythicals; **at most one pseudo** |
| Every `FRONTIER_STAGES_PER_CHALLENGE`-th opponent (the last of each set) | **One guaranteed legendary** at a random slot; the rest drafted normally |
| Frontier Brain | **One legendary + one mythical** at two random slots; the other four drafted normally |

Forced slots are reserved with `ReserveForcedTierSlot` **before** drafting, so the
special mon's party position is randomized rather than always landing last.

### Illusion stays functional

A drafted Illusion mon (Zoroark, the Eon duo) is never placed in the team's **last**
slot. There it would have no party member behind it to disguise as —
`GetIllusionMonPartyId` bails on a last-slot Illusion mon, so the disguise silently
never forms. `IllusionMonRejectsSlot` rejects such a candidate from `partySize - 1`.

Applied to Factory opponents, the Frontier Brain, and Tower opponents; covered by
`test/fork/frontier_draft.c`.

## Gimmick readiness at draft time

`ApplyDraftGimmickReadiness` (`src/fork/frontier_draft.c`) is called once from
`CreateFacilityMon`, so the only upstream edit is a single additive call line. Every
facility mon gets:

- The **maximum Dynamax Level** (`MAX_DYNAMAX_LEVEL`, 10), so a Dynamaxed mon gets the
  full HP boost.
- The **Gigantamax Factor**, if its species has a G-Max form
  (`DoesSpeciesHaveFormChangeMethod(species, FORM_CHANGE_BATTLE_GIGANTAMAX)`) — so it
  Gigantamaxes instead of plain Dynamaxing.

No per-entry annotation is needed, but both honor an explicit override
(`.dynamaxLevel` / `.gigantamaxFactor`). Covered by
`test/fork/frontier_extended_roster.c`.

### Tera types are live, not cosmetic

Each set's `.teraType` is real. Under `FEATURE_FREE_GIMMICKS` (on by default),
`CanTerastallize` (`src/battle_terastal.c`) skips the Tera Orb bag check and the
`B_FLAG_TERA_ORB_*` charge flags entirely, and AI battlers skip them unconditionally —
so drafted mons on both sides can Terastallize into the recorded type, competing for
the one-gimmick-per-trainer slot like any other gimmick. Covered by
`FREE_GIMMICKS: the player can Terastallize without a Tera Orb` in
`test/fork/free_gimmicks.c`.

## Post-battle rental swap reverts battle forms

A defeated opponent's in-battle form change — e.g. Palafin's Zero to Hero →
`PALAFIN_HERO` — is only undone on the *player's* party at battle end. So renting that
mon carried the changed form into the next battle, with Zero to Hero firing on send-out.
`CopySwappedMonData` (`src/battle_factory_screen.c`) now reverts the swapped-in mon to
its base form.

## Scope

Roster swap is **Factory + Tower** so far, via `GetFactoryMonsTable()` /
`GetFactoryMonsCount()`. Dome, Palace, Arena, Pyramid and Pike keep
`gBattleFrontierMons` until they adopt the list — see
[`FRONTIER_ENDLESS.md`](FRONTIER_ENDLESS.md).
