/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/Consistency/IMMChecker.hpp"
#include "ExecutionGraph/Consistency/RAChecker.hpp"
#include "ExecutionGraph/Consistency/RC11Checker.hpp"
#include "ExecutionGraph/Consistency/SCChecker.hpp"
#include "ExecutionGraph/Consistency/TSOChecker.hpp"
#include "Verification/Config.hpp"

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
