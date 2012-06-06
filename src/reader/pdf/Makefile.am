pkglib_LTLIBRARIES = libmume-pdf.la
libmume_pdf_la_SOURCES = mume-pdf-doc.h mume-pdf-doc.c
libmume_pdf_la_CPPFLAGS = -I$(top_srcdir)/include -I$(THIRDPARTY_DIR)
libmume_pdf_la_LDFLAGS = -L$(THIRDPARTY_DIR)/mupdf -lmupdf -lfitz \
	-lfreetype -ljbig2dec -ljpeg -lopenjpeg -lz
libmume_pdf_la_LIBADD = ../../foundation/libmufod.la ../libmurdr.la
