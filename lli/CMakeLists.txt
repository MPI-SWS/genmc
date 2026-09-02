set(LLI_SOURCES
  Runtime/Execution.cpp
  Runtime/ExternalFunctions.cpp
  Runtime/Interpreter.cpp
  ${CMAKE_SOURCE_DIR}/genmc/genmc/Support/ThreadPool.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${LLI_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE genmc_config_includes genmc_lib genmc_passes ffi dl LLVM)

### Faster allocator for the allocation-heavy exploration; off under SANITIZE (own malloc)
set(GENMC_TCMALLOC "AUTO" CACHE STRING "Link tcmalloc: AUTO (use if found), ON (required), OFF")
set_property(CACHE GENMC_TCMALLOC PROPERTY STRINGS AUTO ON OFF)

set(_use_tcmalloc OFF)
if(SANITIZE)
  if(NOT GENMC_TCMALLOC STREQUAL "OFF")
    message(STATUS "tcmalloc: disabled under sanitizers")
  endif()
elseif(NOT GENMC_TCMALLOC STREQUAL "OFF")
  find_library(TCMALLOC_LIB NAMES tcmalloc_minimal libtcmalloc_minimal.so.4)
  if(TCMALLOC_LIB)
    set(_use_tcmalloc ON)
  elseif(GENMC_TCMALLOC STREQUAL "ON")
    message(FATAL_ERROR "tcmalloc_minimal not found (need libgoogle-perftools-dev)")
  endif()
endif()

if(_use_tcmalloc)
  message(STATUS "tcmalloc: ${TCMALLOC_LIB}")
  # nothing references a tcmalloc symbol directly, so keep the linker from dropping it
  target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--no-as-needed ${TCMALLOC_LIB} -Wl,--as-needed)
endif()
