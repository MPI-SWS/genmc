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

#include "Verification/Relinche/LinearizabilityChecker.hpp"
#include "ExecutionGraph/Consistency/ConsistencyChecker.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "Verification/Relinche/Observation.hpp"

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

using ExtEdge = std::pair<MethodEndLabel *, MethodBeginLabel *>;

class LinearizabilityStructureError : public LinearizabilityError {

	auto toString() const -> std::string override
	{
		return "The library implementation returns incorrect return values\n"
		       "(forbidden by the supplied specification).";
	}
};

class LinearizabilitySyncError : public LinearizabilityError {

public:
	explicit LinearizabilitySyncError(std::vector<std::vector<Edge>> &&m)
		: missingSyncs_(std::move(m))
	{
		std::ranges::for_each(missingSyncs_, std::ranges::sort);
	}

	auto toString() const -> std::string override
	{
		std::string str;
		auto s = llvm::raw_string_ostream(str);
		s << "The library implementation does not induce all the synchronization\n"
		  << "required by the specification. Missing synchronizations:\n";
		for (const auto &edges : missingSyncs_) {
			for (const auto &edge : edges)
				s << edge.first << "->" << edge.second << " ";
			s << "\n";
		}
		return s.str();
	}

private:
	std::vector<std::vector<Edge>> missingSyncs_;
};

class LinearizabilityExtensionError : public LinearizabilityError {
public:
	using KCEdge = std::pair<MethodCall::ThreadKindAndCopyIx, MethodCall::ThreadKindAndCopyIx>;

	explicit LinearizabilityExtensionError(const std::vector<ExtEdge> &extensionEdges,
					       std::vector<KCEdge> &&kcEdges)
		: kcEdges_(kcEdges)
	{
		extensionEdges_.reserve(extensionEdges.size());
		for (const auto &[a, b] : extensionEdges)
			extensionEdges_.emplace_back(a->getPos(), b->getPos());
		std::ranges::sort(extensionEdges_);
		std::ranges::sort(kcEdges_);
	}

	auto toString() const -> std::string override
	{
		std::string str;
		auto s = llvm::raw_string_ostream(str);
		s << "The library implementation returns incorrect return values\n"
		  << "under the following extended client:\n";
		for (const auto &edge : extensionEdges_) {
			s << edge.first << "->" << edge.second << " ";
		}
		s << "\nTo run the extended client add --max-hint-size=0 and following "
		     "C-flags after '--' :\n"
		  << "-DGENERATE_SYNC ";
		int i = 0;
		for (const auto &[a, b] : kcEdges_) {
			++i;
			s << "-DFROM" << i << "_KIND_IX=" << a.first << " "
			  << "-DFROM" << i << "_COPY_IX=" << a.second << " "
			  << "-DTO" << i << "_KIND_IX=" << b.first << " "
			  << "-DTO" << i << "_COPY_IX=" << b.second << " ";
		}
		s << "\n";
		return s.str();
	}

private:
	std::vector<Edge> extensionEdges_;
	std::vector<KCEdge> kcEdges_;
};

static auto combineHints(const Specification &spec, const Observation &obs) -> std::vector<Hint>
{
	auto isSymmetricEdge = [&](const auto &edge) {
		return (obs.getCall(edge.first).thdKC.first ==
				obs.getCall(edge.second).thdKC.first &&
			obs.getCall(edge.first).thdKC.second >
				obs.getCall(edge.second).thdKC.second);
	};
	auto isNotSymmetricHint = [&](const Hint &hint) {
		return std::ranges::none_of(hint.edges, isSymmetricEdge);
	};

	/* First, collect the hints of all refining observations */
	std::vector<std::vector<Hint>> hintsOfRefining;
	for (const auto &kv : spec.refined_observations(obs)) {
		std::vector<Hint> hs;
		std::ranges::copy_if(kv.second.hints, std::back_inserter(hs), isNotSymmetricHint);
		hintsOfRefining.emplace_back(std::move(hs));
	}

	/* Fastpath: If no extensions in the cross-product, the candidate cannot be killed */
	if (std::ranges::any_of(hintsOfRefining, [&](auto &hints) { return hints.empty(); }))
		return {};

	/* Fastpath: There is a single candidate, cross-product is trivial */
	if (hintsOfRefining.size() == 1)
		return hintsOfRefining[0];

	/* Slowpath: calculate cross-product using a vector clock */
	std::vector<int> clock(hintsOfRefining.size() + 1, 0);
	std::vector<Hint> result;
	while (clock.back() < 1) {
		std::vector<Observation::CallEdge> edges;
		for (auto i = 0U; i < hintsOfRefining.size(); ++i) {
			auto current = hintsOfRefining[i][clock[i]];
			edges.insert(edges.end(), current.edges.begin(), current.edges.end());
		}
		result.push_back({.edges = edges});

		/* Increment the vector clock */
		++clock[0];
		for (auto i = 0U; i < hintsOfRefining.size(); ++i) {
			if (clock[i] == hintsOfRefining[i].size()) {
				clock[i] = 0;
				++clock[i + 1];
			}
		}
	}
	return result;
}

static auto checkHint(ExecutionGraph &graph, const ConsistencyChecker *consChecker,
		      const Observation &obs, const Hint &hint)
	-> std::unique_ptr<LinearizabilityError>
{
	// map hint's edges to implementation graph
	std::vector<ExtEdge> edges;
	edges.reserve(hint.edges.size());
	for (const auto &[op1, op2] : hint.edges)
		edges.emplace_back(obs.getCall(op1).endLab, obs.getCall(op2).beginLab);

	for (const auto &[endLab, begLab] : edges)
		endLab->addSucc(begLab);

	if (consChecker->isCoherentRelinche(graph) && consChecker->isConsistent(graph)) {
		std::vector<LinearizabilityExtensionError::KCEdge> kcEdges;
		kcEdges.reserve(hint.edges.size());
		for (const auto &[op1, op2] : hint.edges)
			kcEdges.emplace_back(obs.getCall(op1).thdKC, obs.getCall(op2).thdKC);
		return std::make_unique<LinearizabilityExtensionError>(edges, std::move(kcEdges));
	}

	// restore execution graph
	for (const auto &[endLab, begLab] : edges)
		endLab->removeSucc([](auto * /*lab*/) { return true; });

	return nullptr;
}

auto LinearizabilityChecker::refinesSpec(ExecutionGraph &graph) -> Result
{
	/* Phase 1: Check refinement
	 * 1.1. Check return values are correct */
	const Observation implObsGraph(graph, consChecker);
	Result result;

	if (!specification->containsSameOutput(implObsGraph)) {
		result.status = std::make_unique<LinearizabilityStructureError>();
		return result;
	}

	/* 1.2 Check synchronization */
	if (!specification->isRefinedBy(implObsGraph)) {
		auto missedSyncs = specification->refinementMissingEdges(implObsGraph);
		result.status = std::make_unique<LinearizabilitySyncError>(std::move(missedSyncs));
		return result;
	}

	/* Phase 2: Check extensions */
	auto hints = combineHints(*specification, implObsGraph);

	/* Check whether adding the hints back to the original creates a violation */
	for (const Hint &hint : hints) {
		++result.hintsChecked;
		result.status = checkHint(graph, consChecker, implObsGraph, hint);
		if (result.status)
			break;
	}
	return result;
}
