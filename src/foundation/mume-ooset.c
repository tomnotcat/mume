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
#include "mume-ooset.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include MUME_ASSERT_H

#define _ooset_super_class mume_octnr_class

struct _ooset {
    const char _[MUME_SIZEOF_OCTNR];
    mume_oset_t oset;
};

MUME_STATIC_ASSERT(sizeof(struct _ooset) == MUME_SIZEOF_OOSET);

static void* _ooset_begin(struct _ooset *self)
{
    return mume_oset_first(&self->oset);
}

static void* _ooset_end(struct _ooset *self)
{
    return NULL;
}

static void* _ooset_next(struct _ooset *self, void *it)
{
    return mume_oset_next(it);
}

static void* _ooset_value(struct _ooset *self, void *it)
{
    return *(void**)mume_oset_data(it);
}

static void* _ooset_insert(
    struct _ooset *self, void *it, void *object)
{
    return mume_ooset_insert(self, object);
}

static void* _ooset_erase(struct _ooset *self, void *it)
{
    void *next = mume_oset_next(it);
    mume_oset_erase(&self->oset, it);
    return next;
}

static size_t _ooset_size(struct _ooset *self)
{
    return mume_oset_size(&self->oset);
}

static void _ooset_clear(struct _ooset *self)
{
    mume_oset_clear(&self->oset);
}

static void* _ooset_ctor(
    struct _ooset *self, int mode, va_list *app)
{
    if (mode != MUME_CTOR_NORMAL) {
        mume_oset_ctor(&self->oset, mume_object_compare,
                       mume_object_destruct, NULL);
    }

    if (!_mume_ctor(_ooset_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        void *del = va_arg(*app, void*);

        if (del) {
            mume_oset_ctor(&self->oset, mume_object_compare,
                           mume_object_destruct, del);
        }
        else {
            mume_oset_ctor(
                &self->oset, mume_object_compare, NULL, NULL);
        }
    }

    return self;
}

static void* _ooset_dtor(struct _ooset *self)
{
    mume_oset_dtor(&self->oset);
    return _mume_dtor(_ooset_super_class(), self);
}

const void* mume_ooset_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_ooset_meta_class(),
        "ooset",
        _ooset_super_class(),
        sizeof(struct _ooset),
        MUME_PROP_END,
        _mume_ctor, _ooset_ctor,
        _mume_dtor, _ooset_dtor,
        _mume_octnr_begin, _ooset_begin,
        _mume_octnr_end, _ooset_end,
        _mume_octnr_next, _ooset_next,
        _mume_octnr_value, _ooset_value,
        _mume_octnr_insert, _ooset_insert,
        _mume_octnr_erase, _ooset_erase,
        _mume_octnr_size, _ooset_size,
        _mume_octnr_clear, _ooset_clear,
        MUME_FUNC_END);
}

void* mume_ooset_new(void (*del)(void*))
{
    return mume_new(mume_ooset_class(), del);
}

void* mume_ooset_insert(void *_self, void *object)
{
    struct _ooset *self = _self;
    mume_oset_node_t *node;

    assert(mume_is_of(_self, mume_ooset_class()));

    node = mume_oset_newnode(sizeof(void*));
    *(void**)mume_oset_data(node) = object;

    if (mume_oset_insert(&self->oset, node))
        return node;

    mume_oset_delnode(node);
    return NULL;
}

void* mume_ooset_find(const void *_self, const void *key)
{
    const struct _ooset *self = _self;
    assert(mume_is_of(_self, mume_ooset_class()));
    return mume_oset_find(&self->oset, &key);
}
