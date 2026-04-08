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

#include "genmc/Support/Error.hpp"

#include <cstdint>
#include <format>
#include <string>

enum class ModelType : std::uint8_t { SC = 0, TSO = 1, RA = 2, RC11 = 3, IMM = 4 };

/** Make `ModelType` formattable with `std::format`. */
template <> struct std::formatter<ModelType> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const ModelType &model, std::format_context &ctx) const
	{
		switch (model) {
		case ModelType::SC:
			return std::format_to(ctx.out(), "SC");
		case ModelType::TSO:
			return std::format_to(ctx.out(), "TSO");
		case ModelType::RA:
			return std::format_to(ctx.out(), "RA");
		case ModelType::RC11:
			return std::format_to(ctx.out(), "RC11");
		case ModelType::IMM:
			return std::format_to(ctx.out(), "IMM");
		default:
			PRINT_BUGREPORT_INFO_ONCE("missing-model-name",
						  "Unknown memory model name");
			return std::format_to(ctx.out(), "UNKNOWN");
		}
	}
};

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
