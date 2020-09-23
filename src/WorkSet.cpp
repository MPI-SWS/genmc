/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#include "WorkSet.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream& s,  const WorkItem::Kind k)
{
	switch (k) {
	case WorkItem::WI_FRev:
		s << "FR";
		break;
	case WorkItem::WI_FRevLib:
		s << "FRL";
		break;
	case WorkItem::WI_BRev:
		s << "BR";
		break;
	case WorkItem::WI_MO:
		s << "MO";
		break;
	case WorkItem::WI_MOLib:
		s << "MOL";
		break;
	default:
		s << "UNKNOWN";
		break;
	}
	return s;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const WorkItem &item)
{
	switch (item.getKind()) {
	case WorkItem::WI_FRev:
	case WorkItem::WI_FRevLib: {
		auto &fi = static_cast<const FRevItem &>(item);
		s << fi.getKind() << "(" << fi.getPos() << ": " << fi.getRev() << ")";
		break;
	}
	case WorkItem::WI_BRev: {
		auto &bi = static_cast<const BRevItem &>(item);
		s << bi.getKind() << "(" << bi.getPos() << ": ["
		  << bi.getRev() << ", ";
		for (auto &lab : bi.getPrefixNoRel())
			s << lab->getPos();
		s << "]";
		break;
	}
	case WorkItem::WI_MO:
	case WorkItem::WI_MOLib: {
		auto &mi = static_cast<const MOItem &>(item);
		s << mi.getKind() << "(" << mi.getPos() << ": " << mi.getMOPos() << ")";
		break;
	}

	default:
		s << "UNKNOWN";
		break;
	}
	return s;
}
