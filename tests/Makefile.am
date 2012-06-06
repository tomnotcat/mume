AM_CPPFLAGS = -I$(top_srcdir)/include \
	-DTESTS_DATA_DIR=\"$(abs_srcdir)/data\" \
	-DTESTS_THEME_DIR=\"@THEMES_DIR@\"
AM_LDFLAGS = -L../src/foundation -lmufod -L../src/reader -lmurdr
base_env = @TESTS_ENVIRONMENT@
sdl_env = @TESTS_ENVIRONMENT_SDL@
x11_env = @TESTS_ENVIRONMENT_X11@
sdl_params = --backend $(top_builddir)/src/foundation/sdl/.libs/libmusdl.so \
	--width 800 --height 600 --loglvl 3
x11_params = --backend $(top_builddir)/src/foundation/x11/.libs/libmux11.so \
	--loglvl 3
base_params = --backend console --loglvl 3
sdl_scripts =
x11_scripts =
check_PROGRAMS =
check_SCRIPTS =
EXTRA_DIST = test-util.h data/resmgr.res data/test-base-objbase0.xml \
	data/test-base-objbase1.xml data/test-base-objbase2.xml \
	data/test-base-virtfs.txt data/test-base-virtfs.zip

# Test base functions.
check_PROGRAMS += test-base
check_SCRIPTS += test-base.sh
test_base_SOURCES = \
	main.c test-util.c test-base.c test-container.c \
	test-objbase.c test-stream.c test-types.c test-heap.c \
	test-time.c test-virtfs.c test-thread.c
test-base.sh: Makefile
	echo "$(base_env) ./test-base $(base_params)" > $@
	chmod +x $@

# Test base variant.
check_PROGRAMS += test-variant
check_SCRIPTS += test-variant.sh
test_variant_SOURCES = main.c test-util.c test-variant.c
test-variant.sh: Makefile
	echo "$(base_env) ./test-variant $(base_params)" > $@
	chmod +x $@

# Test base object.
check_PROGRAMS += test-object
check_SCRIPTS += test-object.sh
test_object_SOURCES = main.c test-util.c test-object.c
test-object.sh: Makefile
	echo "$(base_env) ./test-object $(base_params)" > $@
	chmod +x $@

# Test base object container.
check_PROGRAMS += test-octnr
check_SCRIPTS += test-octnr.sh
test_octnr_SOURCES = main.c test-util.c test-octnr.c
test-octnr.sh: Makefile
	echo "$(base_env) ./test-octnr $(base_params)" > $@
	chmod +x $@

# Test base serialize.
check_PROGRAMS += test-serialize
check_SCRIPTS += test-serialize.sh
test_serialize_SOURCES = main.c test-util.c test-serialize.c
test-serialize.sh: Makefile
	echo "$(base_env) ./test-serialize $(base_params)" > $@
	chmod +x $@

# Test base plugin.
check_PROGRAMS += test-plugin
check_SCRIPTS += test-plugin.sh
test_plugin_SOURCES = main.c test-util.c test-plugin.c
# Make it possible to open with dlopen.
test_plugin_LDFLAGS = $(AM_LDFLAGS) -rdynamic -pie
test-plugin.sh: Makefile
	echo "$(base_env) ./test-plugin $(base_params)" > $@
	chmod +x $@

# Test base file type check.
check_PROGRAMS += test-filetc
check_SCRIPTS += test-filetc.sh
test_filetc_SOURCES = main.c test-util.c test-filetc.c
test-filetc.sh: Makefile
	echo "$(base_env) ./test-filetc $(base_params)" > $@
	chmod +x $@

# Test GUI events.
check_PROGRAMS += test-event
sdl_scripts += test-event-sdl.sh
x11_scripts +=  test-event-x11.sh
test_event_SOURCES = main.c test-util.c test-event.c
test-event-sdl.sh: Makefile
	echo "$(sdl_env) ./test-event $(sdl_params)" > $@
	chmod +x $@
test-event-x11.sh: Makefile
	echo "$(x11_env) ./test-event $(x11_params)" > $@
	chmod +x $@

# Test GUI timer.
check_PROGRAMS += test-timer
sdl_scripts += test-timer-sdl.sh
x11_scripts +=  test-timer-x11.sh
test_timer_SOURCES = main.c test-util.c test-timer.c
test-timer-sdl.sh: Makefile
	echo "$(sdl_env) ./test-timer $(sdl_params)" > $@
	chmod +x $@
test-timer-x11.sh: Makefile
	echo "$(x11_env) ./test-timer $(x11_params)" > $@
	chmod +x $@

# Test GUI window.
check_PROGRAMS += test-window
sdl_scripts += test-window-sdl.sh
x11_scripts +=  test-window-x11.sh
test_window_SOURCES = main.c test-util.c test-window.c
test-window-sdl.sh: Makefile
	echo "$(sdl_env) ./test-window $(sdl_params)" > $@
	chmod +x $@
test-window-x11.sh: Makefile
	echo "$(x11_env) ./test-window $(x11_params)" > $@
	chmod +x $@

# Test GUI paint.
check_PROGRAMS += test-paint
sdl_scripts += test-paint-sdl.sh
x11_scripts +=  test-paint-x11.sh
test_paint_SOURCES = main.c test-util.c test-paint.c
test-paint-sdl.sh: Makefile
	echo "$(sdl_env) ./test-paint $(sdl_params)" > $@
	chmod +x $@
test-paint-x11.sh: Makefile
	echo "$(x11_env) ./test-paint $(x11_params)" > $@
	chmod +x $@

# Test GUI resource manager.
check_PROGRAMS += test-resmgr
sdl_scripts += test-resmgr-sdl.sh
x11_scripts +=  test-resmgr-x11.sh
test_resmgr_SOURCES = main.c test-util.c test-resmgr.c
test-resmgr-sdl.sh: Makefile
	echo "$(sdl_env) ./test-resmgr $(sdl_params)" > $@
	chmod +x $@
test-resmgr-x11.sh: Makefile
	echo "$(x11_env) ./test-resmgr $(x11_params)" > $@
	chmod +x $@

# Test GUI clipboard.
check_PROGRAMS += test-clipboard
sdl_scripts += test-clipboard-sdl.sh
x11_scripts +=  test-clipboard-x11.sh
test_clipboard_SOURCES = main.c test-util.c test-clipboard.c
test-clipboard-sdl.sh: Makefile
	echo "$(sdl_env) ./test-clipboard $(sdl_params)" > $@
	chmod +x $@
test-clipboard-x11.sh: Makefile
	echo "$(x11_env) ./test-clipboard $(x11_params)" > $@
	chmod +x $@

# Test GUI button.
check_PROGRAMS += test-button
sdl_scripts += test-button-sdl.sh
x11_scripts +=  test-button-x11.sh
test_button_SOURCES = main.c test-util.c test-button.c
test-button-sdl.sh: Makefile
	echo "$(sdl_env) ./test-button $(sdl_params)" > $@
	chmod +x $@
test-button-x11.sh: Makefile
	echo "$(x11_env) ./test-button $(x11_params)" > $@
	chmod +x $@

# Test GUI scroll view.
check_PROGRAMS += test-scrollview
sdl_scripts += test-scrollview-sdl.sh
x11_scripts +=  test-scrollview-x11.sh
test_scrollview_SOURCES = main.c test-util.c test-scrollview.c
test-scrollview-sdl.sh: Makefile
	echo "$(sdl_env) ./test-scrollview $(sdl_params)" > $@
	chmod +x $@
test-scrollview-x11.sh: Makefile
	echo "$(x11_env) ./test-scrollview $(x11_params)" > $@
	chmod +x $@

# Test GUI list view.
check_PROGRAMS += test-listview
sdl_scripts += test-listview-sdl.sh
x11_scripts +=  test-listview-x11.sh
test_listview_SOURCES = main.c test-util.c test-listview.c
test-listview-sdl.sh: Makefile
	echo "$(sdl_env) ./test-listview $(sdl_params)" > $@
	chmod +x $@
test-listview-x11.sh: Makefile
	echo "$(x11_env) ./test-listview $(x11_params)" > $@
	chmod +x $@

# Test GUI tree view.
check_PROGRAMS += test-treeview
sdl_scripts += test-treeview-sdl.sh
x11_scripts +=  test-treeview-x11.sh
test_treeview_SOURCES = main.c test-util.c test-treeview.c
test-treeview-sdl.sh: Makefile
	echo "$(sdl_env) ./test-treeview $(sdl_params)" > $@
	chmod +x $@
test-treeview-x11.sh: Makefile
	echo "$(x11_env) ./test-treeview $(x11_params)" > $@
	chmod +x $@

# Test GUI splitter.
check_PROGRAMS += test-splitter
sdl_scripts += test-splitter-sdl.sh
x11_scripts +=  test-splitter-x11.sh
test_splitter_SOURCES = main.c test-util.c test-splitter.c
test-splitter-sdl.sh: Makefile
	echo "$(sdl_env) ./test-splitter $(sdl_params)" > $@
	chmod +x $@
test-splitter-x11.sh: Makefile
	echo "$(x11_env) ./test-splitter $(x11_params)" > $@
	chmod +x $@

# Test GUI tabctrl.
check_PROGRAMS += test-tabctrl
sdl_scripts += test-tabctrl-sdl.sh
x11_scripts +=  test-tabctrl-x11.sh
test_tabctrl_SOURCES = main.c test-util.c test-tabctrl.c
test-tabctrl-sdl.sh: Makefile
	echo "$(sdl_env) ./test-tabctrl $(sdl_params)" > $@
	chmod +x $@
test-tabctrl-x11.sh: Makefile
	echo "$(x11_env) ./test-tabctrl $(x11_params)" > $@
	chmod +x $@

# Test GUI menubar.
check_PROGRAMS += test-menubar
sdl_scripts += test-menubar-sdl.sh
x11_scripts +=  test-menubar-x11.sh
test_menubar_SOURCES = main.c test-util.c test-menubar.c
test-menubar-sdl.sh: Makefile
	echo "$(sdl_env) ./test-menubar $(sdl_params)" > $@
	chmod +x $@
test-menubar-x11.sh: Makefile
	echo "$(x11_env) ./test-menubar $(x11_params)" > $@
	chmod +x $@

# Test reader document manager.
check_PROGRAMS += test-docmgr
sdl_scripts += test-docmgr-sdl.sh
x11_scripts +=  test-docmgr-x11.sh
test_docmgr_SOURCES = main.c test-util.c test-docmgr.c
test_docmgr_LDFLAGS = $(AM_LDFLAGS) -L../src/reader/pdf -lmume-pdf \
	-L../src/reader/txt -lmume-txt
test-docmgr-sdl.sh: Makefile
	echo "$(sdl_env) ./test-docmgr $(sdl_params) --reader 1" > $@
	chmod +x $@
test-docmgr-x11.sh: Makefile
	echo "$(x11_env) ./test-docmgr $(x11_params) --reader 1" > $@
	chmod +x $@

# Test reader book manager.
check_PROGRAMS += test-bookmgr
sdl_scripts += test-bookmgr-sdl.sh
x11_scripts +=  test-bookmgr-x11.sh
test_bookmgr_SOURCES = main.c test-util.c test-bookmgr.c
test-bookmgr-sdl.sh: Makefile
	echo "$(sdl_env) ./test-bookmgr $(sdl_params) --reader 1" > $@
	chmod +x $@
test-bookmgr-x11.sh: Makefile
	echo "$(x11_env) ./test-bookmgr $(x11_params) --reader 1" > $@
	chmod +x $@

# Test reader docview.
check_PROGRAMS += test-docview
sdl_scripts += test-docview-sdl.sh
x11_scripts +=  test-docview-x11.sh
test_docview_SOURCES = main.c test-util.c test-docview.c
test_docview_LDFLAGS = $(AM_LDFLAGS) -L../src/reader/pdf -lmume-pdf \
	-L../src/reader/txt -lmume-txt
test-docview-sdl.sh: Makefile
	echo "$(sdl_env) ./test-docview $(sdl_params) --reader 1" > $@
	chmod +x $@
test-docview-x11.sh: Makefile
	echo "$(x11_env) ./test-docview $(x11_params) --reader 1" > $@
	chmod +x $@

# Test reader index view.
check_PROGRAMS += test-index-view
sdl_scripts += test-index-view-sdl.sh
x11_scripts +=  test-index-view-x11.sh
test_index_view_SOURCES = main.c test-util.c test-index-view.c
test_index_view_LDFLAGS = $(AM_LDFLAGS) -L../src/reader/pdf \
-lmume-pdf -L../src/reader/txt -lmume-txt
test-index-view-sdl.sh: Makefile
	echo "$(sdl_env) ./test-index-view $(sdl_params) --reader 1" > $@
	chmod +x $@
test-index-view-x11.sh: Makefile
	echo "$(x11_env) ./test-index-view $(x11_params) --reader 1" > $@
	chmod +x $@

# Test reader main form.
check_PROGRAMS += test-mainform
sdl_scripts += test-mainform-sdl.sh
x11_scripts +=  test-mainform-x11.sh
test_mainform_SOURCES = main.c test-util.c test-mainform.c
test-mainform-sdl.sh: Makefile
	echo "$(sdl_env) ./test-mainform $(sdl_params) --reader 1" > $@
	chmod +x $@
test-mainform-x11.sh: Makefile
	echo "$(x11_env) ./test-mainform $(x11_params) --reader 1" > $@
	chmod +x $@

check_SCRIPTS += $(sdl_scripts)
check_SCRIPTS += $(x11_scripts)

TESTS = $(check_SCRIPTS)
CLEANFILES = $(check_SCRIPTS)
