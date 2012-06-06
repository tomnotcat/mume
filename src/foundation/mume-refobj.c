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
#include "mume-refobj.h"
#include "mume-debug.h"
#include MUME_ASSERT_H

#define _refobj_super_class mume_object_class

struct _refobj {
    const char _[MUME_SIZEOF_OBJECT];
    int refcount;
};

MUME_STATIC_ASSERT(sizeof(struct _refobj) == MUME_SIZEOF_REFOBJ);

static void* _refobj_ctor(
    struct _refobj *self, int mode, va_list *app)
{
    if (!_mume_ctor(_refobj_super_class(), self, mode, app))
        return NULL;

    self->refcount = 0;
    return self;
}

static void* _refobj_dtor(struct _refobj *self)
{
    assert(0 == self->refcount);
    return _mume_dtor(_refobj_super_class(), self);
}

const void* mume_refobj_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_refobj_meta_class(),
        "refobj",
        _refobj_super_class(),
        sizeof(struct _refobj),
        MUME_PROP_END,
        _mume_ctor, _refobj_ctor,
        _mume_dtor, _refobj_dtor,
        MUME_FUNC_END);
}

void mume_refobj_addref(void *_self)
{
    struct _refobj *self = _self;

    assert(mume_is_of(_self, mume_refobj_class()));

    ++self->refcount;
}

void mume_refobj_release(void *_self)
{
    struct _refobj *self = _self;

    assert(mume_is_of(_self, mume_refobj_class()));

    if (self->refcount)
        --self->refcount;
    else
        mume_delete(self);
}

int mume_refobj_refcount(const void *_self)
{
    const struct _refobj *self = _self;
    assert(mume_is_of(_self, mume_refobj_class()));
    return self->refcount;
}
