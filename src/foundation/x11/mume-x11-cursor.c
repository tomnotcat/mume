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
#include "mume-x11-cursor.h"
#include "mume-debug.h"
#include <X11/cursorfont.h>
#include MUME_ASSERT_H

#define _x11_cursor_super_class mume_cursor_class

struct _x11_cursor {
    const char _[MUME_SIZEOF_CURSOR];
    Display *display;
    Cursor cursor;
};

struct _x11_cursor_class {
    const char _[MUME_SIZEOF_CURSOR_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _x11_cursor) ==
                   MUME_SIZEOF_X11_CURSOR);

MUME_STATIC_ASSERT(sizeof(struct _x11_cursor_class) ==
                   MUME_SIZEOF_X11_CURSOR_CLASS);

static void* _x11_cursor_ctor(
    struct _x11_cursor *self, int mode, va_list *app)
{
    if (!_mume_ctor(_x11_cursor_super_class(), self, mode, app))
        return NULL;

    self->display = va_arg(*app, Display*);
    self->cursor = va_arg(*app, Cursor);
    return self;
}

static void* _x11_cursor_dtor(struct _x11_cursor *self)
{
    XFreeCursor(self->display, self->cursor);
    return _mume_dtor(_x11_cursor_super_class(), self);
}

const void* mume_x11_cursor_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_x11_cursor_meta_class(),
        "x11 cursor",
        _x11_cursor_super_class(),
        sizeof(struct _x11_cursor),
        MUME_PROP_END,
        _mume_ctor, _x11_cursor_ctor,
        _mume_dtor, _x11_cursor_dtor,
        MUME_FUNC_END);
}

Cursor mume_x11_create_cursor(Display *display, int id)
{
    switch (id) {
    case MUME_CURSOR_ARROW:
        return XCreateFontCursor(display, XC_left_ptr);

    case MUME_CURSOR_HDBLARROW:
        return XCreateFontCursor(display, XC_sb_h_double_arrow);

    case MUME_CURSOR_VDBLARROW:
        return XCreateFontCursor(display, XC_sb_v_double_arrow);

    case MUME_CURSOR_HAND:
        return XCreateFontCursor(display, XC_hand2);

    case MUME_CURSOR_IBEAM:
        return XCreateFontCursor(display, XC_xterm);

    case MUME_CURSOR_WAIT:
        return XCreateFontCursor(display, XC_watch);
    }

    return None;
}

void* mume_x11_cursor_new(Display *display, Cursor cursor)
{
    return mume_new(mume_x11_cursor_class(),
                    display, cursor);
}

Cursor mume_x11_cursor_get_entity(const void *_self)
{
    const struct _x11_cursor *self = _self;
    assert(mume_is_of(_self, mume_x11_cursor_class()));
    return self->cursor;
}
