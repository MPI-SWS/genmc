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

#ifndef GENMC_MEMORY_MODEL_HPP
#define GENMC_MEMORY_MODEL_HPP

#include "Support/Error.hpp"

#include <cstdint>
#include <string>

enum class ModelType : std::uint8_t { SC = 0, TSO = 1, RA = 2, RC11 = 3, IMM = 4 };

inline auto operator<<(llvm::raw_ostream &s, const ModelType &model) -> llvm::raw_ostream &
{
	switch (model) {
	case ModelType::SC:
		return s << "SC";
	case ModelType::TSO:
		return s << "TSO";
	case ModelType::RA:
		return s << "RA";
	case ModelType::RC11:
		return s << "RC11";
	case ModelType::IMM:
		return s << "IMM";
	default:
		PRINT_BUGREPORT_INFO_ONCE("missing-model-name", "Unknown memory model name");
		return s;
	}
}

inline auto isStrongerThan(ModelType model, ModelType other) -> bool
{
	static const bool lookup[5][5] = {
		//          SC     TSO    RA     RC11   IMM
		/* SC   */ {false, true, true, true, true},
		/* TSO  */ {false, false, true, true, true},
		/* RA   */ {false, false, false, true, true},
		/* RC11 */ {false, false, false, false, true},
		/* IMM  */ {false, false, false, false, false},
	};
	return lookup[static_cast<size_t>(model)][static_cast<size_t>(other)];
}

#endif /* GENMC_MEMORY_MODEL_HPP */
