bin_PROGRAMS =
if ENABLE_DZEN2_FN_XFT
bin_PROGRAMS += dzen2-fn-xft
dzen2_fn_xft_SOURCES = ../src/dzen2-fn-xft.c
dzen2_fn_xft_LDADD = $(X11_LIBS) $(XFT_LIBS)
dzen2_fn_xft_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS)

test-dzen2-fn-xft: dzen2-fn-xft
	echo "Hello, World! Καλημέρα κόσμε! 今日は！ Привет, мир! 😊🚀" \
	| ./dzen2-fn-xft "DejaVu Sans" "Noto Sans Greek" "Noto Sans" "Noto Sans CJK JP" "Noto Color Emoji" \
	| dzen2 -p
endif

if ENABLE_DZEN2_MERGE_PIPES
bin_PROGRAMS += dzen2-merge-pipes
dzen2_merge_pipes_SOURCES = ../src/dzen2-merge-pipes.c
endif

if ENABLE_DZEN2_THROTTLE
bin_PROGRAMS += dzen2-throttle
dzen2_throttle_SOURCES = ../src/dzen2-throttle.c
endif

if ENABLE_DZEN2_XTITLE
bin_PROGRAMS += dzen2-xtitle
dzen2_xtitle_SOURCES = ../src/dzen2-xtitle.c
dzen2_xtitle_LDADD = $(X11_LIBS)
dzen2_xtitle_CFLAGS = $(X11_CFLAGS)
endif

bin_PROGRAMS += dzen2-workspaces
dzen2_workspaces_SOURCES = ../src/dzen2-workspaces.c
dzen2_workspaces_LDADD = $(X11_LIBS) $(XFT_LIBS) $(XCB_EWMH_LIBS) $(XCB_ICCCM_LIBS)
dzen2_workspaces_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS) $(XCB_EWMH_CFLAGS) $(XCB_ICCCM_CFLAGS)

bin_SCRIPTS = dzen2-run-statusbar
EXTRA_DIST = $(bin_SCRIPTS)
