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
#ifndef MUME_FOUNDATION_LABEL_H
#define MUME_FOUNDATION_LABEL_H

#include "mume-window.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_LABEL (MUME_SIZEOF_WINDOW + \
                           sizeof(void*))

#define MUME_SIZEOF_LABEL_CLASS (MUME_SIZEOF_WINDOW_CLASS)

mume_public const void* mume_label_class(void);

#define mume_label_meta_class mume_window_meta_class

mume_public void* mume_label_new(
    void *parent, int x, int y, int width, int height);

mume_public void mume_label_set_bkgnd(
    void *self, mume_resobj_brush_t *bkgnd);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_LABEL_H */
