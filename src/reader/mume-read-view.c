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
#include "mume-read-view.h"

#define _read_view_super_class mume_window_class

struct _read_view {
    const char _[MUME_SIZEOF_WINDOW];
};

MUME_STATIC_ASSERT(sizeof(struct _read_view) ==
                   MUME_SIZEOF_READ_VIEW);

static void* _read_view_ctor(
    struct _read_view *self, int mode, va_list *app)
{
    if (!_mume_ctor(_read_view_super_class(), self, mode, app))
        return NULL;

    return self;
}

static void* _read_view_dtor(struct _read_view *self)
{
    return _mume_dtor(_read_view_super_class(), self);
}

const void* mume_read_view_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_read_view_meta_class(),
        "read view",
        _read_view_super_class(),
        sizeof(struct _read_view),
        MUME_PROP_END,
        _mume_ctor, _read_view_ctor,
        _mume_dtor, _read_view_dtor,
        MUME_FUNC_END);
}

void* mume_read_view_new(void *parent, int x, int y, int w, int h)
{
    return mume_new(mume_read_view_class(), parent, x, y, w, h);
}
