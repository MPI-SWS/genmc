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

#ifndef GENMC_MODULE_ID_HPP
#define GENMC_MODULE_ID_HPP

#include <unordered_map>

namespace llvm {
class Value;
}

/**
 * Contains (unique) identification information for some of the module's
 * crucial components (e.g., GVs, functions, etc)
 */
struct ModuleID {
	using ID = unsigned int;

	std::unordered_map<ID, const llvm::Value *> IDV;
	std::unordered_map<const llvm::Value *, ID> VID;

	void clear()
	{
		IDV.clear();
		VID.clear();
	}
};

#endif /* GENMC_MODULE_ID_HPP */
