<!-- FORK: this README is intentionally our own. It is set to merge=ours in
     .gitattributes, so `git merge upstream/master` keeps this file verbatim and
     never pulls in upstream's README. On conflict, keep ours. Upstream's README
     is always available at `upstream/master:README.md`. -->

# Battle Frontier sandbox

A personal fork of [RHH's **`pokeemerald-expansion`**](https://github.com/rh-hideout/pokeemerald-expansion),
set up as a fast-booting **Battle Frontier sandbox** for testing Frontier
features (notably a 6v6 Battle Factory). It is a GBA ROM hack base built on
[pret's `pokeemerald`](https://github.com/pret/pokeemerald) decompilation — **not
a playable game on its own.**

## What this fork changes

See **[`FORK.md`](FORK.md)** for the full list of features this fork adds on top
of upstream, with status and known limitations. Conventions and the
upstream-sync process live in **[`CLAUDE.md`](CLAUDE.md)**.

Everything else is stock `pokeemerald-expansion`; this fork regularly merges in
upstream updates.

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
