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

#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "Library.hpp"
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Parser {
public:
	string readFile(const string &fileName);
	static std::string getFileLineByNumber(const std::string &absPath, int line);
	static void stripWhitespace(std::string &s);
	static void stripSlashes(std::string &absPath);
	static void parseInstFromMData(std::pair<int, std::string> &locAndFile,
				       std::string functionName,
				       llvm::raw_ostream &os = llvm::outs());
	std::vector<Library> parseSpecs(const string &fileName);
};

#endif /* __PARSER_HPP__ */
