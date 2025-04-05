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
 * Author: Pavel Golovin <pgolovin@mpi-sws.org>
 */

#include "Verification/Relinche/Specification.hpp"
#include "ADT/Matrix2D.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "Support/DotPrint.hpp"
#include "Verification/Relinche/Observation.hpp"

#include <llvm/ADT/BitVector.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <istream>
#include <queue>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

auto Specification::lookupSyncOrInsert(const Observation &obs) -> Specification::Record &
{
	for (auto &it : llvm::make_range(data_.equal_range(obs)))
		if (it.first.hasSameHb(obs))
			return it.second;

	return data_.emplace(obs, Record{.lins = {}, .hints = {}})->second;
}

auto Specification::lookupSync(const Observation &obs) const -> const Specification::Record &
{
	for (const auto &it : llvm::make_range(data_.equal_range(obs)))
		if (it.first.hasSameHb(obs))
			return it.second;
	ERROR("Observation not found!");
}

void Specification::addHints(const Observation &obs, std::vector<Hint> &&newHints)
{
	auto &hints = lookupSyncOrInsert(obs).hints;
	for (auto &hint : newHints)
		hints.emplace_back(std::move(hint));
}

auto Specification::isRefinedBy(const Observation &obs) const -> bool
{
	return std::ranges::any_of(llvm::make_range(data_.equal_range(obs)),
				   [&](auto &specObs) { return specObs.first.isRefinedBy(obs); });
}

auto Specification::refinementMissingEdges(const Observation &obs) const
	-> std::vector<std::vector<Edge>>
{
	std::vector<std::vector<Edge>> res;
	for (const auto &specObs : llvm::make_range(data_.equal_range(obs))) {
		std::vector<Edge> missedEdges;
		for (const auto &[op1, op2] : specObs.first.calculateHbDiff((obs)))
			missedEdges.emplace_back(obs.getCall(op1).beginLab->getPos(),
						 obs.getCall(op2).endLab->getPos());
		std::ranges::sort(missedEdges);
		if (std::ranges::none_of(res, [&](auto &es) {
			    return std::ranges::includes(missedEdges, es);
		    })) {
			std::erase_if(res, [&](auto &es) {
				return std::ranges::includes(es, missedEdges);
			});
			res.emplace_back(std::move(missedEdges));
		}
	}
	std::ranges::sort(res);
	return res;
}

void Specification::merge(Specification &&other)
{
	for (auto &it : other.data_) {
		auto &r = lookupSyncOrInsert(it.first);
		r.lins.insert(it.second.lins);
		for (auto &hint : it.second.hints)
			r.hints.emplace_back(std::move(hint));
	}
}

/*******************************************************************************
 **                     Observation addition utilities
 ******************************************************************************/

using CallEquivalencesSet = std::vector<std::vector<MethodCall::Id>>;
using EdgeEquivalencesSet = std::vector<std::vector<Observation::CallEdge>>;
using PermutationMap = Linearization::PermutationMap;

/* calculate hb of specification ignoring abstract lock */
static auto calculateLinearization(const ExecutionGraph &graph, const Observation &obs)
	-> Linearization
{
	/* Find the location of the global lock */
	auto labs = graph.labels();
	auto labIt = std::ranges::find_if(
		labs, [&](auto &lab) { return llvm::isa<AbstractLockCasWriteLabel>(&lab); });
	ERROR_ON(labIt == std::ranges::end(labs), "RELINCHE: Could not find abstract global lock.");
	auto lockLoc = llvm::dyn_cast<AbstractLockCasWriteLabel>(&*labIt)->getAddr();

	/* Project the co of global lock to begin/end events */
	std::vector<MethodCall::Id> result;
	for (const auto &lab : graph.co(lockLoc)) {
		if (!llvm::isa<AbstractLockCasWriteLabel>(&lab))
			continue;
		if (auto opIx = obs.getContainingCallId(lab))
			result.push_back(opIx.value());
	}
	return Linearization(std::move(result));
}

static auto generatePermutation(const auto &&callIdRange, const CallEquivalencesSet &callEqs,
				const EdgeEquivalencesSet &edgeEqs) -> PermutationMap
{
	PermutationMap result(MethodCall::Id(0));

	/* Initial permutation: identity */
	result.resize(std::ranges::distance(callIdRange));
	for (auto id : callIdRange)
		result[id.value()] = id;

	/* Calculate permutation based on callEQs and edgeEQs */
	for (const auto &eq : callEqs) {
		int j = 0;
		for (const auto &item : callIdRange)
			if (std::ranges::find(eq, item) != eq.end()) {
				result[item.value()] = eq[j];
				++j;
			}
	}
	for (const auto &eq : edgeEqs) {
		int j = 0;
		for (const auto &item : callIdRange) {
			using P = Observation::CallEdge;
			if (auto it = std::ranges::find(eq, item, &P::first); it != eq.end()) {
				result[item.value()] = eq[j].first;
				result[it->second.value()] = eq[j].second;
				++j;
			}
		}
	}
	return result;
}

static auto isDataSymmetricOp(const auto &opsRange, const MethodCall &call) -> bool
{
	return call.argVal != 0 && call.retVal == 0 &&
	       std::ranges::count(opsRange, call.argVal, &MethodCall::retVal) == 1;
}

/*
 * This function transposes symmetric calls, i.e., either enqueues adding an item
 * that is never read, or dequeues returning bottom.
 *
 * NOTE: symmetric and DI calls need to be disjoint (see below)
 */
static auto calculateSymmetricCalls(const Observation &obs) -> CallEquivalencesSet
{
	CallEquivalencesSet result;

#ifdef __cpp_lib_ranges_zip
	for (const auto &[call1, call2] : obs.ops() | std::views::adjacent<2>) {
#else /* libc++ lags a bit in terms of ranges */
	for (const auto &id : obs.ids() | std::views::drop(1)) {
		const auto &call1 = obs.getCall(MethodCall::Id(id.value() - 1));
		const auto &call2 = obs.getCall(id);
#endif
		/* Discard non-symmetric calls */
		if (call1.name != call2.name || call1.retVal != call2.retVal)
			continue;
		/* Discard data independent calls */
		if (isDataSymmetricOp(obs.ops(), call1) || isDataSymmetricOp(obs.ops(), call2))
			continue;
		if (!result.empty() && result.back().back() == call1.id)
			result.back().push_back(call2.id);
		else
			result.push_back({call1.id, call2.id});
	}
	return result;
}

/*
 * This function calculates sets of data-independent calls.
 * For instance, if we have `enq(1) -> deq(1) -> enq(2) -> deq(2)`, we get
 * `enq(2) -> deq(2) -> enq(1) -> deq(1)` (which we wouldn't get with SR).
 */
static auto calculateDICalls(const Observation &obs) -> EdgeEquivalencesSet
{
	EdgeEquivalencesSet result;

	for (const auto &call : obs.ops()) {
		if (!isDataSymmetricOp(obs.ops(), call))
			continue;

		auto ops = obs.ops();
		auto it = std::ranges::find(ops, call.argVal, &MethodCall::retVal);
		auto edge = std::make_pair(call.id, it->id);
		if (!result.empty() && obs.getCall(result.back().back().first).name == call.name &&
		    obs.getCall(result.back().back().second).name == it->name)
			result.back().emplace_back(edge);
		else
			result.emplace_back(1, edge);
	}
	return result;
}

/* Returns true if any of V's elements can be permuted. Modifies V in place. */
template <class T> static auto existsPermutation(std::vector<std::vector<T>> &v) -> bool
{
	return std::ranges::any_of(
		v, [](auto &eq) { return std::ranges::next_permutation(eq).found; });
}

void Specification::add(ExecutionGraph &g, const ConsistencyChecker *consChecker,
			bool symmReduction)
{
	Observation obs(g, consChecker);
	auto lin = calculateLinearization(g, obs);
	if (!symmReduction) {
		lookupSyncOrInsert(obs).lins.insert(lin);
		return;
	}

	auto callEqs = calculateSymmetricCalls(obs);
	auto edgeEqs = calculateDICalls(obs);
	do {
		const auto perm = generatePermutation(obs.ids(), callEqs, edgeEqs);
		const auto newObs = obs.applyHbPermutation(perm);
		lookupSyncOrInsert(newObs).lins.insert(lin.applyPermutation(perm));
	} while (existsPermutation(callEqs) || existsPermutation(edgeEqs));
}

/*******************************************************************************
 **                     Hint Calculation Algorithm
 ******************************************************************************/

auto operator<<(llvm::raw_ostream &os, const Hint &hint) -> llvm::raw_ostream &
{
	for (auto const &e : hint.edges)
		printDotEdge(os, e);
	os << "\n";
	return os;
}

struct Extension {
private:
	/* Unlike Observation::CallIdx, Extensions use different indexes for
	 * MethodBegin and MethodEnd events so that we can correctly account
	 * for transitivity. */
	using ExtEvent = unsigned int;

public:
	struct ExtEdge {
		ExtEvent src;
		ExtEvent dst;
		llvm::BitVector contradictingLins; // whether the edge kill i-th lin

		ExtEdge(MethodCall::Id from, MethodCall::Id to)
			: src(toEndExtEvent(from)), dst(toBeginExtEvent(to))
		{}

		[[nodiscard]] auto operator==(const ExtEdge &other) const -> bool
		{
			return std::tie(this->src, this->dst) == std::tie(other.src, other.dst);
		}

		friend auto operator<<(llvm::raw_ostream &os, const ExtEdge &e)
			-> llvm::raw_ostream &
		{
			printDotEdge(os, e.src, e.dst);
			return os;
		}
	};

	explicit Extension(const Observation &obs) : hbext_(obs.getNumOps() * 2)
	{
		for (auto id : obs.ids())
			hbext_.addEdge(toBeginExtEvent(id), toEndExtEvent(id));
		for (const auto &edge : obs.hb())
			hbext_.addEdge(toBeginExtEvent(edge.first), toEndExtEvent(edge.second));
		hbext_.transClosure();
	}

	/* Number of added edges */
	[[nodiscard]] auto size() const -> size_t { return ext_.size(); }

	[[nodiscard]] auto getLastAddedEdge() const -> const ExtEdge *
	{
		return ext_.empty() ? nullptr : ext_.back();
	}

	/* Whether (a, b) \in EXT u HB  */
	[[nodiscard]] auto areHbOrdered(ExtEvent a, ExtEvent b) const -> bool
	{
		return hbext_(a, b);
	}

	/* Whether (a, b) \in (EXT u HB)? */
	[[nodiscard]] auto areHbOptOrdered(ExtEvent a, ExtEvent b) const -> bool
	{
		return a == b || areHbOrdered(a, b);
	}

	/* Whether (a, b) or (b, a) \in EXT u HB */
	[[nodiscard]] auto areConnected(const ExtEdge &e) const -> bool
	{
		return areHbOrdered(e.src, e.dst) || areHbOrdered(e.dst, e.src);
	}

	[[nodiscard]] auto isSubsetOf(const Extension &other) const -> bool
	{
		return std::ranges::all_of(
			ext_, [&](const ExtEdge *e) { return other.areHbOrdered(e->src, e->dst); });
	}

	/* Whether given edge increase transitive closure monotonically i.e. no edges added before
	 * become transitive (could be represented via composition of other extension edges) */
	[[nodiscard]] auto isMonotoneNext(const ExtEdge &newEdge) const -> bool
	{
		return std::ranges::none_of(ext_, [&](const ExtEdge *oldEdge) {
			return areHbOptOrdered(oldEdge->src, newEdge.src) &&
			       areHbOptOrdered(newEdge.dst, oldEdge->dst);
		});
	}

	[[nodiscard]] auto getNumKilledLins() const -> size_t { return killedLins_.count(); }

	[[nodiscard]] auto killsAllLins() const -> bool { return killedLins_.all(); }

	/* Whether the extension can kill all left lins assuming killableLins can be killed via
	 * edges later */
	[[nodiscard]] auto
	killsLeftoverLins(const llvm::BitVector &linsKillableViaFutureEdges) const
	{
		auto tmp = killedLins_; // copy
		tmp |= linsKillableViaFutureEdges;
		return tmp.all();
	}

	/* Grow extension via one edge */
	void addEdge(const ExtEdge &edge)
	{
		hbext_.addEdgeAndTransitive(edge.src, edge.dst);
		killedLins_ |= edge.contradictingLins;
		ext_.push_back(&edge);
	}

	/* Convert extension to Hint */
	[[nodiscard]] auto toHint() const -> Hint
	{
		Hint result;
		for (const auto *e : ext_) {
			result.edges.emplace_back(toCallIdx(e->src), toCallIdx(e->dst));
		}
		return result;
	}

private:
	[[nodiscard]] static auto toBeginExtEvent(MethodCall::Id id) -> ExtEvent
	{
		return 2 * id.value();
	}

	[[nodiscard]] static auto toEndExtEvent(MethodCall::Id id) -> ExtEvent
	{
		return toBeginExtEvent(id) + 1;
	}

	[[nodiscard]] static auto toCallIdx(ExtEvent extEvent) -> MethodCall::Id
	{
		return MethodCall::Id(extEvent / 2);
	}

	/* Extension edges */
	llvm::SmallVector<const ExtEdge *, 8> ext_;

	/* transitive closure of extended happens-before: (hb u ext)+ */
	Matrix2D<unsigned int> hbext_;

	/* cached U_{e in edges} e.contradictingLins */
	llvm::BitVector killedLins_;
};

/* Add extension to set of already calculated hints */
static void maybeAddExtension(std::vector<Extension> &extensions, Extension &&newExt)
{
	/* Discard newExt if a smaller extension already exists */
	if (std::ranges::any_of(extensions,
				[&](auto &oldExt) { return oldExt.isSubsetOf(newExt); }))
		return;

	/* Erase any larger extensions already generated */
	std::erase_if(extensions, [&](auto &oldExt) { return newExt.isSubsetOf(oldExt); });

	/* Add the new extension */
	extensions.emplace_back(std::move(newExt));
}

/* Precalculate pool of interesting edges that make sense do consider along with bitmask of
 * linearizations which the edge kills.
 * NB: order of edges the pool doesn't matter but should be fixed because extensions are
 * always constructed in that order to avoid repetitions. */
static auto calculateEdgePool(const Observation &obs, const VSet<Linearization> &lins,
			      const Extension &ext) -> std::vector<Extension::ExtEdge>
{
	std::vector<Extension::ExtEdge> result;
	for (auto id1 : obs.ids()) {
		for (auto id2 : obs.ids()) {
			auto edge = Extension::ExtEdge(id1, id2);

			if (ext.areConnected(edge))
				continue; // avoid edges that coincide or contradict HB

			for (const auto &lin : lins)
				edge.contradictingLins.push_back(lin.isBefore(id2, id1));

			result.push_back(std::move(edge));
		}
	}
	return result;
}

auto Specification::calculateObservationHints(const Observation &obs) const -> std::vector<Hint>
{
	std::queue<Extension> queue;

	queue.emplace(obs);
	const auto &lins = lookupSync(obs).lins;
	auto edgePool = calculateEdgePool(obs, lins, queue.front());

	std::vector<Extension> extensions;
	size_t extensionsExplored = 0;
	while (!queue.empty()) {
		auto ext = std::move(queue.front());
		queue.pop();

		// Accumulator of all lins could be killed in the future of `newExt` (only
		// edges that made progress took into account).
		llvm::BitVector linsKillableViaFutureEdges;

		/* For each edge `newEdge` consider extension (ext + newEdge) */
		// NB: Loop goes backward so at each iteration we have prediction for
		// current
		//     `newExt` since only edges of suffix[newEdgeIx+1..] could be added
		//     into `newExt` in future.
		auto begin = ext.size() == 0
				     ? edgePool.begin()
				     : ++std::ranges::find(edgePool, *ext.getLastAddedEdge());
		for (auto const &newEdge :
		     std::ranges::subrange(begin, edgePool.end()) | std::views::reverse) {
			/* Should we consider this edge? */
			if (ext.areConnected(newEdge))
				continue; // transitive or lead to cycle
			if (!ext.isMonotoneNext(newEdge))
				continue; // leads to duplication of exploration
			if (ext.size() + 1 > getMaxExtSize())
				continue; // hit user-defined threshold

			GENMC_DEBUG(++extensionsExplored;);
			Extension newExt(ext); // copy
			newExt.addEdge(newEdge);

			/* Is the new extension a hint? */
			if (newExt.killsAllLins()) {
				maybeAddExtension(extensions, std::move(newExt));
				continue; // no further progress due to monotonicity
			}

			if (newExt.getNumKilledLins() == ext.getNumKilledLins())
				continue; // no new killed linearizations

			linsKillableViaFutureEdges |= newEdge.contradictingLins;

			/* Check whether the extension can make progress in the end */
			if (newExt.killsLeftoverLins(linsKillableViaFutureEdges))
				queue.emplace(std::move(newExt));
		}
	}

	std::vector<Hint> result;
	result.reserve(extensions.size());
	for (const auto &ext : extensions)
		result.emplace_back(ext.toHint());

	GENMC_DEBUG(if (debug) llvm::dbgs()
			    << "Stats: #calls: " << obs.getNumOps()
			    << ", #hb:  " << std::ranges::distance(obs.hb()) << ", #lins: " << lins
			    << " => #extension: " << extensionsExplored
			    << ", #hints:  " << result.size() << "\n";);
	GENMC_DEBUG(extensionsExploredInTotal += extensionsExplored;);

	return result;
}

void Specification::calculateHints()
{
	for (const auto &obs : observations())
		addHints(obs, calculateObservationHints(obs));
}

/*******************************************************************************
 **                     Printing utilities
 ******************************************************************************/

/* Print vector in format that easy to parse */
template <typename VectorT, typename FT>
inline static void printlnVector(llvm::raw_ostream &os, const VectorT &v, FT printElem,
				 const std::string &sep = "   ", const std::string &sizeSep = "   ")
{
	os << v.size();
	if (!v.empty()) {
		os << sizeSep;
		auto it = v.begin();
		while (true) {
			printElem(*it);
			if (++it == v.end())
				break;
			os << sep;
		}
	}
	os << "\n";
}

void serialize(llvm::raw_ostream &os, const Specification &spec)
{
	auto printEdge = [&](auto const &edge) { os << edge.first << " " << edge.second; };
	auto printOpCall = [&os](auto &call) {
		os << call.name << " " << call.argVal << " " << call.retVal;
	};

	os << "# Specification\n";
	os << "# Statistics: "
	   << " Size: " << spec.data_.size() << " "
	   << " Hints: " << spec.getNumHints() << " "
	   << " Lins: " << spec.getNumLins() << "\n";
	os << "# Observations:\n";
	std::vector<Observation> v;
	std::ranges::copy(spec.observations(), std::back_inserter(v));
	std::ranges::sort(v, [](const Observation &a, const Observation &b) { return a < b; });
	for (const auto &obs : v) {
		os << "## Observation (Lins: " << spec.lookupSync(obs).lins.size() << "):\n";
		os << "## Outcome:\n";
		printlnVector(os, obs.ops_, printOpCall);
		os << "## RF:\n";
		printlnVector(os, obs.rfs_, printEdge);
		os << "## HB:\n";
		printlnVector(os, obs.hb_, printEdge);
		os << "## Hints:\n";
		printlnVector(
			os, spec.lookupSync(obs).hints,
			[&](auto const &hint) { printlnVector(os, hint.edges, printEdge); },
			/*sep=*/"", /*sizeSep=*/"\n");
	}
}

/* Read std::vector using lambda `readElem` */
template <typename T, typename FT>
inline static void readVector(std::istream &is, std::vector<T> &v, FT readElem)
{
	size_t v_size;
	if (!(is >> v_size))
		return;
	v.resize(v_size);
	for (auto i = 0U; i < v_size; ++i)
		readElem(v[i]);

	/* Using >> will leave trailing newlines.
	 * Consume those before calling getline */
	is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void skipComments(std::istream &is)
{
	static std::string comment;
	while (!is.eof() && !is.fail() && is.peek() == '#')
		std::getline(is, comment);
}

auto deserialize(std::istream &is) -> Specification
{
	auto readEdge = [&is](auto &edge) { is >> edge.first >> edge.second; };
	auto readIdEdge = [&is](auto &edge) {
		std::pair<unsigned, unsigned> tmp;
		is >> tmp.first >> tmp.second;
		edge.first = MethodCall::Id(tmp.first);
		edge.second = MethodCall::Id(tmp.second);
	};
	auto readCall = [&is](auto &call) { is >> call.name >> call.argVal >> call.retVal; };

	Specification result;

	auto i = 0U;
	while (!is.eof()) {
		++i;
		Observation obs;

		std::vector<MethodCall> ops; // ### Outcome
		skipComments(is);
		readVector(is, ops, readCall);
		obs.ops_ = std::move(ops);
		for (auto j = 0U; j < obs.ops_.size(); j++)
			obs.ops_[j].id = MethodCall::Id(j);

		std::vector<std::pair<int, int>> rf; // ### RF
		skipComments(is);
		readVector(is, rf, readEdge);
		obs.rfs_ = std::move(rf);

		std::vector<Observation::CallEdge> hb; // ### HB
		skipComments(is);
		readVector(is, hb, readIdEdge);
		obs.hb_ = std::move(hb);

		std::vector<Hint> hints; // #### Hints
		skipComments(is);
		readVector(is, hints, [&](auto &hint) { readVector(is, hint.edges, readIdEdge); });
		result.addHints(obs, std::move(hints));

		/* empty newline between outcomes */
		is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	return result;
}
