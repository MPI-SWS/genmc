/* Adapted from: https://github.com/LoopPerfect/valuable */

// MIT License

// Copyright (c) 2017 LoopPerfect

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*
 * (For the parts of the code modified from the original)
 *
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
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef GENMC_VALUE_PTR_HPP
#define GENMC_VALUE_PTR_HPP
#include <memory>

#ifdef _MSC_VER
#define DECLSPEC_EMPTY_BASES __declspec(empty_bases)
#else
#define DECLSPEC_EMPTY_BASES
#endif

namespace detail {
struct spacer {};
// For details of class_tag use here, see
// https://mortoray.com/2013/06/03/overriding-the-broken-universal-reference-t/
template <typename T> struct class_tag {};

template <class T, class Deleter, class T2>
struct DECLSPEC_EMPTY_BASES compressed_ptr : std::unique_ptr<T, Deleter>, T2 {
	using T1 = std::unique_ptr<T, Deleter>;
	compressed_ptr() = default;
	compressed_ptr(compressed_ptr &&) = default;
	compressed_ptr(const compressed_ptr &) = default;
	compressed_ptr(T2 &&a2) : T2(std::move(a2)) {}
	compressed_ptr(const T2 &a2) : T2(a2) {}
	template <typename A1>
	compressed_ptr(A1 &&a1)
		: compressed_ptr(std::forward<A1>(a1), class_tag<std::decay_t<A1>>(), spacer(),
				 spacer())
	{}
	template <typename A1, typename A2>
	compressed_ptr(A1 &&a1, A2 &&a2)
		: compressed_ptr(std::forward<A1>(a1), std::forward<A2>(a2),
				 class_tag<std::decay_t<A2>>(), spacer())
	{}
	template <typename A1, typename A2, typename A3>
	compressed_ptr(A1 &&a1, A2 &&a2, A3 &&a3)
		: T1(std::forward<A1>(a1), std::forward<A2>(a2)), T2(std::forward<A3>(a3))
	{}

	template <typename A1>
	compressed_ptr(A1 &&a1, class_tag<typename std::decay<A1>::type>, spacer, spacer)
		: T1(std::forward<A1>(a1))
	{}
	template <typename A1, typename A2>
	compressed_ptr(A1 &&a1, A2 &&a2, class_tag<Deleter>, spacer)
		: T1(std::forward<A1>(a1), std::forward<A2>(a2))
	{}
	template <typename A1, typename A2>
	compressed_ptr(A1 &&a1, A2 &&a2, class_tag<T2>, spacer)
		: T1(std::forward<A1>(a1)), T2(std::forward<A2>(a2))
	{}
};
} // namespace detail

template <typename T> struct default_clone {
	default_clone() = default;
	auto operator()(T const &x) const -> T * { return new T(x); }
	auto operator()(T &&x) const -> T * { return new T(std::move(x)); }
};

template <class T, class Cloner = default_clone<T>, class Deleter = std::default_delete<T>>
class value_ptr {
	::detail::compressed_ptr<T, Deleter, Cloner> ptr_;

	auto ptr() -> std::unique_ptr<T, Deleter> & { return ptr_; }
	auto ptr() const -> std::unique_ptr<T, Deleter> const & { return ptr_; }

	auto clone(T const &x) const -> T * { return get_cloner()(x); }

public:
	using pointer = T *;
	using element_type = T;
	using cloner_type = Cloner;
	using deleter_type = Deleter;

	value_ptr() = default;

	value_ptr(const T &value) : ptr_(cloner_type()(value)) {}
	value_ptr(T &&value) : ptr_(cloner_type()(std::move(value))) {}

	value_ptr(const Cloner &value) : ptr_(value) {}
	value_ptr(Cloner &&value) : ptr_(std::move(value)) {}

	template <typename V, typename ClonerOrDeleter>
	value_ptr(V &&value, ClonerOrDeleter &&a2)
		: ptr_(std::forward<V>(value), std::forward<ClonerOrDeleter>(a2))
	{}

	template <typename V, typename C, typename D>
	value_ptr(V &&value, C &&cloner, D &&deleter)
		: ptr_(std::forward<V>(value), std::forward<D>(deleter), std::forward<C>(cloner))
	{}

	value_ptr(value_ptr const &v) : ptr_{nullptr, v.get_cloner()}
	{
		if (v) {
			ptr().reset(clone(*v));
		}
	}
	value_ptr(value_ptr &&v) = default;

	explicit value_ptr(pointer value) : ptr_(value) {}
	auto release() -> pointer { return ptr().release(); }

	value_ptr(std::nullptr_t) noexcept : ptr_() {}

	auto get() noexcept -> T * { return ptr().get(); }
	auto get() const noexcept -> T const * { return ptr().get(); }

	auto get_cloner() noexcept -> Cloner & { return ptr_; }
	auto get_cloner() const noexcept -> Cloner const & { return ptr_; };

	auto get_deleter() noexcept -> Deleter & { return ptr_; }
	auto get_deleter() const noexcept -> Deleter const & { return ptr_; }

	auto operator*() -> T & { return *get(); }
	auto operator*() const -> T const & { return *get(); }

	auto operator->() const noexcept -> T const * { return get(); }
	auto operator->() noexcept -> T * { return get(); }

	auto operator=(value_ptr &&v) -> value_ptr<T, Cloner, Deleter> &
	{
		ptr() = std::move(v.ptr());
		get_cloner() = std::move(v.get_cloner());
		return *this;
	}

	auto operator=(value_ptr const &v) -> value_ptr<T, Cloner, Deleter> &
	{
		ptr().reset(v.get_cloner()(*v));
		get_cloner() = v.get_cloner();
		return *this;
	}

	auto operator=(std::nullptr_t) noexcept -> value_ptr<T, Cloner, Deleter> &
	{
		ptr().reset();
		return *this;
	}

	auto operator=(std::unique_ptr<T, Deleter> p) -> value_ptr<T, Cloner, Deleter> &
	{
		ptr() = std::move(p);
		return *this;
	}

	operator bool() const noexcept { return !!ptr(); }
	~value_ptr() = default;
};

#undef DECLSPEC_EMPTY_BASES

#endif /* GENMC_VALUE_PTR_HPP */
