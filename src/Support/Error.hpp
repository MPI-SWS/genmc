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

#ifndef GENMC_ERROR_HPP
#define GENMC_ERROR_HPP

#include "Logger.hpp"
#include <llvm/Support/Debug.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>
#include <string>
#include <vector>

#define ECOMPILE 5
#define EGENMC 7
#define EUSER 17
#define EVERIFY 42

#define WARN_ON(condition, msg)                                                                    \
	if (condition) {                                                                           \
		LOG(VerbosityLevel::Warning) << msg;                                               \
	}
#define WARN(msg) WARN_ON(true, msg)

#define WARN_ON_ONCE(condition, id, msg)                                                           \
	if (condition) {                                                                           \
		LOG_ONCE(id, VerbosityLevel::Warning) << msg;                                      \
	}
#define WARN_ONCE(id, msg) WARN_ON_ONCE(true, id, msg)

#define ERROR(msg)                                                                                 \
	({                                                                                         \
		LOG(VerbosityLevel::Error) << msg;                                                 \
		exit(EUSER);                                                                       \
	})
#define ERROR_ON(condition, msg)                                                                   \
	({                                                                                         \
		if (condition) {                                                                   \
			ERROR(msg);                                                                \
		}                                                                                  \
	})
#define BUG()                                                                                      \
	do {                                                                                       \
		LOG(VerbosityLevel::Error) << "BUG: Failure at " << __FILE__ ":" << __LINE__       \
					   << "/" << __func__ << "()!\n";                          \
		exit(EGENMC);                                                                      \
	} while (0)
#define BUG_ON(condition)                                                                          \
	do {                                                                                       \
		if (condition)                                                                     \
			BUG();                                                                     \
	} while (0)

#ifdef ENABLE_GENMC_DEBUG
#define PRINT_BUGREPORT_INFO_ONCE(id, msg) BUG()
#else
#define PRINT_BUGREPORT_INFO_ONCE(id, msg)                                                         \
	WARN_ONCE(id, msg "!\nPlease submit a bug report to " PACKAGE_BUGREPORT "\n")
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

/* Useful for debugging (naive generalized printing doesn't work for llvm::raw_ostream) */
template <typename T> auto print(llvm::raw_ostream &out, const T &val) -> llvm::raw_ostream &
{
	return (out << val);
}

template <typename T1, typename T2>
auto print(llvm::raw_ostream &out, const std::pair<T1, T2> &val) -> llvm::raw_ostream &
{
	return (out << "(" << val.first << ", " << val.second << ")");
}

template <typename Container> auto format(const Container &c) -> std::string
{
	std::string str;
	llvm::raw_string_ostream out(str);

	out << "[ ";
	for (const auto &e : c)
		print(out, e) << " ";
	out << "]";
	return out.str();
}

template <typename T1, typename T2> auto format(const std::pair<T1, T2> &p) -> std::string
{
	std::string str;
	llvm::raw_string_ostream out(str);
	print(out, p);
	return out.str();
}

#endif /* GENMC_ERROR_HPP */
