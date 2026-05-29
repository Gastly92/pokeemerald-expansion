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

## Workflow conventions

- **One branch + one session per feature/PR.** Keep changes scoped; don't tangle
  unrelated features together.
- Branch names use the `claude/<short-description>` prefix.
- Open PRs against our fork's `master`. Don't push directly to `master`.

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
- Baseline (unmodified upstream) test result: all tests pass, exit 0. Compare
  against this after changes to catch regressions.

## CI

- `.github/workflows/build.yml` builds Emerald/FireRed/LeafGreen, runs the test
  suite, and gates everything behind a single `build` status check.
- On pushes to `master`, the `release` job uploads `pokeemerald-release.gba` as a
  downloadable workflow artifact (Actions run -> Artifacts).
