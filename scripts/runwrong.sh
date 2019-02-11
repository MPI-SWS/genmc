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

source terminal.sh
GenMC=../src/genmc

runtime=0
model="${model:-wb}"

runvariants() {
    printf "| ${POWDER_BLUE}%-18s${NC} | " "${dir##*/}${n}"
    vars=0
    test_time=0
    failure=""
    diff=""
    outcome_failure=""
    checker_args="" && [[ -f "${dir}/genmc.in" ]] && checker_args=`head -1 "${dir}/genmc.in"`
    for t in $dir/variants/*.c
    do
	vars=$((vars+1))
	output=`"${GenMC}" "-${model}" -print-error-trace "${checker_args}" -- "${CFLAGS}" "${t}" 2>&1`
	if test "$?" -eq 0
	then
	    outcome_failure=1
	fi
	trace=`echo "${output}" | awk '!/status|time/ {print $0 }' > tmp.trace`
	diff_file="${t%.*}.trace" && [[ -f "${t%.*}.trace-${LLVM_VERSION}" ]] && diff_file="${t%.*}.trace-${LLVM_VERSION}"
	diff=`diff tmp.trace "${diff_file}"`
	if test -n "${diff}"
	then
	    failure=1
	fi
	explored=`echo "${output}" | awk '/explored/ { print $6 }'`
	time=`echo "${output}" | awk '/time/ { print substr($4, 1, length($4)-1) }'`
	time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found
	test_time=`echo "${test_time}+${time}" | bc -l`
	runtime=`echo "scale=2; ${runtime}+${time}" | bc -l`
	rm tmp.trace
    done
    average_time=`echo "scale=2; ${test_time}/${vars}" | bc -l`
    if test -n "${outcome_failure}"
    then
	printf "${RED}%-15s${NC} | %-6s | % 13s |\n" \
	       "BUG NOT FOUND" "${vars}" "${average_time}"
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

runtest() {
    dir=$1
    if test -f "${dir}/args.in"
    then
	while read test_args <&3 && read expected <&4; do
	    n="/`echo ${test_args} |
                 awk ' { if (match($0, /-DN=[0-9]+/)) print substr($0, RSTART+4, RLENGTH-4) } '`"
	    runvariants
	done 3<"${dir}/args.in" 4<"${dir}/expected.in"
    else
	test_args=""
	n=""
	#expected=`head -n 1 "${dir}/expected.in"`
	runvariants
    fi
}

# Update status
echo ''; printline
echo -n '--- Preparing to run testcases in '
echo "${testdir##*/}" 'under' "${model}" | awk '{ print toupper($1), $2, toupper($3) }'
printline; echo ''

# Print table's header
printline
printf "| ${CYAN}%-18s${NC} | ${CYAN}%-15s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-13s${NC} |\n" \
       "Testcase" "Result" "Files" "Avg. time"
printline

# Run wrong testcases
for dir in "${testdir}"/*
do
    runtest "${dir}"
done
printline
echo '--- Test time: ' "${runtime}"
printline; echo ''
