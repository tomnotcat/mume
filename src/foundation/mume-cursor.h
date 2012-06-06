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
#ifndef MUME_FOUNDATION_CURSOR_H
#define MUME_FOUNDATION_CURSOR_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_CURSOR (MUME_SIZEOF_OBJECT)

#define MUME_SIZEOF_CURSOR_CLASS (MUME_SIZEOF_CLASS)

enum mume_cursor_e {
    MUME_CURSOR_NONE,
    MUME_CURSOR_ARROW,
    MUME_CURSOR_HDBLARROW,
    MUME_CURSOR_VDBLARROW,
    MUME_CURSOR_HAND,
    MUME_CURSOR_IBEAM,
    MUME_CURSOR_WAIT,
    MUME_NUMCURSORS
};

#define mume_cursor_class mume_object_class

#define mume_cursor_meta_class mume_meta_class

MUME_END_DECLS

#endif /* MUME_FOUNDATION_CURSOR_H */
