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
#include "mume-tabctrl.h"
#include "mume-debug.h"
#include "mume-drawing.h"
#include "mume-events.h"
#include "mume-geometry.h"
#include "mume-gstate.h"
#include "mume-memory.h"
#include "mume-objbase.h"
#include "mume-resmgr-private.h"
#include "mume-text-layout.h"
#include "mume-types.h"
#include "mume-vector.h"
#include MUME_ASSERT_H

#define _tabctrl_is_closable(_self) \
        mume_test_flag((_self)->flags, _TABCTRL_FLAG_CLOSABLE)

#define _tabctrl_inside_cbtn(_self) \
        mume_test_flag((_self)->flags, _TABCTRL_FLAG_INCBTN)

#define _tabctrl_inside_tbtn(_self) \
        mume_test_flag((_self)->flags, _TABCTRL_FLAG_INTBTN)

#define _tabctrl_super_class mume_window_class

enum _tabctrl_flags_e {
    _TABCTRL_FLAG_TOP,
    _TABCTRL_FLAG_BOTTOM,
    _TABCTRL_FLAG_LEFT,
    _TABCTRL_FLAG_RIGHT,
    _TABCTRL_FLAG_INVALID,
    _TABCTRL_FLAG_PRESSED,
    _TABCTRL_FLAG_CLOSABLE,
    _TABCTRL_FLAG_INCBTN,
    _TABCTRL_FLAG_INTBTN,
    _TABCTRL_FLAG_TABCHANGED,
    _TABCTRL_FLAG_TOOLCHANGED
};

enum _tabitem_state_e {
    _TABITEM_STATE_NORMAL,
    _TABITEM_STATE_HOT,
    _TABITEM_STATE_PRESSED
};

struct _tabitem {
    void *window;
    cairo_surface_t *icon;
    char *text;
    void *data;
    mume_rect_t rect;
};

struct _tabctrl {
    const char _[MUME_SIZEOF_WINDOW];
    unsigned int flags;
    int selected;
    void *highlighted;
    mume_vector_t *tabs;
    mume_vector_t *tools;
};

struct _tabctrl_theme {
    int bar_height;
    mume_rect_t bar_margin;
    mume_rect_t cbtn_rect;
    mume_rect_t tbtn_rect;
    mume_rect_t text_margin;
    int tab_span;
    int tab_gap;
    int tbtn_gap;
    mume_widget_bkgnd_t tab_bg;
    mume_widget_bkgnd_t cbtn_bg;
    mume_widget_bkgnd_t tbtn_bg;
    mume_resobj_charfmt_t text;
    mume_resobj_brush_t barbg;
    mume_resobj_brush_t bkgnd;
    mume_resobj_cursor_t cbtn_cursor;
    mume_resobj_cursor_t tbtn_cursor;
};

MUME_STATIC_ASSERT(sizeof(struct _tabctrl) == MUME_SIZEOF_TABCTRL);

static void _tabitem_destruct(void *obj, void *p)
{
    struct _tabitem *item = obj;

    if (item->text)
        free(item->text);

    if (item->icon)
        cairo_surface_destroy(item->icon);
}

static struct _tabctrl_theme* _tabctrl_get_theme(
    const struct _tabctrl *self)
{
    struct _tabctrl_theme *theme = mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "tabctrl", "theme"),
        mume_typeof_tabctrl_theme());

    if (NULL == theme)
        mume_abort(("Get tabctrl theme failed\n"));

    return theme;
}

static struct _tabitem* _tabctrl_insert_item(
    mume_vector_t *items, int index)
{
    struct _tabitem *item;

    item = mume_vector_insert(items, index, 1);
    item->window = NULL;
    item->icon = NULL;
    item->text = NULL;
    item->data = NULL;
    item->rect = mume_rect_empty;

    return item;
}

static struct _tabitem* _tabctrl_item_at(
    mume_vector_t *items, int index)
{
    if (index >= 0 && index < mume_vector_size(items))
        return mume_vector_at(items, index);

    return NULL;
}

static int _tabctrl_item_state(
    const struct _tabctrl *self, void *item)
{
    if (self->highlighted == item) {
        if (mume_test_flag(self->flags, _TABCTRL_FLAG_PRESSED))
            return _TABITEM_STATE_PRESSED;

        return _TABITEM_STATE_HOT;
    }

    return _TABITEM_STATE_NORMAL;
}

static mume_matrix_t _tabctrl_get_matrix(const struct _tabctrl *self)
{
    mume_matrix_t m;

    if (mume_test_flag(self->flags, _TABCTRL_FLAG_TOP)) {
        m = mume_matrix_identity;
    }
    else {
        int w, h;

        mume_window_get_geometry(self, NULL, NULL, &w, &h);
        if (mume_test_flag(self->flags, _TABCTRL_FLAG_BOTTOM)) {
            m = mume_matrix_translate(-w, -h);
            m = mume_matrix_concat(m, mume_matrix_rotate(180));
        }
        else if (mume_test_flag(self->flags, _TABCTRL_FLAG_LEFT)) {
            m = mume_matrix_translate(-h, 0);
            m = mume_matrix_concat(m, mume_matrix_rotate(270));
        }
        else if (mume_test_flag(self->flags, _TABCTRL_FLAG_RIGHT)) {
            m = mume_matrix_translate(0, -w);
            m = mume_matrix_concat(m, mume_matrix_rotate(90));
        }
    }

    return m;
}

static mume_rect_t _tabctrl_get_bar_rect(const struct _tabctrl *self)
{
    struct _tabctrl_theme *theme;
    mume_rect_t rect;

    theme = _tabctrl_get_theme(self);
    rect.x = rect.y = 0;

    if (mume_test_flag(self->flags, _TABCTRL_FLAG_TOP) ||
        mume_test_flag(self->flags, _TABCTRL_FLAG_BOTTOM))
    {
        rect.width = mume_window_width(self);
    }
    else {
        rect.width = mume_window_height(self);
    }

    rect.height = theme->bar_height;

    return rect;
}

static mume_rect_t _tabctrl_get_panel_rect(const struct _tabctrl *self)
{
    struct _tabctrl_theme *theme;
    mume_rect_t rect;

    theme = _tabctrl_get_theme(self);
    rect.x = 0;
    rect.y = theme->bar_height;

    if (mume_test_flag(self->flags, _TABCTRL_FLAG_TOP) ||
        mume_test_flag(self->flags, _TABCTRL_FLAG_BOTTOM))
    {
        mume_window_get_geometry(
            self, NULL, NULL, &rect.width, &rect.height);
    }
    else {
        mume_window_get_geometry(
            self, NULL, NULL, &rect.height, &rect.width);
    }

    rect.height -= theme->bar_height;

    return rect;
}

static void _tabctrl_move_to_panel(struct _tabctrl *self, void *window)
{
    mume_matrix_t m;
    mume_rect_t r;

    if (mume_window_parent(window) != self)
        return;

    m = _tabctrl_get_matrix(self);
    r = _tabctrl_get_panel_rect(self);
    r = mume_rect_transform(r, m);

    mume_window_set_geometry(
        window, r.x, r.y, r.width, r.height);
}

static void _tabctrl_change_select(
    struct _tabctrl *self, struct _tabitem *o, struct _tabitem *n)
{
    if (o && o->window)
        mume_window_unmap(o->window);

    if (n && n->window) {
        _tabctrl_move_to_panel(self, n->window);
        mume_window_map(n->window);
    }
}

static int _tabctrl_update_rects(struct _tabctrl *self, int reverse)
{
    double item_x, item_y, item_gap;
    double item_width, item_height;
    int i, tab_count, tool_count;
    unsigned int changed = 0;
    mume_rect_t r, or;
    struct _tabctrl_theme *theme;
    struct _tabitem *item;

    theme = _tabctrl_get_theme(self);
    r = _tabctrl_get_bar_rect(self);

    /* Tabs. */
    if (reverse)
        item_x = r.width - theme->bar_margin.width;
    else
        item_x = theme->bar_margin.x;

    item_y = theme->bar_margin.y;

    tab_count = mume_tabctrl_tab_count(self);
    tool_count = mume_tabctrl_tool_count(self);

    if (theme->bar_margin.x + theme->bar_margin.width +
        theme->tab_span * tab_count +
        theme->tab_gap * (tab_count - 1) + theme->tbtn_rect.x +
        theme->tbtn_rect.width * tool_count +
        theme->tbtn_gap * (tool_count - 1) <= r.width)
    {
        item_width = theme->tab_span;
    }
    else {
        item_width = r.width - theme->bar_margin.x -
                     theme->bar_margin.width - theme->tbtn_rect.x -
                     theme->tab_gap * (tab_count - 1) -
                     theme->tbtn_rect.width * tool_count -
                     theme->tbtn_gap * (tool_count - 1);

        if (tab_count > 0)
            item_width /= tab_count;
    }

    if (item_width < theme->cbtn_rect.width +
        theme->cbtn_rect.x + theme->cbtn_rect.x)
    {
        item_width = theme->cbtn_rect.width +
                     theme->cbtn_rect.x + theme->cbtn_rect.x;
    }

#define _MIN_TAB_WIDTH 16
    if (item_width < _MIN_TAB_WIDTH)
        item_width = _MIN_TAB_WIDTH;
#undef _MIN_TAB_WIDTH

    item_height = theme->bar_height - theme->bar_margin.y -
                  theme->bar_margin.height;
    item_gap = theme->tab_gap;

    for (i = 0; i < tab_count; ++i) {
        item = _tabctrl_item_at(self->tabs, i);
        or = item->rect;

        item->rect.width = item_width;
        item->rect.height = item_height;

        if (reverse) {
            item->rect.x = item_x - item->rect.width;
            item_x = item->rect.x - item_gap;
        }
        else {
            item->rect.x = item_x;
            item_x += item_width + item_gap;
        }

        item->rect.y = item_y;

        if (!mume_test_flag(changed, _TABCTRL_FLAG_TABCHANGED)) {
            if (or.x != item->rect.x ||
                or.y != item->rect.y ||
                or.width != item->rect.width ||
                or.height != item->rect.height)
            {
                mume_add_flag(changed, _TABCTRL_FLAG_TABCHANGED);
            }
        }
    }

    /* Tools */
    if (reverse) {
        item_x += item_gap;
        item_x -= theme->tbtn_rect.x;
    }
    else {
        item_x -= item_gap;
        item_x += theme->tbtn_rect.x;
    }

    item_y += theme->tbtn_rect.y;
    item_width = theme->tbtn_rect.width;
    item_height = theme->tbtn_rect.height;
    item_gap = theme->tbtn_gap;

    for (i = 0; i < tool_count; ++i) {
        item = _tabctrl_item_at(self->tools, i);
        or = item->rect;

        item->rect.width = item_width;
        item->rect.height = item_height;

        if (reverse) {
            item->rect.x = item_x - item->rect.width;
            item_x = item->rect.x - item_gap;
        }
        else {
            item->rect.x = item_x;
            item_x += item_width + item_gap;
        }

        item->rect.y = item_y;

        if (!mume_test_flag(changed, _TABCTRL_FLAG_TOOLCHANGED)) {
            if (or.x != item->rect.x ||
                or.y != item->rect.y ||
                or.width != item->rect.width ||
                or.height != item->rect.height)
            {
                mume_add_flag(changed, _TABCTRL_FLAG_TOOLCHANGED);
            }
        }
    }

    return changed;
}

static void _tabctrl_invalidate_bar(struct _tabctrl *self)
{
    mume_matrix_t m;
    mume_rect_t r = _tabctrl_get_bar_rect(self);

    m = _tabctrl_get_matrix(self);
    r = mume_rect_transform(r, m);

    mume_invalidate_rect(self, &r);
}

static void _tabctrl_invalidate_item(
    struct _tabctrl *self, struct _tabitem *item)
{
    if (item) {
        mume_matrix_t m;
        mume_rect_t r = item->rect;

        m = _tabctrl_get_matrix(self);
        r = mume_rect_transform(r, m);

        mume_invalidate_rect(self, &r);
    }
}

static int _tabctrl_update_inside_cbtn(struct _tabctrl *self, int b)
{
    int result = 0;

    if (!_tabctrl_is_closable(self))
        b = 0;

    if (_tabctrl_inside_cbtn(self)) {
        if (!b) {
            mume_remove_flag(self->flags, _TABCTRL_FLAG_INCBTN);
            result = 1;
        }
    }
    else {
        if (b) {
            mume_add_flag(self->flags, _TABCTRL_FLAG_INCBTN);
            result = 1;
        }
    }

    if (result) {
        struct _tabctrl_theme *theme = _tabctrl_get_theme(self);

        if (theme->cbtn_cursor.p) {
            if (b)
                mume_window_set_cursor(self, theme->cbtn_cursor.p);
            else
                mume_window_set_cursor(self, NULL);
        }
    }

    return result;
}

static int _tabctrl_update_inside_tbtn(struct _tabctrl *self, int b)
{
    int result = 0;

    if (_tabctrl_inside_tbtn(self)) {
        if (!b) {
            mume_remove_flag(self->flags, _TABCTRL_FLAG_INTBTN);
            result = 1;
        }
    }
    else {
        if (b) {
            mume_add_flag(self->flags, _TABCTRL_FLAG_INTBTN);
            result = 1;
        }
    }

    if (result) {
        struct _tabctrl_theme *theme = _tabctrl_get_theme(self);

        if (theme->tbtn_cursor.p) {
            if (b)
                mume_window_set_cursor(self, theme->tbtn_cursor.p);
            else
                mume_window_set_cursor(self, NULL);
        }
    }

    return result;
}

static void _tabctrl_invalidate_structure(struct _tabctrl *self)
{
    if (!mume_test_flag(self->flags, _TABCTRL_FLAG_INVALID)) {
        _tabctrl_invalidate_bar(self);
        mume_add_flag(self->flags, _TABCTRL_FLAG_INVALID);
    }
}

static void _tabctrl_update_structure(struct _tabctrl *self)
{
    if (mume_test_flag(self->flags, _TABCTRL_FLAG_INVALID)) {
        int r = (mume_test_flag(self->flags, _TABCTRL_FLAG_LEFT) ||
                 mume_test_flag(self->flags, _TABCTRL_FLAG_BOTTOM));

        if ((r = _tabctrl_update_rects(self, r))) {
            if (mume_test_flag(r, _TABCTRL_FLAG_TABCHANGED))
                _tabctrl_update_inside_cbtn(self, 0);

            if (mume_test_flag(r, _TABCTRL_FLAG_TOOLCHANGED))
            _tabctrl_update_inside_tbtn(self, 0);
        }

        mume_remove_flag(self->flags, _TABCTRL_FLAG_INVALID);
    }
}

static mume_rect_t _tabctrl_get_cbtn_rect(
    struct _tabctrl *self, mume_rect_t rect)
{
    struct _tabctrl_theme *theme;

    theme = _tabctrl_get_theme(self);

    rect.x += rect.width;
    rect.x -= theme->cbtn_rect.x + theme->cbtn_rect.width;
    rect.y += theme->cbtn_rect.y;
    rect.width = theme->cbtn_rect.width;
    rect.height = theme->cbtn_rect.height;

    return rect;
}

static int _tabctrl_item_from_point(
    struct _tabctrl *self, mume_vector_t *items,
    int x, int y, int *inclose)
{
    const struct _tabitem *item;
    mume_matrix_t m;
    mume_point_t p;
    int i, count;

    _tabctrl_update_structure(self);

    p.x = x;
    p.y = y;
    m = _tabctrl_get_matrix(self);
    m = mume_matrix_invert(m);
    p = mume_point_transform(p, m);

    count = mume_vector_size(items);
    for (i = 0; i < count; ++i) {
        item = _tabctrl_item_at(items, i);

        if (mume_rect_inside(item->rect, p.x, p.y)) {
            if (inclose) {
                mume_rect_t cbtn;
                cbtn = _tabctrl_get_cbtn_rect(self, item->rect);
                *inclose = mume_rect_inside(cbtn, p.x, p.y);
            }

            return i;
        }
    }

    return -1;
}

static int _tabctrl_item_from_window(
    const struct _tabctrl *self, mume_vector_t *items, void *window)
{
    const struct _tabitem *item;
    int i, count;

    count = mume_vector_size(items);
    for (i = 0; i < count; ++i) {
        item = _tabctrl_item_at(items, i);
        if (item->window == window)
            return i;
    }

    return -1;
}

static void _tabctrl_draw_tab(
    struct _tabctrl *self, cairo_t *cr, void *item, int state,
    struct _tabctrl_theme *theme, int x, int y, int w, int h)
{
    mume_resobj_brush_t *br;
    const char *text;
    mume_rect_t cbtn;
    int is_selected;

    is_selected = (_tabctrl_item_at(
        self->tabs, self->selected) == item);

    /* Background. */
    if (!is_selected) {
        switch (state) {
        case _TABITEM_STATE_PRESSED:
            br = &theme->tab_bg.pressed;
            break;

        case _TABITEM_STATE_HOT:
            br = &theme->tab_bg.hot;
            break;

        default:
            br = &theme->tab_bg.normal;
            break;
        }
    }
    else {
        br = &theme->tab_bg.pressed;
    }

    mume_draw_resobj_brush(cr, br, x, y, w, h);

    cbtn = _tabctrl_get_cbtn_rect(
        self, mume_rect_make(x, y, w, h));

    /* Texts. */
    text = mume_tabctrl_get_text(self, item);
    if (text) {
        mume_rect_t rect = mume_rect_make(x, y, w, h);
        unsigned int format =
                MUME_TLF_DRAWTEXT | MUME_TLF_SINGLELINE |
                MUME_TLF_LEFT | MUME_TLF_VCENTER;

        if (is_selected && _tabctrl_is_closable(self))
            rect.width = cbtn.x - x;

        rect = mume_rect_inflate(
            rect, -theme->text_margin.x,
            -theme->text_margin.y,
            -theme->text_margin.width,
            -theme->text_margin.height);

        mume_charfmt_draw_text(
            cr, &theme->text, format, text, -1, &rect);
    }

    /* Close button. */
    if (is_selected && _tabctrl_is_closable(self)) {
        br = &theme->cbtn_bg.normal;
        if (_tabctrl_inside_cbtn(self)) {
            switch (state) {
            case _TABITEM_STATE_PRESSED:
                br = &theme->cbtn_bg.pressed;
                break;

            case _TABITEM_STATE_HOT:
                br = &theme->cbtn_bg.hot;
                break;
            }
        }

        mume_draw_resobj_brush(
            cr, br, cbtn.x, cbtn.y, cbtn.width, cbtn.height);
    }
}

static void _tabctrl_paint_tool(
    struct _tabctrl *self, cairo_t *cr,
    struct _tabitem *item, int state,
    struct _tabctrl_theme *theme, int x, int y, int w, int h)
{
    mume_resobj_brush_t *br;

    /* Background. */
    switch (state) {
    case _TABITEM_STATE_PRESSED:
        br = &theme->tbtn_bg.pressed;
        break;

    case _TABITEM_STATE_HOT:
        br = &theme->tbtn_bg.hot;
        break;

    default:
        br = &theme->tbtn_bg.normal;
        break;
    }

    mume_draw_resobj_brush(cr, br, x, y, w, h);

    if (item->icon) {
        int ix, iy, iw, ih;
        iw = cairo_image_surface_get_width(item->icon);
        ih = cairo_image_surface_get_width(item->icon);
        ix = x + (w - iw) / 2;
        iy = y + (h - ih) / 2;
        cairo_set_source_surface(cr, item->icon, ix, iy);
        cairo_rectangle(cr, ix, iy, iw, ih);
        cairo_fill(cr);
    }
}

static void* _tabctrl_ctor(
    struct _tabctrl *self, int mode, va_list *app)
{
    int type;

    if (!_mume_ctor(_tabctrl_super_class(), self, mode, app))
        return NULL;

    type = va_arg(*app, int);
    self->flags = 0;

    switch (type) {
    case MUME_TABCTRL_BOTTOM:
        mume_add_flag(self->flags, _TABCTRL_FLAG_BOTTOM);
        break;

    case MUME_TABCTRL_LEFT:
        mume_add_flag(self->flags, _TABCTRL_FLAG_LEFT);
        break;

    case MUME_TABCTRL_RIGHT:
        mume_add_flag(self->flags, _TABCTRL_FLAG_RIGHT);
        break;

    default:
        mume_add_flag(self->flags, _TABCTRL_FLAG_TOP);
    }

    self->selected = -1;
    self->highlighted = NULL;
    self->tabs = mume_vector_new(
        sizeof(struct _tabitem), _tabitem_destruct, NULL);
    self->tools = mume_vector_new(
        sizeof(struct _tabitem), _tabitem_destruct, NULL);

    return self;
}

static void* _tabctrl_dtor(struct _tabctrl *self)
{
    mume_vector_delete(self->tools);
    mume_vector_delete(self->tabs);
    self->tabs = NULL;
    return _mume_dtor(_tabctrl_super_class(), self);
}

static void _tabctrl_handle_button_down(
    struct _tabctrl *self, int x, int y, int state, int button)
{
    int i, c;

    if (button != MUME_BUTTON_LEFT)
        return;

    mume_add_flag(self->flags, _TABCTRL_FLAG_PRESSED);

    i = _tabctrl_item_from_point(self, self->tabs, x, y, &c);
    if (i != -1) {
        self->highlighted = mume_tabctrl_tab_at(self, i);
        _tabctrl_update_inside_tbtn(self, 0);

        if (i != self->selected) {
            mume_tabctrl_select_tab(self, i);
        }
        else if (_tabctrl_is_closable(self) &&
                 _tabctrl_update_inside_cbtn(self, c))
        {
            _tabctrl_invalidate_item(self, self->highlighted);
        }
    }
    else {
        i = mume_tabctrl_tool_from_point(self, x, y);
        self->highlighted = mume_tabctrl_tool_at(self, i);
        _tabctrl_update_inside_tbtn(self, i != -1);
        _tabctrl_invalidate_item(self, self->highlighted);
    }
}

static void _tabctrl_handle_button_up(
    struct _tabctrl *self, int x, int y, int state, int button)
{
    int i, c;
    void *item;

    mume_remove_flag(self->flags, _TABCTRL_FLAG_PRESSED);

    _tabctrl_invalidate_item(self, self->highlighted);

    i = _tabctrl_item_from_point(self, self->tabs, x, y, &c);
    item = _tabctrl_item_at(self->tabs, i);
    if (item) {
        if (_tabctrl_is_closable(self)) {
            if (c && _tabctrl_inside_cbtn(self) &&
                self->highlighted == item)
            {
                void *parent = mume_window_parent(self);
                if (parent) {
                    mume_post_event(
                        mume_make_command_event(parent, self, i));
                }
            }

            _tabctrl_update_inside_cbtn(self, c);
        }

        _tabctrl_update_inside_tbtn(self, 0);
    }
    else {
        i = mume_tabctrl_tool_from_point(self, x, y);
        item = _tabctrl_item_at(self->tools, i);
        if (item && item == self->highlighted) {
            void *parent = mume_window_parent(self);
            if (parent) {
                i += mume_tabctrl_tab_count(self);
                mume_post_event(
                    mume_make_command_event(parent, self, i));
            }
        }

        _tabctrl_update_inside_cbtn(self, 0);
        _tabctrl_update_inside_tbtn(self, item != NULL);
    }

    if (item != self->highlighted) {
        _tabctrl_invalidate_item(self, self->highlighted);
        self->highlighted = item;
        _tabctrl_invalidate_item(self, self->highlighted);
    }
}

static void _tabctrl_handle_mouse_motion(
    struct _tabctrl *self, int x, int y, int state)
{
    int i, c;
    void *item;

    if (mume_test_flag(self->flags, _TABCTRL_FLAG_PRESSED))
        return;

    i = _tabctrl_item_from_point(self, self->tabs, x, y, &c);
    item = mume_tabctrl_tab_at(self, i);

    if (item) {
        if (c)
            c = (i == self->selected);

        _tabctrl_update_inside_tbtn(self, 0);
    }
    else {
        item = mume_tabctrl_tool_at(
            self, mume_tabctrl_tool_from_point(self, x, y));

        c = 0;
        _tabctrl_update_inside_tbtn(self, item != NULL);
    }

    if (_tabctrl_update_inside_cbtn(self, c))
        _tabctrl_invalidate_item(self, self->highlighted);

    if (item != self->highlighted) {
        _tabctrl_invalidate_item(self, self->highlighted);
        self->highlighted = item;
        _tabctrl_invalidate_item(self, self->highlighted);
    }
}

static void _tabctrl_handle_mouse_leave(
    struct _tabctrl *self, int x, int y, int state, int mode, int detail)
{
    _tabctrl_invalidate_item(self, self->highlighted);
    self->highlighted = NULL;
    _tabctrl_update_inside_cbtn(self, 0);
    _tabctrl_update_inside_tbtn(self, 0);
}

static void _tabctrl_handle_expose(
    struct _tabctrl *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    struct _tabctrl_theme *theme;
    struct _tabitem *item;
    int i, num_items, state;
    mume_matrix_t m;
    cairo_matrix_t cm;
    mume_rect_t r;

    if (count)
        return;

    theme = _tabctrl_get_theme(self);

    _tabctrl_update_structure(self);

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    m = _tabctrl_get_matrix(self);
    cm = mume_matrix_to_cairo(m);
    cairo_transform(cr, &cm);

    /* Background. */
    r = _tabctrl_get_bar_rect(self);
    if (r.width > 0 && r.height > 0) {
        mume_draw_resobj_brush(
            cr, &theme->barbg, r.x, r.y, r.width, r.height);
    }

    r = _tabctrl_get_panel_rect(self);
    if (r.width > 0 && r.height > 0) {
        mume_draw_resobj_brush(
            cr, &theme->bkgnd, r.x, r.y, r.width, r.height);
    }

    /* Only paint items intersected with invalid rect. */
    r = mume_current_invalid_rect();
    m = mume_matrix_invert(m);
    r = mume_rect_transform(r, m);

    /* Tab items. */
    num_items = mume_tabctrl_tab_count(self);
    for (i = 0; i < num_items; ++i) {
        item = _tabctrl_item_at(self->tabs, i);
        state = _tabctrl_item_state(self, item);

        if (!mume_rect_is_empty(
                mume_rect_intersect(item->rect, r)))
        {
            _tabctrl_draw_tab(
                self, cr, item, state, theme,
                item->rect.x, item->rect.y,
                item->rect.width, item->rect.height);
        }
    }

    /* Tool items. */
    num_items = mume_tabctrl_tool_count(self);
    for (i = 0; i < num_items; ++i) {
        item = _tabctrl_item_at(self->tools, i);
        state = _tabctrl_item_state(self, item);

        if (!mume_rect_is_empty(
                mume_rect_intersect(item->rect, r)))
        {
            _tabctrl_paint_tool(
                self, cr, item, state, theme,
                item->rect.x, item->rect.y,
                item->rect.width, item->rect.height);
        }
    }

    mume_window_end_paint(self, cr);
}

static void _tabctrl_handle_create(
    struct _tabctrl *self, void *window, int x, int y, int w, int h)
{
    if (mume_window_parent(window) == self)
        mume_tabctrl_append_tab(self, window);
}

static void _tabctrl_handle_destroy(struct _tabctrl *self, void *window)
{
    if (self->tabs) {
        int index = mume_tabctrl_tab_from_window(self, window);
        if (index != -1)
            mume_tabctrl_remove_tab(self, index);
    }
}

static void _tabctrl_handle_resize(
    struct _tabctrl *self, void *window, int w, int h, int ow, int oh)
{
    if (self == window) {
        struct _tabitem *item;

        item = _tabctrl_item_at(
            self->tabs, mume_tabctrl_get_selected(self));

        if (item && item->window)
            _tabctrl_move_to_panel(self, item->window);

        _tabctrl_invalidate_structure(self);
    }
}

const void* mume_tabctrl_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_tabctrl_meta_class(),
        "tabctrl",
        _tabctrl_super_class(),
        sizeof(struct _tabctrl),
        MUME_PROP_END,
        _mume_ctor, _tabctrl_ctor,
        _mume_dtor, _tabctrl_dtor,
        _mume_window_handle_button_down,
        _tabctrl_handle_button_down,
        _mume_window_handle_button_up,
        _tabctrl_handle_button_up,
        _mume_window_handle_mouse_motion,
        _tabctrl_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _tabctrl_handle_mouse_leave,
        _mume_window_handle_expose,
        _tabctrl_handle_expose,
        _mume_window_handle_create,
        _tabctrl_handle_create,
        _mume_window_handle_destroy,
        _tabctrl_handle_destroy,
        _mume_window_handle_resize,
        _tabctrl_handle_resize,
        MUME_FUNC_END);
}

void* mume_tabctrl_new(
    void *parent, int x, int y, int w, int h, int type)
{
    return mume_new(mume_tabctrl_class(),
                    parent, x, y, w, h, type);
}

void mume_tabctrl_enable_close(void *_self, int able)
{
    struct _tabctrl *self = _self;

    assert(mume_is_of(_self, mume_tabctrl_class()));

    if (able)
        mume_add_flag(self->flags, _TABCTRL_FLAG_CLOSABLE);
    else
        mume_remove_flag(self->flags, _TABCTRL_FLAG_CLOSABLE);

    _tabctrl_invalidate_structure(self);
}

void mume_tabctrl_insert_tab(void *_self, int index, void *window)
{
    struct _tabctrl *self = _self;
    struct _tabitem *item;

    assert(index >= 0 && index <= mume_tabctrl_tab_count(self));

    item = _tabctrl_insert_item(self->tabs, index);
    item->window = window;

    _tabctrl_invalidate_structure(self);
}

void mume_tabctrl_remove_tab(void *_self, int index)
{
    struct _tabctrl *self = _self;
    struct _tabitem *item;

    assert(index >= 0 && index < mume_tabctrl_tab_count(self));

    if (_tabctrl_item_at(self->tabs, index) == self->highlighted)
        self->highlighted = NULL;

    item = _tabctrl_item_at(self->tabs, self->selected);
    _tabctrl_change_select(self, item, NULL);

    mume_vector_erase(self->tabs, index, 1);

    if (self->selected >= mume_tabctrl_tab_count(self))
        self->selected = mume_tabctrl_tab_count(self) - 1;

    item = _tabctrl_item_at(self->tabs, self->selected);
    _tabctrl_change_select(self, NULL, item);

    _tabctrl_invalidate_structure(self);
}

int mume_tabctrl_tab_count(const void *_self)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return mume_vector_size(self->tabs);
}

void* mume_tabctrl_tab_at(const void *_self, int index)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return _tabctrl_item_at(self->tabs, index);
}

int mume_tabctrl_tab_from_point(void *_self, int x, int y)
{
    struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return _tabctrl_item_from_point(self, self->tabs, x, y, NULL);
}

int mume_tabctrl_tab_from_window(const void *_self, void *window)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return _tabctrl_item_from_window(self, self->tabs, window);
}

void mume_tabctrl_select_tab(void *_self, int index)
{
    struct _tabctrl *self = _self;

    assert(index >= 0 && index < mume_tabctrl_tab_count(self));

    if (index != self->selected) {
        struct _tabitem *o, *n;

        o = _tabctrl_item_at(self->tabs, self->selected);

        self->selected = index;
        n = _tabctrl_item_at(self->tabs, self->selected);

        _tabctrl_change_select(self, o, n);
        _tabctrl_invalidate_item(self, o);
        _tabctrl_invalidate_item(self, n);
    }
}

int mume_tabctrl_get_selected(const void *_self)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return self->selected;
}

void mume_tabctrl_insert_tool(
    void *_self, int index, cairo_surface_t *icon)
{
    struct _tabctrl *self = _self;
    struct _tabitem *item;

    assert(index >= 0 && index <= mume_tabctrl_tool_count(self));

    item = _tabctrl_insert_item(self->tools, index);
    item->icon = icon;

    if (icon)
        cairo_surface_reference(icon);

    _tabctrl_invalidate_structure(self);
}

void mume_tabctrl_remove_tool(void *_self, int index)
{
    struct _tabctrl *self = _self;

    assert(index >= 0 && index < mume_tabctrl_tool_count(self));

    if (_tabctrl_item_at(self->tools, index) == self->highlighted)
        self->highlighted = NULL;

    mume_vector_erase(self->tools, index, 1);

    _tabctrl_invalidate_structure(self);
}

int mume_tabctrl_tool_count(const void *_self)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return mume_vector_size(self->tools);
}

void* mume_tabctrl_tool_at(const void *_self, int index)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return _tabctrl_item_at(self->tools, index);
}

int mume_tabctrl_tool_from_point(void *_self, int x, int y)
{
    struct _tabctrl *self = _self;
    assert(mume_is_of(_self, mume_tabctrl_class()));
    return _tabctrl_item_from_point(self, self->tools, x, y, NULL);
}

int mume_tabctrl_tool_from_window(const void *_self, void *window)
{
    const struct _tabctrl *self = _self;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return _tabctrl_item_from_window(self, self->tools, window);
}

void mume_tabctrl_set_window(void *_self, void *_item, void *window)
{
    struct _tabctrl *self = _self;
    struct _tabitem *item = _item;

    assert(mume_is_of(_self, mume_tabctrl_class()));

    item->window = window;
    if (_tabctrl_item_at(self->tabs, self->selected) == item) {
        _tabctrl_move_to_panel(self, item->window);
        mume_window_map(item->window);
    }

    _tabctrl_invalidate_structure(self);
}

void* mume_tabctrl_get_window(const void *self, const void *_item)
{
    const struct _tabitem *item = _item;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return item->window;
}

void mume_tabctrl_set_text(
    void *self, void *_item, const char *text)
{
    struct _tabitem *item = _item;

    assert(mume_is_of(self, mume_tabctrl_class()));

    free(item->text);

    if (text)
        item->text = strdup_abort(text);
    else
        item->text = NULL;

    _tabctrl_invalidate_structure(self);
}

const char* mume_tabctrl_get_text(
    const void *self, const void *_item)
{
    const struct _tabitem *item = _item;

    assert(mume_is_of(self, mume_tabctrl_class()));

    if (item->text)
        return item->text;

    if (item->window)
        return mume_window_get_text(item->window);

    return NULL;
}

void mume_tabctrl_set_icon(
    void *self, void *_item, cairo_surface_t *icon)
{
    struct _tabitem *item = _item;

    assert(mume_is_of(self, mume_tabctrl_class()));

    cairo_surface_destroy(item->icon);
    item->icon = icon;
    if (item->icon)
        cairo_surface_reference(item->icon);

    _tabctrl_invalidate_structure(self);
}

cairo_surface_t* mume_tabctrl_get_icon(
    const void *self, const void *_item)
{
    const struct _tabitem *item = _item;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return item->icon;
}

void mume_tabctrl_set_data(void *self, void *_item, void *data)
{
    struct _tabitem *item = _item;

    assert(mume_is_of(self, mume_tabctrl_class()));

    item->data = data;
    _tabctrl_invalidate_structure(self);
}

void* mume_tabctrl_get_data(const void *self, const void *_item)
{
    const struct _tabitem *item = _item;
    assert(mume_is_of(self, mume_tabctrl_class()));
    return item->data;
}

mume_type_t* mume_typeof_tabctrl_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _tabctrl_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), bar_height);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), bar_margin);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), cbtn_rect);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), tbtn_rect);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), text_margin);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), tab_span);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), tab_gap);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), tbtn_gap);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), tab_bg);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), cbtn_bg);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), tbtn_bg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_charfmt(), text);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), barbg);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_brush(), bkgnd);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_cursor(), cbtn_cursor);
        MUME_DIRECT_PROPERTY(_mume_typeof_resobj_cursor(), tbtn_cursor);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
