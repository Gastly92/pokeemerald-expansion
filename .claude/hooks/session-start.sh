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

# Idempotent: skip the install if the cross-compiler is already present.
if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  echo "GBA toolchain already installed; skipping."
  exit 0
fi

echo "Installing GBA cross-compiler toolchain..."
sudo apt-get update -y
sudo apt-get install -y --no-install-recommends \
  binutils-arm-none-eabi \
  gcc-arm-none-eabi \
  libnewlib-arm-none-eabi \
  libpng-dev \
  python3

echo "GBA toolchain install complete."
