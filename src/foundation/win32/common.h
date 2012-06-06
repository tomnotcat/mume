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
#ifndef CATCH_GUI_CORE_WIN32_COMMON_H
#define CATCH_GUI_CORE_WIN32_COMMON_H

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif

#include <Windows.h>
#include <WindowsX.h>
#include "mume-common.h"

CATCH_BEGIN_DECLS

typedef struct win32_target_s win32_target_t;
typedef struct win32_icontext_s win32_icontext_t;
typedef struct win32_backend_s win32_backend_t;
typedef struct win32_thunk_s win32_thunk_t;

CATCH_END_DECLS

#endif  /* CATCH_GUI_CORE_WIN32_COMMON_H */
