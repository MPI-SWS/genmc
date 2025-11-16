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

#ifndef GENMC_S_EXPR_VISITOR_HPP
#define GENMC_S_EXPR_VISITOR_HPP

#include "ADT/VSet.hpp"
#include "Static/ModuleID.hpp"
#include "Support/Cast.hpp"
#include "Support/Error.hpp"
#include "Support/MemAccess.hpp"
#include "Support/SExpr.hpp"

#include <map>
#include <unordered_map>

/*******************************************************************************
 **                           SExprVisitor Class
 ******************************************************************************/

/**
 * Visitor for SExpr objects. The pattern is implemented using CRTP (LLVM-style)
 * so as to avoid virtual call overhead.
 *
 * Note: Using visitors with SExpr is not strictly necessary; all the required
 * functionality could be embedded in the SExpr base class as virtual functions.
 * However, they might prove useful if we decide to change the way SExprs are
 * stored, or decide to keep collections of SExprs.
 */

template <template <typename T> class Subclass, typename T, typename RetTy = void>
class SExprVisitor {

public:
#define VISIT_EXPR(NAME)                                                                           \
	case SExpr<T>::NAME:                                                                       \
		return static_cast<Subclass<T> *>(this)->visit##NAME##Expr(                        \
			static_cast<NAME##Expr<T> &>(e));

	RetTy visit(SExpr<T> *e) { return visit(*e); }
	RetTy visit(const std::unique_ptr<SExpr<T>> &e) { return visit(*e); }
	RetTy visit(const std::shared_ptr<SExpr<T>> &e) { return visit(*e); }

	RetTy visit(SExpr<T> &e)
	{
		switch (e.getKind()) {
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

#define DELEGATE_EXPR(TO_CLASS)                                                                    \
	return static_cast<Subclass<T> *>(this)->visit##TO_CLASS(static_cast<TO_CLASS<T> &>(e));

	RetTy visitConcreteExpr(ConcreteExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitRegisterExpr(RegisterExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitSelectExpr(SelectExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	// RetTy visitConcatExpr(ConcatExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	// RetTy visitExtractExpr(ExtractExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitConjunctionExpr(ConjunctionExpr<T> &e) { DELEGATE_EXPR(LogicalExpr); }
	RetTy visitDisjunctionExpr(DisjunctionExpr<T> &e) { DELEGATE_EXPR(LogicalExpr); }
	RetTy visitZExtExpr(ZExtExpr<T> &e) { DELEGATE_EXPR(CastExpr); }
	RetTy visitSExtExpr(SExtExpr<T> &e) { DELEGATE_EXPR(CastExpr); }
	RetTy visitTruncExpr(TruncExpr<T> &e) { DELEGATE_EXPR(CastExpr); }
	RetTy visitNotExpr(NotExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitAddExpr(AddExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitSubExpr(SubExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitMulExpr(MulExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitUDivExpr(UDivExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitSDivExpr(SDivExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitURemExpr(URemExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitSRemExpr(SRemExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitAndExpr(AndExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitOrExpr(OrExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitXorExpr(XorExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitShlExpr(ShlExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitLShrExpr(LShrExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitAShrExpr(AShrExpr<T> &e) { DELEGATE_EXPR(BinaryExpr); }
	RetTy visitEqExpr(EqExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitNeExpr(NeExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitUltExpr(UltExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitUleExpr(UleExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitUgtExpr(UgtExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitUgeExpr(UgeExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitSltExpr(SltExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitSleExpr(SleExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitSgtExpr(SgtExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }
	RetTy visitSgeExpr(SgeExpr<T> &e) { DELEGATE_EXPR(CmpExpr); }

	/*
	 * If none of the above matched, propagate to the next level before
	 * calling the generic visitExpr
	 */
	RetTy visitLogicalExpr(LogicalExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitCastExpr(CastExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitBinaryExpr(BinaryExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	RetTy visitCmpExpr(CmpExpr<T> &e) { DELEGATE_EXPR(SExpr); }

	/*
	 * If no one else could handle this particular instruction, we ignore it.
	 * Note: If a subclass overrides RetTy, this function needs to be overrided too
	 */
	RetTy visitSExpr(SExpr<T> &e) { return; }
};

/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

/**
 * Prints an expression to a string.
 */

template <typename T> class SExprPrinter : public SExprVisitor<SExprPrinter, T> {

public:
	const std::string &toString(SExpr<T> &e)
	{
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
	/** Returns the output constructed so far */
	const std::string &getOutput() const { return output; }

	std::string output;
};

/** Make `SExpr` formattable with `std::format`. */
template <typename U> struct std::formatter<SExpr<U>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SExpr<U> &annot, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}",
				      SExprPrinter<U>().toString(const_cast<SExpr<U> &>(annot)));
	}
};

/*******************************************************************************
 **                           SExprEvaluator Class
 ******************************************************************************/

/**
 * Evaluates an expression. Assumes that there is only 1 symbolic variable
 * (the value of which will be replaced by the concrete value provided),
 * and that are no registers (these should have been concretized).
 */

template <typename T> class SExprEvaluator : public SExprVisitor<SExprEvaluator, T, SVal> {

public:
	/*
	 * We could implement this using a stack for intermediate results,
	 * but overriding the return type is easier
	 */
	using RetTy = SVal;
	using VMap = std::unordered_map<T, RetTy>;

	/** BFE: Evaluates the given expression replacing _all_ symbolic variables with v */
	RetTy evaluate(const SExpr<T> *e, SVal v, size_t *numUnknown = nullptr)
	{
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

	/** NBFE: Evaluates according to a given mapping */
	RetTy evaluate(const SExpr<T> *e, const VMap &map, size_t *numUnknown = nullptr)
	{
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
	/** NBFE: Checks whether a symbolic variable has a mapping */
	bool hasKnownMapping(const T &reg) const
	{
		return valueMapping && valueMapping->count(reg);
	}

	/** NBFE: Returns the value of a symbolic variable */
	RetTy getMappingFor(const T &reg) const
	{
		return (hasKnownMapping(reg)) ? valueMapping->at(reg) : SVal(42);
	}

	/** BFE: Returns the value we are evaluating with in a brute-force eval */
	RetTy getVal() const { return val; }

	/** NBFE: Value mapping we are evaluating with */
	const std::unordered_map<T, RetTy> *valueMapping;

	/** BFE: Value we are evaluating with */
	RetTy val;

	/** Whether this is a BFE */
	bool bruteForce = false;

	/** Unknown symbolic variables seen during an evaluation */
	VSet<T> unknown;
};

/*******************************************************************************
 **                           SExprRegSubstitutor Class
 ******************************************************************************/

/**
 * Replaces all occurrences of a given register with a given expression.
 */

template <typename T> class SExprRegSubstitutor : public SExprVisitor<SExprRegSubstitutor, T> {

public:
	/** Performs the substitution (returns a new expression) */
	std::unique_ptr<SExpr<T>> substitute(const SExpr<T> *orig, T &&reg, const SExpr<T> *r)
	{
		auto e = orig->clone();
		if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.get()))
			if (re->getRegister() == reg)
				return r->clone();

		replaceReg = reg;
		replaceExpr = r;
		this->visit(e.get());
		return e;
	}

	void visitSExpr(SExpr<T> &e)
	{
		for (auto i = 0u; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (re->getRegister() == getRegToReplace())
					e.setKid(i, getReplaceExpr()->clone());
		}
	}

private:
	const T &getRegToReplace() const { return replaceReg; }
	const SExpr<T> *getReplaceExpr() const { return replaceExpr; }

	T replaceReg;
	const SExpr<T> *replaceExpr;
};

/*******************************************************************************
 **                           SExprConcretizer Class
 ******************************************************************************/

/**
 * Applies a given mapping "register->values" to a given expression
 */

template <typename T> class SExprConcretizer : public SExprVisitor<SExprConcretizer, T> {

public:
	using ReplaceMap = std::map<T, std::pair<SVal, ASize>>;

	/** Performs the concretization (returns a new expression) */
	std::unique_ptr<SExpr<T>> concretize(const SExpr<T> *orig, const ReplaceMap &rMap)
	{
		replaceMap = &rMap;
		auto e = orig->clone();
		if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.get())) {
			if (shouldReplace(re->getRegister()))
				return ConcreteExpr<T>::create(getReplaceValSize(re->getRegister()),
							       getReplaceVal(re->getRegister()));
		}
		this->visit(e.get());
		return e;
	}

	void visitSExpr(SExpr<T> &e)
	{
		for (auto i = 0u; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (shouldReplace(re->getRegister()))
					e.setKid(i, ConcreteExpr<T>::create(
							    getReplaceValSize(re->getRegister()),
							    getReplaceVal(re->getRegister())));
		}
	}

private:
	bool shouldReplace(const T &reg) const { return replaceMap->count(reg); }
	SVal getReplaceVal(const T &reg) const { return replaceMap->at(reg).first; }
	typename SExpr<T>::Width getReplaceValSize(const T &reg) const
	{
		return replaceMap->at(reg).second.get();
	}

	const ReplaceMap *replaceMap;
};

/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

/**
 * Given a function F: T -> ModuleID::ID, transforms an SExpr<T> to a SExpr<ModuleID::ID>.
 */

template <typename T>
class SExprTransformer
	: public SExprVisitor<SExprTransformer, T, std::unique_ptr<SExpr<ModuleID::ID>>> {

public:
	using RetTy = std::unique_ptr<SExpr<ModuleID::ID>>;

	/** Performs the transformation (returns a new expression) */
	template <typename F> RetTy transform(SExpr<T> *orig, F &&fun)
	{
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

/**** SExprVisitor templates ****/

/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

template <typename T> void SExprPrinter<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{
	output += e.getValue().toString();
}

template <typename T> void SExprPrinter<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	output += e.getName();
}

template <typename T> void SExprPrinter<T>::visitSelectExpr(SelectExpr<T> &e)
{
	output += "(";
	this->visit(e.getKid(0));
	output += " ? ";
	this->visit(e.getKid(1));
	output += " : ";
	this->visit(e.getKid(2));
	output += ")";
}

// template<typename T>
// void SExprPrinter<T>::visitConcat(ConcatExpr<T> &e)
// {
// }

// template<typename T>
// void SExprPrinter<T>::visitExtract(ExtractExpr<T> &e)
// {
// }

template <typename T> void SExprPrinter<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	for (auto i = 0u; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " /\\ ";
	}
}

template <typename T> void SExprPrinter<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	for (auto i = 0u; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " \\/ ";
	}
}

// template<typename T>
// void SExprPrinter<T>::visitZExtExpr(ZExtExpr<T> &e)
// {
// }

// template<typename T>
// void SExprPrinter<T>::visitSExtExpr(SExtExpr<T> &e)
// {
// }

// template<typename T>
// void SExprPrinter<T>::visitTruncExpr(TruncExpr<T> &e)
// {
// }

template <typename T> void SExprPrinter<T>::visitNotExpr(NotExpr<T> &e)
{
	output += "!";
	this->visit(e.getKid(0));
}

#define PRINT_BINARY_EXPR(op)                                                                      \
	this->visit(e.getKid(0));                                                                  \
	output += std::string(" ") + op + " ";                                                     \
	this->visit(e.getKid(1));

template <typename T> void SExprPrinter<T>::visitAddExpr(AddExpr<T> &e) { PRINT_BINARY_EXPR("+"); }

template <typename T> void SExprPrinter<T>::visitSubExpr(SubExpr<T> &e) { PRINT_BINARY_EXPR("-"); }

template <typename T> void SExprPrinter<T>::visitMulExpr(MulExpr<T> &e) { PRINT_BINARY_EXPR("*"); }

template <typename T> void SExprPrinter<T>::visitUDivExpr(UDivExpr<T> &e)
{
	PRINT_BINARY_EXPR("/u");
}

template <typename T> void SExprPrinter<T>::visitSDivExpr(SDivExpr<T> &e)
{
	PRINT_BINARY_EXPR("/s");
}

template <typename T> void SExprPrinter<T>::visitURemExpr(URemExpr<T> &e)
{
	PRINT_BINARY_EXPR("%u");
}

template <typename T> void SExprPrinter<T>::visitSRemExpr(SRemExpr<T> &e)
{
	PRINT_BINARY_EXPR("%s");
}

template <typename T> void SExprPrinter<T>::visitAndExpr(AndExpr<T> &e) { PRINT_BINARY_EXPR("&"); }

template <typename T> void SExprPrinter<T>::visitOrExpr(OrExpr<T> &e) { PRINT_BINARY_EXPR("|"); }

template <typename T> void SExprPrinter<T>::visitXorExpr(XorExpr<T> &e) { PRINT_BINARY_EXPR("^"); }

template <typename T> void SExprPrinter<T>::visitShlExpr(ShlExpr<T> &e) { PRINT_BINARY_EXPR("<<"); }

template <typename T> void SExprPrinter<T>::visitLShrExpr(LShrExpr<T> &e)
{
	PRINT_BINARY_EXPR("<<a");
}

template <typename T> void SExprPrinter<T>::visitAShrExpr(AShrExpr<T> &e)
{
	PRINT_BINARY_EXPR(">>a");
}

template <typename T> void SExprPrinter<T>::visitEqExpr(EqExpr<T> &e) { PRINT_BINARY_EXPR("=="); }

template <typename T> void SExprPrinter<T>::visitNeExpr(NeExpr<T> &e) { PRINT_BINARY_EXPR("!="); }

template <typename T> void SExprPrinter<T>::visitUltExpr(UltExpr<T> &e) { PRINT_BINARY_EXPR("<u"); }

template <typename T> void SExprPrinter<T>::visitUleExpr(UleExpr<T> &e)
{
	PRINT_BINARY_EXPR("<=u");
}

template <typename T> void SExprPrinter<T>::visitUgtExpr(UgtExpr<T> &e) { PRINT_BINARY_EXPR(">u"); }

template <typename T> void SExprPrinter<T>::visitUgeExpr(UgeExpr<T> &e)
{
	PRINT_BINARY_EXPR(">=u");
}

template <typename T> void SExprPrinter<T>::visitSltExpr(SltExpr<T> &e) { PRINT_BINARY_EXPR("<s"); }

template <typename T> void SExprPrinter<T>::visitSleExpr(SleExpr<T> &e)
{
	PRINT_BINARY_EXPR("<=s");
}

template <typename T> void SExprPrinter<T>::visitSgtExpr(SgtExpr<T> &e) { PRINT_BINARY_EXPR(">s"); }

template <typename T> void SExprPrinter<T>::visitSgeExpr(SgeExpr<T> &e)
{
	PRINT_BINARY_EXPR(">=s");
}

template <typename T> void SExprPrinter<T>::visitLogicalExpr(LogicalExpr<T> &e)
{
	output += "LogOp(";
	for (auto i = 0u; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += ", ";
	}
	output += ")";
}

template <typename T> void SExprPrinter<T>::visitCastExpr(CastExpr<T> &e)
{
	output += "cast" + std::to_string(e.getWidth()) + "(";
	this->visit(e.getKid(0));
	output += ")";
}

template <typename T> void SExprPrinter<T>::visitBinaryExpr(BinaryExpr<T> &e)
{
	output += "BinOp(";
	this->visit(e.getKid(0));
	output += ", ";
	this->visit(e.getKid(1));
	output += ")";
}

template <typename T> void SExprPrinter<T>::visitCmpExpr(CmpExpr<T> &e)
{
	output += "CmpOp(";
	this->visit(e.getKid(0));
	output += ", ";
	this->visit(e.getKid(1));
	output += ")";
}

/*******************************************************************************
 **                           SExprEvaluator Class
 ******************************************************************************/

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{

	return e.getValue();
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	if (bruteForce)
		return getVal();

	if (!hasKnownMapping(e.getRegister()))
		unknown.insert(e.getRegister());
	return getMappingFor(e.getRegister());
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSelectExpr(SelectExpr<T> &e)
{
	return this->visit(e.getKid(0)).getBool() ? this->visit(e.getKid(1))
						  : this->visit(e.getKid(2));
}

// template<typename T>
// typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConcat(ConcatExpr<T> &e)
// {
// }

// template<typename T>
// typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitExtract(ExtractExpr<T> &e)
// {
// }

#define IMPLEMENT_LOGOP(op)                                                                        \
	if (op(e.getKids().begin(), e.getKids().end(),                                             \
	       [&](const std::unique_ptr<SExpr<T>> &kid) { return this->visit(kid).getBool(); }))  \
		return SVal(1);                                                                    \
	return SVal(0);

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	IMPLEMENT_LOGOP(std::all_of);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	IMPLEMENT_LOGOP(std::any_of);
}

/* No special care taken using SVals */
#define IMPLEMENT_CAST(op) return this->visit(e.getKid(0))

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitZExtExpr(ZExtExpr<T> &e)
{
	IMPLEMENT_CAST(zext);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSExtExpr(SExtExpr<T> &e)
{
	IMPLEMENT_CAST(sext);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitTruncExpr(TruncExpr<T> &e)
{
	IMPLEMENT_CAST(trunc);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitNotExpr(NotExpr<T> &e)
{
	return SVal(!e.getKid(0));
}

#define IMPLEMENT_BINOP(op) return this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))

#define IMPLEMENT_BINOP_NONMEM(op) return this->visit(e.getKid(0)) op this->visit(e.getKid(1))

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAddExpr(AddExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(+);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSubExpr(SubExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(-);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitMulExpr(MulExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(*);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUDivExpr(UDivExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSDivExpr(SDivExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitURemExpr(URemExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSRemExpr(SRemExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAndExpr(AndExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(&);
}

template <typename T> typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitOrExpr(OrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(|);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitXorExpr(XorExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(^);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitShlExpr(ShlExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(<<);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitLShrExpr(LShrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAShrExpr(AShrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

#define IMPLEMENT_EQOP(op)                                                                         \
	if (this->visit(e.getKid(0)) op(this->visit(e.getKid(1))))                                 \
		return SVal(1);                                                                    \
	return SVal(0);

template <typename T> typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitEqExpr(EqExpr<T> &e)
{
	IMPLEMENT_EQOP(==);
}

template <typename T> typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitNeExpr(NeExpr<T> &e)
{
	IMPLEMENT_EQOP(!=);
}

#define IMPLEMENT_CMPOP(op)                                                                        \
	return (this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))) ? SVal(1) : SVal(0);

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUltExpr(UltExpr<T> &e)
{
	IMPLEMENT_CMPOP(ult);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUleExpr(UleExpr<T> &e)
{
	IMPLEMENT_CMPOP(ule);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUgtExpr(UgtExpr<T> &e)
{
	IMPLEMENT_CMPOP(ugt);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUgeExpr(UgeExpr<T> &e)
{
	IMPLEMENT_CMPOP(uge);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSltExpr(SltExpr<T> &e)
{
	IMPLEMENT_CMPOP(slt);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSleExpr(SleExpr<T> &e)
{
	IMPLEMENT_CMPOP(sle);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSgtExpr(SgtExpr<T> &e)
{
	IMPLEMENT_CMPOP(sgt);
}

template <typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSgeExpr(SgeExpr<T> &e)
{
	IMPLEMENT_CMPOP(sge);
}

/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{
	return ConcreteExpr<ModuleID::ID>::create(e.getWidth(), e.getValue());
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	return RegisterExpr<ModuleID::ID>::create(e.getWidth(), getTransformer()(e.getRegister()),
						  e.getName());
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSelectExpr(SelectExpr<T> &e)
{
	return SelectExpr<ModuleID::ID>::create(e.getWidth(), this->visit(e.getKid(0)),
						this->visit(e.getKid(1)), this->visit(e.getKid(2)));
}

// template<typename T>
// typename SExprTransformer<T>::RetTy
// SExprTransformer<T>::visitConcat(ConcatExpr<T> &e)
// {
// }

// template<typename T>
// typename SExprTransformer<T>::RetTy
// SExprTransformer<T>::visitExtract(ExtractExpr<T> &e)
// {
// }

#define TRANSFORM_LOGOP(name)                                                                      \
	std::vector<std::unique_ptr<SExpr<ModuleID::ID>>> kids;                                    \
	for (auto &k : e.getKids())                                                                \
		kids.push_back(this->visit(*k));                                                   \
	return name##Expr<ModuleID::ID>::create(std::move(kids));

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	TRANSFORM_LOGOP(Conjunction);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	TRANSFORM_LOGOP(Disjunction);
}

#define TRANSFORM_CAST(name)                                                                       \
	return name##Expr<ModuleID::ID>::create(this->getWidth(), this->visit(e.getKid(0)));

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitZExtExpr(ZExtExpr<T> &e)
{
	IMPLEMENT_CAST(ZExt);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSExtExpr(SExtExpr<T> &e)
{
	IMPLEMENT_CAST(SExt);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitTruncExpr(TruncExpr<T> &e)
{
	IMPLEMENT_CAST(Trunc);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitNotExpr(NotExpr<T> &e)
{
	return NotExpr<ModuleID::ID>::create(this->visit(e.getKid(0)));
}

#define TRANSFORM_BINOP(name)                                                                      \
	return name##Expr<ModuleID::ID>::create(e.getWidth(), this->visit(e.getKid(0)),            \
						this->visit(e.getKid(1)));

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitAddExpr(AddExpr<T> &e)
{
	TRANSFORM_BINOP(Add);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSubExpr(SubExpr<T> &e)
{
	TRANSFORM_BINOP(Sub);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitMulExpr(MulExpr<T> &e)
{
	TRANSFORM_BINOP(Mul);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitUDivExpr(UDivExpr<T> &e)
{
	TRANSFORM_BINOP(UDiv);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSDivExpr(SDivExpr<T> &e)
{
	TRANSFORM_BINOP(SDiv);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitURemExpr(URemExpr<T> &e)
{
	TRANSFORM_BINOP(URem);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSRemExpr(SRemExpr<T> &e)
{
	TRANSFORM_BINOP(SRem);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitAndExpr(AndExpr<T> &e)
{
	TRANSFORM_BINOP(And);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitOrExpr(OrExpr<T> &e)
{
	TRANSFORM_BINOP(Or);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitXorExpr(XorExpr<T> &e)
{
	TRANSFORM_BINOP(Xor);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitShlExpr(ShlExpr<T> &e)
{
	TRANSFORM_BINOP(Shl);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitLShrExpr(LShrExpr<T> &e)
{
	TRANSFORM_BINOP(LShr);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitAShrExpr(AShrExpr<T> &e)
{
	TRANSFORM_BINOP(AShr);
}

#define TRANSFORM_CMPOP(name)                                                                      \
	return name##Expr<ModuleID::ID>::create(this->visit(e.getKid(0)), this->visit(e.getKid(1)));

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitEqExpr(EqExpr<T> &e)
{
	TRANSFORM_CMPOP(Eq);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitNeExpr(NeExpr<T> &e)
{
	TRANSFORM_CMPOP(Ne);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitUltExpr(UltExpr<T> &e)
{
	TRANSFORM_CMPOP(Ult);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitUleExpr(UleExpr<T> &e)
{
	TRANSFORM_CMPOP(Ule);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitUgtExpr(UgtExpr<T> &e)
{
	TRANSFORM_CMPOP(Ugt);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitUgeExpr(UgeExpr<T> &e)
{
	TRANSFORM_CMPOP(Uge);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSltExpr(SltExpr<T> &e)
{
	TRANSFORM_CMPOP(Slt);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSleExpr(SleExpr<T> &e)
{
	TRANSFORM_CMPOP(Sle);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSgtExpr(SgtExpr<T> &e)
{
	TRANSFORM_CMPOP(Sgt);
}

template <typename T>
typename SExprTransformer<T>::RetTy SExprTransformer<T>::visitSgeExpr(SgeExpr<T> &e)
{
	TRANSFORM_CMPOP(Sge);
}

#endif /* GENMC_S_EXPR_VISITOR_HPP */
