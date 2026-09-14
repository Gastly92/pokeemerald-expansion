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
| `sDoneItems[]` | Balance is right **and** the roster uses it | Yes — at least one set holds it, **more than one** unless it is a form-change enabler, and it stays under `HELD_ITEM_MAX_ROSTER_SHARE_PERCENT` (20%) of the roster |
| `sPendingItems[]` | Work outstanding: needs a buff, is mechanically fine and needs a set, or is **thinly drafted** (on one set, which with one-item-per-team is a roll away from never appearing) | No — a pending item may sit at zero sets, which is usually why it is pending |
| `sIgnoredItems[]` | Nothing it does is reachable in a frontier battle — the effect can't happen here, **or** its only legal holder can't use the effect it has | Exempt from the done gates, but **no set may hold one** — a Mega Stone or Z-Crystal in the slot does nothing under `FEATURE_FREE_GIMMICKS`, so the set would be playing an item down with no other symptom |

A **one-set count is not automatically a shortfall.** The form-change enablers — Adamant
Crystal, Lustrous Globe, Griseous Core, Red/Blue Orb, Rusted Sword and Shield, the three
Ogerpon masks — each unlock exactly one forme on exactly one species, so one is the
ceiling rather than a gap. They stay done so the zero-set gate keeps watching them: delete
the Giratina-Origin set and CI should notice.

The single-set gate derives that exemption from the form change tables rather than a list,
so it stays correct on its own — but note it is **config-dependent**, which is what caught
Griseous Orb. The Orb only changes Giratina's forme when `I_GRISEOUS_ORB_FORM_CHANGE <
GEN_9`, and this build is at `GEN_LATEST`, so that row is compiled out of
`sGiratinaFormChangeTable` and **Griseous Core** is the enabler here. That makes the Orb an
ordinary signature type item, class-mates with Adamant Orb and Lustrous Orb (both pending at
zero), and it is now pending too. The Giratina-Origin set is unaffected — it names
`SPECIES_GIRATINA_ORIGIN` directly, so the item was never what got it there.

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

### The gate the tracker cannot be: is the item *live* on the set holding it?

Every tracker gate asks whether an item is **on** a set. None can ask whether it **does
anything there**, and a conditional item on the wrong holder passes all six while being a
wasted slot — the same "we buffed it and shipped it to nobody" failure the tracker exists to
catch, in the one form it cannot see. That check lives in the roster suite instead, as
`Frontier extended roster: no set holds an item none of its moves can activate`, and it now
covers four classes rather than the one (Throat Spray) it started with:

| Class | The item is dead when… |
| --- | --- |
| Type-boost items, plates, Drives, Memories | the set carries no damaging move of the item's own type |
| Gems | same, **after** an -ate ability is applied — Galvanize turns Explosion Electric, so a Normal Gem on that holder never fires |
| Resist berries | the holder is immune to the type (by typing *or* ability/innate), or simply **not weak** to it — the berry only fires on a super-effective hit, Chilan excepted |
| Throat Spray | the set carries no sound move |

Two deliberate limits keep it from crying wolf. Judgment, Techno Blast and Multi-Attack are
`EFFECT_CHANGE_TYPE_ON_ITEM` — they *become* the held item's type, so they always satisfy it.
And a set carrying a move whose type is only known at battle time (Weather Ball, Terrain
Pulse, Hidden Power, Natural Gift, Revelation Dance, Tera Blast, Raging Bull, Ivy Cudgel) is
**skipped rather than guessed at**: a gate that false-positives is worse than one with a hole.

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

**1629 sets. 176 distinct held items** (plus one deliberate `ITEM_NONE`, a Persian Thief
set). The tracker classifies **241** items that do something when held, of which **20** do nothing
reachable in a frontier battle (`sIgnoredItems[]` — see "Never expected" below), leaving a live
universe of **221**. Against that, the roster is at **176 used, 45 unused**.

The distribution is heavily top-loaded:

| Item | Sets | Share |
| --- | --- | --- |
| Leftovers | 182 | 11.2% |
| Life Orb | 154 | 9.5% |
| Choice Band | 100 | 6.1% |
| Sitrus Berry | 98 | 6.0% |
| Rocky Helmet | 74 | 4.5% |
| Heavy-Duty Boots | 71 | 4.4% |
| Choice Specs | 69 | 4.2% |
| Assault Vest | 60 | 3.7% |
| Focus Band | 52 | 3.2% |
| Choice Scarf | 44 | 2.7% |

The top two alone are **21% of the roster**, and the tail is long and thin: **86 items
appear on one or two sets** — most of them the species-locked signature items, where one
set is the ceiling rather than a gap.

### Why the concentration matters more than it looks

The Factory draft rejects any candidate whose `heldItem` already appears on the team
being built (`src/battle_frontier.c:344`, "Ensure this Pokemon's held item isn't a
duplicate" — a non-`ITEM_NONE` match makes the draft skip that mon and roll again).
**Only one of each item can appear per team**, for the player's rentals and for each
opponent alike.

So item concentration is a *draft-rate tax*: the 213 Leftovers sets are competing for a
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
| Species-locked but inert | 3 | **Lucky Punch** (Chansey only) is repaired twice over — +2 crit stage, which `DETERMINISTIC_HOLD_EFFECTS` upgrades to a guaranteed first-attack crit — but Chansey has **5 base Attack** and attacks with Seismic Toss, whose fixed damage a crit does not scale, so the crit lands and changes nothing. **Metal Powder / Quick Powder** (Ditto only) both gate on an *untransformed* Ditto, and the roster's Ditto runs Imposter, which transforms on switch-in. The effect is real in each case; the one holder allowed to have it cannot use it. |
| Out-of-battle utility | 17 | Exp. Share, Lucky Egg, Amulet Coin, Luck Incense, Soothe Bell, Cleanse Tag, Pure Incense, Smoke Ball, Everstone, Destiny Knot, Macho Brace and the six Power items. Nothing they do is reachable in a frontier battle (the Power items' Speed halving is reachable, but a Trick Room set gets the same result for free with `IVS(SPE, 0)`). |

That leaves **45 unused items that are live in battle** — the actual backlog (two of them the
parked Clamperl pair, below).

## The backlog

Four groups, in the order they are worth working on.

### Group A — already fixed, just not drafted (no code, roster work)

These items are good, or were *specifically repaired by this fork*. Zero engine work; they
need a line review to pick them up. This is the highest-value group precisely because it
costs nothing. The first five rows — the six fork-repaired items — have now shipped; the
rest of the group is still open.

| Item(s) | Status | Note |
| --- | --- | --- |
| ~~**Wide Lens, Zoom Lens**~~ — **shipped** | Buffed twice, now **2 sets each** | `BUFF_ACCURACY_ITEMS` gave both a job in the PP economy and `BUFF_ACCURACY_ITEMS_REVEAL` made them INFO-viewer instruments. The reveals only work for the **player**, so both went on rentable (`TIER_NORMAL`) sets: **Wide Lens** on Watchog and Uxie, breadth on two scout leads, and Uxie's innate Forewarn already peeks at one foe's strongest move, so the lens extends an instinct the species has; **Zoom Lens** on Forretress and Sudowoodo, both of which move second as a matter of course (Forretress is `SPE_DOWN` with a Gyro Ball, Sudowoodo has 30 base Speed) — that is the only window Zoom Lens's relief opens in. Watchog's innate Keen Eye/Illuminate is *not* a duplicate of its own item: those cancel the evasion **stage** half (`GetAccEvasionStageDelta`), while a lens cancels the **flat** taxes (`GetDeterministicMoveTargetPPTax`, which only the lenses and No Guard clear), so the two halves compose into a fully evasion-proof attacker. |
| ~~**Blunder Policy**~~ — **shipped** | Rebuilt, now **2 sets** | `DETERMINISTIC_HOLD_EFFECTS` re-armed it on the deterministic blunders (Protect, semi-invulnerability, Wide/Quick/Crafty Guard, Psychic Terrain, a type immunity, a blocking ability, an Air Balloon). In doubles any one avoiding target arms it. Placed on two sets whose **own STAB carries a hard immunity**, so the trigger is a matter of course rather than luck: Diggersby (Return and Quick Attack blanked by Ghost, High Horsepower by Flying/Levitate — and it is a Swords Dance set at 78 Speed, so the blundered turn becomes the sweep window) and Brambleghast (Poltergeist and Shadow Sneak blanked by Normal, and its Choice Band was locking a set that carries Rapid Spin). Note the doubles spread-move idea does **not** apply here: the roster deliberately runs no Earthquake on a doubles set, since it hits the ally. |
| ~~**Razor Fang**~~ — **shipped** | Buffed, now **2 sets** | The flinch items became guaranteed one-shots under `DETERMINISTIC_HOLD_EFFECTS`. King's Rock, the identical twin, is on 4 sets, so this was pure scarcity relief on a proven shape. Both homes are fast physical attackers, which is what converts a one-shot flinch into a free turn: Barraskewda (136 Speed, the fastest physical attacker in the rentable pool — and its Choice Band was locking a set that carries Flip Turn) and Lycanroc-Dusk, where the free turn is a Swords Dance. |
| ~~**Lansat Berry**~~ — **shipped** | Buffed, now **2 sets** | Rebuilt into a guaranteed-crit trigger (it borrows Laser Focus's volatile rather than a crit-stage boost that determinism made dead). Needs a holder that actually *reaches* the threshold, so both homes self-chip: Honchkrow (Brave Bird recoil on a 52/52 defensive frame — and its innate **Super Luck** is exactly the crit-stage ability determinism killed, so the berry is that ability's repair) and Emboar (Flare Blitz *and* Wild Charge, innate Reckless, no Rock Head to cancel the recoil). Emboar's innate **Gluttony** also moves the trigger from 1/4 to 1/2 max HP (`HasEnoughHpToEatBerry`), making it the more reliable of the two. |
| ~~**Leppa Berry**~~ — **shipped** | Quietly much stronger, now **2 sets** | `DETERMINISTIC_ACCURACY_EVASION` turned PP into the *currency accuracy is paid in*, and it also **scales max PP down by accuracy**, so the sets that feel it are the ones whose moves are already short. Leppa restores a move that hits 0 PP, so it went to the two lowest-total-PP rentable sets in the roster: Lurantis (17 effective PP across four moves — Focus Blast is scaled to **3**) and Camerupt (24, with Eruption and Fire Blast at 5 and 4). Camerupt gains twice over, since Life Orb chip was working against Eruption's HP scaling. |
| ~~**Odd / Rock / Rose / Sea / Wave Incense**~~ — **shipped** | Drafted, **2 sets each** | All five are `HOLD_EFFECT_TYPE_POWER`, so `BUFF_TYPE_BOOST_ITEMS` already gave them the same **+40%** as Twisted Spoon, Hard Stone, Miracle Seed and Mystic Water — pure placement, no balance question. Each went to a set whose damage is genuinely concentrated in that type, off a crowded item: **Odd** on Munkidori and Tapu Lele (Psychic Surge makes Expanding Force the boosted move), **Rock** on Relicanth and Rampardos — both innate **Rock Head**, so Head Smash is 150 BP with no recoil and the incense boosts it and Rock Slide — **Rose** on Serperior (innate-free **Contrary**, which spams Leaf Storm, exactly the repeat-clicked move a permanent type item wants and a Gem does not) and Sunflora (Choice Specs was locking it into a SpA-dropping Leaf Storm), and **Sea/Wave** on Floatzel, Crawdaunt, Basculegion and Tentacruel. |
| ~~**Lax Incense**~~ — **shipped** | Drafted, **2 sets** | Byte-identical to Bright Powder under the PP economy: `HOLD_EFFECT_EVASION_UP` is a flat `tax++` on the attacker (`src/battle_util.c`) and the item's own param is never read. Both homes are bulky doubles redirectors that already plan to be attacked repeatedly, so the tax compounds — Clefable (Follow Me + Moonlight) and Tangrowth (Rage Powder + Giga Drain). Unlike the Rocky Helmet each gave up, the tax also applies to non-contact and special attackers. |
| **Lax Incense** | Free scarcity relief | Under the PP economy, `HOLD_EFFECT_EVASION_UP` is a **flat +1 PP tax** on the attacker (`src/battle_util.c:12133`) — the item's own param is never read. Lax Incense and Bright Powder are therefore *identical*, and Bright Powder is on 1 set while Lax Incense is on none. |
| ~~**The 18 resist berries**~~ — **shipped** | Drafted, **2 sets each** | Determinism does not touch them, so the whole class was placement only. The rule that made 16 of the 18 mechanical: **put the berry on a genuine 4x weakness**, which it halves back to 2x, since the berry only fires on a super-effective hit (`GetDefenderItemsModifier`). Two screens are mandatory and both drew blood — an ability *or innate* granting immunity to the berry's own type makes it permanently dead (Scizor and Ferrothorn lost their Occa Berry to Well-Baked Body; a Ghost type would have killed Chilan the same way), and the old item may be the point of the set. Two fork innates cut the other way and produced the best placements: **Ripen** doubles the reduction to 0.25x, so the Grass/Dragon apple line takes a 4x Ice hit at *neutral* damage, and **Harvest** recycles the berry outright, which is why Exeggutor took the Tanga. |
| **Haban and Chilan: the two that break the rule** | Placed on their own terms | Recorded because a future audit will re-derive it otherwise. **Haban** (Dragon) has **no** 4x home anywhere — nothing in the game is 4x weak to Dragon — so a 2x Dragon-type holder is its ceiling (Goodra, Kingdra). **Chilan** (Normal) is the mirror: no type is weak to Normal either, but its trigger is special-cased to fire on *any* Normal hit (`moveType == TYPE_NORMAL`), so it wants a holder that merely **expects** Normal damage. Blissey is the case it could have been written for — 255 base HP behind **10 base Defense**, against a roster carrying 126 physical-Normal move instances. |
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
| ~~**The 18 Gems**~~ — **shipped** (#510 balance, #511 roster) | A Gem was **+30%, once, then gone**; a type item is **+40%, every turn, forever**, so the Gem was a strictly worse Charcoal outside the Acrobatics/Unburden interaction. | Done both halves, and the roster half is now complete: **all 18 types carry at least two sets**. `BUFF_GEMS` took the class to **+60%** (break-even against +40% is 1.5 uses, so it wins on a move clicked **once**), and placement followed the settled rule — a self-debuffing nuke (Overheat, Leaf Storm, Draco Meteor, Psycho Boost, Fleur Cannon), a literal one-use move (**Explosion**, the purest case), or true **off-STAB coverage**, which is what gives the eleven types with no nuke of their own a home. One screen is specific to this class: an **-ate ability re-types the move out from under the Gem**. Golem-Alola was the obvious Explosion home for the Normal Gem and its Galvanize turns Explosion Electric, so the Gem would never have fired; plain Golem took it instead. |
| ~~**Soul Dew**~~ — **shipped** (balance + roster) | +20% on Latios/Latias's Psychic and Dragon moves, while Dragon Fang gave them **+40%** on Dragon — the signature item lost to a generic one, and the roster proved it by giving Latios Dragon Fang. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. One number covers the one-type and two-type items alike because each boosts exactly its holder's STAB package. The roster moved **that same Dragon Fang set** onto Soul Dew — Calm Mind / Psyshock / Dragon Pulse splits its damage across both boosted types, which is the shape the item exists for. The gate also had to start reading the holder by **base** species, or a Soul Dew Lati@s that Mega Evolved under `FEATURE_FREE_GIMMICKS` silently lost its own item. **Collateral, fixed later:** that Latios set was Dragon Fang's *second* home, so re-iteming it quietly left Dragon Fang on one set while it sat on the done list. It has since been demoted to pending. |
| ~~**Adamant Orb, Lustrous Orb, Griseous Core**~~ — **shipped** | Same shape as Soul Dew: +20% on two types for one species, weakly dominated by a +40% type item everywhere. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. Each boosts exactly its holder's dual STAB (Dialga is Steel/Dragon, Palkia Water/Dragon, Giratina Ghost/Dragon). The Origin-forme versions share the hold effect, so they were covered by the same change. |
| ~~**The 17 Memories and 4 Drives**~~ — **shipped** (balance + roster) | A Memory or Drive set the holder's type and gave **no damage multiplier at all**, while Arceus's plate — the same idea for a different species — carried the full +40%. Silvally looked compelled to hold it too, though that turned out to be untrue here — see the note below on forme reversion. | Done. `BUFF_SIGNATURE_TYPE_ITEMS` fixed the balance and locked each item to its own species; the roster then drafted **all 17 Memories** (Silvally formes) and **all 4 Drives** (Genesect formes). The Drive sets run Techno Blast, which the Drive re-types — note it does **not** re-type Genesect, which stays Bug/Steel, so a Drive is +40% off-STAB where a Memory is +40% on top of STAB. |

### Group C — weak in stock, still weak here — **shipped**

Not caused by us; just never worth a slot. **This group is now empty, and with it the whole
balance half of the backlog**: every remaining pending item is a placement problem.

| Item(s) | The problem | Fix |
| --- | --- | --- |
| ~~**Oran Berry, Berry Juice**~~ — **shipped** | Flat 10 and 20 HP at ≤1/2 HP. At the frontier's Level 50 that is roughly 5–7% and 10–13% of a typical HP pool, against Sitrus Berry's 25% in the same slot — a flat number does not survive the jump to Level 50, and the roster held neither on a single set. | `BUFF_FLAT_HP_ITEMS` heals `maxHP/4` for both, **matching Sitrus exactly**. Duplication is the goal, not a fallback: one item per team means Sitrus's **103 sets** all compete for one slot, so two more items healing the same amount are two more uncontested slots those sets can move onto — the reasoning that put Sea/Wave Incense on the roster as Mystic Water clones. A **bigger** Berry Juice was considered and rejected: the drawback that would pay for it (Ripen cannot double it, Harvest cannot regrow it) reaches **2.3%** of species, so for the other 97.7% it would just be a strictly better Sitrus. At an equal number it keeps an identity for free by **not being a Berry** — innate Unnerve (44 species) blocks Sitrus but not Berry Juice, while Ripen/Harvest (34) amplify Sitrus but not Berry Juice. Same power, opposite matchups. Keyed on the two **items**, not on `HOLD_EFFECT_RESTORE_HP`, because Sitrus shares that hold effect whenever `I_SITRUS_BERRY_HEAL < GEN_4`. Site: `ItemHealHp()` in `src/battle_hold_effects.c`; `test/fork/buff_flat_hp_items.c`. |

#### Parked: the item works, the holder costs too much

| Item(s) | Why it is parked |
| --- | --- |
| **Deep Sea Tooth, Deep Sea Scale** | Both are **2x**, genuinely enormous, and locked to Clamperl, which has no set (it evolves, so the coverage test excuses it). The price is what stalls them, not the item: graduation takes two sets each, so drafting both means **four Clamperl entries** — a 35/64/85/74/55/32 NFE, four times, in a uniform draw — while Huntail and Gorebyss already carry four sets between them. They stay **pending**, not ignored, because nothing about them is dead: a line review that wants an NFE gimmick can pick them up without any code or list change. Just don't count them when sizing a batch. |

| **Adamant Orb, Lustrous Orb, Griseous Core** | Species-locked by `BUFF_SIGNATURE_TYPE_ITEMS`, and the arithmetic does not close. Graduation needs **two sets**, but Adamant Orb's only legal holders are **three** sets — two Dialga plus Dialga-Origin, which must keep its Adamant Crystal — so the only way to reach two is to put the orb on **both** Dialga sets. Under one-species-per-team those are mutually exclusive, so the second set adds **zero reach** while stripping Dialga of its Leftovers and Choice Specs. Lustrous Orb and Griseous Core have four legal sets each and the same shape. The item is fine; the holder pool is too small to graduate it honestly. Unparking needs *new* Dialga/Palkia/Giratina sets, not a re-item. |
| **Ring Target** | The only item in the backlog whose effect is a **pure drawback to its holder**: it turns the holder's type *immunities* into neutral damage (`MulByTypeEffectiveness`, `src/battle_util.c`). There is no upside term — it exists in the retail games to be handed to an opponent via Trick or Fling, which no set here does. Drafting it means deliberately making a set worse. |

The other three species-locked strays — **Lucky Punch**, **Metal Powder** and **Quick
Powder** — are not parked but *ignored*, because their effects genuinely cannot land on
the only holder allowed to have them. See
[Never expected](#never-expected--the-structural-exclusions).

#### A forme set is NOT compelled to hold its item

Worth stating plainly, because an earlier version of this doc asserted the opposite and it
would needlessly constrain **46 roster sets**. `FORM_CHANGE_ITEM_HOLD` — the method behind the
Arceus plates, Silvally's Memories, Genesect's Drives, Giratina-Origin and the two Origin
orbs — is **never invoked from a battle or frontier path**. Its only callers are the party
menu, the PC and `givemon` (`TryFormChange(mon, FORM_CHANGE_ITEM_HOLD, …)`), all of which are
overworld actions on the player's own party.

Battle setup calls `FORM_CHANGE_BEGIN_BATTLE` instead (`src/battle_main.c`), which is a
*different* method — and that is exactly why Zacian and Zamazenta genuinely do need their
Rusted Sword and Shield, while an item-hold forme does not.

A frontier set naming `SPECIES_GIRATINA_ORIGIN` therefore **stays** Giratina-Origin no matter
what it holds. Two sets already rely on this and are correct: **Palkia-Origin holds Mystic
Water** and **Giratina-Origin holds Choice Specs**. So a forme set's item slot is free, and a
line review may spend it on anything.

The Memories and Drives are still the right items for their holders — they are those species'
signature type boost under `BUFF_SIGNATURE_TYPE_ITEMS` — but they are chosen, not forced.

#### Measured and NOT parked: the items that only look like drawbacks

Five items read as unusable and are not, so the question does not need reopening. Each was
measured against the roster rather than judged by feel:

| Item(s) | Why it is draftable |
| --- | --- |
| **Adrenaline Orb** | Triggers off the **foe's** Intimidate, which looks dead because **zero** sets choose Intimidate as their ability — but **127 sets** (7.8%) are species carrying it as an *innate*, and innates are always on. The most live of the five. |
| **Room Service** | Needs Trick Room up; the roster sets Trick Room on **49 sets**. |
| **Binding Band** | Boosts the holder's *own* binding move, which is a benefit, not a cost. **10 sets** carry one (Infestation, Magma Storm, Whirlpool, Wrap, Bind, Clamp, Snap Trap). |
| **Lagging Tail, Full Incense** | An exact duplicate pair — both are `HOLD_EFFECT_LAGGING_TAIL`. Moving last is a benefit for **Avalanche, Counter and Mirror Coat**, which is **10 sets**. Note it is *not* a Trick Room synergy: Trick Room already inverts the speed order, and the item forces the holder last regardless, so the two work against each other. |
| **Sticky Barb** | Chip on the holder, but it **transfers to an attacker on contact**, so it is a trade rather than a pure cost. |

#### Assessed and rejected as buff candidates

`held_item_tracker.c`'s thin-list comment used to name **Safety Goggles, Power Herb, Mirror
Herb and Custap Berry** as wanting a buff. That was an aside in the tracker's first commit
(#509), written before the Gems and the signature items and never revisited — and it is
**wrong on all four**. Recorded here so it is not re-proposed; each needs a second set, not a
number.

| Item | The claim | Why it fails |
| --- | --- | --- |
| **Safety Goggles** | Underpowered | Backwards — the fork already buffed it, hard. `DETERMINISTIC_ACCURACY_EVASION` makes Sleep Powder (75% in stock) **never miss**, and Goggles is the hard immunity to it. The roster holds **59** powder/spore instances (24 Sleep Powder, 16 Spore, 16 Rage Powder, 3 others) plus **52** sand-chip sources. Note the 45 Snow Warning sets do *not* count: only `BATTLE_WEATHER_HAIL` chips, `BATTLE_WEATHER_SNOW` falls through (`src/battle_end_turn.c`). |
| **Power Herb** | Underpowered | Narrow, not weak. Of 27 charge-move instances, 16 are Solar Beam (free in sun anyway) and 6 are semi-invulnerable moves where skipping the charge **discards the invulnerability** — the Power Herb branch clears `semiInvulnerable = STATE_NONE` (`src/battle_move_resolution.c`), so it fires the move immediately and throws away the turn the move was taken for. Power Herb is actively wrong there. The real surface is Geomancy, Electro Shot, Solar Blade and off-sun Solar Beam, and on Geomancy the skipped turn is worth +2/+2/+2. Few legal homes, correct price. |
| **Mirror Herb** | Underpowered | Reactive, but well fed: **365** setup-move instances across the roster, so roughly a fifth of sets carry something for it to copy. |
| **Custap Berry** | Underpowered | Rests on a misreading. Custap sits in the *same `if`* as Quick Claw (`src/battle_main.c`), and `DETERMINISTIC_HOLD_EFFECTS` rewrote only the Quick Claw half — but correctly: Quick Claw was a **random roll**, Custap was **already** a plain HP threshold (`HasEnoughHpToEatBerry`, fraction 4). Nothing to repair. Innate **Gluttony** also moves it to 1/2 max HP for free, the same lever that earned Emboar its Lansat Berry. |

The general lesson, and the reason this is written down: **determinism silently re-prices items
nobody touched.** An item whose job is to block a move that used to miss got stronger without
a line of code. Re-measure against the current flags before calling anything underpowered.

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
   picks out of the tail instead of on Leftovers. The embarrassing subset is now closed: Wide Lens,
   Zoom Lens, Blunder Policy, Razor Fang, Lansat **and** Leppa each carry two sets and are gated,
   so no fork-repaired item ships to nobody any more. What is left in the group is pure uncontested
   capacity, and it is still the top of the backlog:
   - ~~**The six incenses.**~~ **Shipped** — two sets each, no balance question, no engine work.
   - ~~**The resist berries.**~~ **Shipped** — all 18 carry two sets each.
   - ~~**The Gems.**~~ **Shipped** — all 18 types carry two or more sets.
2. ~~**Group C.**~~ **Shipped.** `BUFF_FLAT_HP_ITEMS` was the last engine work in the backlog.
   **There is no balance work left** — every pending item now needs a set, not a flag. Oran and
   Berry Juice are themselves the next obvious roster picks, since both are still on zero sets.

**The five species-locked strays are not backlog.** Lucky Punch, Metal Powder and Quick Powder
are on `sIgnoredItems[]` — the effect is real but the only legal holder cannot use it — and the
Clamperl pair (Deep Sea Tooth/Scale) is parked in pending at a price of four NFE sets. Don't
count any of them when sizing what is left.

## Processing a batch

This section exists so "process the next batch of pending held items" is a complete
instruction. Work it top to bottom.

### 1. Read the current state

```bash
make check TESTS="Held item tracker"     # the six gates; green means the lists are honest
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

**How much is left** (re-measure, don't trust this after a line review): 45 pending items, **all
of them at zero sets** — the thinly-drafted middle is now empty. Six of the 45 are **parked** and
should not be counted: the Clamperl pair, the three signature orbs whose holder pool is too small
to graduate them, and Ring Target. That leaves **39 items × 2 = 78 set-placements** of real work,
and no engine work at all. At 8–15 a
batch that is **roughly 6 batches**, and after `BUFF_FLAT_HP_ITEMS` shipped **all of it is
roster work** — there is no engine work left in the backlog at all.

What is left is the **long situational tail** — Absorb Bulb, Cell Battery, Eject Button, Red
Card, Room Service, Berserk Gene, Metronome, the Wiki/Mago/Iapapa berries, the narrow status
berries, and the rest of Group A's last row. No class rule covers these; each wants a set
built around what it actually does, so they are best taken in small themed groups.

### 3. Decisions already settled — do not re-litigate

- **A Gem is burst, not defense.** Making Gems grant type *immunity* was considered and
  rejected: it obsoletes Air Balloon outright and out-classes all 18 resist berries, both
  already pending. Fixing 18 items by breaking 19 is not a fix.
- **+60% was chosen deliberately.** A Gem is boost×one turn against a type item's
  boost×every turn, so the break-even is 1.5 uses. Bigger numbers start dominating the
  permanent items, which is the mistake the flag exists to undo.
- **An -ate ability re-types the move out from under a Gem.** Galvanize, Pixilate,
  Refrigerate and Aerilate all convert the holder's **Normal** moves to another type, and
  Normalize converts everything *to* Normal — so a Gem keyed on the move's printed type
  silently never fires. Check them on innates as well as the chosen ability. This cost
  Golem-Alola the Normal Gem: Explosion is the purest fire-once move in the game and its
  Galvanize makes it Electric, so plain Golem took the set instead. Same family of mistake
  as a resist berry on a holder immune to the berry's type — the item is dead in the slot and
  nothing downstream notices.
- **A Gem goes on a move the set fires ONCE** — a self-debuffing nuke (Overheat, Leaf
  Storm, Make It Rain, Psycho Boost, Fleur Cannon), an Acrobatics set, or true coverage.
  Never a set's main STAB. **Contrary users are excluded**: Contrary turns the self-debuff
  into a boost, so they spam the nuke and want a permanent item.
- **A terrain seed wants a holder that sets its own terrain.** Electric Seed and Psychic Seed
  only fire while the matching terrain is up, and the frontier draft is random, so a holder
  that depends on drawing a partner surge-setter is a holder the item usually does nothing
  for. Both of this fork's seed sets are on species that set the terrain themselves — Tapu
  Koko and Pincurchin for Electric, Tapu Lele and Galarian Articuno for Psychic — which makes
  the seed fire on switch-in every time.
- **Power Herb is dead on a sun team.** Solar Beam already skips its charge turn in sun, so a
  Power Herb next to Drought, Sunny Day or a Heat Rock buys nothing. Every Solar Beam set in
  this roster is a sun set, which is why Power Herb's homes are Geomancy (Xerneas) and
  Electro Shot (Archaludon) instead.
- **A defensive item must be able to fire at all.** Before placing a type-conditional item,
  check the holder is not *immune* to the type it keys on — by its chosen ability **or by an
  innate**, since innates are always on. An immune holder can never take the hit the item
  resists, so the item is permanently dead in the slot and the tracker will happily record it
  as drafted. This is not hypothetical: it cost **Scizor and Ferrothorn** their Occa Berry
  (both run Well-Baked Body, a Fire immunity) in the first resist-berry batch. The abilities
  worth screening are Flash Fire and Well-Baked Body (Fire), Water Absorb / Storm Drain /
  Dry Skin (Water), Volt Absorb / Lightning Rod / Motor Drive (Electric), Sap Sipper (Grass),
  and Levitate / Earth Eater (Ground). The check cuts the other way too, and two innates actively
  *earn* a berry: **Ripen** doubles a resist berry from 0.5x to 0.25x, which is why the
  Grass/Dragon apple line are the premium Yache homes — their 4x Ice weakness comes through
  neutral — and **Harvest** regrows the berry after it is eaten, which is why Exeggutor took
  the Tanga. Look for both before placing any berry.
- **Check whether the item IS the set before re-iteming it.** The innate check below is one
  case of a wider rule: some sets are built *around* their item, and swapping it silently
  deletes the build. Three picks were rejected on this in the incense batch, and they are the
  shapes to look for — a **`MOVE_TRICK` set** (Alakazam: the Choice item is the payload it
  throws at a wall, so a replacement item makes Trick pointless), a **`MOVE_BELLY_DRUM` set**
  (Feraligatr: Belly Drum halves HP and the Sitrus Berry is what pays for it), and a
  **Sheer Force set on Life Orb** (Sheer Force cancels Life Orb's recoil on moves with a
  secondary effect). The first two are hard vetoes. The third is only a partial synergy —
  it holds for Liquidation but not Knock Off or Aqua Jet — so Crawdaunt was still re-itemed,
  on the grounds that a Swords Dance sweeper taking real chip from two of its four moves
  would rather have the clean boost. Say which of the three you concluded, and why.
- **Check `src/fork/innate_abilities.c` before taking a berry off a set.** A berry innate can
  make the berry worth several times its face value, and the raw item count cannot see it.
  **Harvest and Ripen are the blocking ones** — they turn a one-shot Sitrus into a recurring
  or oversized one, so the berry is the set's engine. This cost Alolan Exeggutor a Zoom Lens
  in this batch: innate Harvest made its Sitrus worth more than the lens, and the pick moved
  to Sudowoodo. **Gluttony and Cheek Pouch are weaker claims** — a threshold shift and a
  one-off extra heal — and can be outbid: Watchog has Cheek Pouch and still gave up its
  Sitrus, because it is the one species whose innate Keen Eye/Illuminate composes with a lens
  into total evasion immunity. The check also cuts the other way and is worth exploiting:
  Emboar's innate Gluttony moves a Lansat Berry's trigger from 1/4 to 1/2 max HP, which is
  why Emboar got one.
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
- **Re-iteming a set can demote the item it left.** Moving a set onto a new item subtracts
  one from the old item's count, and if that takes a *done* item to one set it is no longer
  graduated. This is how Dragon Fang sat on the done list at a single set after the Soul Dew
  batch took its second, with nothing noticing. **"No done item sits on a single set" now
  gates this**, so a batch that strands an item fails CI rather than drifting — but the fix
  is still yours to choose: give it a second set, or demote it and lower the floor.
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
