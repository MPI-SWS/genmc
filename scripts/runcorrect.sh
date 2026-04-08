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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GenMC="${GenMC:-$DIR/../RelWithDebInfo/bin/genmc}"

source "${DIR}/terminal.sh"

model="${model:-rc11}"
coherence="${coherence:-mo}"
testdir="${testdir:-${DIR}/../tests/correct/litmus}"

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
    printf "| ${CYAN}%-22s${NC} | ${CYAN}%-6s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-8s${NC} | ${CYAN}%-8s${NC} |\n" \
	   "Testcase" "Result" "Execs" "Blocked" "Avg.time"
    printline
}

print_condensed_header() {
    if test -z "${header_printed}"
    then
       # Print table's header
       echo ''; printline
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
#     echo''
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
	printf "| ${POWDER_BLUE}%-22s${NC} | " "${dir##*/}${n}"
    fi
}

print_variant_debug_results() {
   if test -n "${outcome_failure}"
    then
	outcome="${RED}ERROR ${NC}"
	result=1
    elif test -n "${failure}"
    then
	outcome="${LIME_YELLOW}FAILED${NC}"
	if [[ -n "${explored_failed}" ]]
	then
	    explored="${explored_failed}/${expected}"
	fi
	if [[ -n "${blocked_failed}" ]]
	then
	    blocked="${blocked_failed}/${expected_blocked}"
	fi
	result=1
    else
	outcome="${GREEN}SAFE  ${NC}"
    fi
    printf "${outcome} | % 8s | % 8s | % 8s |\n" \
	   "${explored}" "${blocked}" "${average_time}"

    if test -n "${failure}" -o -n "${outcome_failure}"
    then
	echo "${failure_output}"
	printf "Test command: ${cmd}\n"
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

    local i
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
    outcome_failure=""
    failure_output=""
    genmc_args=$(echo "$test_args" | cut -f1 -d'|')
    clang_args=$(echo "$test_args" | cut -f2 -d'|')
    for t in $dir/variants/*.c $dir/variants/*.cpp $dir/variants/*.ll
    do
	vars=$((vars+1))
	cmd="${GenMC} ${GENMCFLAGS} -${model} -disable-mm-detector $genmc_args -- ${CFLAGS} ${clang_args} ${t}"
	output=`${cmd} 2>&1`
	if test "$?" -ne 0
	then
	    failure_output="${output}"
	    outcome_failure=1
	fi
	explored=`echo "${output}" | awk '/explored/ { print $6 }'`
	blocked=`echo "${output}" | awk '/blocked/ { print $6 }'`
	slacked=`echo "${output}" | awk '/exceeded bound/ { print substr($7, 2) }'`
	test -z "${slacked}" && slacked=0
	explored="$((${explored} - ${slacked}))"
	explored_failed=""
	blocked_failed=""
	time=`echo "${output}" | awk '/wall-clock/ { print substr($4, 1, length($4)-1) }'`
	time="${time}" && [[ -z "${time}" ]] && time=0 # if pattern was NOT found
	test_time=`echo "${test_time}+${time}" | bc -l`
	runtime=`echo "scale=2; ${runtime}+${time}" | bc -l`
	expected="${expected:-${explored}}"
	if test -n "${expected_bound}"
	then
            expected="${expected_bound}"
	fi
	if test "${expected}" != "${explored}"
	then
	    explored_failed="${explored}"
	    failure_output="${output}"
	    failure=1
	fi
	# Only run the first variants when testing round-robin bounding
	if test "${bound_type}" = "round"
	then
            break
	fi
    done
    if test -n "${check_blocked}" -a "${blocked}" != "${expected_blocked}"
    then
	blocked_failed="${blocked}"
	if [[ -z "${failure_output}" ]]
	then
	    failure_output="${output}"
	fi
	failure=1
    fi
    average_time=`echo "scale=2; ${test_time}/${vars}" | bc -l`
    print_variant_results
}

runtest() {
    dir=$1
    # Skip empty dirs, dirs without .{c,r}b file (bounding), dirs with no .expected
    if [ -z "$(ls ${dir})" -o ! -d "${dir}/variants" ]
    then
	return
    fi
    test ! -f "${dir}/expected.${model}.${coherence}.in" && return
    bound_file=""
    test "${bound_type}" = "context" && bound_file="${dir}/expected.cb.in"
    test "${bound_type}" = "round" && bound_file="${dir}/expected.rb.in"
    test ! -f "${bound_file}" -a -n "${bound_type}" && return

    if test -f "${dir}/args.${model}.${coherence}.in"
    then
	varNum=0
	while read test_args <&3 && read expected <&4; do
	    varNum=$((varNum + 1))
	    n="/`echo ${test_args} |
                 awk ' { if (match($0, /-DN=[0-9]+/)) print substr($0, RSTART+4, RLENGTH-4) } '`"
	    expected_blocked="" && [[ -f "${dir}/blocked.${model}.${coherence}.in" ]] &&
		expected_blocked=`sed "${varNum}q;d" "${dir}/blocked.${model}.${coherence}.in"`
	    expected_blocked="${expected_blocked}" && [[ -f "${dir}/blocked.${model}.${coherence}.in-${LLVM_VERSION}" ]] &&
		expected_blocked=`sed "${varNum}q;d" "${dir}/blocked.${model}.${coherence}.in-${LLVM_VERSION}"`
            if test -n "${bound_type}"
            then
		local i
		i=0
		for bound in `sed "${varNum}q;d" "${bound_file}"`
		do
                    expected_bound="${bound}" GENMCFLAGS="${GENMCFLAGS} --bound-type=${bound_type} --bound $i" runvariants
                    i=$((i + 1))
		done
            else
		runvariants
            fi
	done 3<"${dir}/args.${model}.${coherence}.in" 4<"${dir}/expected.${model}.${coherence}.in"
    else
	test_args=""
	n=""
	expected=`head -n 1 "${dir}/expected.${model}.${coherence}.in"`
	expected_blocked="" && [[ -f "${dir}/blocked.${model}.${coherence}.in" ]] &&
	    expected_blocked=`head -n 1 "${dir}/blocked.${model}.${coherence}.in"`
	expected_blocked=$expected_blocked && [[ -f "${dir}/blocked.${model}.${coherence}.in-${LLVM_VERSION}" ]] &&
	    expected_blocked=`sed "${varNum}q;d" "${dir}/blocked.${model}.${coherence}.in-${LLVM_VERSION}"`
	if test -n "${bound_type}"
	then
            local i
            i=0
            for bound in `head -n1 "${bound_file}"`
            do
		expected_bound="${bound}" GENMCFLAGS="${GENMCFLAGS} --bound-type=${bound_type} --bound $i" runvariants
		i=$((i + 1))
            done
	else
            runvariants
	fi
    fi
}

[ -z "${TESTFILTER}" ] && TESTFILTER=*

# Run correct testcases and update status
printheader
for dir in "${testdir}"/*
do
	case "${dir##*/}" in
	    ${TESTFILTER}) ;;
	    *)     continue;;
	esac
    if test -n "${fastrun}"
    then
	case "${dir##*/}" in
	    "spin-no-bisim")           continue;;
	    "big1"|"big2"|"fib_bench"|"lastzero")           continue;;
	    "big0"|"barrier2")         test -n "${bound_type}" && continue;;
	    *)                                    		;;
	esac
    fi
    runtest "${dir}"
done
printfooter
