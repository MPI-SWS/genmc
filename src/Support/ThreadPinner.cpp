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

#include "ThreadPinner.hpp"
#include "Error.hpp"

#ifdef HAVE_LIBHWLOC

ThreadPinner::ThreadPinner(unsigned int n) : numTasks(n)
{
	if (hwloc_topology_init(&topology) < 0 || hwloc_topology_load(topology) < 0)
		ERROR("Error during topology initialization\n");
	cpusets.resize(n);

	hwloc_obj_t root = hwloc_get_root_obj(topology);
	int result = hwloc_distrib(topology, &root, 1, cpusets.data(), numTasks,
				   std::numeric_limits<int>::max(), 0u);
	if (result)
		ERROR("Error during topology distribution\n");

	/* Minimize migration costs */
	for (int i = 0; i < numTasks; i++)
		hwloc_bitmap_singlify(cpusets[i]);
}

void ThreadPinner::pin(std::thread &t, unsigned int cpu)
{
	if (hwloc_set_thread_cpubind(topology, t.native_handle(), cpusets[cpu], 0) == -1)
		ERROR("Error during thread pinning\n");
}

#endif /* HAVE_LIBHWLOC */
