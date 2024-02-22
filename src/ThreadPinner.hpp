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

#ifndef __THREAD_PINNER_HPP__
#define __THREAD_PINNER_HPP__

#include "config.h"
#include <thread>
#include <vector>
#ifdef HAVE_LIBHWLOC
#include <hwloc.h>
#endif

/*******************************************************************************
 **                           ThreadPinner Class
 ******************************************************************************/

/* A class responsible for pinning threads to CPUs */

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

#endif /* __THREAD_PINNER_HPP__ */
