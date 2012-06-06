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
#include "mume-listview.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _listview_super_class mume_scrollview_class
#define _listview_super_meta_class mume_scrollview_meta_class

struct _listview {
    const char _[MUME_SIZEOF_SCROLLVIEW];
};

struct _listview_class {
    const char _[MUME_SIZEOF_SCROLLVIEW_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _listview) == MUME_SIZEOF_LISTVIEW);
MUME_STATIC_ASSERT(sizeof(struct _listview_class) ==
                   MUME_SIZEOF_LISTVIEW_CLASS);

static void* _listview_ctor(
    struct _listview *self, int mode, va_list *app)
{
    if (!_mume_ctor(_listview_super_class(), self, mode, app))
        return NULL;

    return self;
}

static void* _listview_dtor(struct _listview *self)
{
    return _mume_dtor(_listview_super_class(), self);
}

static void _listview_handle_expose(
    struct _listview *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;

    if (count)
        return;

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    cairo_rectangle(cr, x, y, w, h);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_fill(cr);

    mume_window_end_paint(self, cr);
}

static void* _listview_class_ctor(
    struct _listview_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_listview_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        (void)method;
    }

    return self;
}

const void* mume_listview_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_listview_meta_class(),
        "listview",
        _listview_super_class(),
        sizeof(struct _listview),
        MUME_PROP_END,
        _mume_ctor, _listview_ctor,
        _mume_dtor, _listview_dtor,
        _mume_window_handle_expose,
        _listview_handle_expose,
        MUME_FUNC_END);
}

const void* mume_listview_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "listview class",
        _listview_super_meta_class(),
        sizeof(struct _listview_class),
        MUME_PROP_END,
        _mume_ctor, _listview_class_ctor,
        MUME_FUNC_END);
}

void* mume_listview_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_listview_class(),
                    parent, x, y, width, height);
}
