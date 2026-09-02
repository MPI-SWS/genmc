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

#ifndef GENMC_DOTPRINT_HPP
#define GENMC_DOTPRINT_HPP

#include <map>
#include <ostream>

template <typename T>
inline void printDotEdge(std::ostream &os, const T &src, const T &dst,
			 const std::map<std::string, std::string> &attrs = {})
{
	os << std::format(R"("{}"->"{}")", src, dst);
	if (!attrs.empty()) {
		os << "[";
		for (const auto &it : attrs) {
			os << it.first << "=" << it.second << " ";
		}
		os << "]";
	}
	os << " ";
};

template <typename T>
inline void printlnDotEdge(std::ostream &os, const T &from, const T &dst,
			   const std::map<std::string, std::string> &attrs = {})
{
	printDotEdge(os, from, dst, std::move(attrs));
	os << "\n";
};

template <typename T>
inline void printDotEdge(std::ostream &os, const std::pair<T, T> &e,
			 const std::map<std::string, std::string> &attrs = {})
{
	printDotEdge(os, e.first, e.second, std::move(attrs));
}

template <typename T>
inline void printlnDotEdge(std::ostream &os, const std::pair<T, T> &e,
			   const std::map<std::string, std::string> &attrs = {})
{
	printlnDotEdge(os, e.first, e.second, std::move(attrs));
}

#endif /* GENMC_DOTPRINT_HPP */
