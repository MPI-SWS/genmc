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

#include "Support/Verbosity.hpp"
#include <llvm/Support/raw_ostream.h>
#include <set>

struct out_tag {};
struct err_tag {};

template <typename U> class Logger {

protected:
	/* So that derived classes can bypass the initial write to buffer */
	Logger(VerbosityLevel l, bool) : buffer_(str_) {}

public:
	Logger(VerbosityLevel l = VerbosityLevel::Warning) : buffer_(str_)
	{
		if (std::is_same_v<U, err_tag>)
			buffer_ << l;
	}

	template <typename T> auto operator<<(const T &msg) -> Logger &
	{
		buffer_ << msg;
		return *this;
	}

	~Logger()
	{
		/*
		 * 1. We don't have to flush (automatic for stderr; don't care for stdout)
		 * 2. Stream ops are atomic according to POSIX:
		 *    http://www.gnu.org/s/libc/manual/html_node/Streams-and-Threads.html
		 */
		if (std::is_same_v<U, out_tag>)
			llvm::outs() << buffer_.str();
		else
			llvm::errs() << buffer_.str();
	}

protected:
	std::string str_;
	llvm::raw_string_ostream buffer_;
};

/** A logger that logs each message only once */
template <typename U> class LoggerOnce : public Logger<U> {

public:
	/* In principle, we could just append to the buffer and check whether the
	 * ID has been encountered before at destruction. This class is extra verbose
	 * so that we avoid writing to the buffer altogether if we have seen this ID */
	LoggerOnce(const std::string &id, VerbosityLevel l = VerbosityLevel::Warning)
		: Logger<U>(l, true), id(id)
	{
		if (!ids.count(id))
			this->buffer_ << l;
	}

	template <typename T> auto operator<<(const T &msg) -> LoggerOnce &
	{
		if (ids.count(id)) {
			return *this;
		}
		return static_cast<LoggerOnce &>(Logger<U>::operator<<(msg));
	}

	~LoggerOnce()
	{
		if (!ids.count(id)) {
			ids.insert(id);
		}
	}

private:
	const std::string &id;
	static thread_local inline std::set<std::string> ids;
};

inline VerbosityLevel logLevel = VerbosityLevel::Tip;

#define LOG(level)                                                                                 \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		Logger<err_tag>(level)

#define LOG_ONCE(id, level)                                                                        \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		LoggerOnce<err_tag>(id, level)

#define PRINT(level)                                                                               \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		Logger<out_tag>(level)

#define PRINT_ONCE(level)                                                                          \
	if (level > logLevel)                                                                      \
		;                                                                                  \
	else                                                                                       \
		Logger<out_tag>(level)

#endif /* GENMC_LOGGER_HPP */
