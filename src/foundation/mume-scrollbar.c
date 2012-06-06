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
#include "mume-scrollbar.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-math.h"
#include "mume-memory.h"
#include "mume-objbase.h"
#include "mume-resmgr-private.h"
#include "mume-timer.h"
#include "mume-types.h"
#include MUME_ASSERT_H

#define _scrollbar_super_class mume_window_class

#define _SCROLLBAR_WAIT_TIME 500
#define _SCROLLBAR_SCROLL_TIME 100
#define _SCROLLBAR_MIN_THUMB_SIZE 14

#define _scrollbar_is_horz(_self) \
    (mume_test_flag((_self)->flags, _SCROLLBAR_FLAG_TOP) ||  \
     mume_test_flag((_self)->flags, _SCROLLBAR_FLAG_BOTTOM))

enum _scrollbar_flags_e {
    _SCROLLBAR_FLAG_RIGHT,
    _SCROLLBAR_FLAG_BOTTOM,
    _SCROLLBAR_FLAG_LEFT,
    _SCROLLBAR_FLAG_TOP,
    _SCROLLBAR_FLAG_INVALID
};

struct _scrollbar {
    const char _[MUME_SIZEOF_WINDOW];
    int size, page, pos;
    int thumb_offset;
    int thumb_pos;
    mume_timer_t *timer;
    short hover;
    short press;
    unsigned int flags;
};

struct _scrollbar_theme {
    mume_widget_bkgnd_t button1;
    mume_widget_bkgnd_t button2;
    mume_widget_bkgnd_t bkgnd;
    mume_widget_bkgnd_t thumb;
};

MUME_STATIC_ASSERT(sizeof(struct _scrollbar) ==
                   MUME_SIZEOF_SCROLLBAR);

static int _scrollbar_normalize_pos(
    const struct _scrollbar *self, int pos)
{
    int limit;

    if (pos < 0)
        return 0;

    limit = mume_scrollbar_get_limit(self);
    if (pos > limit)
        return limit;

    return pos;
}

static void _scrollbar_send_notify(
    struct _scrollbar *self, int hitcode, int position)
{
    mume_event_t event = mume_make_scroll_event(
        mume_window_parent(self), self, hitcode, position);

    mume_send_event(&event);
}

static int _scrollbar_pos2pixel(
    const struct _scrollbar *self, int space)
{
    if (self->size > 1) {
        int limit = mume_scrollbar_get_limit(self);
        return mume_int_mul_div(space, self->pos, limit);
    }

    return 0;
}

static int _scrollbar_pixel2pos(
    const struct _scrollbar *self, int pixel, int space)
{
    if (space > 0) {
        int limit = mume_scrollbar_get_limit(self);

        if (pixel <= space / 2) {
            return mume_int_mul_div(pixel, limit, space);
        }
        else {
            /* Ensure the thumb can stop at "max pos". */
            return limit - mume_int_mul_div(
                space - pixel, limit, space);
        }
    }

    return 0;
}

static void _scrollbar_get_part_infos(
    const struct _scrollbar *self, int *pbutton_size,
    int *pchannel_size, int *pthumb_size, int *pthumb_pos)
{
    int button_size, channel_size;
    int thumb_size, thumb_pos;

    if (_scrollbar_is_horz(self)) {
        int width = mume_window_width(self);

        button_size = mume_metrics(MUME_GM_CYVSCROLL);

        if (button_size + button_size < width) {
            channel_size = width - button_size - button_size;
        }
        else {
            button_size = width / 2;
            channel_size = 0;
        }
    }
    else {
        int height = mume_window_height(self);

        button_size = mume_metrics(MUME_GM_CXHSCROLL);

        if (button_size + button_size < height) {
            channel_size = height - button_size - button_size;
        }
        else {
            button_size = height / 2;
            channel_size = 0;
        }
    }

    thumb_size = 0;
    thumb_pos = 0;

    if (channel_size > 0) {
        if (self->page > 0 && self->size > self->page) {
            thumb_size = channel_size * self->page * 1.0 / self->size;
            thumb_size = MAX(thumb_size, _SCROLLBAR_MIN_THUMB_SIZE);
        }

        if (thumb_size > 0 && thumb_size < channel_size) {
            if (MUME_SBHT_THUMB == self->press) {
                thumb_pos = self->thumb_pos;
            }
            else {
                thumb_pos = button_size + _scrollbar_pos2pixel(
                    self, channel_size - thumb_size);
            }
        }
        else {
            thumb_size = 0;
        }
    }

    if (pbutton_size)
        *pbutton_size = button_size;

    if (pchannel_size)
        *pchannel_size = channel_size;

    if (pthumb_size)
        *pthumb_size = thumb_size;

    if (pthumb_pos)
        *pthumb_pos = thumb_pos;
}

static mume_rect_t _scrollbar_get_rect(
    const struct _scrollbar *self, int hitcode)
{
    mume_rect_t r;
    int button_size, thumb_size, thumb_pos;

    r.x = r.y = 0;
    mume_window_get_geometry(
        self, NULL, NULL, &r.width, &r.height);

    _scrollbar_get_part_infos(
        self, &button_size, NULL, &thumb_size, &thumb_pos);

    switch (hitcode) {
    case MUME_SBHT_BUTTON1:
        if (_scrollbar_is_horz(self))
            r.width = button_size;
        else
            r.height = button_size;
        break;

    case MUME_SBHT_BUTTON2:
        if (_scrollbar_is_horz(self)) {
            r.x = r.width - button_size;
            r.width = button_size;
        }
        else {
            r.y = r.height - button_size;
            r.height = button_size;
        }
        break;

    case MUME_SBHT_CHANNEL1:
        if (_scrollbar_is_horz(self)) {
            r.x = button_size;

            if (thumb_size > 0)
                r.width = thumb_pos - r.x;
            else
                r.width = 0;
        }
        else {
            r.y = button_size;

            if (thumb_size > 0)
                r.height = thumb_pos - r.y;
            else
                r.height = 0;
        }
        break;

    case MUME_SBHT_CHANNEL2:
        if (_scrollbar_is_horz(self)) {
            if (thumb_size > 0) {
                r.x = thumb_pos + thumb_size;
                r.width -= r.x + button_size;
            }
            else {
                r.x = button_size;
                r.width = 0;
            }
        }
        else {
            if (thumb_size > 0) {
                r.y = thumb_pos + thumb_size;
                r.height -= r.y + button_size;
            }
            else {
                r.y = button_size;
                r.height = 0;
            }
        }
        break;

    case MUME_SBHT_THUMB:
        if (_scrollbar_is_horz(self)) {
            r.x = thumb_pos;
            r.width = thumb_size;
        }
        else {
            r.y = thumb_pos;
            r.height = thumb_size;
        }
        break;
    }

    return r;
}

static mume_resobj_brush_t* _scrollbar_choose_brush(
    const struct _scrollbar *self,
    mume_widget_bkgnd_t *brushes, int hitcode)
{
    if (!mume_scrollbar_is_scrollable(self))
        return &brushes->disabled;

    if (self->press == hitcode)
        return &brushes->pressed;

    if (self->hover == hitcode)
        return &brushes->hot;

    return &brushes->normal;
}

static int _scrollbar_hittest(
    const struct _scrollbar *self, int x, int y)
{
    int i;
    mume_rect_t r;

    for (i = MUME_SBHT_BUTTON1; i <= MUME_SBHT_THUMB; ++i) {
        r = _scrollbar_get_rect(self, i);

        if (mume_rect_inside(r, x, y))
            return i;
    }

    return MUME_SBHT_NOWHERE;
}

static int _scrollbar_move_thumb(
    const struct _scrollbar *self, int x, int y, int *thumb_pos)
{
    int button_size, channel_size, thumb_size, thumb_xy;

    _scrollbar_get_part_infos(
        self, &button_size, &channel_size, &thumb_size, NULL);

    if (_scrollbar_is_horz(self))
        thumb_xy = x - self->thumb_offset;
    else
        thumb_xy = y - self->thumb_offset;

    if (thumb_xy < button_size) {
        thumb_xy = button_size;
    }
    else if (thumb_xy > button_size + channel_size - thumb_size)
    {
        thumb_xy = button_size + channel_size - thumb_size;
    }

    if (thumb_pos)
        *thumb_pos = thumb_xy;

    return _scrollbar_pixel2pos(
        self, thumb_xy - button_size, channel_size - thumb_size);
}

static void _scrollbar_kill_timers(struct _scrollbar *self)
{
    if (self->timer) {
        mume_cancel_timer(self->timer);
        mume_timer_delete(self->timer);
        self->timer = NULL;
    }
}

static int _scrollbar_handle_timer(mume_timer_t *tmr)
{
    struct _scrollbar *self = mume_timer_data(tmr);
    int x, y, hitcode;

    mume_query_pointer(self, &x, &y, NULL);
    hitcode = _scrollbar_hittest(self, x, y);

    if (hitcode == self->press)
        _scrollbar_send_notify(self, hitcode, self->pos);

    return _SCROLLBAR_SCROLL_TIME;
}

static cairo_matrix_t _scrollbar_get_matrix(
    const struct _scrollbar *self, int width, int height)
{
    mume_matrix_t m = mume_matrix_identity;

    if (mume_test_flag(self->flags, _SCROLLBAR_FLAG_BOTTOM)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(0, -height));
        m = mume_matrix_concat(m, mume_matrix_rotate(90));
    }
    else if (mume_test_flag(self->flags, _SCROLLBAR_FLAG_LEFT)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-width, -height));
        m = mume_matrix_concat(m, mume_matrix_rotate(180));
    }
    else if (mume_test_flag(self->flags, _SCROLLBAR_FLAG_TOP)) {
        m = mume_matrix_concat(
            m, mume_matrix_translate(-width, 0));
        m = mume_matrix_concat(m, mume_matrix_rotate(270));
    }

    return mume_matrix_to_cairo(m);
}

static void _scrollbar_invalidate(struct _scrollbar *self)
{
    if (!mume_test_flag(self->flags, _SCROLLBAR_FLAG_INVALID)) {
        mume_invalidate_region(self, NULL);
        mume_add_flag(self->flags, _SCROLLBAR_FLAG_INVALID);
    }
}

static void _scrollbar_invalidate_part(
    struct _scrollbar *self, int hitcode)
{
    if (hitcode != MUME_SBHT_NOWHERE) {
        mume_rect_t r = _scrollbar_get_rect(self, hitcode);
        mume_invalidate_rect(self, &r);
    }
}

static void* _scrollbar_ctor(
    struct _scrollbar *self, int mode, va_list *app)
{
    int type;

    if (!_mume_ctor(_scrollbar_super_class(), self, mode, app))
        return NULL;

    self->size = 0;
    self->page = 0;
    self->pos = 0;
    self->thumb_offset = 0;
    self->thumb_pos = 0;
    self->timer = NULL;
    self->hover = MUME_SBHT_NOWHERE;
    self->press = MUME_SBHT_NOWHERE;
    self->flags = 0;

    type = va_arg(*app, int);
    switch (type) {
    case MUME_SCROLLBAR_BOTTOM:
        mume_add_flag(self->flags, _SCROLLBAR_FLAG_BOTTOM);
        break;

    case MUME_SCROLLBAR_LEFT:
        mume_add_flag(self->flags, _SCROLLBAR_FLAG_LEFT);
        break;

    case MUME_SCROLLBAR_TOP:
        mume_add_flag(self->flags, _SCROLLBAR_FLAG_TOP);
        break;

    default:
        mume_add_flag(self->flags, _SCROLLBAR_FLAG_RIGHT);
    }

    return self;
}

static void* _scrollbar_dtor(struct _scrollbar *self)
{
    _scrollbar_kill_timers(self);
    return _mume_dtor(_scrollbar_super_class(), self);
}

static void _scrollbar_handle_button_down(
    struct _scrollbar *self, int x, int y, int state, int button)
{
    int hitcode;
    int notify = 0;
    mume_rect_t r;

    if (button != MUME_BUTTON_LEFT) {
        switch (button) {
        case MUME_BUTTON_WHEELUP:
            if (self->pos > 0) {
                _scrollbar_send_notify(
                    self, MUME_SBHT_BUTTON1, self->pos);
            }
            break;

        case MUME_BUTTON_WHEELDOWN:
            if (self->pos < mume_scrollbar_get_limit(self)) {
                _scrollbar_send_notify(
                    self, MUME_SBHT_BUTTON2, self->pos);
            }
            break;
        }

        return;
    }

    hitcode = _scrollbar_hittest(self, x, y);

    if (MUME_SBHT_NOWHERE == hitcode)
        return;

    switch (hitcode) {
    case MUME_SBHT_BUTTON1:
    case MUME_SBHT_CHANNEL1:
        notify = (self->pos > 0);
        break;

    case MUME_SBHT_BUTTON2:
    case MUME_SBHT_CHANNEL2:
        notify = (self->pos < mume_scrollbar_get_limit(self));
        break;

    case MUME_SBHT_THUMB:
        _scrollbar_get_part_infos(
            self, NULL, NULL, NULL, &self->thumb_pos);

        r = _scrollbar_get_rect(self, MUME_SBHT_THUMB);

        if (_scrollbar_is_horz(self)) {
            self->thumb_offset = x - r.x;
        }
        else {
            self->thumb_offset = y - r.y;
        }

        break;
    }

    self->press = hitcode;
    _scrollbar_invalidate_part(self, self->press);

    if (notify) {
        _scrollbar_send_notify(self, self->press, self->pos);

        if (NULL == self->timer) {
            self->timer = mume_timer_new(
                _scrollbar_handle_timer, self);

            mume_schedule_timer(self->timer, _SCROLLBAR_WAIT_TIME);
        }
    }
}

static void _scrollbar_handle_button_up(
    struct _scrollbar *self, int x, int y, int state, int button)
{
    if (button != MUME_BUTTON_LEFT)
        return;

    if (MUME_SBHT_NOWHERE != self->press) {
        _scrollbar_kill_timers(self);

        _scrollbar_invalidate_part(self, self->press);

        if (MUME_SBHT_THUMB == self->press) {
            self->press = MUME_SBHT_NOWHERE;
            _scrollbar_invalidate_part(self, MUME_SBHT_THUMB);
        }

        self->press = MUME_SBHT_NOWHERE;
    }
}

static void _scrollbar_handle_mouse_motion(
    struct _scrollbar *self, int x, int y, int state)
{
    int hitcode;

    hitcode = _scrollbar_hittest(self, x, y);

    if (MUME_SBHT_THUMB == self->press) {
        /* Drag thumb. */
        int thumb_pos, scroll_pos;

        scroll_pos = _scrollbar_move_thumb(self, x, y, &thumb_pos);
        if (scroll_pos != self->pos)
            _scrollbar_send_notify(self, self->press, scroll_pos);

        if (thumb_pos != self->thumb_pos) {
            mume_rect_t r;

            r = _scrollbar_get_rect(self, MUME_SBHT_THUMB);
            mume_invalidate_rect(self, &r);
            self->thumb_pos = thumb_pos;
            r = _scrollbar_get_rect(self, MUME_SBHT_THUMB);
            mume_invalidate_rect(self, &r);
        }
    }
    else if (self->hover != hitcode) {
        _scrollbar_invalidate_part(self, self->hover);
        self->hover = hitcode;
        _scrollbar_invalidate_part(self, self->hover);
    }
}

static void _scrollbar_handle_mouse_leave(
    struct _scrollbar *self, int x, int y, int state, int mode, int detail)
{
    if (MUME_SBHT_NOWHERE != self->hover) {
        _scrollbar_invalidate_part(self, self->hover);
        self->hover = MUME_SBHT_NOWHERE;
    }
}

static void _scrollbar_handle_expose(
    struct _scrollbar *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    cairo_matrix_t cm;
    mume_resobj_brush_t *br;
    mume_rect_t r, ru;
    struct _scrollbar_theme *theme;

    if (count > 0)
        return;

    theme = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "scrollbar", "theme"),
        mume_typeof_scrollbar_theme());

    if (NULL == theme) {
        mume_warning(("Get scrollbar theme failed\n"));
        return;
    }

    mume_remove_flag(self->flags, _SCROLLBAR_FLAG_INVALID);

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    ru = mume_current_invalid_rect();

    /* Background */
    if (_scrollbar_is_horz(self)) {
        mume_window_get_geometry(
            self, NULL, NULL, &r.height, &r.width);
    }
    else {
        mume_window_get_geometry(
            self, NULL, NULL, &r.width, &r.height);
    }

    cm = _scrollbar_get_matrix(self, r.width, r.height);

    cairo_transform(cr, &cm);

    if (mume_scrollbar_is_scrollable(self))
        br = &theme->bkgnd.normal;
    else
        br = &theme->bkgnd.disabled;

    mume_draw_resobj_brush(
        cr, br, 0, 0, r.width, r.height);

    cairo_matrix_invert(&cm);
    cairo_transform(cr, &cm);

    /* Button1. */
    r = _scrollbar_get_rect(self, MUME_SBHT_BUTTON1);
    if (!mume_rect_is_empty(r) &&
        !mume_rect_is_empty(mume_rect_intersect(ru, r)))
    {
        if (_scrollbar_is_horz(self))
            SWAP(r.width, r.height);

        cm = _scrollbar_get_matrix(self, r.width, r.height);

        cairo_translate(cr, r.x, r.y);
        cairo_transform(cr, &cm);

        if (mume_test_flag(self->flags, _SCROLLBAR_FLAG_TOP) ||
            mume_test_flag(self->flags, _SCROLLBAR_FLAG_RIGHT))
        {
            br = _scrollbar_choose_brush(
                self, &theme->button1, MUME_SBHT_BUTTON1);
        }
        else {
            br = _scrollbar_choose_brush(
                self, &theme->button2, MUME_SBHT_BUTTON1);
        }

        mume_draw_resobj_brush(
            cr, br, 0, 0, r.width, r.height);

        cairo_matrix_invert(&cm);
        cairo_transform(cr, &cm);
        cairo_translate(cr, -r.x, -r.y);
    }

    /* Button2. */
    r = _scrollbar_get_rect(self, MUME_SBHT_BUTTON2);
    if (!mume_rect_is_empty(r) &&
        !mume_rect_is_empty(mume_rect_intersect(ru, r)))
    {
        if (_scrollbar_is_horz(self))
            SWAP(r.width, r.height);

        cm = _scrollbar_get_matrix(self, r.width, r.height);

        cairo_translate(cr, r.x, r.y);
        cairo_transform(cr, &cm);

        if (mume_test_flag(self->flags, _SCROLLBAR_FLAG_TOP) ||
            mume_test_flag(self->flags, _SCROLLBAR_FLAG_RIGHT))
        {
            br = _scrollbar_choose_brush(
                self, &theme->button2, MUME_SBHT_BUTTON2);
        }
        else {
            br = _scrollbar_choose_brush(
                self, &theme->button1, MUME_SBHT_BUTTON2);
        }

        mume_draw_resobj_brush(
            cr, br, 0, 0, r.width, r.height);

        cairo_matrix_invert(&cm);
        cairo_transform(cr, &cm);
        cairo_translate(cr, -r.x, -r.y);
    }

    /* Thumb. */
    r = _scrollbar_get_rect(self, MUME_SBHT_THUMB);
    if (!mume_rect_is_empty(r) &&
        !mume_rect_is_empty(mume_rect_intersect(ru, r)))
    {
        if (_scrollbar_is_horz(self))
            SWAP(r.width, r.height);

        cm = _scrollbar_get_matrix(self, r.width, r.height);

        cairo_translate(cr, r.x, r.y);
        cairo_transform(cr, &cm);

        br = _scrollbar_choose_brush(
            self, &theme->thumb, MUME_SBHT_THUMB);

        mume_draw_resobj_brush(
            cr, br, 0, 0, r.width, r.height);

        cairo_matrix_invert(&cm);
        cairo_transform(cr, &cm);
        cairo_translate(cr, -r.x, -r.y);
    }

    mume_window_end_paint(self, cr);
}

const void* mume_scrollbar_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_scrollbar_meta_class(),
        "scrollbar",
        _scrollbar_super_class(),
        sizeof(struct _scrollbar),
        MUME_PROP_END,
        _mume_ctor, _scrollbar_ctor,
        _mume_dtor, _scrollbar_dtor,
        _mume_window_handle_button_down,
        _scrollbar_handle_button_down,
        _mume_window_handle_button_up,
        _scrollbar_handle_button_up,
        _mume_window_handle_mouse_motion,
        _scrollbar_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _scrollbar_handle_mouse_leave,
        _mume_window_handle_expose,
        _scrollbar_handle_expose,
        MUME_FUNC_END);
}

void* mume_scrollbar_new(
    void *parent, int x, int y, int w, int h, int type)
{
    return mume_new(mume_scrollbar_class(),
                    parent, x, y, w, h, type);
}

void mume_scrollbar_set_size(void *_self, int size)
{
    struct _scrollbar *self = _self;

    assert(mume_is_of(_self, mume_scrollbar_class()));

    if (self->size != size) {
        self->size = size;
        self->pos = _scrollbar_normalize_pos(self, self->pos);

        _scrollbar_invalidate(self);
    }
}

int mume_scrollbar_get_size(const void *_self)
{
    const struct _scrollbar *self = _self;
    assert(mume_is_of(_self, mume_scrollbar_class()));
    return self->size;
}

void mume_scrollbar_set_page(void *_self, int page)
{
    struct _scrollbar *self = _self;

    assert(mume_is_of(_self, mume_scrollbar_class()));

    if (self->page != page) {
        self->page = page;
        self->pos = _scrollbar_normalize_pos(self, self->pos);

        _scrollbar_invalidate(self);
    }
}

int mume_scrollbar_get_page(const void *_self)
{
    const struct _scrollbar *self = _self;
    assert(mume_is_of(_self, mume_scrollbar_class()));
    return self->page;
}

void mume_scrollbar_set_pos(void *_self, int pos)
{
    struct _scrollbar *self = _self;

    assert(mume_is_of(_self, mume_scrollbar_class()));

    pos = _scrollbar_normalize_pos(self, pos);

    if (self->pos != pos) {
        self->pos = pos;

        if (MUME_SBHT_THUMB != self->press)
            _scrollbar_invalidate(self);
    }
}

int mume_scrollbar_get_pos(const void *_self)
{
    const struct _scrollbar *self = _self;
    assert(mume_is_of(_self, mume_scrollbar_class()));
    return self->pos;
}

int mume_scrollbar_get_limit(const void *_self)
{
    const struct _scrollbar *self = _self;
    assert(mume_is_of(_self, mume_scrollbar_class()));
    return self->size > self->page ? self->size - self->page : 0;
}

int mume_scrollbar_is_scrollable(const void *_self)
{
    const struct _scrollbar *self = _self;
    assert(mume_is_of(_self, mume_scrollbar_class()));
    return (self->page > 0 && self->size > self->page);
}

mume_type_t* mume_typeof_scrollbar_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _scrollbar_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), button1);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), button2);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), bkgnd);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), thumb);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
