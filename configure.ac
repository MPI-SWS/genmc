AC_INIT([genmc], [0.2], [michalis@mpi-sws.org])

m4_include([m4/ax_llvm.m4])
m4_include([m4/ax_clang.m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

AC_LANG([C++])

AX_LLVM(,[AC_MSG_FAILURE(LLVM is required.)])
AX_CLANG(,[
  AC_MSG_WARN([genmc cannot be used without clang/clang++.])
  AC_MSG_WARN([Consider indicating the binaries clang/clang++ with switches --with-clang/--with-clangxx,])
  AC_MSG_FAILURE([Failed to detect clang/clang++.])
])

## Checks for libraries
AC_CHECK_LIB([dl], [dlopen],[],[AC_MSG_FAILURE([Could not find library libdl.])])
AC_CHECK_LIB([ffi], [ffi_call],[],[AC_MSG_FAILURE([Could not find library libffi.])])

## XXX: List below is currently INCOMPLETE
AC_CHECK_HEADERS([\
  stdlib.h \
  clang/Driver/Job.h \
  llvm/ExecutionEngine/ExecutionEngine.h \
  llvm/ExecutionEngine/GenericValue.h \
  llvm/Pass.h \
  llvm/Support/ErrorHandling.h
],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])

## Checks for header files in alternative locations
AC_DEFUN([AC_CHECK_HEADERS_ALT],
[
  ac_check_headers_alt_ok="no"
  AC_CHECK_HEADERS([$1],[ac_check_headers_alt_ok="yes"],[],[$4])
  if test "x$ac_check_headers_alt_ok" = "xyes"; then
    $2
    :
  else
    $3
    :
  fi
])

AC_CHECK_HEADERS_ALT([llvm/DataLayout.h llvm/IR/DataLayout.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/DebugInfo.h llvm/Analysis/DebugInfo.h llvm/IR/DebugInfo.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/DerivedTypes.h llvm/IR/DerivedTypes.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/Function.h llvm/IR/Function.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/InstVisitor.h llvm/IR/InstVisitor.h llvm/Support/InstVisitor.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/Module.h llvm/IR/Module.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/LLVMContext.h llvm/IR/LLVMContext.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/PassManager.h llvm/IR/PassManager.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS_ALT([llvm/Transforms/Utils.h llvm/Transforms/Scalar.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])

AC_CHECK_HEADERS([ffi/ffi.h],[],[
  AC_CHECK_HEADERS([ffi.h],[],[AC_MSG_FAILURE([Could not find header ffi.h.])],[AC_INCLUDES_DEFAULT])
],[AC_INCLUDES_DEFAULT])

# Check various functions in LLVM

## Check whether llvm::cl has HideUnrelatedOptions
AC_MSG_CHECKING([whether llvm::cl has HideUnrelatedOptions()])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/CommandLine.h>
]],[[
	llvm::cl::OptionCategory clGeneral("A category");
	const llvm::cl::OptionCategory *cats[] = { &clGeneral };
	llvm::cl::HideUnrelatedOptions(cats);
]])],
        [AC_DEFINE([LLVM_HAS_HIDE_UNRELATED_OPTS],[1],
         [Define if llvm::cl has HideUnrelatedOptions().])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check whether llvm::cl has ResetCommandLineParser()
AC_MSG_CHECKING([whether llvm::cl has ResetCommandLineParser()])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/CommandLine.h>
]],[[
	llvm::cl::ResetCommandLineParser();
]])],
        [AC_DEFINE([LLVM_HAS_RESET_COMMANDLINE_PARSER],[1],
         [Define if llvm::cl has ResetCommandLineParser().])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])


## Check whether llvm::cl::values needs to be null terminated
AC_MSG_CHECKING([whether llvm::cl::values need a sentinel])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/CommandLine.h>
]],[[
  enum Unit{
    UNIT
  };
  auto v = llvm::cl::values(clEnumVal(UNIT, "An option"), clEnumValEnd);
]])],
        [AC_DEFINE([LLVM_CL_VALUES_NEED_SENTINEL],[1],
         [Define if llvm::cl::values need a sentinel.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check if AttributeSet is renamed to AttributeList
AC_MSG_CHECKING([whether llvm::AttributeSet is renamed to llvm::AttributeList])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#elif defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#endif
]],[[
  llvm::Module *M = nullptr;
  llvm::AttributeList assumeAttrs =
    llvm::AttributeList::get(M->getContext(),llvm::AttributeList::FunctionIndex,
                             std::vector<llvm::Attribute::AttrKind>({llvm::Attribute::NoUnwind}));
]])],
        [AC_DEFINE([LLVM_HAS_ATTRIBUTELIST],[1],
         [Define if llvm::AttributeList exists (and not AttributeSet).])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check whether llvm::SwitchInst::CaseIt needs dereference (>= 4.0)
AC_MSG_CHECKING([whether llvm::SwitchInst::CaseIt needs dereference])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/IR/Instructions.h>
]],[[
  llvm::SwitchInst::CaseIt *i = nullptr;
  (*i)->getCaseValue();
]])],
        [AC_DEFINE([LLVM_SWITCHINST_CASEIT_NEEDS_DEREF],[1],
         [Define if llvm::SwitchInst::CaseIt should be dereferenced.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])


## Check whether the class llvm::TerminatorInst exists (LLVM < 8)
AC_MSG_CHECKING([for llvm::TerminatorInst])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/IR/Instructions.h>
]],[[
  llvm::TerminatorInst *I = nullptr;
]])],
        [AC_DEFINE([LLVM_HAS_TERMINATORINST],[1],
         [Define if the class llvm::TerminatorInst exists.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check which method in llvm::Module returns the data layout string
AC_MSG_CHECKING([for llvm::Module::getDataLayoutStr])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#elif defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#endif
#if defined(HAVE_LLVM_LLVMCONTEXT_H)
#include <llvm/LLVMContext.h>
#elif defined(HAVE_LLVM_IR_LLVMCONTEXT_H)
#include <llvm/IR/LLVMContext.h>
#endif
]],[[
          llvm::Module *M = 0;
          std::string s = M->getDataLayoutStr();
        ]])],
        [AC_DEFINE([LLVM_MODULE_GET_DATA_LAYOUT_STRING],[getDataLayoutStr],
            [The name of the method in llvm::Module which returns the data layout string.])
         AC_MSG_RESULT([getDataLayoutStr])
        ],
        [AC_DEFINE([LLVM_MODULE_GET_DATA_LAYOUT_STRING],[getDataLayout],
            [The name of the method in llvm::Module which returns the data layout string.])
         AC_MSG_RESULT([getDataLayout])
        ])

## Check the signature of llvm::Module::materializeAllPermanently
AC_MSG_CHECKING([for llvm::Module::MaterializeAllPermanently])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#elif defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#endif
#if defined(HAVE_LLVM_LLVMCONTEXT_H)
#include <llvm/LLVMContext.h>
#elif defined(HAVE_LLVM_IR_LLVMCONTEXT_H)
#include <llvm/IR/LLVMContext.h>
#endif
]],[[
  llvm::Module *M = 0;
  std::error_code ec = M->materializeAllPermanently();
]])],
           [AC_DEFINE([LLVM_MODULE_MATERIALIZE_ALL_PERMANENTLY_ERRORCODE_BOOL],[1],
            [Define if Module::materializeAllPermanently accepts zero arguments and returns an error_code.])
            AC_MSG_RESULT([(bool) -> error_code])
           ],
           [
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#elif defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#endif
]],[[
  llvm::Module *M = 0;
  std::string *s = 0;
  bool b = M->MaterializeAllPermanently(s);
]])],
           [AC_DEFINE([LLVM_MODULE_MATERIALIZE_ALL_PERMANENTLY_BOOL_STRPTR],[1],
            [Define if Module::MaterializeAllPermanenty has signature std::string* -> bool])
            AC_MSG_RESULT([string* -> bool])],
           [AC_MSG_RESULT([use materializeAll instead])
	    ## Check the signature of llvm::Module::materializeAll
	    AC_MSG_CHECKING([the signature of llvm::Module::materializeAll])
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/Error.h>
#if defined(HAVE_LLVM_MODULE_H)
#include <llvm/Module.h>
#elif defined(HAVE_LLVM_IR_MODULE_H)
#include <llvm/IR/Module.h>
#endif
]],[[
  llvm::Module *M = 0;
  llvm::Error Err = M->materializeAll();
]])],
           [AC_DEFINE([LLVM_MODULE_MATERIALIZE_ALL_LLVM_ERROR],[1],
            [Define if Module::materializeAll has signature () -> llvm::Error])
            AC_MSG_RESULT([() -> llvm::Error])],
           [AC_MSG_RESULT([() -> llvm::error_code])])
])])

## Check if the value F_None in llvm::sys::fs::OpenFlags exists
AC_MSG_CHECKING([for llvm::sys::fs::OpenFlags (F_None)])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/FileSystem.h>
]],[[
  llvm::sys::fs::OpenFlags f = llvm::sys::fs::F_None;
  return f;
]])],
           [AC_DEFINE([HAVE_LLVM_SYS_FS_OPENFLAGS],[1],
            [Define if the type llvm::sys::fs::OpenFlags exists.])
            AC_MSG_RESULT([yes])],
           [AC_MSG_RESULT([no])])

## Check the type of the error reporting argument for llvm::raw_fd_ostream
AC_MSG_CHECKING([for error reporting in llvm::raw_fd_ostream])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
]],[[
  std::string errs;
#ifdef HAVE_LLVM_SYS_FS_OPENFLAGS
  llvm::raw_fd_ostream("foo",errs,llvm::sys::fs::F_None);
#else
  llvm::raw_fd_ostream("foo",errs,0);
#endif
]])],
        [AC_DEFINE([LLVM_RAW_FD_OSTREAM_ERR_STR],[1],
         [Define if llvm::raw_fd_ostream reports errors through a string reference.])
         AC_MSG_RESULT([string])],
        [AC_MSG_RESULT([error_code])])

## Check whether an ExecutionEngine takes a Module pointer or a Module unique_ptr
AC_MSG_CHECKING([for unique_ptr<Module> in ExecutionEngine])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/ExecutionEngine/ExecutionEngine.h>
]],[[
  class EE : public llvm::ExecutionEngine {
  public:
    EE(llvm::Module *M) : llvm::ExecutionEngine(std::unique_ptr<llvm::Module>(M)) {};
  };
]])],
        [AC_DEFINE([LLVM_EXECUTIONENGINE_MODULE_UNIQUE_PTR],[1],
         [Define if llvm::ExecutionEngine takes a unique_ptr to its Module.])
         AC_MSG_RESULT([yes])],
       	[AC_MSG_RESULT([no])])

## Check if Mutexes use the method "lock" or "acquire"
AC_MSG_CHECKING([for llvm::sys::Mutex::lock])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/Mutex.h>
]],[[
  llvm::sys::Mutex m;
  m.lock();
]])],
        [AC_DEFINE([LLVM_SYS_MUTEX_LOCK_FN],[lock],
         [The name of the locking method of llvm::sys::mutex.])
         AC_DEFINE([LLVM_SYS_MUTEX_UNLOCK_FN],[unlock],
         [The name of the unlocking method of llvm::sys::mutex.])
         AC_MSG_RESULT([lock])],
        [AC_DEFINE([LLVM_SYS_MUTEX_LOCK_FN],[acquire],
         [The name of the locking method of llvm::sys::mutex.])
         AC_DEFINE([LLVM_SYS_MUTEX_UNLOCK_FN],[release],
         [The name of the unlocking method of llvm::sys::mutex.])
         AC_MSG_RESULT([acquire])])

## Check the signature of llvm::ParseIR
AC_MSG_CHECKING([for llvm::ParseIR])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#if defined(HAVE_LLVM_LLVMCONTEXT_H)
#include <llvm/LLVMContext.h>
#elif defined(HAVE_LLVM_IR_LLVMCONTEXT_H)
#include <llvm/IR/LLVMContext.h>
#endif
]],[[
  llvm::SMDiagnostic err;
  llvm::LLVMContext ctx;
  llvm::ParseIR((llvm::MemoryBuffer*)0,
                err,
                ctx);
]])],
        [AC_DEFINE([LLVM_PARSE_IR_MEMBUF_PTR],[1],
         [Define if llvm::ParseIR takes a MemoryBuffer pointer as argument.])
         AC_MSG_RESULT([takes MemoryBuffer*])],
        [AC_MSG_RESULT([takes MemoryBufferRef])])

## Check the return type of llvm::MemoryBuffer::getMemBuffer
AC_MSG_CHECKING([for return type of llvm::MemoryBuffer::getMemBuffer])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/MemoryBuffer.h>
]],[[
  llvm::MemoryBuffer *b =
    llvm::MemoryBuffer::getMemBuffer("");
]])],
        [AC_DEFINE([LLVM_GETMEMBUFFER_RET_PTR],[1],
         [Define if llvm::MemoryBuffer::getMemBuffer returns a pointer.])
         AC_MSG_RESULT([llvm::MemoryBuffer*])],
        [AC_MSG_RESULT([std::unique_ptr<llvm::MemoryBuffer>])])

## Check which API is used by llvm:gep_type_iterator
AC_MSG_CHECKING([whether llvm:gep_type_iterator uses the new API])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/IR/GetElementPtrTypeIterator.h>
]],[[
  llvm::gep_type_iterator *it = nullptr;
  llvm::StructType *STy = it->getStructTypeOrNull();
]])],
        [AC_DEFINE([LLVM_NEW_GEP_TYPE_ITERATOR_API],[1],
         [Define if llvm:gep_type_iterator uses the new API.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check the type of list_type items
AC_MSG_CHECKING([for the type of list_type items])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <clang/Driver/Job.h>
]],[[
	class Cmd : public clang::driver::Job {
	      void Print(llvm::raw_ostream &OS, const char *Terminator,
                         bool Quote, bool CrashReport = false) { };
	};
 ]])],
        [AC_DEFINE([CLANG_LIST_TYPE_JOB_PTR],[1],
         [Define if list_type is a SmallVector of Job*.])
         AC_MSG_RESULT([Job*])],
        [AC_MSG_RESULT([std::unique_ptr<Command>])])

## Check how to set a compiler invocation
AC_MSG_CHECKING([how to set a CompilerInvocation])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/CompilerInstance.h>
]],[[
	clang::CompilerInvocation *inv = nullptr;
	clang::CompilerInstance Clang;
	Clang.setInvocation(inv);
 ]])],
        [AC_DEFINE([CLANG_COMPILER_INVOCATION_PTR],[1],
         [Define if setInvocation takes a simple CompilerInvocation*.])
         AC_MSG_RESULT([CompilerInvocation*])],
        [AC_MSG_RESULT([std::shared_ptr<CompilerInvocation>])])

## Check whether getPassName() should return a StringRef
AC_MSG_CHECKING([whether Pass::getPassName() returns a StringRef])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Pass.h>
]],[[
	class LPass : public llvm::Pass {
	      virtual llvm::StringRef getPassName() const { return ""; };
	};
]])],
        [AC_DEFINE([LLVM_PASS_GETPASSNAME_IS_STRINGREF],[1],
         [Define if Pass::getPassName() returns a StringRef.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check whether llvm::Loop::getName exists (>= llvm 4)
AC_MSG_CHECKING([whether llvm::Loop::getName exists])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Analysis/LoopInfo.h>
]],[[
  llvm::Loop * L = 0;
  L->getName();
]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([LLVM_HAVE_LOOP_GET_NAME],[1],
   [Define if llvm::LoopInfo::getName exists])],
  [AC_MSG_RESULT([no])])

## Check whether llvm::LoopInfo::erase exists (> llvm 4)
AC_MSG_CHECKING([whether llvm::LoopInfo::erase exists])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Analysis/LoopPass.h>
]],[[
  llvm::Loop * L = 0;
  llvm::LoopInfo *LI = 0;
  LI->erase(L);
]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([LLVM_HAVE_LOOPINFO_ERASE],[1],
   [Define if llvm::LoopInfo::erase exists])],
  [AC_MSG_RESULT([no])])

## Check how to call getAnalysis to get LoopInfo
AC_MSG_CHECKING([how to get LoopInfo analysis])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Analysis/LoopPass.h>
]],[[
  llvm::LPPassManager *LPM = 0;
  LPM->getAnalysis<llvm::LoopInfo>();
]])],
        [AC_MSG_RESULT([llvm::LoopInfo])
         AC_DEFINE([LLVM_GET_ANALYSIS_LOOP_INFO],[1],
         [Define if LPPassManager::getAnalysis<LoopInfo> can be called.])],
        [AC_MSG_RESULT([llvm::LoopInfoWrapperPass])])

## Check if llvm::cl::getRegisteredOptions takes an argument
AC_MSG_CHECKING([if llvm::cl::getRegisteredOptions takes an argument.])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/CommandLine.h>
]],[[
  llvm::StringMap<llvm::cl::Option*> opts;
  llvm::cl::getRegisteredOptions(opts);
]])],
        [AC_MSG_RESULT([yes])
         AC_DEFINE([LLVM_CL_GETREGISTEREDOPTIONS_TAKES_ARGUMENT],[1],
         [Define if llvm::cl::getRegisteredOptions takes an argument.])],
        [AC_MSG_RESULT([no])])

## Check the parameter type for ExecutionEngine::runFunction
AC_MSG_CHECKING([for parameter type for ExecutionEngine::runFunction])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
]],[[
  class EE : public llvm::ExecutionEngine{
  public:
    virtual llvm::GenericValue runFunction(llvm::Function*, const std::vector<llvm::GenericValue>&) override {
      return llvm::GenericValue();
    };
  };
]])],
        [AC_MSG_RESULT([std::vector<llvm::GenericValue>])
         AC_DEFINE([LLVM_EXECUTION_ENGINE_RUN_FUNCTION_VECTOR],[1],
         [Define if ExecutionEngine::runFunction takes a std::vector<llvm::GenericValue> argument.])],
        [AC_MSG_RESULT([llvm::ArrayRef<llvm::GenericValue>])])

## Check if llvm::PassManager requires a template argument
AC_MSG_CHECKING([if llvm::PassManager requires a template argument])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_PASSMANAGER_H)
#include <llvm/PassManager.h>
#elif defined(HAVE_LLVM_IR_PASSMANAGER_H)
#include <llvm/IR/PassManager.h>
#endif
]],[[
  llvm::PassManager PM;
]])],
        [AC_MSG_RESULT([no])],
        [AC_MSG_RESULT([yes])
         AC_DEFINE([LLVM_PASSMANAGER_TEMPLATE],[1],
         [Define if llvm::PassManager requires a template argument.])
         AC_CHECK_HEADERS([llvm/IR/LegacyPassManager.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])])

## Check if we can get metadata for global variables (LLVM > 4)
AC_MSG_CHECKING([whether getMetadata() for GlobalObject exists])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/GlobalObject.h>
]],[[
          llvm::GlobalObject *o = nullptr;
          llvm::MDNode *md = o->getMetadata("dbg");
        ]])],
        [AC_DEFINE([LLVM_HAS_GLOBALOBJECT_GET_METADATA],[1],
            [Define if we can get metadata from a GlobalObject.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Checking if an llvm::DILocation is an llvm::MDNode
AC_MSG_CHECKING([if an llvm::DILocation is an llvm::MDNode])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_DEBUGINFO_H)
#include <llvm/DebugInfo.h>
#elif defined(HAVE_LLVM_IR_DEBUGINFO_H)
#include <llvm/IR/DebugInfo.h>
#elif defined(HAVE_LLVM_ANALYSIS_DEBUGINFO_H)
#include <llvm/Analysis/DebugInfo.h>
#endif
]],[[
  llvm::MDNode *m = 0;
  llvm::DILocation *l = static_cast<llvm::DILocation*>(m);
]])],
        [AC_MSG_RESULT([yes])
         AC_DEFINE([LLVM_DILOCATION_IS_MDNODE],[1],
         [Define if an llvm::DILocation is an MDNode.])],
        [AC_MSG_RESULT([no])])

## Check if the datalayout of an execution engine is a pointer
AC_MSG_CHECKING([whether DataLayout of ExecutionEngine is a pointer])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/ExecutionEngine/ExecutionEngine.h>
]],[[
  llvm::ExecutionEngine *E = 0;
  const llvm::DataLayout *DL = E->getDataLayout();
]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([LLVM_EXECUTIONENGINE_DATALAYOUT_PTR],[1],
   [Define if the DataLayout field of ExecutionEngine is a pointer.])],
  [AC_MSG_RESULT([no])])

## Check if llvm::LoopInfo::markAsRemoved exists
AC_MSG_CHECKING([for llvm::LoopInfo::markAsRemoved])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Analysis/LoopPass.h>
]],[[
  llvm::Loop * L = 0;
  llvm::LoopInfo *LI = 0;
  LI->markAsRemoved(L);
]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([LLVM_HAVE_LOOPINFO_MARK_AS_REMOVED],[1],
   [Define if llvm::LoopInfo::markAsRemoved exists])],
  [AC_MSG_RESULT([no])])

## Check if initializeIPA exists
AC_MSG_CHECKING([for llvm::initializeIPA])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(HAVE_LLVM_ANALYSIS_VERIFIER_H)
#include <llvm/Analysis/Verifier.h>
#elif defined(HAVE_LLVM_IR_VERIFIER_H)
#include <llvm/IR/Verifier.h>
#endif
#if defined(HAVE_LLVM_PASSMANAGER_H)
#include <llvm/PassManager.h>
#elif defined(HAVE_LLVM_IR_PASSMANAGER_H)
#include <llvm/IR/PassManager.h>
#endif
#if defined(HAVE_LLVM_IR_LEGACYPASSMANAGER_H) && defined(LLVM_PASSMANAGER_TEMPLATE)
#include <llvm/IR/LegacyPassManager.h>
#endif
]],[[
  void (*f)(llvm::PassRegistry&) = llvm::initializeIPA;
]])],
  [AC_MSG_RESULT([yes])
   AC_DEFINE([HAVE_LLVM_INITIALIZE_IPA],[1],
   [Define if llvm::initializeIPA exists])],
  [AC_MSG_RESULT([no])])


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
