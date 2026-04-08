set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts)
add_test(
    NAME fast-driver
    COMMAND bash -c "${SCRIPT_DIR}/fast-driver.sh > ${SCRIPT_DIR}/fast-driver.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(
    NAME randomize-driver
    COMMAND bash -c "${SCRIPT_DIR}/randomize-driver.sh > ${SCRIPT_DIR}/randomize-driver.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME run-parallel
    COMMAND bash -c "${SCRIPT_DIR}/run-parallel.sh > ${SCRIPT_DIR}/run-parallel.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME run-relinche-impl
    COMMAND bash -c "${SCRIPT_DIR}/run-relinche-impl.sh > ${SCRIPT_DIR}/run-relinche-impl.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME run-relinche-spec
    COMMAND bash -c "${SCRIPT_DIR}/run-relinche-spec.sh > ${SCRIPT_DIR}/run-relinche-spec.log"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Unit tests
add_subdirectory(unit)
