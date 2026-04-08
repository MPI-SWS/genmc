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

#include "VectorClock.hpp"
#include "genmc/ADT/DepView.hpp"
#include "genmc/ADT/View.hpp"
#include "genmc/Execution/EventLabel.hpp"

auto VectorClock::contains(const EventLabel *lab) const -> bool { return contains(lab->getPos()); }

auto VectorClock::clone() const -> std::unique_ptr<VectorClock>
{
	switch (getKind()) {
	case VC_View:
		return std::make_unique<View>(*static_cast<const View *>(this));
	case VC_DepView:
		return std::make_unique<DepView>(*static_cast<const DepView *>(this));
	}
	UNREACHABLE();
}

auto VectorClock::empty() const -> bool { return size() == 0; }

auto VectorClock::getMax(Event e) const -> int { return getMax(e.thread); }
