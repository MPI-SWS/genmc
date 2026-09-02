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

#ifndef GENMC_LLVM_UTILS_HPP
#define GENMC_LLVM_UTILS_HPP

#include "genmc/ADT/VSet.hpp"
#include "genmc/Execution/EventAttr.hpp"
#include "genmc/Support/ActionEnums.hpp"
#include "genmc/Support/Error.hpp"
#include "passes/InternalFunctions.hpp"

#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

#include <string>

using TerminatorInst = llvm::Instruction;

#define GLOBALS(M) (M).globals()

class CallInstWrapper {

public:
	/* Constructors */
	CallInstWrapper() : callBase(nullptr) {}
	CallInstWrapper(llvm::CallBase *callBase) : callBase(callBase) {}
	CallInstWrapper(llvm::CallBase &callBase) : callBase(&callBase) {}
	CallInstWrapper(llvm::CallInst *CI) : callBase(CI) {}

	/* Getters emulation */
	auto getCalledOperand() -> llvm::Value * { return callBase->getCalledOperand(); }
	auto getCalledFunction() -> llvm::Function * { return callBase->getCalledFunction(); }
	[[nodiscard]] auto arg_size() const -> size_t { return callBase->arg_size(); }
	auto arg_begin() -> llvm::User::op_iterator { return callBase->arg_begin(); }
	auto arg_end() -> llvm::User::op_iterator { return callBase->arg_end(); }

	auto operator&() -> llvm::Instruction * { return callBase; }

private:
	llvm::CallBase *callBase;
};

/**
 * Returns true if o1 and o2 are the same ordering as far as a load
 * operation is concerned. This catches cases where e.g.,
 * ord1 is acq_rel and ord2 is acq.
 * */
auto areSameLoadOrdering(llvm::AtomicOrdering ord1, llvm::AtomicOrdering ord2) -> bool;

/**
 * Strips all kinds of casts from val (including trunc, zext ops, etc)
 */
auto stripCasts(llvm::Value *val) -> llvm::Value *;

/**
 * Strips all casts and GEPs from val
 */
auto stripCastsGEPs(llvm::Value *val) -> llvm::Value *;

/**
 * Strips all casts and constant operations from val
 */
auto stripCastsConstOps(llvm::Value *val) -> llvm::Value *;

/**
 * Returns the name of the function ci calls
 */
auto getCalledFunOrStripValName(const llvm::CallInst &ci) -> std::string;

/**
 * Returns true if its argument is an intrinsic call that does
 * not have any side-effects.
 */
auto isIntrinsicCallNoSideEffects(const llvm::Instruction &i) -> bool;

// FIXME: Change name
/**
 * Returns true if i1 depends on o2
 */
auto isDependentOn(const llvm::Instruction *i1, const llvm::Instruction *i2) -> bool;

/**
 * If EI extracts its value from an integer CAS instruction, returns said CAS;
 * otherwise returns nullptr.
 */
auto extractsFromCAS(llvm::ExtractValueInst *extract) -> llvm::AtomicCmpXchgInst *;

/**
 * Returns true if its argument has side-effects.
 * Clean intrinsic functions (e.g., assert, assume) are considered side-effect-free.
 * Calls to non-intrinsic functions are considered to produce side-effects
 * unless a clean list "cleanFuns" is provided.
 */
auto hasSideEffects(const llvm::Instruction *i, const VSet<llvm::Function *> *cleanFuns = nullptr)
	-> bool;

/**
 * Returns true if I allocates memory.
 * If a list of allocFuns is provided, then these are also considered as allocating instructions.
 */
auto isAlloc(const llvm::Instruction *i, const VSet<llvm::Function *> *allocFuns = nullptr) -> bool;

/**
 * Returns true if I has load semantics (i.e., is load/FAI/CAS/LoadInternalCall).
 */
auto hasLoadSemantics(llvm::Instruction *I) -> bool;

/**
 * Returns the action of instruction I.
 */
auto getInstKind(llvm::Instruction *I) -> ActionKind;

/**
 * Annotates I by setting the metadata TYPE to VALUE
 */
void annotateInstruction(llvm::Instruction *i, const std::string &type, uint64_t value);

/**
 * Given the terminator TERM of a block B and a successor SUCC of B,
 * tries to make B directly jump to SUCC's successor, if SUCC is an
 * empty block with an unconditional jump.
 * Returns the destination block if it succeeded, and null otherwise.
 */
auto tryThreadSuccessor(llvm::BranchInst *term, llvm::BasicBlock *succ) -> llvm::BasicBlock *;

/**
 * Extracts the write attribute from an (annotated) instruction.
 * Returns NONE if the instruction is not annotated.
 */
inline auto getWriteAttr(llvm::Instruction &I) -> WriteAttr
{
	auto *metadata = I.getMetadata("genmc.attr");
	if (!metadata)
		return WriteAttr::None;

	auto *op = llvm::dyn_cast<llvm::ConstantAsMetadata>(metadata->getOperand(0));
	VERIFY(op);

	auto flags = llvm::cast<llvm::ConstantInt>(op->getValue())->getZExtValue();
	return static_cast<WriteAttr>(flags);
}

namespace details {
template <typename F>
void foreachInBackPathTo(llvm::BasicBlock *curr, llvm::BasicBlock *toBB,
			 // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
			 llvm::SmallVector<llvm::BasicBlock *, 4> &path, F &&fun)
{
	path.push_back(curr);
	if (curr == toBB) {
		for (auto *bb : path)
			std::for_each(bb->rbegin(), bb->rend(), fun);
		path.pop_back();
		return;
	}

	for (auto *pred : predecessors(curr))
		if (std::ranges::find(path, pred) == path.end())
			foreachInBackPathTo(pred, toBB, path, fun);
	path.pop_back();
}
} // namespace details

/**
 * Executes FUN for all instructions from FROM to TO.
 * TO needs to be a predecessor of FROM, otherwise FUN is not applied.
 * FUN is applied in reverse iteration order within a block.
 */
template <typename F>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
void foreachInBackPathTo(llvm::BasicBlock *from, llvm::BasicBlock *toBB, F &&fun)
{
	llvm::SmallVector<llvm::BasicBlock *, 4> path;
	::details::foreachInBackPathTo(from, toBB, path, fun);
}

/*
 * LLVM Utilities for older LLVM versions
 */

void replaceUsesWithIf(llvm::Value *Old, llvm::Value *New,
		       llvm::function_ref<bool(llvm::Use &U)> ShouldReplace);

#endif /* GENMC_LLVM_UTILS_HPP */
