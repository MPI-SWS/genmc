/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "Config/Config.hpp"
#include "ExecutionGraph/Consistency/IMMChecker.hpp"
#include "ExecutionGraph/Consistency/RAChecker.hpp"
#include "ExecutionGraph/Consistency/RC11Checker.hpp"
#include "ExecutionGraph/Consistency/SCChecker.hpp"
#include "ExecutionGraph/Consistency/TSOChecker.hpp"

auto ConsistencyChecker::create(const Config *conf) -> std::unique_ptr<ConsistencyChecker>
{
#define CREATE_CHECKER(_model)                                                                     \
	case ModelType::_model:                                                                    \
		return std::make_unique<_model##Checker>(conf);

	switch (conf->model) {
		CREATE_CHECKER(SC);
		CREATE_CHECKER(TSO);
		CREATE_CHECKER(RA);
		CREATE_CHECKER(RC11);
		CREATE_CHECKER(IMM);
	default:
		BUG();
	}
}
