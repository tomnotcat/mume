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
#ifndef MUME_FOUNDATION_CLIPBOARD_H
#define MUME_FOUNDATION_CLIPBOARD_H

#include "mume-refobj.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_CLIPBOARD (MUME_SIZEOF_REFOBJ + \
                               sizeof(void*))

#define MUME_SIZEOF_CLIPBOARD_CLASS (MUME_SIZEOF_REFOBJ_CLASS + \
                                     sizeof(voidf*) * 2)

mume_public const void* mume_clipboard_class(void);

mume_public const void* mume_clipboard_meta_class(void);

#define mume_clipboard_new() mume_new(mume_clipboard_class())

/* Selector for set the clipboard data source. */
mume_public void _mume_clipboard_set_data(
    const void *clazz, void *self, void *data);

#define mume_clipboard_set_data(_self, _data) \
    _mume_clipboard_set_data(NULL, _self, _data)

/* Selector for get the clipboard data source. */
mume_public void* _mume_clipboard_get_data(
    const void *clazz, void *self);

#define mume_clipboard_get_data(_self) \
    _mume_clipboard_get_data(NULL, _self)

MUME_END_DECLS

#endif /* MUME_FOUNDATION_CLIPBOARD_H */
