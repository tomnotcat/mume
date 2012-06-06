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
#include "mume-urgnmgr.h"
#include "mume-debug.h"
#include "mume-list.h"
#include "mume-memory.h"
#include "mume-window.h"
#include MUME_ASSERT_H

#define _urgnmgr_super_class mume_object_class
#define _urgnmgr_super_meta_class mume_meta_class

struct _urgnitem {
    const void *win;
    cairo_region_t *rgn;
};

struct _urgnmgr {
    const char _[MUME_SIZEOF_OBJECT];
    mume_list_t *urgn_list;
    struct _urgnitem last_urgn;
    void *reserved;
};

struct _urgnmgr_class {
    const char _[MUME_SIZEOF_CLASS];
    void *reserved;
};

MUME_STATIC_ASSERT(sizeof(struct _urgnmgr) == MUME_SIZEOF_URGNMGR);
MUME_STATIC_ASSERT(sizeof(struct _urgnmgr_class) ==
                   MUME_SIZEOF_URGNMGR_CLASS);

static void _urgnitem_destruct(void *obj, void *p)
{
    struct _urgnitem *urgn = obj;
    cairo_region_destroy(urgn->rgn);
}

static void* _urgnmgr_ctor(
    struct _urgnmgr *self, int mode, va_list *app)
{
    if (!_mume_ctor(_urgnmgr_super_class(), self, mode, app))
        return NULL;

    self->urgn_list = mume_list_new(_urgnitem_destruct, NULL);
    self->last_urgn.win = NULL;
    self->last_urgn.rgn = NULL;
    return self;
}

static void* _urgnmgr_dtor(void *_self)
{
    struct _urgnmgr *self = _self;
    cairo_region_destroy(self->last_urgn.rgn);
    mume_list_delete(self->urgn_list);
    return _mume_dtor(_urgnmgr_super_class(), _self);
}

static void* _urgnmgr_class_ctor(
    struct _urgnmgr_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_urgnmgr_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        (void)method;
        /*
        if (selector == (voidf)_mume_urgnmgr_)
            *(voidf*)&self->proc = method;
            */
    }

    return self;
}

static mume_list_node_t* _urgnmgr_find_urgn(
    const struct _urgnmgr *self, const void *win)
{
    mume_list_node_t *node;
    node = mume_list_front(self->urgn_list);
    while (node) {
        if (((const struct _urgnitem*)(
                mume_list_data(node)))->win == win)
        {
            return node;
        }
        node = mume_list_next(node);
    }
    return NULL;
}

static void _urgnmgr_set_urgn(
    void *_self, const void *win, const void *skip,
    const cairo_region_t *rgn, int operation)
{
    struct _urgnmgr *self = _self;
    struct _urgnitem *urgn;
    mume_list_node_t *node;
    if (win == skip)
        return;

    node = _urgnmgr_find_urgn(self, win);
    if (NULL == rgn || cairo_region_is_empty(rgn)) {
        if (node && MUME_URGN_REPLACE == operation) {
            mume_list_erase(self->urgn_list, node);
        }
        return;
    }

    if (NULL == node) {
        if (MUME_URGN_SUBTRACT == operation)
            return;

        node = mume_list_push_back(
            self->urgn_list, sizeof(struct _urgnitem));
        urgn = (struct _urgnitem*)mume_list_data(node);
        urgn->win = win;
        urgn->rgn = NULL;
    }
    else {
        urgn = (struct _urgnitem*)mume_list_data(node);
    }

    if ((MUME_URGN_REPLACE == operation) && urgn->rgn) {
        cairo_region_destroy(urgn->rgn);
        urgn->rgn = NULL;
    }

    if (NULL == urgn->rgn) {
        urgn->rgn = cairo_region_copy(rgn);
    }
    else {
        if (MUME_URGN_UNION == operation) {
            cairo_region_union(urgn->rgn, rgn);
        }
        else {
            cairo_region_subtract(urgn->rgn, rgn);
            if (cairo_region_is_empty(urgn->rgn))
                mume_list_erase(self->urgn_list, node);
        }
    }
}

static void _urgnmgr_set_urgn_recursive(
    void *_self, const void *win, const void *skip,
    cairo_region_t *rgn, int operation)
{
    void **cs;
    unsigned int cc;
    cs = mume_query_children(win, &cc, 0);
    if (cs) {
        mume_rect_t rect;
        cairo_region_t *crgn;
        while (cc-- > 0) {
            if (!mume_window_is_mapped(cs[cc]) || cs[cc] == skip)
                continue;

            if (NULL == rgn) {
                _urgnmgr_set_urgn_recursive(
                    _self, cs[cc], skip, NULL, operation);
                continue;
            }

            mume_window_get_geometry(
                cs[cc], &rect.x, &rect.y, &rect.width, &rect.height);
            crgn = cairo_region_create_rectangle(&rect);
            cairo_region_intersect(crgn, rgn);
            cairo_region_translate(crgn, -rect.x, -rect.y);
            _urgnmgr_set_urgn_recursive(
                _self, cs[cc], skip, crgn, operation);
            cairo_region_destroy(crgn);
            cairo_region_subtract_rectangle(rgn, &rect);
        }
        mume_free_children(win, cs);
    }
    /* FIXME: parent should paint before children,
       need to adjust dirty list */
    _urgnmgr_set_urgn(_self, win, skip, rgn, operation);
}

const void* mume_urgnmgr_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_urgnmgr_meta_class(),
        "urgnmgr",
        _urgnmgr_super_class(),
        sizeof(struct _urgnmgr),
        MUME_PROP_END,
        _mume_ctor, _urgnmgr_ctor,
        _mume_dtor, _urgnmgr_dtor,
        MUME_FUNC_END);
}

const void* mume_urgnmgr_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "urgnmgr class",
        _urgnmgr_super_meta_class(),
        sizeof(struct _urgnmgr_class),
        MUME_PROP_END,
        _mume_ctor, _urgnmgr_class_ctor,
        MUME_FUNC_END);
}

const cairo_region_t* mume_urgnmgr_get_urgn(
    const void *self, const void *win)
{
    mume_list_node_t *node = _urgnmgr_find_urgn(self, win);
    if (node) {
        return ((const struct _urgnitem*)(
            mume_list_data(node)))->rgn;
    }
    return NULL;
}

void mume_urgnmgr_set_urgn(
    void *self, const void *win, const void *skip,
    const cairo_region_t *rgn, int operation, int recursive)
{
    assert(mume_is_ancestors_mapped(win));
    assert(mume_window_is_mapped(win) || (NULL == rgn));
    assert(rgn || MUME_URGN_REPLACE == operation);
    if (recursive) {
        cairo_region_t *xrgn = cairo_region_copy(rgn);
        _urgnmgr_set_urgn_recursive(
            self, win, skip, xrgn, operation);
        cairo_region_destroy(xrgn);
    }
    else {
        _urgnmgr_set_urgn(
            self, win, skip, rgn, operation);
    }
}

int mume_urgnmgr_pop_urgn(void *_self)
{
    struct _urgnmgr *self = _self;
    cairo_region_destroy(self->last_urgn.rgn);
    self->last_urgn.win = NULL;
    self->last_urgn.rgn = NULL;
    if (!mume_list_empty(self->urgn_list)) {
        struct _urgnitem *item;
        item = (struct _urgnitem*)mume_list_data(
            mume_list_front(self->urgn_list));
        self->last_urgn = *item;
        item->win = NULL;
        item->rgn = NULL;
        mume_list_pop_front(self->urgn_list);
        return 1;
    }
    return 0;
}

const void* mume_urgnmgr_last_win(const void *_self)
{
    const struct _urgnmgr *self = _self;
    return self->last_urgn.win;
}

const cairo_region_t* mume_urgnmgr_last_rgn(const void *_self)
{
    const struct _urgnmgr *self = _self;
    return self->last_urgn.rgn;
}
