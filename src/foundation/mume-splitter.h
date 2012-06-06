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
#ifndef MUME_FOUNDATION_SPLITTER_H
#define MUME_FOUNDATION_SPLITTER_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SPLITTER (MUME_SIZEOF_WINDOW + \
                              sizeof(unsigned int) + \
                              sizeof(float) + \
                              sizeof(int) * 5 + \
                              sizeof(void*) * 2)

#define MUME_SIZEOF_SPLITTER_CLASS (MUME_SIZEOF_WINDOW_CLASS)

#define MUME_SPLITTER_MAX_POS MUME_WINDOW_MAX_WIDTH

enum mume_splitter_type_e {
    MUME_SPLITTER_LEFT,
    MUME_SPLITTER_RIGHT,
    MUME_SPLITTER_TOP,
    MUME_SPLITTER_BOTTOM
};

enum mume_splitter_notify_e {
    MUME_SPLITTER_POS_CHANGED = MUME_WINDOW_NOTIFY_LAST,
    MUME_SPLITTER_NOTIFY_LAST
};

mume_public const void* mume_splitter_class(void);

#define mume_splitter_meta_class mume_window_meta_class

mume_public void* mume_splitter_new(
    void *parent, int x, int y, int w, int h, int type);

mume_public void mume_splitter_set_window(
    void *self, int part, void *window);

mume_public void mume_splitter_set_size(
    void *self, int part, int min, int max);

mume_public void mume_splitter_set_pos(void *self, int pos);

mume_public int mume_splitter_get_pos(const void *self);

mume_public void mume_splitter_set_ratio_pos(void *self, float pos);

mume_public float mume_splitter_get_ratio_pos(const void *self);

mume_public mume_type_t* mume_typeof_splitter_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_SPLITTER_H */
