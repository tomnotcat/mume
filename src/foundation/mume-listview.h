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
#ifndef MUME_FOUNDATION_LISTVIEW_H
#define MUME_FOUNDATION_LISTVIEW_H

#include "mume-scrollview.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_LISTVIEW (MUME_SIZEOF_SCROLLVIEW)

#define MUME_SIZEOF_LISTVIEW_CLASS (MUME_SIZEOF_SCROLLVIEW_CLASS)

mume_public const void* mume_listview_class(void);

mume_public const void* mume_listview_meta_class(void);

mume_public void* mume_listview_new(
    void *parent, int x, int y, int width, int height);

mume_public mume_type_t* mume_typeof_treeview_theme(void);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_LISTVIEW_H */
