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

#ifndef __GRAPH_BUILDER_HPP__
#define __GRAPH_BUILDER_HPP__

#include "config.h"
#include "DepExecutionGraph.hpp"
#include "LBCalculatorLAPOR.hpp"
#include "MOCalculator.hpp"
#include "WBCalculator.hpp"
#include "PersistencyChecker.hpp"

/*******************************************************************************
 **                           GraphBuilder Class
 ******************************************************************************/

/*
 * A class that builds a graph object to be handed over to the
 * driver. This class abstracts away all the functionality that is
 * common across different driver instances (and only dependent on the
 * given configuration options), like creating the execution graph and
 * some basic relations on the graph (e.g., coherence).
 */
class GraphBuilder {

public:
	/* Create an execution graph of the provided type */
	GraphBuilder(bool shouldTrackDeps, unsigned int warnOnGraphSize = UINT_MAX) {
                tracksDeps = shouldTrackDeps;
		if (tracksDeps)
			graph = std::unique_ptr<DepExecutionGraph>(new DepExecutionGraph(warnOnGraphSize));
		else
			graph = std::unique_ptr<ExecutionGraph>(new ExecutionGraph(warnOnGraphSize));
	};

	GraphBuilder &withCoherenceType(CoherenceType co) {
		switch (co) {
		case CoherenceType::mo:
			graph->addCalculator(
				LLVM_MAKE_UNIQUE<MOCalculator>(*graph, tracksDeps),
				ExecutionGraph::RelationId::co, true, true);
			break;
		case CoherenceType::wb:
			graph->addCalculator(
				LLVM_MAKE_UNIQUE<WBCalculator>(*graph, tracksDeps),
				ExecutionGraph::RelationId::co, true, true);
			break;
		default:
			WARN("Unhandled coherence type!\n");
			BUG();
		}
		return *this;
	}

	GraphBuilder &withEnabledLAPOR(bool lapor) {
		if (lapor) {
			graph->addCalculator(
				LLVM_MAKE_UNIQUE<LBCalculatorLAPOR>(*graph),
				ExecutionGraph::RelationId::lb, true, true);
		}
		return *this;
	}

	GraphBuilder &withEnabledPersevere(bool pers, unsigned int blockSize) {
		if (pers) {
			graph->addPersistencyChecker(
				LLVM_MAKE_UNIQUE<PersistencyChecker>(*graph, blockSize));
		}
		return *this;
	}

	std::unique_ptr<ExecutionGraph> build() {
		BUG_ON(!graph->getCoherenceCalculator());
		return std::move(graph);
	};

private:
	/* Keep the type of graph constructed */
        bool tracksDeps = false;

	/* The graph to be constructed */
	std::unique_ptr<ExecutionGraph> graph = nullptr;
};

#endif /* __GRAPH_BUILDER_HPP__ */
