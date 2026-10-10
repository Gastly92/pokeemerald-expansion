<!-- FORK: this README is intentionally our own. It is set to merge=ours in
     .gitattributes, so `git merge upstream/master` keeps this file verbatim and
     never pulls in upstream's README. On conflict, keep ours. Upstream's README
     is always available at `upstream/master:README.md`. -->

# Battle Frontier romhack

A standalone single-player Pokémon romhack centered on **quality-of-life
improvements to the Battle Frontier**: 6v6 always-level-100 teams, an endless
challenge, a modern competitive roster, a stronger AI, and a push to strip luck
out of battles. It's a personal fork of
[RHH's **`pokeemerald-expansion`**](https://github.com/rh-hideout/pokeemerald-expansion),
itself built on [pret's `pokeemerald`](https://github.com/pret/pokeemerald)
decompilation, and it regularly merges in upstream updates.

## What this fork changes

- **[`FORK.md`](fork-docs/FORK.md)**: every feature the fork adds, with its flag,
  status and a link to its detail doc.
- **[`DETERMINISM.md`](fork-docs/DETERMINISM.md)**: the central goal of removing
  random chance from battles, one `DETERMINISTIC_*` flag per source.
- **[`FRONTIER_ENDLESS.md`](fork-docs/FRONTIER_ENDLESS.md)**: per-facility progress.
  Facilities are converted one at a time; the **Battle Factory** and **Battle Tower**
  are done, and the other five are still stock.
- **[`CLAUDE.md`](CLAUDE.md)**: conventions and the upstream-sync process.

## Base engine (upstream)

For the base engine, use upstream's docs:

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
Based off RHH's pokeemerald-expansion 1.17.1 https://github.com/rh-hideout/pokeemerald-expansion/
```

Please also consider [crediting all contributors](CREDITS.md) to the upstream
project. The RHH community organizes on their
[Discord server](https://discord.gg/6CzjAG6GZk).
