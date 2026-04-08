#include <memory>

#include <gtest/gtest.h>

#include "genmc/Support/Cast.hpp"

/* A minimal class hierarchy for testing cast utilities */

class Base {
public:
	enum Kind { BaseKind, DerivedAKind, DerivedBKind };

	explicit Base(Kind k) : kind_(k) {}
	virtual ~Base() = default;

	[[nodiscard]] auto getKind() const -> Kind { return kind_; }

private:
	Kind kind_;
};

class DerivedA : public Base {
public:
	DerivedA() : Base(DerivedAKind) {}

	static auto classof(const Base *b) -> bool { return b->getKind() == DerivedAKind; }
};

class DerivedB : public Base {
public:
	DerivedB() : Base(DerivedBKind) {}

	static auto classof(const Base *b) -> bool { return b->getKind() == DerivedBKind; }
};

class Unrelated {
public:
	virtual ~Unrelated() = default;
};

/* isa<> */

TEST(CastTest, IsaDowncastMatch)
{
	DerivedA a;
	Base *bp = &a;
	EXPECT_TRUE(genmc::isa<DerivedA>(bp));
}

TEST(CastTest, IsaDowncastMismatch)
{
	DerivedA a;
	Base *bp = &a;
	EXPECT_FALSE(genmc::isa<DerivedB>(bp));
}

TEST(CastTest, IsaUpcast)
{
	DerivedA a;
	EXPECT_TRUE(genmc::isa<Base>(&a));
}

TEST(CastTest, IsaIdentity)
{
	DerivedA a;
	EXPECT_TRUE(genmc::isa<DerivedA>(&a));
}

/* isa_and_present<> */

TEST(CastTest, IsaAndPresentNull)
{
	EXPECT_FALSE(genmc::isa_and_present<DerivedA>(static_cast<Base *>(nullptr)));
}

TEST(CastTest, IsaAndPresentNonNull)
{
	DerivedA a;
	Base *bp = &a;
	EXPECT_TRUE(genmc::isa_and_present<DerivedA>(bp));
	EXPECT_FALSE(genmc::isa_and_present<DerivedB>(bp));
}

/* cast<> (raw pointer) */

TEST(CastTest, CastDowncast)
{
	DerivedA a;
	Base *bp = &a;
	auto *dp = genmc::cast<DerivedA>(bp);
	EXPECT_EQ(dp, &a);
}

TEST(CastTest, CastUpcast)
{
	DerivedA a;
	auto *bp = genmc::cast<Base>(&a);
	EXPECT_EQ(bp, static_cast<Base *>(&a));
}

TEST(CastTest, CastConstDowncast)
{
	DerivedA a;
	const Base *bp = &a;
	const auto *dp = genmc::cast<DerivedA>(bp);
	EXPECT_EQ(dp, &a);
}

/* cast<> (unique_ptr) */

TEST(CastTest, CastUniquePtrDowncast)
{
	auto bp = std::make_unique<DerivedA>();
	auto *raw = bp.get();
	auto dp = genmc::cast<DerivedA>(std::move(bp));
	EXPECT_EQ(dp.get(), raw);
}

TEST(CastTest, CastConstUniquePtrDowncast)
{
	std::unique_ptr<const Base> bp = std::make_unique<const DerivedA>();
	auto *raw = bp.get();
	auto dp = genmc::cast<DerivedA>(std::move(bp));
	EXPECT_EQ(dp.get(), raw);
}

/* dyn_cast<> */

TEST(CastTest, DynCastDowncastMatch)
{
	DerivedA a;
	Base *bp = &a;
	auto *dp = genmc::dyn_cast<DerivedA>(bp);
	EXPECT_NE(dp, nullptr);
	EXPECT_EQ(dp, &a);
}

TEST(CastTest, DynCastDowncastMismatch)
{
	DerivedA a;
	Base *bp = &a;
	auto *dp = genmc::dyn_cast<DerivedB>(bp);
	EXPECT_EQ(dp, nullptr);
}

TEST(CastTest, DynCastUpcast)
{
	DerivedA a;
	auto *bp = genmc::dyn_cast<Base>(&a);
	EXPECT_NE(bp, nullptr);
	EXPECT_EQ(bp, static_cast<Base *>(&a));
}

TEST(CastTest, DynCastConstUpcast)
{
	const DerivedA a;
	const auto *bp = genmc::dyn_cast<Base>(&a);
	EXPECT_NE(bp, nullptr);
	EXPECT_EQ(bp, static_cast<const Base *>(&a));
}

/* dyn_cast_if_present<> */

TEST(CastTest, DynCastIfPresentNull)
{
	auto *dp = genmc::dyn_cast_if_present<DerivedA>(static_cast<Base *>(nullptr));
	EXPECT_EQ(dp, nullptr);
}

TEST(CastTest, DynCastIfPresentMatch)
{
	DerivedA a;
	Base *bp = &a;
	auto *dp = genmc::dyn_cast_if_present<DerivedA>(bp);
	EXPECT_NE(dp, nullptr);
	EXPECT_EQ(dp, &a);
}

TEST(CastTest, DynCastIfPresentMismatch)
{
	DerivedA a;
	Base *bp = &a;
	auto *dp = genmc::dyn_cast_if_present<DerivedB>(bp);
	EXPECT_EQ(dp, nullptr);
}
