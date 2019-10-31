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

#ifndef __DEP_EXECUTION_GRAPH_HPP__
#define __DEP_EXECUTION_GRAPH_HPP__

#include "ExecutionGraph.hpp"

/*******************************************************************************
 **                        DepExecutionGraph Class
 ******************************************************************************/

/*
 * A specialized type of graph that also tracks dependencies. It also takes
 * these dependencies into account when restricting the graph, or when
 * calculating the prefix of an event to save.
 */
class DepExecutionGraph : public ExecutionGraph {

protected:
	/* Constructor should only be called from the builder */
	friend class GraphBuilder;
	DepExecutionGraph() : ExecutionGraph() {}

public:
	std::vector<Event> getRevisitable(const WriteLabel *sLab) const override;

	std::unique_ptr<VectorClock>
	getRevisitView(const ReadLabel *rLab, const WriteLabel *wLab) const override;

	const VectorClock& getPrefixView(Event e) const override;

	std::unique_ptr<VectorClock> getPredsView(Event e) const override;

	bool revisitModifiesGraph(const ReadLabel *rLab,
				  const EventLabel *sLab) const override;

	std::vector<std::unique_ptr<EventLabel> >
	getPrefixLabelsNotBefore(const EventLabel *sLab,
				 const ReadLabel *rLab) const override;

	void cutToStamp(unsigned int st) override;
};

#endif /* __DEP_EXECUTION_GRAPH_HPP__ */
