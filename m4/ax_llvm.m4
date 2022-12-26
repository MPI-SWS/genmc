#
# AX_LLVM([ACTION-IF-FOUND,[ACTION-IF-NOT-FOUND]])
#
# If LLVM is successfully detected
# - Sets shell variable HAVE_LLVM='yes'
# - If ACTION-IF-FOUND is defined: Runs ACTION-IF-FOUND
# - If ACTION-IF-FOUND is undefined:
#   - Defines HAVE_LLVM.
#   - Defines LLVM_VERSION.
#   - Defines LLVM_BUILDMODE
#   - Defines LLVM_NDEBUG iff the LLVM library was compiled with NDEBUG.
#   - Sets shell variable LLVM_NDEBUG to 'yes' or 'no' correspondingly.
#
# If LLVM is not detected:
# - Sets shell variable HAVE_LLVM='no'
# - Runs ACTION-IF-NOT-FOUND if defined
#

AC_DEFUN([AX_LLVM],
[
  LLVMSEARCHPATH=$PATH
  AC_ARG_WITH([llvm],
              [AS_HELP_STRING([--with-llvm=PATH],[Specify path to root of llvm installation.])],
              [if test "x$withval" != "xyes"; then
                 LLVMSEARCHPATH="${withval%/}/bin"
               fi],
              [])

  ax_llvm_ok='yes'
  old_CXXFLAGS=$CXXFLAGS
  old_LDFLAGS=$LDFLAGS
  old_LIBS=$LIBS

  AC_LANG_PUSH([C++])
  AC_PATH_PROG([LLVMCONFIG],[llvm-config],[no],[$LLVMSEARCHPATH])

  if test "x$LLVMCONFIG" = "xno"; then
    ax_llvm_ok='no'
  fi

  if test "x$ax_llvm_ok" = "xyes"; then

    LLVMVERSION=`$LLVMCONFIG --version`
    LLVMBUILDMODE=`$LLVMCONFIG --build-mode`

    # Is LLVM compiled with NDEBUG?
    LLVMASSERTIONMODE=`$LLVMCONFIG --assertion-mode 2>/dev/null`
    if test "x$?" = "x0"; then
      # Query to llvm-config was successful, assign LLVM_NDEBUG accordingly
      if test "x$LLVMASSERTIONMODE" = "xON" ; then
        LLVM_NDEBUG='no'
      else
        LLVM_NDEBUG='yes'
      fi
    else
      # Query to llvm-config failed (probably too old version)
      # Check it the version string reveals assertions
      if test "x`echo $LLVMBUILDMODE | grep 'Asserts'`" = "x"; then
        LLVM_NDEBUG='yes'
      else
        LLVM_NDEBUG='no'
      fi
    fi

    # Get LLVM flags without the standard (we may want to use a newer one)
    LLVMCXXFLAGS=`$LLVMCONFIG --cxxflags`
    LLVMCXXFLAGS=`echo "$LLVMCXXFLAGS" | sed -E 's/-std=[[^[:space:]]]+//g'`

    CXXFLAGS="$CXXFLAGS $LLVMCXXFLAGS"
    LLVMLDFLAGS=`$LLVMCONFIG --ldflags`
    LLVMLIBS=`$LLVMCONFIG --libs`
    SYSLIBS=`$LLVMCONFIG --system-libs 2>/dev/null`
    if test "x$?" = "x0"; then
      LLVMLDFLAGS="$LLVMLDFLAGS $SYSLIBS"
    fi
    # For each library that LLVM adds to LDFLAGS, check that it is found
    OLDLDFLAGS="$LDFLAGS"
    for lib in $LLVMLDFLAGS; do
      if test "x`echo $lib | grep '^-l'`" != "x"; then
        lname=`echo "$lib" | sed s/^-l//`
        LDFLAGS="$OLDLDFLAGS $lib"
        AC_MSG_CHECKING([for library $lib (required by LLVM)])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
          [AC_MSG_RESULT([yes])],
          [AC_MSG_RESULT([no])
           AC_MSG_FAILURE([Failed to find library $lib which is required by LLVM.])])
      fi
    done
    LDFLAGS="$OLDLDFLAGS $LLVMLDFLAGS"
    LIBS="$LIBS $LLVMLIBS"

    # Get rid of -Wno-maybe-uninitialized from CXXFLAGS, in case it is not accepted by compiler
    if test "x`echo $CXXFLAGS | grep -e -Wno-maybe-uninitialized`" != "x"; then
      OLDCXXFLAGS="$CXXFLAGS"
      CXXFLAGS="-Wno-maybe-uninitialized -Wall -Werror"
      AC_MSG_CHECKING([if compiler accepts -Wno-maybe-uninitialized switch])
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
        [AC_MSG_RESULT([yes])
         CXXFLAGS="$OLDCXXFLAGS"
        ],
        [AC_MSG_RESULT([no])
         CXXFLAGS=`echo "$OLDCXXFLAGS" | sed 's/-Wno-maybe-uninitialized//g'`
        ])
    fi

  fi

  if test "x$ax_llvm_ok" = "xyes"; then
    AC_MSG_CHECKING([linking with LLVM])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
]],[[
  llvm::dbgs() << "Successfully linked.\n";
]])],
            [AC_MSG_RESULT([yes])],
            [AC_MSG_RESULT([no])
             ax_llvm_ok='no'
            ])
    if test "x$ax_llvm_ok" = "xno"; then
      AC_MSG_NOTICE([Trying another way of calling llvm-config.])
      LIBS="$LIBS $LLVMLDFLAGS"
      AC_MSG_CHECKING([linking with LLVM])
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <llvm/Support/Debug.h>
#include <llvm/Support/raw_ostream.h>
]],[[
  llvm::dbgs() << "Successfully linked.\n";
]])],
              [AC_MSG_RESULT([yes])
               ax_llvm_ok='yes'],
              [AC_MSG_RESULT([no])
               ax_llvm_ok='no'])
    fi
  fi

  AC_LANG_POP([C++])
  AC_MSG_CHECKING([for LLVM])
  if test "x$ax_llvm_ok" = "xyes"; then
    AC_MSG_RESULT([$LLVMVERSION ($LLVMBUILDMODE)])
    ifelse([$1],,[AC_DEFINE([HAVE_LLVM],[1],[Define if there is a working LLVM library.])
                  AC_DEFINE_UNQUOTED([LLVM_VERSION],["$LLVMVERSION"],[Version of the LLVM library.])
                  AC_DEFINE_UNQUOTED([LLVM_BUILDMODE],["$LLVMBUILDMODE"],[Build mode of the LLVM library.])
                  if test "x$LLVM_NDEBUG" = "xyes"; then
                    AC_DEFINE([LLVM_NDEBUG],[1],[Define if LLVM was compiled with NDEBUG.])
                  fi
                 ],
                 [$1])
    HAVE_LLVM='yes'
  else
    AC_MSG_RESULT([no])
    CXXFLAGS=$old_CXXFLAGS
    LDFLAGS=$old_LDFLAGS
    LIBS=$old_LIBS
    $2
    HAVE_LLVM='no'
  fi
])
