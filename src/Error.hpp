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

#define WARN(msg) GenMCError::warn() << (msg)
#define WARN_ONCE(id, msg) GenMCError::warnOnce(id) << (msg)
#define WARN_ON(condition, msg) GenMCError::warnOn(condition) << (msg)
#define WARN_ON_ONCE(condition, id, msg) GenMCError::warnOnOnce(condition, id) << (msg)

#define BUG() do { \
	llvm::errs() << "BUG: Failure at " << __FILE__ ":" << __LINE__ \
		     << "/" << __func__ << "()!\n";		       \
	abort();						       \
	} while (0)
#define BUG_ON(condition) do { if (condition) BUG(); } while (0)

/* TODO: Replace these codes with enum? */
#define ECOMPILE 1
#define EVERFAIL 42

namespace GenMCError {

	llvm::raw_ostream &warn();
	llvm::raw_ostream &warnOnce(const std::string &warningID);
	llvm::raw_ostream &warnOn(bool condition);
	llvm::raw_ostream &warnOnOnce(bool condition, const std::string &warningID);

}

#endif /* __ERROR_HPP__ */
