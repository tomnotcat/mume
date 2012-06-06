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
#ifndef MUME_FOUNDATION_GLOBAL_H
#define MUME_FOUNDATION_GLOBAL_H

#ifdef __cplusplus
# define MUME_BEGIN_DECLS extern "C" {
# define MUME_END_DECLS }
#else
# define MUME_BEGIN_DECLS
# define MUME_END_DECLS
#endif

#ifdef _MSC_VER

# define MUME_API_EXPORT __declspec(dllexport)
# define MUME_API_IMPORT __declspec(dllimport)

# ifndef MUME_STDINT_H
#  define MUME_STDINT_H "mume-global.h"
# endif

# ifndef snprintf
#  define snprintf _snprintf
# endif

# ifndef vsnprintf
#  define vsnprintf _vsnprintf
# endif vsnprintf

# ifndef inline
#  define inline __inline
# endif

# ifndef _W64
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#   define _W64 __w64
#  else
#   define _W64
#  endif
# endif

#elif defined(__GNUC__) && defined(__i386) && !defined(__INTEL_COMPILER)
# define MUME_API_EXPORT
# define MUME_API_IMPORT
#else /* !_MSC_VER && !__GNUC__ */
# define MUME_API_EXPORT
# define MUME_API_IMPORT
#endif

#ifndef MUME_ASSERT_H
# define MUME_ASSERT_H <assert.h>
#endif

#ifndef MUME_CTYPE_H
# define MUME_CTYPE_H <ctype.h>
#endif

#ifndef MUME_ERRNO_H
# define MUME_ERRNO_H <errno.h>
#endif

#ifndef MUME_FLOAT_H
# define MUME_FLOAT_H <float.h>
#endif

#ifndef MUME_LIMITS_H
# define MUME_LIMITS_H <limits.h>
#endif

#ifndef MUME_LOCALE_H
# define MUME_LOCALE_H <locale.h>
#endif

#ifndef MUME_MATH_H
# define MUME_MATH_H <math.h>
#endif

#ifndef MUME_STDARG_H
# define MUME_STDARG_H <stdarg.h>
#endif

#ifndef MUME_STDDEF_H
# define MUME_STDDEF_H <stddef.h>
#endif

#ifndef MUME_STDINT_H
# define MUME_STDINT_H <stdint.h>
#endif

#ifndef MUME_STDIO_H
# define MUME_STDIO_H <stdio.h>
#endif

#ifndef MUME_STDLIB_H
# define MUME_STDLIB_H <stdlib.h>
#endif

#ifndef MUME_STRING_H
# define MUME_STRING_H <string.h>
#endif

#include MUME_STDDEF_H

#endif /* MUME_FOUNDATION_GLOBAL_H */
