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
GenMC="${GenMC:-$DIR/../src/genmc}"

source "${DIR}/terminal.sh"

# We need to get the LLVM version for this particular configuration
LLVM_VERSION=`cat ${DIR}/../config.h | awk '/LLVM_VERSION/ {gsub(/"/, "", $3); print $3}'`

# test whether arrays are supported
arrtest[0]='test' ||
    (echo 'Failure: arrays not supported in this version of bash.' && exit 2)

# test whether getopt works
getopt --test > /dev/null
if [[ $? -ne 4 ]]; then
    echo "`getopt --test` failed in this environment."
    exit 1
fi

# command-line arguments
SHORT=f,o,v:
LONG=debug,fast,verbose:

# temporarily store output to be able to check for errors
PARSED=$(getopt --options $SHORT --longoptions $LONG --name "$0" -- "$@")
if [[ $? -ne 0 ]]; then
    # getopt has complained about wrong arguments to stdout
    exit 2
fi
# use eval with "$PARSED" to properly handle the quoting
eval set -- "$PARSED"

# actually parse the options until we see --
while true; do
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
correctdir="${DIR}/../tests/correct"
for model in rc11 imm
do
    for coherence in wb mo
    do
	for testdir in "${correctdir}/"{infr,litmus,liveness,synthetic,data-structures,lapor,fs}
	do
	    source "${DIR}/runcorrect.sh" # the env variables for runcorrect.sh are set
	    increase_total_time
	done
    done
done

# Then, do all the library tests (and reprint header)
header_printed=""
libdir="${DIR}/../tests/libs"
for model in rc11
do
    for coherence in wb mo
    do
	testdir="${libdir}" && source "${DIR}/runcorrect.sh"
	increase_total_time
    done
done

# Finally, run the testcases in the wrong/ directory
header_printed=""
wrongdir="${DIR}/../tests/wrong"
for model in rc11 imm
do

    for cat in safety liveness infr racy memory locking fs
    do
	# under IMM, only run safety and liveness tests
	if test "${model}" = "imm" -a "${cat}" != "safety" -a "${cat}" != "liveness"
	then
	    continue
	fi
	testdir="${wrongdir}/${cat}"
	coherence="wb"
	suppress_diff=""
	if test "${cat}" = "memory" -o "${cat}" = "fs"
	then
	    suppress_diff=1
	fi
	source "${DIR}/runwrong.sh"
	increase_total_time
    done
done

# Print results
print_results
