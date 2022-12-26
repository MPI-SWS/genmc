## Make autotools quit complaining about explicit rules
AUTOMAKE_OPTIONS = -Wno-override

noinst_LIBRARIES = libgenmc.a
libgenmc_a_SOURCES = \
  ARCalculator.cpp ARCalculator.hpp \
  ARCalculatorLKMM.cpp ARCalculatorLKMM.hpp \
  BisimilarityCheckerPass.cpp BisimilarityCheckerPass.hpp \
  Bitmask.hpp \
  CallInfoCollectionPass.cpp CallInfoCollectionPass.hpp \
  CodeCondenserPass.cpp CodeCondenserPass.hpp \
  Calculator.hpp \
  CallInstWrapper.hpp \
  CoherenceCalculator.hpp \
  ConfirmationAnnotationPass.cpp ConfirmationAnnotationPass.hpp \
  Config.cpp Config.hpp \
  DeclareInternalsPass.cpp DeclareInternalsPass.hpp \
  DefineLibcFunsPass.cpp DefineLibcFunsPass.hpp \
  DepExecutionGraph.cpp DepExecutionGraph.hpp \
  DepInfo.cpp DepInfo.hpp \
  DepTracker.hpp \
  DepView.cpp DepView.hpp \
  DriverGraphEnumAPI.hpp \
  DriverFactory.hpp \
  EliminateAnnotationsPass.cpp EliminateAnnotationsPass.hpp \
  EliminateCASPHIsPass.cpp EliminateCASPHIsPass.hpp \
  EliminateCastsPass.cpp EliminateCastsPass.hpp \
  EliminateRedundantInstPass.cpp EliminateRedundantInstPass.hpp \
  EliminateUnusedCodePass.cpp EliminateUnusedCodePass.hpp \
  Error.cpp Error.hpp \
  EscapeCheckerPass.cpp EscapeCheckerPass.hpp \
  Event.cpp Event.hpp \
  EventAttr.hpp \
  EventLabel.cpp EventLabel.hpp \
  Execution.cpp \
  ExecutionGraph.cpp ExecutionGraph.hpp \
  ExternalFunctions.cpp \
  FunctionInlinerPass.cpp FunctionInlinerPass.hpp \
  GenMCDriver.cpp GenMCDriver.hpp \
  GraphBuilder.hpp \
  GraphIterators.hpp \
  IMMDriver.cpp IMMDriver.hpp \
  InstAnnotator.cpp InstAnnotator.hpp \
  Interpreter.cpp Interpreter.h \
  InterpreterEnumAPI.cpp InterpreterEnumAPI.hpp \
  IntrinsicLoweringPass.cpp IntrinsicLoweringPass.hpp \
  LabelVisitor.hpp \
  LoadAnnotationPass.cpp LoadAnnotationPass.hpp \
  LocalSimplifyCFGPass.cpp LocalSimplifyCFGPass.hpp \
  LBCalculatorLAPOR.hpp LBCalculatorLAPOR.cpp \
  LKMMDriver.cpp LKMMDriver.hpp \
  LLVMModule.cpp LLVMModule.hpp \
  LLVMUtils.cpp LLVMUtils.hpp \
  LoopUnrollPass.cpp LoopUnrollPass.hpp \
  LoopJumpThreadingPass.cpp LoopJumpThreadingPass.hpp \
  Matrix2D.hpp Matrix2D.tcc \
  MDataCollectionPass.hpp MDataCollectionPass.cpp \
  MemAccess.hpp \
  MOCalculator.hpp MOCalculator.cpp \
  ModuleID.hpp \
  ModuleInfo.cpp ModuleInfo.hpp \
  NameInfo.cpp NameInfo.hpp \
  Parser.cpp Parser.hpp \
  PBCalculator.cpp PBCalculator.hpp \
  PersistencyChecker.cpp PersistencyChecker.hpp \
  PromoteMemIntrinsicPass.cpp PromoteMemIntrinsicPass.hpp \
  PropagateAssumesPass.cpp PropagateAssumesPass.hpp \
  PROPCalculator.cpp PROPCalculator.hpp \
  PSCCalculator.cpp PSCCalculator.hpp \
  RC11Driver.cpp RC11Driver.hpp \
  RCUCalculator.cpp RCUCalculator.hpp \
  RCULinkCalculator.cpp RCULinkCalculator.hpp \
  RCUFenceCalculator.cpp RCUFenceCalculator.hpp \
  Revisit.cpp Revisit.hpp \
  RevisitSet.cpp RevisitSet.hpp \
  SAddr.cpp SAddr.hpp \
  SAddrAllocator.hpp \
  SExpr.tcc SExpr.hpp \
  SExprVisitor.tcc SExprVisitor.hpp \
  SVal.cpp SVal.hpp \
  SpinAssumePass.cpp SpinAssumePass.hpp \
  WBIterator.hpp \
  WBCalculator.cpp WBCalculator.hpp \
  ThreadPinner.cpp ThreadPinner.hpp \
  ThreadPool.cpp ThreadPool.hpp \
  WorkSet.hpp WorkSet.cpp \
  value_ptr.hpp \
  VectorClock.hpp VectorClock.cpp \
  View.hpp View.cpp \
  VSet.hpp VSet.tcc \
  XBCalculator.hpp XBCalculator.cpp


AM_CXXFLAGS = -Wno-unknown-warning-option -DINCLUDE_DIR=\"$(pkgincludedir)/$(pkg)\" -DSRC_INCLUDE_DIR=\"$(abs_top_builddir)/include\"
bin_PROGRAMS = genmc
genmc_SOURCES = main.cpp
genmc_LDADD   = libgenmc.a -lpthread
