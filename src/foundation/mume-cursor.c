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
#include "mume-cursor.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _cursor_super_class mume_object_class
#define _cursor_super_meta_class mume_meta_class

struct _cursor {
    const char _[MUME_SIZEOF_OBJECT];
};

struct _cursor_class {
    const char _[MUME_SIZEOF_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _cursor) == MUME_SIZEOF_CURSOR);
MUME_STATIC_ASSERT(sizeof(struct _cursor_class) ==
                   MUME_SIZEOF_CURSOR_CLASS);
