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

#include "ThreadPool.hpp"

void ThreadPool::addWorker(unsigned int i, std::unique_ptr<GenMCDriver> d)
{
	using ThreadT = std::packaged_task<
		GenMCDriver::Result(unsigned int, std::unique_ptr<GenMCDriver> driver)>;

	ThreadT t([this](unsigned int i, std::unique_ptr<GenMCDriver> driver){
		while (true) {
			auto state = popTask();

			/* If the state is empty, nothing left to do */
			if (!state)
				break;

			/* Prepare the driver and start the exploration */
			driver->setSharedState(std::move(state));
			driver->run();

			/* If that was the last task, notify everyone */
			if (decRemainingTasks() == 0) {
				halt();
				break;
			}
		}
		return driver->getResult();
	});

	results_.push_back(std::move(t.get_future()));

	workers_.push_back(std::thread(std::move(t), i, std::move(d)));
	pinner_.pin(workers_.back(), i);
	return;
}

void ThreadPool::submit(std::unique_ptr<TaskT> t)
{
	std::lock_guard<std::mutex> lock(stateMtx_);
	incRemainingTasks();
	queue_.push(std::move(t));
	stateCV_.notify_one();
	return;
}

std::unique_ptr<ThreadPool::TaskT> ThreadPool::tryPopPoolQueue()
{
	return queue_.tryPop();
}

std::unique_ptr<ThreadPool::TaskT> ThreadPool::tryStealOtherQueue()
{
	/* TODO: Implement work-stealing */
	return nullptr;
}

std::unique_ptr<ThreadPool::TaskT> ThreadPool::popTask()
{
	while (true) {
		if (auto t = tryPopPoolQueue())
			return t;
		else if (auto t = tryStealOtherQueue())
			return t;

		if (shouldHalt() || getRemainingTasks() == 0)
			return nullptr;

		std::unique_lock<std::mutex> lock(stateMtx_);
		stateCV_.wait(lock);
	}
	return nullptr;
}

std::vector<std::future<GenMCDriver::Result>> ThreadPool::waitForTasks()
{
	while (!shouldHalt() && getRemainingTasks())
		std::this_thread::yield();

	return std::move(results_);
}
