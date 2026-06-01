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

The first is **`DETERMINISTIC_CRITICAL_HITS`** (**enabled** in this fork):
critical hits no longer happen on a random roll. Crits still land when something
*guarantees* one — moves that always crit, Laser Focus, the Merciless ability
against a poisoned target, or enough crit-ratio stacking to reach 100% — but the
"lucky" random crit is gone. More `DETERMINISTIC_*` flags will follow (see
[`FORK.md`](FORK.md)). As random upsides are removed, the plan is to add
balancing systems alongside them so play stays fair rather than just easier or
harder.

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
