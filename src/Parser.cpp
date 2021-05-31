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

#include "Library.hpp"
#include "Parser.hpp"
#include <cstdio>

string Parser::readFile(const string &fileName)
{
	ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);
	/* TODO: Error check here? */
	ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);
	/* TODO: Does tellg work on all platforms? */
	vector<char> bytes(fileSize);
	ifs.read(&bytes[0], fileSize);

	return string(&bytes[0], fileSize);
}

std::string Parser::getFileLineByNumber(const std::string &absPath, int line)
{
	std::ifstream ifs(absPath);
	std::string s;
	int curLine = 0;

	while (ifs.good() && curLine < line) {
		std::getline(ifs, s);
		++curLine;
	}
	return s;
}

void Parser::stripWhitespace(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void Parser::stripSlashes(std::string &absPath)
{
	auto i = absPath.find_last_of('/');
	if (i != std::string::npos)
		absPath = absPath.substr(i + 1);
}

void Parser::parseInstFromMData(std::pair<int, std::string> &locAndFile,
				std::string functionName,
				llvm::raw_ostream &os /* llvm::outs() */)
{
	int line = locAndFile.first;
	std::string absPath = locAndFile.second;

	/* If line is default-valued or malformed, skip... */
	if (line <= 0)
		return;

	auto s = getFileLineByNumber(absPath, line);
	stripWhitespace(s);
	stripSlashes(absPath);

	if (functionName != "")
		os << "[" << functionName << "] ";
	os << absPath << ": " << line << ": ";
	os << s << "\n";
}

std::vector<Library> Parser::parseSpecs(const string &fileName)
{
	FILE *file;
	std::vector<Library> result;
	char name[256], rel1[256], rel2[256], rel3[256], rel4[256];
	char line[256], tmp[256];
	char type[256], ord[256];
	int num, mems, rels, steps;

	/* Open specs file for reading */
	if ((file = fopen(fileName.c_str(), "r")) == NULL)
		goto cleanup;

	/* Read the number libraries taken for granted */
	if (fscanf(file, "GRANTED: %d\n", &num) != 1)
		goto cleanup;

	/* Parse the specs for each of them */
	for (auto i = 0; i < num; i++) {
		auto number = 0;
		/* Get the library's name and create an object */
		if ((number = fscanf(file, "NAME: %s\n", name)) != 1)
			goto cleanup;
		Library lib = Library(std::string(name), Granted);

		/* Read all the member-functions of the library */
		if (fscanf(file, "MEMBERS: %d\n", &mems) != 1)
			goto cleanup;
		for (auto j = 0; j < mems; j++) {
			if (fscanf(file, "%[^_]_%s = %s\n", type, ord, name) != 3)
				goto cleanup;
			lib.addMember(std::string(name), std::string(type), std::string(ord));
		}

		/* Read the specs for this library */
		if (fscanf(file, "RELATIONS: %d\n", &rels) != 1)
			goto cleanup;
		for (auto j = 0; j < rels; j++) {
			if (fscanf(file, "NAME: %s\n", name) != 1)
				goto cleanup;
			lib.addRelation(std::string(name));
			if (fscanf(file, "STEPS: %d\n", &steps) != 1)
				goto cleanup;
			for (auto k = 0; k < steps; k++) {
				fgets(line, 256, file);
				if (sscanf(line, "%s >= %[^;];%[^;];%[^;];%s\n",
					   tmp, rel1, rel2, rel3, rel4) == 5) {
					std::string step1(rel1), step2(rel2), step3(rel3), step4(rel4);
					stripWhitespace(step1);
					stripWhitespace(step2);
					stripWhitespace(step3);
					stripWhitespace(step4);
					lib.addStepToRelation(name, {step1, step2, step3, step4});
				} else if (sscanf(line, "%s >= %[^;];%s\n", tmp, rel1, rel2) == 3) {
					std::string step1(rel1), step2(rel2);
					stripWhitespace(step1);
					stripWhitespace(step2);
					if (step1 == step2)
						lib.makeRelationTransitive(name);
					else
						lib.addStepToRelation(name, {step1, step2});
				} else if (sscanf(line, "%s >= %s\n", tmp, rel1) == 2) {
					std::string step(rel1);
					stripWhitespace(step);
					lib.addStepToRelation(name, {step});
				} else {
					goto cleanup;
				}
			}
		}
		if (fscanf(file, "ACYCLIC: %d\n", &rels) != 1)
			goto cleanup;
		for (auto j = 0; j < rels; j++) {
			if (fscanf(file, "%s\n", name) != 1)
				goto cleanup;
			lib.addConstraint(name);
		}
		if (fscanf(file, "FUNCTIONAL: %s\n", tmp) != 1)
			goto cleanup;
		if (std::string(tmp) == "YES")
			lib.markFunctionalRfs();

		if (fscanf(file, "\n") != 0)
			goto cleanup;

		/* Push the library into the result */
		result.push_back(lib);
	}

cleanup:
	fclose(file);
	return result;
}
