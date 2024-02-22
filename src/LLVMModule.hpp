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

#include "config.h"

#include "Config.hpp"
#include "ModuleInfo.hpp"

#include <llvm/IR/Module.h>
#include <memory>

namespace LLVMModule {

/* Parses an LLVM module from FILENAME into CTX */
std::unique_ptr<llvm::Module> parseLLVMModule(const std::string &filename,
					      const std::unique_ptr<llvm::LLVMContext> &ctx);

/* Clones MOD into CTX */
std::unique_ptr<llvm::Module> cloneModule(const std::unique_ptr<llvm::Module> &mod,
					  const std::unique_ptr<llvm::LLVMContext> &ctx);

/* Transforms MOD according to CONF. Collected info are stored in MI */
bool transformLLVMModule(llvm::Module &mod, ModuleInfo &MI,
			 const std::shared_ptr<const Config> &conf);

/* Prints MOD to the file FILENAME */
void printLLVMModule(llvm::Module &mod, const std::string &filename);

} // namespace LLVMModule
