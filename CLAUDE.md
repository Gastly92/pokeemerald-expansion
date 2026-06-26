# Project notes for Claude

This is a **fork** of [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion)
(a GBA decompilation-based romhacking base). We layer our own custom features on
top of upstream.

## Navigating efficiently (token budget)

This is a huge tree (~30k files, ~1.5M lines) with large auto-generated data
files. Reading one whole costs 25–60k tokens — so don't, and scope searches so
they don't fan out across 11k sprites and palettes.

- **Never `Read` these whole** — `Grep` for the entry, then `Read` with
  `offset`/`limit` around the hit: `data/battle_anim_scripts.s`,
  `src/data/graphics/pokemon.h`, `src/data/moves_info.h`, `src/data/items.h`,
  `src/data/pokemon/level_up_learnsets/*.h`,
  `src/data/pokemon/species_info/*.h`.
- **Scope every search.** Pass `type`/`glob` + a `path`; start with
  `files_with_matches` or `count`, pull content only where it matches. Searches
  that don't need assets should skip `graphics/`, `sound/`, and `data/`.
- **Delegate broad sweeps to a subagent** (`Explore`/`general-purpose`) so the
  file-churn stays out of the main thread — you get the conclusion, not the dump.
- **Iterate with `make check TESTS="<name>"`**, not the full 951-file suite; run
  the full `make check` only before pushing. Keep build logs out of context —
  the actionable signal is the error lines, not the whole transcript.

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

**Web-session caveat (important):** the `upstream` remote does **not** fetch out
of the box from a web session — see "Fetching upstream from a web session" just
below for why and the working recipe. The rest of this section (unshallow, merge,
conflict rules) applies once the upstream commits are reachable. To sync:

```bash
# 0. Web containers clone the fork SHALLOWLY. A shallow clone has no common
#    ancestor with upstream, so `git merge upstream/master` sees two unrelated
#    histories (no merge base) and refuses / would conflict on everything. This
#    also makes GitHub's "N commits behind" look absurd (e.g. 8000+ instead of
#    the real ~70). Deepen to full history FIRST. No-op on a complete clone.
git rev-parse --is-shallow-repository   # "true" => run the next line
git fetch --unshallow origin            # one-time per ephemeral clone; downloads full history
                                        # (lighter alt: git fetch --unshallow --filter=blob:none origin)

# 1. Then sync.
git fetch upstream
git merge upstream/master      # MERGE, not rebase — this is a long-lived shared fork
```

- **Unshallow before merging.** If `git merge-base HEAD upstream/master` prints
  nothing, you're still shallow (or the histories truly diverged) — fix it with
  the unshallow above before doing anything else. This is the single most common
  reason an upstream sync "looks impossible."
- Use **merge**, never rebase: rebasing rewrites history and force-pushes over a
  branch that other sessions/clones may share.
- **Never rewrite the upstream commits** the merge brings in (no `rebase --exec`,
  no amending their author/email even if a hook flags them "Unverified"). Those
  are upstream's real commits; rewriting them severs the shared-history link and
  makes the *next* sync conflict on everything. Only your own merge commit is
  yours to amend.
- On conflicts, favor our intentional feature divergences; keep upstream's
  changes everywhere else.
- After every sync, re-verify the build and tests (see below) before merging to
  `master`.

#### Fetching upstream from a web session (egress is blocked by default)

Findings from a real sync session (the earlier "Verified working" claim was
stale — it is **not** true by default):

- **GitHub traffic uses a separate, scoped GitHub proxy, independent of the
  network-access level.** All git remotes are transparently rewritten (via an
  `insteadOf` rule in `/root/.gitconfig`: `https://github.com/` →
  `http://local_proxy@127.0.0.1:<port>/git/`) to that proxy, which only allows
  the repos in the session's scope — i.e. **our fork only**. So
  `git fetch upstream` against `rh-hideout/...` returns **`403`** ("requested URL
  returned error: 403"). This is a repo-scope denial, not a network outage.
- **`origin/upstream-mirror/master` is unreliable.** The fork carries
  `upstream-mirror/*` branches meant as an in-scope mirror, but they can be far
  out of date (observed sitting at the merge-base, i.e. behind *our* HEAD), so
  merging them may be a no-op. Check `git rev-list --count HEAD..origin/upstream-mirror/master`
  before trusting it.
- **The working recipe — set Full network access, then fetch the public upstream
  directly over egress:**
  1. In the **website UI** (cloud/environment → edit → **Network access → Full**),
     not the iOS app — the iOS app cannot change network access (see Workflow
     conventions). Full access opens general HTTPS egress (verify with
     `curl -o /dev/null -w '%{http_code}' https://github.com/rh-hideout/pokeemerald-expansion`
     → `200`). This takes effect in the **same session** — no restart needed.
  2. Temporarily drop the `insteadOf` rewrite so git won't redirect github.com to
     the scoped proxy, fetch upstream directly, then **restore** the config:
     ```bash
     cp /root/.gitconfig /root/.gitconfig.bak
     git config --global --unset-all "url.http://local_proxy@127.0.0.1:<port>/git/.insteadof"  # exact key: git config --global --get-regexp insteadof
     git remote add upstream-direct https://github.com/rh-hideout/pokeemerald-expansion.git
     git fetch --filter=blob:none upstream-direct master
     mv /root/.gitconfig.bak /root/.gitconfig   # restore the rewrite (origin must keep using the scoped proxy)
     git merge --no-edit upstream-direct/master   # MERGE, not rebase
     ```
  This works because upstream is **public** (unauthenticated fetch). For a private
  upstream you'd instead grant the Claude GitHub App access to that repo and start
  a fresh session (repo scope is fixed at session start). One real run this way
  pulled 19 upstream bugfix commits and merged with **zero conflicts**.
- Alternative with no in-session egress change: use GitHub's **"Sync fork"**
  button / `gh repo sync` to land upstream commits on our fork server-side, then a
  plain `git fetch origin && git merge origin/master` here — but a heavily
  diverged branch often makes "Sync fork" refuse with conflicts.

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
`fork-docs/FORK.md`, `CLAUDE.md`) never conflict and don't need it.

### Minimizing merge conflicts

Conflicts happen line-by-line in files upstream *also* edits. The goal isn't
"separate files" for its own sake — it's keeping our edits to upstream-owned
files small and additive. New files never conflict; small additive edits resolve
easily; rewrites of existing logic conflict the most.

- **Prefer the config system over patching core logic.** Behaviors gated by
  `include/config/*.h` flags (`B_*`/`I_*`/`P_*`/`OW_*`) should be changed via the
  flag, not by editing the function — the change then lands in a file we own.
- **Define our *own* config flags in fork-owned headers, never inline in an
  upstream config header.** Upstream actively edits `config/general.h`,
  `config/battle.h`, etc., so a fork flag wedged into one of those blocks
  conflicts whenever upstream touches a nearby line (this is exactly what bit the
  new-game flags). The fork keeps its flags in headers we own:
  `config/deterministic.h` and `config/feature.h` (runtime-registered via
  `*_CONFIG_DEFINITIONS` for per-test `WITH_CONFIG` toggling) and `config/fork.h`
  (plain compile-time `#if` flags), each pulled in with a one-line additive
  `#include` (`config/fork.h` from `global.h`, right after `general.h`). The
  *usages* still hook into upstream files where the behavior lives, but the
  *definitions* never conflict. To add a flag, drop a `#define` in the right fork
  header — don't add it to an upstream one.
- **Put feature code in new files under the fork code dirs** —
  `src/fork/my_feature.c` + `include/fork/my_feature.h` (+ `test/fork/my_feature.c`)
  — and register it at the smallest possible hook point in an upstream file,
  instead of inlining a whole feature into an existing function. See "Fork-owned
  code lives under `fork/`" below for the directory convention and the one
  exception (config headers).
- **When you must touch a shared file, keep edits additive and localized** (append
  a switch case / table entry / new function) rather than restructuring.
- Caveats: this is not a silver bullet — adding enum values, species, moves, etc.
  still touches shared tables. And new files don't prevent *semantic* conflicts
  (upstream renaming a symbol we call breaks the build without a git conflict),
  which is why re-running `make`/`make check` after every sync is mandatory. Don't
  over-split files just to dodge conflicts; keep it idiomatic.

### Fork-owned code lives under `fork/`

Just like our docs live under `fork-docs/`, our **net-new code files** live under a
`fork/` subdirectory of each tree, so the fork's own code is listable and
greppable at a glance and never tangles with upstream's files:

- **`src/fork/`** — fork-owned C sources (e.g. `src/fork/innate_abilities.c`).
- **`include/fork/`** — fork-owned headers. Because the compiler's `-iquote` points
  at `include/` (not `include/fork/`), these are included with the **`fork/`
  prefix everywhere**: `#include "fork/innate_abilities.h"` — from upstream hook
  points, from other fork sources, *and* from one fork header including another.
- **`test/fork/`** — fork-owned tests (e.g. `test/fork/deterministic_*.c`). Battle
  tests work the same from here; the runner discovers them by content, not path.

This needs **no build-system change**: the Makefile globs sources recursively
(`src/*/*.c`, `test/*/*.c`) and the linker script matches objects with broad
`*.o(...)` wildcards (and `src/*.o` spans nested paths), so a `fork/` subdir is
picked up automatically. New directories never conflict on sync; the only edits
that land in upstream-owned files are the hook-point `#include "fork/…"` lines,
which are conflict-neutral.

**Exceptions (deliberately *not* moved):**
- **Config flag headers stay in `include/config/`** (`config/fork.h`,
  `config/deterministic.h`, `config/buff.h`, `config/feature.h`,
  `config/frontier.h`, `config/accessibility.h`). They're fork-owned *files* but
  belong with upstream's config headers conceptually, and are wired in through
  upstream-owned files (`global.h`, `include/constants/global.h`,
  `include/constants/config_changes.h`). Moving them would mean rewriting those
  upstream `#include "config/…"` lines for zero conflict-reduction — the
  separation that matters (own file, never edited by upstream) is already there.
- **Assets stay where upstream references them by path** (e.g. the fork's
  `graphics/battle_interface/healthbox_*_notail.png`), since relocating an asset
  means chasing its `INCBIN`/graphics-table path through upstream code.

When you add a fork feature, create its files under these `fork/` dirs from the
start. Don't relocate an upstream file into `fork/` (that maximizes conflicts) —
`fork/` is for files that are *ours*.

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

**Special case — a flag that feeds a numeric constant scripts read as a value.**
If a `TRUE`/`FALSE` flag gates a `#define` of a *number* (e.g.
`#if B_FRONTIER_PARTY_SIZE_6V6` → `FRONTIER_PARTY_SIZE` 6 vs 3 in
`constants/global.h`) and a **map script reads that constant as an operand**
(`setvar VAR_0x8005, FRONTIER_PARTY_SIZE`), the `#if` itself runs in the script
cpp pass where `TRUE` is undefined → `#if 0` → scripts silently get the *false*
value (3) while C gets the true one (6). You can't `.if` your way out (it's a
value, not a branch), and you can't `#define TRUE` (breaks the assembler). The fix
is to **define that one flag as a literal `1`/`0`, not `TRUE`/`FALSE`**, so `#if`
evaluates identically in the C and script cpp passes (`.if FLAG` still works, it
becomes `.if 1`). This bit the 6v6 party picker — `B_FRONTIER_PARTY_SIZE_6V6` is
intentionally `1`, with a comment saying why. Flags that only ever appear in `.if`
or as operands can stay `TRUE`/`FALSE`.

### Text in `.string` — only charmap characters (no em-dash, no curly quotes you type)

Map/script text (`.string "…"` in `data/maps/**/scripts.inc`, `data/text/…`) is
encoded through `charmap.txt`, not raw UTF-8. Any character not in the charmap
fails the build with `error: unknown character U+XXXX`. The one that bites
repeatedly is the **em-dash `—` (U+2014): it is NOT in the charmap** — use a
plain hyphen `-`, or the ellipsis `…` (U+2026, which *is* mapped) for a pause.
Curly quotes `“ ” ‘ ’` and `é` (in `POKé`) are mapped and fine; the safe instinct
is "ASCII punctuation + the handful of glyphs vanilla text already uses." Quick
pre-build check for a script file you've edited:

```bash
python3 - <<'EOF'
s=open('data/maps/.../scripts.inc').read()
print(sorted({hex(ord(c)) for c in s if ord(c)>0x7f} - {hex(ord(c)) for c in '…“”‘’é'}))
EOF
```

If it prints anything, that codepoint will break `build/.../event_scripts.o`.

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

### Upstream-mergeable cleanups: the `UPSTREAM:` tag

Sometimes we touch upstream-owned code in a way that is *purely a readability
improvement with no behavior change* — e.g. replacing a magic `3` with the named
`FRONTIER_PARTY_SIZE`, or naming an unexplained constant. These are good
candidates to contribute back upstream (unlike `FORK:` divergences, which are
intentionally ours and must never go upstream).

- **Tag:** `UPSTREAM:` — the counterpart to `FORK:` (FORK: stays ours; UPSTREAM:
  is meant to be sent back). Greppable (`grep -rn "UPSTREAM:" src include`), so
  the set of upstream-mergeable cleanups can be collected into a PR later.
- **Only for behavior-preserving changes** at the vanilla config (a `3 →
  FRONTIER_PARTY_SIZE` swap is behavior-preserving when the flag is off / the
  constant is 3). If the change alters behavior, it's a `FORK:`, not an `UPSTREAM:`.
- Keep the note short; say what was clarified and why it's upstream-safe.

## Documenting fork features

We keep our human-facing docs in files we own (so they never conflict on sync).
`README.md` stays at the repo root (it's the front page); all other fork docs
live under **`fork-docs/`** (`FORK.md`, `DETERMINISM.md`, `FRONTIER_ENDLESS.md`,
`INNATE_ABILITIES.md`, `INNATE_ABILITIES_PROGRESS.md`, `NEW_TYPES.md`). New files
in a fork-owned directory never conflict on sync. The two top-level docs:

- **`README.md`** (root) — the repo's front page, rewritten as our own (a short
  intro describing the standalone Battle Frontier romhack, linking to
  `fork-docs/FORK.md` and crediting upstream).
  It is **not** upstream's README. See "Our own README" below for how that's
  kept conflict-free.
- **`fork-docs/FORK.md`** — the inventory of every feature this fork adds on top
  of upstream: a table of *feature · flag(s) · location · status · notes*. It is
  an **index**, not a spec — the flag comment in `include/config/*.h` stays the
  source of truth for exact behavior; `FORK.md` records what the flag can't
  (status, known limitations, where to look).

**When you add or change a fork feature, update `fork-docs/FORK.md` in the same PR.** Add or
edit its row — especially the *status* and *notes* columns when something is
partial or has a known limitation (e.g. "Battle Dome layout not yet generalized
to 6"). Keep rows one line; don't restate the flag comment. The `FORK:` code tag
(above) and `FORK.md` are complementary: the tag marks the divergence in-code,
the table indexes the feature for a human.

## Workflow conventions

- **The maintainer develops exclusively from a phone** using the Claude Code iOS
  app, occasionally switching to the **website (Safari)** for things the iOS app
  can't do. Concretely: the iOS app **cannot change a session's network access
  level** — setting **Full** access (needed to fetch the public upstream directly,
  see "Fetching upstream from a web session") is only possible from the website.
  Assume there is **no local dev machine** to fall back on — anything that "just
  do it locally" would solve has to be done in-session instead.
- **One branch + one session per feature/PR.** Keep changes scoped; don't tangle
  unrelated features together.
- Branch names use the `claude/<short-description>` prefix.
- Open PRs against our fork's `master`. Don't push directly to `master`.
- **CI runs a full build + test suite on every PR before it can merge, so don't
  burn in-session tokens re-running `make`/`make check` for changes CI will cover
  anyway.** For docs/comment-only changes (e.g. editing `CLAUDE.md`, `fork-docs/`,
  or a code comment), skip the local build/test and let CI gate the PR. Reserve
  local verification for actual code changes, where catching a break before CI is
  worth the tokens (and even then prefer `make check TESTS="<name>"` over the full
  suite — see Building & testing).

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
