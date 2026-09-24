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

#include "GraphPrinting.hpp"
#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/ExecutionState.hpp"
#include "genmc/Execution/LabelVisitor.hpp"
#include "genmc/Support/Cast.hpp"
#include "genmc/Support/DotPrint.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/Parser.hpp"
#include "genmc/Support/ThreadInfo.hpp"
#include "genmc/Verification/GenMCDriver.hpp"
#include "genmc/Verification/Relinche/Observation.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <string>
#include <system_error>
#include <utility>

void printTraceBefore(const GenMCDriver::GraphDbgInfo &dbgInfo, const EventLabel *lab,
		      std::ostream &s /* = std::cerr */)
{
	if (dbgInfo.empty())
		return;

	const auto &g = *lab->getParent();

	s << std::format("Trace to {}:\n", lab->getPos());
	for (const auto &oLab : g.cb_preds(lab)) {
		/* Do not print the line if it is an RMW write, since it will be
		 * the same as the previous one */
		if (genmc::isa<CasWriteLabel>(&oLab) || genmc::isa<FaiWriteLabel>(&oLab))
			continue;
		/* Similarly for a Wna just after the creation of a thread
		 * (it is the store of the PID) */
		if (const auto *pLab = g.po_imm_pred(&oLab);
		    genmc::isa_and_present<ThreadCreateLabel>(pLab))
			continue;

		/* Skip if we don't have anything to print */
		if (!dbgInfo.contains(oLab.getPos()))
			continue;

		const auto &info = dbgInfo.at(oLab.getPos());
		if (!info.functionName.empty())
			s << "[" << info.functionName << "] ";
		s << info.file << ": " << info.line << ": " << info.source << "\n";
	}
}

static void executeMDPrint(const EventLabel * /*lab*/, const GenMCDriver::EventDbgInfo &dbg,
			   std::ostream &os = std::cout)
{
	std::string errPath = dbg.file;
	genmc::extractFilename(errPath);
	os << " " << errPath << ":" << dbg.line;
}

/* Returns true if the corresponding LOC should be printed for this label type */
static auto shouldPrintLOC(const EventLabel *lab) -> bool
{
	/* Begin/End labels don't have a corresponding LOC */
	if (genmc::isa<ThreadStartLabel>(lab) || genmc::isa<ThreadFinishLabel>(lab))
		return false;

	/* Similarly for allocations that don't come from malloc() */
	if (const auto *mLab = genmc::dyn_cast<MallocLabel>(lab))
		return mLab->getAddr().isHeap() && !mLab->getAddr().isInternal();
	return true;
}

static auto printVarName(const MemAccessLabel &lab, const GenMCDriver::GraphDbgInfo &dbgInfo)
	-> std::string
{
	const auto &g = *lab.getParent();
	if (!lab.getAddr().isStatic() && !g.getState().isAllocated(lab.getAddr()))
		return "???";
	return dbgInfo.contains(lab.getPos()) ? dbgInfo.at(lab.getPos()).accessedVarName : "";
}

/* Returns "<parent, id>", followed by the thread's name (if any) */
static auto printThreadName(const ThreadInfo &info) -> std::string
{
	auto ids = std::format("<{}, {}>", info.parentId, info.id);
	return info.name.empty() ? ids : std::format("{} {}", ids, info.name);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void printGraph(const ExecutionGraph &g,
		const GenMCDriver::GraphDbgInfo
			&dbgInfo, // NOLINT(readability-function-cognitive-complexity)
		std::ostream &s /* = std::cerr */)
{
	LabelPrinter printer(
		[&dbgInfo](const MemAccessLabel &lab) { return printVarName(lab, dbgInfo); },
		[](const ReadLabel &lab) {
			return lab.getRf() ? lab.getAccessValue(lab.getAccess()) : SVal();
		});

	/* Print the graph */
	for (auto i = 0; i < g.getNumThreads(); i++) {
		const auto &thrInfo = g.getFirstThreadLabel(i)->getThreadInfo();
		s << printThreadName(thrInfo);
		if (const auto *bLab = g.getFirstThreadLabel(i)) {
			auto symm = bLab->getSymmPredTid();
			if (symm != -1)
				s << " symmetric with " << symm;
		}
		s << ":\n";
		for (const auto &lab : g.po(i)) {
			if (genmc::isa<ThreadStartLabel>(&lab))
				continue;
			s << "\t" << printer.toString(lab);
			if (dbgInfo.contains(lab.getPos()) && shouldPrintLOC(&lab))
				executeMDPrint(&lab, dbgInfo.at(lab.getPos()), s);
			s << "\n";
		}
	}

	/* MO: Print coherence information */
	auto header = false;
	for (auto locIt = g.loc_begin(), locE = g.loc_end(); locIt != locE; ++locIt) {
		/* Skip empty and single-store locations */
		if (g.hasLocMoreThanOneStore(locIt->first)) {
			if (!header) {
				s << "Coherence:\n";
				header = true;
			}
			const auto *wLab = &*std::ranges::begin(g.co(locIt->first));
			s << printVarName(*wLab, dbgInfo) << ": [ ";
			for (const auto &w : g.co(locIt->first))
				s << std::format("{} ", w);
			s << "]\n";
		}
	}
	s << "\n";
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void dotPrintToFile(const std::string &filename,
		    EventLabel *errLab, // NOLINT(readability-function-cognitive-complexity)
		    std::unique_ptr<VectorClock> errView, const EventLabel *confLab,
		    std::unique_ptr<VectorClock> confView, const ConsistencyChecker *checker,
		    const GenMCDriver::GraphDbgInfo &dbgInfo, bool printObservation)
{
	auto &g = *errLab->getParent();

	std::ofstream ss(filename);
	if (!ss) {
		const std::error_code ec = std::make_error_code(std::io_errc::stream);
		handleFSError(ec, "Failed to open dot file " + filename);
	}
	DotPrinter printer(
		[&dbgInfo](const MemAccessLabel &lab) { return printVarName(lab, dbgInfo); },
		[](const ReadLabel &lab) {
			return lab.getRf() ? lab.getAccessValue(lab.getAccess()) : SVal();
		});

	std::unique_ptr<VectorClock> before;
	if (errView)
		before = std::move(errView);
	else
		before = g.getViewFromStamp(g.getMaxStamp());
	if (confLab)
		before->update(*confView);

	/* Create a directed graph */
	ss << "strict digraph {\n";
	/* Specify node shape */
	ss << "node [shape=plaintext]\n";
	/* Left-justify labels for clusters */
	ss << "labeljust=l\n";
	/* Draw straight lines */
	ss << "splines=false\n";

	/* Print all nodes with each thread represented by a cluster */
	for (auto i = 0; i < before->size(); i++) {
		bool inMethod = false;
		const auto &tInfo = g.getFirstThreadLabel(i)->getThreadInfo();
		ss << "subgraph cluster_" << i << "{\n";
		ss << "\tlabel=\"" << printThreadName(tInfo) << "\"\n";
		ss << "\ttooltip=\"thread #" << i << "\"\n";
		for (auto j = 1; j <= before->getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			if (printObservation) {
				if (genmc::isa<MethodBeginLabel>(lab))
					inMethod = true;
				else if (genmc::isa<MethodEndLabel>(lab))
					inMethod = false;
				else if (inMethod)
					continue;
			}
			ss << std::format("\t\"{}\" [label=<", lab->getPos());

			/* First, print the graph label for this node */
			ss << printer.toString(*lab);

			/* And then, print the corresponding line number */
			if (dbgInfo.contains(lab->getPos()) && shouldPrintLOC(lab)) {
				ss << " <FONT COLOR=\"gray\">";
				executeMDPrint(lab, dbgInfo.at(lab->getPos()), ss);
				ss << "</FONT>";
			}
			ss << ">";

			if (errLab && lab->getPos() == errLab->getPos())
				ss << ", style=filled, fillcolor=yellow";
			if (confLab && lab->getPos() == confLab->getPos())
				ss << ", style=filled, fillcolor=yellow";

			ss << std::format(", tooltip=\"{}\"]\n", lab->getPos());
		}
		ss << "}\n";
	}

	/* Print relations between events (po U rf) */
	for (auto i = 0; i < before->size(); i++) {
		bool inMethod = false;
		const EventLabel *lastLab = nullptr;
		for (auto j = 0; j <= before->getMax(i); j++) {
			auto *lab = g.getEventLabel(Event(i, j));

			if (printObservation) {
				if (genmc::isa<MethodBeginLabel>(lab))
					inMethod = true;
				else if (genmc::isa<MethodEndLabel>(lab))
					inMethod = false;
				else if (inMethod)
					continue;
			}

			/* Print a po-edge, but skip dummy start events for
			 * all threads except for the first one */
			if (lastLab)
				printlnDotEdge(ss, lastLab->getPos(), lab->getPos());
			if (!genmc::isa<ThreadStartLabel>(lab))
				lastLab = lab;

			if (auto *rLab = genmc::dyn_cast<ReadLabel>(lab)) {
				/* Do not print RFs from INIT, BOTTOM, and same thread */
				if (genmc::dyn_cast_if_present<WriteLabel>(rLab->getRf()) &&
				    rLab->getRf()->getThread() != lab->getThread()) {
					printlnDotEdge(
						ss, rLab->getRf()->getPos(), rLab->getPos(),
						{{"color", "green"}, {"constraint", "false"}});
				}
			}
			if (auto *bLab = genmc::dyn_cast<ThreadStartLabel>(lab)) {
				if (i == 0)
					continue;
				printlnDotEdge(ss, bLab->getCreate()->getPos(),
					       bLab->getPos().next(),
					       {{"color", "blue"}, {"constraint", "false"}});
			}
			if (auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(lab))
				printlnDotEdge(ss,
					       g.getLastThreadLabel(jLab->getChildId())->getPos(),
					       jLab->getPos(),
					       {{"color", "blue"}, {"constraint", "false"}});

			// print extension edges
			for (const auto &begLab : g.lin_succs(lab))
				printlnDotEdge(ss, lab->getPos(), begLab.getPos(),
					       {{"color", "red"}, {"constraint", "false"}});
		}
	}

	if (printObservation) {
		Observation obs(g, checker);

		for (const auto &[op1, op2] : obs.hb()) {
			auto src = obs.getCall(op1).beginLab->getPos();
			auto dst = obs.getCall(op2).endLab->getPos();
			if (src.thread == dst.thread)
				continue;
			printlnDotEdge(ss, src, dst, {{"color", "blue"}, {"constraint", "false"}});
		}
	}

	ss << "}\n";
}
