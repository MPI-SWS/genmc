#!/bin/bash

# Randomizes test running
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
source "${DIR}/terminal.sh"

iterations="${iterations:-10}"

for i in `seq 1 "${iterations}"`
do
    echo -ne "Iteration: $i"
    res=$(GENMCFLAGS="${GENMCFLAGS} -schedule-policy=arbitrary -print-schedule-seed" "${DIR}/driver.sh" --fast --debug)
    if [[ $? -ne 0 ]]
    then
	echo "${RED} Error detected! ${NC}"
	echo "$res"
	exit 1
    fi
    echo -ne "\r"
    sleep .$[ ( $RANDOM % 20 ) + 1 ]s
done
