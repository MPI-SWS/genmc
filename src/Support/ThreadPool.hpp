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

#ifndef GENMC_THREAD_POOL_HPP
#define GENMC_THREAD_POOL_HPP

#include "ExecutionGraph/DepExecutionGraph.hpp"
#include "Runtime/Interpreter.h"
#include "Runtime/LLIConfig.hpp"
#include "Static/LLVMModule.hpp"
#include "Support/ThreadPinner.hpp"
#include "Verification/GenMCDriver.hpp"
#include "Verification/VerificationResult.hpp"
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <atomic>
#include <future>
#include <memory>
#include <thread>

/*******************************************************************************
 **                           GlobalWorkQueue Class
 ******************************************************************************/

/** Represents the global workqueue shared among threads. */
class GlobalWorkQueue {

public:
	using ItemT = std::unique_ptr<GenMCDriver::Execution>;
	using QueueT = std::vector<ItemT>;

	/*** Constructors ***/

	GlobalWorkQueue() = default;
	GlobalWorkQueue(const GlobalWorkQueue &) = delete;
	GlobalWorkQueue(GlobalWorkQueue &&) = delete;

	auto operator=(const GlobalWorkQueue &) -> GlobalWorkQueue & = delete;
	auto operator=(GlobalWorkQueue &&) -> GlobalWorkQueue & = delete;

	~GlobalWorkQueue() = default;

	/*** Queue operations ***/

	/** Returns true if the queue is empty */
	auto empty() -> bool
	{
		std::lock_guard<std::mutex> lock(qMutex);
		return queue.empty();
	}

	/** Adds a new item to the queue */
	void push(ItemT item)
	{
		std::lock_guard<std::mutex> lock(qMutex);
		queue.push_back(std::move(item));
	}

	/** Tries to pop an item from the queue */
	auto tryPop() -> ItemT
	{
		std::lock_guard<std::mutex> lock(qMutex);
		if (queue.empty())
			return nullptr;
		auto item = std::move(queue.back());
		queue.pop_back();
		return item;
	}

private:
	/** The actual queue data structure */
	QueueT queue;

	/** Protection against unsynchronized accesses */
	std::mutex qMutex;
};

/*******************************************************************************
 **                           ThreadJoiner Class
 ******************************************************************************/

/** A class responsible for joining a bunch of threads */
class ThreadJoiner {

public:
	/*** Constructors ***/
	ThreadJoiner() = delete;
	explicit ThreadJoiner(std::vector<std::thread> &ts) : threads(ts) {}
	ThreadJoiner(ThreadJoiner &&) = delete;
	ThreadJoiner(const ThreadJoiner &) = delete;

	auto operator=(const ThreadJoiner &) -> ThreadJoiner & = delete;
	auto operator=(ThreadJoiner &&) -> ThreadJoiner & = delete;

	/*** Destructor ***/
	~ThreadJoiner()
	{
		for (auto &thread : threads) {
			if (thread.joinable())
				thread.join();
		}
	}

private:
	/** The threads to join */
	std::vector<std::thread> &threads;
};

/*******************************************************************************
 **                            ThreadPool Class
 ******************************************************************************/

/**
 * A class responsible for creating and managing a pool of threads, with tasks
 * submitted dynamically to the threads for execution. Each thread will have
 * each own exploration driver so that they will able to execute the submitted
 * (exploration) tasks concurrently.
 */
class ThreadPool {

public:
	using GlobalQueueT = GlobalWorkQueue;
	using TaskT = GlobalQueueT::ItemT;
	using TFunT = void (*)(GenMCDriver *, llvm::Interpreter *);

	/*** Constructors ***/

	ThreadPool() = delete;
	ThreadPool(const ThreadPool &) = delete; /* non-copyable to avoid rcs for now */
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool(const LLIConfig &lliConfig, const std::shared_ptr<const Config> &conf,
		   const std::unique_ptr<llvm::Module> &mod, const std::unique_ptr<ModuleInfo> &MI,
		   TFunT threadFun)
		: numWorkers_(lliConfig.threads), pinner_(numWorkers_), joiner_(workers_)
	{

		/* Set global variables before spawning the threads */
		shouldHalt_.store(false);
		remainingTasks_.store(0);

		/* Have a non-empty queue before spawning workers */
		auto dummyGetter = [](auto &addr) { return SVal(0); };
		auto execGraph = conf->isDepTrackingModel
					 ? std::make_unique<DepExecutionGraph>(dummyGetter)
					 : std::make_unique<ExecutionGraph>(dummyGetter);
		auto exec = std::make_unique<GenMCDriver::Execution>(
			std::move(execGraph), std::move(WorkList()), std::move(ChoiceMap()),
			std::move(SAddrAllocator()), Event::getInit());
		submit(std::move(exec));

		/* Spawn workers */
		for (auto i = 0U; i < numWorkers_; i++) {
			contexts_.push_back(std::make_unique<llvm::LLVMContext>());
			auto newmod = LLVMModule::cloneModule(mod, contexts_.back());
			auto newMI = MI->clone(*newmod);

			auto dw = GenMCDriver::create(conf, this);
			std::string buf;
			auto EE = llvm::Interpreter::create(std::move(newmod), std::move(newMI),
							    &*dw, &lliConfig,
							    dw->getExec().getAllocator(), &buf);
			addWorker(i, std::move(dw), std::move(EE), threadFun);
		}
	}

	auto operator=(const ThreadPool &) -> ThreadPool & = delete;
	auto operator=(ThreadPool &&) -> ThreadPool & = delete;

	/*** Getters/setters ***/

	/** Returns the (current) number of threads in the pool
	 * (may be called before all threads have been added) */
	[[nodiscard]] auto size() const -> size_t { return workers_.size(); }

	/** Returnst the number of workers that will be added in the pool */
	[[nodiscard]] auto getNumWorkers() const -> unsigned int { return numWorkers_; }

	/** Returns the index of the calling thread */
	[[nodiscard]] auto getIndex() const -> unsigned int { return index_; }

	/** Sets the index of the calling thread */
	void setIndex(unsigned int i) { index_ = i; }

	/*** Tasks-related ***/

	/** Submits a task to be executed by a worker */
	void submit(TaskT task);

	/** Notify the pool about the addition/completion of a task */
	auto incRemainingTasks() -> unsigned { return ++remainingTasks_; }
	auto decRemainingTasks() -> unsigned { return --remainingTasks_; }
	auto getRemainingTasks() -> unsigned { return remainingTasks_.load(); }

	[[nodiscard]] auto shouldHalt() const -> bool { return shouldHalt_.load(); }

	/** Stops all threads */
	void halt()
	{
		std::lock_guard<std::mutex> lock(stateMtx_);
		shouldHalt_.store(true);
		stateCV_.notify_all();
	}

	/** Waits for all tasks to complete */
	auto waitForTasks() -> std::vector<std::future<VerificationResult>>;

	/*** Destructor ***/

	~ThreadPool() { halt(); }

private:
	/** Adds a worker thread to the pool */
	void addWorker(unsigned int index, std::unique_ptr<GenMCDriver> driver,
		       std::unique_ptr<llvm::Interpreter> EE, TFunT threadFun);

	/** Tries to pop a task from the global queue */
	auto tryPopPoolQueue() -> TaskT;

	/** Tries to steal a task from another thread */
	auto tryStealOtherQueue() -> TaskT;

	/** Pops the next task to be executed by a thread */
	auto popTask() -> TaskT;

	std::vector<std::unique_ptr<llvm::LLVMContext>> contexts_;

	/** Result of each thread */
	std::vector<std::future<VerificationResult>> results_;

	/** Whether the pool is active (i.e., accepting more jobs) or not */
	std::atomic<bool> shouldHalt_;

	/** The number of workers the pool should reach */
	unsigned int numWorkers_;

	/** The worker threads */
	std::vector<std::thread> workers_;

	/** A queue where tasks are stored */
	GlobalQueueT queue_;

	/** Number of tasks that need to be executed across threads */
	std::atomic<unsigned> remainingTasks_;

	/** The index of a worker thread */
	static thread_local unsigned int index_;

	/** Mutex+CV to determine whether the pool state has changed:
	 * a new task has been submitted or all tasks have been completed */
	std::mutex stateMtx_;
	std::condition_variable stateCV_;

	ThreadPinner pinner_;

	/** The thread joiner */
	ThreadJoiner joiner_;
};

#endif /* GENMC_THREAD_POOL_HPP */
