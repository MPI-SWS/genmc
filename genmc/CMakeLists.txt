set(LIB_SOURCES
  genmc/ADT/AdaptiveView.cpp
  genmc/ADT/DepView.cpp
  genmc/ADT/SmallVector.cpp
  genmc/ADT/VectorClock.cpp
  genmc/ADT/View.cpp
  genmc/Execution/CBIterator.cpp
  genmc/Execution/DepExecutionGraph.cpp
  genmc/Execution/EventLabel.cpp
  genmc/Execution/ExecutionGraph.cpp
  genmc/Execution/GraphUtils.cpp
  genmc/Execution/ExecutionState.cpp
  genmc/Execution/Consistency/BoundDecider.cpp
  genmc/Execution/Consistency/ConsistencyChecker.cpp
  genmc/Execution/Consistency/ContextBoundDecider.cpp
  genmc/Execution/Consistency/IMMChecker.cpp
  genmc/Execution/Consistency/RAChecker.cpp
  genmc/Execution/Consistency/RC11Checker.cpp
  genmc/Execution/Consistency/RoundBoundDecider.cpp
  genmc/Execution/Consistency/SCChecker.cpp
  genmc/Execution/Consistency/SymmetryChecker.cpp
  genmc/Execution/Consistency/TSOChecker.cpp
  genmc/Support/Error.cpp
  genmc/Support/NameInfo.cpp
  genmc/Support/Parser.cpp
  genmc/Support/SAddrAllocator.cpp
  genmc/Support/SVal.cpp
  genmc/Support/ThreadPinner.cpp
  genmc/Verification/ChoiceMap.cpp
  genmc/Verification/Config.cpp
  genmc/Verification/GenMCDriver.cpp
  genmc/Verification/Relinche/LinearizabilityChecker.cpp
  genmc/Verification/Relinche/Linearization.cpp
  genmc/Verification/Relinche/Observation.cpp
  genmc/Verification/Relinche/Specification.cpp
  genmc/Verification/Scheduler.cpp
)

### GenMC library
find_package(Threads REQUIRED)
find_library(HWLOC hwloc)

add_library(genmc_lib ${GENMC_LIB_TYPE} ${LIB_SOURCES})
install(TARGETS genmc_lib ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/genmc/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.def")

target_include_directories(genmc_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(genmc_lib PRIVATE ${GENMC_WARNING_FLAGS})
target_link_libraries(genmc_lib PUBLIC Threads::Threads genmc_config_includes)
if(HWLOC)
  target_link_libraries(genmc_lib PRIVATE hwloc)
else()
  message(STATUS "hwloc not found. Thread pinning disabled.")
endif()
