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
#ifndef MUME_FOUNDATION_REFOBJ_H
#define MUME_FOUNDATION_REFOBJ_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_REFOBJ (MUME_SIZEOF_OBJECT + \
                            sizeof(int))

#define MUME_SIZEOF_REFOBJ_CLASS (MUME_SIZEOF_CLASS)

mume_public const void* mume_refobj_class(void);

#define mume_refobj_meta_class mume_meta_class

mume_public void mume_refobj_addref(void *self);

mume_public void mume_refobj_release(void *self);

mume_public int mume_refobj_refcount(const void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_REFOBJ_H */
