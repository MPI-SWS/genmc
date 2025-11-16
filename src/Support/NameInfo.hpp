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

#ifndef GENMC_NAME_INFO_HPP
#define GENMC_NAME_INFO_HPP

#include "Error.hpp"

#include <format>
#include <string>
#include <vector>

/**
 * Represents naming information for a specific type/allocation
 */
class NameInfo {

public:
	NameInfo() = default;

	/** Mark name at offset O as N */
	void addOffsetInfo(unsigned int o, std::string n);

	/** Returns name at offset O */
	[[nodiscard]] auto getNameAtOffset(unsigned int o) const -> std::string;

	/** Returns the number of different offset information registered */
	[[nodiscard]] auto size() const -> size_t { return info.size(); }

	/** Whether we have any information stored */
	[[nodiscard]] auto empty() const -> bool { return info.empty(); }

	friend struct std::formatter<NameInfo>;

private:
	/*
	 * We keep a map (Values -> (offset, name_at_offset)), and after
	 * the interpreter and the variables are allocated and initialized,
	 * we use the map to dynamically find out the name corresponding to
	 * a particular address.
	 */
	using OffsetInfo = std::vector<std::pair<unsigned, std::string>>;

	/** Naming information at different offsets */
	OffsetInfo info;
};

/** Make `NameInfo` formattable with `std::format`. */
template <> struct std::formatter<NameInfo> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const NameInfo &info, std::format_context &ctx) const
	{
		auto out = ctx.out();
		for (const auto &kv : info.info) {
			out = std::format_to(out, "{}: {}\n", kv.first, kv.second);
		}
		return out;
	}
};

#endif /* GENMC_NAME_INFO_HPP */
