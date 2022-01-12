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

#include "Error.hpp"
#include <set>

llvm::raw_ostream &GenMCError::warn()
{
	return llvm::errs();
}

llvm::raw_ostream &GenMCError::warnOnce(const std::string &warningID)
{
	return warnOnOnce(true, warningID);
}

llvm::raw_ostream &GenMCError::warnOn(bool condition)
{
	static thread_local std::string buf;
	static thread_local llvm::raw_string_ostream s(buf);

	if (!condition) {
		buf.clear();
		return s;
	}

	return llvm::errs();
}

llvm::raw_ostream &GenMCError::warnOnOnce(bool condition, const std::string &warningID)
{
	static thread_local std::set<std::string> ids;
	static thread_local std::string buf;
	static thread_local llvm::raw_string_ostream s(buf);

	if (!condition || ids.count(warningID)) {
		buf.clear();
		return s;
	}

	ids.insert(warningID);
	return llvm::errs();
}
