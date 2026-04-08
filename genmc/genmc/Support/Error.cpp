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

#include "genmc/Support/Error.hpp"

#include <cstdlib>
#include <iostream>
#include <unistd.h>

namespace genmc::detail {

static void print_crash_message(std::string_view msg, std::source_location loc)
{
	std::cerr << "\n***************************************************************************"
		     "*****\n"
		  << "INTERNAL FAILURE: " << msg << "\n"
		  << "  at " << loc.file_name() << ":" << loc.line() << ":" << loc.function_name()
		  << "\n"
		  << "*****************************************************************************"
		     "***\n";
}

#ifdef ENABLE_GENMC_DEBUG
static void debuggable_exit()
{
	if (!isatty(STDIN_FILENO))
		std::abort();

	while (true) {
		std::cerr << "\n(C)ontinue, (A)bort, (S)top/Trap, (G)DB\n" << "> " << std::flush;
		char result;
		if (!(std::cin >> result))
			std::abort();

		switch (std::toupper(result)) {
		case 'C':
			return;
		case 'A':
			std::abort();
		case 'S':
			__builtin_trap();
			return;
		case 'G': {
			// Note: may fail silently if ptrace is restricted
			// (Yama LSM, /proc/sys/kernel/yama/ptrace_scope=1).
			// Use 'S' (trap) as a workaround when already in a debugger.
			std::string cmd = std::format("gdb -p {}", getpid());
			std::cerr << "Executing: " << cmd << " ...\n";
			if (std::system(cmd.c_str()) != 0) {
				std::cerr << "Failed to start gdb.\n";
			}
			return;
		}
		default:
			std::cerr << "Invalid option.\n";
		}
	}
}
#endif

[[noreturn]] void report_internal_error(std::string_view msg, std::source_location loc)
{
	print_crash_message(msg, loc);

#ifdef ENABLE_GENMC_DEBUG
	debuggable_exit();
#endif
	std::abort();
}

} /* namespace genmc::detail */
