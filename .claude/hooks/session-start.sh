#!/bin/bash
# SessionStart hook for pokeemerald-expansion.
# Installs the GBA cross-compiler toolchain so `make` and `make check`
# work in fresh Claude Code on the web containers.
set -euo pipefail

# Only run in remote (Claude Code on the web) environments. Local machines
# are expected to already have a toolchain set up per INSTALL.md.
if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi

# Ensure the upstream remote exists so `git fetch upstream` works for syncing
# updates from rh-hideout/pokeemerald-expansion. The container is ephemeral, so
# this is re-added every session. Idempotent: ignore the error if it exists.
git remote add upstream https://github.com/rh-hideout/pokeemerald-expansion.git 2>/dev/null || true

# Define the "ours" merge driver referenced by .gitattributes (e.g. README.md
# merge=ours), so `git merge upstream/master` keeps our version of those files
# verbatim instead of merging upstream's. This is per-clone local config, so it
# must be re-set each ephemeral session. Idempotent.
git config merge.ours.driver true || true

# Idempotent: skip the install if the cross-compiler is already present.
if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  echo "GBA toolchain already installed; skipping."
  exit 0
fi

echo "Installing GBA cross-compiler toolchain..."

PKGS="binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi libpng-dev python3"

# Install directly first: these packages live in the base Ubuntu repos, whose
# index is already present in the container. We skip `apt-get update` in the
# common case because unrelated third-party PPAs (e.g. deadsnakes, ondrej/php)
# can 403 on update, which would otherwise abort this hook under `set -e` and
# waste startup time on retries. Only if the direct install fails do we fall
# back to an update (kept non-fatal with `|| true`) and retry.
if ! sudo apt-get install -y --no-install-recommends $PKGS; then
  sudo apt-get update -y || true
  sudo apt-get install -y --no-install-recommends $PKGS
fi

echo "GBA toolchain install complete."
