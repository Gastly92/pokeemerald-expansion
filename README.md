<!-- FORK: this README is intentionally our own. It is set to merge=ours in
     .gitattributes, so `git merge upstream/master` keeps this file verbatim and
     never pulls in upstream's README. On conflict, keep ours. Upstream's README
     is always available at `upstream/master:README.md`. -->

# Battle Frontier romhack

A standalone single-player Pokémon romhack centered on **quality-of-life
improvements to the Battle Frontier facilities** (starting with a 6v6,
always-level-100 Battle Factory), with more features added over time to grow it
into a fuller game. It's a personal fork of
[RHH's **`pokeemerald-expansion`**](https://github.com/rh-hideout/pokeemerald-expansion),
itself built on [pret's `pokeemerald`](https://github.com/pret/pokeemerald)
decompilation.

## What this fork changes

See **[`FORK.md`](FORK.md)** for the full list of features this fork adds on top
of upstream, with status and known limitations. Conventions and the
upstream-sync process live in **[`CLAUDE.md`](CLAUDE.md)**.

Everything else is stock `pokeemerald-expansion`; this fork regularly merges in
upstream updates.

## Determinism: removing RNG

A central goal of this fork is to **strip as much random chance out of the game
as possible**, so that what happens in a battle follows from the player's
choices and the state on the field rather than from luck. This is rolled out
gradually through a family of **`DETERMINISTIC_*`** flags (in
[`include/config/deterministic.h`](include/config/deterministic.h)), each of
which removes one specific source of randomness. A flag set to `FALSE` is stock
`pokeemerald-expansion` behavior; this fork enables them as features mature.

**Why, given this is a single-player game.** This isn't a competitive-meta
rebalance — there's no human opponent to keep things fair for, so the usual
multiplayer concerns (how a move like Scald shifts a metagame, etc.) don't apply
here. The thing being protected is the *single player's* experience against the
AI, especially in the Battle Frontier facilities, where the appeal is **building
a long win streak through your own skill and Pokémon knowledge**. In stock play
a great run can be ended by pure hax with no counterplay — a surprise critical
hit, a Quick Claw turn flip into a Sheer Cold OHKO, a flinch you never get to act
through. Determinism cuts both the lucky *and* the unlucky variance out of the
loop, so a streak is won or lost on decisions, not dice. (The flip side — the
player also loses their own lucky breaks — is intentional, and where the
compensating `BUFF_*` systems come in.)

The idea is that a random *upside* (a lucky crit, a lucky burn, a lucky full
paralysis) is replaced by something that the player can read off the board and
plan around. Flags shipped so far (all **enabled**):

- **`DETERMINISTIC_CRITICAL_HITS`** — crits no longer come from a random roll.
  They still land when something *guarantees* one (always-crit moves, Laser
  Focus, Merciless vs. a poisoned target, or enough crit-ratio stacking to reach
  100%), but the lucky random crit is gone.
- **`DETERMINISTIC_DAMAGE`** — the random 85%–100% damage roll becomes a fixed
  multiplier that scales with the turn count, so damage is predictable (and, in
  long battles, can climb past the old maximum).
- **`DETERMINISTIC_PARALYSIS`** — paralysis stops being a coin-flip (no random
  full-paralysis miss, no Speed cut) and becomes a flat, predictable tax on PP
  and move priority instead.
- **`DETERMINISTIC_ADDITIONAL_EFFECTS`** — a move's chance-based secondary effect
  (burn, paralysis, a flinch, a stat drop, …) is decided by the matchup rather
  than by a roll: effects on a type that *can* be super effective land only on a
  super effective hit (Fire Punch burns only a Fire-weak target, Iron Head flinches
  only a Steel-weak target), while effects on a type that never can (Normal) land
  only from a STAB user (Body Slam paralyzes / Stomp flinches only from a Normal
  user). Guaranteed (100%) effects are unchanged.
- **`DETERMINISTIC_FLINCH`** — layered on top of the rule above: a flinch that
  passes the super-effective/STAB gate still can't be re-applied to a target that
  flinched *last* turn, so flinch stays meaningful without letting a fast flincher
  chain it into an inescapable lock.

More `DETERMINISTIC_*` flags will follow (see [`FORK.md`](FORK.md)). As random
upsides are removed, the plan is to add balancing systems alongside them — the
**`BUFF_*`** flags — so play stays fair rather than just easier or harder. The AI
is taught each flag's new rules so it still plays to the actual odds.

## Base engine (upstream)

This fork inherits the full `pokeemerald-expansion` feature set and toolchain.
For the base engine's docs, use upstream's:

- 📋 **Features:** [`FEATURES.md`](FEATURES.md)
- 📥 **Install / build / update:** [`INSTALL.md`](INSTALL.md)
- 📖 **Documentation:** <https://rh-hideout.github.io/pokeemerald-expansion/>
- 🤝 **Contributing (upstream):** [`CONTRIBUTING.md`](CONTRIBUTING.md)

❗ Do not use GitHub's "Download Zip" option — it omits commit history, which is
needed to update or merge feature branches.

## Credits

Built on **RHH (Rom Hacking Hideout)**'s `pokeemerald-expansion`. If you use this
base, please credit RHH:

```
Based off RHH's pokeemerald-expansion 1.16.0 https://github.com/rh-hideout/pokeemerald-expansion/
```

Please also consider [crediting all contributors](CREDITS.md) to the upstream
project. The RHH community organizes on their
[Discord server](https://discord.gg/6CzjAG6GZk).
