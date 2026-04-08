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

#ifndef GENMC_HASH_HPP
#define GENMC_HASH_HPP

#include <functional>

template <class T> inline void hash_combine(std::size_t &seed, const T &v)
{
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T1, typename T2> struct PairHasher {
	auto operator()(const std::pair<T1, T2> &p) const -> size_t
	{
		std::size_t hash = 0;
		hash_combine(hash, p.first);
		hash_combine(hash, p.second);
		return hash;
	}
};

#endif /* GENMC_HASH_HPP */
