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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GenMC="${GenMC:-$DIR/../src/genmc}"

source "${DIR}/terminal.sh"

model="${model:-rc11}"
coherence="${coherence:-wb}"
suppress_diff="${suppress_diff:-}"

runtime=0
tests_success=0
tests_fail=0

shopt -s nullglob

print_debug_header() {
    # Print status
    echo ''; printline
    echo -n '--- Preparing to run testcases in '
    echo "${testdir##*/}" 'under' "${model}" 'with' "${coherence}" |
	awk '{ print toupper($1), $2, toupper($3), $4, toupper($5) }'
    printline; echo ''

    # Print table's header
    printline
    printf "| ${CYAN}%-17s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-8s${NC} |\n" \
	   "Testcase" "Result" "Executions" "Blocked" "Avg.time"
    printline
}

print_condensed_header() {
    if test -z "${header_printed}"
    then
       # Print table's header
	echo '';printline
	printf "| ${CYAN}%-10s${NC} | ${CYAN}%-15s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-9s${NC} | ${CYAN}%-5s${NC} |\n" \
	       "Model" "Category" "Successful" "Failed" "Time"
	printline

	header_printed=1
    else
	echo ''
    fi
}

printheader() {
    if test -n "${debug_mode}"
    then
	print_debug_header
    else
	print_condensed_header
    fi
}

print_debug_footer() {
    printline
    echo '--- Test time: ' "${runtime}"
    printline; echo ''
}

# print_condensed_footer() {
# }

printfooter() {
    if test -n "${debug_mode}"
    then
	print_debug_footer
    # else
    # 	print_condensed_footer
    fi
}

print_variant_info() {
    if test -n "${debug_mode}"
    then
	printf "| ${POWDER_BLUE}%-17s${NC} | " "${dir##*/}${n}"
    fi
}

print_variant_debug_results() {
    if test -n "${outcome_failure}"
    then
	printf "${RED}%-15s${NC} | %-6s | % 13s |\n" \
	       "BUG NOT FOUND" "${vars}" "${average_time}"
	echo "${output}" # also print the output in this case
	echo "Return status: ${failure_status}"
	result=1
    elif test -n "${failure}"
    then
	printf "${LIME_YELLOW}%-15s${NC} | %-6s | % 13s |\n" \
	       "TRACE DIFF" "${vars}" "${average_time}"
	result=1
    else
	printf "${GREEN}%-15s${NC} | %-6s | % 13s |\n" \
	       "BUG FOUND" "${vars}" "${average_time}"
    fi
}

print_variant_condensed_results() {
    if test -n "${outcome_failure}"
    then
	((tests_fail++))
	result=1
    elif test -n "${failure}"
    then
	((tests_fail++))
	result=1
    else
	((tests_success++))
    fi

    # replace previous results
    for ((i=0;i<1000;i++)); do printf "\r"; done

    # print results so far
    print_fail="" && [[ "${tests_fail}" -ne 0 ]] && print_fail="${tests_fail}"
    printf "| ${POWDER_BLUE}%-10s${NC} | ${NC}%-15s${NC} | ${GREEN}%-10s${NC} | ${RED}%-9s${NC} | ${NC}%-5s${NC} |" \
	   "${model^^}-${coherence^^}" "${testdir##*/}" "${tests_success}" "${print_fail}" "${runtime}"
}

print_variant_results() {
    if test -n "${debug_mode}"
    then
	print_variant_debug_results
    else
	print_variant_condensed_results
    fi
}

runvariants() {
    print_variant_info
    vars=0
    test_time=0
    failure=""
    diff=""
    outcome_failure=""
    unroll="" && [[ -f "${dir}/unroll.in" ]] && unroll="-unroll="`head -1 "${dir}/unroll.in"`
    checker_args=() && [[ -f "${dir}/genmc.${model}.${coherence}.in" ]] &&
	checker_args=(`cat "${dir}/genmc.${model}.${coherence}.in"`)
    for t in $dir/variants/*.c $dir/variants/*.cpp
    do
	vars=$((vars+1))
	output=`"${GenMC}" "-${model}" "-${coherence}" "${unroll}" -print-error-trace $(echo ${checker_args[@]}) -- ${CFLAGS} ${test_args} ${t} 2>&1`
	status="$?"
	trace=`echo "${output}" | awk '!/status|Total wall-clock time/ {print $0 }' > tmp.trace`
	diff_file="${t%.*}.${model}.${coherence}.trace" &&
	    [[ -f "${t%.*}.${model}.${coherence}.trace-${LLVM_VERSION}" ]] &&
	    diff_file="${t%.*}.${model}.${coherence}.trace-${LLVM_VERSION}"
	diff=`diff tmp.trace "${diff_file}"`
	if test -n "${diff}" -a -z "${suppress_diff}"
	then
	    if test "$status" -ne 42
	    then
		failure_status="$status"
		outcome_failure=1
	    fi
	    echo "${diff}"
	    failure=1
	else
	    # if no diff was found (or the diff is suppressed), check non-zero status
	    if test "$status" -eq 0
	    then
		failure_status="$status"
		outcome_failure=1
		failure=1
	    fi
	fi
	explored=`echo "${output}" | awk '/explored/ { print $6 }'`
	time=`echo "${output}" | awk '/wall-clock/ { print substr($4, 1, length($4)-1) }'`
	time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found
	test_time=`echo "${test_time}+${time}" | bc -l`
	runtime=`echo "scale=2; ${runtime}+${time}" | bc -l`
	rm tmp.trace
    done
    average_time=`echo "scale=2; ${test_time}/${vars}" | bc -l`
    print_variant_results
}

runtest() {
    dir=$1
    n=""
    test_args="" && [[ -f "${dir}/args.${model}.${coherence}.in" ]] &&
	test_args=`head -n 1 "${dir}/args.${model}.${coherence}.in"`
    #expected=`head -n 1 "${dir}/expected.in"`
    runvariants
}

[ -z "${TESTFILTER}" ] && TESTFILTER=*

# Run wrong testcases
printheader
for dir in "${testdir}"/*
do
    case "${dir##*/}" in
	${TESTFILTER}) ;;
	*)     continue;;
    esac
    runtest "${dir}"
done
printfooter
