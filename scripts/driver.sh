#!/bin/bash

# Driver script for running tests with GenMC.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you can access it online at
# http://www.gnu.org/licenses/gpl-2.0.html.
#
# Author: Michalis Kokologiannakis <mixaskok@gmail.com>

# Get binary's full path
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GenMC="${GenMC:-$DIR/../genmc}"
GENMCFLAGS="${GENMCFLAGS:-}"
MODELS=(rc11 imm)

# Set the directories for testing
if [ -z ${CORRECT_DIRS+x} ]
then
    CORRECT_DIRS=(infr litmus saver ipr sr liveness synthetic data-structures)
    CORRECT_DIRS=("${CORRECT_DIRS[@]/#/${DIR}/../tests/correct/}" )
fi
if [ -z ${WRONG_DIRS+x} ]
then
    WRONG_DIRS=(safety liveness infr racy memory locking barriers helper)
    WRONG_DIRS=("${WRONG_DIRS[@]/#/${DIR}/../tests/wrong/}" )
fi

source "${DIR}/terminal.sh"

# We need to get the LLVM version for this particular configuration
LLVM_VERSION=`cat ${DIR}/../config.h | awk '/LLVM_VERSION/ {gsub(/"/, "", $3); print $3}'`

# test whether arrays are supported
arrtest[0]='test' ||
    (echo 'Failure: arrays not supported in this version of bash.' && exit 2)

# parse the options until we see --
while [[ $# -gt 0 ]]; do
    case "$1" in
	-f|--fast)
	    fastrun=1
	    shift
	    ;;
	-d|--debug)
	    debug_mode=1
	    shift
	    ;;
        -v|--verbose)
            verbosity_level="$2"
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Programming error"
            exit 3
            ;;
    esac
done

# assumes that the result variable is appropriate set by the child script
initialize_results() {
    total_time=0
    result=""
    header_printed=""
}

# assumes that the (sourced) child script sets the runtime variable
increase_total_time() {
    total_time=`echo "scale=2; ${total_time}+${runtime}" | bc -l`
}

print_results() {
    if test -n "$result"
    then
	echo ''; printline
	echo '!!! ' UNEXPECTED TESTING RESULTS ' !!!' Total time: "${total_time}"
	printline
	exit 1
    else
	echo ''; printline
	echo '--- ' Testing proceeded as expected. Total time: "${total_time}"
	printline
	exit 0
    fi
}

# Do initializations
initialize_results

# First, run the test cases in the correct/ directory
for model in "${MODELS[@]}"
do
    for testdir in "${CORRECT_DIRS[@]}"
    do
	cat="${testdir##*/}"
	if [[ ("${model}" == "lkmm" && "${cat}" != "lkmm" && "${cat}" != "fs") ||
		  ("${model}" != "lkmm" && "${cat}" == "lkmm") ]]
	then
	    continue
	fi
	if [[ "${cat}" == "helper" && "${GENMCFLAGS}" =~ "policy=arbitrary" ]]
	then
	    continue
	fi
	if [[ "${model}" == "imm" && "${cat}" == "sr" ]]
	then
	    continue
	fi
	check_blocked="" && [[ "${cat}" == "saver" || "${cat}" == "helper" ]] &&
	    [[ (! "${GENMCFLAGS}" =~ "policy=arbitrary") ]] && check_blocked="yes"
	if [[ "${cat}" == "ipr" && "${model}" != "imm" ]]
	then
	    check_blocked="yes"
	fi
	source "${DIR}/runcorrect.sh" # the env variables for runcorrect.sh are set
	increase_total_time
    done
done

# Then, run the testcases in the wrong/ directory
header_printed=""

for model in "${MODELS[@]}"
do
    for testdir in "${WRONG_DIRS[@]}"
    do
	cat="${testdir##*/}"
	# under IMM, only run safety and liveness tests
	if test "${model}" = "imm" -a "${cat}" != "safety" -a "${cat}" != "liveness"
	then
	    continue
	fi
	if [[ "${cat}" == "liveness" || "${cat}" == "helper" ]]
	then
	    coherence="mo"
	else
	    coherence="wb"
	fi
	suppress_diff=1
	# if test "${cat}" = "memory" -o "${cat}" = "fs"
	# then
	#     suppress_diff=1
	# fi
	source "${DIR}/runwrong.sh"
	increase_total_time
    done
done

# Print results
print_results
