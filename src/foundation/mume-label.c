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
#include "mume-label.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include MUME_ASSERT_H

#define _label_super_class mume_window_class

struct _label {
    const char _[MUME_SIZEOF_WINDOW];
    mume_resobj_brush_t *bkgnd;
};

MUME_STATIC_ASSERT(sizeof(struct _label) == MUME_SIZEOF_LABEL);

static void _label_handle_expose(
    struct _label *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;

    if (count)
        return;

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    mume_window_get_geometry(self, NULL, NULL, &w, &h);
    if (self->bkgnd)
        mume_draw_resobj_brush(cr, self->bkgnd, 0, 0, w, h);

    mume_window_end_paint(self, cr);
}

static void* _label_ctor(
    struct _label *self, int mode, va_list *app)
{
    if (!_mume_ctor(_label_super_class(), self, mode, app))
        return NULL;

    self->bkgnd = NULL;
    return self;
}

const void* mume_label_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_label_meta_class(),
        "label",
        _label_super_class(),
        sizeof(struct _label),
        MUME_PROP_END,
        _mume_ctor, _label_ctor,
        _mume_window_handle_expose,
        _label_handle_expose,
        MUME_FUNC_END);
}

void* mume_label_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_label_class(),
                    parent, x, y, width, height);
}

void mume_label_set_bkgnd(void *_self, mume_resobj_brush_t *bkgnd)
{
    struct _label *self = _self;

    assert(mume_is_of(_self, mume_label_class()));

    self->bkgnd = bkgnd;
    mume_invalidate_region(self, NULL);
}
