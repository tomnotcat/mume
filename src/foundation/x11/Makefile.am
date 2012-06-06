pkglib_LTLIBRARIES = libmux11.la
libmux11_la_SOURCES = \
	mume-x11-common.h mume-x11-backend.h mume-x11-backend.c \
	mume-x11-dbgutil.h mume-x11-dbgutil.c mume-x11-backwin.h \
	mume-x11-backwin.c mume-x11-cursor.h mume-x11-cursor.c \
	mume-x11-datasrc.h mume-x11-datasrc.c mume-x11-util.h \
	mume-x11-util.c mume-x11-clipboard.h mume-x11-clipboard.c

libmux11_la_CPPFLAGS = -I..
libmux11_la_LDFLAGS = -lX11
libmux11_la_LIBADD = ../libmufod.la
