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
#ifndef MUME_FOUNDATION_SCROLLVIEW_H
#define MUME_FOUNDATION_SCROLLVIEW_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_SCROLLVIEW (MUME_SIZEOF_WINDOW + \
                                sizeof(void*) * 2 +  \
                                sizeof(int) * 4)

#define MUME_SIZEOF_SCROLLVIEW_CLASS (MUME_SIZEOF_WINDOW_CLASS)

enum mume_scrollview_notify_e {
    MUME_SCROLLVIEW_SCROLL = MUME_WINDOW_NOTIFY_LAST,
    MUME_SCROLLVIEW_NOTIFY_LAST
};

mume_public const void* mume_scrollview_class(void);

#define mume_scrollview_meta_class mume_window_meta_class

mume_public void* mume_scrollview_new(
    void *parent, int x, int y, int width, int height);

mume_public void mume_scrollview_set_size(
    void *self, int cx, int cy);

mume_public void mume_scrollview_get_size(
    const void *self, int *cx, int *cy);

mume_public void mume_scrollview_set_page(
    void *self, int cx, int cy);

mume_public void mume_scrollview_get_page(
    const void *self, int *cx, int *cy);

mume_public void mume_scrollview_set_line(
    void *self, int cx, int cy);

mume_public void mume_scrollview_get_line(
    const void *self, int *cx, int *cy);

mume_public void mume_scrollview_set_scroll(
    void *self, int x, int y);

mume_public void mume_scrollview_get_scroll(
    const void *self, int *x, int *y);

mume_public void mume_scrollview_get_client(
    const void *self, int *x, int *y, int *w, int *h);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_SCROLLVIEW_H */
