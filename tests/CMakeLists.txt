set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts)
add_test(
    NAME fast-driver
    COMMAND bash -c "${SCRIPT_DIR}/fast-driver.sh > ${SCRIPT_DIR}/fast-driver.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(fast-driver PROPERTIES LABELS "integration;fast" TIMEOUT 300)

add_test(
    NAME randomize-driver
    COMMAND bash -c "${SCRIPT_DIR}/randomize-driver.sh > ${SCRIPT_DIR}/randomize-driver.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(randomize-driver PROPERTIES LABELS "integration" TIMEOUT 600)

add_test(NAME run-parallel
    COMMAND bash -c "${SCRIPT_DIR}/run-parallel.sh > ${SCRIPT_DIR}/run-parallel.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(run-parallel PROPERTIES LABELS "integration;concurrency" TIMEOUT 300)

add_test(NAME run-relinche-impl
    COMMAND bash -c "${SCRIPT_DIR}/run-relinche-impl.sh > ${SCRIPT_DIR}/run-relinche-impl.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(run-relinche-impl PROPERTIES LABELS "integration;relinche" TIMEOUT 300)

add_test(NAME run-relinche-spec
    COMMAND bash -c "${SCRIPT_DIR}/run-relinche-spec.sh > ${SCRIPT_DIR}/run-relinche-spec.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(run-relinche-spec PROPERTIES LABELS "integration;relinche" TIMEOUT 300)

if(SANITIZE MATCHES "address")
  set(_asan_env "ASAN_OPTIONS=detect_leaks=0:halt_on_error=0")
  set_tests_properties(fast-driver randomize-driver run-parallel PROPERTIES
      ENVIRONMENT "${_asan_env}")
endif()

# Unit tests
add_subdirectory(unit)
