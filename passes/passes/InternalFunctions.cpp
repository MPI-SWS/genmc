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

#include "passes/InternalFunctions.hpp"

const std::unordered_map<std::string, InternalFunctions> internalFunNames = {
#define HANDLE_FUNCTION(NUM, FUN, NAME) {FUN, InternalFunctions::NAME},
#include "passes/InternalFunction.def"

	/* Some extra C++ calls */
	{"_Znwm", InternalFunctions::Malloc},
	{"_ZdlPv", InternalFunctions::Free},
};
