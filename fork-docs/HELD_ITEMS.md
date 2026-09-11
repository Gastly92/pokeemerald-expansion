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

**1596 sets. 102 distinct held items** (plus one deliberate `ITEM_NONE`, a Persian Thief
set). Against the **239 items in the build that have a battle-relevant hold effect**
(excluding Mega Stones and Z-Crystals — see "Never expected" below), that is **100 used,
139 unused**.

The distribution is heavily top-loaded:

| Item | Sets | Share |
| --- | --- | --- |
| Leftovers | 217 | 13.6% |
| Life Orb | 189 | 11.8% |
| Choice Band | 110 | 6.9% |
| Sitrus Berry | 105 | 6.6% |
| Rocky Helmet | 87 | 5.5% |
| Heavy-Duty Boots / Choice Specs | 71 each | 4.4% each |
| Assault Vest | 60 | 3.8% |
| Focus Band | 52 | 3.3% |
| Choice Scarf | 44 | 2.8% |

The top two alone are **25% of the roster**, and the tail is long and thin: **45 items
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
| **11 unused Gems** (Bug, Dark, Electric, Fire, Grass, Ground, Ice, Normal, Poison, Steel, Water) | A gem is **+30%, once, then gone** (`GEM_BOOST_PARAM`). A type item is **+40%, every turn, forever**. The gem is strictly worse on every axis except the Acrobatics/Unburden interaction. The seven used gems are on 1–4 sets each and are mostly there for that interaction. | `BUFF_GEMS`: make the one-shot a genuine nuke rather than a worse Charcoal — e.g. +80% or 2x — restoring the "spend it on the right turn" identity. The magnitude belongs in a `BUFF_GEM_PERCENT` constant, mirroring `BUFF_TYPE_BOOST_PERCENT`. Site: the `gemBoost` branch in `CalcDamage()`, `src/battle_util.c:7311`. |
| **Soul Dew** | +20% on Latios/Latias's Psychic and Dragon moves. Dragon Fang gives them **+40%** on Dragon, Twisted Spoon +40% on Psychic. The signature item loses to a generic one. Confirmed by the roster: Latios holds Dragon Fang and Choice Specs; Latias holds Leftovers, Boots and Light Clay. | Fold the signature two-type items into the buffed scale — a `BUFF_SIGNATURE_TYPE_ITEMS` at, say, +50% on both types would beat a type item on a split-damage set and lose on a mono-attacker, which is the identity they are supposed to have. Site: the `HOLD_EFFECT_SOUL_DEW` case in `CalcDamage()`, `src/battle_util.c:7647`. |
| **Adamant Orb, Lustrous Orb, Griseous Core** | Same shape: +20% on two types for one species. A type item at +40% on one type ties them on a perfectly split set and beats them on any concentrated one, so they are *weakly dominated everywhere*. The roster uses the Origin-forme versions (Adamant Crystal, Lustrous Globe, Griseous Orb) — but for the **form change**, not the boost. | Same flag as Soul Dew. Sites: `src/battle_util.c:7635`–`7645`. |
| **13 unused Memories** and **all 4 Drives** | Silvally's memory and Genesect's drive change the holder's type / signature-move type and give **no damage multiplier at all** — `HOLD_EFFECT_MEMORY` and `HOLD_EFFECT_DRIVE` have no case in `CalcDamage()`. Arceus's plate, which is the same idea for a different species, gets the full +40%. That asymmetry is now much wider than upstream intended it to be. `buff.h` names this exclusion explicitly ("Silvally's memories and Genesect's drives are a different hold effect and are untouched"), so it is a known, deliberate gap — not an oversight to be surprised by. | Extend `BUFF_TYPE_BOOST_ITEMS` (or a sibling flag) to give `HOLD_EFFECT_MEMORY` and `HOLD_EFFECT_DRIVE` the same `BUFF_TYPE_BOOST_PERCENT` on the type they set. Cheap: two extra labels on the existing `HOLD_EFFECT_TYPE_POWER` / `HOLD_EFFECT_PLATE` case. Note the Rusted Sword/Shield and the Ogerpon masks sit in the same family and should be checked for consistency at the same time. |

### Group C — weak in stock, still weak here (needs code)

Not caused by us; just never worth a slot.

| Item(s) | The problem | Sketch of a fix |
| --- | --- | --- |
| **Oran Berry, Berry Juice** | Flat 20 HP at ≤1/2 HP. At the frontier's Level 50 that is roughly 10–13% of a typical HP pool, against Sitrus Berry's 25% — and Sitrus is on 105 sets. A flat number does not survive the jump to level 50. | `BUFF_FLAT_HP_ITEMS`: convert both to a fraction of max HP (Oran ~1/6, Berry Juice ~1/3, giving Berry Juice a real identity as the bigger, rarer Sitrus). Site: `HOLD_EFFECT_RESTORE_HP` in `src/battle_hold_effects.c`. |
| **Deep Sea Tooth, Deep Sea Scale** | Both are **2x** — enormous — but locked to Clamperl, which has no set at all (it evolves, so the coverage test excuses it). | No engine work. Needs a Clamperl set, which the coverage rules currently do not ask for. Worth one deliberately-built NFE entry: a 2x Sp. Atk Clamperl on an uncontested item is a genuinely interesting rental. |
| **Lucky Punch, Metal Powder, Quick Powder** | Species-locked to Chansey and Ditto, both of which have sets that hold something better (Chansey: Eviolite; Ditto: Choice Scarf). Lucky Punch was *also* repaired by `DETERMINISTIC_HOLD_EFFECTS` (guaranteed first-attack crit) and ships to nobody. | No engine work; a second Chansey and a second Ditto set. Chansey with Eviolite *and* a guaranteed crit are different sets, and the roster deliberately carries several builds per species. |

### Group D — the 17 Plates

Worth its own heading because it is the single largest fully-unused group with a
**shipped buff attached**.

All 17 plates are `HOLD_EFFECT_PLATE`, so `BUFF_TYPE_BOOST_ITEMS` already gives them
+40% — *and* a plate changes Arceus's type, so it is a type item and a STAB rewrite in
one slot. The roster's two Arceus sets hold Life Orb and Leftovers, so the entire class
is idle and Arceus only ever appears as Normal.

There is no code to write. Arceus with a plate is 17 distinct, mutually exclusive,
completely uncontested draft slots, and it is the most flavor-accurate thing Arceus can
do. This is the largest single win in the audit and it is pure roster work.

## Priority

1. **Group A and Group D first.** No engine risk, no new flag, no test surface — a line
   review that spends its item picks out of the tail instead of on Leftovers. The
   already-buffed-but-undrafted items (Wide Lens, Zoom Lens, Blunder Policy, Razor Fang,
   Lansat) are the most embarrassing subset: shipped work reaching no one.
2. **Group B's memories/drives extension.** Smallest code change with the clearest
   justification — it closes an asymmetry our own buff opened.
3. **Group B's gems and signature orbs.** Needs a magnitude decision, so it wants the
   same treatment `BUFF_TYPE_BOOST_PERCENT` got: a registered toggle plus a plain
   compile-time constant.
4. **Group C.** Lowest value; the flat-HP items are a small fix and the rest is roster
   work gated on coverage decisions.

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
