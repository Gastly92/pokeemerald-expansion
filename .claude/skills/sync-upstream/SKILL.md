---
name: sync-upstream
description: Merge rh-hideout/pokeemerald-expansion master into this fork and drive the sync to a green build and test suite. Triggers when the user asks to sync / merge / pull in upstream, update to a new expansion release, or says the fork is "N commits behind". Covers the whole run — unshallow, fetch, merge, conflict resolution, the silent breaks a clean merge hides, and the post-sync checks — not just the git commands.
---

# Sync upstream

Merge upstream `master` into the fork and take it all the way to a green
`make check`. **CLAUDE.md is the reference for *why* each rule exists** (fork
conventions, `FORK:`/`UPSTREAM:` tags, ID spaces, the config system); this skill is
the *procedure*, in the order it has to happen. Read CLAUDE.md's "Fork & upstream
sync" sections alongside it — do not duplicate its content here.

Budget honestly: the 1.17.0 sync was 239 commits, 33 conflicted files, 106 conflict
hunks, and the conflicts were **less than half the work**.

## 0. Before touching git

- Branch: `claude/<short-description>`, one session for the whole sync.
- **The merge is not the deliverable.** A clean merge proves nothing about behaviour.
  Plan for: resolve → build → full `make check` → chase every red test to a root
  cause. Do not open the PR at "merge resolved".

## 1. Fetch upstream

```bash
git remote add upstream https://github.com/rh-hideout/pokeemerald-expansion.git  # session-start hook usually did this
git ls-remote upstream HEAD            # 403 => see CLAUDE.md "Fetching upstream from a web session"
git rev-parse --is-shallow-repository  # "true" => the next line is mandatory
git fetch --unshallow --filter=blob:none origin
git fetch upstream
git merge-base HEAD upstream/master    # prints nothing => still shallow, fix before going on
```

`git merge upstream/master` — **merge, never rebase; never squash the sync PR.**

## 2. Resolve conflicts

Work file by file, biggest first (`git diff --name-only --diff-filter=U`, then count
`<<<<<<<` per file). Two different jobs hide in that list:

**Small hunks (≤ ~5 lines)** — a fork clause inside a statement upstream also edited.
Resolve in place: take upstream's new line, re-append our `||` clause. Most of the
count, little of the time.

**Large hunks, especially `ours=N lines / theirs=0`** — upstream *moved or deleted* the
code we had edited (1.17.0 moved `SetMoveEffect` and friends into the new
`src/battle_set_effect.c`: one 1,310-line hunk). Do **not** hand-resolve these by
reading markers; the two sides are not aligned and you will silently drop fork edits.
Use the **fork-delta method**:

```bash
git show :1:path/to/file.c > base.c    # merge base
git show :2:path/to/file.c > ours.c    # our pre-merge version
git show :3:path/to/file.c > theirs.c  # upstream's new version
diff -u base.c ours.c > fork.diff      # exactly our divergence, nothing else
cp theirs.c result.c
patch --fuzz=0 result.c < fork.diff    # hand-resolve only the rejected hunks
```

Then read every `.rej` hunk and re-apply it *where upstream moved that logic to* — a
reject is usually a hook that needs re-attaching, not a hunk to drop. When upstream
split a file, run the same recipe against the new file as `theirs.c`.

**Count `FORK` per conflicted file before and after** (`grep -c FORK <file>`). A drop
means a divergence went missing inside a hunk upstream had rewritten. This is the
cheapest guard there is; do it on every file you resolve.

Resolution policy, per CLAUDE.md: keep our intentional divergences, take upstream
everywhere else. Before deleting a fork divergence because upstream "fixed it too",
diff the two over the edge cases ours was written for.

## 3. The breaks a clean merge hides

Work all of these *after* the markers are gone. CLAUDE.md's "After the merge: what a
git conflict does *not* tell you" lists the classes with worked examples; the checks:

```bash
grep -rn "BATTLE_CONFIG_DEFINITIONS(" include/ src/   # the 3 fork groups must sit beside upstream's at EVERY hit
grep -rn "FORK:" src include | wc -l                  # compare against the pre-merge count
```

- **Fork config groups un-registered** — upstream adds a new macro *instance*; the fork
  must appear at every one, not just the first.
- **ID collisions** (ability ids, AI flag bits) — often produce *no* marker, because
  fork ids live in fork headers upstream never touches. `src/fork/fork_id_guards.c`
  turns these into build errors; if a guard fires, renumber **ours**, never upstream's.
- **Bitfield padding that agrees by coincidence** — recompute as upstream's value minus
  our bits in that word; never take either side's number.
- **Hooks moved out from under a fork feature** — upstream relocates the exact line we
  hooked, the merge takes upstream's version cleanly, the feature stops running. Only
  fork *tests* catch these.
- **A new upstream *caller*** walking a fork helper into a case it was never written
  for (1.17.0: the AI's new `gimmickAtk` damage simulation into `GetZMoveBasePower`).

## 4. Build and test — both are mandatory

One `make` at a time; they share `build/`.

```bash
UNUSED_ERROR=1 DEPRECATED_ERROR=1 make -j$(nproc) -O all   # CI's flags; a plain make is not enough
make -j$(nproc) check                                      # full suite
```

Expect signature churn the compiler finds (renamed fields, new parameters, moved
enums) — grep upstream's own call sites and copy how they now call it.

## 5. Every red test is a lead, not noise

Chase each one to a named cause. Two shapes, and they need opposite responses:

- **A fork test goes red** → usually a hook that needs re-attaching (§3), or an
  upstream fix that invalidated the test's *control* case. Read the upstream commit
  behind it before touching the test.
- **An upstream test goes red** → do not assume upstream shipped it broken. **Build a
  clean upstream worktree at the merged commit and run that same test there:**

  ```bash
  git worktree add --detach ../upstream-check <upstream-sha>
  cd ../upstream-check && make -j$(nproc) check TESTS="<name>"
  ```

  Passing there and failing here means it is ours. In 1.17.0 this is exactly what
  turned "upstream ships a flaky AI test" into a two-line fork bug. Remove the
  worktree when done.

Do not open the PR with a known-failing test and a paragraph of speculation about it.
Either root-cause it or say plainly that you could not, with what you ruled out.

## 6. Land it

- PR against **our** `master`. Never against `rh-hideout/pokeemerald-expansion`.
- **Merge commit, never squash** — squashing severs the shared-history link and makes
  the next sync conflict on everything.
- In the PR body: the conflicts that produced no marker, the fork features re-hooked,
  and anything left as follow-up.
- **Feed the sync back into the docs.** Every sync teaches a new failure mode; add it
  to CLAUDE.md's "After the merge" list with the concrete example, and correct any rule
  this sync proved wrong. That is what keeps the next one cheaper.
