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

#ifndef GENMC_EVENT_ATTR_HPP
#define GENMC_EVENT_ATTR_HPP

#include "genmc/ADT/Bitmask.hpp"

/**
 * Attributes for write events
 */
enum class WriteAttr : std::uint8_t {
	None = 0x0,
	Local = 0x1,
	Final = 0x2,
	WWRacy = 0x4,
	Complete = 0x8,
};

ENABLE_BITMASK_OPERATORS(WriteAttr);

#endif /* GENMC_EVENT_ATTR_HPP */
