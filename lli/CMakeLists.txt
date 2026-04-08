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
