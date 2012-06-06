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
#ifndef MUME_FOUNDATION_URGNMGR_H
#define MUME_FOUNDATION_URGNMGR_H

#include "mume-object.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_URGNMGR (MUME_SIZEOF_OBJECT + \
                             sizeof(void*) * 4)

#define MUME_SIZEOF_URGNMGR_CLASS (MUME_SIZEOF_CLASS + \
                                   sizeof(void*))

enum mume_urgn_operation_e {
    MUME_URGN_UNION = 0,
    MUME_URGN_SUBTRACT,
    MUME_URGN_REPLACE
};

mume_public const void* mume_urgnmgr_class(void);

mume_public const void* mume_urgnmgr_meta_class(void);

#define mume_urgnmgr_new() mume_new(mume_urgnmgr_class())

/* Get the update region of the window <win>. */
mume_public const cairo_region_t* mume_urgnmgr_get_urgn(
    const void *self, const void *win);

/* Set the update region of the window <win>. */
mume_public void mume_urgnmgr_set_urgn(
    void *self, const void *win, const void *skip,
    const cairo_region_t *rgn, int operation, int recursive);

mume_public int mume_urgnmgr_pop_urgn(void *self);

mume_public const void* mume_urgnmgr_last_win(const void *self);

mume_public const cairo_region_t* mume_urgnmgr_last_rgn(const void *self);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_URGNMGR_H */
