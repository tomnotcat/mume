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
#ifndef MUME_X11_CURSOR_H
#define MUME_X11_CURSOR_H

#include "mume-cursor.h"
#include "mume-x11-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_X11_CURSOR (MUME_SIZEOF_CURSOR + \
                                sizeof(Display*) +   \
                                sizeof(Cursor))

#define MUME_SIZEOF_X11_CURSOR_CLASS (MUME_SIZEOF_CURSOR_CLASS)

mux11_public const void* mume_x11_cursor_class(void);

#define mume_x11_cursor_meta_class mume_cursor_meta_class

mux11_public Cursor mume_x11_create_cursor(Display *display, int id);

mux11_public void* mume_x11_cursor_new(Display *display, Cursor cursor);

mux11_public Cursor mume_x11_cursor_get_entity(const void *self);

MUME_END_DECLS

#endif  /* MUME_X11_CURSOR_H */
