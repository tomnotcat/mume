#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([mume], [1.0], [tomnotcat@gmail.com])
AC_CONFIG_SRCDIR([src/foundation/mume-common.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_PREREQ([2.2])
LT_INIT([disable-static])
AC_DEFINE([BUILD_UNIX], [1], [Build under unix like system])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([expat], [XML_ParserCreate], [have_expat=yes], [have_expat=no])
AC_CHECK_LIB([physfs], [PHYSFS_init], [have_physfs=yes], [have_physfs=no])
AC_CHECK_LIB([cairo], [cairo_destroy], [have_cairo=yes], [have_cairo=no])
AC_CHECK_LIB([harfbuzz], [hb_ft_font_create],
                         [have_harfbuzz=yes], [have_harfbuzz=no])

AM_PATH_LIBGCRYPT([], [have_gcrypt=yes], [have_gcrypt=no])

if test "x${have_gcrypt}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find libgcrypt on this system.
        <http://directory.fsf.org/wiki/Libgcrypt>
        ------------------------------------------])
fi

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([ \
   assert.h ctype.h dlfunc.h errno.h float.h limits.h locale.h \
   math.h pthread.h stdarg.h stddef.h stdint.h stdio.h stdlib.h \
   string.h sys/time.h time.h])

if test "x${have_expat}" = xyes; then
   AC_CHECK_HEADERS([expat.h], [], [have_expat=no])
fi

if test "x${have_expat}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find expat on this system.
        <http://expat.sourceforge.net/>
        ------------------------------------------])
fi

if test "x${have_physfs}" = xyes; then
   AC_CHECK_HEADERS([physfs.h], [], [have_physfs=no])
fi

if test "x${have_physfs}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find physfs on this system.
        <http://icculus.org/physfs/>
        ------------------------------------------])
fi

if test "x${have_cairo}" = xyes; then
   AC_CHECK_HEADERS([cairo/cairo.h], [], [have_cairo=no])
fi

if test "x${have_cairo}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find cairo on this system.
        <http://www.cairographics.org/>
        ------------------------------------------])
fi

AC_CHECK_HEADERS([SDL/SDL.h], [], [have_sdl=no])

if test "x${have_sdl}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find SDL on this system.
        <http://www.libsdl.org/>
        ------------------------------------------])
fi

AC_PATH_PROG(ft_config, freetype-config, no)
if test "x$ft_config" = "xno"; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find freetype on this system.
        <http://www.freetype.org/>
        ------------------------------------------])
fi

AC_DEFINE([HAVE_FT2BUILD], [1],
[Define to 1 if you have the <ft2build.h> header file.])

FREETYPE_CFLAGS="`$ft_config --cflags`"
FREETYPE_LIBS="`$ft_config --libs`"
LIBS="$LIBS $FREETYPE_LIBS"
CFLAGS="$CFLAGS $FREETYPE_CFLAGS"

if test "x${have_harfbuzz}" = xyes; then
   AC_CHECK_HEADERS([harfbuzz/hb.h], [], [have_harfbuzz=no])
fi

if test "x${have_harfbuzz}" = xno; then
   AC_MSG_ERROR([
        ------------------------------------------
        Unable to find harfbuzz on this system.
        <http://www.freedesktop.org/wiki/Software/HarfBuzz>
        ------------------------------------------])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memmove memset select setlocale \
                        strchr strspn strtol strtoul])

# Configure options: --enable-debug[=no].
AC_ARG_ENABLE([debug],
        [AS_HELP_STRING([--enable-debug],
                [enable debug code (default is no)])],
        [debug=yes], [debug=no])

AM_CONDITIONAL([DEBUG], [test x$debug = xyes])

if test "x${debug}" != "xno"; then
   AC_DEFINE([DEBUG], [1], [Debugging mode])
   AC_CHECK_PROG([VALGRIND], [valgrind], [valgrind])
   CFLAGS="${CFLAGS} -g3 -gdwarf-2 -O0"
fi

AC_SUBST([THEMES_DIR], ["\$(abs_top_srcdir)/themes"])

if test -n "${VALGRIND}"; then
   TESTS_ENVIRONMENT="libtool --mode=execute ${VALGRIND} -q --tool=memcheck \
--leak-check=yes --error-exitcode=1 --malloc-fill=4 --free-fill=4"
   TESTS_ENVIRONMENT_SDL="${TESTS_ENVIRONMENT} \
--suppressions=\$(top_srcdir)/tests/valgrind-sdl.supp"
   TESTS_ENVIRONMENT_X11="${TESTS_ENVIRONMENT} \
--suppressions=\$(top_srcdir)/tests/valgrind-x11.supp"
   AC_SUBST([TESTS_ENVIRONMENT])
   AC_SUBST([TESTS_ENVIRONMENT_SDL])
   AC_SUBST([TESTS_ENVIRONMENT_X11])
fi

CFLAGS="${CFLAGS} -pedantic -Wall -Werror -Wno-long-long -Wstrict-prototypes"

test -z "${THIRDPARTY_DIR}" && THIRDPARTY_DIR="${MUME_THIRDPARTY_DIR}"
if test -z "${THIRDPARTY_DIR}"; then
   AC_MSG_ERROR([
        ------------------------------------------
        Thirdparty directory not set, you can set it
        either in the command line variable THIRDPARTY_DIR
        or through the global environment variable
        MUME_THIRDPARTY_DIR.
        ------------------------------------------])
fi

AC_SUBST([THIRDPARTY_DIR])

AC_CONFIG_FILES([Makefile
        src/foundation/Makefile
        src/foundation/x11/Makefile
        src/foundation/sdl/Makefile
        src/reader/Makefile
        src/reader/pdf/Makefile
        src/reader/txt/Makefile
        tests/Makefile])

AC_OUTPUT

echo \
"-------------------------------------------------
${PACKAGE_NAME} Version ${PACKAGE_VERSION}
Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Libraries: '${LIBS}'
Debug: '${debug}'
TESTS_ENVIRONMENT: '${TESTS_ENVIRONMENT}'
THIRDPARTY_DIR: '${THIRDPARTY_DIR}'
-------------------------------------------------"
