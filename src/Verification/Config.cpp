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

#include "config.h"

#include "Support/Error.hpp"
#include "Verification/Config.hpp"

#include <filesystem>

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
	BUG(); /* Unknown SchedulePolicy */
}

void checkConfig(Config &conf)
{
	/* Check exploration options */
	if (conf.LAPOR) {
		ERROR("LAPOR is temporarily disabled.\n");
	}
	if (conf.confirmation) {
		ERROR("Confirmation is temporarily disabled.\n");
	}
	if (conf.model == ModelType::IMM && (conf.ipr || conf.symmetryReduction)) {
		WARN("In-place revisiting and symmetry reduction have no effect under IMM\n");
		conf.symmetryReduction = false;
		conf.ipr = false;
	}

	/* Check debugging options */
	if (!doesPolicySupportSeed(conf.schedulePolicy) && conf.printRandomScheduleSeed)
		WARN("--print-schedule-seed used without --schedule-policy={arbitrary,wfr}.\n");
	if (!doesPolicySupportSeed(conf.schedulePolicy) && !conf.randomScheduleSeed.empty())
		WARN("--schedule-seed used without --schedule-policy={arbitrary,wfr}.\n");

	/* Check bounding options */
	if (conf.bound.has_value() && conf.model != ModelType::SC) {
		ERROR("Bounding can only be used with --sc.\n");
	}
	GENMC_DEBUG(ERROR_ON(conf.bound.has_value() && conf.boundsHistogram,
			     "Bounds histogram cannot be used when bounding.\n"););
	if (conf.bound.has_value() && conf.boundType != BoundType::none) {
		WARN("--bound-type used without --bound.\n");
	}

	/* Sanitize bounding options */
	bool bounding = conf.bound.has_value();
	GENMC_DEBUG(bounding |= conf.boundsHistogram;);
	if (bounding && (conf.LAPOR || !conf.disableBAM || conf.symmetryReduction || conf.ipr ||
			 conf.schedulePolicy != SchedulePolicy::LTR)) {
		WARN("LAPOR/BAM/SR/IPR have no effect when --bound is used. Scheduling "
		     "defaults to LTR.\n");
		conf.LAPOR = conf.symmetryReduction = conf.ipr = false;
		conf.disableBAM = true;
		conf.schedulePolicy = SchedulePolicy::LTR;
	}

	/* Check Relinche options */
	if (conf.collectLinSpec.has_value() && !conf.checkLinSpec->empty()) {
		ERROR("Cannot collect and analyze linearizability specification in a single "
		      "run.\n");
	}
	if (conf.checkLinSpec.has_value() &&
	    (!std::filesystem::exists(*conf.checkLinSpec) ||
	     !std::filesystem::is_regular_file(*conf.checkLinSpec))) {
		ERROR("Specification file is not a regular file!\n");
	}
}
