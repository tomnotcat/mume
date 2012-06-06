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
#ifndef CATCH_GUI_BACKENDS_WIN32_THUNK_H
#define CATCH_GUI_BACKENDS_WIN32_THUNK_H

#include "common.h"

CATCH_BEGIN_DECLS

win32_thunk_t* win32_create_thunk(
    uintptr_t proc, uintptr_t arg);
/* get thunk code address */
void* win32_thunk_code(win32_thunk_t *thk);
void win32_destroy_thunk(win32_thunk_t *thk);

CATCH_END_DECLS

#endif  /* CATCH_GUI_BACKENDS_WIN32_THUNK_H */
