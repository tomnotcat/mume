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
#include "mume-olist.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _olist_super_class mume_octnr_class

struct _olist {
    const char _[MUME_SIZEOF_OCTNR];
    mume_list_t list;
};

MUME_STATIC_ASSERT(sizeof(struct _olist) == MUME_SIZEOF_OLIST);

static void* _olist_begin(struct _olist *self)
{
    return mume_list_front(&self->list);
}

static void* _olist_end(struct _olist *self)
{
    return NULL;
}

static void* _olist_next(struct _olist *self, void *it)
{
    return mume_list_next((mume_list_node_t*)it);
}

static void* _olist_value(struct _olist *self, void *it)
{
    return *(void**)mume_list_data(it);
}

static void* _olist_insert(
    struct _olist *self, void *it, void *object)
{
    mume_list_node_t *node;
    node = mume_list_insert(&self->list, it, sizeof(void*));
    *(void**)mume_list_data(node) = object;
    return node;
}

static void* _olist_erase(struct _olist *self, void *it)
{
    void *next = mume_list_next((mume_list_node_t*)it);
    mume_list_erase(&self->list, it);
    return next;
}

static size_t _olist_size(struct _olist *self)
{
    return mume_list_size(&self->list);
}

static void _olist_clear(struct _olist *self)
{
    mume_list_clear(&self->list);
}

static void* _olist_ctor(
    struct _olist *self, int mode, va_list *app)
{
    if (mode != MUME_CTOR_NORMAL)
        mume_list_ctor(&self->list, mume_object_destruct, NULL);

    if (!_mume_ctor(_olist_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        void *del = va_arg(*app, void*);

        if (del) {
            mume_list_ctor(
                &self->list, mume_object_destruct, del);
        }
        else {
            mume_list_ctor(&self->list, NULL, NULL);
        }
    }

    return self;
}

static void* _olist_dtor(struct _olist *self)
{
    mume_list_dtor(&self->list);
    return _mume_dtor(_olist_super_class(), self);
}

const void* mume_olist_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_olist_meta_class(),
        "olist",
        _olist_super_class(),
        sizeof(struct _olist),
        MUME_PROP_END,
        _mume_ctor, _olist_ctor,
        _mume_dtor, _olist_dtor,
        _mume_octnr_begin, _olist_begin,
        _mume_octnr_end, _olist_end,
        _mume_octnr_next, _olist_next,
        _mume_octnr_value, _olist_value,
        _mume_octnr_insert, _olist_insert,
        _mume_octnr_erase, _olist_erase,
        _mume_octnr_size, _olist_size,
        _mume_octnr_clear, _olist_clear,
        MUME_FUNC_END);
}

void* mume_olist_new(void (*del)(void*))
{
    return mume_new(mume_olist_class(), del);
}

void* mume_olist_front(const void *_self)
{
    const struct _olist *self = _self;
    mume_list_node_t *node;

    assert(mume_is_of(_self, mume_olist_class()));

    node = mume_list_front(&self->list);
    return node ? *(void**)mume_list_data(node) : NULL;
}

void* mume_olist_back(const void *_self)
{
    const struct _olist *self = _self;
    mume_list_node_t *node;

    assert(mume_is_of(_self, mume_olist_class()));

    node = mume_list_back(&self->list);
    return node ? *(void**)mume_list_data(node) : NULL;
}

void mume_olist_push_front(void *self, void *object)
{
    mume_octnr_insert(self, mume_octnr_begin(self), object);
}

void mume_olist_push_back(void *self, void *object)
{
    mume_octnr_insert(self, mume_octnr_end(self), object);
}

void mume_olist_remove(void *_self, void *object)
{
    struct _olist *self = _self;
    mume_list_node_t *nd, *tn;
    void **obj;

    assert(mume_is_of(_self, mume_olist_class()));

    mume_list_foreach_safe(&self->list, nd, obj, tn) {
        if ((*obj) == object)
            mume_list_erase(&self->list, nd);
    }
}
