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
		[](int c){ return !std::isspace(c); }));
	s.erase(std::find_if(s.rbegin(), s.rend(),
		[](int c){ return !std::isspace(c); }).base(), s.end());
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
