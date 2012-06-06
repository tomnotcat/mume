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
#ifndef MUME_FOUNDATION_SCROLLBAR_H
#define MUME_FOUNDATION_SCROLLBAR_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SCROLLBAR (MUME_SIZEOF_WINDOW + \
                               sizeof(int) * 5 + \
                               sizeof(short) * 2 + \
                               sizeof(void*) + \
                               sizeof(unsigned int))

#define MUME_SIZEOF_SCROLLBAR_CLASS (MUME_SIZEOF_WINDOW_CLASS)

enum mume_scrollbar_e {
    MUME_SCROLLBAR_RIGHT,
    MUME_SCROLLBAR_BOTTOM,
    MUME_SCROLLBAR_LEFT,
    MUME_SCROLLBAR_TOP
};

enum mume_scrollbar_hitcode_e {
    MUME_SBHT_NOWHERE,
    MUME_SBHT_BUTTON1,
    MUME_SBHT_BUTTON2,
    MUME_SBHT_CHANNEL1,
    MUME_SBHT_CHANNEL2,
    MUME_SBHT_THUMB
};

mume_public const void* mume_scrollbar_class(void);

#define mume_scrollbar_meta_class mume_window_meta_class

mume_public void* mume_scrollbar_new(
    void *parent, int x, int y, int w, int h, int type);

mume_public void mume_scrollbar_set_size(void *self, int size);

mume_public int mume_scrollbar_get_size(const void *self);

mume_public void mume_scrollbar_set_page(void *self, int page);

mume_public int mume_scrollbar_get_page(const void *self);

mume_public void mume_scrollbar_set_pos(void *self, int pos);

mume_public int mume_scrollbar_get_pos(const void *self);

mume_public int mume_scrollbar_get_limit(const void *self);

mume_public int mume_scrollbar_is_scrollable(const void *self);

mume_public mume_type_t* mume_typeof_scrollbar_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_SCROLLBAR_H */
