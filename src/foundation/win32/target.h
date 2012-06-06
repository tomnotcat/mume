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
#ifndef CATCH_GUI_BACKENDS_WIN32_TARGET_H
#define CATCH_GUI_BACKENDS_WIN32_TARGET_H

#include "common.h"
#include "mume-target.h"

CATCH_BEGIN_DECLS

struct win32_target_s {
    mume_target_t base;
    win32_thunk_t *thunk;
    HWND hwnd;
};

mume_target_t* win32_root_target(void *back);
mume_target_t* win32_create_target(
    void *back, mume_target_t *pnt,
    int x, int y, int width, int height);

CATCH_END_DECLS

#endif  /* CATCH_GUI_BACKENDS_WIN32_TARGET_H */
