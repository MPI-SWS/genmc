#!/bin/bash
#
# This program is dual-licensed under the Apache License 2.0 and the MIT License.
# You may choose to use, distribute, or modify this software under either license.
#
# Apache License 2.0:
#     http://www.apache.org/licenses/LICENSE-2.0
#
# MIT License:
#     https://opensource.org/licenses/MIT

# Usage: ./scripts/lint.sh [build-dir]      # default build-dir: RelWithDebInfo
#
# The build dir needs to contain a compile_commands.json and generated headers.
# To update clang tooling:
#   1. Bump CLANG_FORMAT / RUN_CLANG_TIDY below.
#   2. Update runner with the new binaries.
#   3. Ensure auto-generated code uses same formatters.

set -uo pipefail

CLANG_FORMAT="${CLANG_FORMAT:-clang-format-21}"
RUN_CLANG_TIDY="${RUN_CLANG_TIDY:-run-clang-tidy-21}"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "${DIR}/.." || exit 1

BUILD_DIR="${1:-RelWithDebInfo}"
CORES="$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)"
rc=0

# Formatting (style only): genmc/ passes/ lli/
find genmc/ passes/ lli/ -name '*.cpp' -o -name '*.hpp' \
	| xargs "${CLANG_FORMAT}" --dry-run --Werror || rc=1

# clang-tidy: only genmc/ and passes/ (lli/ is vendored); one pass per tier so
# each header is judged under its own directory's .clang-tidy.
log="$(mktemp)"
"${RUN_CLANG_TIDY}" -p "${BUILD_DIR}" -header-filter='genmc/genmc/' -j"${CORES}" \
	$(find genmc/ -name '*.cpp') 2>&1 | tee "${log}"
"${RUN_CLANG_TIDY}" -p "${BUILD_DIR}" -header-filter='passes/passes/' -j"${CORES}" \
	$(find passes/ -name '*.cpp') 2>&1 | tee -a "${log}"
grep -qE 'warning:|error:' "${log}" && rc=1
rm -f "${log}"

exit "${rc}"
