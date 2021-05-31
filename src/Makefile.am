## Make autotools quit complaining about explicit rules
AUTOMAKE_OPTIONS = -Wno-override

# where to install the headers on the system
libgenmc_adir = $(pkgincludedir)

# the list of header files that belong to the library
libgenmc_a_HEADERS = $(top_builddir)/include/*.h

noinst_LIBRARIES = libgenmc.a
libgenmc_a_SOURCES = \
  $(libgenmc_a_HEADERS) \
  ARCalculator.cpp ARCalculator.hpp \
  ARCalculatorLKMM.cpp ARCalculatorLKMM.hpp \
  BisimilarityCheckerPass.cpp BisimilarityCheckerPass.hpp \
  CallInfoCollectionPass.cpp CallInfoCollectionPass.hpp \
  CodeCondenserPass.cpp CodeCondenserPass.hpp \
  Calculator.hpp \
  CallInstWrapper.hpp \
  CoherenceCalculator.hpp \
  Config.cpp Config.hpp \
  DeclareInternalsPass.cpp DeclareInternalsPass.hpp \
  DefineLibcFunsPass.cpp DefineLibcFunsPass.hpp \
  DepExecutionGraph.cpp DepExecutionGraph.hpp \
  DepInfo.cpp DepInfo.hpp \
  DepTracker.hpp \
  DepView.cpp DepView.hpp \
  DriverGraphEnumAPI.hpp \
  DriverFactory.cpp DriverFactory.hpp \
  Error.cpp Error.hpp \
  Event.cpp Event.hpp \
  EventLabel.cpp EventLabel.hpp \
  Execution.cpp \
  ExecutionGraph.cpp ExecutionGraph.hpp \
  ExternalFunctions.cpp \
  GenMCDriver.cpp GenMCDriver.hpp \
  GraphBuilder.hpp \
  IMMDepTracker.hpp \
  IMMDriver.cpp IMMDriver.hpp \
  InstAnnotator.cpp InstAnnotator.hpp \
  Interpreter.cpp Interpreter.h \
  IntrinsicLoweringPass.cpp IntrinsicLoweringPass.hpp \
  LoadAnnotationPass.cpp LoadAnnotationPass.hpp \
  LBCalculatorLAPOR.hpp LBCalculatorLAPOR.cpp \
  Library.cpp Library.hpp \
  LKMMDriver.cpp LKMMDriver.hpp \
  LLVMModule.cpp LLVMModule.hpp \
  LLVMUtils.cpp LLVMUtils.hpp \
  LoopUnrollPass.cpp LoopUnrollPass.hpp \
  LoopJumpThreadingPass.cpp LoopJumpThreadingPass.hpp \
  Matrix2D.hpp Matrix2D.tcc \
  MDataCollectionPass.hpp MDataCollectionPass.cpp \
  MOCalculator.hpp MOCalculator.cpp \
  ModuleInfo.cpp ModuleInfo.hpp \
  Parser.cpp Parser.hpp \
  PersistencyChecker.cpp PersistencyChecker.hpp \
  PromoteMemIntrinsicPass.cpp PromoteMemIntrinsicPass.hpp \
  PBCalculator.cpp PBCalculator.hpp \
  PROPCalculator.cpp PROPCalculator.hpp \
  PSCCalculator.cpp PSCCalculator.hpp \
  RC11Driver.cpp RC11Driver.hpp \
  RCUCalculator.cpp RCUCalculator.hpp \
  RCULinkCalculator.cpp RCULinkCalculator.hpp \
  RCUFenceCalculator.cpp RCUFenceCalculator.hpp \
  RevisitSet.cpp RevisitSet.hpp \
  SExpr.cpp SExpr.hpp \
  SExprVisitor.cpp SExprVisitor.hpp \
  SpinAssumePass.cpp SpinAssumePass.hpp \
  WBCalculator.cpp WBCalculator.hpp \
  WorkSet.hpp WorkSet.cpp \
  VectorClock.hpp VectorClock.cpp \
  View.hpp View.cpp \
  VSet.hpp VSet.tcc \
  XBCalculator.hpp XBCalculator.cpp


AM_CXXFLAGS = -Wno-unknown-warning-option -DINCLUDE_DIR=\"$(pkgincludedir)/$(pkg)\" -DSRC_INCLUDE_DIR=\"$(abs_top_builddir)/include\"
bin_PROGRAMS = genmc
genmc_SOURCES = main.cpp
genmc_LDADD   = libgenmc.a -lclangCodeGen -lclangFrontend -lclangFrontendTool -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangRewriteFrontend -lclangEdit -lclangAST -lclangASTMatchers -lclangLex -lclangBasic
