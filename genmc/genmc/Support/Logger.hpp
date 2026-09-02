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

#ifndef GENMC_LOGGER_HPP
#define GENMC_LOGGER_HPP

#include "genmc/Support/Verbosity.hpp"

#include <format>
#include <iostream>
#include <set>
#include <string>

/* Feature detection for std::print{ln}, fall back to <iostream> otherwise. */
#ifdef __cpp_lib_print
#include <print>
#endif /* __cpp_lib_print */

struct out_tag {};
struct err_tag {};

template <typename T>
concept StreamTag = std::same_as<T, out_tag> || std::same_as<T, err_tag>;

namespace detail {
template <StreamTag U> constexpr auto to_stream() -> std::ostream *
{
	if constexpr (std::is_same_v<U, err_tag>) {
		return &std::cerr;
	} else if constexpr (std::is_same_v<U, out_tag>) {
		return &std::cout;
	}
}
}; // namespace detail

template <StreamTag U> class Logger {

protected:
	/* So that derived classes can bypass the initial write to buffer */
	Logger(VerbosityLevel /*l*/, bool /*unused*/) : out_(::detail::to_stream<U>()) {}

	/* Allow derived classes to print the level the same way this class does. */
	void printLevel(VerbosityLevel l) const
	{
#ifdef __cpp_lib_print
		std::print(*out_, "{}", l);
#else
		*out_ << std::format("{}", l);
#endif /* __cpp_lib_print */
	}

public:
	Logger(VerbosityLevel l = VerbosityLevel::Warning) : out_(::detail::to_stream<U>())
	{
		if (std::is_same_v<U, err_tag>)
			printLevel(l);
	}

	template <typename... Args> void log(std::format_string<Args...> fmt, Args &&...args) const
	{
#ifdef __cpp_lib_print
		std::print(*out_, fmt, std::forward<Args>(args)...);
#else
		*out_ << std::format(fmt, std::forward<Args>(args)...);
#endif /* __cpp_lib_print */
		if constexpr (std::is_same_v<U, out_tag>)
			out_->flush();
	}

protected:
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	std::ostream *out_;
};

/** A logger that logs each message only once */
template <StreamTag U> class LoggerOnce : public Logger<U> {

public:
	/* In case multiple `LoggerOnce` are created with the same id, the one that was created
	 * first will be used. */
	LoggerOnce(const std::string &id, VerbosityLevel l = VerbosityLevel::Warning)
		: Logger<U>(l, true), shouldPrint_(!ids_.contains(id))
	{
		if (shouldPrint_) {
			ids_.insert(id);
			Logger<U>::printLevel(l);
		}
	}

	template <typename... Args> void log(std::format_string<Args...> fmt, Args &&...args) const
	{
		if (shouldPrint_)
			Logger<U>::log(fmt, std::forward<Args>(args)...);
	}

private:
	const bool shouldPrint_{}; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	static thread_local inline std::set<std::string> ids_;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline VerbosityLevel logLevel = VerbosityLevel::Tip;

/** Logs a formatted message (adds newline), in `std::format` style */
#define LOG(level, fmt, ...)                                                                       \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		Logger<err_tag>(level).log(fmt "\n", ##__VA_ARGS__)

/** Logs a formatted message (adds newline), but only once per `id`. */
#define LOG_ONCE(id, level, fmt, ...)                                                              \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		LoggerOnce<err_tag>(id, level).log(fmt "\n", ##__VA_ARGS__)

/** Prints a formatted message to stdout (no newline) */
#define PRINT(level, fmt, ...)                                                                     \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		Logger<out_tag>(level).log(fmt, ##__VA_ARGS__)

#endif /* GENMC_LOGGER_HPP */
