/* Mume Reader - a full featured reading environment.
 *
 * Copyright © 2012 Soft Flag, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "mume-virtfs2.h"
#include "mume-gstate.h"

static int _virtfs_message(mume_object_t *obj, mume_message_t *msg)
{
    return 0;
}

const mume_class_t* mume_virtfs_class(void)
{
    static const mume_class_t *clazz;
    if (NULL == clazz) {
        clazz = mume_register_class(
            "virtfs",
            sizeof(mume_class_t),
            NULL,
            NULL,
            mume_object_class2(),
            sizeof(mume_virtfs2_t),
            _virtfs_message);
    }

    return clazz;
}
