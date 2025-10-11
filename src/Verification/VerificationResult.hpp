#ifndef GENMC_VERIFICATION_RESULT_HPP
#define GENMC_VERIFICATION_RESULT_HPP

#include "ADT/VSet.hpp"
#include "Verification/Relinche/LinearizabilityChecker.hpp"
#include "Verification/Relinche/Specification.hpp"
#include "Verification/VerificationError.hpp"

#include <memory>
#include <string>
#include <utility>

struct VerificationResult {
	std::optional<VerificationError> status{}; /**< Whether the verification
				    completed successfully */
	unsigned explored{};		  /**< Number of complete executions explored */
	unsigned exploredBlocked{};	  /**< Number of blocked executions explored */
	unsigned boundExceeding{};	  /**< Number of bound-exceeding executions explored */
	long double estimationMean{};	  /**< The mean of estimations */
	long double estimationVariance{}; /**< The (biased) variance of the estimations */
#ifdef ENABLE_GENMC_DEBUG
	unsigned exploredMoot{};		/**< Number of moot executions _encountered_ */
	unsigned duplicates{};			/**< Number of duplicate executions explored */
	llvm::IndexedMap<int> exploredBounds{}; /**< Number of complete executions not
			       exceeding each bound */
#endif
	std::string message;				 /**< A message to be printed */
	VSet<VerificationError> warnings;		 /**< The warnings encountered */
	std::unique_ptr<Specification> specification;	 /**< Spec collected (if any) */
	LinearizabilityChecker::Result relincheResult{}; /**< Spec analysis result */

	VerificationResult() = default;

	auto operator+=(VerificationResult &&other) -> VerificationResult &
	{
		/* Propagate latest error */
		if (other.status.has_value())
			status = other.status;
		message += other.message;
		explored += other.explored;
		exploredBlocked += other.exploredBlocked;
		boundExceeding += other.boundExceeding;
		estimationMean += other.estimationMean;
		estimationVariance += other.estimationVariance;
#ifdef ENABLE_GENMC_DEBUG
		exploredMoot += other.exploredMoot;
		/* Bound-blocked executions are calculated at the end */
		exploredBounds.grow(other.exploredBounds.size() - 1);
		for (auto i = 0U; i < other.exploredBounds.size(); i++)
			exploredBounds[i] += other.exploredBounds[i];
		duplicates += other.duplicates;
#endif
		warnings.insert(other.warnings);
		if (other.specification)
			specification->merge(
				std::move(*other.specification)); // FIXME other is const lvalue
		relincheResult += std::move(other.relincheResult);
		return *this;
	}
};

#endif /* GENMC_VERIFICATION_RESULT_HPP */
