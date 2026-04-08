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

#ifndef GENMC_PARSER_HPP
#define GENMC_PARSER_HPP

#include <string>

namespace genmc {

/** Reads the file `absPath` and returns its contents into a string */
auto readFile(const std::string &absPath) -> std::string;

/** Reads the `line`-th number of `absPath` */
auto getFileLineByNumber(const std::string &absPath, int line) -> std::string;

/** Strips leading and trailing whitespace from `s` */
void stripWhitespace(std::string &s);

/** Extracts the filename given an absolute path `absPath` */
void extractFilename(std::string &absPath);

auto parseInstFromMData(int line, std::string absPath, const std::string &functionName)
	-> std::string;
} /* namespace genmc */

#endif /* GENMC_PARSER_HPP */
