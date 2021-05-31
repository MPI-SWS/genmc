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

/*******************************************************************************
 **                           SExprPrinter Class
 ******************************************************************************/

void SExprPrinter::visitConcreteExpr(ConcreteExpr &e)
{
	output += e.getValue().toString(10, false);
}

void SExprPrinter::visitRegisterExpr(RegisterExpr &e)
{
	output += e.getName();
}

void SExprPrinter::visitSelectExpr(SelectExpr &e)
{
	output += "(";
	visit(e.getKid(0));
	output += " ? ";
	visit(e.getKid(1));
	output += " : ";
	visit(e.getKid(2));
	output += ")";
}

// void SExprPrinter::visitConcat(ConcatExpr &e)
// {
// }

// void SExprPrinter::visitExtract(ExtractExpr &e)
// {
// }

void SExprPrinter::visitConjunctionExpr(ConjunctionExpr &e)
{
	for (auto i = 0u; i < e.getNumKids(); i++) {
		visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " /\\ ";
	}
}

void SExprPrinter::visitDisjunctionExpr(DisjunctionExpr &e)
{
	for (auto i = 0u; i < e.getNumKids(); i++) {
		visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += " \\/ ";
	}
}

// void SExprPrinter::visitZExtExpr(ZExtExpr &e)
// {
// }

// void SExprPrinter::visitSExtExpr(SExtExpr &e)
// {
// }

// void SExprPrinter::visitTruncExpr(TruncExpr &e)
// {
// }

void SExprPrinter::visitNotExpr(NotExpr &e)
{
	output += "!";
	visit(e.getKid(0));
}

#define PRINT_BINARY_EXPR(op)			\
	visit(e.getKid(0));			\
	output += std::string(" ") + op + " ";	\
	visit(e.getKid(1));

void SExprPrinter::visitAddExpr(AddExpr &e)
{
	PRINT_BINARY_EXPR("+");
}

void SExprPrinter::visitSubExpr(SubExpr &e)
{
	PRINT_BINARY_EXPR("-");
}

void SExprPrinter::visitMulExpr(MulExpr &e)
{
	PRINT_BINARY_EXPR("*");
}

void SExprPrinter::visitUDivExpr(UDivExpr &e)
{
	PRINT_BINARY_EXPR("/u");
}

void SExprPrinter::visitSDivExpr(SDivExpr &e)
{
	PRINT_BINARY_EXPR("/s");
}

void SExprPrinter::visitURemExpr(URemExpr &e)
{
	PRINT_BINARY_EXPR("%u");
}

void SExprPrinter::visitSRemExpr(SRemExpr &e)
{
	PRINT_BINARY_EXPR("%s");
}

void SExprPrinter::visitAndExpr(AndExpr &e)
{
	PRINT_BINARY_EXPR("&");
}

void SExprPrinter::visitOrExpr(OrExpr &e)
{
	PRINT_BINARY_EXPR("|");
}

void SExprPrinter::visitXorExpr(XorExpr &e)
{
	PRINT_BINARY_EXPR("^");
}

void SExprPrinter::visitShlExpr(ShlExpr &e)
{
	PRINT_BINARY_EXPR("<<");
}

void SExprPrinter::visitLShrExpr(LShrExpr &e)
{
	PRINT_BINARY_EXPR("<<a");
}

void SExprPrinter::visitAShrExpr(AShrExpr &e)
{
	PRINT_BINARY_EXPR(">>a");
}

void SExprPrinter::visitEqExpr(EqExpr &e)
{
	PRINT_BINARY_EXPR("==");
}

void SExprPrinter::visitNeExpr(NeExpr &e)
{
	PRINT_BINARY_EXPR("!=");
}

void SExprPrinter::visitUltExpr(UltExpr &e)
{
	PRINT_BINARY_EXPR("<u");
}

void SExprPrinter::visitUleExpr(UleExpr &e)
{
	PRINT_BINARY_EXPR("<=u");
}

void SExprPrinter::visitUgtExpr(UgtExpr &e)
{
	PRINT_BINARY_EXPR(">u");
}

void SExprPrinter::visitUgeExpr(UgeExpr &e)
{
	PRINT_BINARY_EXPR(">=u");
}

void SExprPrinter::visitSltExpr(SltExpr &e)
{
	PRINT_BINARY_EXPR("<s");
}

void SExprPrinter::visitSleExpr(SleExpr &e)
{
	PRINT_BINARY_EXPR("<=s");
}

void SExprPrinter::visitSgtExpr(SgtExpr &e)
{
	PRINT_BINARY_EXPR(">s");
}

void SExprPrinter::visitSgeExpr(SgeExpr &e)
{
	PRINT_BINARY_EXPR(">=s");
}

void SExprPrinter::visitLogicalExpr(LogicalExpr &e)
{
	output += "LogOp(";
	for (auto i = 0u; i < e.getNumKids(); i++) {
		visit(e.getKid(i));
		if (i != e.getNumKids() - 1)
			output += ", ";
	}
	output += ")";
}

void SExprPrinter::visitCastExpr(CastExpr &e)
{
	output += "cast" + std::to_string(e.getWidth()) + "(";
	visit(e.getKid(0));
	output += ")";
}

void SExprPrinter::visitBinaryExpr(BinaryExpr &e)
{
	output += "BinOp(";
	visit(e.getKid(0));
	output += ", ";
	visit(e.getKid(1));
	output += ")";
}

void SExprPrinter::visitCmpExpr(CmpExpr &e)
{
	output += "CmpOp(";
	visit(e.getKid(0));
	output += ", ";
	visit(e.getKid(1));
	output += ")";
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const SExpr &annot)
{
	rhs << SExprPrinter().toString(const_cast<SExpr&>(annot));
	return rhs;
}


/*******************************************************************************
 **                           SExprEvaluator Class
 ******************************************************************************/

llvm::APInt SExprEvaluator::visitConcreteExpr(ConcreteExpr &e)
{

	return e.getValue();
}

llvm::APInt SExprEvaluator::visitRegisterExpr(RegisterExpr &e)
{
	if (bruteForce)
		return getVal();

	if (!hasKnownMapping(e.getRegister()))
		unknown.insert(e.getRegister());
	return getMappingFor(e.getRegister());
}

llvm::APInt SExprEvaluator::visitSelectExpr(SelectExpr &e)
{
	return visit(e.getKid(0)).getBoolValue() ? visit(e.getKid(1)) : visit(e.getKid(2));
}

// llvm::APInt SExprEvaluator::visitConcat(ConcatExpr &e)
// {
// }

// llvm::APInt SExprEvaluator::visitExtract(ExtractExpr &e)
// {
// }

#define IMPLEMENT_LOGOP(op)						\
	if (op(e.getKids().begin(), e.getKids().end(),			\
	       [&](const std::unique_ptr<SExpr> &kid){ return visit(kid).getBoolValue(); })) \
		return llvm::APInt(e.getWidth(), 1);			\
	return llvm::APInt(e.getWidth(), 0)

llvm::APInt SExprEvaluator::visitConjunctionExpr(ConjunctionExpr &e)
{
	IMPLEMENT_LOGOP(std::all_of);
}

llvm::APInt SExprEvaluator::visitDisjunctionExpr(DisjunctionExpr &e)
{
	IMPLEMENT_LOGOP(std::any_of);
}

#define IMPLEMENT_CAST(op)				\
	return visit(e.getKid(0)).op(e.getWidth())

llvm::APInt SExprEvaluator::visitZExtExpr(ZExtExpr &e)
{
	IMPLEMENT_CAST(zext);
}

llvm::APInt SExprEvaluator::visitSExtExpr(SExtExpr &e)
{
	IMPLEMENT_CAST(sext);
}

llvm::APInt SExprEvaluator::visitTruncExpr(TruncExpr &e)
{
	IMPLEMENT_CAST(trunc);
}

llvm::APInt SExprEvaluator::visitNotExpr(NotExpr &e)
{
	return llvm::APInt(1, !visit(e.getKid(0)));
}

#define IMPLEMENT_BINOP(op)					\
	return visit(e.getKid(0)).op(visit(e.getKid(1)))

#define IMPLEMENT_BINOP_NONMEM(op)				\
	return visit(e.getKid(0)) op visit(e.getKid(1))

llvm::APInt SExprEvaluator::visitAddExpr(AddExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(+);
}

llvm::APInt SExprEvaluator::visitSubExpr(SubExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(-);
}

llvm::APInt SExprEvaluator::visitMulExpr(MulExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(*);
}

llvm::APInt SExprEvaluator::visitUDivExpr(UDivExpr &e)
{
	IMPLEMENT_BINOP(udiv);
}

llvm::APInt SExprEvaluator::visitSDivExpr(SDivExpr &e)
{
	IMPLEMENT_BINOP(sdiv);
}

llvm::APInt SExprEvaluator::visitURemExpr(URemExpr &e)
{
	IMPLEMENT_BINOP(urem);
}

llvm::APInt SExprEvaluator::visitSRemExpr(SRemExpr &e)
{
	IMPLEMENT_BINOP(srem);
}

llvm::APInt SExprEvaluator::visitAndExpr(AndExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(&);
}

llvm::APInt SExprEvaluator::visitOrExpr(OrExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(|);
}

llvm::APInt SExprEvaluator::visitXorExpr(XorExpr &e)
{
	IMPLEMENT_BINOP_NONMEM(^);
}

llvm::APInt SExprEvaluator::visitShlExpr(ShlExpr &e)
{
	IMPLEMENT_BINOP(shl);
}

llvm::APInt SExprEvaluator::visitLShrExpr(LShrExpr &e)
{
	IMPLEMENT_BINOP(lshr);
}

llvm::APInt SExprEvaluator::visitAShrExpr(AShrExpr &e)
{
	IMPLEMENT_BINOP(ashr);
}

#define IMPLEMENT_CMPOP(op)				\
	if (visit(e.getKid(0)).op(visit(e.getKid(1))))	\
		return llvm::APInt(e.getWidth(), 1);	\
	return llvm::APInt(e.getWidth(), 0);

llvm::APInt SExprEvaluator::visitEqExpr(EqExpr &e)
{
	IMPLEMENT_CMPOP(eq);
}

llvm::APInt SExprEvaluator::visitNeExpr(NeExpr &e)
{
	IMPLEMENT_CMPOP(ne);
}

llvm::APInt SExprEvaluator::visitUltExpr(UltExpr &e)
{
	IMPLEMENT_CMPOP(ult);
}

llvm::APInt SExprEvaluator::visitUleExpr(UleExpr &e)
{
	IMPLEMENT_CMPOP(ule);
}

llvm::APInt SExprEvaluator::visitUgtExpr(UgtExpr &e)
{
	IMPLEMENT_CMPOP(ugt);
}

llvm::APInt SExprEvaluator::visitUgeExpr(UgeExpr &e)
{
	IMPLEMENT_CMPOP(uge);
}

llvm::APInt SExprEvaluator::visitSltExpr(SltExpr &e)
{
	IMPLEMENT_CMPOP(slt);
}

llvm::APInt SExprEvaluator::visitSleExpr(SleExpr &e)
{
	IMPLEMENT_CMPOP(sle);
}

llvm::APInt SExprEvaluator::visitSgtExpr(SgtExpr &e)
{
	IMPLEMENT_CMPOP(sgt);
}

llvm::APInt SExprEvaluator::visitSgeExpr(SgeExpr &e)
{
	IMPLEMENT_CMPOP(sge);
}
