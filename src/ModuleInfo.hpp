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

#ifndef __MODULE_INFO_HPP__
#define __MODULE_INFO_HPP__

#include "Config.hpp"
#include "ModuleID.hpp"
#include "NameInfo.hpp"
#include "SExpr.hpp"
#include "VSet.hpp"
#include "config.h"
#include <llvm/ADT/BitVector.h>
#include <llvm/ADT/IndexedMap.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace llvm {
class Value;
};

/*
 * Information kept about the module under test by the interpreter.
 */

/*
 * VariableInfo struct -- This struct contains source-code level (naming)
 * information for variables.
 */
template <typename Key> struct VariableInfo {

	/* Internal types (not exposed to user programs) for which we might
	 * want to collect naming information */
	using ID = Key;
	using InternalKey = std::string;

	std::unordered_map<ID, std::shared_ptr<NameInfo>> globalInfo;
	std::unordered_map<ID, std::shared_ptr<NameInfo>> localInfo;
	std::unordered_map<InternalKey, std::shared_ptr<NameInfo>> internalInfo;

	void clear()
	{
		globalInfo.clear();
		localInfo.clear();
		internalInfo.clear();
	}
};

/*
 * SAVer: AnnotationInfo struct -- Contains annotations for loads used by assume()s
 */
template <typename K, typename V> struct AnnotationInfo {

	using AnnotUM = std::unordered_map<K, std::unique_ptr<SExpr<V>>>;

	void clear() { annotMap.clear(); }

	AnnotUM annotMap;
};

/*
 * Pers: FsInfo struct -- Maintains some information regarding the
 * filesystem (e.g., type of inodes, files, etc)
 */
struct FsInfo {

	/* Explicitly initialize PODs to be C++11-compatible */
	FsInfo()
		: inodeTyp(nullptr), fileTyp(nullptr), blockSize(0), maxFileSize(0),
		  journalData(JournalDataFS::writeback), delalloc(false), dirInode(nullptr)
	{}

	/* Type information */
	llvm::StructType *inodeTyp;
	llvm::StructType *fileTyp;

	/* Filesystem options*/
	unsigned int blockSize;
	unsigned int maxFileSize;

	/* "Mount" options */
	JournalDataFS journalData;
	bool delalloc;

	/* Filenames in the module. These must be known statically. */
	VSet<std::string> filenames;

	/* Should hold the address of the directory's inode */
	void *dirInode;

	void clear()
	{
		inodeTyp = nullptr;
		fileTyp = nullptr;
		blockSize = 0;
		maxFileSize = 0;
		journalData = JournalDataFS::writeback;
		delalloc = false;
		filenames.clear();
		dirInode = nullptr;
	}
};

/*
 * PassModuleInfo -- A struct to be used from LLVM passes where
 * different kinds of data can be stored. It is different from
 * ModuleInfo as it does not require the module to have assigned IDs.
 */
struct PassModuleInfo {

	PassModuleInfo() = default;

	VariableInfo<llvm::Value *> varInfo;
	AnnotationInfo<llvm::Instruction *, llvm::Value *> annotInfo;
	VSet<std::string> filenames;
	std::optional<ModelType> determinedMM;
};

/*
 * ModuleInfo -- A struct to pack together all useful information like
 * VariableInfo and FsInfo for a given module
 */
struct ModuleInfo {

	ModuleInfo() = delete;
	ModuleInfo(const llvm::Module &mod);

	void clear();

	/* Collects all IDs for the given module.
	 * Should be manually called after the module is modified */
	void collectIDs();

	/* Assumes only statis information have been collected */
	std::unique_ptr<ModuleInfo> clone(const llvm::Module &mod) const;

	ModuleID idInfo;
	VariableInfo<ModuleID::ID> varInfo;
	AnnotationInfo<ModuleID::ID, ModuleID::ID> annotInfo;
	FsInfo fsInfo;
	std::optional<ModelType> determinedMM;

private:
	const llvm::Module &mod;
};

#endif /* __MODULE_INFO_HPP__ */
