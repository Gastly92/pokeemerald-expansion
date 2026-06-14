<!-- FORK: this README is intentionally our own. It is set to merge=ours in
     .gitattributes, so `git merge upstream/master` keeps this file verbatim and
     never pulls in upstream's README. On conflict, keep ours. Upstream's README
     is always available at `upstream/master:README.md`. -->

# Battle Frontier romhack

A standalone single-player Pokémon romhack centered on **quality-of-life
improvements to the Battle Frontier facilities** — 6v6 always-level-100 teams, an
endless challenge, a modern competitive roster, a stronger AI, and a from-scratch
push to strip luck out of battles — with more features added over time to grow it
into a fuller game. The frontier features are designed to span **all seven
facilities**; they're rolled out one facility at a time, and the **Battle Factory
is the first brought to a complete v1**. It's a personal fork of
[RHH's **`pokeemerald-expansion`**](https://github.com/rh-hideout/pokeemerald-expansion),
itself built on [pret's `pokeemerald`](https://github.com/pret/pokeemerald)
decompilation.

## What this fork changes

See **[`FORK.md`](fork-docs/FORK.md)** for the full list of features this fork adds on top
of upstream, with status and known limitations. Conventions and the
upstream-sync process live in **[`CLAUDE.md`](CLAUDE.md)**.

Everything else is stock `pokeemerald-expansion`; this fork regularly merges in
upstream updates.

## Frontier facilities

The goal is to bring every Battle Frontier facility up to the same standard, so
the frontier features are written to be **facility-agnostic wherever possible**.
Many already apply across the board — the determinism engine, max PP,
"all species legal", and the in-battle info viewer run in the shared facility code
paths and so cover (almost) every facility at once. The remaining per-facility
work is wiring up the parts that
still carry facility-specific layout or flow (rosters, the endless challenge flow,
the 6v6 layouts, the swap/info screens).

Facilities are converted one at a time. Progress (**1 / 7**):

- ✅ **Battle Factory** — complete (v1): 6v6, always-Lv100, endless challenge,
  extended modern roster, hard AI, opponent-summary swaps, in-battle info viewer.
- 🔜 **Battle Tower** — next.
- ⬜ Battle Dome · Battle Palace · Battle Arena · Battle Pyramid · Battle Pike.

See **[`FORK.md`](fork-docs/FORK.md)** for the per-feature status, including which features
are still Factory-only and which already cover all facilities.

## Determinism: removing RNG

A central goal of this fork is to **strip as much random chance out of the game
as possible**, so that what happens in a battle follows from the player's choices
and the state on the field rather than from luck. This is rolled out gradually
through a family of **`DETERMINISTIC_*`** flags (in
[`include/config/deterministic.h`](include/config/deterministic.h)), each
removing one specific source of randomness — random crits, the damage roll,
paralysis, secondary effects, and flinch so far, with more to come.

See **[`DETERMINISM.md`](fork-docs/DETERMINISM.md)** for the rationale and the per-flag
breakdown of what each one changes.

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
