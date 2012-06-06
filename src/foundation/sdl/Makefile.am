pkglib_LTLIBRARIES = libmusdl.la
libmusdl_la_SOURCES = \
	mume-sdl-common.h mume-sdl-backend.h mume-sdl-backend.c \
	mume-sdl-backwin.h mume-sdl-backwin.c cairosdl.h cairosdl.c \
	mume-sdl-dbgutil.h mume-sdl-dbgutil.c mume-sdl-cursor.h \
	mume-sdl-cursor.c

libmusdl_la_CPPFLAGS = -I..
libmusdl_la_LDFLAGS = -lSDL
libmusdl_la_LIBADD = ../libmufod.la
