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

#include "ExecutionGraph/EventLabel.hpp"
#include "Verification/VerificationError.hpp"

#include <format>
#include <utility>

class ReadRevisit;

/** Abstract class representing a revisit operation */
class Revisit {

public:
	/** LLVM-style RTTI discriminator */
	enum Kind {
		RV_FRev,
		RV_FRevRead,
		RV_FRevMO,
		RV_FRevOpt,
		RV_FRevRerun,
		RV_FRevReplay,
		RV_FRevLast,
		RV_BRev,
		RV_BRevLast,

	};

protected:
	/** Constructors */
	Revisit() = delete;
	Revisit(Kind k, Event p) : kind(k), pos(p) {}

public:
	/** Returns the kind of this item */
	Kind getKind() const { return kind; }

	/** Returns the event for which we are exploring an alternative exploration option */
	Event getPos() const { return pos; }

	static bool classofKind(Kind K) { return true; }
	static ReadRevisit *castToReadRevisit(const Revisit *);
	static Revisit *castFromReadRevisit(const ReadRevisit *);

	/** Destructor and printing facilities */
	virtual ~Revisit() {}

private:
	Kind kind;
	Event pos;
};

/** Multiple hierarchy for read revisits */
class ReadRevisit {

protected:
	ReadRevisit(Revisit::Kind k, Event rev) : revisitKind(k), rev(rev) {}

public:
	Event getRev() const { return rev; }

	Revisit::Kind getRevisitKind() const { return revisitKind; }
	static bool classof(const Revisit *r)
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
	ForwardRevisit() = delete;
	ForwardRevisit(Kind k, Event p) : Revisit(k, p) {}

public:
	static bool classof(const Revisit *item)
	{
		return item->getKind() >= RV_FRev && item->getKind() <= RV_FRevLast;
	}
};

/** Forward revisit a read */
class ReadForwardRevisit : public ForwardRevisit, public ReadRevisit {

public:
	/** Constructors */
	ReadForwardRevisit() = delete;
	ReadForwardRevisit(Event p, Event r, bool m = false)
		: ForwardRevisit(RV_FRevRead, p), maximal(m), ReadRevisit(RV_FRevRead, r)
	{}

	bool isMaximal() const { return maximal; }

	static bool classof(const Revisit *item) { return item->getKind() == RV_FRevRead; }
	static ReadRevisit *castToReadRevisit(const ReadForwardRevisit *r)
	{
		return static_cast<ReadRevisit *>(const_cast<ReadForwardRevisit *>(r));
	}
	static ReadForwardRevisit *castFromReadRevisit(const ReadRevisit *r)
	{
		return static_cast<ReadForwardRevisit *>(const_cast<ReadRevisit *>(r));
	}

private:
	bool maximal;
};

/** Represents an alternative MO position for a store */
class WriteForwardRevisit : public ForwardRevisit {

protected:
	WriteForwardRevisit(Kind k, Event p, Event moPred) : ForwardRevisit(k, p), moPred(moPred) {}

public:
	WriteForwardRevisit(Event p, Event moPred) : WriteForwardRevisit(RV_FRevMO, p, moPred) {}

	/** Returns the new MO predecessor of the event for which
	 * we are exploring alternative exploration options */
	Event getPred() const { return moPred; }

	static bool classof(const Revisit *item) { return item->getKind() == RV_FRevMO; }

private:
	Event moPred;
};

/** Represents the revisit of an optional block */
class OptionalForwardRevisit : public ForwardRevisit {

public:
	OptionalForwardRevisit(Event p) : ForwardRevisit(RV_FRevOpt, p) {}

	static bool classof(const Revisit *item) { return item->getKind() == RV_FRevOpt; }
};

/** Represents an execution "rerun".  Helpful e.g., when operating under EstimationMode */
class RerunForwardRevisit : public ForwardRevisit {

public:
	RerunForwardRevisit() : ForwardRevisit(RV_FRevRerun, Event::getInit()) {}

	static bool classof(const Revisit *item) { return item->getKind() == RV_FRevRerun; }
};

/** Represents an execution replay.  Helpful in error reporting */
class ReplayForwardRevisit : public ForwardRevisit {

public:
	ReplayForwardRevisit(Event pos, ErrorDetails details)
		: ForwardRevisit(RV_FRevReplay, pos), details_(std::move(details))
	{}

	const ErrorDetails &getDetails() const { return details_; }

	static bool classof(const Revisit *item) { return item->getKind() == RV_FRevReplay; }

private:
	ErrorDetails details_;
};

/** Represents a backward revisit */
class BackwardRevisit : public Revisit, public ReadRevisit {

protected:
	BackwardRevisit(Kind k, Event p, Event r, std::unique_ptr<VectorClock> view)
		: Revisit(k, p), view(std::move(view)), ReadRevisit(k, r)
	{}

public:
	BackwardRevisit(Event p, Event r, std::unique_ptr<VectorClock> view)
		: BackwardRevisit(RV_BRev, p, r, std::move(view))
	{}
	BackwardRevisit(const ReadLabel *rLab, const WriteLabel *wLab,
			std::unique_ptr<VectorClock> view)
		: BackwardRevisit(rLab->getPos(), wLab->getPos(), std::move(view))
	{}

	/** Returns (releases) the prefix of the revisiting event */
	std::unique_ptr<VectorClock> getViewRel() { return std::move(view); }

	/** Returns (but does not release) the prefix of the revisiting event */
	const std::unique_ptr<VectorClock> &getViewNoRel() const { return view; }

	static bool classof(const Revisit *item)
	{
		return item->getKind() >= RV_BRev && item->getKind() <= RV_BRevLast;
	}
	static ReadRevisit *castToReadRevisit(const BackwardRevisit *r)
	{
		return static_cast<ReadRevisit *>(const_cast<BackwardRevisit *>(r));
	}
	static BackwardRevisit *castFromReadRevisit(const ReadRevisit *r)
	{
		return static_cast<BackwardRevisit *>(const_cast<ReadRevisit *>(r));
	}

private:
	std::unique_ptr<VectorClock> view;
};

/*******************************************************************************
 **                             Static methods
 *******************************************************************************/

inline Revisit *Revisit::castFromReadRevisit(const ReadRevisit *r)
{
	auto rk = r->getRevisitKind();
	switch (rk) {
	case Revisit::Kind::RV_FRevRead:
		return static_cast<ReadForwardRevisit *>(const_cast<ReadRevisit *>(r));
	case Revisit::Kind::RV_BRev:
		return static_cast<BackwardRevisit *>(const_cast<ReadRevisit *>(r));
	default:
		return nullptr;
	}
}

inline ReadRevisit *Revisit::castToReadRevisit(const Revisit *r)
{
	auto rk = r->getKind();
	switch (rk) {
	case Revisit::Kind::RV_FRevRead:
		return static_cast<ReadForwardRevisit *>(const_cast<Revisit *>(r));
	case Revisit::Kind::RV_BRev:
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

	auto format(const Revisit::Kind &k, std::format_context &ctx) const
	{
		std::string_view str;
		switch (k) {
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
			auto &fi = static_cast<const ReadForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: {})", fi.getKind(), fi.getPos(),
					      fi.getRev());
		}
		case Revisit::RV_FRevMO: {
			auto &mi = static_cast<const WriteForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: {})", mi.getKind(), mi.getPos(),
					      mi.getPred());
		}
		case Revisit::RV_FRevOpt: {
			auto &mi = static_cast<const OptionalForwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({})", mi.getKind(), mi.getPos());
		}
		case Revisit::RV_BRev: {
			auto &bi = static_cast<const BackwardRevisit &>(item);
			return std::format_to(ctx.out(), "{}({}: [{}, {}])", bi.getKind(),
					      bi.getPos(), bi.getRev(), *bi.getViewNoRel());
		}
		default:
			return std::format_to(ctx.out(), "{}", item.getKind());
		}
	}
};

#endif /* GENMC_REVISIT_HPP */
