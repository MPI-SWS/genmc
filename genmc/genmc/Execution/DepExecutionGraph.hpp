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

#ifndef GENMC_DEP_EXECUTION_GRAPH_HPP
#define GENMC_DEP_EXECUTION_GRAPH_HPP

#include "genmc/ADT/DepView.hpp"
#include "ExecutionGraph.hpp"

/*******************************************************************************
 **                        DepExecutionGraph Class
 ******************************************************************************/

/**
 * A specialized type of graph that also tracks dependencies. It also takes
 * these dependencies into account when restricting the graph, or when
 * calculating the prefix of an event to save.
 */
class DepExecutionGraph : public ExecutionGraph {

public:
	DepExecutionGraph(ExecutionGraph::Config cfg) : ExecutionGraph(std::move(cfg))
	{
		getEventLabel(Event::getInit())->setPrefixView(std::make_unique<DepView>());
	}

	std::unique_ptr<VectorClock> getViewFromStamp(Stamp) const override;

	void cutToStamp(Stamp st) override;

	std::unique_ptr<ExecutionGraph> getCopyUpTo(const VectorClock &v) const override;
};

#endif /* GENMC_DEP_EXECUTION_GRAPH_HPP */
