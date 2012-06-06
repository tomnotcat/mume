base_sources = \
	mume-common.h mume-math.h mume-math.c mume-list.h mume-list.c \
	mume-global.h mume-debug.h mume-debug.c mume-vector.h \
	mume-vector.c mume-string.h mume-string.c mume-memory.h \
	mume-memory.c mume-dlfcn.h mume-dlfcn.c mume-virtfs.h \
	mume-virtfs.c mume-object.h mume-object.c mume-variant.h \
	mume-variant.c mume-property.h mume-property.c mume-olist.h \
	mume-olist.c mume-oset.h mume-oset.c mume-time.h mume-time.c \
	mume-heap.h mume-heap.c mume-userdata.h mume-userdata.c	\
	mume-rbtree.h mume-rbtree.c mume-octnr.h mume-octnr.c \
	mume-ooset.h mume-ooset.c mume-stream.h mume-stream.c \
	mume-thread.h mume-thread.c mume-ovector.h mume-ovector.c \
	mume-plugin.h mume-plugin.c mume-encoding.h mume-encoding.c \
	mume-filetc.h mume-filetc.c mume-geometry.h mume-geometry.c \
	mume-serialize.h mume-serialize.c mume-refobj.h mume-refobj.c \
	mume-types.h mume-types.c mume-types-private.h mume-objbase.h \
	mume-objbase.c mume-message.h mume-message.c mume-class.h \
	mume-class.c mume-clsmgr.h mume-clsmgr.c mume-virtfs2.h \
	mume-virtfs2.c mume-virtfs-native.h mume-virtfs-native.c \
	mume-virtfs-zip.h mume-virtfs-zip.c mume-error.h mume-error.c

base_ldflags = -ldl -lpthread -lexpat -lphysfs

gui_sources = \
	mume-backend.h mume-backend.c mume-backwin.h mume-backwin.c \
	mume-frontend.h mume-frontend.c mume-timer.h mume-timer.c \
	mume-common.h mume-keysym.h mume-gstate.h mume-gstate.c \
	mume-gstate-private.h mume-dbgutil.h mume-dbgutil.c \
	mume-events.h mume-events.c mume-window.h mume-window.c \
	mume-resmgr.h mume-resmgr.c mume-urgnmgr.h mume-urgnmgr.c \
	mume-resmgr-private.h mume-dibfcn.h mume-dibfcn.c \
	mume-drawing.h mume-drawing.c mume-label.h mume-label.c \
	mume-button.h mume-button.c mume-cursor.h mume-cursor.c \
	mume-clipboard.h mume-clipboard.c mume-splitter.h \
	mume-splitter.c mume-ratiobox.h mume-ratiobox.c \
	mume-text-layout.h mume-text-layout.c mume-scrollview.h \
	mume-scrollview.c mume-scrollbar.h mume-scrollbar.c \
	mume-treeview.h mume-treeview.c mume-listview.h mume-listview.c \
	mume-tabctrl.h mume-tabctrl.c mume-datasrc.h mume-datasrc.c \
	mume-menuitem.h mume-menuitem.c mume-menubar.h mume-menubar.c

gui_ldflags =  -lcairo -lfreetype -lharfbuzz

pkglib_LTLIBRARIES = libmufod.la
libmufod_la_SOURCES = ${base_sources} ${gui_sources}
libmufod_la_LDFLAGS = ${base_ldflags} ${gui_ldflags} \
	-L$(THIRDPARTY_DIR)/mupdf -lz
EXTRA_DIST = mume-getopt.h mume-getopt.c
