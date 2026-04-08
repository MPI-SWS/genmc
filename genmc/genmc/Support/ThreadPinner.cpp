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
