#
# AX_CLANG([ACTION-IF-FOUND,[ACTION-IF-NOT-FOUND]])
#
# If both clang and clang++ are detected
# - Sets shell variable HAVE_CLANG='yes'
# - If ACTION-IF-FOUND is defined: Runs ACTION-IF-FOUND
# - If ACTION-IF-FOUND is undefined:
#   - Runs AC_SUBST for variables CLANG and CLANGXX, setting them to the
#     corresponding paths.
#
# If not both clang and clang++ are detected
# - Sets shell variable HAVE_CLANG='no'
# - Runs ACTION-IF-NOT-FOUND if defined
#

AC_DEFUN([AX_CLANG],
[
  ax_clang_ok='yes'
  CLANG=''
  CLANGXX=''
  CLANG_VERSION_NO_MINOR=7 # From clang-7 onwards, no minor version is printed
  AC_ARG_WITH([clang],
              [AS_HELP_STRING([--with-clang=PATH],[Specify path to clang, as it will be called for C program analysis.])],
              [if test "x$withval" != "xyes"; then
                 CLANG="$withval"
                 AC_MSG_CHECKING([for clang])
                 AC_MSG_RESULT([$CLANG])
               fi
              ],
              [])
  AC_ARG_WITH([clangxx],
              [AS_HELP_STRING([--with-clangxx=PATH],[Specify path to clang++, as it will be called for C++ program analysis.])],
              [if test "x$withval" != "xyes"; then
                 CLANGXX="$withval"
                 AC_MSG_CHECKING([for clang++])
                 AC_MSG_RESULT([$CLANGXX])
               fi
              ],
              [])

  ax_clang_llvm_clang_version_check='yes'
  AC_ARG_ENABLE([llvm-clang-version-check],
                [AS_HELP_STRING([--disable-llvm-clang-version-check],[Allow build to proceed despite mismatching versions of LLVM and clang/clang++.])],
                [if test "x$enableval" = "xno"; then
                   ax_clang_llvm_clang_version_check='no'
                 elif test "x$enableval" != "xyes"; then
                   AC_MSG_WARN([Unknown value given to --enable-llvm-clang-version-check. Defaulting to enabled.])
                 fi],[])

  if test "x$CLANG" = "x"; then
    if test "x$LLVMVERSION" != "x"; then
      ax_clang_llvmversion=`echo "$LLVMVERSION" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
      ax_clang_llvmversion_major=`echo $ax_clang_llvmversion | cut -d '.' -f1`
      if test "$ax_clang_llvmversion_major" -ge "$CLANG_VERSION_NO_MINOR"; then
        ax_clang_llvmversion="$ax_clang_llvmversion_major"
      fi
      AC_PATH_PROG([CLANG],[clang-$ax_clang_llvmversion])
    fi
  fi
  if test "x$CLANG" = "x"; then
    AC_PATH_PROG([CLANG],[clang])
  fi

  if test "x$CLANG" = "x"; then
    ax_clang_ok='no'
  else
    ax_clang_clangversion=`$CLANG --version`
    if test "x$?" = "x0"; then
      if test "x$ax_clang_llvm_clang_version_check" = "xyes"; then
        if test "x$LLVMVERSION" != "x"; then
          ax_clang_llvmversion=`echo "$LLVMVERSION" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
          ax_clang_clangversion=`echo "$ax_clang_clangversion" | tr '\n' ' ' | sed 's/^[[^0-9]]*\([[0-9]][[0-9.]]*[[0-9]]\).*$/\1/g'`
          ax_clang_clangversion=`echo "$ax_clang_clangversion" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
          if test "x$ax_clang_llvmversion" != "x$ax_clang_clangversion"; then
            AC_MSG_WARN([Failure: clang version ($ax_clang_clangversion) differs from llvm version ($ax_clang_llvmversion).])
            AC_MSG_WARN([Consider indicating the correct version of clang using the switch --with-clang,])
            AC_MSG_WARN([or disable the version check with --disable-llvm-clang-version-check.])
            ax_clang_ok='no'
          fi
        fi
      fi
    else
      ax_clang_ok='no'
    fi
  fi

  if test "x$CLANGXX" = "x"; then
    if test "x$LLVMVERSION" != "x"; then
      ax_clang_llvmversion=`echo "$LLVMVERSION" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
      ax_clang_llvmversion_major=`echo $ax_clang_llvmversion | cut -d '.' -f1`
      if test "$ax_clang_llvmversion_major" -ge "$CLANG_VERSION_NO_MINOR"; then
        ax_clang_llvmversion="$ax_clang_llvmversion_major"
      fi
      AC_PATH_PROG([CLANGXX],[clang++-$ax_clang_llvmversion])
    fi
  fi
  if test "x$CLANGXX" = "x"; then
    AC_PATH_PROG([CLANGXX],[clang++])
  fi

  if test "x$CLANGXX" = "x"; then
    ax_clang_ok='no'
  else
    ax_clang_clangxxversion=`$CLANGXX --version`
    if test "x$?" = "x0"; then
      if test "x$ax_clang_llvm_clang_version_check" = "xyes"; then
        if test "x$LLVMVERSION" != "x"; then
          ax_clang_llvmversion=`echo "$LLVMVERSION" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
          ax_clang_clangxxversion=`echo "$ax_clang_clangxxversion" | tr '\n' ' ' | sed 's/^[[^0-9]]*\([[0-9]][[0-9.]]*[[0-9]]\).*$/\1/g'`
          ax_clang_clangxxversion=`echo "$ax_clang_clangxxversion" | sed 's/\([[0-9]]*\.[[0-9]]*\)\.[[0-9]]*/\1/g'`
          if test "x$ax_clang_llvmversion" != "x$ax_clang_clangxxversion"; then
            AC_MSG_WARN([Failure: clang++ version ($ax_clang_clangxxversion) differs from llvm version ($ax_clang_llvmversion).])
            AC_MSG_WARN([Consider indicating the correct version of clang++ using the switch --with-clangxx,])
            AC_MSG_WARN([or disable the version check with --disable-llvm-clang-version-check.])
            ax_clang_ok='no'
          fi
        fi
      fi
    else
      ax_clang_ok='no'
    fi
  fi

  if test "x$ax_clang_ok" = "xyes"; then
    ifelse([$1],,
           [AC_SUBST(CLANG)
            AC_SUBST(CLANGXX)
            AC_DEFINE_UNQUOTED([CLANGPATH],["$CLANG"],[Path of clang executable.])
            :
           ],
           [$1])
  else
    ifelse([$2], , :,[$2])
  fi

])
