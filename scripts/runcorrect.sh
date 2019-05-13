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

header_printed=""
runtime=0
model="${model:-wb}"

printheader() {
    if test -z "${header_printed}"
    then
        header_printed=1

        # Update status
	echo ''; printline
	echo -n '--- Preparing to run testcases in '
	echo "${testdir##*/}" 'under' "${model}" | awk '{ print toupper($1), $2, toupper($3) }'
	printline; echo ''

	# Print table's header
	printline
	printf "| ${CYAN}%-17s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-10s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-8s${NC} |\n" \
	       "Testcase" "Result" "Executions" "Blocked" "Avg.time"
	printline
    fi
}

printfooter() {
    if test -n "${header_printed}"
    then
        printline
        echo '--- Test time: ' "${runtime}"
        printline; echo ''
    fi
}

runvariants() {
    printf "| ${POWDER_BLUE}%-17s${NC} | " "${dir##*/}${n}"
    vars=0
    test_time=0
    failure=""
    outcome_failure=""
    unroll="" && [[ -f "${dir}/unroll.in" ]] && unroll="-unroll="`head -1 "${dir}/unroll.in"`
    checker_args="" && [[ -f "${dir}/genmc.${model}.in" ]] && checker_args=`head -1 "${dir}/genmc.${model}.in"`
    for t in $dir/variants/*.c
    do
	vars=$((vars+1))
	output=`"${GenMC}" ${GENMCFLAGS} "-${model}" "${unroll}" ${checker_args} -- ${CFLAGS} ${test_args} "${t}" 2>&1`
	if test "$?" -ne 0
	then
	    outcome_failure=1
	fi
	explored=`echo "${output}" | awk '/explored/ { print $6 }'`
	blocked=`echo "${output}" | awk '/blocked/ { print $6 }'`
	time=`echo "${output}" | awk '/time/ { print substr($4, 1, length($4)-1) }'`
	time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found
	test_time=`echo "${test_time}+${time}" | bc -l`
	runtime=`echo "scale=2; ${runtime}+${time}" | bc -l`
	expected="${expected:-${explored}}"
	if test "${expected}" != "${explored}"
	then
	    explored_failed="${explored}"
	    failure=1
	fi
    done
    average_time=`echo "scale=2; ${test_time}/${vars}" | bc -l`
    if test -n "${outcome_failure}"
    then
	outcome="${RED}ERROR ${NC}"
	result=1
    elif test -n "${failure}"
    then
	outcome="${LIME_YELLOW}FAILED${NC}"
	explored="${explored_failed:-0}/${expected}"
	result=1
    else
	outcome="${GREEN}SAFE  ${NC}"
    fi
    printf "${outcome} | % 10s | % 8s | % 8s |\n" \
       "${explored}" "${blocked}" "${average_time}"
}

runtest() {
    dir=$1
    if [ -z "$(ls ${dir})" -o ! -d "${dir}/variants" ] # Skip empty directories
    then
	return
    fi
    if test -f "${dir}/args.${model}.in"
    then
	while read test_args <&3 && read expected <&4; do
	    n="/`echo ${test_args} |
                 awk ' { if (match($0, /-DN=[0-9]+/)) print substr($0, RSTART+4, RLENGTH-4) } '`"
	    runvariants
	done 3<"${dir}/args.${model}.in" 4<"${dir}/expected.${model}.in"
    else
	test_args=""
	n=""
	expected=`head -n 1 "${dir}/expected.${model}.in"`
	runvariants
    fi
}

[ -z "${TESTFILTER}" ] && TESTFILTER=*

# Run correct testcases and update status
for dir in "${testdir}"/*
do
    if test -n "${fastrun}"
    then
	case "${dir##*/}" in
	    "big1"|"big2"|"fib_bench"|"lastzero") continue;;
	    ${TESTFILTER})                                ;;
	    *)                                    continue;;
	esac
    else
	case "${dir##*/}" in
	    ${TESTFILTER}) ;;
	    *)     continue;;
	esac
    fi
    printheader
    runtest "${dir}"
done
printfooter
