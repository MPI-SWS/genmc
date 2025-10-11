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

#include "Static/ModuleInfo.hpp"

#include <filesystem>
#include <llvm/IR/Module.h>
#include <memory>

class LLIConfig;

namespace LLVMModule {

/** Parses an LLVM module from FILENAME into CTX */
std::unique_ptr<llvm::Module> parseLLVMModule(const std::string &filename,
					      const std::unique_ptr<llvm::LLVMContext> &ctx);

/* Link a vector of modules into a single module */
std::unique_ptr<llvm::Module> linkAllModules(std::vector<std::unique_ptr<llvm::Module>> modules);

/* Parses and Links all LLVM modules from "dirname/target/ * /deps" into CTX */
std::unique_ptr<llvm::Module>
parseLinkAllLLVMModules(const std::filesystem::path &dirname,
			const std::unique_ptr<llvm::LLVMContext> &ctx);

/** Clones MOD into CTX */
std::unique_ptr<llvm::Module> cloneModule(const std::unique_ptr<llvm::Module> &mod,
					  const std::unique_ptr<llvm::LLVMContext> &ctx);

/** Transforms MOD according to CONF. Collected info are stored in MI */
bool transformLLVMModule(llvm::Module &mod, ModuleInfo &MI, const LLIConfig *conf);

/** Prints MOD to the file FILENAME */
void printLLVMModule(llvm::Module &mod, const std::string &filename);

} // namespace LLVMModule
