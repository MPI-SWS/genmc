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

#ifndef GENMC_LOAD_ANNOTATION_HPP
#define GENMC_LOAD_ANNOTATION_HPP

#include "genmc/ADT/value_ptr.hpp"
#include "genmc/Support/ModuleVarID.hpp"
#include "genmc/Support/SExpr.hpp"

#include <cstdint>

/* Currently values matched manually to assume_internal() (fixed size; see include) */
enum class AssumeType : std::uint8_t {
	User = 0,
	Barrier = 1,
	Spinloop = 2,
};

struct Annotation {
	using Expr = SExpr<ModuleVarID>;
	using ExprVP = value_ptr<Expr, SExprCloner<ModuleVarID>>;

	AssumeType type;
	ExprVP expr;
};

#endif /* GENMC_LOAD_ANNOTATION_HPP */
