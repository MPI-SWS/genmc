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

#ifndef GENMC_EVENT_ATTR_HPP
#define GENMC_EVENT_ATTR_HPP

#include "ADT/Bitmask.hpp"

/**
 * Attributes for write events
 */
enum class WriteAttr {
	None = 0x0,
	Local = 0x1,
	Final = 0x2,
	WWRacy = 0x4,
};

ENABLE_BITMASK_OPERATORS(WriteAttr);

#endif /* GENMC_EVENT_ATTR_HPP */
