# Project notes for Claude

This is a **fork** of [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion)
(a GBA decompilation-based romhacking base). We layer our own custom features on
top of upstream.

## Fork & upstream sync

- **We pull in upstream updates, but we do NOT push our features back upstream.**
  Never open pull requests against `rh-hideout/pokeemerald-expansion`.
- Our custom features live on top of upstream. All our PRs target *our* fork's
  `master` (`Gastly92/pokeemerald-expansion`).
- Upstream has two relevant branches: `master` (stable releases) and `upcoming`
  (bleeding edge). We track **`master`** by default.

### Syncing from upstream

The web container is ephemeral, so the `upstream` remote must be re-added each
session (the session-start hook does this automatically; the manual command is):

```bash
git remote add upstream https://github.com/rh-hideout/pokeemerald-expansion.git
```

Verified working from web sessions: `git fetch upstream` succeeds (both ref
listing and object download). To sync:

```bash
git fetch upstream
git merge upstream/master      # MERGE, not rebase — this is a long-lived shared fork
```

- Use **merge**, never rebase: rebasing rewrites history and force-pushes over a
  branch that other sessions/clones may share.
- On conflicts, favor our intentional feature divergences; keep upstream's
  changes everywhere else.
- After every sync, re-verify the build and tests (see below) before merging to
  `master`.

#### Our own README (`merge=ours`)

`README.md` is ours, not upstream's. To keep upstream syncs from touching it,
`.gitattributes` marks it `merge=ours`, which needs the driver defined per-clone:

```bash
git config merge.ours.driver true   # web: done by the session-start hook; run once in local clones
```

With the driver set, `git merge upstream/master` keeps our `README.md` verbatim
and never reports a conflict on it. If the driver is *not* set, git just falls
back to a normal merge for that file (no worse than any other conflict). To add
another fork-owned file to this scheme, list it in `.gitattributes` with
`merge=ours` — only for files upstream *also* edits; new fork-only files (like
`FORK.md`, `CLAUDE.md`) never conflict and don't need it.

### Minimizing merge conflicts

Conflicts happen line-by-line in files upstream *also* edits. The goal isn't
"separate files" for its own sake — it's keeping our edits to upstream-owned
files small and additive. New files never conflict; small additive edits resolve
easily; rewrites of existing logic conflict the most.

- **Prefer the config system over patching core logic.** Behaviors gated by
  `include/config/*.h` flags (`B_*`/`I_*`/`P_*`/`OW_*`) should be changed via the
  flag, not by editing the function — the change then lands in a file we own.
- **Put feature code in new files** (`src/my_feature.c` + header) and register it
  at the smallest possible hook point in an upstream file, instead of inlining a
  whole feature into an existing function.
- **When you must touch a shared file, keep edits additive and localized** (append
  a switch case / table entry / new function) rather than restructuring.
- Caveats: this is not a silver bullet — adding enum values, species, moves, etc.
  still touches shared tables. And new files don't prevent *semantic* conflicts
  (upstream renaming a symbol we call breaks the build without a git conflict),
  which is why re-running `make`/`make check` after every sync is mandatory. Don't
  over-split files just to dodge conflicts; keep it idiomatic.

### Using config flags in scripts (`.inc`/`.s`) — use `.if`, NOT `#if`

Reading a config flag (`B_*`/`I_*`/`P_*`/`OW_*`) inside a map/event script is a
known footgun. Scripts are built by `preproc | cpp | preproc | as`, and the cpp
pass for scripts only pulls in `constants/global.h`, **not** `gba/gba.h` — so
`TRUE`/`FALSE` are *undefined* during cpp. A value-style flag like
`#define B_FRONTIER_FORCE_LVL_100 TRUE` therefore breaks two ways:

- **`#if B_FRONTIER_FORCE_LVL_100`** → cpp expands it to `#if TRUE` → undefined
  identifier → `#if 0`. It **silently takes the `#else` branch** with no build
  error. This is the trap; don't do it.
- **`#define TRUE 1` in a header to "fix" it** → cpp then rewrites the `.set TRUE, 1`
  line that `constants/global.inc` feeds the assembler into `.set 1, 1` → build
  fails. Don't do this either.

Do one of these instead:

1. **Assembler `.if FLAG` (preferred for pure script/text branches).** `.if`
   runs *after* cpp, in the assembler, where `TRUE`/`FALSE` exist as real `.set`
   symbols (from `global.inc`). So `.if B_FRONTIER_FORCE_LVL_100` … `.else` …
   `.endif` evaluates correctly and the dead branch is eliminated. Precedent:
   `data/maps/MtChimney/scripts.inc` (`.if OW_SHOW_ITEM_DESCRIPTIONS == ...`).
2. **A `special` that returns the flag, branched on with `goto_if_eq`.** Use this
   only when C also needs to act on the flag (the decision genuinely lives in C),
   not just to swap script text — it costs a runtime branch and a registered
   special vs. compile-time elimination.

Note that flags work fine as plain script *operands* (`goto_if_eq VAR_RESULT, TRUE`,
`case FRONTIER_LVL_OPEN, ...`): those are resolved by the assembler, not cpp. Only
cpp `#if`/`#elif` on a config value is the problem.

### Marking intentional divergences: the `FORK:` tag

When we deliberately diverge from upstream inside a file upstream also owns —
especially a **deletion**, or a moved/replaced block git can't auto-merge — leave
a `FORK:` comment at the divergence point so a future sync resolves it correctly
instead of silently re-inlining or dropping our change.

- **Tag:** `FORK:` — greppable, so `grep -rn "FORK:" src include` lists every
  intentional divergence at a glance.
- **Make it a durable resolution hint, not a description of upstream's internals**
  (the latter goes stale). Say what we did, where the logic went, and how to
  resolve a conflict here — e.g. "extracted to `LoadGameSaveAfterBootup()`; on
  conflict, port upstream's change there rather than re-inlining."
- Keep it at the exact spot that would conflict (it shows up on *our* side of the
  conflict markers, where the person resolving will see it).

### Upstream-clarity changes: the `CLARITY:` tag

Sometimes we touch upstream-owned code in a way that is *purely a readability
improvement with no behavior change* — e.g. replacing a magic `3` with the named
`FRONTIER_PARTY_SIZE`, or naming an unexplained constant. These are good
candidates to contribute back upstream (unlike `FORK:` divergences, which are
intentionally ours and must never go upstream).

- **Tag:** `CLARITY:` — greppable (`grep -rn "CLARITY:" src include`), so the set
  of upstream-mergeable cleanups can be collected into an upstream PR later.
- **Only for behavior-preserving changes** at the vanilla config (a `3 →
  FRONTIER_PARTY_SIZE` swap is behavior-preserving when the flag is off / the
  constant is 3). If the change alters behavior, it's a `FORK:`, not a `CLARITY:`.
- Keep the note short; say what was clarified and why it's upstream-safe.

## Documenting fork features

We keep two human-facing docs in files we own (so they never conflict on sync):

- **`README.md`** — the repo's front page, rewritten as our own (a short intro
  describing the standalone Battle Frontier romhack, linking to `FORK.md` and
  crediting upstream).
  It is **not** upstream's README. See "Our own README" below for how that's
  kept conflict-free.
- **`FORK.md`** — the inventory of every feature this fork adds on top of
  upstream: a table of *feature · flag(s) · location · status · notes*. It is an
  **index**, not a spec — the flag comment in `include/config/*.h` stays the
  source of truth for exact behavior; `FORK.md` records what the flag can't
  (status, known limitations, where to look).

**When you add or change a fork feature, update `FORK.md` in the same PR.** Add or
edit its row — especially the *status* and *notes* columns when something is
partial or has a known limitation (e.g. "Battle Dome layout not yet generalized
to 6"). Keep rows one line; don't restate the flag comment. The `FORK:` code tag
(above) and `FORK.md` are complementary: the tag marks the divergence in-code,
the table indexes the feature for a human.

## Workflow conventions

- **One branch + one session per feature/PR.** Keep changes scoped; don't tangle
  unrelated features together.
- Branch names use the `claude/<short-description>` prefix.
- Open PRs against our fork's `master`. Don't push directly to `master`.

### Merging

- **Feature PRs: squash merge.** One clean commit per feature on `master`; drops
  intermediate "fix review"/"trigger CI" noise.
- **Upstream sync PRs: merge commit, never squash.** Squashing a sync collapses
  upstream's commits into one and severs the shared-history link, which makes the
  *next* `git merge upstream/master` conflict heavily. Preserve the merge.
- Don't enable "Require linear history" on `master` — it would block the upstream
  merge commits above.

## Building & testing

Toolchain: the GBA cross-compiler (`arm-none-eabi-*`). In web sessions the
`.claude/hooks/session-start.sh` hook installs it automatically. Locally, see
`INSTALL.md`.

```bash
make -j$(nproc) -O all        # build the Emerald ROM -> pokeemerald.gba
make -j$(nproc) check         # build + run the full test suite (mGBA test runner)
make -j$(nproc) check TESTS="<name>"   # run a single/filtered test (much faster)
make -j$(nproc) release        # optimized build -> pokeemerald-release.gba
```

- A clean `make` doubles as the linter: CI sets `UNUSED_ERROR=1` and
  `DEPRECATED_ERROR=1`, so warnings fail the build.
- **Verify locally with CI's flags before pushing.** A plain `make` won't error
  on unused statics/vars, but CI does. Build with them set —
  `UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j$(nproc) -O all` — so e.g. a function
  referenced only inside a now-disabled `#if` is caught here, not in CI.
- **Build new config flags in both states.** Code under an inactive `#if FLAG`
  isn't compiled in the default build, so errors there stay hidden until the flag
  flips (or CI builds another config). Toggle each new/changed flag — and relevant
  combinations with related flags (e.g. `SKIP_TITLE_SEQUENCE` × `EXPANSION_INTRO`)
  — and rebuild before pushing.
- Baseline (unmodified upstream) test result: all tests pass, exit 0. Compare
  against this after changes to catch regressions.
- **Run one `make` target at a time — don't launch builds concurrently.** All
  targets (`-O all`, `check`, per-flag rebuilds) share the same `build/` tree, so
  running them in parallel makes them clobber each other's object files and
  produces bogus failures (e.g. a spurious non-zero `check` exit). Serialize them,
  or `rm -rf build` between configs. To wait on a background build, block on its
  completion (a single bounded wait) rather than firing off the next `make`.

## CI

- `.github/workflows/build.yml` builds Emerald/FireRed/LeafGreen, runs the test
  suite, and gates everything behind a single `build` status check.
- On pushes to `master`, the `release` job uploads `pokeemerald-release.gba` as a
  downloadable workflow artifact (Actions run -> Artifacts).
