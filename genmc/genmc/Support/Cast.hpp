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

#ifndef GENMC_CAST_HPP
#define GENMC_CAST_HPP

#include "genmc/Support/Error.hpp"

#include <memory>
#include <type_traits>

/** Casting utilities modeled after LLVM's isa<> and dyn_cast<> templates. */

namespace genmc {

template <typename Derived, typename Base> inline auto isa(const Base *base) -> bool
{
	if constexpr (std::is_base_of_v<Derived, Base>)
		return true;
	if constexpr (std::is_base_of_v<Base, Derived>)
		return Derived::classof(base);
	else
		return false;
}

template <typename Derived, typename Base> inline auto isa_and_present(const Base *base) -> bool
{
	return base ? isa<Derived, Base>(base) : false;
}

template <typename Derived, typename Base> inline auto cast(const Base *base) -> const Derived *
{
	ASSERT(genmc::isa<Derived>(base));
	return static_cast<const Derived *>(base);
}

template <typename Derived, typename Base> inline auto cast(Base *base) -> Derived *
{
	ASSERT(genmc::isa<Derived>(base));
	return static_cast<Derived *>(base);
}

template <typename Derived, typename Base> inline auto dyn_cast(const Base *base) -> const Derived *
{
	if constexpr (!std::is_base_of_v<Base, Derived> && !std::is_base_of_v<Derived, Base>)
		return nullptr;
	else
		return genmc::isa<Derived>(base) ? static_cast<const Derived *>(base) : nullptr;
}

template <typename Derived, typename Base> inline auto dyn_cast(Base *base) -> Derived *
{
	if constexpr (!std::is_base_of_v<Base, Derived> && !std::is_base_of_v<Derived, Base>)
		return nullptr;
	else
		return genmc::isa<Derived>(base) ? static_cast<Derived *>(base) : nullptr;
}

template <typename Derived, typename Base>
inline auto dyn_cast_if_present(const Base *base) -> const Derived *
{
	return base ? genmc::dyn_cast<Derived>(base) : nullptr;
}

template <typename Derived, typename Base> inline auto dyn_cast_if_present(Base *base) -> Derived *
{
	return base ? genmc::dyn_cast<Derived>(base) : nullptr;
}

/* unique_ptr specializations */
template <typename Derived, typename Base>
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
inline auto cast(std::unique_ptr<const Base> &&base) -> std::unique_ptr<const Derived>
{
	ASSERT(isa<Derived>(base.get()));
	return std::unique_ptr<const Derived>(static_cast<const Derived *>(base.release()));
}

template <typename Derived, typename Base>
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
inline auto cast(std::unique_ptr<Base> &&base) -> std::unique_ptr<Derived>
{
	ASSERT(isa<Derived>(base.get()));
	return std::unique_ptr<Derived>(static_cast<Derived *>(base.release()));
}

} // namespace genmc

#endif /* GENMC_CAST_HPP */
