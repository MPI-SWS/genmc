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

#include "Verbosity.hpp"

auto operator<<(llvm::raw_ostream &s, VerbosityLevel l) -> llvm::raw_ostream &
{
	switch (l) {
	case VerbosityLevel::Error:
		s << "ERROR: ";
		break;
	case VerbosityLevel::Warning:
		s << "WARNING: ";
		break;
	case VerbosityLevel::Tip:
		s << "Tip: ";
		break;
	default:
		break;
	};
	return s;
}
