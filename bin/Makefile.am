bin_PROGRAMS =
if ENABLE_NOLIFEBAR_FN_XFT
bin_PROGRAMS += nolifebar-fn-xft
nolifebar_fn_xft_SOURCES = ../src/nolifebar-fn-xft.c
nolifebar_fn_xft_LDADD = $(X11_LIBS) $(XFT_LIBS)
nolifebar_fn_xft_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS)

test-nolifebar-fn-xft: nolifebar-fn-xft
	echo "Hello, World! Καλημέρα κόσμε! 今日は！ Привет, мир! 😊🚀" \
	| ./nolifebar-fn-xft "DejaVu Sans" "Noto Sans Greek" "Noto Sans" "Noto Sans CJK JP" "Noto Color Emoji" \
	| dzen2 -p
endif

if ENABLE_NOLIFEBAR_MULTIPLEXER
bin_PROGRAMS += nolifebar-multiplexer
nolifebar_multiplexer_SOURCES = ../src/nolifebar-multiplexer.c
endif

if ENABLE_NOLIFEBAR_THROTTLE
bin_PROGRAMS += nolifebar-throttle
nolifebar_throttle_SOURCES = ../src/nolifebar-throttle.c
endif

if ENABLE_NOLIFEBAR_XTITLE
bin_PROGRAMS += nolifebar-xtitle
nolifebar_xtitle_SOURCES = ../src/nolifebar-xtitle.c
nolifebar_xtitle_LDADD = $(X11_LIBS)
nolifebar_xtitle_CFLAGS = $(X11_CFLAGS)
endif

bin_PROGRAMS += nolifebar-workspaces
nolifebar_workspaces_SOURCES = ../src/nolifebar-workspaces.c
nolifebar_workspaces_LDADD = $(X11_LIBS) $(XFT_LIBS) $(XCB_EWMH_LIBS) $(XCB_ICCCM_LIBS)
nolifebar_workspaces_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS) $(XCB_EWMH_CFLAGS) $(XCB_ICCCM_CFLAGS)

bin_SCRIPTS = nolifebar-start
EXTRA_DIST = $(bin_SCRIPTS)
