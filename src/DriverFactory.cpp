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
#include "IMMDriver.hpp"
#include "RC11Driver.hpp"

std::unique_ptr<GenMCDriver>
DriverFactory::create(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		      std::vector<Library> &granted, std::vector<Library> &toVerify,
		      clock_t start)
{
	switch (conf->model) {
	case ModelType::rc11:
		return std::unique_ptr<RC11Driver>(
			new RC11Driver(std::move(conf), std::move(mod),
				       granted, toVerify, start));
	case ModelType::imm:
		return std::unique_ptr<IMMDriver>(
			new IMMDriver(std::move(conf), std::move(mod),
				      granted, toVerify, start));
	default:
		BUG();
	}
}
