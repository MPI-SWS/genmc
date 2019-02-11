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

#include "Event.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const EventType &t)
{
	switch (t) {
	case ERead    : return s << "R";
	case EWrite   : return s << "W";
	case EFence   : return s << "F";
	case EMalloc  : return s << "A";
	case EFree    : return s << "D";
	case EStart   : return s << "B";
	case EFinish  : return s << "E";
	case ETCreate : return s << "C";
	case ETJoin   : return s << "J";
	default : return s;
	}
}

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const EventAttr &a)
{
	switch (a) {
	case ATTR_PLAIN : return s;
	case ATTR_CAS   : return s << "CAS";
	case ATTR_FAI   : return s << "FAI";
	case ATTR_LOCK  : return s << "LOCK";
	case ATTR_UNLOCK: return s << "UNLOCK";
	default : return s;
	}
}

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const Event &e)
{
	if (e.isInitializer())
		return s << "INIT";
	return s << "\"Event (" << e.thread << ", " << e.index << ")\"";
}
