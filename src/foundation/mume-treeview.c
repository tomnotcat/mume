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
#include "mume-treeview.h"
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
#include MUME_ASSERT_H

#define _treeview_super_class mume_scrollview_class

#define _treenode_is_expanded(_self) \
    mume_test_flag((_self)->flags, _TREENODE_FLAG_EXPAND)

enum _treenode_flags_e {
    _TREENODE_FLAG_STATIC_NAME,
    _TREENODE_FLAG_EXPAND,
    _TREENODE_FLAG_MEASURED
};

enum _treeview_flags_e {
    _TREEVIEW_FLAG_INVALID
};

struct _treeview_theme {
    int item_indent;
    mume_rect_t item_margin;
    mume_point_t expcol_size;
    mume_resobj_brush_t exp_normal;
    mume_resobj_brush_t exp_selected;
    mume_resobj_brush_t col_normal;
    mume_resobj_brush_t col_selected;
    mume_resobj_brush_t bkgnd;
    mume_widget_bkgnd_t itembg;
    mume_widget_texts_t texts;
};

struct _treenode {
    char *text;
    void *data;
    mume_rect_t rect;
    unsigned int flags;
    struct _treenode *parent;
    struct _treenode *next;
    struct _treenode *child;
};

struct _treeview {
    const char _[MUME_SIZEOF_SCROLLVIEW];
    unsigned int flags;
    struct _treenode *root;
    struct _treenode *first_visible;
    struct _treenode *selected;
    struct _treenode *highlighted;
};

MUME_STATIC_ASSERT(sizeof(struct _treeview) == MUME_SIZEOF_TREEVIEW);

static struct _treeview_theme* _treeview_get_theme(
    const struct _treeview *self)
{
    return mume_objdesc_cast(
        mume_resmgr_get_object(mume_resmgr(), "treeview", "theme"),
        mume_typeof_treeview_theme());
}

static void _treeview_measure_node(
    struct _treeview *self, struct _treenode *node,
    struct _treeview_theme *theme, int *width, int *height)
{
    mume_point_t point = { 0, 0 };

    if (node->text) {
        point = mume_charfmt_text_extents(
            &theme->texts.normal, node->text, -1);
    }

    if (width) {
        *width = point.x + theme->item_margin.x +
                 theme->item_margin.width;
    }

    if (height) {
        *height = point.y + theme->item_margin.y +
                  theme->item_margin.height;
    }
}

static void _treeview_draw_item(
    struct _treeview *self, struct _treenode *node, cairo_t *cr,
    struct _treeview_theme *theme, int x, int y, int w, int h)
{
    mume_resobj_charfmt_t *cf;
    mume_rect_t rect = mume_rect_make(x, y, w, h);
    unsigned int flags = MUME_TLF_DRAWTEXT |
                         MUME_TLF_SINGLELINE |
                         MUME_TLF_LEFT |
                         MUME_TLF_VCENTER |
                         MUME_TLF_NOCLIP;

    rect = mume_rect_deflate(
        rect, theme->item_margin.x, theme->item_margin.y,
        theme->item_margin.width, theme->item_margin.height);

    if (self->selected == node) {
        cf = &theme->texts.pressed;
    }
    else if (self->highlighted == node) {
        cf = &theme->texts.hot;
    }
    else {
        cf = &theme->texts.normal;
    }

    mume_charfmt_draw_text(cr, cf, flags, node->text, -1, &rect);
}

static struct _treenode* _treenode_prev(const struct _treenode *node)
{
    struct _treenode *prev = NULL;
    struct _treenode *it;

    if (NULL == node->parent)
        return NULL;

    it = node->parent->child;

    /* Find the previous sibling node. */
    while (it != node) {
        prev = it;
        it = it->next;
    }

    return prev;
}

static struct _treenode* _treenode_last_leaf(struct _treenode *node)
{
    if (node->child) {
        if (_treenode_is_expanded(node)) {
            struct _treenode *it = node->child;

            while (it->next)
                it = it->next;

            return _treenode_last_leaf(it);
        }
    }

    return node;
}

static struct _treenode* _treenode_traverse_next(
    struct _treenode *node, struct _treenode *stop)
{
    if (node->child && _treenode_is_expanded(node)) {
        return node->child;
    }
    else if (node->next) {
        return node->next;
    }
    else {
        while ((node = node->parent) != stop) {
            if (node->next)
                return node->next;
        }
    }

    return NULL;
}

static mume_rect_t _treenode_expcol_rect(
    struct _treenode *node, struct _treeview_theme *theme)
{
    mume_rect_t rect = node->rect;

    rect.x -= theme->expcol_size.x;
    rect.y += (rect.height - theme->expcol_size.y) / 2;
    rect.width = theme->expcol_size.x;
    rect.height = theme->expcol_size.y;

    return rect;
}

static void _treenode_expand_collapse(
    struct _treenode *node, int expand, int recur)
{
    if (expand)
        mume_add_flag(node->flags, _TREENODE_FLAG_EXPAND);
    else
        mume_remove_flag(node->flags, _TREENODE_FLAG_EXPAND);

    if (recur) {
        struct _treenode *it = node->child;
        while (it) {
            if (expand)
                mume_add_flag(it->flags, _TREENODE_FLAG_EXPAND);
            else
                mume_remove_flag(it->flags, _TREENODE_FLAG_EXPAND);

            it = _treenode_traverse_next(it, node);
        }
    }
}

static void _treenode_destroy(struct _treenode *node)
{
    if (node->child) {
        struct _treenode *next;
        struct _treenode *it = node->child;

        while (it) {
            next = it->next;
            _treenode_destroy(it);
            it = next;
        }
    }

    if (!mume_test_flag(node->flags, _TREENODE_FLAG_STATIC_NAME))
        free(node->text);

    free(node);
}

static void _treeview_update_rects(struct _treeview *self)
{
    int x = 0;
    int y = 0;
    int max_width = 0;

    struct _treenode *it = self->root->child;
    struct _treeview_theme *theme = _treeview_get_theme(self);

    x = theme->item_indent;
    while (it) {
        it->rect.x = x;
        it->rect.y = y;

        if (!mume_test_flag(it->flags, _TREENODE_FLAG_MEASURED)) {
            _treeview_measure_node(
                self, it, theme, &it->rect.width, &it->rect.height);

            mume_add_flag(it->flags, _TREENODE_FLAG_MEASURED);
        }

        max_width = MAX(x + it->rect.width, max_width);
        y += it->rect.height;

        if (it->child && _treenode_is_expanded(it)) {
            it = it->child;
            x += theme->item_indent;
        }
        else if (it->next) {
            it = it->next;
        }
        else {
            while ((it = it->parent)) {
                x -= theme->item_indent;

                if (it->next) {
                    it = it->next;
                    break;
                }
            }
        }
    }

    mume_scrollview_set_size(self, max_width, y);
}

static void _treeview_invalidate_item(
    struct _treeview *self, struct _treenode *node)
{
    if (node) {
        int sy;
        mume_rect_t r;

        mume_scrollview_get_scroll(self, NULL, &sy);

        r.x = 0;
        r.y = node->rect.y - sy;
        r.width = mume_window_width(self);
        r.height = node->rect.height;

        mume_invalidate_rect(self, &r);
    }
}

static void _treeview_invalidate_structure(struct _treeview *self)
{
    if (!mume_test_flag(self->flags, _TREEVIEW_FLAG_INVALID)) {
        self->first_visible = NULL;
        mume_invalidate_region(self, NULL);
        mume_add_flag(self->flags, _TREEVIEW_FLAG_INVALID);
    }
}

static void _treeview_update_structure(struct _treeview *self)
{
    if (mume_test_flag(self->flags, _TREEVIEW_FLAG_INVALID)) {
        _treeview_update_rects(self);
        mume_remove_flag(self->flags, _TREEVIEW_FLAG_INVALID);
    }
}

static void _treeview_set_node_text(
    void *_self, void *_node, const char *text, int static_text)
{
    struct _treeview *self = _self;
    struct _treenode *node = _node;

    assert(mume_is_of(_self, mume_treeview_class()));

    if (!mume_test_flag(node->flags, _TREENODE_FLAG_STATIC_NAME))
        free(node->text);

    if (static_text) {
        node->text = (char*)text;
        mume_add_flag(node->flags, _TREENODE_FLAG_STATIC_NAME);
    }
    else {
        node->text = strdup_abort(text);
        mume_remove_flag(node->flags, _TREENODE_FLAG_STATIC_NAME);
    }

    mume_remove_flag(node->flags, _TREENODE_FLAG_MEASURED);

    _treeview_invalidate_structure(self);
}

static void _treeview_button_clicked(
    struct _treeview *self, int x, int y, int dblclk)
{
    struct _treenode *node;
    mume_rect_t rect;
    int sx, sy;

    node = mume_treeview_node_from(self, y);
    if (!node)
        return;

    mume_treeview_set_selected(self, node);

    /* Expand/Collapse if needed. */
    if (!node->child)
        return;

    if (dblclk) {
        rect.x = 0;
        rect.y = node->rect.y;
        rect.width = mume_window_width(self);
        rect.height = node->rect.height;
    }
    else {
        struct _treeview_theme *theme;
        theme = _treeview_get_theme(self);
        rect = _treenode_expcol_rect(node, theme);
    }

    mume_scrollview_get_scroll(self, &sx, &sy);
    if (mume_rect_inside(rect, sx + x, sy + y)) {
        if (_treenode_is_expanded(node)) {
            mume_treeview_collapse(self, node, 0);
        }
        else {
            int h;
            struct _treenode *next;

            mume_treeview_expand(self, node, 0);
            _treeview_update_structure(self);

            /* Make the expanded node visible.  */
            if (node->next) {
                next = node->next;

                if (next->child && _treenode_is_expanded(next))
                    next = next->child;
                else if (next->next)
                    next = next->next;
            }
            else {
                next = _treenode_last_leaf(node);
            }

            mume_scrollview_get_client(self, NULL, NULL, NULL, &h);
            if (next->rect.y + next->rect.height - node->rect.y < h) {
                mume_treeview_ensure_visible(self, next);
            }
            else {
                mume_scrollview_set_scroll(self, sx, node->rect.y);
            }
        }
    }
}

static struct _treenode* _treeview_prev_selectable(
    const struct _treeview *self, const struct _treenode *node)
{
    struct _treenode *prev = _treenode_prev(node);
    if (prev)
        return _treenode_last_leaf(prev);

    if (node->parent != mume_treeview_root(self))
        return node->parent;

    return NULL;
}

static struct _treenode* _treeview_next_selectable(
    const struct _treeview *self, const struct _treenode *node)
{
    if (node->child && _treenode_is_expanded(node))
        return node->child;

    while (node && !node->next)
        node = node->parent;

    if (node)
        return node->next;

    return NULL;
}

static void* _treeview_ctor(
    struct _treeview *self, int mode, va_list *app)
{
    if (!_mume_ctor(_treeview_super_class(), self, mode, app))
        return NULL;

    mume_scrollview_set_line(self, 64, 64);

    self->flags = 0;
    self->root = malloc_abort(sizeof(*(self->root)));
    memset(self->root, 0, sizeof(*(self->root)));
    self->first_visible = NULL;
    self->selected = NULL;
    self->highlighted = NULL;

    return self;
}

static void* _treeview_dtor(struct _treeview *self)
{
    _treenode_destroy(self->root);
    return _mume_dtor(_treeview_super_class(), self);
}

static void _treeview_handle_key_down(
    struct _treeview *self, int x, int y, int state, int keysym)
{
    struct _treenode *node = mume_treeview_get_selected(self);

    switch (keysym) {
    case MUME_KEY_LEFT:
        if (node)
            mume_treeview_collapse(self, node, 0);
        break;

    case MUME_KEY_RIGHT:
        if (node)
            mume_treeview_expand(self, node, 0);
        break;

    case MUME_KEY_UP:
        node = _treeview_prev_selectable(self, node);
        if (node)
            mume_treeview_set_selected(self, node);
        break;

    case MUME_KEY_DOWN:
        node = _treeview_next_selectable(self, node);
        if (node)
            mume_treeview_set_selected(self, node);
        break;
    }

    _mume_window_handle_key_down(
        _treeview_super_class(), self, x, y, state, keysym);
}

static void _treeview_handle_button_down(
    struct _treeview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        _treeview_button_clicked(self, x, y, 0);
        break;
    }

    _mume_window_handle_button_down(
        _treeview_super_class(), self, x, y, state, button);
}

static void _treeview_handle_button_up(
    struct _treeview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_RIGHT:
        {
            mume_point_t pt;
            mume_event_t event;

            pt.x = x;
            pt.y = y;

            event = mume_make_notify_event(
                self, self, MUME_TREEVIEW_CONTEXTMENU, &pt);

            mume_send_event(&event);
        }
        break;
    }
}

static void _treeview_handle_button_dblclk(
    struct _treeview *self, int x, int y, int state, int button)
{
    switch (button) {
    case MUME_BUTTON_LEFT:
        _treeview_button_clicked(self, x, y, 1);
        break;
    }

    _mume_window_handle_button_dblclk(
        _treeview_super_class(), self, x, y, state, button);
}

static void _treeview_handle_mouse_motion(
    struct _treeview *self, int x, int y, int state)
{
    if (!state) {
        struct _treenode *node;

        node = mume_treeview_node_from(self, y);

        if (self->highlighted != node) {
            _treeview_invalidate_item(self, self->highlighted);
            self->highlighted = node;
            _treeview_invalidate_item(self, self->highlighted);
        }
    }
}

static void _treeview_handle_mouse_leave(
    struct _treeview *self, int x, int y, int state, int mode, int detail)
{
    _treeview_invalidate_item(self, self->highlighted);
    self->highlighted = NULL;
}

static void _treeview_handle_expose(
    struct _treeview *self, int x, int y, int w, int h, int count)
{
    cairo_t *cr;
    int sx, sy;
    mume_rect_t r, ru;
    mume_resobj_brush_t *br;
    struct _treenode *it;
    struct _treeview_theme *theme;

    if (count)
        return;

    theme = _treeview_get_theme(self);
    if (NULL == theme) {
        mume_warning(("Get treeview theme failed\n"));
        return;
    }

    cr = mume_window_begin_paint(self, MUME_PM_INVALID);
    if (NULL == cr) {
        mume_warning(("Begin paint failed\n"));
        return;
    }

    _treeview_update_structure(self);

    ru = mume_current_invalid_rect();

    /* Draw background. */
    mume_scrollview_get_client(self, &x, &y, &w, &h);
    mume_draw_resobj_brush(cr, &theme->bkgnd, x, y, w, h);

    /* Draw items. */
    mume_scrollview_get_scroll(self, &sx, &sy);
    it = mume_treeview_first_visible(self);
    while (it && (it->rect.y < sy + h)) {
        r.x = 0;
        r.y = it->rect.y - sy;
        r.width = w;
        r.height = it->rect.height;

        if (mume_rect_is_empty(
                mume_rect_intersect(ru, r)))
        {
            it = _treenode_traverse_next(it, NULL);
            continue;
        }

        /* Item background. */
        if (it == self->selected) {
            br = &theme->itembg.pressed;
        }
        else if (it == self->highlighted) {
            br = &theme->itembg.hot;
        }
        else {
            br = &theme->itembg.normal;
        }

        mume_draw_resobj_brush(
            cr, br, r.x, r.y, r.width, r.height);

        /* Item content. */
        _treeview_draw_item(
            self, it, cr, theme,
            it->rect.x - sx, it->rect.y - sy,
            it->rect.width, it->rect.height);

        /* Expand/Collapse mark. */
        if (it->child) {
            r = _treenode_expcol_rect(it, theme);
            r.x -= sx;
            r.y -= sy;

            if (_treenode_is_expanded(it)) {
                if (it == self->selected)
                    br = &theme->col_selected;
                else
                    br = &theme->col_normal;
            }
            else {
                if (it == self->selected)
                    br = &theme->exp_selected;
                else
                    br = &theme->exp_normal;
            }

            mume_draw_resobj_brush(
                cr, br, r.x, r.y, r.width, r.height);
        }

        it = _treenode_traverse_next(it, NULL);
    }

    mume_window_end_paint(self, cr);
}

static void _treeview_handle_notify(
    struct _treeview *self, void *window, int code, void *data)
{
    if (self == window && MUME_SCROLLVIEW_SCROLL == code) {
        const mume_point_t *pt = data;
        int sy;

        mume_scrollview_get_scroll(self, NULL, &sy);
        if (pt->y != sy) {
            /* Recalculate the first visible node. */
            self->first_visible = NULL;
        }

        return;
    }

    _mume_window_handle_notify(
        _treeview_super_class(), self, window, code, data);
}

const void* mume_treeview_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_treeview_meta_class(),
        "treeview",
        _treeview_super_class(),
        sizeof(struct _treeview),
        MUME_PROP_END,
        _mume_ctor, _treeview_ctor,
        _mume_dtor, _treeview_dtor,
        _mume_window_handle_key_down,
        _treeview_handle_key_down,
        _mume_window_handle_button_down,
        _treeview_handle_button_down,
        _mume_window_handle_button_up,
        _treeview_handle_button_up,
        _mume_window_handle_button_dblclk,
        _treeview_handle_button_dblclk,
        _mume_window_handle_mouse_motion,
        _treeview_handle_mouse_motion,
        _mume_window_handle_mouse_leave,
        _treeview_handle_mouse_leave,
        _mume_window_handle_expose,
        _treeview_handle_expose,
        _mume_window_handle_notify,
        _treeview_handle_notify,
        MUME_FUNC_END);
}

void* mume_treeview_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_treeview_class(),
                    parent, x, y, width, height);
}

void* mume_treeview_root(const void *_self)
{
    const struct _treeview *self = _self;
    assert(mume_is_of(_self, mume_treeview_class()));
    return self->root;
}

void mume_treeview_set_selected(void *_self, void *node)
{
    struct _treeview *self = _self;

    assert(mume_is_of(_self, mume_treeview_class()));

    if (self->selected != node) {
        _treeview_invalidate_item(self, self->selected);
        self->selected = node;
        _treeview_invalidate_item(self, self->selected);

        mume_treeview_ensure_visible(self, node);
    }
}

void* mume_treeview_get_selected(const void *_self)
{
    const struct _treeview *self = _self;
    assert(mume_is_of(_self, mume_treeview_class()));
    return self->selected;
}

void* mume_treeview_insert(
    void *_self, void *_parent, void *_prev,
    const char *text, unsigned int flags)
{
    struct _treeview *self = _self;
    struct _treenode *parent = _parent;
    struct _treenode *prev = _prev;
    struct _treenode *node, *temp;

    assert(mume_is_of(_self, mume_treeview_class()));
    assert(NULL == prev || prev->parent == parent);

    node = malloc_abort(sizeof(struct _treenode));
    node->data = NULL;
    node->flags = 0;

    if (flags & MUME_TREENODE_STATIC_NAME) {
        node->text = (char*)text;
        mume_add_flag(node->flags, _TREENODE_FLAG_STATIC_NAME);
    }
    else {
        node->text = strdup_abort(text);
    }

    if (prev) {
        temp = prev->next;
        prev->next = node;
    }
    else {
        temp = parent->child;
        parent->child = node;
    }

    node->parent = parent;
    node->next = temp;
    node->child = NULL;

    _treeview_invalidate_structure(self);

    return node;
}

void mume_treeview_remove(void *_self, void *_node)
{
    struct _treeview *self = _self;
    struct _treenode *node = _node;
    struct _treenode *prev = _treenode_prev(node);

    assert(mume_is_of(_self, mume_treeview_class()));
    assert(_node != self->root);

    if (prev) {
        prev->next = node->next;
    }
    else {
        node->parent->child = node->next;
    }

    if (self->selected == node)
        self->selected = NULL;

    if (self->highlighted == node)
        self->highlighted = NULL;

    _treenode_destroy(node);
    _treeview_invalidate_structure(self);
}

void mume_treeview_remove_children(void *_self, void *_parent)
{
    struct _treeview *self = _self;
    struct _treenode *parent = _parent;
    struct _treenode *it = parent->child;
    struct _treenode *next;

    while (it) {
        next = it->next;
        _treenode_destroy(it);
        it = next;
    }

    parent->child = NULL;
    _treeview_invalidate_structure(self);
}

void mume_treeview_set_text(void *_self, void *_node, const char *text)
{
    _treeview_set_node_text(_self, _node, text, 0);
}

void mume_treeview_set_static_text(
    void *_self, void *_node, const char *text)
{
    _treeview_set_node_text(_self, _node, text, 1);
}

const char* mume_treeview_get_text(const void *_self, const void *_node)
{
    const struct _treenode *node = _node;
    return node->text;
}

void mume_treeview_set_data(void *_self, void *_node, void *data)
{
    struct _treeview *self = _self;
    struct _treenode *node = _node;

    assert(mume_is_of(_self, mume_treeview_class()));

    node->data = data;

    _treeview_invalidate_structure(self);
}

void* mume_treeview_get_data(const void *_self, const void *_node)
{
    const struct _treenode *node = _node;
    return node->data;
}

void mume_treeview_expand(void *self, void *node, int recur)
{
    _treenode_expand_collapse(node, -1, recur);
    _treeview_invalidate_structure(self);
}

void mume_treeview_collapse(void *self, void *node, int recur)
{
    _treenode_expand_collapse(node, 0, recur);
    _treeview_invalidate_structure(self);
}

void* mume_treeview_first_visible(void *_self)
{
    struct _treeview *self = _self;

    assert(mume_is_of(_self, mume_treeview_class()));

    if (NULL == self->first_visible)
        self->first_visible = mume_treeview_node_from(self, 0);

    return self->first_visible;
}

void* mume_treeview_node_from(const void *_self, int y)
{
    const struct _treeview *self = _self;
    struct _treenode *n;
    int sy;

    assert(mume_is_of(_self, mume_treeview_class()));

    _treeview_update_structure((struct _treeview*)self);
    mume_scrollview_get_scroll(self, NULL, &sy);
    y += sy;
    n = self->first_visible ? self->first_visible : self->root->child;
    while (n) {
        if (y >= n->rect.y && (y < n->rect.y + n->rect.height))
            return n;

        n = _treenode_traverse_next(n, NULL);
    }

    return NULL;
}

void mume_treeview_ensure_visible(void *_self, const void *_node)
{
    struct _treeview *self = _self;
    const struct _treenode *node = _node;
    int cx, cy, sx, sy;

    assert(mume_is_of(_self, mume_treeview_class()));

    _treeview_update_structure(self);

    mume_scrollview_get_client(self, NULL, NULL, &cx, &cy);
    mume_scrollview_get_scroll(self, &sx, &sy);
    if (node->rect.x + node->rect.width <= sx) {
        sx = node->rect.x;
    }
    else if (node->rect.x >= sx + cx) {
        sx = node->rect.x + node->rect.width - cx;
    }

    if (node->rect.y <= sy) {
        sy = node->rect.y;
    }
    else if (node->rect.y + node->rect.height >= sy + cy) {
        sy = node->rect.y + node->rect.height - cy;
    }

    mume_scrollview_set_scroll(self, sx, sy);
}

mume_type_t* mume_typeof_treeview_theme(void)
{
    static void *tp;

    if (!tp) {
        MUME_COMPOUND_CREATE(
            tp, struct _treeview_theme, NULL, NULL, NULL, NULL);
        MUME_DIRECT_PROPERTY(mume_typeof_int(), item_indent);
        MUME_DIRECT_PROPERTY(_mume_typeof_rect(), item_margin);
        MUME_DIRECT_PROPERTY(_mume_typeof_point(), expcol_size);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), exp_normal);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), exp_selected);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), col_normal);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), col_selected);
        MUME_DIRECT_PROPERTY(
            _mume_typeof_resobj_brush(), bkgnd);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_bkgnd(), itembg);
        MUME_DIRECT_PROPERTY(_mume_typeof_widget_texts(), texts);
        MUME_COMPOUND_FINISH();
    }

    return tp;
}
