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

#include "SExprVisitor.hpp"
#include "Error.hpp"

/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

template<typename T>
void SExprPrinter<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{
	output += e.getValue().toString();
}

template<typename T>
void SExprPrinter<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	output += e.getName();
}

template<typename T>
void SExprPrinter<T>::visitSelectExpr(SelectExpr<T> &e)
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

template<typename T>
void SExprPrinter<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	for (auto i = 0u; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " /\\ ";
	}
}

template<typename T>
void SExprPrinter<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
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

template<typename T>
void SExprPrinter<T>::visitNotExpr(NotExpr<T> &e)
{
	output += "!";
	this->visit(e.getKid(0));
}

#define PRINT_BINARY_EXPR(op)			\
	this->visit(e.getKid(0));			\
	output += std::string(" ") + op + " ";	\
	this->visit(e.getKid(1));

template<typename T>
void SExprPrinter<T>::visitAddExpr(AddExpr<T> &e)
{
	PRINT_BINARY_EXPR("+");
}

template<typename T>
void SExprPrinter<T>::visitSubExpr(SubExpr<T> &e)
{
	PRINT_BINARY_EXPR("-");
}

template<typename T>
void SExprPrinter<T>::visitMulExpr(MulExpr<T> &e)
{
	PRINT_BINARY_EXPR("*");
}

template<typename T>
void SExprPrinter<T>::visitUDivExpr(UDivExpr<T> &e)
{
	PRINT_BINARY_EXPR("/u");
}

template<typename T>
void SExprPrinter<T>::visitSDivExpr(SDivExpr<T> &e)
{
	PRINT_BINARY_EXPR("/s");
}

template<typename T>
void SExprPrinter<T>::visitURemExpr(URemExpr<T> &e)
{
	PRINT_BINARY_EXPR("%u");
}

template<typename T>
void SExprPrinter<T>::visitSRemExpr(SRemExpr<T> &e)
{
	PRINT_BINARY_EXPR("%s");
}

template<typename T>
void SExprPrinter<T>::visitAndExpr(AndExpr<T> &e)
{
	PRINT_BINARY_EXPR("&");
}

template<typename T>
void SExprPrinter<T>::visitOrExpr(OrExpr<T> &e)
{
	PRINT_BINARY_EXPR("|");
}

template<typename T>
void SExprPrinter<T>::visitXorExpr(XorExpr<T> &e)
{
	PRINT_BINARY_EXPR("^");
}

template<typename T>
void SExprPrinter<T>::visitShlExpr(ShlExpr<T> &e)
{
	PRINT_BINARY_EXPR("<<");
}

template<typename T>
void SExprPrinter<T>::visitLShrExpr(LShrExpr<T> &e)
{
	PRINT_BINARY_EXPR("<<a");
}

template<typename T>
void SExprPrinter<T>::visitAShrExpr(AShrExpr<T> &e)
{
	PRINT_BINARY_EXPR(">>a");
}

template<typename T>
void SExprPrinter<T>::visitEqExpr(EqExpr<T> &e)
{
	PRINT_BINARY_EXPR("==");
}

template<typename T>
void SExprPrinter<T>::visitNeExpr(NeExpr<T> &e)
{
	PRINT_BINARY_EXPR("!=");
}

template<typename T>
void SExprPrinter<T>::visitUltExpr(UltExpr<T> &e)
{
	PRINT_BINARY_EXPR("<u");
}

template<typename T>
void SExprPrinter<T>::visitUleExpr(UleExpr<T> &e)
{
	PRINT_BINARY_EXPR("<=u");
}

template<typename T>
void SExprPrinter<T>::visitUgtExpr(UgtExpr<T> &e)
{
	PRINT_BINARY_EXPR(">u");
}

template<typename T>
void SExprPrinter<T>::visitUgeExpr(UgeExpr<T> &e)
{
	PRINT_BINARY_EXPR(">=u");
}

template<typename T>
void SExprPrinter<T>::visitSltExpr(SltExpr<T> &e)
{
	PRINT_BINARY_EXPR("<s");
}

template<typename T>
void SExprPrinter<T>::visitSleExpr(SleExpr<T> &e)
{
	PRINT_BINARY_EXPR("<=s");
}

template<typename T>
void SExprPrinter<T>::visitSgtExpr(SgtExpr<T> &e)
{
	PRINT_BINARY_EXPR(">s");
}

template<typename T>
void SExprPrinter<T>::visitSgeExpr(SgeExpr<T> &e)
{
	PRINT_BINARY_EXPR(">=s");
}

template<typename T>
void SExprPrinter<T>::visitLogicalExpr(LogicalExpr<T> &e)
{
	output += "LogOp(";
	for (auto i = 0u; i < e.getNumKids(); i++) {
		this->visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += ", ";
	}
	output += ")";
}

template<typename T>
void SExprPrinter<T>::visitCastExpr(CastExpr<T> &e)
{
	output += "cast" + std::to_string(e.getWidth()) + "(";
	this->visit(e.getKid(0));
	output += ")";
}

template<typename T>
void SExprPrinter<T>::visitBinaryExpr(BinaryExpr<T> &e)
{
	output += "BinOp(";
	this->visit(e.getKid(0));
	output += ", ";
	this->visit(e.getKid(1));
	output += ")";
}

template<typename T>
void SExprPrinter<T>::visitCmpExpr(CmpExpr<T> &e)
{
	output += "CmpOp(";
	this->visit(e.getKid(0));
	output += ", ";
	this->visit(e.getKid(1));
	output += ")";
}

template<typename U>
llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const SExpr<U> &annot)
{
	rhs << SExprPrinter<U>().toString(const_cast<SExpr<U>&>(annot));
	return rhs;
}


/*******************************************************************************
 **                           SExprEvaluator Class
 ******************************************************************************/

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{

	return e.getValue();
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	if (bruteForce)
		return getVal();

	if (!hasKnownMapping(e.getRegister()))
		unknown.insert(e.getRegister());
	return getMappingFor(e.getRegister());
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSelectExpr(SelectExpr<T> &e)
{
	return this->visit(e.getKid(0)).getBool() ? this->visit(e.getKid(1)) : this->visit(e.getKid(2));
}

// template<typename T>
// typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConcat(ConcatExpr<T> &e)
// {
// }

// template<typename T>
// typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitExtract(ExtractExpr<T> &e)
// {
// }

#define IMPLEMENT_LOGOP(op)						\
	if (op(e.getKids().begin(), e.getKids().end(),			\
	       [&](const std::unique_ptr<SExpr<T>> &kid){ return this->visit(kid).getBool(); })) \
		return SVal(1);						\
	return SVal(0);

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	IMPLEMENT_LOGOP(std::all_of);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	IMPLEMENT_LOGOP(std::any_of);
}

/* No special care taken using SVals */
#define IMPLEMENT_CAST(op)			\
	return this->visit(e.getKid(0))

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitZExtExpr(ZExtExpr<T> &e)
{
	IMPLEMENT_CAST(zext);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSExtExpr(SExtExpr<T> &e)
{
	IMPLEMENT_CAST(sext);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitTruncExpr(TruncExpr<T> &e)
{
	IMPLEMENT_CAST(trunc);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitNotExpr(NotExpr<T> &e)
{
	return SVal(!e.getKid(0));
}

#define IMPLEMENT_BINOP(op)					\
	return this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))

#define IMPLEMENT_BINOP_NONMEM(op)				\
	return this->visit(e.getKid(0)) op this->visit(e.getKid(1))

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAddExpr(AddExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(+);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSubExpr(SubExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(-);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitMulExpr(MulExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(*);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUDivExpr(UDivExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSDivExpr(SDivExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitURemExpr(URemExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSRemExpr(SRemExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(/);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAndExpr(AndExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(&);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitOrExpr(OrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(|);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitXorExpr(XorExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(^);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitShlExpr(ShlExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(<<);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitLShrExpr(LShrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitAShrExpr(AShrExpr<T> &e)
{
	IMPLEMENT_BINOP_NONMEM(>>);
}

#define IMPLEMENT_EQOP(op)				\
	if (this->visit(e.getKid(0)) op (this->visit(e.getKid(1))))	\
		return SVal(1);				\
	return SVal(0);

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitEqExpr(EqExpr<T> &e)
{
	IMPLEMENT_EQOP(==);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitNeExpr(NeExpr<T> &e)
{
	IMPLEMENT_EQOP(!=);
}

#define IMPLEMENT_CMPOP(op)						\
	return (this->visit(e.getKid(0)).op(this->visit(e.getKid(1)))) ? \
	SVal(1) : SVal(0);

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUltExpr(UltExpr<T> &e)
{
	IMPLEMENT_CMPOP(ult);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUleExpr(UleExpr<T> &e)
{
	IMPLEMENT_CMPOP(ule);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUgtExpr(UgtExpr<T> &e)
{
	IMPLEMENT_CMPOP(ugt);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitUgeExpr(UgeExpr<T> &e)
{
	IMPLEMENT_CMPOP(uge);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSltExpr(SltExpr<T> &e)
{
	IMPLEMENT_CMPOP(slt);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSleExpr(SleExpr<T> &e)
{
	IMPLEMENT_CMPOP(sle);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSgtExpr(SgtExpr<T> &e)
{
	IMPLEMENT_CMPOP(sgt);
}

template<typename T>
typename SExprEvaluator<T>::RetTy SExprEvaluator<T>::visitSgeExpr(SgeExpr<T> &e)
{
	IMPLEMENT_CMPOP(sge);
}


/*******************************************************************************
 **                           SExprTransformer Class
 ******************************************************************************/

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitConcreteExpr(ConcreteExpr<T> &e)
{
	return ConcreteExpr<ModuleID::ID>::create(e.getWidth(), e.getValue());
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitRegisterExpr(RegisterExpr<T> &e)
{
	return RegisterExpr<ModuleID::ID>::create(
		e.getWidth(), getTransformer()(e.getRegister()), e.getName());
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSelectExpr(SelectExpr<T> &e)
{
	return SelectExpr<ModuleID::ID>::create(
		e.getWidth(), this->visit(e.getKid(0)), this->visit(e.getKid(1)), this->visit(e.getKid(2)));
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

#define TRANSFORM_LOGOP(name)						\
	std::vector<std::unique_ptr<SExpr<ModuleID::ID>>> kids;		\
	for (auto &k : e.getKids())					\
		kids.push_back(this->visit(*k));			\
	return name##Expr<ModuleID::ID>::create(std::move(kids));

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitConjunctionExpr(ConjunctionExpr<T> &e)
{
	TRANSFORM_LOGOP(Conjunction);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitDisjunctionExpr(DisjunctionExpr<T> &e)
{
	TRANSFORM_LOGOP(Disjunction);
}

#define TRANSFORM_CAST(name)			\
	return name##Expr<ModuleID::ID>::create(this->getWidth(), this->visit(e.getKid(0)));

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitZExtExpr(ZExtExpr<T> &e)
{
	IMPLEMENT_CAST(ZExt);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSExtExpr(SExtExpr<T> &e)
{
	IMPLEMENT_CAST(SExt);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitTruncExpr(TruncExpr<T> &e)
{
	IMPLEMENT_CAST(Trunc);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitNotExpr(NotExpr<T> &e)
{
	return NotExpr<ModuleID::ID>::create(this->visit(e.getKid(0)));
}

#define TRANSFORM_BINOP(name)						\
	return name##Expr<ModuleID::ID>::create(e.getWidth(), this->visit(e.getKid(0)), this->visit(e.getKid(1)));

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitAddExpr(AddExpr<T> &e)
{
	TRANSFORM_BINOP(Add);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSubExpr(SubExpr<T> &e)
{
	TRANSFORM_BINOP(Sub);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitMulExpr(MulExpr<T> &e)
{
	TRANSFORM_BINOP(Mul);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitUDivExpr(UDivExpr<T> &e)
{
	TRANSFORM_BINOP(UDiv);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSDivExpr(SDivExpr<T> &e)
{
	TRANSFORM_BINOP(SDiv);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitURemExpr(URemExpr<T> &e)
{
	TRANSFORM_BINOP(URem);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSRemExpr(SRemExpr<T> &e)
{
	TRANSFORM_BINOP(SRem);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitAndExpr(AndExpr<T> &e)
{
	TRANSFORM_BINOP(And);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitOrExpr(OrExpr<T> &e)
{
	TRANSFORM_BINOP(Or);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitXorExpr(XorExpr<T> &e)
{
	TRANSFORM_BINOP(Xor);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitShlExpr(ShlExpr<T> &e)
{
	TRANSFORM_BINOP(Shl);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitLShrExpr(LShrExpr<T> &e)
{
	TRANSFORM_BINOP(LShr);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitAShrExpr(AShrExpr<T> &e)
{
	TRANSFORM_BINOP(AShr);
}

#define TRANSFORM_CMPOP(name)						\
	return name##Expr<ModuleID::ID>::create(this->visit(e.getKid(0)), this->visit(e.getKid(1)));

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitEqExpr(EqExpr<T> &e)
{
	TRANSFORM_CMPOP(Eq);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitNeExpr(NeExpr<T> &e)
{
	TRANSFORM_CMPOP(Ne);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitUltExpr(UltExpr<T> &e)
{
	TRANSFORM_CMPOP(Ult);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitUleExpr(UleExpr<T> &e)
{
	TRANSFORM_CMPOP(Ule);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitUgtExpr(UgtExpr<T> &e)
{
	TRANSFORM_CMPOP(Ugt);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitUgeExpr(UgeExpr<T> &e)
{
	TRANSFORM_CMPOP(Uge);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSltExpr(SltExpr<T> &e)
{
	TRANSFORM_CMPOP(Slt);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSleExpr(SleExpr<T> &e)
{
	TRANSFORM_CMPOP(Sle);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSgtExpr(SgtExpr<T> &e)
{
	TRANSFORM_CMPOP(Sgt);
}

template<typename T>
typename SExprTransformer<T>::RetTy
SExprTransformer<T>::visitSgeExpr(SgeExpr<T> &e)
{
	TRANSFORM_CMPOP(Sge);
}
