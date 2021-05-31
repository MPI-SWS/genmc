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

#include "config.h"
#include "Config.hpp"
#include <llvm/ADT/BitVector.h>
#include <llvm/ADT/IndexedMap.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>

#include <memory>
#include <string>
#include <unordered_map>

/*
 * Information kept about the module under test by the interpreter.
 */

class SExpr;

/*
 * VariableInfo struct -- This struct contains source-code level (naming)
 * information for variables.
 */
struct VariableInfo {

  /*
   * We keep a map (Values -> (offset, name_at_offset)), and after
   * the interpreter and the variables are allocated and initialized,
   * we use the map to dynamically find out the name corresponding to
   * a particular address.
   */
  using NameInfo = std::vector<std::pair<unsigned, std::string > >;

  /* Internal types (not exposed to user programs) for which we might
   * want to collect naming information */
  using InternalType = std::string;

  std::unordered_map<llvm::Value *, NameInfo> globalInfo;
  std::unordered_map<llvm::Value *, NameInfo> localInfo;
  std::unordered_map<InternalType, NameInfo> internalInfo;
};

/*
 * SAVer: AnnotationInfo struct -- Contains annotations for loads used by assume()s
 */
struct AnnotationInfo {

  using AnnotUM = std::unordered_map<llvm::Instruction *, std::unique_ptr<SExpr> >;

  /* Forward declarations (pimpl-style) */
  AnnotationInfo();
  AnnotationInfo(AnnotationInfo &&other);
  ~AnnotationInfo();
  AnnotationInfo &operator=(AnnotationInfo &&other);

  AnnotUM annotMap;
};

/*
 * Pers: FsInfo struct -- Maintains some information regarding the
 * filesystem (e.g., type of inodes, files, etc)
 */
struct FsInfo {

  using Filename = std::string;
  using NameMap = std::unordered_map<Filename, void *>;

  /* Type information */
  llvm::StructType *inodeTyp;
  llvm::StructType *fileTyp;

  /* A bitvector of available file descriptors */
  llvm::BitVector fds;

  /* Filesystem options*/
  unsigned int blockSize;
  unsigned int maxFileSize;

  /* "Mount" options */
  JournalDataFS journalData;
  bool delalloc;

  /* A map from file descriptors to file descriptions */
  llvm::IndexedMap<void *> fdToFile;

  /* Should hold the address of the directory's inode */
  void *dirInode;

  /* Maps a filename to the address of the contents of the directory's inode for
   * said name (the contents should have the address of the file's inode) */
  NameMap nameToInodeAddr;
};

/*
 * ModuleInfo -- A struct to pack together all useful information like
 * VariableInfo and FsInfo
 */
struct ModuleInfo {

  VariableInfo varInfo;
  AnnotationInfo annotInfo;
  FsInfo fsInfo;
};

#endif /* __MODULE_INFO_HPP__ */
