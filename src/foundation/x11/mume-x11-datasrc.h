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
#ifndef MUME_X11_DATASRC_H
#define MUME_X11_DATASRC_H

#include "mume-datasrc.h"
#include "mume-x11-common.h"

MUME_BEGIN_DECLS

/* x11 datafmt class. */
#define MUME_SIZEOF_X11_DATAFMT (MUME_SIZEOF_DATAFMT + \
                                 sizeof(Atom))

#define MUME_SIZEOF_X11_DATAFMT_CLASS (MUME_SIZEOF_DATAFMT_CLASS)

mux11_public const void* mume_x11_datafmt_class(void);

#define mume_x11_datafmt_meta_class mume_datafmt_meta_class

mux11_public void* mume_x11_datafmt_new(
    Display *display, const char *name);

mux11_public Atom mume_x11_datafmt_get_atom(const void *self);

/* x11 datasrc class. */
#define MUME_SIZEOF_X11_DATASRC (MUME_SIZEOF_DATASRC + \
                                 sizeof(void*) * 2 + \
                                 sizeof(Window))

#define MUME_SIZEOF_X11_DATASRC_CLASS (MUME_SIZEOF_DATASRC_CLASS)

mux11_public const void* mume_x11_datasrc_class(void);

#define mume_x11_datasrc_meta_class mume_datasrc_meta_class

mux11_public void* mume_x11_datasrc_new(
    Display *display, Window window);

MUME_END_DECLS

#endif  /* MUME_X11_DATASRC_H */
