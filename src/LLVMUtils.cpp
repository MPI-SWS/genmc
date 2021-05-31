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

#include "LLVMUtils.hpp"
#include "CallInstWrapper.hpp"
#include "InterpreterEnumAPI.hpp"

#include <unordered_set>

using namespace llvm;

bool areSameLoadOrdering(AtomicOrdering o1, AtomicOrdering o2)
{
	return o1 == o2 ||
	       (o1 == AtomicOrdering::Acquire && o2 == AtomicOrdering::AcquireRelease) ||
	       (o1 == AtomicOrdering::AcquireRelease && o2 == AtomicOrdering::Acquire);
}

Value *stripCasts(Value *val)
{
	while (isa<CastInst>(val))
		val = dyn_cast<CastInst>(val)->getOperand(0);
	return val;
}

std::string getCalledFunOrStripValName(const CallInst &ci)
{
	if (auto *fun = ci.getCalledFunction())
		return fun->getName().str();
	return CallInstWrapper(const_cast<CallInst *>(&ci)).getCalledOperand()->stripPointerCasts()->getName().str();
}

bool isIntrinsicCallNoSideEffects(const Instruction &i)
{
	auto *ci = dyn_cast<CallInst>(&i);
	if (!ci)
		return false;

	return isCleanInternalFunction(getCalledFunOrStripValName(*ci));
}

bool isDependentOn(const Instruction *i1, const Instruction *i2, VSet<const Instruction *> chain)
{
	if (!i1 || !i2 || chain.find(i1) != chain.end())
		return false;

	for (auto &u : i1->operands()) {
		if (auto *i = dyn_cast<Instruction>(u.get())) {
			chain.insert(i1);
			if (i == i2 || isDependentOn(i, i2, chain))
				return true;
			chain.erase(i1);
		}
	}
	return false;
}

bool isDependentOn(const Instruction *i1, const Instruction *i2)
{
	VSet<const Instruction *> chain;
	return isDependentOn(i1, i2, chain);
}

bool hasSideEffects(const Instruction *i, const VSet<Function *> *cleanFuns /* = nullptr */)
{
	if (isa<AllocaInst>(i))
		return true;
	if (i->mayHaveSideEffects()) {
		if (auto *ci = dyn_cast<CallInst>(i)) {
			auto name = getCalledFunOrStripValName(*ci);
			if (isInternalFunction(name))
				return !isCleanInternalFunction(name);
			if (!cleanFuns)
				return true;
			CallInstWrapper CW(const_cast<CallInst *>(ci));
			const auto *fun = dyn_cast<Function>(CW.getCalledOperand()->stripPointerCasts());
			if (!fun || !cleanFuns->count(const_cast<Function*>(fun)))
				return true;
		} else if (!isa<LoadInst>(i) && !isa<FenceInst>(i)) {
			return true;
		}
	}
	return false;
}
