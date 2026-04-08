#include <gtest/gtest.h>
#include "genmc/Support/Error.hpp"

TEST(ErrorHandlingTest, VerifySuccesful) {
    // Should not crash
    VERIFY(true, "This should pass");
    VERIFY(1 + 1 == 2);
}

TEST(ErrorHandlingTest, VerifyFailure) {
    // Should crash in both Debug and Release
    EXPECT_DEATH(VERIFY(false, "Should fail"), "Internal check failed: false: Should fail");
}

TEST(ErrorHandlingTest, VerifyNegatedFailure) {
    // VERIFY(!(cond)) crashes when cond is true
    EXPECT_DEATH(VERIFY(!(true), "Should fail"), "Internal check failed: !\\(true\\): Should fail");
}

TEST(ErrorHandlingTest, AssertFailure) {
#ifdef ENABLE_GENMC_DEBUG
    // Should crash only in Debug
    EXPECT_DEATH(ASSERT(false, "Should fail in debug"), "Assertion failed: false: Should fail in debug");
#else
    // Should not crash in Release
    ASSERT(false, "This should be ignored in release");
#endif
}

TEST(ErrorHandlingTest, UnreachableFailure) {
#ifdef ENABLE_GENMC_DEBUG
    EXPECT_DEATH(UNREACHABLE("This point is unreachable"),
                 "Unreachable code reached: This point is unreachable");
#else
    // In release, __builtin_trap() fires SIGILL — process always dies
    EXPECT_DEATH(UNREACHABLE(), "");
#endif
}
