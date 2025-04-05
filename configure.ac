AC_INIT([GenMC], [0.11.0], [michalis.kokologiannakis@inf.ethz.ch], [genmc], [https://plv.mpi-sws.org/genmc])

m4_include([m4/ax_llvm.m4])
m4_include([m4/ax_clang.m4])
m4_include([m4/ax_git_commit.m4])
m4_include([m4/ax_cxx_compile_stdcxx.m4])
m4_include([m4/ax_prog_doxygen.m4])
m4_include([m4/gcov.m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AM_SILENT_RULES([yes])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([23], [noext], [mandatory])

AC_PROG_RANLIB
AM_PROG_AR

AC_LANG([C++])

AX_LLVM(,[AC_MSG_FAILURE(LLVM is required.)])
AX_CLANG(,[
  AC_MSG_WARN([genmc cannot be used without clang/clang++.])
  AC_MSG_WARN([Consider indicating the binaries clang/clang++ with switches --with-clang/--with-clangxx,])
  AC_MSG_FAILURE([Failed to detect clang/clang++.])
])
AX_GIT_COMMIT

## Check for libraries
AC_CHECK_LIB([dl], [dlopen],[],[AC_MSG_FAILURE([Could not find library libdl.])])
AC_CHECK_LIB([ffi], [ffi_call],[],[AC_MSG_FAILURE([Could not find library libffi.])])
AC_CHECK_LIB([hwloc], [hwloc_topology_init], [], [])

## Check for (proper) C++11 stdlibc++ support
AC_MSG_CHECKING([whether enum class can be used as unordered_map<> key])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <unordered_map>
]],[[
	enum class Foo {};

	std::unordered_map<Foo, int> bar;
 ]])],
        [AC_DEFINE([STDLIBCPP_SUPPORTS_ENUM_MAP_KEYS],[1],
         [Define if stdlibc++ supports enum class as keys to unordered_map<>.])
         AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])])

## Check for LLVM headers
## XXX: List below is currently INCOMPLETE
AC_CHECK_HEADERS([\
  stdlib.h \
  llvm/ExecutionEngine/ExecutionEngine.h \
  llvm/IR/LegacyPassManager.h \
],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])

## Checks for headers in alternative locations (useful for LLVM headers)
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

AC_CHECK_HEADERS_ALT([llvm/Transforms/Utils.h llvm/Transforms/Scalar.h],[],[AC_MSG_FAILURE([Could not find necessary headers.])],[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([ffi/ffi.h],[],[
  AC_CHECK_HEADERS([ffi.h],[],[AC_MSG_FAILURE([Could not find header ffi.h.])],[AC_INCLUDES_DEFAULT])
],[AC_INCLUDES_DEFAULT])

## Enable build of doxygen-generated documentation
DX_DOXYGEN_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(genmc, [doc/Doxyfile], [doc/doxygen])

## Enable coverage checks
AC_TDD_GCOV
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

## Enable lint checks
AC_PATH_PROG([CPPLINT], [clang-tidy], [no])
AM_CONDITIONAL([LINT], [test x$CPPLINT != "xno"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
