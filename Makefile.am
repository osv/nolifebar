bin_PROGRAMS =
if ENABLE_DZEN2_FN_XFT
bin_PROGRAMS += bin/dzen2-fn-xft
bin_dzen2_fn_xft_SOURCES = src/dzen2-fn-xft.c
bin_dzen2_fn_xft_LDADD = $(X11_LIBS) $(XFT_LIBS)
bin_dzen2_fn_xft_CFLAGS = $(X11_CFLAGS) $(XFT_CFLAGS)

test-dzen2-fn-xft: bin/dzen2-fn-xft
	echo "Hello, World! Καλημέρα κόσμε! 今日は！ Привет, мир! 😊🚀" \
	| bin/dzen2-fn-xft "DejaVu Sans" "Noto Sans Greek" "Noto Sans" "Noto Sans CJK JP" "Noto Color Emoji" \
	| dzen2 -p
endif

if ENABLE_DZEN2_MERGE_PIPES
bin_PROGRAMS += bin/dzen2-merge-pipes
bin_dzen2_merge_pipes_SOURCES = src/dzen2-merge-pipes.c
endif

if ENABLE_DZEN2_THROTTLE
bin_PROGRAMS += bin/dzen2-throttle
bin_dzen2_throttle_SOURCES = src/dzen2-throttle.c
endif
