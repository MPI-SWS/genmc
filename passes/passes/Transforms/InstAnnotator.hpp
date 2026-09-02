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

#ifndef GENMC_INST_ANNOTATOR_HPP
#define GENMC_INST_ANNOTATOR_HPP

#include "genmc/ADT/VSet.hpp"
#include "genmc/Support/Error.hpp"
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Pass.h>

#include <unordered_map>

using namespace llvm;

template <typename T> class SExpr;

/*
 * A class that annotates loads by performing a DFS-like propagation procedure.
 * Also exports utilities for calculating symbolic expressions across basic blocks.
 *
 * NOTE: For annotations to be used in a multithreaded environment
 * we have to translate the annotation info we get in a module-agnostic form
 */

class InstAnnotator {

public:
	using IRExpr = SExpr<Value *>;
	using IRExprUP = std::unique_ptr<SExpr<Value *>>;

	/* Returns the annotation for a load */
	auto annotate(Instruction *curr) -> IRExprUP;

	/* Returns the condition under which bb jumps to its first successor.
	 * If PRED is non-null, assumes that the predecessor of the basic block is PRED
	 * during the calculation of the annotation */
	auto annotateBBCond(BasicBlock *bb, BasicBlock *pred = nullptr) -> IRExprUP;

	/* Returns the annotation for a CAS associated with the backedge LATCH->header(L) */
	auto annotateCASWithBackedgeCond(AtomicCmpXchgInst *curr, BasicBlock *latch, Loop *l,
					 const VSet<llvm::Function *> *cleanSet = nullptr)
		-> IRExprUP;

private:
	/* Helper types for the annotation routines */
	// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
	enum Status : std::uint8_t { unseen, entered, left };

	/* InstAnnotMap maps void * so that we can use it both with ID keys and Value *.
	 * It is a big ugly, but on par with RegisterExpr identifiers (see SExpr.hpp) */
	using InstAnnotMap = std::unordered_map<Value *, IRExprUP>;
	using InstStatusMap = DenseMap<Instruction *, Status>;

	/* Resets all helper members used in the annotation */
	void reset();

	/* Generates an expression for a given instruction operand */
	auto generateOperandExpr(Module *mod, Value *op) -> IRExprUP;

	/* Generates an expression for an instruction */
	auto generateInstExpr(Instruction *curr) -> IRExprUP;

	/* Helper that returns the annotation for CURR by propagating SUCC's annotation backwards */
	auto propagateAnnotFromSucc(Instruction *curr, Instruction *succ) -> IRExprUP;

	/* Helper for annotate(); performs the actual annotation */
	void annotateDFS(Instruction *curr);

	/* Similar to propagateAnnotFromSucc, but for when annotating backedges */
	auto propagateAnnotFromSuccInLoop(Instruction *curr, Instruction *succ,
					  const VSet<BasicBlock *> &backedgePaths, Loop *l)
		-> IRExprUP;

	/* Helper for annotateCASWithBackedgeCond(); performs the actual annotation (for backedge
	 * paths) */
	void annotateCASWithBackedgeCondDFS(Instruction *curr,
					    const VSet<BasicBlock *> &backedgePaths, Loop *l,
					    const VSet<llvm::Function *> *cleanSet);

	/* Various getters/setters */

	/* Returns the appropriate key to be used when accessing annotMaps depending on useIDs */
	static auto getAnnotMapKey(Value *i) -> Value *;

	/* Returns the annotation of I */
	auto getAnnot(Instruction *i) -> const IRExpr *;

	/* Assumes ownership of I's annotation */
	auto releaseAnnot(Instruction *i) -> IRExprUP;

	/* Sets the annotation of I To be ANNOT */
	void setAnnot(Instruction *i, IRExprUP annot);

	/* A helper status map */
	InstStatusMap statusMap;

	/* Maps instructions to annotations */
	InstAnnotMap annotMap;
};

#endif /* GENMC_INST_ANNOTATOR_HPP */
