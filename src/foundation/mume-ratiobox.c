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
#include "mume-ratiobox.h"
#include "mume-debug.h"
#include "mume-list.h"
#include "mume-memory.h"
#include MUME_ASSERT_H

#define _ratiobox_super_class mume_window_class

struct _ratiobox {
    const char _[MUME_SIZEOF_WINDOW];
    mume_list_t *items;
};

struct _layout_item {
    void *window;
    int boxw, boxh;
    int x, y, w, h;
    float rx, ry, rw, rh;
};

MUME_STATIC_ASSERT(sizeof(struct _ratiobox) == MUME_SIZEOF_RATIOBOX);

static void _ratiobox_layout_items(
    struct _ratiobox *self, int x, int y, int width, int height)
{
    struct _layout_item *item;
    mume_list_node_t *nd;
    int dx, dy;

    mume_list_foreach(self->items, nd, item) {
        dx = width - item->boxw;
        dy = height - item->boxh;

        mume_window_set_geometry(
            item->window,
            x + item->x + dx * item->rx,
            y + item->y + dy * item->ry,
            item->w + dx * item->rw,
            item->h + dy * item->rh);
    }
}

static void* _ratiobox_ctor(
    struct _ratiobox *self, int mode, va_list *app)
{
    self->items = NULL;

    if (!_mume_ctor(_ratiobox_super_class(), self, mode, app))
        return NULL;

    return self;
}

static void* _ratiobox_dtor(struct _ratiobox *self)
{
    mume_list_delete(self->items);
    return _mume_dtor(_ratiobox_super_class(), self);
}

static void _ratiobox_handle_resize(
    struct _ratiobox *self, void *window, int w, int h, int ow, int oh)
{
    if (self == window)
        _ratiobox_layout_items(self, 0, 0, w, h);
}

const void* mume_ratiobox_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_ratiobox_meta_class(),
        "ratiobox",
        _ratiobox_super_class(),
        sizeof(struct _ratiobox),
        MUME_PROP_END,
        _mume_ctor, _ratiobox_ctor,
        _mume_dtor, _ratiobox_dtor,
        _mume_window_handle_resize,
        _ratiobox_handle_resize,
        MUME_FUNC_END);
}

void* mume_ratiobox_new(
    void *parent, int x, int y, int width, int height)
{
    return mume_new(mume_ratiobox_class(),
                    parent, x, y, width, height);
}

void mume_ratiobox_setup(
    void *_self, void *window, float rx, float ry, float rw, float rh)
{
    struct _ratiobox *self = _self;
    struct _layout_item *item;
    mume_list_node_t *nd = NULL;

    assert(mume_is_of(_self, mume_ratiobox_class()));
    assert(mume_is_of(window, mume_window_class()));
    assert(mume_window_parent(window) == self);

    if (self->items) {
        mume_list_foreach(self->items, nd, item) {
            if (item->window == window)
                break;
        }
    }
    else {
        self->items = mume_list_new(NULL, NULL);
    }

    if (NULL == nd) {
        nd = mume_list_push_back(self->items, sizeof(*item));
        item = (struct _layout_item*)mume_list_data(nd);
        item->window = window;
    }

    mume_window_get_geometry(
        self, NULL, NULL, &item->boxw, &item->boxh);

    mume_window_get_geometry(
        window, &item->x, &item->y, &item->w, &item->h);

    item->rx = rx;
    item->ry = ry;
    item->rw = rw;
    item->rh = rh;
}

void mume_ratiobox_remove(void *_self, void *window)
{
    struct _ratiobox *self = _self;

    assert(mume_is_of(_self, mume_ratiobox_class()));

    if (self->items) {
        struct _layout_item *item;
        mume_list_node_t *nd;

        mume_list_foreach(self->items, nd, item) {
            if (item->window == window) {
                mume_list_erase(self->items, nd);
                break;
            }
        }
    }
}
