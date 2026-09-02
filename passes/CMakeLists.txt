set(PASSES_SOURCES
  passes/InternalFunctions.cpp
  passes/LLVMModule.cpp
  passes/LLVMUtils.cpp
  passes/ModuleInfo.cpp
  passes/Transforms/BisimilarityCheckerPass.cpp
  passes/Transforms/CallInfoCollectionPass.cpp
  passes/Transforms/CodeCondenserPass.cpp
  passes/Transforms/ConfirmationAnnotationPass.cpp
  passes/Transforms/DeclareInternalsPass.cpp
  passes/Transforms/DefineLibcFunsPass.cpp
  passes/Transforms/EliminateAnnotationsPass.cpp
  passes/Transforms/EliminateCASPHIsPass.cpp
  passes/Transforms/EliminateCastsPass.cpp
  passes/Transforms/EliminateRedundantInstPass.cpp
  passes/Transforms/EliminateUnusedCodePass.cpp
  passes/Transforms/EscapeCheckerPass.cpp
  passes/Transforms/FunctionInlinerPass.cpp
  passes/Transforms/InstAnnotator.cpp
  passes/Transforms/IntrinsicLoweringPass.cpp
  passes/Transforms/LoadAnnotationPass.cpp
  passes/Transforms/LocalSimplifyCFGPass.cpp
  passes/Transforms/LoopJumpThreadingPass.cpp
  passes/Transforms/LoopUnrollPass.cpp
  passes/Transforms/MDataCollectionPass.cpp
  passes/Transforms/MMDetectorPass.cpp
  passes/Transforms/PromoteMemIntrinsicPass.cpp
  passes/Transforms/PropagateAssumesPass.cpp
  passes/Transforms/RustPrepPass.cpp
  passes/Transforms/SpinAssumePass.cpp
  passes/Transforms/StrengthenCASPass.cpp
  passes/Transforms/BarrierResultCheckerPass.cpp
)

add_library(genmc_passes OBJECT ${PASSES_SOURCES})

target_include_directories(genmc_passes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(genmc_passes PRIVATE ${GENMC_WARNING_FLAGS})
# Mark LLVM headers as SYSTEM so neither the compiler nor clang-tidy reports
# warnings originating inside LLVM's own headers (e.g. User-operand ArrayBound
# false positives, aligned_union_t deprecation).
target_include_directories(genmc_passes SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(genmc_passes PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(genmc_passes PUBLIC genmc_lib)
