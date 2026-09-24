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

#ifndef GENMC_GRAPH_PRINTING_HPP
#define GENMC_GRAPH_PRINTING_HPP

#include "genmc/Verification/GenMCDriver.hpp"

#include <iostream>
#include <memory>
#include <string>

class ConsistencyChecker;
class EventLabel;
class ExecutionGraph;
class VectorClock;

/** Pretty-prints an execution graph according to the available debugging info */
void printGraph(const ExecutionGraph &g, const GenMCDriver::GraphDbgInfo &dbgInfo,
		std::ostream &s = std::cerr);

/** Uses available debugging information to print the trace of source-code
 * instructions up to LAB */
void printTraceBefore(const GenMCDriver::GraphDbgInfo &dbgInfo, const EventLabel *lab,
		      std::ostream &s = std::cerr);

/** Outputs the current graph into a file (DOT format),
 * and visually marks events e and c (conflicting).
 * Assumes debugging information have already been collected  */
void dotPrintToFile(const std::string &filename, EventLabel *errLab,
		    std::unique_ptr<VectorClock> errView, const EventLabel *confLab,
		    std::unique_ptr<VectorClock> confView, const ConsistencyChecker *checker,
		    const GenMCDriver::GraphDbgInfo &dbgInfo, bool printObservation);

#endif /* GENMC_GRAPH_PRINTING_HPP */
