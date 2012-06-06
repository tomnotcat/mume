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
#include "mume-property.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-variant.h"
#include MUME_ASSERT_H

#define _property_super_class mume_object_class

struct _property {
    const char _[MUME_SIZEOF_OBJECT];
    int type;
    int id;
    char *name;
    unsigned int flags;
};

MUME_STATIC_ASSERT(sizeof(struct _property) == MUME_SIZEOF_PROPERTY);

static void _property_uninit(struct _property *self)
{
    if (!(self->flags & MUME_PROP_STATIC_NAME))
        free(self->name);
}

static void* _property_ctor(
    struct _property *self, int mode, va_list *app)
{
    self->type = 0;
    self->name = NULL;
    self->flags = 0;

    if (!_mume_ctor(_property_super_class(), self, mode, app))
        return NULL;

    switch (mode) {
    case MUME_CTOR_NORMAL:
        self->type = va_arg(*app, int);
        self->name = va_arg(*app, char*);
        self->id = va_arg(*app, int);
        self->flags = va_arg(*app, unsigned int);

        if (self->name && !(self->flags & MUME_PROP_STATIC_NAME))
            self->name = strdup_abort(self->name);

        assert(mume_type_is_valid(self->type));
        break;

    case MUME_CTOR_PROPERTY:
        if (NULL == self->name ||
            !mume_type_is_valid(self->type))
        {
            return NULL;
        }
        break;

    case MUME_CTOR_KEY:
        self->name = va_arg(*app, char*);
        self->flags = MUME_PROP_STATIC_NAME;
        break;
    }

    return self;
}

static void* _property_dtor(struct _property *self)
{
    _property_uninit(self);
    return _mume_dtor(_property_super_class(), self);
}

static void* _property_copy(
    struct _property *self, const struct _property *src)
{
    _property_uninit(self);

    self->type = src->type;

    if (src->name && !(src->flags & MUME_PROP_STATIC_NAME))
        self->name = strdup_abort(src->name);
    else
        self->name = src->name;

    self->id = src->id;
    self->flags = src->flags;

    return self;
}

static int _property_compare(
    const struct _property *a, const struct _property *b)
{
    return strcmp(a->name, b->name);
}

const void* mume_property_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_property_meta_class(),
        "property",
        _property_super_class(),
        sizeof(struct _property),
        MUME_PROP_END,
        _mume_ctor, _property_ctor,
        _mume_dtor, _property_dtor,
        _mume_copy, _property_copy,
        _mume_compare, _property_compare,
        MUME_FUNC_END);
}

void* mume_property_new(
    int type, const char *name, int id, unsigned int flags)
{
    return mume_new(
        mume_property_class(), type, name, id, flags);
}

void* mume_property_key(void *self, const char *name)
{
    return mume_ctor(
        mume_property_class(), self, MUME_CTOR_KEY, name);
}

int mume_property_get_type(const void *_self)
{
    const struct _property *self = _self;
    assert(mume_is_of(_self, mume_property_class()));
    return self->type;
}

const char* mume_property_get_name(const void *_self)
{
    const struct _property *self = _self;
    assert(mume_is_of(_self, mume_property_class()));
    return self->name;
}

int mume_property_get_id(const void *_self)
{
    const struct _property *self = _self;
    assert(mume_is_of(_self, mume_property_class()));
    return self->id;
}

unsigned int mume_property_get_flags(const void *_self)
{
    const struct _property *self = _self;
    assert(mume_is_of(_self, mume_property_class()));
    return self->flags;
}
