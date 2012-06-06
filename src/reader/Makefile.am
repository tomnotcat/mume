pkglib_LTLIBRARIES = libmurdr.la
libmurdr_la_SOURCES = \
	mume-common.h mume-gstate.h mume-gstate.c mume-docdoc.h \
	mume-docdoc.c mume-docmgr.h mume-docmgr.c mume-mainform.h \
	mume-mainform.c mume-index-view.h mume-index-view.c \
	mume-docview.h mume-docview.c mume-profile.h mume-profile.c \
	mume-home-view.h mume-home-view.c mume-read-view.h \
	mume-read-view.c mume-book.h mume-book.c mume-bookmgr.h \
	mume-bookmgr.c mume-bookshelf.h mume-bookshelf.c \
	mume-bookslot.h mume-bookslot.c

libmurdr_la_CPPFLAGS = -I$(top_srcdir)/include -I$(THIRDPARTY_DIR) \
	$(LIBGCRYPT_CFLAGS)
libmurdr_la_LDFLAGS = -L$(THIRDPARTY_DIR)/mupdf -lmupdf -lfitz \
	-lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz $(LIBGCRYPT_LIBS)
libmurdr_la_LIBADD = ../foundation/libmufod.la

bin_PROGRAMS = mume
mume_SOURCES = mume-main.c
mume_CPPFLAGS = -I$(top_srcdir)/include \
	-DMUME_DATA_DIR=\"$(datadir)/mume/\"
mume_LDADD = ../foundation/libmufod.la libmurdr.la
