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

#ifndef __S_EXPR_VISITOR_HPP__
#define __S_EXPR_VISITOR_HPP__

#include "Error.hpp"
#include "SExpr.hpp"
#include "VSet.hpp"
#include <llvm/IR/Type.h>

#include <map>
#include <unordered_map>

/*******************************************************************************
 **                           SExprVisitor Class
 ******************************************************************************/

/*
 * Visitor for SExpr objects. The pattern is implemented using CRTP (LLVM-style)
 * so as to avoid virtual call overhead.
 *
 * Note: Using visitors with SExpr is not strictly necessary; all the required
 * functionality could be embedded in the SExpr base class as virtual functions.
 * However, they might prove useful if we decide to change the way SExprs are
 * stored, or decide to keep collections of SExprs.
 */

template<typename Subclass, typename RetTy = void>
class SExprVisitor {

public:
#define VISIT_EXPR(NAME)						\
	case SExpr::NAME:						\
		return static_cast<Subclass *>(this)->			\
		visit##NAME##Expr(static_cast<NAME##Expr&>(e));

	RetTy visit(SExpr *e) { return visit(*e); }
	RetTy visit(const std::unique_ptr<SExpr> &e) { return visit(*e); }
	RetTy visit(const std::shared_ptr<SExpr> &e) { return visit(*e); }

	RetTy visit(SExpr &e) {
		switch(e.getKind()) {
			VISIT_EXPR(Concrete);
			VISIT_EXPR(Register);
			VISIT_EXPR(Select);
			// VISIT_EXPR(Concat);
			// VISIT_EXPR(Extract);
			VISIT_EXPR(Conjunction);
			VISIT_EXPR(Disjunction);
			VISIT_EXPR(ZExt);
			VISIT_EXPR(SExt);
			VISIT_EXPR(Trunc);
			VISIT_EXPR(Not);
			VISIT_EXPR(Add);
			VISIT_EXPR(Sub);
			VISIT_EXPR(Mul);
			VISIT_EXPR(UDiv);
			VISIT_EXPR(SDiv);
			VISIT_EXPR(URem);
			VISIT_EXPR(SRem);
			VISIT_EXPR(And);
			VISIT_EXPR(Or);
			VISIT_EXPR(Xor);
			VISIT_EXPR(Shl);
			VISIT_EXPR(LShr);
			VISIT_EXPR(AShr);
			VISIT_EXPR(Eq);
			VISIT_EXPR(Ne);
			VISIT_EXPR(Ult);
			VISIT_EXPR(Ule);
			VISIT_EXPR(Ugt);
			VISIT_EXPR(Uge);
			VISIT_EXPR(Slt);
			VISIT_EXPR(Sle);
			VISIT_EXPR(Sgt);
			VISIT_EXPR(Sge);
		default:
			BUG();
		}
	}

#define DELEGATE(TO_CLASS)						\
	return static_cast<Subclass *>(this)->				\
	visit##TO_CLASS(static_cast<TO_CLASS&>(e));

	RetTy visitConcreteExpr(ConcreteExpr &e) { DELEGATE(SExpr); }
	RetTy visitRegisterExpr(RegisterExpr &e) { DELEGATE(SExpr); }
	RetTy visitSelectExpr(SelectExpr &e) { DELEGATE(SExpr); }
	// RetTy visitConcatExpr(ConcatExpr &e) { DELEGATE(SExpr); }
	// RetTy visitExtractExpr(ExtractExpr &e) { DELEGATE(SExpr); }
	RetTy visitConjunctionExpr(ConjunctionExpr &e) { DELEGATE(LogicalExpr); }
	RetTy visitDisjunctionExpr(DisjunctionExpr &e) { DELEGATE(LogicalExpr); }
	RetTy visitZExtExpr(ZExtExpr &e) { DELEGATE(CastExpr); }
	RetTy visitSExtExpr(SExtExpr &e) { DELEGATE(CastExpr); }
	RetTy visitTruncExpr(TruncExpr &e) { DELEGATE(CastExpr); }
	RetTy visitNotExpr(NotExpr &e) { DELEGATE(SExpr); }
	RetTy visitAddExpr(AddExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitSubExpr(SubExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitMulExpr(MulExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitUDivExpr(UDivExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitSDivExpr(SDivExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitURemExpr(URemExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitSRemExpr(SRemExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitAndExpr(AndExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitOrExpr(OrExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitXorExpr(XorExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitShlExpr(ShlExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitLShrExpr(LShrExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitAShrExpr(AShrExpr &e) { DELEGATE(BinaryExpr); }
	RetTy visitEqExpr(EqExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitNeExpr(NeExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitUltExpr(UltExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitUleExpr(UleExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitUgtExpr(UgtExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitUgeExpr(UgeExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitSltExpr(SltExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitSleExpr(SleExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitSgtExpr(SgtExpr &e) { DELEGATE(CmpExpr); }
	RetTy visitSgeExpr(SgeExpr &e) { DELEGATE(CmpExpr); }

	/*
	 * If none of the above matched, propagate to the next level before
	 * calling the generic visitExpr
	 */
	RetTy visitLogicalExpr(LogicalExpr &e) { DELEGATE(SExpr); }
	RetTy visitCastExpr(CastExpr &e) { DELEGATE(SExpr); }
	RetTy visitBinaryExpr(BinaryExpr &e) { DELEGATE(SExpr); }
	RetTy visitCmpExpr(CmpExpr &e) { DELEGATE(SExpr); }

	/*
	 * If no one else could handle this particular instruction, we ignore it.
	 * Note: If a subclass overrides RetTy, this function needs to be overrided too
	 */
	RetTy visitSExpr(SExpr &e) { return; }

};


/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

/*
 * Prints an expression to a string.
 */

class SExprPrinter: public SExprVisitor<SExprPrinter> {

public:
	const std::string &toString(SExpr &e) {
		visit(e);
		return getOutput();
	}

	void visitConcreteExpr(ConcreteExpr &e);
	void visitRegisterExpr(RegisterExpr &e);
	void visitSelectExpr(SelectExpr &e);
	// void visitConcatExpr(ConcatExpr &e);
	// void visitExtractExpr(ExtractExpr &e);
	void visitConjunctionExpr(ConjunctionExpr &e);
	void visitDisjunctionExpr(DisjunctionExpr &e);
	// void visitZExtExpr(ZExtExpr &e);
	// void visitSExtExpr(SExtExpr &e);
	// void visitTruncExpr(TruncExpr &e);
	void visitNotExpr(NotExpr &e);
	void visitAddExpr(AddExpr &e);
	void visitSubExpr(SubExpr &e);
	void visitMulExpr(MulExpr &e);
	void visitUDivExpr(UDivExpr &e);
	void visitSDivExpr(SDivExpr &e);
	void visitURemExpr(URemExpr &e);
	void visitSRemExpr(SRemExpr &e);
	void visitAndExpr(AndExpr &e);
	void visitOrExpr(OrExpr &e);
	void visitXorExpr(XorExpr &e);
	void visitShlExpr(ShlExpr &e);
	void visitLShrExpr(LShrExpr &e);
	void visitAShrExpr(AShrExpr &e);
	void visitEqExpr(EqExpr &e);
	void visitNeExpr(NeExpr &e);
	void visitUltExpr(UltExpr &e);
	void visitUleExpr(UleExpr &e);
	void visitUgtExpr(UgtExpr &e);
	void visitUgeExpr(UgeExpr &e);
	void visitSltExpr(SltExpr &e);
	void visitSleExpr(SleExpr &e);
	void visitSgtExpr(SgtExpr &e);
	void visitSgeExpr(SgeExpr &e);

	void visitLogicalExpr(LogicalExpr &e);
	void visitCastExpr(CastExpr &e);
	void visitBinaryExpr(BinaryExpr &e);
	void visitCmpExpr(CmpExpr &e);

	void visitSExpr(SExpr &e) { output += "unhandled"; }

private:
	/* Returns the output constructed so far */
	const std::string &getOutput() const { return output; }

	std::string output;
};


/*******************************************************************************
 **                           SExprEvaluator Class
 ******************************************************************************/

/*
 * Evaluates an expression. Assumes that there is only 1 symbolic variable
 * (the value of which will be replaced by the concrete value provided),
 * and that are no registers (these should have been concretized).
 */

class SExprEvaluator: public SExprVisitor<SExprEvaluator, llvm::APInt> {

public:
	/*
	 * We could implement this using a stack for intermediate results,
	 * but overriding the return type is easier
	 */
	using RetTy = llvm::APInt;

	/* BFE: Evaluates the given expression replacing _all_ symbolic variables with v */
	RetTy evaluate(const SExpr *e, llvm::APInt v, size_t *numUnknown = nullptr) {
		bruteForce = true;
		val = v;
		unknown.clear();
		valueMapping = nullptr;
		auto res = visit(const_cast<SExpr *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		bruteForce = false;
		return res;
	}
	RetTy evaluate(const SExpr *e, const llvm::GenericValue &v, size_t *numUnknown = nullptr) {
		return evaluate(e, v.IntVal, numUnknown);
	}

	/* NBFE: Evaluates according to a given mapping */
	RetTy evaluate(const SExpr *e, const std::unordered_map<llvm::Value *, llvm::APInt> &map,
		       size_t *numUnknown = nullptr) {
		valueMapping = &map;
		auto res = visit(const_cast<SExpr *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		return res;
	}

	RetTy visitConcreteExpr(ConcreteExpr &e);
	RetTy visitRegisterExpr(RegisterExpr &e);
	RetTy visitSelectExpr(SelectExpr &e);
	// RetTy visitConcatExpr(ConcatExpr &e);
	// RetTy visitExtractExpr(ExtractExpr &e);
	RetTy visitConjunctionExpr(ConjunctionExpr &e);
	RetTy visitDisjunctionExpr(DisjunctionExpr &e);
	RetTy visitZExtExpr(ZExtExpr &e);
	RetTy visitSExtExpr(SExtExpr &e);
	RetTy visitTruncExpr(TruncExpr &e);
	RetTy visitNotExpr(NotExpr &e);
	RetTy visitAddExpr(AddExpr &e);
	RetTy visitSubExpr(SubExpr &e);
	RetTy visitMulExpr(MulExpr &e);
	RetTy visitUDivExpr(UDivExpr &e);
	RetTy visitSDivExpr(SDivExpr &e);
	RetTy visitURemExpr(URemExpr &e);
	RetTy visitSRemExpr(SRemExpr &e);
	RetTy visitAndExpr(AndExpr &e);
	RetTy visitOrExpr(OrExpr &e);
	RetTy visitXorExpr(XorExpr &e);
	RetTy visitShlExpr(ShlExpr &e);
	RetTy visitLShrExpr(LShrExpr &e);
	RetTy visitAShrExpr(AShrExpr &e);
	RetTy visitEqExpr(EqExpr &e);
	RetTy visitNeExpr(NeExpr &e);
	RetTy visitUltExpr(UltExpr &e);
	RetTy visitUleExpr(UleExpr &e);
	RetTy visitUgtExpr(UgtExpr &e);
	RetTy visitUgeExpr(UgeExpr &e);
	RetTy visitSltExpr(SltExpr &e);
	RetTy visitSleExpr(SleExpr &e);
	RetTy visitSgtExpr(SgtExpr &e);
	RetTy visitSgeExpr(SgeExpr &e);

	RetTy visitSExpr(SExpr &e) { BUG(); }

private:
	/* NBFE: Checks whether a symbolic variable has a mapping */
	bool hasKnownMapping(llvm::Value *reg) const { return valueMapping && valueMapping->count(reg); }

	/* NBFE: Returns the value of a symbolic variable */
	llvm::APInt getMappingFor(llvm::Value *reg) const {
		return (hasKnownMapping(reg)) ? valueMapping->at(reg) :
			llvm::APInt(reg->getType()->getPrimitiveSizeInBits(), 42);
	}

	/* BFE: Returns the value we are evaluating with in a brute-force eval */
	llvm::APInt getVal() const { return val; }

	/* NBFE: Value mapping we are evaluating with */
	const std::unordered_map<llvm::Value *, llvm::APInt> *valueMapping;

	/* BFE: Value we are evaluating with */
	llvm::APInt val;

	/* Whether this is a BFE */
	bool bruteForce = false;

	/* Unknown symbolic variables seen during an evaluation */
	VSet<llvm::Value *> unknown;
};

/*******************************************************************************
 **                           SExprRegSubstitutor Class
 ******************************************************************************/

/*
 * Replaces all occurrences of a given register with a given expression.
 */

class SExprRegSubstitutor: public SExprVisitor<SExprRegSubstitutor> {

public:
	/* Performs the substitution (returns a new expression) */
	std::unique_ptr<SExpr>
	substitute(const SExpr *orig, llvm::Value *reg,
		   const SExpr *r) {
		auto e = orig->clone();
		if (auto *re = llvm::dyn_cast<RegisterExpr>(e.get()))
			if (re->getRegister() == reg)
				return r->clone();

		replaceReg = reg;
		replaceExpr = r;
		visit(e.get());
		return e;
	}

	void visitSExpr(SExpr &e) {
		for (auto i = 0u; i < e.getNumKids(); i++) {
			visit(e.getKid(i));
			if (auto *re = llvm::dyn_cast<RegisterExpr>(e.getKid(i)))
				if (re->getRegister() == getRegToReplace())
					e.setKid(i, getReplaceExpr()->clone());
		}
	}

private:
	llvm::Value *getRegToReplace() const { return replaceReg; }
	const SExpr *getReplaceExpr() const { return replaceExpr; }

	llvm::Value *replaceReg;
	const SExpr *replaceExpr;
};


/*******************************************************************************
 **                           SExprConcretizer Class
 ******************************************************************************/

/*
 * Applies a given mapping "register->values" to a given expression
 */

class SExprConcretizer: public SExprVisitor<SExprConcretizer> {

public:
	/* Performs the concretization (returns a new expression) */
	std::unique_ptr<SExpr>
	concretize(const SExpr *orig,
		   const std::map<llvm::Value *, llvm::GenericValue> &rMap) {
		auto e = orig->clone();
		if (auto *re = llvm::dyn_cast<RegisterExpr>(e.get()))
			if (shouldReplace(re->getRegister()))
				return ConcreteExpr::create(getReplaceVal(re->getRegister()));

		replaceMap = &rMap;
		visit(e.get());
		return e;
	}

	void visitSExpr(SExpr &e) {
		for (auto i = 0u; i < e.getNumKids(); i++) {
			visit(e.getKid(i));
			if (auto *re = llvm::dyn_cast<RegisterExpr>(e.getKid(i)))
				if (shouldReplace(re->getRegister()))
					e.setKid(i, ConcreteExpr::create(getReplaceVal(re->getRegister())));
		}
	}

private:
	bool shouldReplace(llvm::Value *reg) const { return replaceMap->count(reg); }
	llvm::APInt getReplaceVal(llvm::Value *reg) const { return replaceMap->at(reg).IntVal; }

	const std::map<llvm::Value *, llvm::GenericValue> *replaceMap;
};

#endif /* __S_EXPR_VISITOR_HPP__ */
