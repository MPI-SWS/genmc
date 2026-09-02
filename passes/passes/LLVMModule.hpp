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

#include "passes/ModuleInfo.hpp"

#include <llvm/IR/Module.h>

#include <filesystem>
#include <memory>

struct LLIConfig;

namespace LLVMModule {

/** Parses an LLVM module from FILENAME into CTX */
auto parseLLVMModule(const std::string &filename, const std::unique_ptr<llvm::LLVMContext> &ctx)
	-> std::unique_ptr<llvm::Module>;

/* Link a vector of modules into a single module */
auto linkAllModules(std::vector<std::unique_ptr<llvm::Module>> modules)
	-> std::unique_ptr<llvm::Module>;

/* Parses and Links all LLVM modules from "dirname/target/ * /deps" into CTX */
auto parseLinkAllLLVMModules(const std::filesystem::path &dirname,
			     const std::unique_ptr<llvm::LLVMContext> &ctx)
	-> std::unique_ptr<llvm::Module>;

/** Clones MOD into CTX */
auto cloneModule(const std::unique_ptr<llvm::Module> &mod,
		 const std::unique_ptr<llvm::LLVMContext> &ctx) -> std::unique_ptr<llvm::Module>;

/** Transforms MOD according to CONF. Collected info are stored in MI */
auto transformLLVMModule(llvm::Module &mod, ModuleInfo &MI, const LLIConfig *conf) -> bool;

/** Prints MOD to the file FILENAME */
void printLLVMModule(llvm::Module &mod, const std::string &filename);

} // namespace LLVMModule
