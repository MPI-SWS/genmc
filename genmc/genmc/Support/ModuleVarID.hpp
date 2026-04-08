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

#ifndef GENMC_MODULE_VAR_ID_HPP
#define GENMC_MODULE_VAR_ID_HPP

/** Unique identifier for module-level variables (global variables, functions, etc.).
 *  Used by the library to refer to module components without depending on LLVM types. */
using ModuleVarID = unsigned int;

#endif /* GENMC_MODULE_VAR_ID_HPP */
