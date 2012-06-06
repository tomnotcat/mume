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
#include "mume-scrollview.h"
#include "mume-debug.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-scrollbar.h"
#include MUME_ASSERT_H

#define _scrollview_super_class mume_window_class

struct _scrollview {
    const char _[MUME_SIZEOF_WINDOW];
    void *horz_bar;
    void *vert_bar;
    int page_cx;
    int page_cy;
    int line_cx;
    int line_cy;
};

MUME_STATIC_ASSERT(sizeof(struct _scrollview) ==
                   MUME_SIZEOF_SCROLLVIEW);

static void _scrollview_calc_pages(struct _scrollview *self)
{
    int width, height;
    int hscroable, hmapped, hscb_size;
    int vscroable, vmapped, vscb_size;
    int horz_ppp, vert_ppp;
    int horz_page, vert_page;

    mume_scrollview_get_client(self, NULL, NULL, &width, &height);

    hscroable = mume_scrollbar_is_scrollable(self->horz_bar);
    hmapped = mume_window_is_mapped(self->horz_bar);
    hscb_size = mume_window_height(self->horz_bar);
    vscroable = mume_scrollbar_is_scrollable(self->vert_bar);
    vmapped = mume_window_is_mapped(self->vert_bar);
    vscb_size = mume_window_width(self->vert_bar);

    horz_ppp = 1;
    vert_ppp = 1;

    if (hscroable && !hmapped) {
        /* Horizontal bar become visible */
        vert_page = (height - hscb_size) / vert_ppp;
        mume_scrollbar_set_page(self->vert_bar, vert_page);

        if (mume_scrollbar_is_scrollable(self->vert_bar) && !vmapped) {
            horz_page = (width - vscb_size) / horz_ppp;
            mume_scrollbar_set_page(self->horz_bar, horz_page);
        }
    }
    else if (!hscroable && hmapped) {
        /* Horizontal bar become invisible */
        vert_page = (height + hscb_size) / vert_ppp;
        mume_scrollbar_set_page(self->vert_bar, vert_page);

        if (!mume_scrollbar_is_scrollable(self->vert_bar) && vmapped) {
            horz_page = (width - vscb_size) / horz_ppp;
            mume_scrollbar_set_page(self->horz_bar, horz_page);
        }
    }
    else if (vscroable && !vmapped) {
        /* Vertical bar become visible */
        horz_page = (width - vscb_size) / horz_ppp;
        mume_scrollbar_set_page(self->horz_bar, horz_page);

        if (mume_scrollbar_is_scrollable(self->horz_bar) && !hmapped) {
            vert_page = (height - hscb_size) / vert_ppp;
            mume_scrollbar_set_page(self->vert_bar, vert_page);
        }
    }
    else if (!vscroable && vmapped) {
        /* Vertical bar become invisible */
        horz_page = (width + vscb_size) / horz_ppp;
        mume_scrollbar_set_page(self->horz_bar, horz_page);

        if (!mume_scrollbar_is_scrollable(self->horz_bar) && hmapped) {
            vert_page = (height + hscb_size) / vert_ppp;
            mume_scrollbar_set_page(self->vert_bar, vert_page);
        }
    }
}

static void _scrollview_update_bars(struct _scrollview *self)
{
    int x, y, w, h;

    _scrollview_calc_pages(self);

    if (mume_scrollbar_is_scrollable(self->horz_bar)) {
        x = 0;
        h = mume_window_height(self->horz_bar);
        y = mume_window_height(self) - h;
        w = mume_window_width(self);

        if (mume_scrollbar_is_scrollable(self->vert_bar))
            w -= mume_window_width(self->vert_bar);

        mume_window_set_geometry(self->horz_bar, x, y, w, h);
        mume_window_map(self->horz_bar);
    }
    else {
        mume_window_unmap(self->horz_bar);
    }

    if (mume_scrollbar_is_scrollable(self->vert_bar)) {
        w = mume_window_width(self->vert_bar);
        x = mume_window_width(self) - w;
        y = 0;
        h = mume_window_height(self);

        if (mume_scrollbar_is_scrollable(self->horz_bar))
            h -= mume_window_height(self->horz_bar);
        mume_window_set_geometry(
            self->vert_bar, x, y, w, h);
        mume_window_map(self->vert_bar);
    }
    else {
        mume_window_unmap(self->vert_bar);
    }
}

static void* _scrollview_ctor(
    struct _scrollview *self, int mode, va_list *app)
{
    int width, height;

    if (!_mume_ctor(_scrollview_super_class(), self, mode, app))
        return NULL;

    self->horz_bar = mume_scrollbar_new(
        self, 0, 0, 0, mume_metrics(MUME_GM_CYHSCROLL),
        MUME_SCROLLBAR_BOTTOM);

    self->vert_bar = mume_scrollbar_new(
        self, 0, 0, mume_metrics(MUME_GM_CXVSCROLL), 0,
        MUME_SCROLLBAR_RIGHT);

    self->page_cx = -1;
    self->page_cy = -1;
    self->line_cx = -1;
    self->line_cy = -1;

    mume_scrollview_get_client(
        self, NULL, NULL, &width, &height);

    mume_scrollbar_set_page(self->horz_bar, width);
    mume_scrollbar_set_page(self->vert_bar, height);

    return self;
}

static void* _scrollview_dtor(struct _scrollview *self)
{
    mume_delete(self->horz_bar);
    mume_delete(self->vert_bar);

    return _mume_dtor(_scrollview_super_class(), self);
}

static void _scrollview_handle_button_down(
    struct _scrollview *self, int x, int y, int state, int button)
{
    if (state)
        return;

    mume_scrollview_get_scroll(self, &x, &y);

    switch (button) {
    case MUME_BUTTON_WHEELUP:
        mume_scrollview_set_scroll(self, x, y - self->line_cy);
        break;

    case MUME_BUTTON_WHEELDOWN:
        mume_scrollview_set_scroll(self, x, y + self->line_cy);
        break;
    }
}

static void _scrollview_handle_resize(
    struct _scrollview *self, void *window, int w, int h, int ow, int oh)
{
    if (self == window) {
        int width, height;
        mume_point_t pt1, pt2;

        mume_scrollview_get_scroll(self, &pt1.x, &pt1.y);

        mume_scrollview_get_client(
            self, NULL, NULL, &width, &height);

        mume_scrollbar_set_page(self->horz_bar, width);
        mume_scrollbar_set_page(self->vert_bar, height);

        _scrollview_update_bars(self);

        mume_scrollview_get_scroll(self, &pt2.x, &pt2.y);

        /* Scroll pos changed after resize. */
        if (pt1.x != pt2.x || pt1.y != pt2.y) {
            mume_event_t event;

            event = mume_make_notify_event(
                self, self, MUME_SCROLLVIEW_SCROLL, &pt1);

            mume_send_event(&event);
        }
    }
}

static void _scrollview_handle_scroll(
    struct _scrollview *self, void *window, int hitcode, int pos)
{
    int x, page, line;

    if (window == self->horz_bar) {
        page = self->page_cx;
        line = self->line_cx;

        if (page < 0) {
            mume_scrollview_get_client(
                self, NULL, NULL, &page, 0);
        }
    }
    else {
        page = self->page_cy;
        line = self->line_cy;

        if (page < 0) {
            mume_scrollview_get_client(
                self, NULL, NULL, 0, &page);
        }
    }

    if (line < 0)
        line = 1;

    switch (hitcode) {
    case MUME_SBHT_BUTTON1:
        pos -= line;
        break;

    case MUME_SBHT_BUTTON2:
        pos += line;
        break;

    case MUME_SBHT_CHANNEL1:
        pos -= page;
        break;

    case MUME_SBHT_CHANNEL2:
        pos += page;
        break;
    }

    if (window == self->horz_bar) {
        mume_scrollview_get_scroll(self, NULL, &x);
        mume_scrollview_set_scroll(self, pos, x);
    }
    else {
        mume_scrollview_get_scroll(self, &x, NULL);
        mume_scrollview_set_scroll(self, x, pos);
    }
}

const void* mume_scrollview_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_scrollview_meta_class(),
        "scrollview",
        _scrollview_super_class(),
        sizeof(struct _scrollview),
        MUME_PROP_END,
        _mume_ctor, _scrollview_ctor,
        _mume_dtor, _scrollview_dtor,
        _mume_window_handle_button_down,
        _scrollview_handle_button_down,
        _mume_window_handle_resize,
        _scrollview_handle_resize,
        _mume_window_handle_scroll,
        _scrollview_handle_scroll,
        MUME_FUNC_END);
}

void* mume_scrollview_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_scrollview_class(),
                    parent, x, y, width, height);
}

void mume_scrollview_set_size(void *_self, int cx, int cy)
{
    struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    mume_scrollbar_set_size(self->horz_bar, cx);
    mume_scrollbar_set_size(self->vert_bar, cy);

    _scrollview_update_bars(self);
}

void mume_scrollview_get_size(const void *_self, int *cx, int *cy)
{
    const struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    if (cx)
        *cx = mume_scrollbar_get_size(self->horz_bar);

    if (cy)
        *cy = mume_scrollbar_get_size(self->vert_bar);
}

void mume_scrollview_set_page(void *_self, int cx, int cy)
{
    struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    self->page_cx = cx;
    self->page_cy = cy;
}

void mume_scrollview_get_page(const void *_self, int *cx, int *cy)
{
    const struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    if (cx)
        *cx = self->page_cx;

    if (cy)
        *cy = self->page_cy;
}

void mume_scrollview_set_line(void *_self, int cx, int cy)
{
    struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    self->line_cx = cx;
    self->line_cy = cy;
}

void mume_scrollview_get_line(const void *_self, int *cx, int *cy)
{
    const struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    if (cx)
        *cx = self->line_cx;

    if (cy)
        *cy = self->line_cy;
}

void mume_scrollview_set_scroll(void *_self, int x, int y)
{
    struct _scrollview *self = _self;
    mume_point_t pt;

    assert(mume_is_of(_self, mume_scrollview_class()));

    pt.x = mume_scrollbar_get_pos(self->horz_bar);
    pt.y = mume_scrollbar_get_pos(self->vert_bar);

    if (x != pt.x || y != pt.y) {
        mume_event_t event;

        mume_scrollbar_set_pos(self->horz_bar, x);
        mume_scrollbar_set_pos(self->vert_bar, y);

        event = mume_make_notify_event(
            self, self, MUME_SCROLLVIEW_SCROLL, &pt);

        mume_send_event(&event);

        mume_invalidate_region(self, NULL);
    }
}

void mume_scrollview_get_scroll(const void *_self, int *x, int *y)
{
    const struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    if (x)
        *x = mume_scrollbar_get_pos(self->horz_bar);

    if (y)
        *y = mume_scrollbar_get_pos(self->vert_bar);
}

void mume_scrollview_get_client(
    const void *_self, int *x, int *y, int *w, int *h)
{
    const struct _scrollview *self = _self;

    assert(mume_is_of(_self, mume_scrollview_class()));

    if (x)
        *x = 0;

    if (y)
        *y = 0;

    mume_window_get_geometry(self, NULL, NULL, w, h);

    if (w) {
        if (mume_window_is_mapped(self->vert_bar))
            *w -= mume_window_width(self->vert_bar);
    }

    if (h) {
        if (mume_window_is_mapped(self->horz_bar))
            *h -= mume_window_height(self->horz_bar);
    }
}
