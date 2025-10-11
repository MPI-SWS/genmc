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

#ifndef GENMC_THREAD_INFO_HPP
#define GENMC_THREAD_INFO_HPP

#include "SVal.hpp"

/** Information about a thread (thread id, SR, etc.) */
struct ThreadInfo {
	int id{};	      /**< Thread identifier */
	int parentId = -1;    /**< ID of parent thread */
	unsigned int funId{}; /**< ID for thread body (function argument to pthread_create) */
	SVal arg;	      /**< parameter argument to pthread_create */
	int symmId = -1;      /**< ID of previous symmetric thread if any, -1 otherwise */

	ThreadInfo() = default;
	ThreadInfo(int id, int parentId, unsigned funId, SVal arg, int symm = -1)
		: id(id), parentId(parentId), funId(funId), arg(arg), symmId(symm)
	{}
};

#endif /* GENMC_THREAD_INFO_HPP */
