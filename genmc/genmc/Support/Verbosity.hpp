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

#include <format>

enum class VerbosityLevel : std::uint8_t {
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

template <> struct std::formatter<VerbosityLevel> : std::formatter<std::string_view> {
	constexpr auto parse(std::format_parse_context &ctx)
	{
		return std::formatter<std::string_view>::parse(ctx);
	}

	auto format(const VerbosityLevel l, std::format_context &ctx) const
	{
		switch (l) {
		case VerbosityLevel::Error:
			return std::format_to(ctx.out(), "ERROR: ");
		case VerbosityLevel::Warning:
			return std::format_to(ctx.out(), "WARNING: ");
		case VerbosityLevel::Tip:
			return std::format_to(ctx.out(), "Tip: ");
		default:
			return std::format_to(ctx.out(), "");
		}
	}
};

#endif /* GENMC_VERBOSITY_HPP */
