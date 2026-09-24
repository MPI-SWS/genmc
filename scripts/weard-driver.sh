#!/bin/bash

# Driver script for GenMC's graph pruning (--mode=random --max-graph-size).
#
# This program is dual-licensed under the Apache License 2.0 and the MIT License.
# You may choose to use, distribute, or modify this software under either license.
#
# Apache License 2.0:
#     http://www.apache.org/licenses/LICENSE-2.0
#
# MIT License:
#     https://opensource.org/licenses/MIT

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GenMC="${GenMC:-$DIR/../RelWithDebInfo/bin/genmc}"

WINDOWS="${WINDOWS:-1 2 5 10 30}"
CORRECT_BUDGET="${CORRECT_BUDGET:-500}"
WRONG_BUDGET="${WRONG_BUDGET:-2000}"
SEED="${SEED:-42}"

GENMCFLAGS="${GENMCFLAGS:-} --mode=random -schedule-seed=${SEED} -disable-estimation"

failures=0
detected=0

run_one() {
    expect=$1 window=$2 file=$3
    cmd=("${GenMC}" ${GENMCFLAGS} -max-graph-size="${window}" \
	 -random-budget="$([ "${expect}" = correct ] && echo "${CORRECT_BUDGET}" || echo "${WRONG_BUDGET}")" \
	 -- "${file}")
    output=$("${cmd[@]}" 2>&1)
    status=$?
    if [ "${expect}" = correct ]; then
	if [ "${status}" -ne 0 ]; then
	    echo ""
	    echo "FAILED (correct, window=${window}, exit=${status}): ${cmd[*]}"
	    failures=$((failures + 1))
	fi
	return 0
    fi
    # wrong: record detection; crashes fail immediately
    if [ "${status}" -ge 128 ] || grep -qE "INTERNAL FAILURE|Assertion" <<< "${output}"; then
	echo ""
	echo "FAILED (crash, window=${window}, exit=${status}): ${cmd[*]}"
	failures=$((failures + 1))
	return 0
    fi
    if [ "${status}" -ne 0 ] && grep -q "Non-atomic race" <<< "${output}"; then
	detected=$((detected + 1))
    fi
}

run_suite() {
    expect=$1 suite=$2
    for testdir in "${suite}"/*/; do
	name=$(basename "${testdir}")
	echo -n "--- ${expect}/${name}: windows"
	detected=0
	for window in ${WINDOWS}; do
	    echo -n " ${window}"
	    for file in "${testdir}"variants/*.c; do
		run_one "${expect}" "${window}" "${file}"
	    done
	done
	if [ "${expect}" = wrong ] && [ "${detected}" -eq 0 ]; then
	    echo " -- race never detected"
	    failures=$((failures + 1))
	    continue
	fi
	echo ""
    done
}

run_suite correct "${DIR}/../tests/correct/weard"
run_suite wrong "${DIR}/../tests/wrong/weard"

if [ "${failures}" -ne 0 ]; then
    echo "*** ${failures} weard test invocation(s) failed (seed=${SEED})"
    exit 1
fi
echo "*** All weard tests behaved as expected (seed=${SEED})"
