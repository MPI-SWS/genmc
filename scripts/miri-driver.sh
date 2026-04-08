#!/bin/bash

# Runs Miri's GenMC integration tests against the current GenMC source tree.
#
# This program is dual-licensed under the Apache License 2.0 and the MIT License.
# You may choose to use, distribute, or modify this software under either license.
#
# Apache License 2.0:
#     http://www.apache.org/licenses/LICENSE-2.0
#
# MIT License:
#     https://opensource.org/licenses/MIT

set -euo pipefail

MIRI_REPO="${MIRI_REPO:-https://github.com/rust-lang/miri}"
MIRI_REVISION="${MIRI_REVISION:-master}"
MIRI_DIR="/tmp/miri"

echo "=== Installing Rust toolchain ==="
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain stable
source "$HOME/.cargo/env"

echo "=== Installing rustup-toolchain-install-master ==="
cargo install rustup-toolchain-install-master

echo "=== Cloning Miri (${MIRI_REVISION}) ==="
git clone "${MIRI_REPO}" "${MIRI_DIR}"
cd "${MIRI_DIR}"
git checkout "${MIRI_REVISION}"

echo "=== Setting up Miri toolchain ==="
./miri toolchain

echo "=== Running Miri GenMC tests ==="
export GENMC_SRC_PATH="${CI_PROJECT_DIR}"
./miri test --features=genmc -- genmc
