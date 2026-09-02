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

#ifndef GENMC_S_EXPR_HPP
#define GENMC_S_EXPR_HPP

#include "genmc/Support/Error.hpp"
#include "genmc/Support/SVal.hpp"

#include <algorithm>
#include <memory>

/*
 * Note: The infrastructure below uses unique_ptr<>s. Another alternative
 * would be to use shared_ptr<>s, which would enable sharing of expression
 * nodes. That, however, would complicate cloning a bit, which does occur
 * frequently in the current setting. Given that the expressions constructed
 * are probably going to be small, using unique_ptr<>s should be OK.
 */

/*
 * The hierarchy below is largely inspired from the KLEE one:
 *      https://github.com/klee/klee/
 */

/*******************************************************************************
 **                           SExpr Class (Abstract)
 ******************************************************************************/

/**
 * An abstract class that models expressions containing symbolic variables.
 * Contains some things all subclasses provide (e.g., the kids for this node).
 * Currently supports expressions with integers only.
 */
template <typename T> class SExpr { // NOLINT(cppcoreguidelines-special-member-functions)

public:
	using Width = unsigned;
	static const Width BoolWidth = 1;

	// NOLINTNEXTLINE(cppcoreguidelines-use-enum-class,readability-enum-initial-value)
	enum Kind : std::int8_t {
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
	SExpr() = delete; // NOLINT(modernize-use-equals-delete)
	SExpr(Kind kind, Width w, std::vector<std::unique_ptr<SExpr>> &&kids = {})
		: kind(kind), width(w), kids(std::move(kids))
	{}

public:
	virtual ~SExpr() = default;

	/** The kind of this node (LLVM-style RTTI) */
	[[nodiscard]] auto getKind() const -> Kind { return kind; }

	/** The width of this integer (in bits) */
	[[nodiscard]] auto getWidth() const -> Width { return width; }

	/** The kids of this node */
	[[nodiscard]] auto getNumKids() const -> size_t { return kids.size(); }

	/** Fetches the i-th kid */
	[[nodiscard]] auto getKid(unsigned i) const -> const SExpr<T> *
	{
		ASSERT(i < kids.size(), "Index out of bounds!");
		return kids[i].get();
	}
	auto getKid(unsigned i) -> SExpr<T> *
	{
		ASSERT(i < kids.size(), "Index out of bounds!");
		return kids[i].get();
	}

	/** Sets the i-th kid to e */
	void setKid(unsigned i, std::unique_ptr<SExpr> &&e)
	{
		ASSERT(i < kids.size(), "Index out of bounds!");
		kids[i] = std::move(e);
	}

	[[nodiscard]] virtual auto clone() const -> std::unique_ptr<SExpr> = 0;

	static auto classof(const SExpr<T> * /*unused*/) -> bool { return true; }

protected:
	template <typename U> friend class SExprEvaluator;
	template <typename U> friend class SExprTransformer;

	/** Returns a container with this node's kids */
	[[nodiscard]] auto getKids() const -> const std::vector<std::unique_ptr<SExpr<T>>> &
	{
		return kids;
	}

	/** This function is necessary because we cannot move from initializer lists,
	 * and also cannot copy unique_ptr<>s. We do need a way to construct a vector
	 * of unique_ptr<>s from its elements though... */
	void addKid(std::unique_ptr<SExpr<T>> &&kid) { kids.push_back(std::move(kid)); }

private:
	Kind kind;
	Width width;
	std::vector<std::unique_ptr<SExpr<T>>> kids;
};

/** Helper class to clone SExprs w/ value_ptr<>s */
template <typename T> struct SExprCloner {
	auto operator()(const SExpr<T> &expr) const -> SExpr<T> * { return expr.clone().release(); }
	// SExpr *operator()(SExpr &&expr) const { return new SExpr(std::move(expr)); }
};

/*******************************************************************************
 **                           ConcreteExpr Class
 ******************************************************************************/

/**
 * Represents a constant. For the time being, only integer constants are supported.
 */

template <typename T> class ConcreteExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	ConcreteExpr(Width w, SVal val) : SExpr<T>(Kind::Concrete, w), value(val) {}

public:
	/** Returns the constant value */
	[[nodiscard]] auto getValue() const -> const SVal & { return value; }

	template <typename... Ts>
	static auto create(Ts &&...params) -> std::unique_ptr<ConcreteExpr<T>>
	{
		return std::unique_ptr<ConcreteExpr<T>>(
			new ConcreteExpr(std::forward<Ts>(params)...));
	}
	static auto createTrue() -> std::unique_ptr<ConcreteExpr<T>>
	{
		return std::unique_ptr<ConcreteExpr<T>>(new ConcreteExpr(1, SVal(1)));
	}
	static auto createFalse() -> std::unique_ptr<ConcreteExpr<T>>
	{
		return std::unique_ptr<ConcreteExpr<T>>(new ConcreteExpr(1, SVal(0)));
	}

	[[nodiscard]] auto clone() const -> std::unique_ptr<SExpr<T>> override
	{
		return create(this->getWidth(), getValue());
	}

	static auto classof(const SExpr<T> *expr) -> bool
	{
		return expr->getKind() == Kind::Concrete;
	}

private:
	SVal value;
};

/*******************************************************************************
 **                            RegisterExpr Class
 ******************************************************************************/

/**
 * Represents a register the value of which is still unknown (symbolic variable).
 * Each register is (uniquely) represented using a void *. This class is completely
 * oblivious to what the void * actually points to and does not use/dereference it.
 * The users have to be careful and not assign the same pointer to different
 * registers. (Perhaps using a dedicated "ID" class for that would be better.)
 */

template <typename T> class RegisterExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	explicit RegisterExpr(Width width, const T &reg, const std::string &argname = "")
		: SExpr<T>(Kind::Register, width), reg(reg),
		  name(!argname.empty() ? argname : ("#s" + std::to_string(regCount++)))
	{}

public:
	/** Returns an identifier to this register */
	[[nodiscard]] auto getRegister() const -> const T & { return reg; }

	/** Returns the name of this register (in LLVM-IR) */
	[[nodiscard]] auto getName() const -> const std::string & { return name; }

	template <typename... Ts>
	static auto create(Ts &&...params) -> std::unique_ptr<RegisterExpr<T>>
	{
		return std::unique_ptr<RegisterExpr<T>>(
			new RegisterExpr(std::forward<Ts>(params)...));
	}

	[[nodiscard]] auto clone() const -> std::unique_ptr<SExpr<T>> override
	{
		return std::unique_ptr<SExpr<T>>(
			new RegisterExpr(this->getWidth(), getRegister(), getName()));
	}

	static auto classof(const SExpr<T> *expr) -> bool
	{
		return expr->getKind() == Kind::Register;
	}

private:
	/** Unique identifier for the symbolic variable */
	T reg;

	/** The name of this symbolic variable */
	const std::string name; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

	/** Counter used when creating names for symbolic vars */
	static unsigned regCount;
};

/*******************************************************************************
 **                            SelectExpr Class
 ******************************************************************************/

/**
 * Represents a select instruction. Equivalently, this can be thought of as
 * an 'if-then-else' statement, where if the condition (0th kid) holds
 * the expression takes the value of the first child, and if it does not
 * hold the expression takes the value of the second child (2nd kid).
 */

template <typename T> class SelectExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	SelectExpr(Width w, std::unique_ptr<SExpr<T>> &&cond, std::unique_ptr<SExpr<T>> &&trueExpr,
		   std::unique_ptr<SExpr<T>> &&f)
		: SExpr<T>(Kind::Select, w)
	{
		this->addKid(std::move(cond));
		this->addKid(std::move(trueExpr));
		this->addKid(std::move(f));
	}

	SelectExpr(std::unique_ptr<SExpr<T>> &&cond, std::unique_ptr<SExpr<T>> &&trueExpr,
		   std::unique_ptr<SExpr<T>> &&f)
		: SelectExpr(trueExpr->getWidth(), std::move(cond), std::move(trueExpr),
			     std::move(f))
	{}

public:
	template <typename... Ts>
	static auto create(Ts &&...params) -> std::unique_ptr<SelectExpr<T>>
	{
		return std::unique_ptr<SelectExpr<T>>(new SelectExpr(std::forward<Ts>(params)...));
	}

	[[nodiscard]] auto clone() const -> std::unique_ptr<SExpr<T>> override
	{
		auto cCond = this->getKid(0)->clone();
		auto cTrue = this->getKid(1)->clone();
		auto cFalse = this->getKid(2)->clone();
		return create(this->getWidth(), std::move(cCond), std::move(cTrue),
			      std::move(cFalse));
	}

	static auto classof(const SExpr<T> *expr) -> bool
	{
		return expr->getKind() == Kind::Select;
	}
};

/*******************************************************************************
 **                        LogicalExpr Class (Abstract)
 ******************************************************************************/

/**
 * Represents a logical operation (e.g., AND, OR). These do not correspond to LLVM-IR
 * instructions, but may be useful if we ever decide to construct such expressions.
 * They always have a width of 1, irrespective of the widths of their arguments.
 */

template <typename T> class LogicalExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	LogicalExpr(Kind kind, Width w, std::vector<std::unique_ptr<SExpr<T>>> &&es)
		: SExpr<T>(kind, w, std::move(es))
	{
		ASSERT(!this->getKids().empty());
	}
	LogicalExpr(Kind kind, std::vector<std::unique_ptr<SExpr<T>>> &&es)
		: LogicalExpr(kind, SExpr<T>::BoolWidth, std::move(es))
	{}

	/** For convenience */
	LogicalExpr(Kind kind, std::unique_ptr<SExpr<T>> &&e) : SExpr<T>(kind, SExpr<T>::BoolWidth)
	{
		this->addKid(std::move(e));
	}
	LogicalExpr(Kind kind, std::unique_ptr<SExpr<T>> &&e1, std::unique_ptr<SExpr<T>> &&e2)
		: SExpr<T>(kind, SExpr<T>::BoolWidth)
	{
		this->addKid(std::move(e1));
		this->addKid(std::move(e2));
	}

public:
	static auto classof(const SExpr<T> *expr) -> bool
	{
		auto kind = expr->getKind();
		return Kind::LogicalKindFirst <= kind && kind <= Kind::LogicalKindLast;
	}
};

#define LOGICAL_EXPR_CLASS(_class_kind)                                                            \
	template <typename T> class _class_kind##Expr : public LogicalExpr<T> {                    \
                                                                                                   \
	protected:                                                                                 \
		using Kind = typename SExpr<T>::Kind;                                              \
                                                                                                   \
		template <typename BEGIN, typename END>                                            \
		_class_kind##Expr(BEGIN itb, END ite)                                              \
			: LogicalExpr<T>(Kind::_class_kind, itb, ite)                              \
		{}                                                                                 \
		_class_kind##Expr(std::vector<std::unique_ptr<SExpr<T>>> &&es)                     \
			: LogicalExpr<T>(Kind::_class_kind, std::move(es))                         \
		{}                                                                                 \
		_class_kind##Expr(std::unique_ptr<SExpr<T>> &&e)                                   \
			: LogicalExpr<T>(Kind::_class_kind, std::move(e))                          \
		{}                                                                                 \
		_class_kind##Expr(std::unique_ptr<SExpr<T>> &&e1, std::unique_ptr<SExpr<T>> &&e2)  \
			: LogicalExpr<T>(Kind::_class_kind, std::move(e1), std::move(e2))          \
		{}                                                                                 \
                                                                                                   \
	public:                                                                                    \
		template <typename... Ts>                                                          \
		static auto create(Ts &&...params) -> std::unique_ptr<_class_kind##Expr<T>>        \
		{                                                                                  \
			return std::unique_ptr<_class_kind##Expr<T>>(                              \
				new _class_kind##Expr(std::forward<Ts>(params)...));               \
		}                                                                                  \
                                                                                                   \
		auto clone() const -> std::unique_ptr<SExpr<T>> override                           \
		{                                                                                  \
			std::vector<std::unique_ptr<SExpr<T>>> kidsCopy;                           \
			std::for_each(this->getKids().begin(), this->getKids().end(),              \
				      [&](const std::unique_ptr<SExpr<T>> &s) {                    \
					      kidsCopy.push_back(s->clone());                      \
				      });                                                          \
			return create(std::move(kidsCopy));                                        \
		}                                                                                  \
                                                                                                   \
		static auto classof(const SExpr<T> *expr) -> bool                                  \
		{                                                                                  \
			return expr->getKind() == SExpr<T>::_class_kind;                           \
		}                                                                                  \
	};

LOGICAL_EXPR_CLASS(Conjunction)

LOGICAL_EXPR_CLASS(Disjunction)

/** Represents a boolean negation */
template <typename T> class NotExpr : public LogicalExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;

	NotExpr(std::unique_ptr<SExpr<T>> &&e) : LogicalExpr<T>(Kind::Not, std::move(e)) {}

public:
	template <typename... Ts> static auto create(Ts &&...params) -> std::unique_ptr<NotExpr<T>>
	{
		return std::unique_ptr<NotExpr<T>>(new NotExpr(std::forward<Ts>(params)...));
	}

	[[nodiscard]] auto clone() const -> std::unique_ptr<SExpr<T>> override
	{
		return create(this->getKid(0)->clone());
	}

	static auto classof(const SExpr<T> *expr) -> bool { return expr->getKind() == Kind::Not; }
};

/*******************************************************************************
 **                           CastExpr Class (Abstract)
 ******************************************************************************/

/**
 * Represents a cast instruction (e.g., zext, trunc).
 */

template <typename T> class CastExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	CastExpr(Kind kind, Width w, std::unique_ptr<SExpr<T>> &&e) : SExpr<T>(kind, w)
	{
		this->addKid(std::move(e));
	}

public:
	static auto classof(const SExpr<T> *expr) -> bool
	{
		auto kind = expr->getKind();
		return Kind::CastKindFirst <= kind && kind <= Kind::CastKindLast;
	}
};

#define CAST_EXPR_CLASS(_class_kind)                                                               \
	template <typename T> class _class_kind##Expr : public CastExpr<T> {                       \
                                                                                                   \
	protected:                                                                                 \
		using Kind = typename SExpr<T>::Kind;                                              \
		using Width = typename SExpr<T>::Width;                                            \
                                                                                                   \
		_class_kind##Expr(Width w, std::unique_ptr<SExpr<T>> &&e)                          \
			: CastExpr<T>(Kind::_class_kind, w, std::move(e))                          \
		{}                                                                                 \
                                                                                                   \
	public:                                                                                    \
		template <typename... Ts>                                                          \
		static auto create(Ts &&...params) -> std::unique_ptr<_class_kind##Expr<T>>        \
		{                                                                                  \
			return std::unique_ptr<_class_kind##Expr<T>>(                              \
				new _class_kind##Expr(std::forward<Ts>(params)...));               \
		}                                                                                  \
                                                                                                   \
		auto clone() const -> std::unique_ptr<SExpr<T>> override                           \
		{                                                                                  \
			return create(this->getWidth(), this->getKid(0)->clone());                 \
		}                                                                                  \
                                                                                                   \
		static auto classof(const SExpr<T> *expr) -> bool                                  \
		{                                                                                  \
			return expr->getKind() == SExpr<T>::_class_kind;                           \
		}                                                                                  \
	};

CAST_EXPR_CLASS(SExt)

CAST_EXPR_CLASS(ZExt)

CAST_EXPR_CLASS(Trunc)

/*******************************************************************************
 **                           BinaryExpr Class (Abstract)
 ******************************************************************************/

/**
 * Represents a binary instruction.
 * (In LLVM-IR such instructions are used also to e.g., negate a number.)
 */
template <typename T> class BinaryExpr : public SExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	BinaryExpr(Kind kind, Width w, std::unique_ptr<SExpr<T>> &&l, std::unique_ptr<SExpr<T>> &&r)
		: SExpr<T>(kind, w)
	{
		this->addKid(std::move(l));
		this->addKid(std::move(r));
	}
	BinaryExpr(Kind kind, std::unique_ptr<SExpr<T>> &&l, std::unique_ptr<SExpr<T>> &&r)
		: BinaryExpr(kind, l->getWidth(), std::move(l), std::move(r))
	{}

public:
	static auto classof(const SExpr<T> *expr) -> bool
	{
		auto kind = expr->getKind();
		return Kind::BinaryKindFirst <= kind && kind <= Kind::BinaryKindLast;
	}
};

/** Arithmetic/Bit Exprs */
#define ARITHMETIC_EXPR_CLASS(_class_kind)                                                         \
	template <typename T> class _class_kind##Expr : public BinaryExpr<T> {                     \
                                                                                                   \
	protected:                                                                                 \
		using Kind = typename SExpr<T>::Kind;                                              \
		using Width = typename SExpr<T>::Width;                                            \
                                                                                                   \
		_class_kind##Expr(std::unique_ptr<SExpr<T>> &&l, std::unique_ptr<SExpr<T>> &&r)    \
			: BinaryExpr<T>(Kind::_class_kind, std::move(l), std::move(r))             \
		{}                                                                                 \
		_class_kind##Expr(Width w, std::unique_ptr<SExpr<T>> &&l,                          \
				  std::unique_ptr<SExpr<T>> &&r)                                   \
			: BinaryExpr<T>(Kind::_class_kind, w, std::move(l), std::move(r))          \
		{}                                                                                 \
                                                                                                   \
	public:                                                                                    \
		template <typename... Ts>                                                          \
		static auto create(Ts &&...params) -> std::unique_ptr<_class_kind##Expr<T>>        \
		{                                                                                  \
			return std::unique_ptr<_class_kind##Expr<T>>(                              \
				new _class_kind##Expr(std::forward<Ts>(params)...));               \
		}                                                                                  \
                                                                                                   \
		auto clone() const -> std::unique_ptr<SExpr<T>> override                           \
		{                                                                                  \
			return create(this->getKid(0)->clone(), this->getKid(1)->clone());         \
		}                                                                                  \
                                                                                                   \
		static auto classof(const SExpr<T> *expr) -> bool                                  \
		{                                                                                  \
			return expr->getKind() == Kind::_class_kind;                               \
		}                                                                                  \
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

/** Comparison Expressions */
template <typename T> class CmpExpr : public BinaryExpr<T> {

protected:
	using Kind = typename SExpr<T>::Kind;
	using Width = typename SExpr<T>::Width;

	CmpExpr(Kind kind, std::unique_ptr<SExpr<T>> &&l, std::unique_ptr<SExpr<T>> &&r)
		: BinaryExpr<T>(kind, SExpr<T>::BoolWidth, std::move(l), std::move(r))
	{}

public:
	static auto classof(const SExpr<T> *expr) -> bool
	{
		auto kind = expr->getKind();
		return Kind::CmpKindFirst <= kind && kind <= Kind::CmpKindLast;
	}
};

#define COMPARISON_EXPR_CLASS(_class_kind)                                                         \
	template <typename T> class _class_kind##Expr : public CmpExpr<T> {                        \
                                                                                                   \
	protected:                                                                                 \
		using Kind = typename SExpr<T>::Kind;                                              \
		using Width = typename SExpr<T>::Width;                                            \
                                                                                                   \
		_class_kind##Expr(std::unique_ptr<SExpr<T>> &&l, std::unique_ptr<SExpr<T>> &&r)    \
			: CmpExpr<T>(Kind::_class_kind, std::move(l), std::move(r))                \
		{}                                                                                 \
                                                                                                   \
	public:                                                                                    \
		template <typename... Ts>                                                          \
		static auto create(Ts &&...params) -> std::unique_ptr<_class_kind##Expr<T>>        \
		{                                                                                  \
			return std::unique_ptr<_class_kind##Expr<T>>(                              \
				new _class_kind##Expr(std::forward<Ts>(params)...));               \
		}                                                                                  \
                                                                                                   \
		auto clone() const -> std::unique_ptr<SExpr<T>> override                           \
		{                                                                                  \
			return create(this->getKid(0)->clone(), this->getKid(1)->clone());         \
		}                                                                                  \
                                                                                                   \
		static auto classof(const SExpr<T> *expr) -> bool                                  \
		{                                                                                  \
			return expr->getKind() == Kind::_class_kind;                               \
		}                                                                                  \
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

template <typename T> unsigned RegisterExpr<T>::regCount = 0;

#endif /* GENMC_S_EXPR_HPP */
