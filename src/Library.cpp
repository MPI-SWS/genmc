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

#include "Error.hpp"
#include "Library.hpp"
#include <llvm/Support/Debug.h>
#include <algorithm>

/************************************************************
 ** Library constructors
 ***********************************************************/

Library::Library(std::string name, LibType typ)
	: name(name), typ(typ), functionalRfs(false),
	  coherence(false) {}


/************************************************************
 ** Basic getters/setters
 ***********************************************************/

std::string Library::getName() const { return name; }

LibType Library::getType() const { return typ; }

const std::vector<LibMem> &Library::getMembers() const { return mems; }

const std::vector<Relation> &Library::getRelations() const { return relations; }

const std::vector<Constraint> &Library::getConstraints() const { return constraints; }

llvm::AtomicOrdering strToOrd(std::string &ord)
{
	if (ord == "rlx") {
		return llvm::AtomicOrdering::Monotonic;
	} else if (ord == "acq") {
		return llvm::AtomicOrdering::Acquire;
	} else if (ord == "rel") {
		return llvm::AtomicOrdering::Release;
	} else {
		WARN("Wrong ordering to parse!\n");
		BUG();
	}
}

void Library::addMember(std::string name, std::string typ, std::string ord)
{
	if (typ == "read")
		mems.push_back(LibMem(name, LibMem::LM_Read,
				      strToOrd(ord), false));
	else if (typ == "write")
		mems.push_back(LibMem(name, LibMem::LM_Write,
				      strToOrd(ord), false));
	else if (typ == "init")
		mems.push_back(LibMem(name, LibMem::LM_Write,
				      strToOrd(ord), true));
	else
		WARN("Erroneous library member type in specs!\n");
}

bool Library::hasMember(const std::string &name) const
{
	return std::any_of(mems.begin(), mems.end(),
			   [&](const LibMem &mem){ return mem.getName() == name; });
}

const LibMem *Library::getMember(const std::string &name) const
{
	for (auto &m : getMembers())
		  if (m.getName() == name)
			  return &m;
	return nullptr;
}

/* Given a collection of libraries, returns a pointer to the library that contains
 * the given name as a member, if there is any */
const Library *Library::getLibByMemberName(const std::vector<Library> &libs,
				     const std::string &functionName)
{
	for (auto &l : libs)
		if (l.hasMember(functionName))
			return &l;
	return nullptr;
}

void Library::addRelation(std::string name)
{
	relations.push_back(Relation(name));
}

void Library::makeRelationTransitive(std::string relation)
{
	for (auto &r : relations)
		if (r.getName() == relation)
			r.makeTransitive();
}

void Library::addStepToRelation(std::string relation, std::vector<std::string> preds)
{
	for (auto &r : relations)
		if (r.getName() == relation)
			r.addStep(preds);

	/* Record whether this library tracks coherence */
	if (std::any_of(preds.begin(), preds.end(), [](std::string &s)
			{ return s == "mo"; }))
		markCoherenceTrack();
}

void Library::addConstraint(std::string name)
{
	constraints.push_back(Constraint(name));
}

/************************************************************
 ** Printing utilities
 ***********************************************************/

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Library &l)
{
	s << "Library: " << l.name << "\t\nMembers:\n";
	for (auto &m : l.mems)
		s << "\t\t" << m.getName() << "\n";
	return s;
}
