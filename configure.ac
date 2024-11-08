AC_INIT([nolifebar-status-bar], [1.0], [olexandr.syd@email.com])
AM_INIT_AUTOMAKE([subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [round])

PKG_CHECK_MODULES([X11], [x11])
PKG_CHECK_MODULES([XFT], [xft])
PKG_CHECK_MODULES([XCB],       [xcb],       [have_xcb=yes],       [have_xcb=no])
PKG_CHECK_MODULES([XCB_EWMH],  [xcb-ewmh],  [have_xcb_ewmh=yes],  [have_xcb_ewmh=no])
PKG_CHECK_MODULES([XCB_ICCCM], [xcb-icccm], [have_xcb_icccm=yes], [have_xcb_icccm=no])
PKG_CHECK_MODULES([PULSE],     [libpulse],  [have_pulse=yes],     [have_pulse=no])

# multiplexer
AC_ARG_ENABLE([nolifebar-multiplexer],
  [AS_HELP_STRING([--disable-nolifebar-multiplexer], [Disable compilation of nolifebar-multiplexer program. This program enables you to merge multiple named pipes and format the final output using STDIN])],
  [enable_nolifebar_multiplexer=$enableval],
  [enable_nolifebar_multiplexer=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_MULTIPLEXER], [test "x$enable_nolifebar_multiplexer" = "xyes"])

# fn-xft
AC_ARG_ENABLE([nolifebar-fn-xft],
  [AS_HELP_STRING([--disable-nolifebar-fn-xft], [Disable compilation of nolifebar-fn-xft program. This program enables you to use multiple fonts in nolifebar (fallback)])],
  [enable_nolifebar_fn_xft=$enableval],
  [enable_nolifebar_fn_xft=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_FN_XFT], [test "x$enable_nolifebar_fn_xft" = "xyes"])

# throttle
AC_ARG_ENABLE([nolifebar-throttle],
  [AS_HELP_STRING([--disable-nolifebar-throttle], [Disable compilation of nolifebar-throttle program. Throttles STDIN])],
  [enable_nolifebar_throttle=$enableval],
  [enable_nolifebar_throttle=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_THROTTLE], [test "x$enable_nolifebar_throttle" = "xyes"])

# replacer
AC_ARG_ENABLE([nolifebar-replacer],
  [AS_HELP_STRING([--disable-nolifebar-replacer], [Disable compilation of nolifebar-replacer program. Replace text from STDIN])],
  [enable_nolifebar_replacer=$enableval],
  [enable_nolifebar_replacer=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_REPLACER], [test "x$enable_nolifebar_replacer" = "xyes"])

# xtitle
AC_ARG_ENABLE([nolifebar-xtitle],
  [AS_HELP_STRING([--disable-nolifebar-xtitle], [Disable compilation of nolifebar-xtitle program. Prints title of current window])],
  [enable_nolifebar_xtitle=$enableval],
  [enable_nolifebar_xtitle=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_XTITLE], [test "x$enable_nolifebar_xtitle" = "xyes"])

# pa-status
AC_ARG_ENABLE([nolifebar-pa-status],
  [AS_HELP_STRING([--disable-nolifebar-pa-status], [Disable compilation of nolifebar-pa-status program. Prints volume/mute status by using Pulse Audio])],
  [enable_nolifebar_pa_status=$enableval],
  [enable_nolifebar_pa_status=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_PA_STATUS], [test "x$enable_nolifebar_pa_status" = "xyes"])
if test "$enable_nolifebar_pa_status" = "yes"; then
  test "$have_pulse" = "no" && AC_MSG_ERROR([nolifebar_pa_status program was enabled, but libpulse was not found. You can disable this program if not needed by --disable-nolifebar-pa-status])
fi

# kbd
AC_ARG_ENABLE([nolifebar-kbd],
  [AS_HELP_STRING([--disable-nolifebar-kbd], [Disable compilation of nolifebar-kbd program. Prints keyboard layout and caps/num status])],
  [enable_nolifebar_kbd=$enableval],
  [enable_nolifebar_kbd=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_KBD], [test "x$enable_nolifebar_kbd" = "xyes"])

# xworkspaces
AC_ARG_ENABLE([nolifebar-xworkspaces],
  [AS_HELP_STRING([--disable-nolifebar-xworkspaces], [Disable compilation of nolifebar-xworkspaces program. Prints workspace status])],
  [enable_nolifebar_xworkspaces=$enableval],
  [enable_nolifebar_xworkspaces=yes]
)
AM_CONDITIONAL([ENABLE_NOLIFEBAR_XWORKSPACES], [test "x$enable_nolifebar_xworkspaces" = "xyes"])
if test "$enable_nolifebar_xworkspaces" = "yes"; then
  test "$have_xcb" = "no" && AC_MSG_ERROR([nolifebar_xworkspaces program was enabled, but libxcb was not found.])
  test "$have_xcb_ewmh" = "no" && AC_MSG_ERROR([nolifebar_xworkspaces program was enabled, but xcb-ewmh was not found.])
  test "$have_xcb_icccm" = "no" && AC_MSG_ERROR([nolifebar_xworkspaces program was enabled, but xcb-icccm was not found.])
fi

# Checks for optional programs.
AC_PATH_PROG([DZEN2], [dzen2])
if test -z "$DZEN2"; then
  AC_MSG_WARN([dzen2 program not found. Please install it (e.g., sudo apt-get install dzen2)])
  dzen2_status="no"
else
  dzen2_status="yes"
fi

AC_CONFIG_FILES([
  Makefile
  bin/Makefile
  lib/nolifebar/Makefile
])

# Generate the configuration script.
AC_OUTPUT

# Print summary
echo ""
echo "Configuration Summary:"
echo "==========================="
echo "Will be compiled binaries: "
echo "  nolifebar-multiplexer : $enable_nolifebar_multiplexer"
if test "$enable_nolifebar_fn_xft" = "yes"; then
  echo "  nolifebar-fn-xft      : $enable_nolifebar_fn_xft (You can test it: 'make test-nolifebar-fn-xft')"
else
  echo "  nolifebar-fn-xft      : $enable_nolifebar_fn_xft"
fi
echo "  nolifebar-throttle    : $enable_nolifebar_throttle"
echo "  nolifebar-replacer    : $enable_nolifebar_replacer"
echo "  nolifebar-xworkspaces : $enable_nolifebar_xworkspaces"
echo "  nolifebar-xtitle      : $enable_nolifebar_xtitle"
echo "  nolifebar-pa-status   : $enable_nolifebar_pa_status"
echo "  nolifebar-kbd         : $enable_nolifebar_kbd"
echo ""
echo "Required runtime binaries/apps:"
echo "  dzen2                 : ${dzen2_status}"
echo "==========================="
