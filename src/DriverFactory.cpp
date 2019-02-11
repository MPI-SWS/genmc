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

#include "DriverFactory.hpp"
#include "RC11WeakRADriver.hpp"
#include "RC11WBDriver.hpp"
#include "RC11MODriver.hpp"

std::unique_ptr<GenMCDriver>
DriverFactory::create(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		      std::vector<Library> &granted, std::vector<Library> &toVerify,
		      clock_t start)
{
	switch (conf->model) {
	case ModelType::weakra:
		return std::unique_ptr<RC11WeakRADriver>(
			new RC11WeakRADriver(std::move(conf), std::move(mod),
					     granted, toVerify, start));
	case ModelType::mo:
		return std::unique_ptr<RC11MODriver>(
			new RC11MODriver(std::move(conf), std::move(mod),
					 granted, toVerify, start));
	case ModelType::wb:
		return std::unique_ptr<RC11WBDriver>(
			new RC11WBDriver(std::move(conf), std::move(mod),
					 granted, toVerify, start));
	default:
		WARN("Unsupported model type! Exiting...\n");
		abort();
	}
}
