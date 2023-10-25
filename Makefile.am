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
  src/ASize.cpp src/ASize.hpp \
  src/BisimilarityCheckerPass.cpp src/BisimilarityCheckerPass.hpp \
  src/Bitmask.hpp \
  src/BoundDecider.cpp src/BoundDecider.hpp \
  src/CallInfoCollectionPass.cpp src/CallInfoCollectionPass.hpp \
  src/CodeCondenserPass.cpp src/CodeCondenserPass.hpp \
  src/ContextBoundDecider.cpp src/ContextBoundDecider.hpp \
  src/Calculator.hpp \
  src/CallInstWrapper.hpp \
  src/ConfirmationAnnotationPass.cpp src/ConfirmationAnnotationPass.hpp \
  src/Config.cpp src/Config.hpp \
  src/DeclareInternalsPass.cpp src/DeclareInternalsPass.hpp \
  src/DefineLibcFunsPass.cpp src/DefineLibcFunsPass.hpp \
  src/DepExecutionGraph.cpp src/DepExecutionGraph.hpp \
  src/DepInfo.cpp src/DepInfo.hpp \
  src/DepTracker.hpp \
  src/DepView.cpp src/DepView.hpp \
  src/DriverFactory.hpp \
  src/EliminateAnnotationsPass.cpp src/EliminateAnnotationsPass.hpp \
  src/EliminateCASPHIsPass.cpp src/EliminateCASPHIsPass.hpp \
  src/EliminateCastsPass.cpp src/EliminateCastsPass.hpp \
  src/EliminateRedundantInstPass.cpp src/EliminateRedundantInstPass.hpp \
  src/EliminateUnusedCodePass.cpp src/EliminateUnusedCodePass.hpp \
  src/Error.hpp \
  src/EscapeCheckerPass.cpp src/EscapeCheckerPass.hpp \
  src/Event.cpp src/Event.hpp \
  src/EventAttr.hpp \
  src/EventLabel.cpp src/EventLabel.hpp \
  src/Execution.cpp \
  src/ExecutionGraph.cpp src/ExecutionGraph.hpp \
  src/ExternalFunctions.cpp \
  src/FunctionInlinerPass.cpp src/FunctionInlinerPass.hpp \
  src/GenMCDriver.cpp src/GenMCDriver.hpp \
  src/GraphIterators.hpp \
  src/IMMDriver.cpp src/IMMDriver.hpp \
  src/InstAnnotator.cpp src/InstAnnotator.hpp \
  src/Interpreter.cpp Interpreter.h \
  src/InterpreterEnumAPI.cpp src/InterpreterEnumAPI.hpp \
  src/IntrinsicLoweringPass.cpp src/IntrinsicLoweringPass.hpp \
  src/LabelVisitor.hpp \
  src/LoadAnnotationPass.cpp src/LoadAnnotationPass.hpp \
  src/LocalSimplifyCFGPass.cpp src/LocalSimplifyCFGPass.hpp \
  src/Logger.hpp \
  src/LLVMModule.cpp src/LLVMModule.hpp \
  src/LLVMUtils.cpp src/LLVMUtils.hpp \
  src/LoopUnrollPass.cpp src/LoopUnrollPass.hpp \
  src/LoopJumpThreadingPass.cpp src/LoopJumpThreadingPass.hpp \
  src/Matrix2D.hpp Matrix2D.tcc \
  src/MaximalIterator.hpp \
  src/MDataCollectionPass.hpp src/MDataCollectionPass.cpp \
  src/MemAccess.hpp \
  src/MMDetectorPass.cpp src/MMDetectorPass.hpp \
  src/ModuleID.hpp \
  src/ModuleInfo.cpp src/ModuleInfo.hpp \
  src/NameInfo.cpp src/NameInfo.hpp \
  src/Parser.cpp src/Parser.hpp \
  src/PromoteMemIntrinsicPass.cpp src/PromoteMemIntrinsicPass.hpp \
  src/PropagateAssumesPass.cpp src/PropagateAssumesPass.hpp \
  src/RADriver.cpp src/RADriver.hpp \
  src/RC11Driver.cpp src/RC11Driver.hpp \
  src/Revisit.cpp src/Revisit.hpp \
  src/RoundBoundDecider.cpp src/RoundBoundDecider.hpp \
  src/SAddr.cpp src/SAddr.hpp \
  src/SAddrAllocator.hpp \
  src/SCDriver.cpp src/SCDriver.hpp \
  src/SExpr.tcc src/SExpr.hpp \
  src/SExprVisitor.tcc src/SExprVisitor.hpp \
  src/Stamp.cpp src/Stamp.hpp \
  src/SVal.cpp src/SVal.hpp \
  src/SpinAssumePass.cpp src/SpinAssumePass.hpp \
  src/WBIterator.hpp \
  src/ThreadInfo.hpp \
  src/ThreadPinner.cpp src/ThreadPinner.hpp \
  src/ThreadPool.cpp src/ThreadPool.hpp \
  src/TSODriver.cpp src/TSODriver.hpp \
  src/WorkSet.hpp src/WorkSet.cpp \
  src/value_ptr.hpp \
  src/Verbosity.hpp src/Verbosity.cpp \
  src/VectorClock.hpp src/VectorClock.cpp \
  src/VerificationError.hpp src/VerificationError.cpp \
  src/View.hpp src/View.cpp \
  src/VSet.hpp src/VSet.tcc

TESTS=scripts/fast-driver.sh scripts/randomize-driver.sh

AM_CXXFLAGS = -DINCLUDE_DIR=\"$(pkgincludedir)/$(pkg)\" -DSRC_INCLUDE_DIR=\"$(abs_top_srcdir)/include\" $(COVERAGE_CXXFLAGS)
bin_PROGRAMS = genmc
genmc_SOURCES = src/main.cpp
genmc_LDADD = libgenmc.a -lpthread $(COVERAGE_LDFLAGS)

include Makefile.am.coverage

@DX_RULES@
MOSTLYCLEANFILES = DX_CLEANFILES

if LINT
lint-local: $(libgenmc_a_SOURCES) $(genmc_SOURCES)
	-$(CPPLINT) -header-filter='^(?!llvm/).+' -config-file='$(top_srcdir)/.clang-tidy' $^

check-local: lint-local
endif
