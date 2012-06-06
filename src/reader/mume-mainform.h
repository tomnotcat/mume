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
#ifndef MUME_READER_MAINFORM_H
#define MUME_READER_MAINFORM_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_MAINFORM (MUME_SIZEOF_RATIOBOX + \
                              sizeof(void*) * 3)

#define MUME_SIZEOF_MAINFORM_CLASS (MUME_SIZEOF_RATIOBOX_CLASS)

#define MUME_MAINFORM_WIDTH 800
#define MUME_MAINFORM_HEIGHT 600

murdr_public const void* mume_mainform_class(void);

#define mume_mainform_meta_class mume_ratiobox_meta_class

murdr_public void* mume_mainform_new(
    void *parent, int x, int y, int width, int height);

murdr_public void mume_mainform_add_view(void *self, void *view);

murdr_public void mume_mainform_remove_view(void *self, void *view);

murdr_public void* mume_mainform_new_tab(void *self);

murdr_public void mume_mainform_set_active(void *self, void *view);

murdr_public void* mume_mainform_get_active(const void *self);

MUME_END_DECLS

#endif /* MUME_READER_MAINFORM_H */
