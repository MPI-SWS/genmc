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

#ifndef __DRIVER_FACTORY_HPP__
#define __DRIVER_FACTORY_HPP__

#include "Config.hpp"
#include "IMMDriver.hpp"
#include "RADriver.hpp"
#include "RC11Driver.hpp"
#include "SCDriver.hpp"
#include "TSODriver.hpp"
#include <llvm/IR/Module.h>

class DriverFactory {

 public:
	template<typename... Ts>
	static std::unique_ptr<GenMCDriver>
	create(std::shared_ptr<const Config> conf, Ts&&... params) {
		return DriverFactory::create(nullptr, std::move(conf), std::forward<Ts>(params)...);
	}

	template<typename... Ts>
	static std::unique_ptr<GenMCDriver>
	create(ThreadPool *pool, std::shared_ptr<const Config> conf, Ts&&... params) {

#define CREATE_MODEL_DRIVER(_model)					\
		case ModelType::_model:					\
			driver = new _model##Driver(std::move(conf), std::forward<Ts>(params)...); \
			break

		GenMCDriver *driver = nullptr;
		switch (conf->model) {
			CREATE_MODEL_DRIVER(SC);
			CREATE_MODEL_DRIVER(TSO);
			CREATE_MODEL_DRIVER(RA);
			CREATE_MODEL_DRIVER(RC11);
			CREATE_MODEL_DRIVER(IMM);
		default:
			BUG();
		}
		driver->setThreadPool(pool);
		return std::unique_ptr<GenMCDriver>(driver);
	}
};

#endif /* __DRIVER_FACTORY_HPP__ */
