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

#ifndef GENMC_DOTPRINT_HPP
#define GENMC_DOTPRINT_HPP

#include <llvm/Support/raw_ostream.h>

#include <map>

template <typename T>
inline void printDotEdge(llvm::raw_ostream &os, const T &from, const T &to,
			 std::map<std::string, std::string> &&attrs = {})
{
	os << "\"" << from << "\"" << "->" << "\"" << to << "\"";
	if (!attrs.empty()) {
		os << "[";
		for (auto const &it : attrs) {
			os << it.first << "=" << it.second << " ";
		}
		os << "]";
	}
	os << " ";
};

template <typename T>
inline void printlnDotEdge(llvm::raw_ostream &os, const T &from, const T &to,
			   std::map<std::string, std::string> &&attrs = {})
{
	printDotEdge(os, from, to, std::move(attrs));
	os << "\n";
};

template <typename T>
inline void printDotEdge(llvm::raw_ostream &os, const std::pair<T, T> &e,
			 std::map<std::string, std::string> &&attrs = {})
{
	printDotEdge(os, e.first, e.second, std::move(attrs));
}

template <typename T>
inline void printlnDotEdge(llvm::raw_ostream &os, const std::pair<T, T> &e,
			   std::map<std::string, std::string> &&attrs = {})
{
	printlnDotEdge(os, e.first, e.second, std::move(attrs));
}

#endif /* GENMC_DOTPRINT_HPP */
