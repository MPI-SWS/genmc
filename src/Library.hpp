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

#ifndef __LIBRARY_HPP__
#define __LIBRARY_HPP__

#include "Event.hpp"
#include <llvm/IR/Instructions.h>
#include <vector>
#include <string>

/* A library is either taken for granted, or needs to be verified */
enum LibType { Granted, ToVerify };

/* Each library has some function that make the library up */
struct LibMem {
	std::string name;
	EventType typ;
	llvm::AtomicOrdering ord;
	bool initial;

	LibMem(std::string name, EventType typ, llvm::AtomicOrdering ord, bool isLibInit) :
		name(name), typ(typ), ord(ord), initial(isLibInit) {};

	std::string getName() { return name; };
	EventType getType()   { return typ; };
	llvm::AtomicOrdering getOrdering() { return ord; };
	bool isLibInit()         { return initial; };
	bool hasReadSemantics()  { return typ == ERead; };
	bool hasWriteSemantics() { return typ == EWrite; };
};

struct Relation {
	bool transitive;
	std::string name;
	std::vector<std::vector<std::string> > steps;

	Relation(std::string name) : transitive(false), name(name) {};

	bool isTransitive()     { return transitive; };
	std::string getName() { return name; };
	std::vector<std::vector<std::string> > &getSteps() { return steps; };

	void addStep(std::vector<std::string> step) { steps.push_back(step); };
	void makeTransitive()    { transitive = true; };
};

struct Constraint {
	std::string name;

	Constraint(std::string name) : name(name) {};

	std::string getName() { return name; };
};

/* Main class for libraries */
class Library {

private:
	std::string name;
	LibType typ;
	std::vector<LibMem> mems;
	std::vector<Relation> relations;
	std::vector<Constraint> constraints;
	bool functionalRfs;
	bool coherence;

public:
	Library(std::string name, LibType typ);

	std::string getName();
	LibType getType();
	std::vector<LibMem> &getMembers();
	std::vector<Relation> &getRelations();
	std::vector<Constraint> &getConstraints();
	bool hasFunctionalRfs() { return functionalRfs; };
	bool tracksCoherence()  { return coherence; };
	void addMember(std::string name, std::string typ, std::string ord);
	bool hasMember(std::string &name);
	LibMem *getMember(std::string &name);
	void addRelation(std::string name);
	void makeRelationTransitive(std::string name);
	void addStepToRelation(std::string relation, std::vector<std::string> substeps);
	void addConstraint(std::string name);
	void markFunctionalRfs()  { functionalRfs = true; };
	void markCoherenceTrack() { coherence = true; };

	static Library *getLibByMemberName(std::vector<Library> &libs, std::string &functionName);

	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Library &l);
};

#endif /* __LIBRARY_HPP__ */
