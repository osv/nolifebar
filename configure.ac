AC_INIT([dzen2-status-bar], [1.0], [olexandr.syd@email.com])
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([X11], [x11])
PKG_CHECK_MODULES([XFT], [xft])

# merge pipe
AC_ARG_ENABLE([dzen2-merge-pipes],
  [AS_HELP_STRING([--disable-dzen2-merge-pipes], [Disable compilation of dzen2-merge-pipes program.
                                                 This program enables you to merge multiple named pipes and format the final output using STDIN])],
  [enable_dzen2_merge_pipes=$enableval],
  [enable_dzen2_merge_pipes=yes]
)

AM_CONDITIONAL([ENABLE_DZEN2_MERGE_PIPES], [test "x$enable_dzen2_merge_pipes" = "xyes"])

if test "$enable_dzen2_merge_pipes" = "yes"; then
  AC_DEFINE([ENABLE_DZEN2_MERGE_PIPES], [1], [Define if dzen2_merge_pipes program is enabled])
fi


# fn-xft
AC_ARG_ENABLE([dzen2-fn-xft],
  [AS_HELP_STRING([--disable-dzen2-fn-xft], [Disable compilation of dzen2-fn-xft program.
                                             This program enables you to use multiple fonts in dzen2 (fallback)])],
  [enable_dzen2_fn_xft=$enableval],
  [enable_dzen2_fn_xft=yes]
)

AM_CONDITIONAL([ENABLE_DZEN2_FN_XFT], [test "x$enable_dzen2_fn_xft" = "xyes"])

if test "$enable_dzen2_fn_xft" = "yes"; then
  AC_DEFINE([ENABLE_DZEN2_FN_XFT], [1], [Define if dzen2-fn-xft program is enabled])
fi

# throttle
AC_ARG_ENABLE([dzen2-throttle],
  [AS_HELP_STRING([--disable-dzen2-throttle], [Disable compilation of dzen2-throttle program.
                                             Throttles STDIN])],
  [enable_dzen2_throttle=$enableval],
  [enable_dzen2_throttle=yes]
)

AM_CONDITIONAL([ENABLE_DZEN2_THROTTLE], [test "x$enable_dzen2_throttle" = "xyes"])

if test "$enable_dzen2_throttle" = "yes"; then
  AC_DEFINE([ENABLE_DZEN2_THROTTLE], [1], [Define if dzen2_throttle program is enabled])
fi

# Checks for optional programs.
AC_PATH_PROG([DZEN2], [dzen2])
if test -z "$DZEN2"; then
  AC_MSG_WARN([dzen2 program not found. Please install it (e.g., sudo apt-get install dzen2)])
  dzen2_status="no"
else
  dzen2_status="yes"
fi

AC_CONFIG_FILES([Makefile
                 bin/Makefile
                 lib/dzen2-status-bar/Makefile])

# Generate the configuration script.
AC_OUTPUT

# Print summary
echo ""
echo "Configuration Summary:"
echo "==========================="
echo "Will be compiled binaries: "
echo "  dzen2-merge-pipes : $enable_dzen2_merge_pipes"

if test "$enable_dzen2_fn_xft" = "yes"; then
echo "  dzen2-fn-xft      : $enable_dzen2_fn_xft (You can test it: 'make test-dzen2-fn-xft')"
else
echo "  dzen2-fn-xft      : $enable_dzen2_fn_xft"
fi

echo "  dzen2-throttle    : $enable_dzen2_throttle"
echo ""
echo "Required runtime binaries/apps:"
echo "  dzen2             : ${dzen2_status}"
echo "==========================="
