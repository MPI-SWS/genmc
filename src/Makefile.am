## Make autotools quit complaining about explicit rules
AUTOMAKE_OPTIONS = -Wno-override

# where to install the headers on the system
libgenmc_adir = $(pkgincludedir)

# the list of header files that belong to the library
libgenmc_a_HEADERS = $(top_builddir)/include/*.h

noinst_LIBRARIES = libgenmc.a
libgenmc_a_SOURCES = \
  $(libgenmc_a_HEADERS) \
  Config.cpp Config.hpp \
  DeclareAssumePass.cpp DeclareAssumePass.hpp \
  DeclareEndLoopPass.cpp DeclareEndLoopPass.hpp \
  DefineLibcFunsPass.cpp DefineLibcFunsPass.hpp \
  DriverFactory.cpp DriverFactory.hpp \
  Error.cpp Error.hpp \
  Event.cpp Event.hpp \
  EventLabel.cpp EventLabel.hpp \
  Execution.cpp \
  ExecutionGraph.cpp ExecutionGraph.hpp \
  ExternalFunctions.cpp \
  Interpreter.cpp Interpreter.h \
  IntrinsicLoweringPass.cpp IntrinsicLoweringPass.hpp \
  Library.cpp Library.hpp \
  LLVMModule.cpp LLVMModule.hpp \
  LoopUnrollPass.cpp LoopUnrollPass.hpp \
  Matrix2D.hpp Matrix2D.tcc \
  ModOrder.cpp ModOrder.hpp \
  Parser.cpp Parser.hpp \
  GenMCDriver.cpp GenMCDriver.hpp \
  RC11WeakRADriver.cpp RC11WeakRADriver.hpp \
  RC11WBDriver.cpp RC11WBDriver.hpp \
  RC11MODriver.cpp RC11MODriver.hpp \
  RevisitSet.cpp RevisitSet.hpp \
  SpinAssumePass.cpp SpinAssumePass.hpp \
  VSet.hpp VSet.tcc \
  View.hpp View.cpp

CXX = clang++
AM_CXXFLAGS = -std=c++1z -Wno-unknown-warning-option -DINCLUDE_DIR=\"$(pkgincludedir)/$(pkg)\" -DSRC_INCLUDE_DIR=\"$(abs_top_builddir)/include\"
bin_PROGRAMS = genmc
genmc_SOURCES = main.cpp
genmc_LDADD   = libgenmc.a -lclangCodeGen -lclangFrontend -lclangFrontendTool -lclangDriver -lclangSerialization -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangRewriteFrontend -lclangEdit -lclangAST -lclangLex -lclangBasic
