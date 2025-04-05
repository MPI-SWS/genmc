# where to install the headers on the system
genmcdir = $(pkgincludedir)

# the list of header files that belong to the library
nobase_genmc_HEADERS = \
  include/assert.h \
  include/atomic \
  include/bits/stat.h \
  include/cassert \
  include/errno.h \
  include/fcntl.h \
  include/genmc.h \
  include/genmc_internal.h \
  include/lkmm.h \
  include/pthread.h \
  include/stdatomic.h \
  include/stdio.h \
  include/stdlib.h \
  include/sys/stat.h \
  include/sys/types.h \
  include/thread \
  include/threads.h \
  include/unistd.h

noinst_LIBRARIES = libgenmc.a
libgenmc_a_SOURCES = \
  src/ADT/DepView.cpp \
  src/ADT/VectorClock.cpp \
  src/ADT/View.cpp \
  src/Config/Config.cpp \
  src/Config/Verbosity.cpp \
  src/ExecutionGraph/DepExecutionGraph.cpp \
  src/ExecutionGraph/DepInfo.cpp \
  src/ExecutionGraph/Event.cpp \
  src/ExecutionGraph/EventLabel.cpp \
  src/ExecutionGraph/ExecutionGraph.cpp \
  src/ExecutionGraph/GraphUtils.cpp \
  src/ExecutionGraph/Stamp.cpp \
  src/ExecutionGraph/Consistency/BoundDecider.cpp \
  src/ExecutionGraph/Consistency/ConsistencyChecker.cpp \
  src/ExecutionGraph/Consistency/ContextBoundDecider.cpp \
  src/ExecutionGraph/Consistency/IMMChecker.cpp \
  src/ExecutionGraph/Consistency/RAChecker.cpp \
  src/ExecutionGraph/Consistency/RC11Checker.cpp \
  src/ExecutionGraph/Consistency/RoundBoundDecider.cpp \
  src/ExecutionGraph/Consistency/SCChecker.cpp \
  src/ExecutionGraph/Consistency/SymmetryChecker.cpp \
  src/ExecutionGraph/Consistency/TSOChecker.cpp \
  src/Runtime/Execution.cpp \
  src/Runtime/ExternalFunctions.cpp \
  src/Runtime/Interpreter.cpp \
  src/Runtime/InterpreterEnumAPI.cpp \
  src/Static/LLVMModule.cpp \
  src/Static/LLVMUtils.cpp \
  src/Static/ModuleInfo.cpp \
  src/Static/Transforms/BisimilarityCheckerPass.cpp \
  src/Static/Transforms/CallInfoCollectionPass.cpp \
  src/Static/Transforms/CodeCondenserPass.cpp \
  src/Static/Transforms/ConfirmationAnnotationPass.cpp \
  src/Static/Transforms/DeclareInternalsPass.cpp \
  src/Static/Transforms/DefineLibcFunsPass.cpp \
  src/Static/Transforms/EliminateAnnotationsPass.cpp \
  src/Static/Transforms/EliminateCASPHIsPass.cpp \
  src/Static/Transforms/EliminateCastsPass.cpp \
  src/Static/Transforms/EliminateRedundantInstPass.cpp \
  src/Static/Transforms/EliminateUnusedCodePass.cpp \
  src/Static/Transforms/EscapeCheckerPass.cpp \
  src/Static/Transforms/FunctionInlinerPass.cpp \
  src/Static/Transforms/InstAnnotator.cpp \
  src/Static/Transforms/IntrinsicLoweringPass.cpp \
  src/Static/Transforms/LoadAnnotationPass.cpp \
  src/Static/Transforms/LocalSimplifyCFGPass.cpp \
  src/Static/Transforms/LoopJumpThreadingPass.cpp \
  src/Static/Transforms/LoopUnrollPass.cpp \
  src/Static/Transforms/MDataCollectionPass.cpp \
  src/Static/Transforms/MMDetectorPass.cpp \
  src/Static/Transforms/PromoteMemIntrinsicPass.cpp \
  src/Static/Transforms/PropagateAssumesPass.cpp \
  src/Static/Transforms/SpinAssumePass.cpp \
  src/Support/ASize.cpp \
  src/Support/MemOrdering.cpp \
  src/Support/NameInfo.cpp \
  src/Support/Parser.cpp \
  src/Support/SAddr.cpp \
  src/Support/SAddrAllocator.cpp \
  src/Support/SVal.cpp \
  src/Support/ThreadPinner.cpp \
  src/Support/ThreadPool.cpp \
  src/Verification/ChoiceMap.cpp \
  src/Verification/GenMCDriver.cpp \
  src/Verification/WorkList.cpp \
  src/Verification/VerificationError.cpp \
  src/Verification/Revisit.cpp \
  src/Verification/Relinche/LinearizabilityChecker.cpp \
  src/Verification/Relinche/Linearization.cpp \
  src/Verification/Relinche/Observation.cpp \
  src/Verification/Relinche/Specification.cpp \
  src/main.cpp

TESTS=scripts/fast-driver.sh scripts/randomize-driver.sh scripts/run-relinche-tests.sh

AM_CXXFLAGS = -I$(abs_top_srcdir)/src -DINCLUDE_DIR=\"$(pkgincludedir)/$(pkg)/include\" -DSRC_INCLUDE_DIR=\"$(abs_top_srcdir)/include\" $(COVERAGE_CXXFLAGS)
bin_PROGRAMS = genmc
genmc_SOURCES = src/main.cpp
genmc_LDADD = libgenmc.a -lpthread $(COVERAGE_LDFLAGS)

include Makefile.am.coverage

@DX_RULES@
MOSTLYCLEANFILES = $(DX_CLEANFILES)

if LINT
lint-local: $(libgenmc_a_SOURCES) $(genmc_SOURCES)
	-$(CPPLINT) -header-filter='^(?!llvm/).+' -config-file='$(top_srcdir)/.clang-tidy' $^

check-local: lint-local
endif
