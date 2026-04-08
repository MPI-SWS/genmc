cmake_minimum_required(VERSION 3.12...4.0)
project(genmc VERSION 0.17.0 LANGUAGES CXX C)

enable_testing()
include(CTest)

include(GNUInstallDirs)

### Package information
set(PKG_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/runtime")
set(PACKAGE_NAME "GenMC")
set(PACKAGE_BUGREPORT "michalis.kokologiannakis@inf.ethz.ch")
set(PACKAGE_URL "https://plv.mpi-sws.org/genmc")
set(PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/${PKG_INCLUDE_DIR}")
set(SRC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lli/runtime-include/c")
set(RUST_DIR "${CMAKE_SOURCE_DIR}/lli/runtime-include/rust")

# Executable output goes to bin/ to avoid conflict with genmc/ library directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

### Whether debugging infra is enabled
option(GENMC_DEBUG "Enable GenMC debugging infrastructure (ON for Debug)" OFF)

### Interface library for includes and compile features
add_library(genmc_config_includes INTERFACE)
target_compile_features(genmc_config_includes INTERFACE cxx_std_23)
target_include_directories(genmc_config_includes INTERFACE ${CMAKE_BINARY_DIR}/include)

install(DIRECTORY lli/runtime-include/c/ DESTINATION "${PKG_INCLUDE_DIR}" FILES_MATCHING PATTERN "*.h")

### Whether to build lli-based binary
option(BUILD_LLI "Build lli-based executable (requires LLVM)" ON)
if(BUILD_LLI)
  find_library(HAVE_LIBDL dl REQUIRED)
  find_library(HAVE_LIBFFI ffi REQUIRED)

  # For linking to LLVM, see: https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
  # We unset the cached variables here, to look for different LLVM installations if `CMAKE_PREFIX_PATH` changes
  unset(LLVM_DIR CACHE)
  unset(LLVM_FOUND CACHE)
  find_package(LLVM REQUIRED CONFIG)

  find_program(CLANGPATH clang PATHS ${LLVM_TOOLS_BINARY_DIR} NO_CACHE NO_DEFAULT_PATH REQUIRED)
  find_program(LLVM_CONFIG_PATH llvm-config PATHS ${LLVM_TOOLS_BINARY_DIR} NO_CACHE NO_DEFAULT_PATH REQUIRED)
  message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION} (${LLVM_CONFIG_PATH})")
  message(STATUS "Using clang ${CLANGPATH}")

  separate_arguments(LLVM_DEFINITIONS)
  execute_process(COMMAND "${LLVM_CONFIG_PATH}" --build-mode OUTPUT_VARIABLE LLVM_BUILDMODE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

### Whether to build lli-based binary
option(EMIT_NA_LABELS "Whether to emit labels for non-atomic events" ON)
if(BUILD_LLI AND NOT EMIT_NA_LABELS)
  message(FATAL_ERROR "BUILD_LLI=ON requires EMIT_NA_LABELS=ON.")
endif()
target_compile_definitions(genmc_config_includes INTERFACE
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${GENMC_DEBUG}>>:ENABLE_GENMC_DEBUG>
    EMIT_NA_LABELS=$<BOOL:${EMIT_NA_LABELS}>
)

### Whether to build lli binary with Rust support
option(ENABLE_RUST "Enable compiling using Rust inputs" OFF)
if(ENABLE_RUST)
  message(STATUS "Rust enabled!")
  find_program(RUSTC_PATH rustc HINTS ${CMAKE_PREFIX_PATH} NO_CACHE NO_DEFAULT_PATH)
  find_program(CARGO_PATH cargo HINTS ${CMAKE_PREFIX_PATH} NO_CACHE NO_DEFAULT_PATH)
  if(NOT RUSTC_PATH)
    message(FATAL_ERROR "Could not find a Rust installation")
  else()
    # Determine the LLVM-version used by Rust, reject if it's a different major version
    execute_process(COMMAND "${RUSTC_PATH}" -vV OUTPUT_VARIABLE RUSTC_VERSION)
    string(REGEX MATCH "LLVM version: (([0-9]+)\\.[0-9]+\\.[0-9]+)" _ ${RUSTC_VERSION})
    set(RUSTC_LLVM_VERSION "${CMAKE_MATCH_1}")
    set(RUSTC_LLVM_VERSION_MAJOR "${CMAKE_MATCH_2}")

    message(STATUS "Found rustc ${RUSTC_PATH} (LLVM-version: ${RUSTC_LLVM_VERSION})")
    message(STATUS "Found cargo ${CARGO_PATH}")
    if(LLVM_PACKAGE_VERSION MATCHES "${RUSTC_LLVM_VERSION_MAJOR}\\.[0-9]+\\.[0-9]+")
      set(CONFIG_RUSTC_PATH "${RUSTC_PATH}")
      set(CONFIG_CARGO_PATH "${CARGO_PATH}")
    else()
      message(FATAL_ERROR "Rust using different major LLVM-version then GenMC!")
    endif()
  endif()
endif()

if(ENABLE_RUST AND NOT BUILD_LLI)
  message(FATAL_ERROR "ENABLE_RUST=ON requires BUILD_LLI=ON.")
endif()

execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE GIT_RESULT
)
if(NOT GIT_RESULT EQUAL 0)
  message(STATUS "Unable to determine git commit hash: ${GIT_ERROR}")
  set(GIT_COMMIT "unknown")
endif()

### Coverage/linting
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  add_compile_options(-g --coverage)
  add_link_options(--coverage)
endif()

option(ENABLE_LINT "Enable clang-tidy reporting" OFF)
if(ENABLE_LINT)
  set(CMAKE_CXX_CLANG_TIDY
    clang-tidy
    "-header-filter='^(?!llvm/).+'"
    "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
  )
endif()

### Generate config headers (after all variables are set)
# Bridge the cmake option name to the C++ preprocessor guard for configure_file.
set(ENABLE_GENMC_DEBUG ${GENMC_DEBUG})
# config.h: library-public flags (installed with the library)
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/include/genmc/config.h)
install(FILES ${CMAKE_BINARY_DIR}/include/genmc/config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/genmc)
# lli_config.h: tool-private paths and package info (not installed)
configure_file(${CMAKE_SOURCE_DIR}/lli_config.h.in ${CMAKE_BINARY_DIR}/include/genmc/lli_config.h)

### Component subdirectories
add_subdirectory(genmc)                    # Core library (always built)

option(BUILD_PASSES "Build LLVM instrumentation passes" ${BUILD_LLI})
if(BUILD_PASSES OR BUILD_LLI)
  add_subdirectory(passes)                 # LLVM passes (needs LLVM)
endif()

if(BUILD_LLI)
  add_subdirectory(lli)                    # Interpreter frontend (needs LLVM)
endif()

option(BUILD_TESTS "Enable test building" OFF)
if(BUILD_TESTS)
add_subdirectory(tests)
endif()

OPTION(BUILD_DOC "Enable documentation building" OFF)
if(BUILD_DOC)
  add_subdirectory(${CMAKE_SOURCE_DIR}/doc)
endif()
