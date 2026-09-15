# Held items: what the roster uses, and why

An audit of every held item against the extended frontier roster
(`src/fork/frontier_extended_mons.c`), and the record of what it changed: for each item,
why it sits where it sits, and — for the handful still on nobody — why that is the right
answer rather than a gap. It exists because "held items are deliberately varied" is a
claim worth measuring.

**The audit is closed.** Every held item that is live in a frontier battle now sits on at
least two sets, except six that were each examined and deliberately left undrafted. No
balance work is outstanding either — every complaint the audit raised shipped a `BUFF_*`
flag in [`config/buff.h`](../include/config/buff.h). What follows is a reference, not a
queue. It is kept because the *reasoning* is what a future line review or upstream sync
needs, and because several conclusions here were expensive to reach and easy to re-derive
wrongly.

This doc is the counterpart to [`FRONTIER_ROSTER.md`](FRONTIER_ROSTER.md) (what the
roster *is*) and [`LINE_REVIEW.md`](LINE_REVIEW.md) (how to author a set).

> **Just need the current status?** It is not here — it is
> `test/fork/held_item_tracker.c`, which CI gates. See
> [Status lives in the tracker](#status-lives-in-the-tracker-not-here).
>
> **Placing an item on a set?** The screens that stop a dead placement are in
> [Decisions already settled](#3-decisions-already-settled--do-not-re-litigate).

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
`sGiratinaFormChangeTable` and **Griseous Core** is the enabler here. The Giratina-Origin
set is unaffected either way — it names `SPECIES_GIRATINA_ORIGIN` directly, so the item was
never what got it there.

**A second exemption covers the other half of each pair**, and it exists because the first
one is config-dependent in exactly that way. An orb and its crystal share a hold effect, a
species gate and a boost — `ITEM_ADAMANT_ORB` and `ITEM_ADAMANT_CRYSTAL` are both
`HOLD_EFFECT_ADAMANT_ORB` — so which of the two happens to appear in a form change table
says nothing about how either behaves in battle. `ItemSharesHoldEffectWithAFormeUnlocker()`
therefore exempts an item that shares a (non-`HOLD_EFFECT_NONE`) hold effect with an
enabler, which lets each orb sit at the one set that is its ceiling. The `NONE` guard is
load-bearing: Rusted Sword and Shield are enablers with **no hold effect at all**, so
without it every effectless item in the build would look like their twin and the gate would
quietly switch itself off for most of the item table.

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

**1629 sets. 218 distinct held items** (plus one deliberate `ITEM_NONE`, a Persian Thief
set). The tracker classifies **241** items that do something when held, of which **20** do nothing
reachable in a frontier battle (`sIgnoredItems[]` — see "Never expected" below), leaving a live
universe of **221**. Against that, the roster is at **218 used, 3 unused** — and all three
are deliberately parked.

The distribution is heavily top-loaded:

| Item | Sets | Share |
| --- | --- | --- |
| Leftovers | 176 | 10.8% |
| Life Orb | 147 | 9.0% |
| Choice Band | 95 | 5.8% |
| Sitrus Berry | 92 | 5.6% |
| Heavy-Duty Boots | 71 | 4.4% |
| Rocky Helmet | 69 | 4.2% |
| Choice Specs | 62 | 3.8% |
| Assault Vest | 52 | 3.2% |
| Focus Band | 45 | 2.8% |
| Choice Scarf | 38 | 2.3% |

The top two alone are **19.8% of the roster**, down from 25% when this audit opened, and
the tail is long and thin: **160 items appear on one or two sets** — a large share of them
the species-locked signature items, where one set is the ceiling rather than a gap.

> Every number in this section was re-measured with the command under
> [Reproducing the audit](#reproducing-the-audit) as of this commit. The rows below Rocky
> Helmet had drifted by up to eight sets before that, which is the standing argument for
> re-measuring rather than trusting the table.

### Why the concentration matters more than it looks

The Factory draft rejects any candidate whose `heldItem` already appears on the team
being built (`src/battle_frontier.c:344`, "Ensure this Pokemon's held item isn't a
duplicate" — a non-`ITEM_NONE` match makes the draft skip that mon and roll again).
**Only one of each item can appear per team**, for the player's rentals and for each
opponent alike.

So item concentration is a *draft-rate tax*: the 176 Leftovers sets are competing for a
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

That leaves **3 unused items that are live in battle**, and all three are deliberately
*parked* rather than outstanding — see
[Parked](#parked-the-item-works-the-holder-costs-too-much) for the arithmetic behind each.

## How each class was settled

Four groups, in the order they were worked. All four are closed; the headings survive
because the reasoning is indexed by them from `FORK.md` and the tracker.

### Group A — already fixed, just not drafted (no code, roster work) — **shipped**

These items were good, or were *specifically repaired by this fork*, and simply had no
home. Zero engine work — the whole group was placement — which is why it was worked first
and why it was the cheapest capacity in the audit.

| Item(s) | Status | Note |
| --- | --- | --- |
| ~~**Wide Lens, Zoom Lens**~~ — **shipped** | Buffed twice, now **2 sets each** | `BUFF_ACCURACY_ITEMS` gave both a job in the PP economy and `BUFF_ACCURACY_ITEMS_REVEAL` made them INFO-viewer instruments. The reveals only work for the **player**, so both went on rentable (`TIER_NORMAL`) sets: **Wide Lens** on Watchog and Uxie, breadth on two scout leads, and Uxie's innate Forewarn already peeks at one foe's strongest move, so the lens extends an instinct the species has; **Zoom Lens** on Forretress and Sudowoodo, both of which move second as a matter of course (Forretress is `SPE_DOWN` with a Gyro Ball, Sudowoodo has 30 base Speed) — that is the only window Zoom Lens's relief opens in. Watchog's innate Keen Eye/Illuminate is *not* a duplicate of its own item: those cancel the evasion **stage** half (`GetAccEvasionStageDelta`), while a lens cancels the **flat** taxes (`GetDeterministicMoveTargetPPTax`, which only the lenses and No Guard clear), so the two halves compose into a fully evasion-proof attacker. |
| ~~**Blunder Policy**~~ — **shipped** | Rebuilt, now **2 sets** | `DETERMINISTIC_HOLD_EFFECTS` re-armed it on the deterministic blunders (Protect, semi-invulnerability, Wide/Quick/Crafty Guard, Psychic Terrain, a type immunity, a blocking ability, an Air Balloon). In doubles any one avoiding target arms it. Placed on two sets whose **own STAB carries a hard immunity**, so the trigger is a matter of course rather than luck: Diggersby (Return and Quick Attack blanked by Ghost, High Horsepower by Flying/Levitate — and it is a Swords Dance set at 78 Speed, so the blundered turn becomes the sweep window) and Brambleghast (Poltergeist and Shadow Sneak blanked by Normal, and its Choice Band was locking a set that carries Rapid Spin). Note the doubles spread-move idea does **not** apply here: the roster deliberately runs no Earthquake on a doubles set, since it hits the ally. |
| ~~**Razor Fang**~~ — **shipped** | Buffed, now **2 sets** | The flinch items became guaranteed one-shots under `DETERMINISTIC_HOLD_EFFECTS`. King's Rock, the identical twin, is on 4 sets, so this was pure scarcity relief on a proven shape. Both homes are fast physical attackers, which is what converts a one-shot flinch into a free turn: Barraskewda (136 Speed, the fastest physical attacker in the rentable pool — and its Choice Band was locking a set that carries Flip Turn) and Lycanroc-Dusk, where the free turn is a Swords Dance. |
| ~~**Lansat Berry**~~ — **shipped** | Buffed, now **2 sets** | Rebuilt into a guaranteed-crit trigger (it borrows Laser Focus's volatile rather than a crit-stage boost that determinism made dead). Needs a holder that actually *reaches* the threshold, so both homes self-chip: Honchkrow (Brave Bird recoil on a 52/52 defensive frame — and its innate **Super Luck** is exactly the crit-stage ability determinism killed, so the berry is that ability's repair) and Emboar (Flare Blitz *and* Wild Charge, innate Reckless, no Rock Head to cancel the recoil). Emboar's innate **Gluttony** also moves the trigger from 1/4 to 1/2 max HP (`HasEnoughHpToEatBerry`), making it the more reliable of the two. |
| ~~**Leppa Berry**~~ — **shipped** | Quietly much stronger, now **2 sets** | `DETERMINISTIC_ACCURACY_EVASION` turned PP into the *currency accuracy is paid in*, and it also **scales max PP down by accuracy**, so the sets that feel it are the ones whose moves are already short. Leppa restores a move that hits 0 PP, so it went to the two lowest-total-PP rentable sets in the roster: Lurantis (17 effective PP across four moves — Focus Blast is scaled to **3**) and Camerupt (24, with Eruption and Fire Blast at 5 and 4). Camerupt gains twice over, since Life Orb chip was working against Eruption's HP scaling. |
| ~~**Odd / Rock / Rose / Sea / Wave Incense**~~ — **shipped** | Drafted, **2 sets each** | All five are `HOLD_EFFECT_TYPE_POWER`, so `BUFF_TYPE_BOOST_ITEMS` already gave them the same **+40%** as Twisted Spoon, Hard Stone, Miracle Seed and Mystic Water — pure placement, no balance question. Each went to a set whose damage is genuinely concentrated in that type, off a crowded item: **Odd** on Munkidori and Tapu Lele (Psychic Surge makes Expanding Force the boosted move), **Rock** on Relicanth and Rampardos — both innate **Rock Head**, so Head Smash is 150 BP with no recoil and the incense boosts it and Rock Slide — **Rose** on Serperior (innate-free **Contrary**, which spams Leaf Storm, exactly the repeat-clicked move a permanent type item wants and a Gem does not) and Sunflora (Choice Specs was locking it into a SpA-dropping Leaf Storm), and **Sea/Wave** on Floatzel, Crawdaunt, Basculegion and Tentacruel. |
| ~~**Lax Incense**~~ — **shipped** | Drafted, **2 sets** | Byte-identical to Bright Powder under the PP economy: `HOLD_EFFECT_EVASION_UP` is a flat `tax++` on the attacker (`src/battle_util.c`) and the item's own param is never read. Both homes are bulky doubles redirectors that already plan to be attacked repeatedly, so the tax compounds — Clefable (Follow Me + Moonlight) and Tangrowth (Rage Powder + Giga Drain). Unlike the Rocky Helmet each gave up, the tax also applies to non-contact and special attackers. |
| ~~**The 18 resist berries**~~ — **shipped** | Drafted, **2 sets each** | Determinism does not touch them, so the whole class was placement only. The rule that made 16 of the 18 mechanical: **put the berry on a genuine 4x weakness**, which it halves back to 2x, since the berry only fires on a super-effective hit (`GetDefenderItemsModifier`). Two screens are mandatory and both drew blood — an ability *or innate* granting immunity to the berry's own type makes it permanently dead (Scizor and Ferrothorn lost their Occa Berry to Well-Baked Body; a Ghost type would have killed Chilan the same way), and the old item may be the point of the set. Two fork innates cut the other way and produced the best placements: **Ripen** doubles the reduction to 0.25x, so the Grass/Dragon apple line takes a 4x Ice hit at *neutral* damage, and **Harvest** recycles the berry outright, which is why Exeggutor took the Tanga. |
| **Haban and Chilan: the two that break the rule** | Placed on their own terms | Recorded because a future audit will re-derive it otherwise. **Haban** (Dragon) has **no** 4x home anywhere — nothing in the game is 4x weak to Dragon — so a 2x Dragon-type holder is its ceiling (Goodra, Kingdra). **Chilan** (Normal) is the mirror: no type is weak to Normal either, but its trigger is special-cased to fire on *any* Normal hit (`moveType == TYPE_NORMAL`), so it wants a holder that merely **expects** Normal damage. Blissey is the case it could have been written for — 255 base HP behind **10 base Defense**, against a roster carrying 126 physical-Normal move instances. |
| ~~**Wiki, Mago, Iapapa Berry**~~ — **shipped** | Drafted, **2 sets each** | Mechanically identical to Figy and Aguav — the only difference is which nature dislikes the flavor, so placement was the whole job. One screen matters and it is not obvious: a holder whose nature **dislikes** the berry's flavor is *confused* by it rather than healed cleanly, and the disliked flavor tracks the nature's **lowered** stat (`gPokeblockFlavorCompatibilityTable`). That is deterministic here, because `src/battle_frontier.c` runs `ModifyPersonalityForNature()` before the set is built — the nature written on the set is the nature in battle. |
| ~~**Cheri, Pecha, Rawst, Aspear, Persim Berry**~~ — **shipped** | Drafted, **2 sets each** | Narrower Lum Berries, and narrow is the point on an uncrowded slot. Screened against the typing that cannot take the status at all — an Electric type cannot be paralysed, a Fire type cannot be burned — which would leave the berry permanently dead in the slot, the same failure a resist berry on an immune holder produces. |
| ~~**Liechi, Ganlon, Apicot, Starf Berry**~~ — **shipped** | Drafted, **2 sets each** | Pinch berries, so the holder has to reach the threshold *and* be able to use the stat when it does — the stat raised is matched to what the set actually attacks with. Starf is deterministic here (it raises the holder's **currently highest** stat, not a random one), and innate **Gluttony** moves any of them from 1/4 to 1/2 max HP. |
| ~~**The situational tail**~~ — **shipped** | Drafted, **2 sets each** | The last 39 draftable items, the ones with no class rule: the twelve berries in the three rows above, plus Absorb Bulb, Cell Battery, Snowball, Luminous Moss, Eject Button, Eject Pack, Red Card, Room Service, Adrenaline Orb, Ability Shield, Clear Amulet, Protective Pads, Utility Umbrella, Float Stone, Shed Shell, Sticky Barb, Binding Band, Lagging Tail, Full Incense, Metronome, Berserk Gene, Micle Berry, Enigma Berry, Jaboca, Rowap, Kee and Maranga. Each got an explicit rule of its own instead of a shared lever — Eject Pack on a self-lowering nuke, Room Service on a set that carries Trick Room, Lagging Tail on Avalanche/Counter/Mirror Coat, Metronome on a genuine single-move spammer, Utility Umbrella on a holder that does *not* set its own weather. **Ring Target** is the only item of this shape deliberately left out, being the one whose effect is a pure drawback to its own holder; it is [parked](#parked-the-item-works-the-holder-costs-too-much). See the caveat below before re-reading these picks. |

#### The situational tail is rule-governed, not hand-tuned

Worth knowing before re-reading the last row's picks. Those 78 placements closed out the
items with no class rule, where each wants a set built *around* what it does — so each was
placed by an explicit rule and every one was checked against a "can this item ever fire"
sweep before being applied (zero dead placements, zero species reused). But the bar was
**"this item can work here"**, not "this is its best home in the roster". A line review
that wants to re-home one of them is improving a set, not fixing a bug, and should feel
free to.

Two corrections during that batch are worth keeping, because both were the picker being
clever rather than right. The first pass drew **74 of 78 sets from Leftovers alone**, which
would have meant nearly every pick was a defensive staller regardless of the item it was
handed; the source is now spread across fourteen items. The second put Absorb Bulb (**+Sp.
Atk**) on a Choice Band Luxray whose damage is entirely physical — it passed only because
Volt Switch counts as special — and Jaboca, which punishes *physical* attackers, on Blissey
and its ten base Defence. **Count damaging moves by category, and tell a physical wall from
a special one.**

### Group B — dominated by our own buffs — **shipped**

`BUFF_TYPE_BOOST_ITEMS` raised the generic type items from +20% to **+40%** for good
reasons, but it moved a goalpost several *other* item classes were standing on, leaving
each strictly or near-strictly worse than a Charcoal — which is why nothing held them.
This group was regression collateral of our own making, and all of it has shipped.

| Item(s) | The problem | Sketch of a fix |
| --- | --- | --- |
| ~~**The 18 Gems**~~ — **shipped** (#510 balance, #511 roster) | A Gem was **+30%, once, then gone**; a type item is **+40%, every turn, forever**, so the Gem was a strictly worse Charcoal outside the Acrobatics/Unburden interaction. | Done both halves, and the roster half is now complete: **all 18 types carry at least two sets**. `BUFF_GEMS` took the class to **+60%** (break-even against +40% is 1.5 uses, so it wins on a move clicked **once**), and placement followed the settled rule — a self-debuffing nuke (Overheat, Leaf Storm, Draco Meteor, Psycho Boost, Fleur Cannon), a literal one-use move (**Explosion**, the purest case), or true **off-STAB coverage**, which is what gives the eleven types with no nuke of their own a home. One screen is specific to this class: an **-ate ability re-types the move out from under the Gem**. Golem-Alola was the obvious Explosion home for the Normal Gem and its Galvanize turns Explosion Electric, so the Gem would never have fired; plain Golem took it instead. |
| ~~**Soul Dew**~~ — **shipped** (balance + roster) | +20% on Latios/Latias's Psychic and Dragon moves, while Dragon Fang gave them **+40%** on Dragon — the signature item lost to a generic one, and the roster proved it by giving Latios Dragon Fang. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. One number covers the one-type and two-type items alike because each boosts exactly its holder's STAB package. The roster moved **that same Dragon Fang set** onto Soul Dew — Calm Mind / Psyshock / Dragon Pulse splits its damage across both boosted types, which is the shape the item exists for. The gate also had to start reading the holder by **base** species, or a Soul Dew Lati@s that Mega Evolved under `FEATURE_FREE_GIMMICKS` silently lost its own item. **Collateral, fixed later:** that Latios set was Dragon Fang's *second* home, so re-iteming it quietly left Dragon Fang on one set while it sat on the done list. It has since been demoted to pending. |
| ~~**Adamant Orb, Lustrous Orb, Griseous Core**~~ — **shipped** | Same shape as Soul Dew: +20% on two types for one species, weakly dominated by a +40% type item everywhere. | Done under `BUFF_SIGNATURE_TYPE_ITEMS`. Each boosts exactly its holder's dual STAB (Dialga is Steel/Dragon, Palkia Water/Dragon, Giratina Ghost/Dragon). The Origin-forme versions share the hold effect, so they were covered by the same change. **Roster half, done later:** each orb now sits on its own forme's set — Adamant Orb on Dialga, Lustrous Orb on Palkia, Griseous Core on Giratina-Origin — with the crystal on the counterpart forme. One set each is the ceiling, since an orb and its crystal are the same item in battle. |
| ~~**The 17 Memories and 4 Drives**~~ — **shipped** (balance + roster) | A Memory or Drive set the holder's type and gave **no damage multiplier at all**, while Arceus's plate — the same idea for a different species — carried the full +40%. Silvally looked compelled to hold it too, though that turned out to be untrue here — see the note below on forme reversion. | Done. `BUFF_SIGNATURE_TYPE_ITEMS` fixed the balance and locked each item to its own species; the roster then drafted **all 17 Memories** (Silvally formes) and **all 4 Drives** (Genesect formes). The Drive sets run Techno Blast, which the Drive re-types — note it does **not** re-type Genesect, which stays Bug/Steel, so a Drive is +40% off-STAB where a Memory is +40% on top of STAB. |

### Group C — weak in stock, still weak here — **shipped**

Not caused by us; just never worth a slot. **This group closed the balance half of the
audit** — after it, nothing pending was a balance problem, only a placement one.

| Item(s) | The problem | Fix |
| --- | --- | --- |
| ~~**Oran Berry, Berry Juice**~~ — **shipped** | Flat 10 and 20 HP at ≤1/2 HP. At the frontier's Level 50 that is roughly 5–7% and 10–13% of a typical HP pool, against Sitrus Berry's 25% in the same slot — a flat number does not survive the jump to Level 50, and the roster held neither on a single set. | `BUFF_FLAT_HP_ITEMS` heals `maxHP/4` for both, **matching Sitrus exactly**. Duplication is the goal, not a fallback: one item per team means Sitrus's **103 sets** all compete for one slot, so two more items healing the same amount are two more uncontested slots those sets can move onto — the reasoning that put Sea/Wave Incense on the roster as Mystic Water clones. A **bigger** Berry Juice was considered and rejected: the drawback that would pay for it (Ripen cannot double it, Harvest cannot regrow it) reaches **2.3%** of species, so for the other 97.7% it would just be a strictly better Sitrus. At an equal number it keeps an identity for free by **not being a Berry** — innate Unnerve (44 species) blocks Sitrus but not Berry Juice, while Ripen/Harvest (34) amplify Sitrus but not Berry Juice. Same power, opposite matchups. Keyed on the two **items**, not on `HOLD_EFFECT_RESTORE_HP`, because Sitrus shares that hold effect whenever `I_SITRUS_BERRY_HEAL < GEN_4`. Site: `ItemHealHp()` in `src/battle_hold_effects.c`; `test/fork/buff_flat_hp_items.c`. |

#### Parked: the item works, the holder costs too much

| Item(s) | Why it is parked |
| --- | --- |
| **Deep Sea Tooth, Deep Sea Scale** | Both are **2x**, genuinely enormous, and locked to Clamperl, which has no set (it evolves, so the coverage test excuses it). The price is what stalls them, not the item: graduation takes two sets each, so drafting both means **four Clamperl entries** — a 35/64/85/74/55/32 NFE, four times, in a uniform draw — while Huntail and Gorebyss already carry four sets between them. They stay **pending**, not ignored, because nothing about them is dead: a line review that wants an NFE gimmick can pick them up without any code or list change. Just don't count them when sizing a batch. |
| **Ring Target** | The only held item whose effect is a **pure drawback to its holder**: it turns the holder's type *immunities* into neutral damage (`MulByTypeEffectiveness`, `src/battle_util.c`). There is no upside term — it exists in the retail games to be handed to an opponent via Trick or Fling, which no set here does. Drafting it means deliberately making a set worse. |

**Unparked, and why the original reasoning was wrong.** Adamant Orb, Lustrous Orb and
Griseous Core sat in this table until the flavor question — shouldn't the orb belong to the
base forme and the crystal to the Origin forme? — turned out to have a mechanical answer.
The park rested on two claims and **both were false**:

- *"Reaching two sets means doubling the orb onto a second set of the same forme, which adds
  no reach under one-species-per-team."* The draw is uniform over **sets**
  (`GetRandomFrontierExtendedMonId()`), so a second set holding an item does roughly double
  its appearance rate. One-item-per-team stops two sets appearing **together**; it does not
  stop either being drawn.
- *"Dialga-Origin must keep its Adamant Crystal, so only two legal sets remain."* No — this
  doc's own [forme-set finding](#a-forme-set-is-not-compelled-to-hold-its-item) says
  `FORM_CHANGE_ITEM_HOLD` never runs on a battle path. Every set in the family is a legal
  holder, and the orbs read their holder with `GET_BASE_SPECIES_ID`, so either item works on
  either forme.

What actually settles it is that **the orb and the crystal are the same item in battle**:
`ITEM_ADAMANT_ORB` and `ITEM_ADAMANT_CRYSTAL` both carry `HOLD_EFFECT_ADAMANT_ORB` and get
the identical `BUFF_SIGNATURE_TYPE_ITEMS` boost on the identical two types. So one set each
is the **ceiling**, for the same reason a form-change enabler's one set is — and since the
draft compares **exact** species, a team can field Dialga with the Orb and Dialga-Origin
with the Crystal at once. The single-set gate was widened to say so; see
`ItemSharesHoldEffectWithAFormeUnlocker()` in the tracker.

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

## Where it landed

Both halves of the audit are finished.

**Balance.** Three flags settled every complaint the audit raised: `BUFF_GEMS` (+60%,
chosen so the break-even against a permanent item is 1.5 uses), `BUFF_SIGNATURE_TYPE_ITEMS`
(the 17 Plates, 17 Memories, 4 Drives, Soul Dew and the three signature orbs, each locked
to its own species) and `BUFF_FLAT_HP_ITEMS` (Oran Berry and Berry Juice at `maxHP/4`,
matching Sitrus). `BUFF_TYPE_BOOST_ITEMS` is the odd one out: it predates the audit and is
what *created* Group B by moving the goalpost to +40%. Nothing is pending on a number, and
four further claims that *looked* like balance work were assessed and rejected rather than
shipped — see [Assessed and rejected](#assessed-and-rejected-as-buff-candidates).

**Roster.** Distinct held items used went **102 → 218**, and the top two went from **25% of
the roster to 19.8%** — under a fifth for the first time. Every one of those placements was
a **re-item**, not an appended set, which is both what pulled the concentration down and
why no saved rental was ever invalidated. (The roster itself grew 1596 → 1629 sets over the
same period, but from line reviews, not from this audit.)

**What is still on nobody, and why.** Three items, each a deliberate call rather than a gap:
the **Clamperl pair** (Deep Sea Tooth/Scale — the effect is enormous, but graduating both
costs four NFE sets in a uniform draw) and **Ring Target** (a pure drawback to its own
holder). They sit in `sPendingItems[]` rather than `sIgnoredItems[]` because the ignored
list makes a stronger claim — that the effect *cannot happen* — and neither of these is
dead. See [Parked](#parked-the-item-works-the-holder-costs-too-much).

The three signature orbs used to be here as well. They were unparked once the flavor
question turned out to have a mechanical answer: an orb and its crystal are the **same item
in battle**, so the base forme holding the orb and the Origin forme holding the crystal is
one set each at its ceiling, not a shortfall. The park's original arithmetic was wrong in
both directions — see the correction under
[Parked](#parked-the-item-works-the-holder-costs-too-much).

**Not backlog, and not counted anywhere above:** Lucky Punch, Metal Powder and Quick
Powder are on `sIgnoredItems[]` — the effect is real, but the only legal holder cannot use
it. See [Never expected](#never-expected--the-structural-exclusions).

## Method, for when this comes up again

Nothing is queued, so this is no longer a to-do list. It is kept for the two cases that
will put an item back in play: an **upstream sync** adding one, or a **line review**
re-homing one. Both are the same job at a smaller scale, and
[Decisions already settled](#3-decisions-already-settled--do-not-re-litigate) is the part
that stops a dead placement — read it even for a single re-item.

### 1. Read the current state

```bash
make check TESTS="Held item tracker"     # the six gates; green means the lists are honest
```

`test/fork/held_item_tracker.c` is the status. Read its three lists and the comment blocks
inside `sPendingItems[]`, which are grouped by *what kind of work is outstanding*: **needs
a buff**, **thinly drafted** (on one set), **needs a set** (on none). The first two blocks
are empty and the third holds only the six parked items, so **a name you did not expect to
see there is the signal** — it means an upstream sync added an item, or a done item lost a
set. Re-measure usage before trusting any count in this doc:

```bash
grep -oP '\.heldItem = \KITEM_\w+' src/fork/frontier_extended_mons.c | sort | uniq -c | sort -rn
```

### 2. Scope it

**One PR does one kind of work.** A buff PR ships one `BUFF_*` flag; a roster PR moves
items onto sets. Don't mix them — the Gems took two PRs on purpose (#510 balance, #511
roster), and that split is what let each be reviewed on its own merits.

A sensible batch is **one buff flag**, or **8–15 roster re-items**: bigger roster batches
get hard to review, smaller ones waste a CI cycle. The audit ran long batches past that
only where every placement was mechanical (all 18 resist berries at once, all 18 Gems, the
final 78), and the last of those is exactly the batch carrying the
[rule-governed caveat](#the-situational-tail-is-rule-governed-not-hand-tuned).

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
