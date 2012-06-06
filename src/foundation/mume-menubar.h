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
#ifndef MUME_FOUNDATION_MENUBAR_H
#define MUME_FOUNDATION_MENUBAR_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_MENUBAR (MUME_SIZEOF_WINDOW + \
                             sizeof(void*) * 3 + \
                             sizeof(int) * 2 + \
                             sizeof(unsigned int))

#define MUME_SIZEOF_MENUBAR_CLASS (MUME_SIZEOF_WINDOW_CLASS)

enum mume_menubar_notify_e {
    MUME_MENUBAR_CLOSE = MUME_WINDOW_NOTIFY_LAST,
    MUME_MENUBAR_DELETE,
    MUME_MENUBAR_NOTIFY_LAST
};

mume_public const void* mume_menubar_class(void);

#define mume_menubar_meta_class mume_window_meta_class

mume_public void* mume_menubar_new(
    void *parent, int x, int y, int w, int h);

mume_public void* mume_menubar_new_popup(void *host);

mume_public int mume_menubar_load(void *self, const char *res);

mume_public void mume_menubar_insert_item(
    void *self, int index, void *item);

mume_public void mume_menubar_add_item(void *self, void *item);

mume_public void mume_menubar_remove_items(
    void *self, int index, int count);

#define mume_menubar_remove_item(_self, _index) \
    mume_menubar_remove_items(_self, _index, 1)

mume_public int mume_menubar_count_items(const void *self);

mume_public void mume_menubar_clear_items(void *self);

mume_public void* mume_menubar_get_item(
    const void *self, int index);

mume_public void mume_menubar_popup(
    void *self, int x, int y,
    const mume_rect_t *exclude, int auto_delete);

mume_public void mume_menubar_popdown(void *self);

mume_public mume_type_t* mume_typeof_menubar_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_MENUBAR_H */
