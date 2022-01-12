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

#ifndef __INST_ANNOTATOR_HPP__
#define __INST_ANNOTATOR_HPP__

#include "Error.hpp"
#include "VSet.hpp"
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/IR/Instructions.h>

#include <unordered_map>

using namespace llvm;

template<typename T>
class SExpr;

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

	/* Returns the annotation for a load L */
	IRExprUP annotate(LoadInst *l);

	/* Returns the condition under which BB jumps to its first successor.
	 * If PRED is non-null, assumes that the predecessor of the basic block is PRED
	 * during the calculation of the annotation */
	IRExprUP annotateBBCond(BasicBlock *bb, BasicBlock *pred = nullptr);

	/* Returns the annotation for a CAS associated with the backedge LATCH->header(L) */
	IRExprUP annotateCASWithBackedgeCond(AtomicCmpXchgInst *cas, BasicBlock *latch, Loop *l,
					     const VSet<llvm::Function *> *cleanSet = nullptr);

private:
	/* Helper types for the annotation routines */
	enum Status { unseen, entered, left };

	/* InstAnnotMap maps void * so that we can use it both with ID keys and Value *.
	 * It is a big ugly, but on par with RegisterExpr identifiers (see SExpr.hpp) */
	using InstAnnotMap = std::unordered_map<Value *, IRExprUP>;
	using InstStatusMap = DenseMap<Instruction *, Status>;

	/* Resets all helper members used in the annotation */
	void reset();

	/* Generates an expression for a given instruction operand */
	IRExprUP generateOperandExpr(Value *op);

	/* Generates an expression for an instruction */
	IRExprUP generateInstExpr(Instruction *curr);

	/* Helper that returns the annotation for CURR by propagating SUCC's annotation backwards */
	IRExprUP propagateAnnotFromSucc(Instruction *curr, Instruction *succ);

	/* Helper for annotate(); performs the actual annotation */
	void annotateDFS(Instruction *curr);

	/* Similar to propagateAnnotFromSucc, but for when annotating backedges */
	IRExprUP propagateAnnotFromSuccInLoop(Instruction *curr, Instruction *succ,
							    const VSet<BasicBlock *> &latch, Loop *l);

	/* Helper for annotateCASWithBackedgeCond(); performs the actual annotation (for backedge paths) */
	void annotateCASWithBackedgeCondDFS(Instruction *curr, const VSet<BasicBlock *> &backedgePaths,
					    Loop *l, const VSet<llvm::Function *> *cleanSet);

	/* Various getters/setters */

	/* Returns the appropriate key to be used when accessing annotMaps depending on useIDs */
	Value *getAnnotMapKey(Value *i) const;

	/* Returns the annotation of I */
	const IRExpr *getAnnot(Instruction *i);

	/* Assumes ownership of I's annotation */
	IRExprUP releaseAnnot(Instruction *i);

	/* Sets the annotation of I To be ANNOT */
	void setAnnot(Instruction *i, IRExprUP annot);

	/* A helper status map */
	InstStatusMap statusMap;

	/* Maps instructions to annotations */
	InstAnnotMap annotMap;
};

#endif /* __INST_ANNOTATOR_HPP__ */
