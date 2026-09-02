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

#ifndef GENMC_REVISIT_HPP
#define GENMC_REVISIT_HPP

#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Verification/VerificationError.hpp"

#include <format>
#include <utility>

class ReadRevisit;

/** Abstract class representing a revisit operation */
class Revisit { // NOLINT(cppcoreguidelines-special-member-functions)

public:
	/** LLVM-style RTTI discriminator */
	enum Kind : std::uint8_t { // NOLINT(cppcoreguidelines-use-enum-class)
		RV_FRev,
		RV_FRevRead,
		RV_FRevMO,
		RV_FRevOpt,
		RV_FRevRerun,
		RV_FRevReplay,
		RV_FRevWeakCasFail,
		RV_FRevLast,
		RV_BRev,
		RV_BRevLast,

	};

protected:
	/** Constructors */
	Revisit() = delete; // NOLINT(modernize-use-equals-delete)
	Revisit(Kind kind, Event pos) : kind(kind), pos(pos) {}

public:
	/** Returns the kind of this item */
	[[nodiscard]] auto getKind() const -> Kind { return kind; }

	/** Returns the event for which we are exploring an alternative exploration option */
	[[nodiscard]] auto getPos() const -> Event { return pos; }

	static auto classofKind(Kind /*K*/) -> bool { return true; }
	static auto castToReadRevisit(const Revisit * /*r*/) -> ReadRevisit *;
	static auto castFromReadRevisit(const ReadRevisit * /*r*/) -> Revisit *;

	/** Destructor and printing facilities */
	virtual ~Revisit() = default;

private:
	Kind kind;
	Event pos;
};

/** Multiple hierarchy for read revisits */
class ReadRevisit {

protected:
	ReadRevisit(Revisit::Kind kind, Event rev) : revisitKind(kind), rev(rev) {}

public:
	[[nodiscard]] auto getRev() const -> Event { return rev; }

	[[nodiscard]] auto getRevisitKind() const -> Revisit::Kind { return revisitKind; }
	static auto classof(const Revisit *r) -> bool
	{
		return r->getKind() == Revisit::RV_FRevRead ||
		       (r->getKind() >= Revisit::RV_BRev && r->getKind() <= Revisit::RV_BRevLast);
	}

private:
	Revisit::Kind revisitKind;

	/** The store revisiting the read */
	Event rev;
};

/** Represents a forward revisit */
class ForwardRevisit : public Revisit {

protected:
	ForwardRevisit() = delete; // NOLINT(modernize-use-equals-delete)
	ForwardRevisit(Kind kind, Event pos) : Revisit(kind, pos) {}

public:
	static auto classof(const Revisit *item) -> bool
	{
		return item->getKind() >= RV_FRev && item->getKind() <= RV_FRevLast;
	}
};

/** Forward revisit a read */
class ReadForwardRevisit : public ForwardRevisit, public ReadRevisit {

public:
	/** Constructors */
	ReadForwardRevisit() = delete;
	ReadForwardRevisit(Event pos, Event r, bool isMax = false)
		: ForwardRevisit(RV_FRevRead, pos), ReadRevisit(RV_FRevRead, r), maximal(isMax)
	{}

	[[nodiscard]] auto isMaximal() const -> bool { return maximal; }

	static auto classof(const Revisit *item) -> bool { return item->getKind() == RV_FRevRead; }
	static auto castToReadRevisit(const ReadForwardRevisit *r) -> ReadRevisit *
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<ReadRevisit *>(const_cast<ReadForwardRevisit *>(r));
	}
	static auto castFromReadRevisit(const ReadRevisit *r) -> ReadForwardRevisit *
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<ReadForwardRevisit *>(const_cast<ReadRevisit *>(r));
	}

private:
	bool maximal;
};

/** Represents an alternative MO position for a store */
class WriteForwardRevisit : public ForwardRevisit {

protected:
	WriteForwardRevisit(Kind kind, Event pos, Event moPred)
		: ForwardRevisit(kind, pos), moPred(moPred)
	{}

public:
	WriteForwardRevisit(Event pos, Event moPred) : WriteForwardRevisit(RV_FRevMO, pos, moPred)
	{}

	/** Returns the new MO predecessor of the event for which
	 * we are exploring alternative exploration options */
	[[nodiscard]] auto getPred() const -> Event { return moPred; }

	static auto classof(const Revisit *item) -> bool { return item->getKind() == RV_FRevMO; }

private:
	Event moPred;
};

/** Represents the revisit of an optional block */
class OptionalForwardRevisit : public ForwardRevisit {

public:
	OptionalForwardRevisit(Event pos) : ForwardRevisit(RV_FRevOpt, pos) {}

	static auto classof(const Revisit *item) -> bool { return item->getKind() == RV_FRevOpt; }
};

/** Represents an execution "rerun".  Helpful e.g., when operating under EstimationMode */
class RerunForwardRevisit : public ForwardRevisit {

public:
	RerunForwardRevisit() : ForwardRevisit(RV_FRevRerun, Event::getInit()) {}

	static auto classof(const Revisit *item) -> bool { return item->getKind() == RV_FRevRerun; }
};

/** Represents an execution replay.  Helpful in error reporting */
class ReplayForwardRevisit : public ForwardRevisit {

public:
	ReplayForwardRevisit(Event pos, ErrorDetails details)
		: ForwardRevisit(RV_FRevReplay, pos), details_(std::move(details))
	{}

	[[nodiscard]] auto getDetails() const -> const ErrorDetails & { return details_; }

	static auto classof(const Revisit *item) -> bool
	{
		return item->getKind() == RV_FRevReplay;
	}

private:
	ErrorDetails details_;
};

/** Represents the spurious-failure option of a weak CAS (always non-maximal).
 * Should not perform the actions of a ReadRevisit, hence not a subclass. */
class WeakCasFailureRevisit : public ForwardRevisit {

public:
	WeakCasFailureRevisit(Event pos, Event rev)
		: ForwardRevisit(RV_FRevWeakCasFail, pos), rev(rev)
	{}

	/** The store the spuriously-failing CAS reads from */
	[[nodiscard]] auto getRev() const -> Event { return rev; }

	static auto classof(const Revisit *item) -> bool
	{
		return item->getKind() == RV_FRevWeakCasFail;
	}

private:
	Event rev;
};

/** Represents a backward revisit */
class BackwardRevisit : public Revisit, public ReadRevisit {

protected:
	BackwardRevisit(Kind kind, Event pos, Event r, std::unique_ptr<VectorClock> view)
		: Revisit(kind, pos), ReadRevisit(kind, r), view(std::move(view))
	{}

public:
	BackwardRevisit(Event pos, Event r, std::unique_ptr<VectorClock> view)
		: BackwardRevisit(RV_BRev, pos, r, std::move(view))
	{}
	BackwardRevisit(const ReadLabel *rLab, const WriteLabel *wLab,
			std::unique_ptr<VectorClock> view)
		: BackwardRevisit(rLab->getPos(), wLab->getPos(), std::move(view))
	{}

	/** Returns (releases) the prefix of the revisiting event */
	auto getViewRel() -> std::unique_ptr<VectorClock> { return std::move(view); }

	/** Returns (but does not release) the prefix of the revisiting event */
	[[nodiscard]] auto getViewNoRel() const -> const std::unique_ptr<VectorClock> &
	{
		return view;
	}

	static auto classof(const Revisit *item) -> bool
	{
		return item->getKind() >= RV_BRev && item->getKind() <= RV_BRevLast;
	}
	static auto castToReadRevisit(const BackwardRevisit *r) -> ReadRevisit *
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<ReadRevisit *>(const_cast<BackwardRevisit *>(r));
	}
	static auto castFromReadRevisit(const ReadRevisit *r) -> BackwardRevisit *
	{
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<BackwardRevisit *>(const_cast<ReadRevisit *>(r));
	}

private:
	std::unique_ptr<VectorClock> view;
};

/*******************************************************************************
 **                             Static methods
 *******************************************************************************/

inline auto Revisit::castFromReadRevisit(const ReadRevisit *r) -> Revisit *
{
	auto rk = r->getRevisitKind();
	switch (rk) {
	case Revisit::Kind::RV_FRevRead:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<ReadForwardRevisit *>(const_cast<ReadRevisit *>(r));
	case Revisit::Kind::RV_BRev:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<BackwardRevisit *>(const_cast<ReadRevisit *>(r));
	default:
		return nullptr;
	}
}

inline auto Revisit::castToReadRevisit(const Revisit *r) -> ReadRevisit *
{
	auto rk = r->getKind();
	switch (rk) {
	case Revisit::Kind::RV_FRevRead:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<ReadForwardRevisit *>(const_cast<Revisit *>(r));
	case Revisit::Kind::RV_BRev:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast,cppcoreguidelines-pro-type-const-cast)
		return static_cast<BackwardRevisit *>(const_cast<Revisit *>(r));
	default:
		return nullptr;
	}
}

/*******************************************************************************
 **                             RTTI helpers
 *******************************************************************************/

namespace genmc {

template <> inline auto isa<ReadRevisit>(const ::Revisit *Base) -> bool
{
	return ::Revisit::castToReadRevisit(Base);
}

template <> inline auto dyn_cast<ReadRevisit>(const Revisit *Base) -> const ::ReadRevisit *
{
	return ::Revisit::castToReadRevisit(Base);
}

template <> inline auto dyn_cast(::Revisit *Base) -> ::ReadRevisit *
{
	return ::Revisit::castToReadRevisit(Base);
}

} /* namespace genmc */

/**** Formatting ****/
/** Make `Revisit::Kind` formattable with `std::format`. */
template <> struct std::formatter<Revisit::Kind> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Revisit::Kind &kind, std::format_context &ctx) const
	{
		std::string_view str;
		switch (kind) {
		case Revisit::RV_FRevRead:
			str = "FR";
			break;
		case Revisit::RV_FRevOpt:
			str = "OPT";
			break;
		case Revisit::RV_FRevRerun:
			str = "RERUN";
			break;
		case Revisit::RV_FRevReplay:
			str = "REPLAY";
			break;
		case Revisit::RV_FRevMO:
			str = "MO";
			break;
		case Revisit::RV_FRevWeakCasFail:
			str = "WCF";
			break;
		case Revisit::RV_BRev:
			str = "BR";
			break;
		default:
			PRINT_BUGREPORT_INFO_ONCE("print-revisit-type",
						  "Cannot print revisit type");
			str = "UNKNOWN";
			break;
		}
		return std::format_to(ctx.out(), "{}", str);
	}
};

/** Make `Revisit` formattable with `std::format`. */
template <> struct std::formatter<Revisit> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Revisit &item, std::format_context &ctx) const
	{
		switch (item.getKind()) {
		case Revisit::RV_FRevRead: {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			const auto &fi = static_cast<const ReadForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: {})", fi.getKind(), fi.getPos(),
					      fi.getRev());
		}
		case Revisit::RV_FRevMO: {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			const auto &mi = static_cast<const WriteForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: {})", mi.getKind(), mi.getPos(),
					      mi.getPred());
		}
		case Revisit::RV_FRevOpt: {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			const auto &mi = static_cast<const OptionalForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({})", mi.getKind(), mi.getPos());
		}
		case Revisit::RV_BRev: {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			const auto &bi = static_cast<const BackwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: [{}, {}])", bi.getKind(),
					      bi.getPos(), bi.getRev(), *bi.getViewNoRel());
		}
		default:
			return std::format_to(ctx.out(), "{}", item.getKind());
		}
	}
};

#endif /* GENMC_REVISIT_HPP */
