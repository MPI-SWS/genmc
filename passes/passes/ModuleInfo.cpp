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

#include "ModuleInfo.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/SExpr.hpp"
#include "passes/LLVMUtils.hpp"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <utility>

void ModuleInfo::clear()
{
	idInfo.clear();
	varInfo.clear();
	annotInfo.clear();
}

void ModuleInfo::collectIDs(const llvm::Module &mod)
{
	clear();

	auto valueCount = 0U;

	for (const auto &gv : GLOBALS(mod)) {
		auto id = valueCount++;
		idInfo.VID[&gv] = id;
		idInfo.IDV[id] = &gv;
	}

	for (const auto &fun : mod.getFunctionList()) {
		auto id = valueCount++;
		idInfo.VID[&fun] = id;
		idInfo.IDV[id] = &fun;

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		for (auto ai = fun.arg_begin(), ae = fun.arg_end(); ai != ae; ++ai) {
			auto id = valueCount++;
			idInfo.VID[&*ai] = id;
			idInfo.IDV[id] = &*ai;
		}

		for (auto iit = inst_begin(fun), iie = inst_end(fun); iit != iie; ++iit) {
			auto id = valueCount++;
			idInfo.VID[&*iit] = id;
			idInfo.IDV[id] = &*iit;
		}
	}
}

ModuleInfo::ModuleInfo(const llvm::Module &mod) : varInfo(), annotInfo() { collectIDs(mod); }

/*
 * If we ever use different contexts, we have to be very careful when
 * cloning annotation/fs information, as these may contain LLVM
 * type information.
 */
auto ModuleInfo::clone(const llvm::Module &mod) const -> std::unique_ptr<ModuleInfo>
{
	auto info = std::make_unique<ModuleInfo>(mod);

	/* Copy variable information */
	for (const auto &kv : varInfo.globalInfo) {
		VERIFY(idInfo.IDV.count(kv.first));
		info->varInfo.globalInfo[kv.first] = kv.second;
	}
	for (const auto &kv : varInfo.localInfo) {
		/* We may have collected information about allocas that got deleted ... */
		if (!idInfo.IDV.contains(kv.first))
			continue;
		info->varInfo.localInfo[kv.first] = kv.second;
	}
	for (const auto &kv : varInfo.internalInfo)
		info->varInfo.internalInfo[kv.first] = kv.second;

	/* Copy annotation information */
	for (const auto &kv : annotInfo.annotMap)
		info->annotInfo.annotMap[kv.first] =
			std::make_pair(kv.second.first, kv.second.second->clone());

	return info;
}
