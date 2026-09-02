include(FetchContent)

# Fetch googletest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# set(CMAKE_CXX_STANDARD 20)
FetchContent_MakeAvailable(googletest)

# Fetch rapidcheck
FetchContent_Declare(
  rapidcheck
  GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
  GIT_TAG        master
)
set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rapidcheck)

add_executable(unit_tests IntervalMapTest.cpp IntervalSetTest.cpp ViewTest.cpp ErrorTest.cpp CastTest.cpp CacheCounterTest.cpp)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main rapidcheck_gtest genmc_config_includes genmc_lib)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/genmc)
target_compile_definitions(unit_tests PRIVATE ENABLE_GENMC_DEBUG) # to access validate()

# register with ctest
include(GoogleTest)
gtest_discover_tests(unit_tests)
