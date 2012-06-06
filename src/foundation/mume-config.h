/* Mume Reader - a full featured reading environment.
 *
 * Copyright © 2012 Soft Flag, Inc.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version
 * 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MUME_FOUNDATION_CONFIG_H
#define MUME_FOUNDATION_CONFIG_H

#include "config.h"

#ifndef MUME_ASSERT_H
# if HAVE_ASSERT_H
#  define MUME_ASSERT_H <assert.h>
# else
#  define MUME_ASSERT_H "mume-config.h"
# endif
#endif

#ifndef MUME_ERRNO_H
# if HAVE_ERRNO_H
#  define MUME_ERRNO_H <errno.h>
# else
#  define MUME_ERRNO_H "mume-config.h"
# endif
#endif

#ifndef MUME_ERRNO_H
# if HAVE_FLOAT_H
#  define MUME_FLOAT_H <float.h>
# else
#  define MUME_FLOAT_H "mume-config.h"
# endif
#endif

#ifndef MUME_CTYPE_H
# if HAVE_CTYPE_H
#  define MUME_CTYPE_H <ctype.h>
# else
#  define MUME_CTYPE_H "mume-config.h"
# endif
#endif

#ifndef MUME_LIMITS_H
# if HAVE_LIMITS_H
#  define MUME_LIMITS_H <limits.h>
# else
#  define MUME_LIMITS_H "mume-config.h"
# endif
#endif

#ifndef MUME_LOCALE_H
# if HAVE_LOCALE_H
#  define MUME_LOCALE_H <locale.h>
# else
#  define MUME_LOCALE_H "mume-config.h"
# endif
#endif

#ifndef MUME_MATH_H
# if HAVE_MATH_H
#  define MUME_MATH_H <math.h>
# else
#  define MUME_MATH_H "mume-config.h"
# endif
#endif

#ifndef MUME_STDARG_H
# if HAVE_STDARG_H
#  define MUME_STDARG_H <stdarg.h>
# else
#  define MUME_STDARG_H "mume-config.h"
# endif
#endif

#ifndef MUME_STDDEF_H
# if HAVE_STDDEF_H
#  define MUME_STDDEF_H <stddef.h>
# else
#  define MUME_STDDEF_H "mume-config.h"
# endif
#endif

#ifndef MUME_STDINT_H
# if HAVE_STDINT_H
#  define MUME_STDINT_H <stdint.h>
# else
#  define MUME_STDINT_H "mume-config.h"
# endif
#endif

#ifndef MUME_STDIO_H
# if HAVE_STDIO_H
#  define MUME_STDIO_H <stdio.h>
# else
#  define MUME_STDIO_H "mume-config.h"
# endif
#endif

#ifndef MUME_STDLIB_H
# if HAVE_STDLIB_H
#  define MUME_STDLIB_H <stdlib.h>
# else
#  define MUME_STDLIB_H "mume-config.h"
# endif
#endif

#ifndef MUME_STRING_H
# if HAVE_STRING_H
#  define MUME_STRING_H <string.h>
# else
#  define MUME_STRING_H "mume-config.h"
# endif
#endif

#ifndef MUME_TIME_H
# if HAVE_TIME_H
#  define MUME_TIME_H <time.h>
# else
#  define MUME_TIME_H "mume-config.h"
# endif
#endif

#ifndef MUME_SYS_TIME_H
# if HAVE_SYS_TIME_H
#  define MUME_SYS_TIME_H <sys/time.h>
# else
#  define MUME_SYS_TIME_H "mume-config.h"
# endif
#endif

#ifndef MUME_DLFCN_H
# if HAVE_DLFCN_H
#  define MUME_DLFCN_H <dlfcn.h>
# else
#  define MUME_DLFCN_H "mume-config.h"
# endif
#endif

#ifndef MUME_WINDOWS_H
# if HAVE_WINDOWS_H
#  define MUME_WINDOWS_H <Windows.h>
# else
#  define MUME_WINDOWS_H "mume-config.h"
# endif
#endif

#ifndef MUME_PHYSFS_H
# if HAVE_PHYSFS_H
#  define MUME_PHYSFS_H <physfs.h>
# else
# error physfs.h not found
# endif
#endif

#ifndef MUME_EXPAT_H
# if HAVE_EXPAT_H
#  define MUME_EXPAT_H <expat.h>
# else
# error expat.h not found
# endif
#endif

#endif /* MUME_FOUNDATION_CONFIG_H */
