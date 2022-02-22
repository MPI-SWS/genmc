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

#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include <llvm/Support/Debug.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <sstream>
#include <vector>

#define WARN_MESSAGE(msg) "WARNING: " << (msg)
#define ERROR_MESSAGE(msg) "ERROR: " << (msg)

#define WARN(msg) GenMCError::warn() << WARN_MESSAGE(msg)
#define WARN_ONCE(id, msg) GenMCError::warnOnce(id) << WARN_MESSAGE(msg)
#define WARN_ON(condition, msg) GenMCError::warnOn(condition) << WARN_MESSAGE(msg)
#define WARN_ON_ONCE(condition, id, msg) GenMCError::warnOnOnce(condition, id) << WARN_MESSAGE(msg)
#define ERROR(msg) ({ GenMCError::warn() << ERROR_MESSAGE(msg); exit(EUSER); })
#define ERROR_ON(condition, msg) ({ if (condition) { ERROR(msg); } })
#define BUG() do { \
	llvm::errs() << "BUG: Failure at " << __FILE__ ":" << __LINE__ \
		     << "/" << __func__ << "()!\n";		       \
	exit(EGENMC);						       \
	} while (0)
#define BUG_ON(condition) do { if (condition) BUG(); } while (0)

#ifdef ENABLE_GENMC_DEBUG
# define PRINT_BUGREPORT_INFO_ONCE(id, msg) BUG()
#else
# define PRINT_BUGREPORT_INFO_ONCE(id, msg)				\
	WARN_ONCE(id, msg "!\nPlease submit a bug report to "		\
		  PACKAGE_BUGREPORT "\n")
#endif

#ifdef ENABLE_GENMC_DEBUG
# define GENMC_DEBUG(s) do {			\
	s					\
	} while (0)
#else
# define GENMC_DEBUG(s) do {} while (0)
#endif

#define ECOMPILE 5
#define EGENMC   7
#define EUSER    17
#define EVERIFY  42

namespace GenMCError {

	llvm::raw_ostream &warn();
	llvm::raw_ostream &warnOnce(const std::string &warningID);
	llvm::raw_ostream &warnOn(bool condition);
	llvm::raw_ostream &warnOnOnce(bool condition, const std::string &warningID);

}

/* Useful for debugging (naive generalized printing doesn't work for llvm::raw_ostream) */
template<typename T>
llvm::raw_ostream& print(llvm::raw_ostream &out, const T &val)
{
	return (out << val);
}

template<typename T1, typename T2>
llvm::raw_ostream& print(llvm::raw_ostream &out, const std::pair<T1, T2> &val)
{
	return (out << "(" << val.first << ", " << val.second << ")");
}

template <typename Container>
std::string format(const Container &c)
{
	std::string str;
	llvm::raw_string_ostream out(str);

	out << "[ ";
	for (const auto &e : c)
		print(out, e) << " ";
	out << "]";
	return out.str();
}

template <typename T1, typename T2>
std::string format(const std::pair<T1, T2> &p)
{
	std::string str;
	llvm::raw_string_ostream out(str);
	print(out, p);
	return out.str();
}

#endif /* __ERROR_HPP__ */
