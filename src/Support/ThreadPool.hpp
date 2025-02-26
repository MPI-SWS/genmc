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

#ifndef GENMC_THREAD_POOL_HPP
#define GENMC_THREAD_POOL_HPP

#include "ExecutionGraph/ExecutionGraph.hpp"
#include "Static/LLVMModule.hpp"
#include "Support/Error.hpp"
#include "Support/ThreadPinner.hpp"
#include "Verification/GenMCDriver.hpp"
#include "config.h"
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <atomic>
#include <future>
#include <memory>
#include <thread>

/*******************************************************************************
 **                           GlobalWorkQueue Class
 ******************************************************************************/

/* Represents the global workqueue shared among threads. */
class GlobalWorkQueue {

public:
	using ItemT = std::unique_ptr<GenMCDriver::Execution>;
	using QueueT = std::vector<ItemT>;

	/*** Constructors ***/

	GlobalWorkQueue() {}
	GlobalWorkQueue(const GlobalWorkQueue &) = delete;

	/*** Queue operations ***/

	/* Returns true if the queue is empty */
	bool empty()
	{
		std::lock_guard<std::mutex> lock(qMutex);
		return queue.empty();
	}

	/* Adds a new item to the queue */
	void push(ItemT item)
	{
		std::lock_guard<std::mutex> lock(qMutex);
		queue.push_back(std::move(item));
	}

	/* Tries to pop an item from the queue */
	ItemT tryPop()
	{
		std::lock_guard<std::mutex> lock(qMutex);
		if (queue.empty())
			return nullptr;
		auto item = std::move(queue.back());
		queue.pop_back();
		return item;
	}

private:
	/* The actual queue data structure */
	QueueT queue;

	/* Protection against unsynchronized accesses */
	std::mutex qMutex;
};

/*******************************************************************************
 **                           ThreadJoiner Class
 ******************************************************************************/

/* A class responsible for joining a bunch of threads */
class ThreadJoiner {

public:
	/*** Constructor ***/
	explicit ThreadJoiner(std::vector<std::thread> &ts) : threads(ts) {}
	ThreadJoiner() = delete;
	ThreadJoiner(const ThreadJoiner &) = delete;

	/*** Destructor ***/
	~ThreadJoiner()
	{
		for (auto i = 0u; i < threads.size(); i++) {
			if (threads[i].joinable())
				threads[i].join();
		}
	}

private:
	/* The threads to join */
	std::vector<std::thread> &threads;
};

/*******************************************************************************
 **                            ThreadPool Class
 ******************************************************************************/

/*
 * A class responsible for creating and managing a pool of threads, with tasks
 * submitted dynamically to the threads for execution. Each thread will have
 * each own exploration driver so that they will able to execute the submitted
 * (exploration) tasks concurrently.
 */
class ThreadPool {

public:
	using GlobalQueueT = GlobalWorkQueue;
	using TaskT = GlobalQueueT::ItemT;

	/*** Constructors ***/

	ThreadPool(const std::shared_ptr<const Config> conf,
		   const std::unique_ptr<llvm::Module> &mod, const std::unique_ptr<ModuleInfo> &MI)
		: numWorkers_(conf->threads), pinner_(numWorkers_), joiner_(workers_)
	{
		numWorkers_ = conf->threads;

		/* Set global variables before spawning the threads */
		shouldHalt_.store(false);
		remainingTasks_.store(0);

		for (auto i = 0u; i < numWorkers_; i++) {
			contexts_.push_back(std::make_unique<llvm::LLVMContext>());
			auto newmod = LLVMModule::cloneModule(mod, contexts_.back());
			auto newMI = MI->clone(*newmod);

			auto dw = GenMCDriver::create(conf, std::move(newmod), std::move(newMI),
						      this);
			if (i == 0)
				submit(std::move(dw->extractState()));
			addWorker(i, std::move(dw));
		}
	}

	ThreadPool() = delete;
	ThreadPool(const ThreadPool &) = delete; /* non-copyable to avoid rcs for now */

	/*** Getters/setters ***/

	/* Returns the (current) number of threads in the pool
	 * (may be called before all threads have been added) */
	size_t size() const { return workers_.size(); }

	/* Returnst the number of workers that will be added in the pool */
	unsigned int getNumWorkers() const { return numWorkers_; }

	/* Returns the index of the calling thread */
	unsigned int getIndex() const { return index_; }

	/* Sets the index of the calling thread */
	void setIndex(unsigned int i) { index_ = i; }

	/*** Tasks-related ***/

	/* Submits a task to be executed by a worker */
	void submit(TaskT task);

	/* Notify the pool about the addition/completion of a task */
	unsigned incRemainingTasks() { return ++remainingTasks_; }
	unsigned decRemainingTasks() { return --remainingTasks_; }
	unsigned getRemainingTasks() { return remainingTasks_.load(); }

	bool shouldHalt() const { return shouldHalt_.load(); }

	/* Stops all threads */
	void halt()
	{
		std::lock_guard<std::mutex> lock(stateMtx_);
		shouldHalt_.store(true);
		stateCV_.notify_all();
	}

	/* Waits for all tasks to complete */
	std::vector<std::future<GenMCDriver::Result>> waitForTasks();

	/*** Destructor ***/

	~ThreadPool() { halt(); }

private:
	/* Adds a worker thread to the pool */
	void addWorker(unsigned int index, std::unique_ptr<GenMCDriver> driver);

	/* Tries to pop a task from the global queue */
	TaskT tryPopPoolQueue();

	/* Tries to steal a task from another thread */
	TaskT tryStealOtherQueue();

	/* Pops the next task to be executed by a thread */
	TaskT popTask();

	std::vector<std::unique_ptr<llvm::LLVMContext>> contexts_;

	/* Result of each thread */
	std::vector<std::future<GenMCDriver::Result>> results_;

	/* Whether the pool is active (i.e., accepting more jobs) or not */
	std::atomic<bool> shouldHalt_;

	/* The number of workers the pool should reach */
	unsigned int numWorkers_;

	/* The worker threads */
	std::vector<std::thread> workers_;

	/* A queue where tasks are stored */
	GlobalQueueT queue_;

	/* Number of tasks that need to be executed across threads */
	std::atomic<unsigned> remainingTasks_;

	/* The index of a worker thread */
	static thread_local unsigned int index_;

	/* Mutex+CV to determine whether the pool state has changed:
	 * a new task has been submitted or all tasks have been completed */
	std::mutex stateMtx_;
	std::condition_variable stateCV_;

	ThreadPinner pinner_;

	/* The thread joiner */
	ThreadJoiner joiner_;
};

#endif /* GENMC_THREAD_POOL_HPP */
