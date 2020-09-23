#
# AX_GIT_COMMIT
#
# If git is detected and a repository is found
# - Sets GIT_COMMIT to the hash of the commit corresponding
#   to HEAD in the build repository
#

AC_DEFUN([AX_GIT_COMMIT],
[
  GIT_COMMIT=''
  ax_git_commit_ok='yes'
  AC_PATH_PROG([GIT], [git], [no])

  if test "x$GIT" = "xno"
  then
    ax_git_commit_ok='no'
    AC_MSG_NOTICE([Unable to find git -- commit number cannot be determined])
  fi

  if test "x$ax_git_commit_ok" = "xyes"
  then
    AC_MSG_CHECKING([the git commit of build repository])

    GIT_COMMIT=`$GIT rev-parse --short HEAD 2>/dev/null`
    if test "$?" -ne 0
    then
      ax_git_commit_ok='no'
      AC_MSG_RESULT([no])
    else
      AC_MSG_RESULT([$GIT_COMMIT])
      AC_DEFINE_UNQUOTED([GIT_COMMIT], ["$GIT_COMMIT"],
  		         [The hash of HEAD in the build repository])
    fi
  fi
])
