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

#ifndef GENMC_RC_HPP
#define GENMC_RC_HPP

#include <cassert>
#include <cstddef>
#include <utility>

namespace genmc {

template <typename T> class intrusive_ptr;

/*
 * A "ref counted" class that maintains the count intrusively.
 * Rc<T>  should be inherited by T objects. The template is
 * parameterized so that we can fire up the dtor without it
 * being virtual (CRTP style).
 *
 * NOTE: This class is *not* thread-safe. It assumes that all
 * pointers to an object are owned by a single thread.
 */
template <typename T> class Rc {
protected:
	friend class intrusive_ptr<T>;

	/* Ctors/dtors should be called manually, hence protected */
	Rc() noexcept = default;

	/* Do not copy the count when copying/moving the object */
	Rc(const Rc & /*other */) noexcept {}
	Rc(Rc && /* other */) noexcept {}
	auto operator=(const Rc & /* other */) noexcept -> Rc & { return *this; }
	auto operator=(Rc && /* other */) noexcept -> Rc & { return *this; }

	~Rc() = default;

	void add_ref() const noexcept { ++count_; }

	void release() const noexcept
	{
		if (--count_ == 0)
			delete static_cast<const T *>(this);
	}

	auto use_count() const noexcept { return count_; }

private:
	/* Mutable so that intrusive_ptr<const T> works */
	mutable long count_{};
};

/*
 * An implementation of an intrusive shared pointer.
 * Pointees are assumed to inherit from Rc<T>.
 *
 * Usage example:
 *
 * class Node : public genmc::Rc<Node> {
 * public:
 *   int data;
 *
 * Node(int v) : data(v) {} // no virtual dtor necessary
 *
 * };
 *
 * int main()
 * {
 *
 *   auto n = genmc::make_intrusive<Node>(42);
 *   auto copy = n;
 *
 * }
 */
template <typename T> class intrusive_ptr {
public:
	constexpr intrusive_ptr() noexcept = default;
	constexpr intrusive_ptr(std::nullptr_t) noexcept : ptr_(nullptr) {}
	explicit intrusive_ptr(T *ptr) noexcept : ptr_(ptr)
	{
		if (ptr_)
			ptr_->add_ref();
	}

	intrusive_ptr(const intrusive_ptr &other) noexcept : ptr_(other.ptr_)
	{
		if (ptr_)
			ptr_->add_ref();
	}
	intrusive_ptr(intrusive_ptr &&other) noexcept : ptr_(other.ptr_) { other.ptr_ = nullptr; }

	auto operator=(const intrusive_ptr &other) noexcept -> intrusive_ptr &
	{
		/* Hack: use copy-and-swap */
		intrusive_ptr(other).swap(*this);
		return *this;
	}
	auto operator=(intrusive_ptr &&other) noexcept -> intrusive_ptr &
	{
		intrusive_ptr(std::move(other)).swap(*this);
		return *this;
	}

	~intrusive_ptr()
	{
		if (ptr_)
			ptr_->release();
	}

	void reset(T *ptr = nullptr) noexcept { intrusive_ptr(ptr).swap(*this); }

	void swap(intrusive_ptr &other) noexcept { std::swap(ptr_, other.ptr_); }

	[[nodiscard]] auto use_count() const noexcept -> long
	{
		return ptr_ ? ptr_->use_count() : 0;
	}

	auto get() const noexcept -> T * { return ptr_; }
	auto operator*() const noexcept -> T & { return *ptr_; }
	auto operator->() const noexcept -> T * { return ptr_; }
	explicit operator bool() const noexcept { return ptr_ != nullptr; }

	auto operator<=>(const intrusive_ptr &other) const = default;

private:
	T *ptr_{};
};

template <typename T, typename... Args> auto make_intrusive(Args &&...args) -> intrusive_ptr<T>
{
	return intrusive_ptr<T>(new T(std::forward<Args>(args)...));
}
}; /* namespace genmc */

#endif /* GENMC_RC_HPP */
