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

#ifndef GENMC_THREAD_PINNER_HPP
#define GENMC_THREAD_PINNER_HPP

#include <thread>
#include <vector>
#ifdef HAVE_LIBHWLOC
#include <hwloc.h>
#endif

/*******************************************************************************
 **                           ThreadPinner Class
 ******************************************************************************/

/** A class responsible for pinning threads to CPUs */

#ifdef HAVE_LIBHWLOC

class ThreadPinner {

public:
	/*** Constructor ***/
	explicit ThreadPinner(unsigned int n);
	ThreadPinner() = delete;
	ThreadPinner(const ThreadPinner &) = delete;

	void pin(std::thread &t, unsigned int cpu);

	/*** Destructor ***/
	~ThreadPinner()
	{
		hwloc_topology_destroy(topology);
		for (auto set : cpusets)
			hwloc_bitmap_free(set);
	}

private:
	unsigned int numTasks;
	hwloc_topology_t topology;
	std::vector<hwloc_cpuset_t> cpusets;
};

#else /* !HAVE_LIBHWLOC */

class ThreadPinner {

public:
	/*** Constructor ***/
	explicit ThreadPinner(unsigned int n) {}
	ThreadPinner() = delete;
	ThreadPinner(const ThreadPinner &) = delete;

	void pin(std::thread &t, unsigned int cpu) {}

	/*** Destructor ***/
	~ThreadPinner() {}
};

#endif /* HAVE_LIBHWLOC */

#endif /* GENMC_THREAD_PINNER_HPP */
