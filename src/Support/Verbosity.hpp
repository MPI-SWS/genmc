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

#ifndef GENMC_VERBOSITY_HPP
#define GENMC_VERBOSITY_HPP

#include <llvm/Support/raw_ostream.h>

enum class VerbosityLevel {
	Quiet,
	Error,
	Warning,
	Tip,
#ifdef ENABLE_GENMC_DEBUG
	Debug1,
	Debug2,
	Debug3,
	Debug4,
#endif
};

auto operator<<(llvm::raw_ostream &rhs, VerbosityLevel l) -> llvm::raw_ostream &;

#endif /* GENMC_VERBOSITY_HPP */
