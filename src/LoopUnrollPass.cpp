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

#include "VSet.hpp"
#include "LoopUnrollPass.hpp"
#include "DeclareAssumePass.hpp"
#include "DeclareEndLoopPass.hpp"
#include "SpinAssumePass.hpp"
#include "Error.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <sstream>

#ifdef LLVM_HAS_TERMINATORINST
 typedef llvm::TerminatorInst TerminatorInst;
#else
 typedef llvm::Instruction TerminatorInst;
#endif

void LoopUnrollPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	llvm::LoopPass::getAnalysisUsage(au);
	au.addRequired<DeclareEndLoopPass>();
	au.addPreserved<DeclareEndLoopPass>();
}

llvm::BasicBlock *LoopUnrollPass::makeDivergeBlock(llvm::Loop *l)
{
	llvm::Function *parentFun, *endLoopFun;
	llvm::BasicBlock *divergeBlock;

	parentFun = (*l->block_begin())->getParent();
	divergeBlock = llvm::BasicBlock::Create(parentFun->getContext(), "diverge", parentFun);

	endLoopFun = parentFun->getParent()->getFunction("__end_loop");
	BUG_ON(!endLoopFun);

	llvm::CallInst::Create(endLoopFun, {}, "", divergeBlock);
	llvm::BranchInst::Create(divergeBlock, divergeBlock);
	return divergeBlock;
}

void LoopUnrollPass::redirectBranch(int bodyIdx, int blockIdx, int unrollDepth,
				    llvm::BasicBlock *divergeBlock,
				    std::map<llvm::BasicBlock const *, int> &loopBlockIdx,
				    std::vector<std::vector<llvm::BasicBlock *> > &loopBodies)
{
	TerminatorInst *ti;
	llvm::BasicBlock *succ;
	int nsucc;
	int targetBlock;

	ti = loopBodies[bodyIdx][blockIdx]->getTerminator();
	nsucc = ti->getNumSuccessors();
	for (int i = 0; i < nsucc; i++) {
		/*
		 * Check the successors of this block. If one of them directs
		 * within the loop body we have to check whether it directs
		 * to the header. Else, we do nothing.
		 */
		succ = ti->getSuccessor(i);
		if (loopBlockIdx.count(succ)) {
			/*
			 * This successor directs to the loop header. We either have to
			 * redirect it to the next loop body, or, if it is the last body,
			 * to the divergence block. In a different case, we just connect
			 * it to some block within the same loop body.
			 */
			targetBlock = loopBlockIdx[succ];
			if (targetBlock == 0) {
				if (bodyIdx < unrollDepth - 1)
					ti->setSuccessor(i, loopBodies[bodyIdx+1][targetBlock]);
				else
					ti->setSuccessor(i, divergeBlock);
			} else
				ti->setSuccessor(i, loopBodies[bodyIdx][targetBlock]);
		}
	}
}

void LoopUnrollPass::redirectPHIOrValue(int bodyIdx, int blockIdx,
					std::vector<llvm::ValueToValueMapTy> &VMaps,
					std::map<llvm::BasicBlock const *, int> &loopBlockIdx,
					std::vector<std::vector<llvm::BasicBlock *> > &loopBodies)
{
	llvm::PHINode *p;
	int targetIdx;
	int nvals;
	int nops;

	for (auto it = loopBodies[bodyIdx][blockIdx]->begin();
	     it != loopBodies[bodyIdx][blockIdx]->end(); it++) {
		if (llvm::isa<llvm::PHINode>(*it)) {
			p = static_cast<llvm::PHINode *>(&*it);
			nvals = p->getNumIncomingValues();
			for (int k = 0; k < nvals; k++) {
				if (loopBlockIdx.count(p->getIncomingBlock(k))) {
					targetIdx = loopBlockIdx[p->getIncomingBlock(k)];
					/*
					 * Check whether this is an edge coming from outside the
					 * loop or from a previous body
					 */
					if (blockIdx == 0) {
						if (bodyIdx == 0) {
							p->removeIncomingValue(k);
							k--;
							nvals--;
						} else {
							p->setIncomingBlock(k, loopBodies[bodyIdx-1][targetIdx]);
							if (bodyIdx - 1 != 0 &&
							    VMaps[bodyIdx].count(p->getIncomingValue(k)))
								p->setIncomingValue(
									k,
									VMaps[bodyIdx-1][p->getIncomingValue(k)]);
						}
					} else {
						p->setIncomingBlock(k, loopBodies[bodyIdx][targetIdx]);
						if (bodyIdx != 0 &&
						    VMaps[bodyIdx].count(p->getIncomingValue(k)))
							p->setIncomingValue(
								k,
								VMaps[bodyIdx][p->getIncomingValue(k)]);
					}
				} else {
					if (bodyIdx == 0)
						;
					else {
						p->removeIncomingValue(k);
						k--;
						nvals--;
					}
				}
			}
		} else {
			nops = it->getNumOperands();
			for (int k = 0; k < nops; k++)
				if (VMaps[bodyIdx].count(it->getOperand(k)))
					it->setOperand(k, VMaps[bodyIdx][it->getOperand(k)]);
		}
	}
}

bool LoopUnrollPass::runOnLoop(llvm::Loop *l, llvm::LPPassManager &lpm)
{
	llvm::SmallVector<llvm::BasicBlock *, 42> succBlocks;
	llvm::BasicBlock *divergeBlock;
	llvm::Function *parentFun;
	std::vector<std::vector<llvm::BasicBlock *> > loopBodies(1, l->getBlocks());
	std::vector<llvm::ValueToValueMapTy> VMaps(unrollDepth);
	std::map<llvm::BasicBlock const *, int> loopBlockIdx;

	l->getExitBlocks(succBlocks);
	parentFun = (*l->block_begin())->getParent();
	divergeBlock = makeDivergeBlock(l);
	for (unsigned i = 0; i < loopBodies[0].size(); i++)
		loopBlockIdx[loopBodies[0][i]] = i;

	/* Clone the blocks making up the loop */
	for (int d = 1; d < unrollDepth; d++) {
		std::stringstream ss;
		ss << "." << d;
		loopBodies.push_back({});
		for (auto it = loopBodies[0].begin(); it != loopBodies[0].end(); it++) {
			llvm::BasicBlock *b = llvm::CloneBasicBlock(*it, VMaps[d], ss.str());
			parentFun->getBasicBlockList().push_back(b);
			loopBodies.back().push_back(b);
#ifdef LLVM_GET_ANALYSIS_LOOP_INFO
			l->addBasicBlockToLoop(b, lpm.getAnalysis<llvm::LoopInfo>().getBase());
#else
			l->addBasicBlockToLoop(b, lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo());
#endif
		}
	}

	/* Redirect all branches, value uses, and Φ nodes to the correct loop version */
	for (int i = 0; i < unrollDepth; i++) {
		for (int j = 0; j < int(loopBodies[i].size()); j++) {
			redirectBranch(i, j, unrollDepth, divergeBlock, loopBlockIdx, loopBodies);
			redirectPHIOrValue(i, j, VMaps, loopBlockIdx, loopBodies);
		}
	}

	VSet<llvm::BasicBlock *> uniqueSuccBlocks(succBlocks.begin(), succBlocks.end());
	for (llvm::BasicBlock *sb : uniqueSuccBlocks) {
		for (auto it = sb->begin(); llvm::isa<llvm::PHINode>(*it) && it != sb->end(); it++) {

			llvm::PHINode *p = static_cast<llvm::PHINode *>(&*it);
			std::vector<llvm::Value *> inVals;
			std::vector<llvm::BasicBlock *> inBlocks;
			std::vector<int> inBlockIdxs;

			for (unsigned i = 0; i < p->getNumIncomingValues(); i++) {
				llvm::BasicBlock *ib = p->getIncomingBlock(i);
				if (loopBlockIdx.count(ib)) {
					inVals.push_back(p->getIncomingValue(i));
					inBlocks.push_back(p->getIncomingBlock(i));
					inBlockIdxs.push_back(loopBlockIdx[ib]);;
				}
			}

			for (int i = 1; i < int(loopBodies.size()); i++) {
				for (int j = 0; j < int(inBlockIdxs.size()); j++) {
					int k = inBlockIdxs[j];
					if (VMaps[i].count(inVals[j]))
						p->addIncoming(VMaps[i][inVals[j]], loopBodies[i][k]);
					else
						p->addIncoming(inVals[j], loopBodies[i][k]);
				}
			}
		}
	}
#ifdef LLVM_HAVE_LOOPINFO_ERASE
	lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo().erase(l);
	lpm.markLoopAsDeleted(*l);
#elif  LLVM_HAVE_LOOPINFO_MARK_AS_REMOVED
	lpm.getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo().markAsRemoved(l);
#else
	lpm.deleteLoopFromQueue(l);
#endif
	return true;
}




char LoopUnrollPass::ID = 42;
// static llvm::RegisterPass<LoopUnrollPass> P("loop-unroll",
// 					    "Unrolls all loops at LLVM-IR level.");
