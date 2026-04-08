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

#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/Consistency/IMMChecker.hpp"
#include "genmc/Execution/Consistency/RAChecker.hpp"
#include "genmc/Execution/Consistency/RC11Checker.hpp"
#include "genmc/Execution/Consistency/SCChecker.hpp"
#include "genmc/Execution/Consistency/TSOChecker.hpp"
#include "genmc/Verification/Config.hpp"

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
		UNREACHABLE();
	}
}
