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

#ifndef __GRAPH_MANAGER_HPP__
#define __GRAPH_MANAGER_HPP__

#include "AdjList.hpp"
#include "Calculator.hpp"
#include "ExecutionGraph.hpp"
#include <unordered_map>

class CoherenceCalculator;
class LBCalculatorLAPOR;
class PSCCalculator;

/*******************************************************************************
 **                           GraphManager Class
 ******************************************************************************/

/*
 * A simple class that manages an execution graph and relations on said graph.
 * Instances of this class should be created via the GraphBuilder class.
 * It is instantiated differently depending on the memory model, and is
 * largely responsible for performing the consistency checks. The relations
 * needed to be calculated may entail the initiation of one or more fixpoints.
 */
class GraphManager {

	void changeStoreOffset(const llvm::GenericValue *addr,
			       Event s, int newOffset);

	std::vector<std::pair<Event, Event> >
	saveCoherenceStatus(const std::vector<std::unique_ptr<EventLabel> > &prefix,
			    const ReadLabel *rLab) const;

	void cutToLabel(const EventLabel *rLab);

	void restoreStorePrefix(const ReadLabel *rLab,
				std::vector<std::unique_ptr<EventLabel> > &storePrefix,
				std::vector<std::pair<Event, Event> > &moPlacings);

};

#include "CoherenceCalculator.hpp"
#include "LBCalculatorLAPOR.hpp"
// #include "PSCCalculator.hpp"

#endif /* __GRAPH_MANAGER_HPP__ */
