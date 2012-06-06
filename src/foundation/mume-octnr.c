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
#include "mume-octnr.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _octnr_super_class mume_object_class
#define _octnr_super_meta_class mume_meta_class

struct _octnr {
    const char _[MUME_SIZEOF_OBJECT];
};

struct _octnr_class {
    const char _[MUME_SIZEOF_CLASS];
    void* (*begin)(void *self);
    void* (*end)(void *self);
    void* (*next)(void *self, void *it);
    void* (*prev)(void *self, void *it);
    void* (*value)(void *self, void *it);
    void* (*insert)(void *self, void *it, void *object);
    void* (*find)(void *self, const void *object);
    void* (*erase)(void *self, void *it);
    size_t (*size)(void *self);
    void (*clear)(void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _octnr) == MUME_SIZEOF_OCTNR);
MUME_STATIC_ASSERT(sizeof(struct _octnr_class) ==
                   MUME_SIZEOF_OCTNR_CLASS);

static void* _octnr_copy(
    struct _octnr *dest, const struct _octnr *src, int mode)
{
    void *it, *end, *obj;

    assert(mume_is_of(dest, mume_octnr_class()));
    assert(mume_is_of(src, mume_octnr_class()));

    mume_octnr_clear(dest);
    end = mume_octnr_end(src);

    for (it = mume_octnr_begin(src); it != end;
         it = mume_octnr_next(src, it))
    {
        obj = mume_octnr_value(src, it);
        if (obj) {
            mume_octnr_insert(
                dest, mume_octnr_end(dest), mume_clone(obj));
        }
    }

    return dest;
}

static void* _octnr_begin(void *self)
{
    return NULL;
}

static void* _octnr_end(void *self)
{
    return NULL;
}

static void* _octnr_next(void *self, void *it)
{
    return NULL;
}

static void* _octnr_value(void *self, void *it)
{
    return NULL;
}

static void* _octnr_insert(void *self, void *it, void *object)
{
    return NULL;
}

static void* _octnr_find(void *self, const void *object)
{
    void *it, *obj;

    mume_octnr_foreach(self, it, obj) {
        if (obj == object)
            return it;
    }

    return mume_octnr_end(self);
}

static void* _octnr_erase(void *self, void *it)
{
    return NULL;
}

static size_t _octnr_size(void *self)
{
    return 0;
}

static void _octnr_clear(void *self)
{
}

static void* _octnr_class_ctor(
    struct _octnr_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_octnr_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_octnr_begin)
            *(voidf**)&self->begin = method;
        else if (selector == (voidf*)_mume_octnr_end)
            *(voidf**)&self->end = method;
        else if (selector == (voidf*)_mume_octnr_next)
            *(voidf**)&self->next = method;
        else if (selector == (voidf*)_mume_octnr_value)
            *(voidf**)&self->value = method;
        else if (selector == (voidf*)_mume_octnr_insert)
            *(voidf**)&self->insert = method;
        else if (selector == (voidf*)_mume_octnr_find)
            *(voidf**)&self->find = method;
        else if (selector == (voidf*)_mume_octnr_erase)
            *(voidf**)&self->erase = method;
        else if (selector == (voidf*)_mume_octnr_size)
            *(voidf**)&self->size = method;
        else if (selector == (voidf*)_mume_octnr_clear)
            *(voidf**)&self->clear = method;
    }

    return self;
}

const void* mume_octnr_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_octnr_meta_class(),
        "octnr",
        _octnr_super_class(),
        sizeof(struct _octnr),
        MUME_PROP_END,
        _mume_copy, _octnr_copy,
        _mume_octnr_begin, _octnr_begin,
        _mume_octnr_end, _octnr_end,
        _mume_octnr_next, _octnr_next,
        _mume_octnr_value, _octnr_value,
        _mume_octnr_insert, _octnr_insert,
        _mume_octnr_find, _octnr_find,
        _mume_octnr_erase, _octnr_erase,
        _mume_octnr_size, _octnr_size,
        _mume_octnr_clear, _octnr_clear,
        MUME_FUNC_END);
}

const void* mume_octnr_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "octnr class",
        _octnr_super_meta_class(),
        sizeof(struct _octnr_class),
        MUME_PROP_END,
        _mume_ctor, _octnr_class_ctor,
        MUME_FUNC_END);
}

void mume_octnr_enumerate(
    const void *self, void (*proc)(void*, void*), void *closure)
{
    void *it, *end;

    end = mume_octnr_end(self);
    for (it = mume_octnr_begin(self); it != end;
         it = mume_octnr_next(self, it))
    {
        proc(closure, mume_octnr_value(self, it));
    }
}

void mume_octnr_append(
    void *self, const void *from, const void *clazz)
{
    void *it;
    void *end;
    void *pos;
    void *obj;

    end = mume_octnr_end(from);
    pos = mume_octnr_end(self);

    for (it = mume_octnr_begin(from); it != end;
         it = mume_octnr_next(from, it))
    {
        obj = mume_octnr_value(from, it);
        if (mume_is_of(obj, clazz)) {
            pos = mume_octnr_insert(self, pos, mume_clone(obj));
            pos = mume_octnr_next(self, pos);
        }
    }
}

void* _mume_octnr_begin(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, begin, (_self));
}

void* _mume_octnr_end(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, end, (_self));
}

void* _mume_octnr_next(const void *_clazz, void *_self, void *it)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, next, (_self, it));
}

void* _mume_octnr_value(const void *_clazz, void *_self, void *it)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, value, (_self, it));
}

void* _mume_octnr_insert(
    const void *_clazz, void *_self, void *it, void *object)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, insert, (_self, it, object));
}

void* _mume_octnr_find(
    const void *_clazz, void *_self, const void *object)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, find, (_self, object));
}

void* _mume_octnr_erase(const void *_clazz, void *_self, void *it)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, erase, (_self, it));
}

size_t _mume_octnr_size(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, size, (_self));
}

void _mume_octnr_clear(const void *_clazz, void *_self)
{
    MUME_SELECTOR_NORETURN(
        mume_octnr_meta_class(), mume_octnr_class(),
        struct _octnr_class, clear, (_self));
}
