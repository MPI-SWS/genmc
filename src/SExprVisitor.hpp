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
#include "MemAccess.hpp"
#include "ModuleID.hpp"
#include "SExpr.hpp"
#include "VSet.hpp"
#include <llvm/Support/Casting.h>

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

template<template<typename T> class Subclass, typename T, typename RetTy = void>
class SExprVisitor {

public:
#define VISIT_EXPR(NAME)						\
	case SExpr<T>::NAME:						\
		return static_cast<Subclass<T> *>(this)->		\
		visit##NAME##Expr(static_cast<NAME##Expr<T>&>(e));

	RetTy visit(SExpr<T> *e) { return visit(*e); }
	RetTy visit(const std::unique_ptr<SExpr<T>> &e) { return visit(*e); }
	RetTy visit(const std::shared_ptr<SExpr<T>> &e) { return visit(*e); }

	RetTy visit(SExpr<T> &e) {
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
	return static_cast<Subclass<T> *>(this)->			\
	visit##TO_CLASS(static_cast<TO_CLASS<T>&>(e));

	RetTy visitConcreteExpr(ConcreteExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitRegisterExpr(RegisterExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitSelectExpr(SelectExpr<T> &e) { DELEGATE(SExpr); }
	// RetTy visitConcatExpr(ConcatExpr<T> &e) { DELEGATE(SExpr); }
	// RetTy visitExtractExpr(ExtractExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitConjunctionExpr(ConjunctionExpr<T> &e) { DELEGATE(LogicalExpr); }
	RetTy visitDisjunctionExpr(DisjunctionExpr<T> &e) { DELEGATE(LogicalExpr); }
	RetTy visitZExtExpr(ZExtExpr<T> &e) { DELEGATE(CastExpr); }
	RetTy visitSExtExpr(SExtExpr<T> &e) { DELEGATE(CastExpr); }
	RetTy visitTruncExpr(TruncExpr<T> &e) { DELEGATE(CastExpr); }
	RetTy visitNotExpr(NotExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitAddExpr(AddExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitSubExpr(SubExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitMulExpr(MulExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitUDivExpr(UDivExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitSDivExpr(SDivExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitURemExpr(URemExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitSRemExpr(SRemExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitAndExpr(AndExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitOrExpr(OrExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitXorExpr(XorExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitShlExpr(ShlExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitLShrExpr(LShrExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitAShrExpr(AShrExpr<T> &e) { DELEGATE(BinaryExpr); }
	RetTy visitEqExpr(EqExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitNeExpr(NeExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitUltExpr(UltExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitUleExpr(UleExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitUgtExpr(UgtExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitUgeExpr(UgeExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitSltExpr(SltExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitSleExpr(SleExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitSgtExpr(SgtExpr<T> &e) { DELEGATE(CmpExpr); }
	RetTy visitSgeExpr(SgeExpr<T> &e) { DELEGATE(CmpExpr); }

	/*
	 * If none of the above matched, propagate to the next level before
	 * calling the generic visitExpr
	 */
	RetTy visitLogicalExpr(LogicalExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitCastExpr(CastExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitBinaryExpr(BinaryExpr<T> &e) { DELEGATE(SExpr); }
	RetTy visitCmpExpr(CmpExpr<T> &e) { DELEGATE(SExpr); }

	/*
	 * If no one else could handle this particular instruction, we ignore it.
	 * Note: If a subclass overrides RetTy, this function needs to be overrided too
	 */
	RetTy visitSExpr(SExpr<T> &e) { return; }

};


/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

/*
 * Prints an expression to a string.
 */

template<typename T>
class SExprPrinter : public SExprVisitor<SExprPrinter, T> {

public:
	const std::string &toString(SExpr<T> &e) {
		this->visit(e);
		return getOutput();
	}

	void visitConcreteExpr(ConcreteExpr<T> &e);
	void visitRegisterExpr(RegisterExpr<T> &e);
	void visitSelectExpr(SelectExpr<T> &e);
	// void visitConcatExpr(ConcatExpr<T> &e);
	// void visitExtractExpr(ExtractExpr<T> &e);
	void visitConjunctionExpr(ConjunctionExpr<T> &e);
	void visitDisjunctionExpr(DisjunctionExpr<T> &e);
	// void visitZExtExpr(ZExtExpr<T> &e);
	// void visitSExtExpr(SExtExpr<T> &e);
	// void visitTruncExpr(TruncExpr<T> &e);
	void visitNotExpr(NotExpr<T> &e);
	void visitAddExpr(AddExpr<T> &e);
	void visitSubExpr(SubExpr<T> &e);
	void visitMulExpr(MulExpr<T> &e);
	void visitUDivExpr(UDivExpr<T> &e);
	void visitSDivExpr(SDivExpr<T> &e);
	void visitURemExpr(URemExpr<T> &e);
	void visitSRemExpr(SRemExpr<T> &e);
	void visitAndExpr(AndExpr<T> &e);
	void visitOrExpr(OrExpr<T> &e);
	void visitXorExpr(XorExpr<T> &e);
	void visitShlExpr(ShlExpr<T> &e);
	void visitLShrExpr(LShrExpr<T> &e);
	void visitAShrExpr(AShrExpr<T> &e);
	void visitEqExpr(EqExpr<T> &e);
	void visitNeExpr(NeExpr<T> &e);
	void visitUltExpr(UltExpr<T> &e);
	void visitUleExpr(UleExpr<T> &e);
	void visitUgtExpr(UgtExpr<T> &e);
	void visitUgeExpr(UgeExpr<T> &e);
	void visitSltExpr(SltExpr<T> &e);
	void visitSleExpr(SleExpr<T> &e);
	void visitSgtExpr(SgtExpr<T> &e);
	void visitSgeExpr(SgeExpr<T> &e);

	void visitLogicalExpr(LogicalExpr<T> &e);
	void visitCastExpr(CastExpr<T> &e);
	void visitBinaryExpr(BinaryExpr<T> &e);
	void visitCmpExpr(CmpExpr<T> &e);

	void visitSExpr(SExpr<T> &e) { output += "unhandled"; }

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

template<typename T>
class SExprEvaluator : public SExprVisitor<SExprEvaluator, T, SVal> {

public:
	/*
	 * We could implement this using a stack for intermediate results,
	 * but overriding the return type is easier
	 */
	using RetTy = SVal;
	using VMap = std::unordered_map<T, RetTy>;

	/* BFE: Evaluates the given expression replacing _all_ symbolic variables with v */
	RetTy evaluate(const SExpr<T> *e, SVal v, size_t *numUnknown = nullptr) {
		bruteForce = true;
		val = v;
		unknown.clear();
		valueMapping = nullptr;
		auto res = this->visit(const_cast<SExpr<T> *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		bruteForce = false;
		return res;
	}

	/* NBFE: Evaluates according to a given mapping */
	RetTy evaluate(const SExpr<T> *e, const VMap &map, size_t *numUnknown = nullptr) {
		valueMapping = &map;
		auto res = this->visit(const_cast<SExpr<T> *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		return res;
	}

	RetTy visitConcreteExpr(ConcreteExpr<T> &e);
	RetTy visitRegisterExpr(RegisterExpr<T> &e);
	RetTy visitSelectExpr(SelectExpr<T> &e);
	// RetTy visitConcatExpr(ConcatExpr<T> &e);
	// RetTy visitExtractExpr(ExtractExpr<T> &e);
	RetTy visitConjunctionExpr(ConjunctionExpr<T> &e);
	RetTy visitDisjunctionExpr(DisjunctionExpr<T> &e);
	RetTy visitZExtExpr(ZExtExpr<T> &e);
	RetTy visitSExtExpr(SExtExpr<T> &e);
	RetTy visitTruncExpr(TruncExpr<T> &e);
	RetTy visitNotExpr(NotExpr<T> &e);
	RetTy visitAddExpr(AddExpr<T> &e);
	RetTy visitSubExpr(SubExpr<T> &e);
	RetTy visitMulExpr(MulExpr<T> &e);
	RetTy visitUDivExpr(UDivExpr<T> &e);
	RetTy visitSDivExpr(SDivExpr<T> &e);
	RetTy visitURemExpr(URemExpr<T> &e);
	RetTy visitSRemExpr(SRemExpr<T> &e);
	RetTy visitAndExpr(AndExpr<T> &e);
	RetTy visitOrExpr(OrExpr<T> &e);
	RetTy visitXorExpr(XorExpr<T> &e);
	RetTy visitShlExpr(ShlExpr<T> &e);
	RetTy visitLShrExpr(LShrExpr<T> &e);
	RetTy visitAShrExpr(AShrExpr<T> &e);
	RetTy visitEqExpr(EqExpr<T> &e);
	RetTy visitNeExpr(NeExpr<T> &e);
	RetTy visitUltExpr(UltExpr<T> &e);
	RetTy visitUleExpr(UleExpr<T> &e);
	RetTy visitUgtExpr(UgtExpr<T> &e);
	RetTy visitUgeExpr(UgeExpr<T> &e);
	RetTy visitSltExpr(SltExpr<T> &e);
	RetTy visitSleExpr(SleExpr<T> &e);
	RetTy visitSgtExpr(SgtExpr<T> &e);
	RetTy visitSgeExpr(SgeExpr<T> &e);

	RetTy visitSExpr(SExpr<T> &e) { BUG(); }

private:
	/* NBFE: Checks whether a symbolic variable has a mapping */
	bool hasKnownMapping(const T& reg) const { return valueMapping && valueMapping->count(reg); }

	/* NBFE: Returns the value of a symbolic variable */
	RetTy getMappingFor(const T& reg) const {
		return (hasKnownMapping(reg)) ? valueMapping->at(reg) :	SVal(42);
	}

	/* BFE: Returns the value we are evaluating with in a brute-force eval */
	RetTy getVal() const { return val; }

	/* NBFE: Value mapping we are evaluating with */
	const std::unordered_map<T, RetTy> *valueMapping;

	/* BFE: Value we are evaluating with */
	RetTy val;

	/* Whether this is a BFE */
	bool bruteForce = false;

	/* Unknown symbolic variables seen during an evaluation */
	VSet<T> unknown;
};

/*******************************************************************************
 **                           SExprRegSubstitutor Class
 ******************************************************************************/

/*
 * Replaces all occurrences of a given register with a given expression.
 */

template<typename T>
class SExprRegSubstitutor : public SExprVisitor<SExprRegSubstitutor, T> {

public:
	/* Performs the substitution (returns a new expression) */
	std::unique_ptr<SExpr<T>>
	substitute(const SExpr<T> *orig, T&& reg, const SExpr<T> *r) {
		auto e = orig->clone();
		if (auto *re = llvm::dyn_cast<RegisterExpr<T>>(e.get()))
			if (re->getRegister() == reg)
				return r->clone();

		replaceReg = reg;
		replaceExpr = r;
		this->visit(e.get());
		return e;
	}

	void visitSExpr(SExpr<T> &e) {
		for (auto i = 0u; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = llvm::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (re->getRegister() == getRegToReplace())
					e.setKid(i, getReplaceExpr()->clone());
		}
	}

private:
	const T& getRegToReplace() const { return replaceReg; }
	const SExpr<T> *getReplaceExpr() const { return replaceExpr; }

	T replaceReg;
	const SExpr<T> *replaceExpr;
};


/*******************************************************************************
 **                           SExprConcretizer Class
 ******************************************************************************/

/*
 * Applies a given mapping "register->values" to a given expression
 */

template<typename T>
class SExprConcretizer: public SExprVisitor<SExprConcretizer, T> {

public:
	using ReplaceMap = std::map<T, std::pair<SVal, ASize>>;

	/* Performs the concretization (returns a new expression) */
	std::unique_ptr<SExpr<T>>
	concretize(const SExpr<T> *orig, const ReplaceMap &rMap) {
		replaceMap = &rMap;
		auto e = orig->clone();
		if (auto *re = llvm::dyn_cast<RegisterExpr<T>>(e.get())) {
			if (shouldReplace(re->getRegister()))
				return ConcreteExpr<T>::create(getReplaceValSize(re->getRegister()),
							       getReplaceVal(re->getRegister()));
		}
		this->visit(e.get());
		return e;
	}

	void visitSExpr(SExpr<T> &e) {
		for (auto i = 0u; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = llvm::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (shouldReplace(re->getRegister()))
					e.setKid(i, ConcreteExpr<T>::create(getReplaceValSize(re->getRegister()),
									    getReplaceVal(re->getRegister())));
		}
	}

private:
	bool shouldReplace(const T& reg) const { return replaceMap->count(reg); }
	SVal getReplaceVal(const T& reg) const { return replaceMap->at(reg).first; }
	typename SExpr<T>::Width getReplaceValSize(const T& reg) const { return replaceMap->at(reg).second.get(); }

	const ReplaceMap *replaceMap;
};


/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

/*
 * Given a function F: T -> ModuleID::ID, transforms an SExpr<T> to a SExpr<ModuleID::ID>.
 */

template<typename T>
class SExprTransformer : public SExprVisitor<SExprTransformer, T, std::unique_ptr<SExpr<ModuleID::ID>>> {

public:
	using RetTy = std::unique_ptr<SExpr<ModuleID::ID>>;

	/* Performs the transformation (returns a new expression) */
	template<typename F>
	RetTy transform(SExpr<T> *orig, F&& fun) {
		transformer = fun;
		return this->visit(*orig);
	}

	RetTy visitConcreteExpr(ConcreteExpr<T> &e);
	RetTy visitRegisterExpr(RegisterExpr<T> &e);
	RetTy visitSelectExpr(SelectExpr<T> &e);
	// RetTy visitConcatExpr(ConcatExpr<T> &e);
	// RetTy visitExtractExpr(ExtractExpr<T> &e);
	RetTy visitConjunctionExpr(ConjunctionExpr<T> &e);
	RetTy visitDisjunctionExpr(DisjunctionExpr<T> &e);
	RetTy visitZExtExpr(ZExtExpr<T> &e);
	RetTy visitSExtExpr(SExtExpr<T> &e);
	RetTy visitTruncExpr(TruncExpr<T> &e);
	RetTy visitNotExpr(NotExpr<T> &e);
	RetTy visitAddExpr(AddExpr<T> &e);
	RetTy visitSubExpr(SubExpr<T> &e);
	RetTy visitMulExpr(MulExpr<T> &e);
	RetTy visitUDivExpr(UDivExpr<T> &e);
	RetTy visitSDivExpr(SDivExpr<T> &e);
	RetTy visitURemExpr(URemExpr<T> &e);
	RetTy visitSRemExpr(SRemExpr<T> &e);
	RetTy visitAndExpr(AndExpr<T> &e);
	RetTy visitOrExpr(OrExpr<T> &e);
	RetTy visitXorExpr(XorExpr<T> &e);
	RetTy visitShlExpr(ShlExpr<T> &e);
	RetTy visitLShrExpr(LShrExpr<T> &e);
	RetTy visitAShrExpr(AShrExpr<T> &e);
	RetTy visitEqExpr(EqExpr<T> &e);
	RetTy visitNeExpr(NeExpr<T> &e);
	RetTy visitUltExpr(UltExpr<T> &e);
	RetTy visitUleExpr(UleExpr<T> &e);
	RetTy visitUgtExpr(UgtExpr<T> &e);
	RetTy visitUgeExpr(UgeExpr<T> &e);
	RetTy visitSltExpr(SltExpr<T> &e);
	RetTy visitSleExpr(SleExpr<T> &e);
	RetTy visitSgtExpr(SgtExpr<T> &e);
	RetTy visitSgeExpr(SgeExpr<T> &e);

	RetTy visitSExpr(SExpr<T> &e) { BUG(); }

private:
	const std::function<ModuleID::ID(T)> &getTransformer() const { return transformer; }

	std::function<ModuleID::ID(T)> transformer;
};

#include "SExprVisitor.tcc"

#endif /* __S_EXPR_VISITOR_HPP__ */
