SUBDIRS = src/foundation src/reader src/reader/pdf src/reader/txt \
	src/foundation/x11 src/foundation/sdl tests
EXTRA_DIST = tests/valgrind-sdl.supp \
	tests/valgrind-x11.supp themes/default

pkgdata_DATA = default.theme

theme_files = ./themes/default/main.xml \
	./themes/default/images.xml \
	./themes/default/reader.xml \
	./themes/default/checkbox.bmp \
	./themes/default/menubar.png \
	./themes/default/pushbtn.bmp \
	./themes/default/radiobtn.bmp \
	./themes/default/scrollbar.bmp \
	./themes/default/splitter.bmp \
	./themes/default/tabctrl.png \
	./themes/default/treeview.png \
	./themes/default/wqy-microhei.ttc

default.theme : $(theme_files)
	zip -r -j $@ $^

CLEANFILES = default.theme
