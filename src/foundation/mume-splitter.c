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
#include "mume-splitter.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-objbase.h"
#include "mume-resmgr-private.h"
#include "mume-types.h"
#include MUME_ASSERT_H
#include MUME_MATH_H

#define _SPLITTER_PART_COUNT 2
#define _SPLITTER_COMMAND_UPDATE (MUME_NUM_COMMANDS + 1)

#define _splitter_super_class mume_window_class

#define _splitter_is_horz(_self) \
    (mume_test_flag((_self)->flags, _SPLITTER_FLAG_TOP) ||  \
     mume_test_flag((_self)->flags, _SPLITTER_FLAG_BOTTOM))

enum _splitter_flags_e {
    _SPLITTER_FLAG_TOP,
    _SPLITTER_FLAG_BOTTOM,
    _SPLITTER_FLAG_LEFT,
    _SPLITTER_FLAG_RIGHT,
    _SPLITTER_FLAG_PRESSED,
    _SPLITTER_FLAG_CHANGED
};

struct _split_part {
    void *window;
    int min;
    int max;
};

struct _splitter {
    const char _[MUME_SIZEOF_WINDOW];
    unsigned int flags;
    float ratio_pos;
    int offset;
    struct _split_part parts[_SPLITTER_PART_COUNT];
};

struct _splitter_theme {
    int bar_width;
    mume_point_t grip_size;
    mume_resobj_brush_t bar_bg;
    mume_resobj_brush_t grip_bg;
    mume_resobj_cursor_t horz_cursor;
    mume_resobj_cursor_t vert_cursor;
};

MUME_STATIC_ASSERT(sizeof(struct _splitter) == MUME_SIZEOF_SPLITTER);

static struct _splitter_theme*  _splitter_get_theme(
    const struct _splitter *self)
{
    struct _splitter_theme *theme = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "splitter", "theme"),
        mume_typeof_splitter_theme());

    if (NULL == theme)
        mume_abort(("Get splitter theme failed\n"));

    return theme;
}

static int _splitter_get_max_pos(const struct _splitter *self)
{
    struct _splitter_theme *theme;
    int max;

    theme = _splitter_get_theme(self);
    if (_splitter_is_horz(self))
        max = mume_window_height(self) - theme->bar_width;
    else
        max = mume_window_width(self) - theme->bar_width;

    return MAX(max, 0);
}

static mume_rect_t _splitter_get_bar_rect(
    const struct _splitter *self)
{
    struct _splitter_theme *theme;
    mume_rect_t rect;

    theme = _splitter_get_theme(self);

    if (_splitter_is_horz(self)) {
        rect.x = 0;
        rect.y = mume_splitter_get_pos(self);
        rect.width = mume_window_width(self);
        rect.height = theme->bar_width;;
    }
    else {
        rect.x = mume_splitter_get_pos(self);
        rect.y = 0;
        rect.width = theme->bar_width;
        rect.height = mume_window_height(self);
    }

    return rect;
}

static mume_rect_t _splitter_get_part_rect(
    const struct _splitter *self, int i)
{
    struct _splitter_theme *theme;
    mume_rect_t rect;
    int width, height, pos;

    theme = _splitter_get_theme(self);
    mume_window_get_geometry(self, NULL, NULL, &width, &height);
    pos = mume_splitter_get_pos(self);

    if (_splitter_is_horz(self)) {
        rect.x = 0;
        rect.width = width;

        if (0 == i) {
            rect.y = 0;
            rect.height = pos;
        }
        else {
            rect.y = pos + theme->bar_width;
            rect.height = height - pos - theme->bar_width;
        }
    }
    else {
        rect.y = 0;
        rect.height = height;

        if (0 == i) {
            rect.x = 0;
            rect.width = pos;
        }
        else {
            rect.x = pos + theme->bar_width;
            rect.width = width - pos - theme->bar_width;
        }
    }

    return rect;
}

static int _splitter_validate_pos(struct _splitter *self)
{
    int pos, max, part1;

    pos = mume_splitter_get_pos(self);
    max = _splitter_get_max_pos(self);

    if (pos < self->parts[0].min)
        pos = self->parts[0].min;

    if (pos > self->parts[0].max)
        pos = self->parts[0].max;

    part1 = max - pos;
    if (part1 < self->parts[1].min)
        pos -= self->parts[1].min - part1;

    if (part1 > self->parts[1].max)
        pos += part1 - self->parts[1].max;

    if (pos < 0)
        pos = 0;
    else if (pos > max)
        pos = max;

    if (pos != mume_splitter_get_pos(self)) {
        self->ratio_pos = (float)pos / max;
        return 1;
    }

    return 0;
}

static void _splitter_layout_windows(struct _splitter *self)
{
    struct _splitter_theme *theme;
    int pos, width, height;

    theme = _splitter_get_theme(self);
    pos = mume_splitter_get_pos(self);

    mume_window_get_geometry(
        self, NULL, NULL, &width, &height);

    if (_splitter_is_horz(self)) {
        if (self->parts[0].window) {
            mume_window_set_geometry(
                self->parts[0].window, 0, 0, width, pos);
        }

        if (self->parts[1].window) {
            mume_window_set_geometry(
                self->parts[1].window,
                0, pos + theme->bar_width,
                width, height - pos - theme->bar_width);
        }
    }
    else {
        if (self->parts[0].window) {
            mume_window_set_geometry(
                self->parts[0].window, 0, 0, pos, height);
        }

        if (self->parts[1].window) {
            mume_window_set_geometry(
                self->parts[1].window,
                pos + theme->bar_width, 0,
                width - pos - theme->bar_width, height);
        }
    }
}

static void _splitter_handle_button_down(
    struct _splitter *self, int x, int y, int state, int button)
{
    mume_rect_t r;

    if (button != MUME_BUTTON_LEFT)
        return;

    r = _splitter_get_bar_rect(self);

    if (mume_rect_inside(r, x, y)) {
        mume_add_flag(self->flags, _SPLITTER_FLAG_PRESSED);

        if (_splitter_is_horz(self))
            self->offset = y - r.y;
        else
            self->offset = x - r.x;
    }
}

static void _splitter_handle_button_up(
    struct _splitter *self, int x, int y, int state, int button)
{
    if (button != MUME_BUTTON_LEFT)
        return;

    mume_remove_flag(self->flags, _SPLITTER_FLAG_PRESSED);

    if (mume_test_flag(self->flags, _SPLITTER_FLAG_CHANGED)) {
        mume_event_t event;

        event = mume_make_notify_event(
            self, self, MUME_SPLITTER_POS_CHANGED,
            (void*)(intptr_t)mume_splitter_get_pos(self));

        mume_send_event(&event);

        mume_remove_flag(self->flags, _SPLITTER_FLAG_CHANGED);
    }
}

static void _splitter_handle_mouse_motion(
    struct _splitter *self, int x, int y, int state)
{
    if (mume_test_flag(self->flags, _SPLITTER_FLAG_PRESSED)) {
        int old_pos = 0;

        if (!mume_test_flag(self->flags, _SPLITTER_FLAG_CHANGED))
            old_pos = mume_splitter_get_pos(self);

        if (_splitter_is_horz(self))
            mume_splitter_set_pos(self, y - self->offset);
        else
            mume_splitter_set_pos(self, x - self->offset);

        if (!mume_test_flag(self->flags, _SPLITTER_FLAG_CHANGED)) {
            if (mume_splitter_get_pos(self) != old_pos)
                mume_add_flag(self->flags, _SPLITTER_FLAG_CHANGED);
        }
    }
    else {
        struct _splitter_theme *theme = _splitter_get_theme(self);
        mume_resobj_cursor_t *cursor;

        if (_splitter_is_horz(self))
            cursor = &theme->vert_cursor;
        else
            cursor = &theme->horz_cursor;

        if (cursor->p) {
            mume_rect_t r = _splitter_get_bar_rect(self);
            if (mume_rect_inside(r, x, y))
                mume_window_set_cursor(self, cursor->p);
            else
                mume_window_set_cursor(self, NULL);
        }
    }
}

static void _splitter_handle_mouse_leave(
    struct _splitter *self, int x, int y,
    int state, int mode, int detail)
{
    if (!mume_test_flag(self->flags, _SPLITTER_FLAG_PRESSED))
        mume_window_set_cursor(self, NULL);
}

static void _splitter_paint_bar(
    struct _splitter *self, cairo_t *cr,
    struct _splitter_theme *theme, mume_rect_t r)
{
    mume_matrix_t m;
    cairo_matrix_t cm;
    int width, height;

    if (_splitter_is_horz(self)) {
        width = r.height;
        height = r.width;
        m = mume_matrix_translate(r.y, r.x);
    }
    else {
        width = r.width;
        height = r.height;
        m = mume_matrix_translate(r.x, r.y);
    }

    if (mume_test_flag(self->flags, _SPLITTER_FLAG_RIGHT)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-width, -height));
        m = mume_matrix_concat(m, mume_matrix_rotate(180));
    }
    else if (mume_test_flag(self->flags, _SPLITTER_FLAG_TOP)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(0, -height));
        m = mume_matrix_concat(m, mume_matrix_rotate(90));
    }
    else if (mume_test_flag(self->flags, _SPLITTER_FLAG_BOTTOM)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-width, 0));
        m = mume_matrix_concat(m, mume_matrix_rotate(270));
    }

    cm = mume_matrix_to_cairo(m);
    cairo_transform(cr, &cm);

    mume_draw_resobj_brush(
        cr, &theme->bar_bg, 0, 0, width, height);

    if (theme->grip_size.x > 0 && theme->grip_size.y > 0) {
        mume_draw_resobj_brush(
            cr, &theme->grip_bg,
            (theme->bar_width - theme->grip_size.x) / 2,
            (height - theme->grip_size.y) / 2,
            theme->grip_size.x, theme->grip_size.y);
    }

    cairo_matrix_invert(&cm);
    cairo_transform(cr, &cm);
}

static void _splitter_handle_expose(
    struct _splitter *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    struct _splitter_theme *theme;
    mume_rect_t r;
    int i;

    if (count)
        return;

    theme = _splitter_get_theme(self);

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    r = _splitter_get_bar_rect(self);
    _splitter_paint_bar(self, cr, theme, r);

    for (i = 0; i < _SPLITTER_PART_COUNT; ++i) {
        if (NULL == self->parts[i].window ||
            !mume_window_is_mapped(self->parts[i].window))
        {
            r = _splitter_get_part_rect(self, i);

            _mume_window_handle_expose(
                _splitter_super_class(), self,
                r.x, r.y, r.width, r.height, 0);
        }
    }

    mume_window_end_paint(self, cr);
}

static void _splitter_handle_create(
    struct _splitter *self, void *window, int x, int y, int w, int h)
{
    if (self != window) {
        int i;
        for (i = 0; i < _SPLITTER_PART_COUNT; ++i) {
            if (NULL == self->parts[i].window) {
                self->parts[i].window = window;
                /* Don't call _splitter_update_structure directly.
                 * The child is not completely initialized at
                 * this time. */
                mume_post_event(mume_make_command_event(
                    self, self, _SPLITTER_COMMAND_UPDATE));
                break;
            }
        }
    }
}

static void _splitter_handle_destroy(
    struct _splitter *self, void *window)
{
    if (self != window) {
        int i;
        for (i = 0; i < _SPLITTER_PART_COUNT; ++i) {
            if (window == self->parts[i].window) {
                self->parts[i].window = NULL;
                break;
            }
        }
    }
}

static void _splitter_handle_resize(
    struct _splitter *self, void *window, int w, int h, int ow, int oh)
{
    if (self == window) {
        _splitter_validate_pos(self);
        _splitter_layout_windows(self);
    }
}

static void _splitter_handle_command(
    void *self, void *window, int command)
{
    if (self == window && command == _SPLITTER_COMMAND_UPDATE)
        _splitter_layout_windows(self);
}

static void* _splitter_ctor(
    struct _splitter *self, int mode, va_list *app)
{
    int i, type;

    if (!_mume_ctor(_splitter_super_class(), self, mode, app))
        return NULL;

    type = va_arg(*app, int);
    self->flags = 0;
    self->ratio_pos = 0;
    self->offset = 0;

    switch (type) {
    case MUME_SPLITTER_RIGHT:
        mume_add_flag(self->flags, _SPLITTER_FLAG_RIGHT);
        break;
    case MUME_SPLITTER_TOP:
        mume_add_flag(self->flags, _SPLITTER_FLAG_TOP);
        break;

    case MUME_SPLITTER_BOTTOM:
        mume_add_flag(self->flags, _SPLITTER_FLAG_BOTTOM);
        break;

    default:
        mume_add_flag(self->flags, _SPLITTER_FLAG_LEFT);
    }

    for (i = 0; i < _SPLITTER_PART_COUNT; ++i) {
        self->parts[i].window = NULL;
        self->parts[i].min = 0;
        self->parts[i].max = MUME_SPLITTER_MAX_POS;
    }

    return self;
}

const void* mume_splitter_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_splitter_meta_class(),
        "splitter",
        _splitter_super_class(),
        sizeof(struct _splitter),
        MUME_PROP_END,
        _mume_ctor, _splitter_ctor,
        _mume_window_handle_button_down,
        _splitter_handle_button_down,
        _mume_window_handle_button_up,
        _splitter_handle_button_up,
        _mume_window_handle_mouse_motion,
        _splitter_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _splitter_handle_mouse_leave,
        _mume_window_handle_expose,
        _splitter_handle_expose,
        _mume_window_handle_create,
        _splitter_handle_create,
        _mume_window_handle_destroy,
        _splitter_handle_destroy,
        _mume_window_handle_resize,
        _splitter_handle_resize,
        _mume_window_handle_command,
        _splitter_handle_command,
        MUME_FUNC_END);
}

void* mume_splitter_new(
    void *parent, int x, int y, int w, int h, int type)
{
    return mume_new(mume_splitter_class(),
                    parent, x, y, w, h, type);
}

void mume_splitter_set_window(void *_self, int part, void *window)
{
    struct _splitter *self = _self;

    assert(mume_is_of(_self, mume_splitter_class()));
    assert(part >= 0 && part < _SPLITTER_PART_COUNT);
    assert(NULL == window || mume_window_parent(window) == self);

    if (self->parts[part].window != window) {
        self->parts[part].window = window;
        _splitter_layout_windows(self);
    }
}

void mume_splitter_set_size(
    void *_self, int part, int min, int max)
{
    struct _splitter *self = _self;

    assert(mume_is_of(_self, mume_splitter_class()));
    assert(part >= 0 && part < _SPLITTER_PART_COUNT);
    assert(min >= 0 && max >= min);

    self->parts[part].min = min;
    self->parts[part].max = max;

    if (_splitter_validate_pos(self)) {
        _splitter_layout_windows(self);
        mume_invalidate_region(self, NULL);
    }
}

void mume_splitter_set_pos(void *_self, int pos)
{
    struct _splitter *self = _self;
    int max;

    assert(mume_is_of(_self, mume_splitter_class()));

    max = _splitter_get_max_pos(self);
    if (max > 0)
        mume_splitter_set_ratio_pos(self, (float)pos / max);
}

int mume_splitter_get_pos(const void *_self)
{
    const struct _splitter *self = _self;
    assert(mume_is_of(_self, mume_splitter_class()));
    return roundf(self->ratio_pos * _splitter_get_max_pos(self));
}

void mume_splitter_set_ratio_pos(void *_self, float pos)
{
    struct _splitter *self = _self;

    assert(mume_is_of(_self, mume_splitter_class()));

#define EPSINON 0.00001
    if (fabs(self->ratio_pos - pos) > EPSINON) {
        mume_rect_t r;

        r = _splitter_get_bar_rect(self);
        mume_invalidate_rect(self, &r);

        self->ratio_pos = pos;
        _splitter_validate_pos(self);
        _splitter_layout_windows(self);

        r = _splitter_get_bar_rect(self);
        mume_invalidate_rect(self, &r);
    }
#undef EPSINON
}

float mume_splitter_get_ratio_pos(const void *_self)
{
    const struct _splitter *self = _self;
    assert(mume_is_of(_self, mume_splitter_class()));
    return self->ratio_pos;
}

mume_type_t* mume_typeof_splitter_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _splitter_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), bar_width);
        MUME_DIRECT_PROPERTY(_mume_typeof_point(), grip_size);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), bar_bg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), grip_bg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_cursor(), horz_cursor);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_cursor(), vert_cursor);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
