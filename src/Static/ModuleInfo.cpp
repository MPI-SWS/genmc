/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "ModuleInfo.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/SExpr.hpp"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>

void ModuleInfo::clear()
{
	idInfo.clear();
	varInfo.clear();
	annotInfo.clear();
	fsInfo.clear();
}

void ModuleInfo::collectIDs()
{
	clear();

	auto valueCount = 0u;

	for (auto &gv : GLOBALS(mod)) {
		auto id = valueCount++;
		idInfo.VID[&gv] = id;
		idInfo.IDV[id] = &gv;
	}

	for (auto &fun : mod.getFunctionList()) {
		auto id = valueCount++;
		idInfo.VID[&fun] = id;
		idInfo.IDV[id] = &fun;

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
	return;
}

ModuleInfo::ModuleInfo(const llvm::Module &mod) : mod(mod), varInfo(), annotInfo(), fsInfo()
{
	collectIDs();
	return;
}

/*
 * If we ever use different contexts, we have to be very careful when
 * cloning annotation/fs information, as these may contain LLVM
 * type information.
 */
std::unique_ptr<ModuleInfo> ModuleInfo::clone(const llvm::Module &mod) const
{
	auto info = std::make_unique<ModuleInfo>(mod);

	/* Copy variable information */
	for (auto &kv : varInfo.globalInfo) {
		BUG_ON(!idInfo.IDV.count(kv.first));
		info->varInfo.globalInfo[kv.first] = kv.second;
	}
	for (auto &kv : varInfo.localInfo) {
		/* We may have collected information about allocas that got deleted ... */
		if (!idInfo.IDV.count(kv.first))
			continue;
		info->varInfo.localInfo[kv.first] = kv.second;
	}
	for (auto &kv : varInfo.internalInfo)
		info->varInfo.internalInfo[kv.first] = kv.second;

	/* Copy annotation information */
	for (auto &kv : annotInfo.annotMap)
		info->annotInfo.annotMap[kv.first] = kv.second->clone();

	/* Copy fs information */
	info->fsInfo.inodeTyp = fsInfo.inodeTyp;
	info->fsInfo.fileTyp = fsInfo.fileTyp;

	info->fsInfo.blockSize = fsInfo.blockSize;
	info->fsInfo.blockSize = fsInfo.blockSize;

	info->fsInfo.journalData = fsInfo.journalData;
	info->fsInfo.delalloc = fsInfo.delalloc;

	BUG_ON(fsInfo.dirInode != nullptr);
	info->fsInfo.dirInode = fsInfo.dirInode;

	for (auto &name : fsInfo.filenames)
		info->fsInfo.filenames.insert(name);

	return info;
}
