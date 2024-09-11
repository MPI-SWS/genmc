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

#ifndef GENMC_MEMORY_MODEL_HPP
#define GENMC_MEMORY_MODEL_HPP

#include "Support/Error.hpp"
#include "config.h"

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
