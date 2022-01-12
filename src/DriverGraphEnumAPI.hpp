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

#ifndef __DRIVER_GRAPH_ENUM_API_HPP__
#define __DRIVER_GRAPH_ENUM_API_HPP__

/* Types of coherence */
enum class CoherenceType { mo, wb };

/* Whether we should approximate consistency checks (when we get the chance) */
enum class CheckConsType { fast = 1, slow, full };

/* Points of the program (at which consistency/persistency can be checked) */
enum class ProgramPoint { error = 1, exec, step };

#include "InterpreterEnumAPI.hpp"

#endif /* __DRIVER_GRAPH_ENUM_API_HPP__ */
