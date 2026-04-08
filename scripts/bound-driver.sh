#!/bin/bash

# Driver script for running tests with GenMC.
#
# This program is dual-licensed under the Apache License 2.0 and the MIT License.
# You may choose to use, distribute, or modify this software under either license.
#
# Apache License 2.0:
#     http://www.apache.org/licenses/LICENSE-2.0
#
# MIT License:
#     https://opensource.org/licenses/MIT

# Get binary's full path
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GenMC="${GenMC:-$DIR/../RelWithDebInfo/bin/genmc}"

source "${DIR}/terminal.sh"

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

correctdir="${DIR}/../tests/correct"

# For now only run fastrun
fastrun=1
for cat in infr litmus saver ipr helper liveness synthetic data-structures #lapor fs
do
    testdir="${correctdir}/${cat}"
    model=sc
    for bound_type in context round
    do
        source "${DIR}/runcorrect.sh" # the env variables for runcorrect.sh are set
        increase_total_time
    done
done

# Print results
print_results
