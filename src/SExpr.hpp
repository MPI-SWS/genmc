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

#ifndef __S_EXPR_HPP__
#define __S_EXPR_HPP__

#include "Error.hpp"

#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

#include <memory>

/*
 * Note: The infrastructure below uses unique_ptr<>s. Another alternative
 * would be to use shared_ptr<>s, which would enable sharing of expression
 * nodes. That, however, would complicate cloning a bit, which does occur
 * frequently in the current setting. Given that the expressions constructed
 * are probably going to be small,
 */

/*
 * The hierarchy below is largely inspired from the KLEE one:
 *      https://github.com/klee/klee/
 */

/*******************************************************************************
 **                           SExpr Class (Abstract)
 ******************************************************************************/

/*
 * An abstract class that models expressions containing symbolic variables.
 * Contains some things all subclasses provide (e.g., the kids for this node).
 * Currently supports expressions with integers only.
 */
class SExpr {

public:
	using Width = unsigned;
	static const Width BoolWidth = 1;

	enum Kind {
		InvalidKind = -1,

		/* Primitive */
		Concrete = 0,
		Register,

		/* Various */
		Select,
		Concat,
		Extract,

		/* Logical */
		LogicalKindFirst,
		Conjunction,
		Disjunction,
		Not,
		LogicalKindLast,

		/* Casting */
		CastKindFirst,
		ZExt,
		SExt,
		Trunc,
		CastKindLast,

		/* All subsequent kinds are binary */
		BinaryKindFirst,

		/* Arithmetic */
		Add,
		Sub,
		Mul,
		UDiv,
		SDiv,
		URem,
		SRem,

		/* Bit */
		And,
		Or,
		Xor,
		Shl,
		LShr,
		AShr,

		/* Compare */
		CmpKindFirst,
		Eq,
		Ne,
		Ult,
		Ule,
		Ugt,
		Uge,
		Slt,
		Sle,
		Sgt,
		Sge,
		CmpKindLast,

		BinaryKindLast,

		LastKind = Sge,
	};

protected:
	SExpr() = delete;

	SExpr(Kind k, Width w, std::vector<std::unique_ptr<SExpr> > &&kids = {})
		: kind(k), width(w), kids(std::move(kids)) {}
	SExpr(Kind k, const llvm::Type *t, std::vector<std::unique_ptr<SExpr> > &&kids = {})
		: SExpr(k, t->getIntegerBitWidth(), std::move(kids)) {}

public:

	virtual ~SExpr() {}

	/* The kind of this node (LLVM-style RTTI) */
	Kind getKind() const { return kind; }

	/* The width of this integer (in bits) */
	Width getWidth() const { return width; }

	/* The kids of this node */
	size_t getNumKids() const { return kids.size(); }

	/* Fetches the i-th kid */
	const SExpr *getKid(unsigned i) const {
		BUG_ON(i >= kids.size() && "Index out of bounds!");
		return kids[i].get();
	}
	SExpr *getKid(unsigned i) {
		BUG_ON(i >= kids.size() && "Index out of bounds!");
		return kids[i].get();
	}

	/* Sets the i-th kid to e */
	void setKid(unsigned i, std::unique_ptr<SExpr> &&e) {
		BUG_ON(i >= kids.size() && "Index out of bounds!");
		kids[i] = std::move(e);
	}

	virtual std::unique_ptr<SExpr> clone() const = 0;

	static bool classof(const SExpr *) { return true; }

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
					     const SExpr &annot); /* as a visitor */

protected:
	friend class SExprEvaluator;

	/* Returns a container with this node's kids */
	const std::vector<std::unique_ptr<SExpr> > &getKids() const { return kids; }

	/* This function is necessary because we cannot move from initializer lists,
	 * and also cannot copy unique_ptr<>s. We do need a way to construct a vector
	 * of unique_ptr<>s from its elements though... */
	void addKid(std::unique_ptr<SExpr> &&k) { kids.push_back(std::move(k)); }

private:
	Kind kind;
	Width width;
	std::vector<std::unique_ptr<SExpr> > kids;
};


/*******************************************************************************
 **                           ConcreteExpr Class
 ******************************************************************************/

/*
 * Represents a constant. For the time being, only integer constants are supported.
 */

class ConcreteExpr : public SExpr {

protected:
	ConcreteExpr(Width w, llvm::APInt val) : SExpr(Concrete, w), value(val) {}
	ConcreteExpr(llvm::APInt val) : ConcreteExpr(val.getBitWidth(), val) {}

public:
	/* Returns the constant value */
	const llvm::APInt &getValue() const { return value; }

	template<typename... Ts>
	static std::unique_ptr<ConcreteExpr> create(Ts&&... params) {
		return std::unique_ptr<ConcreteExpr>(
			new ConcreteExpr(std::forward<Ts>(params)...));
	}
	static std::unique_ptr<ConcreteExpr> createTrue() {
		return std::unique_ptr<ConcreteExpr>(new ConcreteExpr(llvm::APInt(1, 1)));
	}
	static std::unique_ptr<ConcreteExpr> createFalse() {
		return std::unique_ptr<ConcreteExpr>(new ConcreteExpr(llvm::APInt(1, 0)));
	}

	std::unique_ptr<SExpr> clone() const override {
		return create(getWidth(), getValue());
	}

	static bool classof(const SExpr *E) { return E->getKind() == SExpr::Concrete; }

private:
	llvm::APInt value;
};


/*******************************************************************************
 **                            RegisterExpr Class
 ******************************************************************************/

/*
 * Represents a register the value of which is still unknown (symbolic variable).
 */

class RegisterExpr : public SExpr {

protected:
	explicit RegisterExpr(Width width, llvm::Value *reg, const std::string &argname = "")
		: SExpr(Register, width), reg(reg),
		  name(!argname.empty() ? argname : ("#s" + std::to_string(regCount++))) {}

	explicit RegisterExpr(llvm::Value *reg, const std::string &argname = "")
		: RegisterExpr(reg->getType()->getIntegerBitWidth(), reg, argname) {}

public:
	/* Returns an identifier to this register */
	llvm::Value *getRegister() const { return reg; }

	/* Returns the name of this register (in LLVM-IR) */
	const std::string &getName() const { return name; }

	template<typename... Ts>
	static std::unique_ptr<RegisterExpr> create(Ts&&... params) {
		return std::unique_ptr<RegisterExpr>(
			new RegisterExpr(std::forward<Ts>(params)...));
	}

	std::unique_ptr<SExpr> clone() const override {
		return std::unique_ptr<SExpr>(new RegisterExpr(getWidth(), getRegister(), getName()));
	}

	static bool classof(const SExpr *E) {
		return E->getKind() == SExpr::Register;
	}

private:
	/* Unique identifier for the symbolic variable */
	llvm::Value *reg;

	/* The name of this symbolic variable */
	const std::string name;

	/* Counter used when creating names for symbolic vars */
	static unsigned regCount;
};


/*******************************************************************************
 **                            SelectExpr Class
 ******************************************************************************/

/*
 * Represents a select instruction. Equivalently, this can be thought of as
 * an 'if-then-else' statement, where if the condition (0th kid) holds
 * the expression takes the value of the first child, and if it does not
 * hold the expression takes the value of the second child (2nd kid).
 */

class SelectExpr : public SExpr {

protected:
	SelectExpr(Width w,
		   std::unique_ptr<SExpr> &&c,
		   std::unique_ptr<SExpr> &&t,
		   std::unique_ptr<SExpr> &&f)
		: SExpr(Select, w) { addKid(std::move(c)); addKid(std::move(t)); addKid(std::move(f)); }

	SelectExpr(std::unique_ptr<SExpr> &&c,
		   std::unique_ptr<SExpr> &&t,
		   std::unique_ptr<SExpr> &&f)
		: SelectExpr(t->getWidth(), std::move(c), std::move(t), std::move(f)) {}

public:
	template<typename... Ts>
	static std::unique_ptr<SelectExpr> create(Ts&&... params) {
		return std::unique_ptr<SelectExpr>(
			new SelectExpr(std::forward<Ts>(params)...));
	}

	std::unique_ptr<SExpr> clone() const override {
		auto cCond = getKid(0)->clone();
		auto cTrue = getKid(1)->clone();
		auto cFalse = getKid(2)->clone();
		return create(getWidth(), std::move(cCond), std::move(cTrue), std::move(cFalse));
	}

	static bool classof(const SExpr *E) {
		return E->getKind() == SExpr::Select;
	}
};


/*******************************************************************************
 **                        LogicalExpr Class (Abstract)
 ******************************************************************************/

/*
 * Represents a logical operation (e.g., AND, OR). These do not correspond to LLVM-IR
 * instructions, but may be useful if we ever decide to construct such expressions.
 * They always have a width of 1, irrespective of the widths of their arguments.
 */

class LogicalExpr : public SExpr {

protected:
	LogicalExpr(Kind k, Width w, std::vector<std::unique_ptr<SExpr> > &&es)
		: SExpr(k, w, std::move(es)) { BUG_ON(getKids().empty()); }
	LogicalExpr(Kind k, std::vector<std::unique_ptr<SExpr> > &&es)
		: LogicalExpr(k, BoolWidth, std::move(es)) {}
	/* For convenience */
	LogicalExpr(Kind k, std::unique_ptr<SExpr> &&e)
		: SExpr(k, BoolWidth) { addKid(std::move(e)); }
	LogicalExpr(Kind k, std::unique_ptr<SExpr> &&e1, std::unique_ptr<SExpr> &&e2)
		: SExpr(k, BoolWidth) { addKid(std::move(e1)); addKid(std::move(e2)); }

public:

	static bool classof(const SExpr *E) {
		SExpr::Kind k = E->getKind();
		return SExpr::LogicalKindFirst <= k && k <= SExpr::LogicalKindLast;
	}
};

#define LOGICAL_EXPR_CLASS(_class_kind)                                                                   \
class _class_kind ## Expr : public LogicalExpr {                                                          \
									                                  \
protected:                                                                                                \
        _class_kind ## Expr(std::vector<std::unique_ptr<SExpr> > &&es)                      	  	  \
	: LogicalExpr(_class_kind, std::move(es)) {}							  \
        _class_kind ## Expr(std::unique_ptr<SExpr> &&e)                                     	  	  \
	: LogicalExpr(_class_kind, std::move(e)) {}							  \
        _class_kind ## Expr(std::unique_ptr<SExpr> &&e1, std::unique_ptr<SExpr> &&e2) 			  \
	: LogicalExpr(_class_kind, std::move(e1), std::move(e2)) {}					  \
													  \
public:													  \
	template<typename... Ts>									  \
	static std::unique_ptr<_class_kind##Expr> create(Ts&&... params) {				  \
		return std::unique_ptr<_class_kind##Expr>(						  \
			new _class_kind##Expr(std::forward<Ts>(params)...));				  \
	}				 								  \
								                                          \
	std::unique_ptr<SExpr> clone() const override {                              		  	  \
		std::vector<std::unique_ptr<SExpr> > kidsCopy; 				  		  \
		std::for_each(getKids().begin(), getKids().end(),		                          \
			      [&](const std::unique_ptr<SExpr> &s){ kidsCopy.push_back(s->clone()); });   \
		return create(std::move(kidsCopy));							  \
	}                                                                                                 \
								                                          \
	static bool classof(const SExpr *E) {		                                          	  \
		return E->getKind() == SExpr::_class_kind;                                       	  \
	}							                                          \
								                                          \
};								                                          \

LOGICAL_EXPR_CLASS(Conjunction)

LOGICAL_EXPR_CLASS(Disjunction)

class NotExpr : public LogicalExpr {

protected:
	NotExpr(std::unique_ptr<SExpr> &&e)
		: LogicalExpr(Not, std::move(e)) {}

public:
	template<typename... Ts>
	static std::unique_ptr<NotExpr> create(Ts&&... params) {
		return std::unique_ptr<NotExpr>(new NotExpr(std::forward<Ts>(params)...));
	}

	std::unique_ptr<SExpr> clone() const override {
		return create(getKid(0)->clone());
	}

	static bool classof(const SExpr *E) {
		return E->getKind() == SExpr::Not;
	}
};


/*******************************************************************************
 **                           CastExpr Class (Abstract)
 ******************************************************************************/

/*
 * Represents a cast instruction (e.g., zext, trunc).
 */

class CastExpr : public SExpr {

protected:
	CastExpr(Kind k, Width w, std::unique_ptr<SExpr> &&e)
		: SExpr(k, w) { addKid(std::move(e)); }

public:
	static bool classof(const SExpr *E) {
		SExpr::Kind k = E->getKind();
		return SExpr::CastKindFirst <= k && k <= SExpr::CastKindLast;
	}
};

#define CAST_EXPR_CLASS(_class_kind)                                                                      \
class _class_kind ## Expr : public CastExpr {                                                             \
													  \
protected:                                                                                                \
	_class_kind ## Expr(Width w, std::unique_ptr<SExpr> &&e)				  	  \
	: CastExpr(_class_kind, w, std::move(e)) {}							  \
													  \
public:													  \
	template<typename... Ts>									  \
	static std::unique_ptr<_class_kind##Expr> create(Ts&&... params) {				  \
		return std::unique_ptr<_class_kind##Expr>(						  \
			new _class_kind##Expr(std::forward<Ts>(params)...));				  \
	}				 								  \
								                                          \
	std::unique_ptr<SExpr> clone() const override {		                          		  \
		return create(getWidth(), getKid(0)->clone()); 						  \
	}                                                                                                 \
								                                          \
	static bool classof(const SExpr *E) {		                                          	  \
		return E->getKind() == SExpr::_class_kind;                                       	  \
	}							                                          \
};								                                          \

CAST_EXPR_CLASS(SExt)

CAST_EXPR_CLASS(ZExt)

CAST_EXPR_CLASS(Trunc)


/*******************************************************************************
 **                           BinaryExpr Class (Abstract)
 ******************************************************************************/

/*
 * Represents a binary instruction.
 * (In LLVM-IR such instructions are used also to e.g., negate a number.)
 */

class BinaryExpr : public SExpr {

protected:
	BinaryExpr(Kind k, Width w, std::unique_ptr<SExpr> &&l,
		   std::unique_ptr<SExpr> &&r)
		: SExpr(k, w) { addKid(std::move(l)); addKid(std::move(r)); }
	BinaryExpr(Kind k, const llvm::Type *t, std::unique_ptr<SExpr> &&l,
		   std::unique_ptr<SExpr> &&r)
		: SExpr(k, t) { addKid(std::move(l)); addKid(std::move(r)); }

	BinaryExpr(Kind k, std::unique_ptr<SExpr> &&l, std::unique_ptr<SExpr> &&r)
		: BinaryExpr(k, l->getWidth(), std::move(l), std::move(r)) {
		BUG_ON(getKid(0)->getWidth() != getKid(1)->getWidth());
	}

public:
	static bool classof(const SExpr *E) {
		Kind k = E->getKind();
		return SExpr::BinaryKindFirst <= k && k <= SExpr::BinaryKindLast;
	}
};

/* Arithmetic/Bit Exprs */
#define ARITHMETIC_EXPR_CLASS(_class_kind)                                             			\
class _class_kind##Expr : public BinaryExpr {	                                                        \
													\
protected:							                                        \
	_class_kind##Expr(std::unique_ptr<SExpr> &&l,							\
			  std::unique_ptr<SExpr> &&r)							\
	: BinaryExpr(_class_kind, std::move(l), std::move(r)) {}					\
	_class_kind##Expr(Width w, std::unique_ptr<SExpr> &&l, 						\
			  std::unique_ptr<SExpr> &&r)							\
	: BinaryExpr(_class_kind, w, std::move(l), std::move(r)) {}					\
	_class_kind##Expr(const llvm::Type *t, std::unique_ptr<SExpr> &&l, 				\
			  std::unique_ptr<SExpr> &&r)							\
	: BinaryExpr(_class_kind, t, std::move(l), std::move(r)) {}					\
													\
public:													\
	template<typename... Ts>									\
	static std::unique_ptr<_class_kind##Expr> create(Ts&&... params) {				\
		return std::unique_ptr<_class_kind##Expr>(						\
			new _class_kind##Expr(std::forward<Ts>(params)...));				\
	}				 								\
								                                        \
	std::unique_ptr<SExpr> clone() const override {                            			\
		return create(getKid(0)->clone(), getKid(1)->clone());                 			\
	}                                                                                               \
								                                        \
	static bool classof(const SExpr *E) {			                                	\
		return E->getKind() == SExpr::_class_kind;	                                	\
	}								                                \
};

ARITHMETIC_EXPR_CLASS(Add)

ARITHMETIC_EXPR_CLASS(Sub)

ARITHMETIC_EXPR_CLASS(Mul)

ARITHMETIC_EXPR_CLASS(UDiv)

ARITHMETIC_EXPR_CLASS(SDiv)

ARITHMETIC_EXPR_CLASS(URem)

ARITHMETIC_EXPR_CLASS(SRem)

ARITHMETIC_EXPR_CLASS(And)

ARITHMETIC_EXPR_CLASS(Or)

ARITHMETIC_EXPR_CLASS(Xor)

ARITHMETIC_EXPR_CLASS(Shl)

ARITHMETIC_EXPR_CLASS(LShr)

ARITHMETIC_EXPR_CLASS(AShr)

/* Comparison Exprs */
class CmpExpr : public BinaryExpr {

protected:
	CmpExpr(Kind k, std::unique_ptr<SExpr> &&l, std::unique_ptr<SExpr> &&r)
		: BinaryExpr(k, BoolWidth, std::move(l), std::move(r)) {}

public:
	static bool classof(const SExpr *E) {
		Kind k = E->getKind();
		return SExpr::CmpKindFirst <= k && k <= SExpr::CmpKindLast;
	}
};

#define COMPARISON_EXPR_CLASS(_class_kind)                                                 		      \
class _class_kind##Expr : public CmpExpr {                                                                    \
													      \
protected:									                              \
        _class_kind##Expr(std::unique_ptr<SExpr> &&l, std::unique_ptr<SExpr> &&r)           		      \
        : CmpExpr(_class_kind, std::move(l), std::move(r)) {}		                                      \
									                                      \
public:													      \
	template<typename... Ts>									      \
	static std::unique_ptr<_class_kind##Expr> create(Ts&&... params) {				      \
		return std::unique_ptr<_class_kind##Expr>(						      \
			new _class_kind##Expr(std::forward<Ts>(params)...));				      \
	}				 								      \
									                                      \
	std::unique_ptr<SExpr> clone() const override {                                  	      	      \
		return create(getKid(0)->clone(), getKid(1)->clone());                       	      	      \
	}                                                                                                     \
								                                              \
	static bool classof(const SExpr *E) {			                                      	      \
		return E->getKind() == SExpr::_class_kind;	                                      	      \
	}								                                      \
};

COMPARISON_EXPR_CLASS(Eq)

COMPARISON_EXPR_CLASS(Ne)

COMPARISON_EXPR_CLASS(Ult)

COMPARISON_EXPR_CLASS(Ule)

COMPARISON_EXPR_CLASS(Ugt)

COMPARISON_EXPR_CLASS(Uge)

COMPARISON_EXPR_CLASS(Slt)

COMPARISON_EXPR_CLASS(Sle)

COMPARISON_EXPR_CLASS(Sgt)

COMPARISON_EXPR_CLASS(Sge)

#endif /* __S_EXPR_HPP__ */
