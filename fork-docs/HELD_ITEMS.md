# Held items: what the roster uses, and the buff backlog

An audit of every held item against the extended frontier roster
(`src/fork/frontier_extended_mons.c`), plus the design backlog it produced: for each
item nothing holds, *why* nothing holds it, and what would have to change for a set to
want it.

This doc is the counterpart to [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md) (what the
roster *is*) and [`LINE_REVIEW.md`](LINE_REVIEW.md) (how to author a set). It exists
because "held items are deliberately varied" is a claim worth measuring, and because the
`BUFF_*` project needs a queue: [`config/buff.h`](../include/config/buff.h) already
fixed Shell Bell, Leech Seed, the accuracy lenses and the type-boost items, and the
question "what next" should be answered from data rather than vibes.

> **Picking up this work fresh?** Jump to [Processing a batch](#processing-a-batch) — it
> is written to make "process the next batch of pending held items" a complete
> instruction, including the decisions already settled so they don't get re-argued.

## Status lives in the tracker, not here

`test/fork/held_item_tracker.c` is the **source of truth for where each item stands**,
and CI gates it. Every item that does something when held sits on exactly one of three
lists, and the list *is* the status:

| List | Meaning | Gated? |
| --- | --- | --- |
| `sDoneItems[]` | Balance is right **and** the roster uses it | Yes — at least one set holds it, and it stays under `HELD_ITEM_MAX_ROSTER_SHARE_PERCENT` (20%) of the roster |
| `sPendingItems[]` | Work outstanding: needs a buff, is mechanically fine and needs a set, or is **thinly drafted** (on one set, which with one-item-per-team is a roll away from never appearing) | No — a pending item may sit at zero sets, which is usually why it is pending |
| `sIgnoredItems[]` | Unreachable in a frontier battle | Exempt from the done gates, but **no set may hold one** — a Mega Stone or Z-Crystal in the slot does nothing under `FEATURE_FREE_GIMMICKS`, so the set would be playing an item down with no other symptom |

A **one-set count is not automatically a shortfall.** The form-change enablers — Adamant
Crystal, Lustrous Globe, Griseous Orb, Red/Blue Orb, Rusted Sword and Shield, the three
Ogerpon masks — each unlock exactly one forme on exactly one species, so one is the
ceiling rather than a gap. They stay done so the one-set gate keeps watching them: delete
the Giratina-Origin set and CI should notice.

Graduating an item to `sDoneItems[]` is what **arms** the gates for it. That is the
failure the tracker exists to catch: Wide Lens, Zoom Lens, Blunder Policy, Razor Fang and
Lansat Berry all received real engine work and then shipped to nobody, because nothing
connected "we buffed it" to "a set holds it".

The lists are swept for completeness against `gItemsInfo[]`, so an item arriving with an
upstream sync cannot sit unclassified — and a genuinely new hold effect fails CI until
someone judges it. Mega Stones and Z-Crystals are excluded by hold effect rather than by
127 boilerplate rows.

**So: don't record status in this doc.** Two copies drift, and the prose copy is the one
that goes stale. This doc carries the *reasoning* — why an item is where it is, and the
buff sketch that would move it. The groups below are that reasoning, not a status board.

Run it with `make check TESTS="Held item tracker"`.

## Reproducing the audit

```bash
# What the roster holds, by frequency.
grep -oP '\.heldItem = \KITEM_\w+' src/fork/frontier_extended_mons.c | sort | uniq -c | sort -rn

# The universe to compare against: every item in the build with a hold effect.
grep -B2 -A30 '^    \[ITEM_' src/data/items.h | grep -E '\[ITEM_|\.holdEffect ='
```

Everything below was measured this way against the roster as of this doc's commit.
**Re-measure before acting on it** — the roster grows every line review, and the counts
here will drift.

## What the roster uses today

**1629 sets. 138 distinct held items** (plus one deliberate `ITEM_NONE`, a Persian Thief
set). Against the **239 items in the build that have a battle-relevant hold effect**
(excluding Mega Stones and Z-Crystals — see "Never expected" below), that is **138 used,
101 unused**.

The distribution is heavily top-loaded:

| Item | Sets | Share |
| --- | --- | --- |
| Leftovers | 216 | 13.3% |
| Life Orb | 186 | 11.7% |
| Choice Band | 110 | 6.9% |
| Sitrus Berry | 105 | 6.6% |
| Rocky Helmet | 87 | 5.5% |
| Heavy-Duty Boots / Choice Specs | 71 each | 4.4% each |
| Assault Vest | 60 | 3.8% |
| Focus Band | 52 | 3.3% |
| Choice Scarf | 44 | 2.8% |

The top two alone are **25% of the roster**, and the tail is long and thin: **44 items
appear on one or two sets**.

### Why the concentration matters more than it looks

The Factory draft rejects any candidate whose `heldItem` already appears on the team
being built (`src/battle_frontier.c:344`, "Ensure this Pokemon's held item isn't a
duplicate" — a non-`ITEM_NONE` match makes the draft skip that mon and roll again).
**Only one of each item can appear per team**, for the player's rentals and for each
opponent alike.

So item concentration is a *draft-rate tax*: the 217 Leftovers sets are competing for a
single team slot, and each one loses every roll where another mon already took it.
Moving a set off a crowded item onto an uncrowded one makes that set appear more often
at no balance cost. That is the real prize in this audit — unused items are unused
*capacity*, not just unused flavor.

> `LINE_REVIEW.md` step 4 carries this rule for set authors. The numbers there are from
> an earlier roster; these are current.

## Never expected — the structural exclusions

Do not re-litigate these in a future audit; they are correctly at zero.

| Group | Count | Why zero is right |
| --- | --- | --- |
| Mega Stones | 92 | `FEATURE_FREE_GIMMICKS` drops the item requirement entirely — Megas are picked from the trigger, and X/Y is chosen from the battler's stats. See [`FREE_GIMMICKS.md`](FREE_GIMMICKS.md). A stone in the slot would be a wasted item, not an enabler. |
| Z-Crystals | 35 | Same flag: Z-Moves derive from species + move with no crystal. |
| Out-of-battle utility | 17 | Exp. Share, Lucky Egg, Amulet Coin, Luck Incense, Soothe Bell, Cleanse Tag, Pure Incense, Smoke Ball, Everstone, Destiny Knot, Macho Brace and the six Power items. Nothing they do is reachable in a frontier battle (the Power items' Speed halving is reachable, but a Trick Room set gets the same result for free with `IVS(SPE, 0)`). |

That leaves **122 unused items that are live in battle** — the actual backlog.

## The backlog

Four groups, in the order they are worth working on.

### Group A — already fixed, just not drafted (no code, roster work)

These items are good, or were *specifically repaired by this fork*, and nothing holds
them. Zero engine work; they need a line review to pick them up. This is the
highest-value group precisely because it costs nothing.

| Item(s) | Status | Note |
| --- | --- | --- |
| **Wide Lens, Zoom Lens** | Buffed twice, held by **zero** sets | `BUFF_ACCURACY_ITEMS` gave both a job in the PP economy and `BUFF_ACCURACY_ITEMS_REVEAL` made them INFO-viewer instruments. Both buffs currently ship to nobody. Note the lens reveals only work for the **player**, so these belong on sets the player will want to *rent*. |
| **Blunder Policy** | Rebuilt, held by **zero** sets | `DETERMINISTIC_HOLD_EFFECTS` re-armed it on the deterministic blunders (Protect, semi-invulnerability, Wide/Quick/Crafty Guard, Psychic Terrain, a type immunity, a blocking ability, an Air Balloon). In doubles any one avoiding target arms it. |
| **Razor Fang** | Buffed, held by **zero** sets | The flinch items became guaranteed one-shots under `DETERMINISTIC_HOLD_EFFECTS`. King's Rock, the identical twin, is on 4 sets. |
| **Lansat Berry** | Buffed, held by **zero** sets | Rebuilt into a guaranteed-crit trigger (it borrows Laser Focus's volatile rather than a crit-stage boost that determinism made dead). |
| **Leppa Berry** | Quietly much stronger, held by **zero** sets | `DETERMINISTIC_ACCURACY_EVASION` turned PP into the *currency accuracy is paid in*. Restoring 10 PP is worth materially more here than in stock, and no set exploits it. |
| **Odd / Rock / Rose / Sea / Wave Incense** | Free scarcity relief | All five are `HOLD_EFFECT_TYPE_POWER`, so `BUFF_TYPE_BOOST_ITEMS` gives them the same **+40%** as Twisted Spoon, Hard Stone, Miracle Seed and Mystic Water. They are exact mechanical duplicates on an uncontested draft slot. Sea/Wave Incense in particular duplicate Mystic Water, the most-used type item (16 sets). |
| **Lax Incense** | Free scarcity relief | Under the PP economy, `HOLD_EFFECT_EVASION_UP` is a **flat +1 PP tax** on the attacker (`src/battle_util.c:12133`) — the item's own param is never read. Lax Incense and Bright Powder are therefore *identical*, and Bright Powder is on 1 set while Lax Incense is on none. |
| **14 resist berries** | Fine as-is | Babiri, Charti, Chilan, Coba, Haban, Kasib, Kebia, Occa, Payapa, Rindo, Roseli, Tanga, Wacan, Yache. Determinism does not touch them. Only Shuca, Passho, Colbur and Chople are used, one set each. Eighteen uncontested draft slots sitting idle. |
| **Wiki, Mago, Iapapa Berry** | Fine as-is | Mechanically identical to Figy (6 sets) and Aguav (3 sets) — the only difference is which nature dislikes the flavor. Pure arbitrary selection. |
| **Cheri, Pecha, Rawst, Aspear, Persim Berry** | Fine as-is | Narrower Lum Berries (Lum: 12 sets, Chesto: 2). Narrower is the point on an uncrowded slot. |
| **Liechi, Ganlon, Apicot, Starf Berry** | Fine as-is | Salac (1 set) and Petaya (2) are used; the other four are not. Starf was also made deterministic — it raises the holder's *currently highest* stat, not a random one. |
| **Absorb Bulb, Cell Battery, Snowball, Luminous Moss, Eject Button, Eject Pack, Red Card, Room Service, Adrenaline Orb, Ability Shield, Clear Amulet, Protective Pads, Utility Umbrella, Float Stone, Ring Target, Shed Shell, Sticky Barb, Binding Band, Lagging Tail, Metronome, Berserk Gene, Micle Berry, Enigma Berry, Jaboca, Rowap, Kee, Maranga** | Niche but functional | Ordinary situational items. Several are strong build-arounds nothing has been built around yet — Berserk Gene (+2 Attack on entry at the cost of confusion) and Metronome (+20% per consecutive use, to 2x) both define a set on their own. |

### Group B — dominated by our own buffs (needs code)

`BUFF_TYPE_BOOST_ITEMS` raised the generic type items from +20% to **+40%** for good
reasons, but it moved a goalpost several *other* item classes were standing on. These
are now strictly or near-strictly worse than a Charcoal, which is why nothing holds
them. This is regression collateral, and it is the most defensible buff work available.

| Item(s) | The problem | Sketch of a fix |
| --- | --- | --- |
| ~~**The 18 Gems**~~ — **shipped** (#510 balance, #511 roster) | A Gem was **+30%, once, then gone**; a type item is **+40%, every turn, forever**, so the Gem was a strictly worse Charcoal outside the Acrobatics/Unburden interaction. | Done both halves. `BUFF_GEMS` took the class to **+60%** (break-even against +40% is 1.5 uses, so it wins on a move clicked **once**), and the roster re-itemed the 7 sets where a Gem sat on a repeat-clicked move. Flying Gem is on the done list; six more sit on one set each and are tracked as thinly drafted; 11 types still want a first set. |
| ~~**Soul Dew**~~ — **shipped** (balance + roster) | +20% on Latios/Latias's Psychic and Dragon moves, while Dragon Fang gave them **+40%** on Dragon — the signature item lost to a generic one, and the roster proved it by giving Latios Dragon Fang. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. One number covers the one-type and two-type items alike because each boosts exactly its holder's STAB package. The roster moved **that same Dragon Fang set** onto Soul Dew — Calm Mind / Psyshock / Dragon Pulse splits its damage across both boosted types, which is the shape the item exists for. The gate also had to start reading the holder by **base** species, or a Soul Dew Lati@s that Mega Evolved under `FEATURE_FREE_GIMMICKS` silently lost its own item. |
| ~~**Adamant Orb, Lustrous Orb, Griseous Core**~~ — **shipped** | Same shape as Soul Dew: +20% on two types for one species, weakly dominated by a +40% type item everywhere. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. Each boosts exactly its holder's dual STAB (Dialga is Steel/Dragon, Palkia Water/Dragon, Giratina Ghost/Dragon). The Origin-forme versions share the hold effect, so they were covered by the same change. |
| ~~**The 17 Memories and 4 Drives**~~ — **shipped** (balance + roster) | A Memory or Drive set the holder's type and gave **no damage multiplier at all**, while Arceus's plate — the same idea for a different species — carried the full +40%. Silvally could not decline it either: `FORM_CHANGE_ITEM_HOLD` means dropping the Memory reverts the forme, so all four Silvally sets were compelled to hold a dead item. | Done. `BUFF_SIGNATURE_TYPE_ITEMS` fixed the balance and locked each item to its own species; the roster then drafted **all 17 Memories** (Silvally formes) and **all 4 Drives** (Genesect formes). The Drive sets run Techno Blast, which the Drive re-types — note it does **not** re-type Genesect, which stays Bug/Steel, so a Drive is +40% off-STAB where a Memory is +40% on top of STAB. |

### Group C — weak in stock, still weak here (needs code)

Not caused by us; just never worth a slot.

| Item(s) | The problem | Sketch of a fix |
| --- | --- | --- |
| **Oran Berry, Berry Juice** | Flat 20 HP at ≤1/2 HP. At the frontier's Level 50 that is roughly 10–13% of a typical HP pool, against Sitrus Berry's 25% — and Sitrus is on 105 sets. A flat number does not survive the jump to level 50. | `BUFF_FLAT_HP_ITEMS`: convert both to a fraction of max HP (Oran ~1/6, Berry Juice ~1/3, giving Berry Juice a real identity as the bigger, rarer Sitrus). Site: `HOLD_EFFECT_RESTORE_HP` in `src/battle_hold_effects.c`. |
| **Deep Sea Tooth, Deep Sea Scale** | Both are **2x** — enormous — but locked to Clamperl, which has no set at all (it evolves, so the coverage test excuses it). | No engine work. Needs a Clamperl set, which the coverage rules currently do not ask for. Worth one deliberately-built NFE entry: a 2x Sp. Atk Clamperl on an uncontested item is a genuinely interesting rental. |
| **Lucky Punch, Metal Powder, Quick Powder** | Species-locked to Chansey and Ditto, both of which have sets that hold something better (Chansey: Eviolite; Ditto: Choice Scarf). Lucky Punch was *also* repaired by `DETERMINISTIC_HOLD_EFFECTS` (guaranteed first-attack crit) and ships to nobody. | No engine work; a second Chansey and a second Ditto set. Chansey with Eviolite *and* a guaranteed crit are different sets, and the roster deliberately carries several builds per species. |

### Group D — the 17 Plates — **shipped**

All 17 plates are `HOLD_EFFECT_PLATE`, so `BUFF_TYPE_BOOST_ITEMS` gives them +40%, and
`BUFF_SIGNATURE_TYPE_ITEMS` then locked that boost to Arceus — which is what makes a plate
a signature item rather than a Charcoal clone. A plate also changes Arceus's type, so it is
a type item and a STAB rewrite in one slot. The roster now carries **all 17 plate formes**,
each on exactly one set, plus the two original Arceus-Normal sets (Life Orb, Leftovers),
which correctly hold no plate.

**This was never 17 uncontested draft slots**, and an earlier version of this doc called it
"the largest single win in the audit" on exactly that mistake. Three facts cap it, and they
are worth keeping here so the error is not repeated for some other species-locked class:

- **Arceus is `TIER_MYTHICAL`** (`src/fork/species_tiers.c`). `TierRejectsCandidate()` bans
  legendaries and mythicals from every normal slot — the player's rental pool
  (`src/battle_factory.c:625`) and ordinary Tower opponents (`src/battle_frontier.c:320`)
  alike. Arceus is reachable only through a *reserved forced-tier slot* on Factory
  opponents, plus fixed boss teams. **The player can never rent one fresh.**
- **The draft rejects duplicate species**, so one Arceus per team means **one plate per
  team, ever** — the 17 are mutually exclusive, not additive.
- **The draw is uniform over sets** (`GetRandomFrontierExtendedMonId()`), so set count
  scales appearance rate directly. Measured after the batch landed: **19 Arceus sets against
  an 84-set mythical pool, 22.6%.**

That share was reviewed and **accepted**, on the grounds that it is a *different* Arceus each
time — 17 formes sharing no typing, no moveset and no item — so the variety the audit cares
about is preserved even though one species' rate goes up. Recorded here as a deliberate call,
not free capacity.

The sets deliberately mix special and physical builds, because a plate boosts **any** move of
its type rather than only Judgment: Arceus-Dark runs Knock Off, Arceus-Fighting runs Close
Combat, and Arceus-Ground runs a Cosmic Power / High Horsepower stall build, all collecting
the same +40% a Judgment set gets.

## Priority

**The signature type items are finished** — balance and roster both. `BUFF_SIGNATURE_TYPE_ITEMS`
settled the class and the roster drafted every member: 17 Memories on Silvally formes, 17 Plates
on Arceus formes, 4 Drives on Genesect formes, and Soul Dew on Latios. Group B is empty.

What is left is Group A and Group C, and it is overwhelmingly **roster work**:

1. **Group A.** No engine risk, no new flag, no test surface — a line review that spends its item
   picks out of the tail instead of on Leftovers. The already-buffed-but-undrafted items (Wide
   Lens, Zoom Lens, Blunder Policy, Razor Fang, Lansat) are the most embarrassing subset: shipped
   work reaching no one. **This is now the top item in the backlog.**
2. **Group C.** `BUFF_FLAT_HP_ITEMS` for Oran Berry and Berry Juice is the only code left in the
   whole backlog; the rest is roster work gated on coverage decisions (a Clamperl set, a second
   Chansey and Ditto).

## Processing a batch

This section exists so "process the next batch of pending held items" is a complete
instruction. Work it top to bottom.

### 1. Read the current state

```bash
make check TESTS="Held item tracker"     # the five gates; green means the lists are honest
```

`test/fork/held_item_tracker.c` is the status. Read its three lists and the comment
blocks inside `sPendingItems[]` — they are grouped by *what kind of work is outstanding*:
**needs a buff**, **thinly drafted** (on one set), **needs a set** (on none). Re-measure
usage before trusting any count in this doc:

```bash
grep -oP '\.heldItem = \KITEM_\w+' src/fork/frontier_extended_mons.c | sort | uniq -c | sort -rn
```

### 2. Pick the batch

**One PR does one kind of work.** A buff PR ships one `BUFF_*` flag; a roster PR moves
items onto sets. Don't mix them — the Gems took two PRs on purpose (#510 balance, #511
roster), and that split is what let each be reviewed on its own merits.

Order of value: the **Priority** section above. In short — roster work first, since it
costs no engine risk, then the memories/drives extension, then the signature orbs.

A sensible batch is **one buff flag**, or **8–15 roster re-items**. Bigger roster batches
get hard to review; smaller ones waste a CI cycle.

### 3. Decisions already settled — do not re-litigate

- **A Gem is burst, not defense.** Making Gems grant type *immunity* was considered and
  rejected: it obsoletes Air Balloon outright and out-classes all 18 resist berries, both
  already pending. Fixing 18 items by breaking 19 is not a fix.
- **+60% was chosen deliberately.** A Gem is boost×one turn against a type item's
  boost×every turn, so the break-even is 1.5 uses. Bigger numbers start dominating the
  permanent items, which is the mistake the flag exists to undo.
- **A Gem goes on a move the set fires ONCE** — a self-debuffing nuke (Overheat, Leaf
  Storm, Make It Rain, Psycho Boost, Fleur Cannon), an Acrobatics set, or true coverage.
  Never a set's main STAB. **Contrary users are excluded**: Contrary turns the self-debuff
  into a boost, so they spam the nuke and want a permanent item.
- **Prefer re-iteming an existing set to appending a new one.** Saved rentals key on array
  index and the roster is kept in dex order, so a new set is a mid-list insertion that
  invalidates an in-progress rented team ([`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md),
  "Save compatibility"). Re-iteming shifts no index *and* pulls down the Leftovers / Life
  Orb concentration, which is the point of the audit. Append only when the roster genuinely
  needs a new build.
- **A signature type item is locked to its own species.** `BUFF_SIGNATURE_TYPE_ITEMS`
  settled this: a Plate boosts only Arceus, a Memory only Silvally, a Drive only Genesect.
  The ungated alternative — letting anyone hold a Bug Memory as a 40% Bug item — was
  considered and rejected: it hands the roster 21 more Charcoal-clones, which is a capacity
  change dressed as a fix, and it costs the three species the one thing that makes their
  item theirs. Consequence for set authors: these items have exactly one legal home each.
- **One number covers the one-type and two-type signature items.** Each boosts exactly its
  holder's STAB package — Arceus is mono-typed, Latios is Dragon/Psychic, Dialga
  Steel/Dragon — so "+40% on one type" and "+40% on two" are the same rule on different
  mons. There is no second magnitude to argue about.
- **One set is one set, whoever placed it.** A freshly, deliberately placed single set is
  still thinly drafted. The one exception is a **form-change enabler** (Adamant Crystal,
  Rusted Sword, an Ogerpon mask…), where one is the *ceiling* rather than a shortfall.

### 4. Graduate what you finished

An item joins `sDoneItems[]` only when **both** halves are true: balance is right **and**
the roster uses it on more than one set. Graduating is what arms the gates for it, so a
premature promotion is worse than leaving it pending. Bump `HELD_ITEM_DONE_FLOOR` by
however many you promoted — the ratchet is meant to make a demotion a visible, reviewed
edit rather than a quiet way to go green.

### 5. Verify and ship

```bash
make check TESTS="Held item tracker"        # always
make check TESTS="Frontier extended roster" # if you touched the roster
make check                                  # before pushing
UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j$(nproc) -O all   # if you touched engine code
```

Branch `claude/<short-description>`, PR against our `master`, squash merge. Update this
doc's reasoning and `FORK.md`'s balance row in the same PR — but **status goes in the
tracker, never here.**

## Adding one of these buffs

Follow the existing convention exactly — see the header comment in
[`config/buff.h`](../include/config/buff.h):

- `#define BUFF_X TRUE` in `config/buff.h`, with a comment that says what stock behavior
  was, what the buff does, **why the item needed it**, and where the effect site is.
- One line in `BUFF_CONFIG_DEFINITIONS` (`include/constants/config_changes.h`) so tests
  can flip it with `WITH_CONFIG`. The test baseline forces every `BUFF_*` flag off, so
  the inherited suite keeps exercising stock behavior.
- Magnitudes go in a plain compile-time constant next to the flag
  (`BUFF_X_PERCENT` / `BUFF_X_DENOMINATOR`), not in the registered flag.
- A dedicated `test/fork/buff_x.c` that opts in explicitly.
- A row in [`FORK.md`](FORK.md)'s balance table, one or two sentences.
