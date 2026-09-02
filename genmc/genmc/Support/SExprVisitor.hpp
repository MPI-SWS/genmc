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

#include "genmc/ADT/VSet.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/MemAccess.hpp"
#include "genmc/Support/ModuleVarID.hpp"
#include "genmc/Support/SExpr.hpp"

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

	auto visit(SExpr<T> *e) -> RetTy { return visit(*e); }
	auto visit(const std::unique_ptr<SExpr<T>> &e) -> RetTy { return visit(*e); }
	auto visit(const std::shared_ptr<SExpr<T>> &e) -> RetTy { return visit(*e); }

	auto visit(SExpr<T> &e) -> RetTy
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
			UNREACHABLE();
		}
	}

#define DELEGATE_EXPR(TO_CLASS)                                                                    \
	return static_cast<Subclass<T> *>(this)->visit##TO_CLASS(static_cast<TO_CLASS<T> &>(e));

	auto visitConcreteExpr(ConcreteExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitRegisterExpr(RegisterExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitSelectExpr(SelectExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	// RetTy visitConcatExpr(ConcatExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	// RetTy visitExtractExpr(ExtractExpr<T> &e) { DELEGATE_EXPR(SExpr); }
	auto visitConjunctionExpr(ConjunctionExpr<T> &e) -> RetTy { DELEGATE_EXPR(LogicalExpr); }
	auto visitDisjunctionExpr(DisjunctionExpr<T> &e) -> RetTy { DELEGATE_EXPR(LogicalExpr); }
	auto visitZExtExpr(ZExtExpr<T> &e) -> RetTy { DELEGATE_EXPR(CastExpr); }
	auto visitSExtExpr(SExtExpr<T> &e) -> RetTy { DELEGATE_EXPR(CastExpr); }
	auto visitTruncExpr(TruncExpr<T> &e) -> RetTy { DELEGATE_EXPR(CastExpr); }
	auto visitNotExpr(NotExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitAddExpr(AddExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitSubExpr(SubExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitMulExpr(MulExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitUDivExpr(UDivExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitSDivExpr(SDivExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitURemExpr(URemExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitSRemExpr(SRemExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitAndExpr(AndExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitOrExpr(OrExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitXorExpr(XorExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitShlExpr(ShlExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitLShrExpr(LShrExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitAShrExpr(AShrExpr<T> &e) -> RetTy { DELEGATE_EXPR(BinaryExpr); }
	auto visitEqExpr(EqExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitNeExpr(NeExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitUltExpr(UltExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitUleExpr(UleExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitUgtExpr(UgtExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitUgeExpr(UgeExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitSltExpr(SltExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitSleExpr(SleExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitSgtExpr(SgtExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }
	auto visitSgeExpr(SgeExpr<T> &e) -> RetTy { DELEGATE_EXPR(CmpExpr); }

	/*
	 * If none of the above matched, propagate to the next level before
	 * calling the generic visitExpr
	 */
	auto visitLogicalExpr(LogicalExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitCastExpr(CastExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitBinaryExpr(BinaryExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }
	auto visitCmpExpr(CmpExpr<T> &e) -> RetTy { DELEGATE_EXPR(SExpr); }

	/*
	 * If no one else could handle this particular instruction, we ignore it.
	 * Note: If a subclass overrides RetTy, this function needs to be overrided too
	 */
	auto visitSExpr(SExpr<T> & /*e*/) -> RetTy { return; }
};

/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

/**
 * Prints an expression to a string.
 */

template <typename T> class SExprPrinter : public SExprVisitor<SExprPrinter, T> {

public:
	auto toString(SExpr<T> &e) -> const std::string &
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

	void visitSExpr(SExpr<T> & /*e*/) { output += "unhandled"; }

private:
	/** Returns the output constructed so far */
	[[nodiscard]] auto getOutput() const -> const std::string & { return output; }

	std::string output;
};

/** Make `SExpr` formattable with `std::format`. */
template <typename U> struct std::formatter<SExpr<U>> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SExpr<U> &annot, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}",
				      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
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
	auto evaluate(const SExpr<T> *e, SVal v, size_t *numUnknown = nullptr) -> RetTy
	{
		bruteForce = true;
		val = v;
		unknown.clear();
		valueMapping = nullptr;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		auto res = this->visit(const_cast<SExpr<T> *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		bruteForce = false;
		return res;
	}

	/** NBFE: Evaluates according to a given mapping */
	auto evaluate(const SExpr<T> *e, const VMap &map, size_t *numUnknown = nullptr) -> RetTy
	{
		valueMapping = &map;
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
		auto res = this->visit(const_cast<SExpr<T> *>(e));
		if (numUnknown)
			*numUnknown = unknown.size();
		return res;
	}

	auto visitConcreteExpr(ConcreteExpr<T> &e) -> RetTy;
	auto visitRegisterExpr(RegisterExpr<T> &e) -> RetTy;
	auto visitSelectExpr(SelectExpr<T> &e) -> RetTy;
	// RetTy visitConcatExpr(ConcatExpr<T> &e);
	// RetTy visitExtractExpr(ExtractExpr<T> &e);
	auto visitConjunctionExpr(ConjunctionExpr<T> &e) -> RetTy;
	auto visitDisjunctionExpr(DisjunctionExpr<T> &e) -> RetTy;
	auto visitZExtExpr(ZExtExpr<T> &e) -> RetTy;
	auto visitSExtExpr(SExtExpr<T> &e) -> RetTy;
	auto visitTruncExpr(TruncExpr<T> &e) -> RetTy;
	auto visitNotExpr(NotExpr<T> &e) -> RetTy;
	auto visitAddExpr(AddExpr<T> &e) -> RetTy;
	auto visitSubExpr(SubExpr<T> &e) -> RetTy;
	auto visitMulExpr(MulExpr<T> &e) -> RetTy;
	auto visitUDivExpr(UDivExpr<T> &e) -> RetTy;
	auto visitSDivExpr(SDivExpr<T> &e) -> RetTy;
	auto visitURemExpr(URemExpr<T> &e) -> RetTy;
	auto visitSRemExpr(SRemExpr<T> &e) -> RetTy;
	auto visitAndExpr(AndExpr<T> &e) -> RetTy;
	auto visitOrExpr(OrExpr<T> &e) -> RetTy;
	auto visitXorExpr(XorExpr<T> &e) -> RetTy;
	auto visitShlExpr(ShlExpr<T> &e) -> RetTy;
	auto visitLShrExpr(LShrExpr<T> &e) -> RetTy;
	auto visitAShrExpr(AShrExpr<T> &e) -> RetTy;
	auto visitEqExpr(EqExpr<T> &e) -> RetTy;
	auto visitNeExpr(NeExpr<T> &e) -> RetTy;
	auto visitUltExpr(UltExpr<T> &e) -> RetTy;
	auto visitUleExpr(UleExpr<T> &e) -> RetTy;
	auto visitUgtExpr(UgtExpr<T> &e) -> RetTy;
	auto visitUgeExpr(UgeExpr<T> &e) -> RetTy;
	auto visitSltExpr(SltExpr<T> &e) -> RetTy;
	auto visitSleExpr(SleExpr<T> &e) -> RetTy;
	auto visitSgtExpr(SgtExpr<T> &e) -> RetTy;
	auto visitSgeExpr(SgeExpr<T> &e) -> RetTy;

	auto visitSExpr(SExpr<T> & /*e*/) -> RetTy { UNREACHABLE(); }

private:
	/** NBFE: Checks whether a symbolic variable has a mapping */
	[[nodiscard]] auto hasKnownMapping(const T &reg) const -> bool
	{
		return valueMapping && valueMapping->contains(reg);
	}

	/** NBFE: Returns the value of a symbolic variable */
	[[nodiscard]] auto getMappingFor(const T &reg) const -> RetTy
	{
		// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
		return (hasKnownMapping(reg)) ? valueMapping->at(reg) : SVal(42);
	}

	/** BFE: Returns the value we are evaluating with in a brute-force eval */
	[[nodiscard]] auto getVal() const -> RetTy { return val; }

	/** NBFE: Value mapping we are evaluating with */
	const std::unordered_map<T, RetTy> *valueMapping = nullptr;

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
	auto substitute(const SExpr<T> *orig, const T &reg, const SExpr<T> *r)
		-> std::unique_ptr<SExpr<T>>
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
		for (auto i = 0U; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (re->getRegister() == getRegToReplace())
					e.setKid(i, getReplaceExpr()->clone());
		}
	}

private:
	auto getRegToReplace() const -> const T & { return replaceReg; }
	auto getReplaceExpr() const -> const SExpr<T> * { return replaceExpr; }

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
	auto concretize(const SExpr<T> *orig, const ReplaceMap &rMap) -> std::unique_ptr<SExpr<T>>
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
		for (auto i = 0U; i < e.getNumKids(); i++) {
			this->visit(e.getKid(i));
			if (auto *re = genmc::dyn_cast<RegisterExpr<T>>(e.getKid(i)))
				if (shouldReplace(re->getRegister()))
					e.setKid(i, ConcreteExpr<T>::create(
							    getReplaceValSize(re->getRegister()),
							    getReplaceVal(re->getRegister())));
		}
	}

private:
	auto shouldReplace(const T &reg) const -> bool { return replaceMap->count(reg); }
	auto getReplaceVal(const T &reg) const -> SVal { return replaceMap->at(reg).first; }
	auto getReplaceValSize(const T &reg) const -> typename SExpr<T>::Width
	{
		return replaceMap->at(reg).second.get();
	}

	const ReplaceMap *replaceMap;
};

/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

/**
 * Given a function F: T -> ModuleVarID, transforms an SExpr<T> to a SExpr<ModuleVarID>.
 */

template <typename T>
class SExprTransformer
	: public SExprVisitor<SExprTransformer, T, std::unique_ptr<SExpr<ModuleVarID>>> {

public:
	using RetTy = std::unique_ptr<SExpr<ModuleVarID>>;

	/** Performs the transformation (returns a new expression) */
	// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
	template <typename F> auto transform(SExpr<T> *orig, F &&fun) -> RetTy
	{
		transformer = fun;
		return this->visit(*orig);
	}

	auto visitConcreteExpr(ConcreteExpr<T> &e) -> RetTy;
	auto visitRegisterExpr(RegisterExpr<T> &e) -> RetTy;
	auto visitSelectExpr(SelectExpr<T> &e) -> RetTy;
	// RetTy visitConcatExpr(ConcatExpr<T> &e);
	// RetTy visitExtractExpr(ExtractExpr<T> &e);
	auto visitConjunctionExpr(ConjunctionExpr<T> &e) -> RetTy;
	auto visitDisjunctionExpr(DisjunctionExpr<T> &e) -> RetTy;
	auto visitZExtExpr(ZExtExpr<T> &e) -> RetTy;
	auto visitSExtExpr(SExtExpr<T> &e) -> RetTy;
	auto visitTruncExpr(TruncExpr<T> &e) -> RetTy;
	auto visitNotExpr(NotExpr<T> &e) -> RetTy;
	auto visitAddExpr(AddExpr<T> &e) -> RetTy;
	auto visitSubExpr(SubExpr<T> &e) -> RetTy;
	auto visitMulExpr(MulExpr<T> &e) -> RetTy;
	auto visitUDivExpr(UDivExpr<T> &e) -> RetTy;
	auto visitSDivExpr(SDivExpr<T> &e) -> RetTy;
	auto visitURemExpr(URemExpr<T> &e) -> RetTy;
	auto visitSRemExpr(SRemExpr<T> &e) -> RetTy;
	auto visitAndExpr(AndExpr<T> &e) -> RetTy;
	auto visitOrExpr(OrExpr<T> &e) -> RetTy;
	auto visitXorExpr(XorExpr<T> &e) -> RetTy;
	auto visitShlExpr(ShlExpr<T> &e) -> RetTy;
	auto visitLShrExpr(LShrExpr<T> &e) -> RetTy;
	auto visitAShrExpr(AShrExpr<T> &e) -> RetTy;
	auto visitEqExpr(EqExpr<T> &e) -> RetTy;
	auto visitNeExpr(NeExpr<T> &e) -> RetTy;
	auto visitUltExpr(UltExpr<T> &e) -> RetTy;
	auto visitUleExpr(UleExpr<T> &e) -> RetTy;
	auto visitUgtExpr(UgtExpr<T> &e) -> RetTy;
	auto visitUgeExpr(UgeExpr<T> &e) -> RetTy;
	auto visitSltExpr(SltExpr<T> &e) -> RetTy;
	auto visitSleExpr(SleExpr<T> &e) -> RetTy;
	auto visitSgtExpr(SgtExpr<T> &e) -> RetTy;
	auto visitSgeExpr(SgeExpr<T> &e) -> RetTy;

	auto visitSExpr(SExpr<T> & /*e*/) -> RetTy { UNREACHABLE(); }

private:
	auto getTransformer() const -> const std::function<ModuleVarID(T)> & { return transformer; }

	std::function<ModuleVarID(T)> transformer;
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
	for (auto i = 0U; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " /\\ ";
	}
}

template <typename T> void SExprPrinter<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	for (auto i = 0U; i < e.getNumKids(); i++) {
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
	for (auto i = 0U; i < e.getNumKids(); i++) {
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

template <typename T> auto SExprEvaluator<T>::visitConcreteExpr(ConcreteExpr<T> &e) -> RetTy
{

	return e.getValue();
}

template <typename T> auto SExprEvaluator<T>::visitRegisterExpr(RegisterExpr<T> &e) -> RetTy
{
	if (bruteForce)
		return getVal();

	if (!hasKnownMapping(e.getRegister()))
		unknown.insert(e.getRegister());
	return getMappingFor(e.getRegister());
}

template <typename T> auto SExprEvaluator<T>::visitSelectExpr(SelectExpr<T> &e) -> RetTy
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

template <typename T> auto SExprEvaluator<T>::visitConjunctionExpr(ConjunctionExpr<T> &e) -> RetTy
{
	IMPLEMENT_LOGOP(std::all_of);
}

template <typename T> auto SExprEvaluator<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e) -> RetTy
{
	IMPLEMENT_LOGOP(std::any_of);
}

/* No special care taken using SVals */
#define IMPLEMENT_CAST(op) return this->visit(e.getKid(0))

template <typename T> auto SExprEvaluator<T>::visitZExtExpr(ZExtExpr<T> &e) -> RetTy
{
	IMPLEMENT_CAST(zext);
}

template <typename T> auto SExprEvaluator<T>::visitSExtExpr(SExtExpr<T> &e) -> RetTy
{
	IMPLEMENT_CAST(sext);
}

template <typename T> auto SExprEvaluator<T>::visitTruncExpr(TruncExpr<T> &e) -> RetTy
{
	IMPLEMENT_CAST(trunc);
}

template <typename T> auto SExprEvaluator<T>::visitNotExpr(NotExpr<T> &e) -> RetTy
{
	return SVal(!e.getKid(0));
}

#define IMPLEMENT_BINOP(op) return this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))

#define IMPLEMENT_BINOP_NONMEM(op) return this->visit(e.getKid(0)) op this->visit(e.getKid(1))

template <typename T> auto SExprEvaluator<T>::visitAddExpr(AddExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(+);
}

template <typename T> auto SExprEvaluator<T>::visitSubExpr(SubExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(-);
}

template <typename T> auto SExprEvaluator<T>::visitMulExpr(MulExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(*);
}

template <typename T> auto SExprEvaluator<T>::visitUDivExpr(UDivExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T> auto SExprEvaluator<T>::visitSDivExpr(SDivExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T> auto SExprEvaluator<T>::visitURemExpr(URemExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T> auto SExprEvaluator<T>::visitSRemExpr(SRemExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template <typename T> auto SExprEvaluator<T>::visitAndExpr(AndExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(&);
}

template <typename T> auto SExprEvaluator<T>::visitOrExpr(OrExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(|);
}

template <typename T> auto SExprEvaluator<T>::visitXorExpr(XorExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(^);
}

template <typename T> auto SExprEvaluator<T>::visitShlExpr(ShlExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(<<);
}

template <typename T> auto SExprEvaluator<T>::visitLShrExpr(LShrExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

template <typename T> auto SExprEvaluator<T>::visitAShrExpr(AShrExpr<T> &e) -> RetTy
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

#define IMPLEMENT_EQOP(op)                                                                         \
	if (this->visit(e.getKid(0)) op(this->visit(e.getKid(1))))                                 \
		return SVal(1);                                                                    \
	return SVal(0);

template <typename T> auto SExprEvaluator<T>::visitEqExpr(EqExpr<T> &e) -> RetTy
{
	IMPLEMENT_EQOP(==);
}

template <typename T> auto SExprEvaluator<T>::visitNeExpr(NeExpr<T> &e) -> RetTy
{
	IMPLEMENT_EQOP(!=);
}

#define IMPLEMENT_CMPOP(op)                                                                        \
	return (this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))) ? SVal(1) : SVal(0);

template <typename T> auto SExprEvaluator<T>::visitUltExpr(UltExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(ult);
}

template <typename T> auto SExprEvaluator<T>::visitUleExpr(UleExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(ule);
}

template <typename T> auto SExprEvaluator<T>::visitUgtExpr(UgtExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(ugt);
}

template <typename T> auto SExprEvaluator<T>::visitUgeExpr(UgeExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(uge);
}

template <typename T> auto SExprEvaluator<T>::visitSltExpr(SltExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(slt);
}

template <typename T> auto SExprEvaluator<T>::visitSleExpr(SleExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(sle);
}

template <typename T> auto SExprEvaluator<T>::visitSgtExpr(SgtExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(sgt);
}

template <typename T> auto SExprEvaluator<T>::visitSgeExpr(SgeExpr<T> &e) -> RetTy
{
	IMPLEMENT_CMPOP(sge);
}

/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

template <typename T>
auto SExprTransformer<T>::visitConcreteExpr(ConcreteExpr<T> &e) ->
	typename SExprTransformer<T>::RetTy
{
	return ConcreteExpr<ModuleVarID>::create(e.getWidth(), e.getValue());
}

template <typename T>
auto SExprTransformer<T>::visitRegisterExpr(RegisterExpr<T> &e) ->
	typename SExprTransformer<T>::RetTy
{
	return RegisterExpr<ModuleVarID>::create(e.getWidth(), getTransformer()(e.getRegister()),
						 e.getName());
}

template <typename T>
auto SExprTransformer<T>::visitSelectExpr(SelectExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	return SelectExpr<ModuleVarID>::create(e.getWidth(), this->visit(e.getKid(0)),
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
	std::vector<std::unique_ptr<SExpr<ModuleVarID>>> kids;                                     \
	for (auto &kid : e.getKids())                                                              \
		kids.push_back(this->visit(*kid));                                                 \
	return name##Expr<ModuleVarID>::create(std::move(kids));

template <typename T>
auto SExprTransformer<T>::visitConjunctionExpr(ConjunctionExpr<T> &e) ->
	typename SExprTransformer<T>::RetTy
{
	TRANSFORM_LOGOP(Conjunction);
}

template <typename T>
auto SExprTransformer<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e) ->
	typename SExprTransformer<T>::RetTy
{
	TRANSFORM_LOGOP(Disjunction);
}

#define TRANSFORM_CAST(name)                                                                       \
	return name##Expr<ModuleVarID>::create(this->getWidth(), this->visit(e.getKid(0)));

template <typename T>
auto SExprTransformer<T>::visitZExtExpr(ZExtExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	IMPLEMENT_CAST(ZExt);
}

template <typename T>
auto SExprTransformer<T>::visitSExtExpr(SExtExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	IMPLEMENT_CAST(SExt);
}

template <typename T>
auto SExprTransformer<T>::visitTruncExpr(TruncExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	IMPLEMENT_CAST(Trunc);
}

template <typename T>
auto SExprTransformer<T>::visitNotExpr(NotExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	return NotExpr<ModuleVarID>::create(this->visit(e.getKid(0)));
}

#define TRANSFORM_BINOP(name)                                                                      \
	return name##Expr<ModuleVarID>::create(e.getWidth(), this->visit(e.getKid(0)),             \
					       this->visit(e.getKid(1)));

template <typename T>
auto SExprTransformer<T>::visitAddExpr(AddExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Add);
}

template <typename T>
auto SExprTransformer<T>::visitSubExpr(SubExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Sub);
}

template <typename T>
auto SExprTransformer<T>::visitMulExpr(MulExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Mul);
}

template <typename T>
auto SExprTransformer<T>::visitUDivExpr(UDivExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(UDiv);
}

template <typename T>
auto SExprTransformer<T>::visitSDivExpr(SDivExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(SDiv);
}

template <typename T>
auto SExprTransformer<T>::visitURemExpr(URemExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(URem);
}

template <typename T>
auto SExprTransformer<T>::visitSRemExpr(SRemExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(SRem);
}

template <typename T>
auto SExprTransformer<T>::visitAndExpr(AndExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(And);
}

template <typename T>
auto SExprTransformer<T>::visitOrExpr(OrExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Or);
}

template <typename T>
auto SExprTransformer<T>::visitXorExpr(XorExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Xor);
}

template <typename T>
auto SExprTransformer<T>::visitShlExpr(ShlExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(Shl);
}

template <typename T>
auto SExprTransformer<T>::visitLShrExpr(LShrExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(LShr);
}

template <typename T>
auto SExprTransformer<T>::visitAShrExpr(AShrExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_BINOP(AShr);
}

#define TRANSFORM_CMPOP(name)                                                                      \
	return name##Expr<ModuleVarID>::create(this->visit(e.getKid(0)), this->visit(e.getKid(1)));

template <typename T>
auto SExprTransformer<T>::visitEqExpr(EqExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Eq);
}

template <typename T>
auto SExprTransformer<T>::visitNeExpr(NeExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Ne);
}

template <typename T>
auto SExprTransformer<T>::visitUltExpr(UltExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Ult);
}

template <typename T>
auto SExprTransformer<T>::visitUleExpr(UleExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Ule);
}

template <typename T>
auto SExprTransformer<T>::visitUgtExpr(UgtExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Ugt);
}

template <typename T>
auto SExprTransformer<T>::visitUgeExpr(UgeExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Uge);
}

template <typename T>
auto SExprTransformer<T>::visitSltExpr(SltExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Slt);
}

template <typename T>
auto SExprTransformer<T>::visitSleExpr(SleExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Sle);
}

template <typename T>
auto SExprTransformer<T>::visitSgtExpr(SgtExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Sgt);
}

template <typename T>
auto SExprTransformer<T>::visitSgeExpr(SgeExpr<T> &e) -> typename SExprTransformer<T>::RetTy
{
	TRANSFORM_CMPOP(Sge);
}

#endif /* GENMC_S_EXPR_VISITOR_HPP */
