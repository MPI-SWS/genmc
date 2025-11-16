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

#ifndef GENMC_ERROR_HPP
#define GENMC_ERROR_HPP

#include "config.h"

#include "Support/Logger.hpp"

#include <format>

#include <string>
#include <system_error>

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

/**** Error codes ****/

#define ECOMPILE 5
#define EGENMC 7
#define EUSER 17
#define EVERIFY 42

/**** Warning reporting ****/

#define WARN_ON(condition, fmt, ...)                                                               \
	do {                                                                                       \
		if (condition) {                                                                   \
			LOG(VerbosityLevel::Warning, fmt, ##__VA_ARGS__);                          \
		}                                                                                  \
	} while (0)

#define WARN(fmt, ...) WARN_ON(true, fmt, ##__VA_ARGS__)

#define WARN_ON_ONCE(condition, id, fmt, ...)                                                      \
	do {                                                                                       \
		if (condition) {                                                                   \
			LOG_ONCE(id, VerbosityLevel::Warning, fmt, ##__VA_ARGS__);                 \
		}                                                                                  \
	} while (0)

#define WARN_ONCE(id, fmt, ...) WARN_ON_ONCE(true, id, fmt, ##__VA_ARGS__)

/**** Error reporting ****/

#define ERROR(fmt, ...)                                                                            \
	({                                                                                         \
		LOG(VerbosityLevel::Error, fmt, ##__VA_ARGS__);                                    \
		exit(EUSER);                                                                       \
	})

#define ERROR_ON(condition, fmt, ...)                                                              \
	({                                                                                         \
		if (condition) {                                                                   \
			ERROR(fmt, ##__VA_ARGS__);                                                 \
		}                                                                                  \
	})

#ifdef ENABLE_GENMC_DEBUG
#define BUG()                                                                                      \
	do {                                                                                       \
		LOG(VerbosityLevel::Error, "BUG: Internal failure at {}:{}:{}()!\n", __FILE__,     \
		    STRINGIFY(__LINE__), __func__);                                                \
		abort();                                                                           \
	} while (0)
#else
#define BUG() __builtin_unreachable()
#endif
#ifdef ENABLE_GENMC_DEBUG
#define BUG_ON(condition)                                                                          \
	do {                                                                                       \
		if (condition)                                                                     \
			BUG();                                                                     \
	} while (0)
#else
#define BUG_ON(condition)                                                                          \
	do {                                                                                       \
	} while (0)
#endif

#ifdef ENABLE_GENMC_DEBUG
#define PRINT_BUGREPORT_INFO_ONCE(id, msg) BUG()
#else
#define PRINT_BUGREPORT_INFO_ONCE(id, msg)                                                         \
	WARN_ONCE(id, "{}!\nPlease submit a bug report to {}", msg, PACKAGE_BUGREPORT)
#endif

#ifdef ENABLE_GENMC_DEBUG
#define GENMC_DEBUG(s)                                                                             \
	do {                                                                                       \
		s                                                                                  \
	} while (0)
#else
#define GENMC_DEBUG(s)                                                                             \
	do {                                                                                       \
	} while (0)
#endif

inline void handleFSError(std::error_code const &err, std::string const &details = "")
{
	ERROR_ON(err, "Filesystem error: {}\n{}", err.message(), details.empty() ? "" : details);
}

/**** Formatting helpers ****/

/** Make `std::pair` formattable with `std::format`. */
template <typename T1, typename T2> struct std::formatter<std::pair<T1, T2>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const std::pair<T1, T2> &p, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "({}, {})", p.first, p.second);
	}
};

/** Make containers formattable with `std::format` if it isn't an already formattable type.
 *  One formatter is specialized to print `EventLabel *` in a container. */

class EventLabel;

template <typename Container>
requires std::ranges::range<Container> && (!std::is_array_v<Container>) &&
	 (!std::convertible_to<Container, std::string_view>) &&
	 (!std::derived_from<std::remove_pointer_t<std::ranges::range_value_t<Container>>,
			     EventLabel>)
struct std::formatter<Container> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Container &c, std::format_context &ctx) const
	{
		auto out = std::format_to(ctx.out(), "[ ");
		for (const auto &elem : c) {
			out = std::format_to(out, "{} ", elem);
		}
		return std::format_to(out, "]");
	}
};

template <typename Container>
requires std::ranges::range<Container> && (!std::is_array_v<Container>) &&
	 (!std::convertible_to<Container, std::string_view>) &&
	 std::derived_from<std::remove_pointer_t<std::ranges::range_value_t<Container>>, EventLabel>
struct std::formatter<Container> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Container &c, std::format_context &ctx) const
	{
		auto out = std::format_to(ctx.out(), "[ ");
		for (const auto *lab : c) {
			if (lab)
				out = std::format_to(out, "{} ", *lab);
			else
				out = std::format_to(out, "null ");
		}
		return std::format_to(out, "]");
	}
};

#endif /* GENMC_ERROR_HPP */
