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
#include "mume-ovector.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _ovector_super_class mume_octnr_class

struct _ovector {
    const char _[MUME_SIZEOF_OCTNR];
    mume_vector_t vector;
};

MUME_STATIC_ASSERT(sizeof(struct _ovector) == MUME_SIZEOF_OVECTOR);

static void* _ovector_begin(struct _ovector *self)
{
    return (void*)0;
}

static void* _ovector_end(struct _ovector *self)
{
    return (void*)mume_vector_size(&self->vector);
}

static void* _ovector_next(struct _ovector *self, void *it)
{
    return (void*)(((intptr_t)it) + 1);
}

static void* _ovector_value(struct _ovector *self, void *it)
{
    return *(void**)mume_vector_at(&self->vector, (intptr_t)it);
}

static void* _ovector_insert(
    struct _ovector *self, void *it, void *object)
{
    *(void**)mume_vector_insert(
        &self->vector, (intptr_t)it, 1) = object;

    return it;
}

static void* _ovector_erase(struct _ovector *self, void *it)
{
    mume_vector_erase(&self->vector, (intptr_t)it, 1);
    return it;
}

static size_t _ovector_size(struct _ovector *self)
{
    return mume_vector_size(&self->vector);
}

static void _ovector_clear(struct _ovector *self)
{
    mume_vector_clear(&self->vector);
}

static void* _ovector_ctor(
    struct _ovector *self, int mode, va_list *app)
{
    if (mode != MUME_CTOR_NORMAL) {
        mume_vector_ctor(&self->vector, sizeof(void*),
                         mume_object_destruct, NULL);
    }

    if (!_mume_ctor(_ovector_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        void *del = va_arg(*app, void*);

        if (del) {
            mume_vector_ctor(&self->vector, sizeof(void*),
                             mume_object_destruct, del);
        }
        else {
            mume_vector_ctor(
                &self->vector, sizeof(void*), NULL, NULL);
        }
    }

    return self;
}

static void* _ovector_dtor(struct _ovector *self)
{
    mume_vector_dtor(&self->vector);
    return _mume_dtor(_ovector_super_class(), self);
}

const void* mume_ovector_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_ovector_meta_class(),
        "ovector",
        _ovector_super_class(),
        sizeof(struct _ovector),
        MUME_PROP_END,
        _mume_ctor, _ovector_ctor,
        _mume_dtor, _ovector_dtor,
        _mume_octnr_begin, _ovector_begin,
        _mume_octnr_end, _ovector_end,
        _mume_octnr_next, _ovector_next,
        _mume_octnr_value, _ovector_value,
        _mume_octnr_insert, _ovector_insert,
        _mume_octnr_erase, _ovector_erase,
        _mume_octnr_size, _ovector_size,
        _mume_octnr_clear, _ovector_clear,
        MUME_FUNC_END);
}

void* mume_ovector_new(void (*del)(void*))
{
    return mume_new(mume_ovector_class(), del);
}

void mume_ovector_insert(void *self, int index, void *object)
{
    assert(mume_is_of(self, mume_ovector_class()));
    _ovector_insert(self, (void*)index, object);
}

void mume_ovector_push_back(void *self, void *object)
{
    assert(mume_is_of(self, mume_ovector_class()));
    _ovector_insert(self, _ovector_end(self), object);
}

void mume_ovector_erase(void *_self, int index, int count)
{
    struct _ovector *self = _self;
    assert(mume_is_of(_self, mume_ovector_class()));
    mume_vector_erase(&self->vector, index, count);
}

void* mume_ovector_at(const void *self, int index)
{
    assert(mume_is_of(self, mume_ovector_class()));
    return _ovector_value((void*)self, (void*)index);
}
