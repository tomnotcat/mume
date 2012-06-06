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
#ifndef CATCH_GUI_BACKENDS_WIN32_MSGPROC_H
#define CATCH_GUI_BACKENDS_WIN32_MSGPROC_H

#include "common.h"

CATCH_BEGIN_DECLS

#define WIN32_WINCLS_NAME "mume_win32_target"

void win32_initialize(void);
void win32_createwin_begin(win32_target_t *wtgt);
void win32_createwin_end(win32_target_t *wtgt);

CATCH_END_DECLS

#endif  /* CATCH_GUI_BACKENDS_WIN32_MSGPROC_H */
