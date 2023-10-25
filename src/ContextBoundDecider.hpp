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
 * Author: Iason Marmanis <imarmanis@mpi-sws.org>
 */

#ifndef __CONTEXT_BOUND_DECIDER_HPP__
#define __CONTEXT_BOUND_DECIDER_HPP__

#include "BoundDecider.hpp"
#include "View.hpp"

class ContextBoundDecider : public BoundDecider {

public:
	ContextBoundDecider() = default;

private:
	[[nodiscard]] auto doesExecutionExceedBound(unsigned int bound) const -> bool override;
	[[nodiscard]] auto doesPrefixExceedBound(View v, int t, unsigned int bound) const -> bool;
	[[nodiscard]] auto getSlack() const -> unsigned override;
#ifdef ENABLE_GENMC_DEBUG
	[[nodiscard]] auto calculate() const -> unsigned override;
	[[nodiscard]] auto calculate(View v, int t) const -> unsigned;
#endif

};

#endif /* __CONTEXT_BOUND_DECIDER_HPP__ */
