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

#include "genmc/config.h"

#include "genmc/Support/Error.hpp"
#include "genmc/Verification/Config.hpp"

#include <filesystem>
#include <random>

static auto doesPolicySupportSeed(const SchedulePolicy policy) -> bool
{
	switch (policy) {
	case SchedulePolicy::Arbitrary:
	case SchedulePolicy::WFR:
		return true;
	case SchedulePolicy::LTR:
	case SchedulePolicy::WF:
		return false;
	}
	UNREACHABLE(); /* Unknown SchedulePolicy */
}

auto Config::validate(std::vector<std::string> &warnings) -> ValidationStatus
{
	ConfigErrorList errors;

	/* Check exploration options */
	if (LAPOR) {
		errors.emplace_back("LAPOR is temporarily disabled.");
	}
	if (confirmation) {
		errors.emplace_back("Confirmation is temporarily disabled.");
	}
	if (model == ModelType::IMM && (ipr || symmetryReduction)) {
		warnings.emplace_back(
			"In-place revisiting and symmetry reduction have no effect under IMM");
		symmetryReduction = false;
		ipr = false;
	}
	if (!emitNALabels && instructionCaching)
		errors.emplace_back("Instruction caching implies NA-label emission");

	/* Check sampling options */
	if (mode == ExplorationMode::random && randomMax == 0)
		errors.emplace_back("Random exploration budget must be greater than 0.");

	/* Check debugging options */
	if (!doesPolicySupportSeed(schedulePolicy) && printRandomScheduleSeed)
		warnings.emplace_back(
			"--print-schedule-seed used without --schedule-policy={{arbitrary,wfr}}.");
	if (!doesPolicySupportSeed(schedulePolicy) && randomScheduleSeed.has_value())
		warnings.emplace_back(
			"--schedule-seed used without --schedule-policy={{arbitrary,wfr}}.");

	/* Populate seed if not provided by user */
	if (!randomScheduleSeed)
		randomScheduleSeed = std::random_device()();

	/* Check bounding options */
	if (bound.has_value() && model != ModelType::SC) {
		errors.emplace_back("Bounding can only be used with --sc.");
	}
	GENMC_DEBUG(if (bound.has_value() && boundsHistogram)
			    errors.emplace_back("Bounds histogram cannot be used when bounding."););
	if (!bound.has_value() && boundType != BoundType::none) {
		warnings.emplace_back("--bound-type used without --bound.");
	}

	/* Sanitize bounding options */
	auto bounding = bound.has_value();
	GENMC_DEBUG(bounding |= boundsHistogram;);
	if (bounding && (LAPOR || !disableBAM || symmetryReduction || ipr ||
			 schedulePolicy != SchedulePolicy::LTR)) {
		warnings.emplace_back(
			"LAPOR/BAM/SR/IPR have no effect when --bound is used. Scheduling "
			"defaults to LTR.");
		LAPOR = symmetryReduction = ipr = false;
		disableBAM = true;
		schedulePolicy = SchedulePolicy::LTR;
	}

	/* Check Relinche options */
	if (collectLinSpec.has_value() && checkLinSpec.has_value()) {
		errors.emplace_back(
			"Cannot collect and analyze linearizability specification in a single "
			"run.");
	}
	if (checkLinSpec.has_value() && (!std::filesystem::exists(*checkLinSpec) ||
					 !std::filesystem::is_regular_file(*checkLinSpec))) {
		errors.emplace_back("Specification file is not a regular file!");
	}
	return errors.empty() ? ValidationStatus() : ValidationStatus(std::move(errors));
}
