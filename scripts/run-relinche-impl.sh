#!/bin/bash

# Test driver for testing ReLinChe
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
# Author: Pavel Golovin <gopavel0@gmail.com>

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
GenMC="${GenMC:-$DIR/../genmc}"

source "${DIR}/terminal.sh"

model="${model:-rc11}"
coherence="${coherence:-mo}"
testdir="${testdir:-${DIR}/../tests/correct/relinche}"

runtime=0
tests_success=0
tests_fail=0
result=0

shopt -s nullglob

print_header() {
	# Print status
	echo ''
	printline
	echo -n '--- Preparing to check conformance in '
	echo "${testdir##*/}" 'under' "${model}" 'with' "${coherence}" |
		awk '{ print toupper($1), $2, toupper($3), $4, toupper($5) }'
	printline
	echo ''

	# Print table's header
	printline
	printf "| ${CYAN}%-26s${NC} | ${CYAN}%-4s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-8s${NC} |\n" \
	       "Testcase" "Spec" "Result" "Execs" "Hints"
	printline
}

printfooter() {
    printline
    echo '--- Test time: ' "${runtime}"
    printline
    echo ''
}

print_info() {
	line_suffix=""
	if [ "${test_line_num}" -gt 1 ]; then
		line_suffix=" (${test_line_num})"
	fi
	SPEC_TYPE="v"
	if [[ "${args_in}" =~ "DSYNC_INS" ]]; then SPEC_TYPE="${SPEC_TYPE}i"; fi
	if [[ "${args_in}" =~ "DSYNC_REM" ]]; then SPEC_TYPE="${SPEC_TYPE}r"; fi
	TEST_COLOR="${POWDER_BLUE}"
	if [[ -n "${should_fail}" ]]; then TEST_COLOR="${ORANGE}"; fi
	printf "| ${TEST_COLOR}%-26s${NC} | ${POWDER_BLUE}%-4s${NC} | " \
	       "${test_name:5:${#test_name}-8}${line_suffix}" "${SPEC_TYPE}"
}

print_results() {
    if test -n "${outcome_failure}"; then
	outcome="${RED}ERROR ${NC}"
	result=1
    elif test -n "${failure}"; then
	outcome="${YELLOW}FAILED${NC}"
	result=1
    else
	outcome="${GREEN}SAFE  ${NC}"
    fi
    printf "${outcome} | %8s | %8s |\n" "${che_execs}"  "${che_hints}"
    if test -n "${failure}" -o -n "${outcome_failure}"; then
	echo "${failure_output}"
    fi
}

check_client_extension() {
    che_output_s=$(echo "${che_output}" | tr '\n' ' ')
    if ! [[ "${che_output_s}" =~ "The library implementation returns incorrect return values" ]]
    then
	return;
    fi;

    che_args=$(echo "${args_in}")
    extension_flags=$(echo "${che_output_s}" | grep 'DGENERATE_SYNC' | sed 's/.*\(-DGENERATE_SYNC \(-D[^ ]* \)*\).*/\1/')
    che_ext_cmd="${GenMC} ${GENMCFLAGS} ${CHE_FLAGS} --check-lin-spec=${spec_file} --max-hint-size=0 -- ${extension_flags} ${che_args} ${client} 2>&1"
    if test -n "${print_cmd}"; then printf "\n${che_ext_cmd}\n"; fi # debug print
    che_ext_output=$(eval "${che_ext_cmd}")
    if test "$?" -eq 0; then
	failure_output="Erroneous extension is found, but extend client passed!"
	failure=1
    fi
}

check_hints() {
    hint_num="${che_hints}"
    if test -n "${promote_mode}"; then
	echo "${hint_num}" >> "${expected_file}"
    else
	expected_results=$(sed "${test_line_num}q;d" "${expected_file}")
	if test "${hint_num}" != "${expected_results}"; then
	    failure_output="Expected number of checked hints: ${expected_results}; actual: ${hint_num}"
	    failure=1
	fi
    fi
}

# reads: che_output_s
# writes: failure_output, failure
check_error_message() {
    error_message_s=$(echo "${che_output_s}" | sed -n 's/.*\(The library implementation.*\)\ \ \ .*/\1/p')
    expected_results=$(sed "${test_line_num}q;d" "${expected_file}")
    if test -n "${promote_mode}"; then
	echo "${error_message_s}" >> "${expected_file}"
    elif test "${error_message_s}" != "${expected_results}"; then
	NL=$'\n'
	failure_output="Expected error message: ${expected_results}${NL}Actual error message:   ${error_message_s}"
	failure=1
    fi
}

[ -z "${TESTFILTER}" ] && TESTFILTER=*

runimpl() {
    failure=""
    failure_output=""
    dir="$1"
    spec="${dir}/spec.c"
    client="${dir}/mpc.c"
    GENMCFLAGS="${GENMCFLAGS:--disable-estimation --disable-mm-detector --rc11}"
    che_hint_time="" # requires genmc to be compiled with -DGENMC_DEBUG
    che_genmc_time=""
    for test_in in "${dir}"/args.*.in; do
	# skip big tests
	if test -n "${fastrun}"; then
	    if [[ "${test_in}" =~ "NxN" ]]; then
		continue
	    fi
	fi

	# filter tests
	case "${test_in}" in
	    ${TESTFILTER}) ;;
	    *) continue;;
	esac

	# check whether test is negative
	should_fail=""
	if [[ "${test_in}" =~ "fail" ]]; then
	    should_fail=1
	fi

	test_line_num=0
	# find expected file and recreate it in --promote mode
	expected_file=$(echo "${test_in}" | sed 's/args\./expected./')
	if test -n "${promote_mode}"; then
	    [ -e "${expected_file}" ] && rm -v "${expected_file}";
	    touch "${expected_file}"
	fi

	# run all subtest in the file
	while read args_in || [[ $args_in ]]; do
	    # initialize params
	    ((test_line_num++))
	    failure=""
	    outcome_failure=""
	    failure_output=""
	    test_name="$(basename ${test_in})"

	    # print test info
	    print_info

	    # (after print_info); calculate spec_file name
	    cds="${dir##*/}"
	    spec_file="${dir}/${cds}_spec_${SPEC_TYPE}.in"

	    # run the tool and collect output
	    che_args=$(echo "${args_in}")
	    che_cmd="${GenMC} ${GENMCFLAGS} --check-lin-spec=${spec_file} ${CHE_FLAGS} -- ${che_args} ${client} 2>&1"

	    # print running command if requested
	    if test -n "${print_cmd}"
	    then
		printf "\n${che_cmd}\n"
	    fi

	    che_output=$(eval "${che_cmd}")
	    ret_code="$?"
	    che_output_s=$(echo "${che_output}" | tr '\n' ' ')

	    # failed but shouldn't
	    if [ "${ret_code}" -ne 0 ] && [ -z "${should_fail}" ]; then
		failure_output=${che_output}
		failure=1
		print_results
		continue
	    fi

	    # failed as expected
	    if [ "${ret_code}" -ne 0 ] && [ -n "${should_fail}" ]; then
		check_error_message
		if test -z "${failure}"; then
		    check_client_extension
		fi
		print_results
		continue
	    fi

	    # didn't fail but should
	    if [ "${ret_code}" -eq 0 ] && [ -n "${should_fail}" ]; then
		failure_output="Linearizability error in not detected in negative test"
		failure=1
		print_results
		continue
	    fi

	    # extract results
	    che_execs=$(echo "${che_output_s}" | sed -n 's/.*Number of complete executions explored: \([0-9][0-9]*\).*/\1/p')
	    che_hints=$(echo "${che_output_s}" | sed -n 's/.*Number of checked hints: \([0-9][0-9]*\).*/\1/p')
	    che_time=$(echo "${che_output_s}" | sed -n 's/.*Total wall-clock time: \([0-9\.][0-9\.]*\).*/\1/p')
	    che_hint_time=$(echo "${che_output_s}" | sed -n 's/.*Relinche time: \([0-9\.][0-9\.]*\)s.*/\1/p')
	    if test -n "${che_hint_time}"; then
		che_genmc_time=$(echo "${che_time}-${che_hint_time}" | bc -l)
	    fi
	    check_hints

	    # update total time counter
	    runtime=$(echo "scale=2; ${runtime}+${che_time}" | bc -l)
	    print_results
	done <"${test_in}"
    done
}


# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
	-f | --fast)
	    fastrun=1
	    shift
	    ;;
	-c | --print-cmd)
	    print_cmd=1
	    shift
	    ;;
	-p | --promote)
	    promote_mode=1
	    shift
	    ;;
	*)
	    shift
	    ;;
    esac
done

# Run correct testcases and update status
runtime=0
print_header
for dir in "${testdir}"/*; do
    runimpl "${dir}"
done
printfooter

exit "${result}"
