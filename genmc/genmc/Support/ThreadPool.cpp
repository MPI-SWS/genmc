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

#include "genmc/Support/ThreadPool.hpp"
#include "genmc/Execution/DepExecutionGraph.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Verification/ChoiceMap.hpp"
#include "genmc/Verification/Config.hpp"
#include "genmc/Verification/VerificationResult.hpp"
#include "genmc/Verification/WorkList.hpp"
#ifdef BUILD_LLI
#include "Runtime/Interpreter.h"
#include "passes/LLIConfig.hpp"
#include "passes/LLVMModule.hpp"
#include "passes/ModuleInfo.hpp"
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>
#else
#include "genmc/Support/Error.hpp"
#endif

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

ThreadPool::ThreadPool(const LLIConfig &lliConfig, const std::shared_ptr<const Config> &conf,
		       const std::unique_ptr<llvm::Module> &mod,
		       const std::unique_ptr<ModuleInfo> &modInfo, TFunT threadFun)
	: numWorkers_(lliConfig.threads), pinner_(numWorkers_), joiner_(workers_)
{

#ifndef BUILD_LLI
	UNREACHABLE(); /* unsupported */
#else
	/* Set global variables before spawning the threads */
	shouldHalt_.store(false);
	remainingTasks_.store(0);

	/* Have a non-empty queue before spawning workers */
	ExecutionGraph::Config dummyCfg{};
	dummyCfg.emitNALabels = Config::emitNALabels;
	auto execGraph = conf->isDepTrackingModel ? std::make_unique<DepExecutionGraph>(dummyCfg)
						  : std::make_unique<ExecutionGraph>(dummyCfg);
	auto exec = std::make_unique<GenMCDriver::Execution>(std::move(execGraph), WorkList(),
							     ChoiceMap());
	submit(std::move(exec));

	/* Spawn workers */
	for (auto i = 0U; i < numWorkers_; i++) {
		contexts_.push_back(std::make_unique<llvm::LLVMContext>());
		auto newmod = LLVMModule::cloneModule(mod, contexts_.back());
		auto newMI = modInfo->clone(*newmod);

		auto dw = GenMCDriver::create(conf, this);
		std::string buf;
		auto interp = llvm::Interpreter::create(std::move(newmod), std::move(newMI), &*dw,
							&lliConfig, &buf);
		addWorker(i, std::move(dw), std::move(interp), threadFun);
	}
#endif
}

ThreadPool::~ThreadPool() { halt(); }

void ThreadPool::addWorker(unsigned int i, std::unique_ptr<GenMCDriver> driver,
			   std::unique_ptr<llvm::Interpreter> interp, TFunT threadFun)
{
#ifndef BUILD_LLI
	UNREACHABLE();
#else
	using ThreadT = std::packaged_task<VerificationResult(
		unsigned int, std::unique_ptr<GenMCDriver> driver,
		std::unique_ptr<llvm::Interpreter> interp, TFunT threadFun)>;

	ThreadT thread([this](unsigned int /*i*/, std::unique_ptr<GenMCDriver> driver,
			      std::unique_ptr<llvm::Interpreter> interp, TFunT threadFun) {
		while (true) {
			auto taskUP = popTask();

			/* If the state is empty, nothing left to do */
			if (!taskUP)
				break;

			/* Prepare the driver and start the exploration */
			driver->initFromState(std::move(taskUP));
			threadFun(&*driver, &*interp);

			/* If that was the last task, notify everyone */
			std::scoped_lock lock(stateMtx_); // NOLINT(misc-const-correctness)
			if (decRemainingTasks() == 0) {
				stateCV_.notify_all();
				break;
			}
		}
		return std::move(driver->getResult());
	});

	results_.push_back(thread.get_future());

	workers_.emplace_back(std::move(thread), i, std::move(driver), std::move(interp),
			      threadFun);
	pinner_.pin(workers_.back(), i);
#endif
}

#ifdef BUILD_LLI
void ThreadPool::submit(ThreadPool::TaskT task)
{
	std::scoped_lock lock(stateMtx_); // NOLINT(misc-const-correctness)
	incRemainingTasks();
	queue_.push(std::move(task));
	stateCV_.notify_one();
}
#endif

auto ThreadPool::tryPopPoolQueue() -> ThreadPool::TaskT { return queue_.tryPop(); }

auto ThreadPool::tryStealOtherQueue() -> ThreadPool::TaskT
{
	/* TODO: Implement work-stealing */
	return nullptr;
}

auto ThreadPool::popTask() -> ThreadPool::TaskT
{
	while (true) {
		if (auto task = tryPopPoolQueue())
			return task;
		if (auto task = tryStealOtherQueue())
			return task;

		std::unique_lock<std::mutex> lock(stateMtx_);
		if (shouldHalt() || getRemainingTasks() == 0)
			return nullptr;
		stateCV_.wait(lock);
	}
	return nullptr;
}

auto ThreadPool::waitForTasks() -> std::vector<std::future<VerificationResult>>
{
	while (!shouldHalt() && getRemainingTasks() > 0)
		std::this_thread::yield();

	return std::move(results_);
}
