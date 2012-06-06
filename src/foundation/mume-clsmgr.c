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
#include "mume-clsmgr.h"

static int _clsmgr_message(mume_object_t *obj, mume_message_t *msg)
{
    return 0;
}

const mume_class_t* mume_clsmgr_class(void)
{
    static mume_class_t clazz = {
        "clsmgr",
        NULL,
        sizeof(mume_clsmgr_t),
        _clsmgr_message,
    };

    return &clazz;
}
