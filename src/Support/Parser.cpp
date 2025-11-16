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

#include "Parser.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

namespace genmc {

auto readFile(const std::string &absPath) -> std::string
{
	std::ifstream ifs(absPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	/* TODO: Error check here? */
	std::ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	/* TODO: Does tellg work on all platforms? */
	std::vector<char> bytes(fileSize);
	ifs.read(bytes.data(), fileSize);

	return std::string(bytes.data(), fileSize);
}

auto getFileLineByNumber(const std::string &absPath, int line) -> std::string
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

void stripWhitespace(std::string &s)
{
	s.erase(s.begin(),
		std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(),
		s.end());
}

void extractFilename(std::string &absPath)
{
	auto i = absPath.find_last_of('/');
	if (i != std::string::npos)
		absPath = absPath.substr(i + 1);
}

}; /* namespace genmc */
