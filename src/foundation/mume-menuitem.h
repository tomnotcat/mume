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
#ifndef MUME_FOUNDATION_MENUITEM_H
#define MUME_FOUNDATION_MENUITEM_H

#include "mume-refobj.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_MENUITEM (MUME_SIZEOF_REFOBJ + \
                              sizeof(int) + \
                              sizeof(char*) + \
                              sizeof(void*))

#define MUME_SIZEOF_MENUITEM_CLASS (MUME_SIZEOF_REFOBJ_CLASS)

mume_public const void* mume_menuitem_class(void);

#define mume_menuitem_meta_class mume_refobj_meta_class

mume_public void* mume_menuitem_new(int command, const char *text);

mume_public int mume_menuitem_get_command(const void *self);

mume_public const char* mume_menuitem_get_text(const void *self);

mume_public void mume_menuitem_insert_subitem(
    void *self, int index, void *item);

mume_public void mume_menuitem_add_subitem(void *self, void *item);

mume_public void mume_menuitem_remove_subitems(
    void *self, int index, int count);

#define mume_menuitem_remove_subitem(_self, _index) \
    mume_menuitem_remove_subitems(_self, _index, 1)

mume_public int mume_menuitem_count_subitems(const void *self);

mume_public void* mume_menuitem_get_subitem(
    const void *self, int index);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_MENUITEM_H */
