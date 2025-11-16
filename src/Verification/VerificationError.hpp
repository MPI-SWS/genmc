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

#ifndef GENMC_VERIFICATION_ERROR_HPP
#define GENMC_VERIFICATION_ERROR_HPP

#include "ExecutionGraph/Event.hpp"
#include "Support/Error.hpp"

#include <string>
#include <unordered_map>

/** Some basic system error codes for the user -- should match include/errno.h */
enum class SystemError {
	SE_EPERM = 1,
	SE_ENOENT = 2,
	SE_EIO = 5,
	SE_EBADF = 9,
	SE_ENOMEM = 12,
	SE_EEXIST = 17,
	SE_EINVAL = 22,
	SE_EMFILE = 24,
	SE_ENFILE = 23,
	SE_ETXTBSY = 26,
	SE_EFBIG = 27,
	SE_ESPIPE = 29,
};

/** Different errors that might be encountered during verification.
 * Public to enable the interpreter utilize it */
enum class VerificationError {
	VE_NonErrorBegin,
	VE_WWRace,
	VE_UnfreedMemory,
	VE_NonErrorLast,

	VE_Safety,
	VE_Recovery,
	VE_Liveness,
	VE_RaceNotAtomic,
	VE_RaceFreeMalloc,
	VE_FreeNonMalloc,
	VE_DoubleFree,
	VE_Allocation,

	VE_InvalidAccessBegin,
	VE_UninitializedMem,
	VE_AccessNonMalloc,
	VE_AccessFreed,
	VE_InvalidAccessEnd,

	VE_InvalidCreate,
	VE_InvalidJoin,
	VE_InvalidUnlock,
	VE_InvalidBInit,
	VE_BarrierWellFormedness,
	VE_Annotation,
	VE_MixedSize,
	VE_LinearizabilityError,
	VE_SystemError,
};

inline auto isInvalidAccessError(VerificationError s) -> bool
{
	return VerificationError::VE_InvalidAccessBegin <= s &&
	       s <= VerificationError::VE_InvalidAccessEnd;
}

inline auto isHardError(VerificationError err) -> bool
{
	/* An invalid unlock *must* be a hard error:
	 * the lock optimization depends on the absence of unordered
	 * writes to the lock location, which is not guaranteed if
	 * the lock is unlocked without being locked by the same thread first.
	 */
	return !(err >= VerificationError::VE_NonErrorBegin &&
		 err <= VerificationError::VE_NonErrorLast);
}

inline static SystemError systemErrorNumber; // just to inform the driver

/** Details for an error to be reported */
struct ErrorDetails {
	ErrorDetails() = default;
	ErrorDetails(Event pos, VerificationError r, std::string err = std::string(),
		     const EventLabel *racyLab = nullptr, bool shouldHalt = true)
		: pos(pos), type(r), msg(std::move(err)), racyLab(racyLab), shouldHalt(shouldHalt)
	{}

	Event pos{};
	VerificationError type{};
	std::string msg{};
	const EventLabel *racyLab{};
	bool shouldHalt = true;
};

/* Helper macro to reduce switch verbosity. */
#define FORMAT_CASE(error_case, s)                                                                 \
	case error_case:                                                                           \
		return std::format_to(ctx.out(), "{}", s);                                         \
		break

/** Make `SystemError` formattable with `std::format`. */
template <> struct std::formatter<SystemError> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const SystemError &err, std::format_context &ctx) const
	{
		switch (err) {
			FORMAT_CASE(SystemError::SE_EPERM, "Operation not permitted");
			FORMAT_CASE(SystemError::SE_ENOENT, "No such file or directory");
			FORMAT_CASE(SystemError::SE_EIO, "Input/output error");
			FORMAT_CASE(SystemError::SE_EBADF, "Bad file descriptor");
			FORMAT_CASE(SystemError::SE_ENOMEM, "Cannot allocate memory");
			FORMAT_CASE(SystemError::SE_EEXIST, "File exists");
			FORMAT_CASE(SystemError::SE_EINVAL, "Invalid argument");
			FORMAT_CASE(SystemError::SE_EMFILE, "Too many open files");
			FORMAT_CASE(SystemError::SE_ENFILE, "Too many open files in system");
			FORMAT_CASE(SystemError::SE_EFBIG, "File too large");
		default:
			return std::format_to(ctx.out(), "{}", "Unknown system error");
		}
	}
};

/** Make `VerificationError` formattable with `std::format`. */
template <> struct std::formatter<VerificationError> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const VerificationError &err, std::format_context &ctx) const
	{
		switch (err) {
			FORMAT_CASE(VerificationError::VE_Safety, "Safety violation");
			FORMAT_CASE(VerificationError::VE_Recovery, "Recovery error");
			FORMAT_CASE(VerificationError::VE_Liveness, "Liveness violation");
			FORMAT_CASE(VerificationError::VE_RaceNotAtomic, "Non-atomic race");
			FORMAT_CASE(VerificationError::VE_WWRace, "Unordered writes");
			FORMAT_CASE(VerificationError::VE_UnfreedMemory, "Unfreed memory");
			FORMAT_CASE(VerificationError::VE_RaceFreeMalloc, "Malloc-free race");
			FORMAT_CASE(VerificationError::VE_FreeNonMalloc,
				    "Attempt to free non-allocated memory");
			FORMAT_CASE(VerificationError::VE_DoubleFree, "Double-free error");
			FORMAT_CASE(VerificationError::VE_Allocation, "Allocation error");
			FORMAT_CASE(VerificationError::VE_UninitializedMem,
				    "Attempt to read from uninitialized memory");
			FORMAT_CASE(VerificationError::VE_AccessNonMalloc,
				    "Attempt to access non-allocated memory");
			FORMAT_CASE(VerificationError::VE_AccessFreed,
				    "Attempt to access freed memory");
			FORMAT_CASE(VerificationError::VE_InvalidCreate,
				    "Invalid create() operation");
			FORMAT_CASE(VerificationError::VE_InvalidJoin, "Invalid join() operation");
			FORMAT_CASE(VerificationError::VE_InvalidUnlock,
				    "Invalid unlock() operation");
			FORMAT_CASE(VerificationError::VE_InvalidBInit,
				    "Invalid barrier_init() operation");
			FORMAT_CASE(VerificationError::VE_BarrierWellFormedness,
				    "Execution not barrier-well-formed");
			FORMAT_CASE(VerificationError::VE_Annotation, "Annotation error");
			FORMAT_CASE(VerificationError::VE_MixedSize, "Mixed-size accesses");
			FORMAT_CASE(VerificationError::VE_LinearizabilityError,
				    "Linearizability error");
			FORMAT_CASE(VerificationError::VE_SystemError, systemErrorNumber);
		default:
			return std::format_to(ctx.out(), "Unknown status");
		}
	}
};

#endif /* GENMC_VERIFICATION_ERROR_HPP */
