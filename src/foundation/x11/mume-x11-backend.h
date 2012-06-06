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
#ifndef MUME_X11_BACKEND_H
#define MUME_X11_BACKEND_H

#include "mume-backend.h"
#include "mume-x11-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_X11_BACKEND (MUME_SIZEOF_BACKEND + \
                                 sizeof(void*) * 3 +   \
                                 sizeof(int) * 3)

#define MUME_SIZEOF_X11_BACKEND_CLASS (MUME_SIZEOF_BACKEND_CLASS)

mux11_public const void* mume_x11_backend_class(void);

#define mume_x11_backend_meta_class mume_backend_meta_class

mux11_public const void* mume_backend_class_sym(void);

mux11_public Display* mume_x11_backend_get_display(const void *self);

mux11_public int mume_x11_backend_get_screen(const void *self);

mux11_public void mume_x11_backend_bind(void *self, void *window);

mux11_public void mume_x11_backend_unbind(void *self, void *window);

mux11_public void mume_x11_backend_dispatch(void *self, XEvent *xevent);

MUME_END_DECLS

#endif  /* MUME_X11_BACKEND_H */
