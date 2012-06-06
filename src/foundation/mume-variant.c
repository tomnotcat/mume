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
#include "mume-variant.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-property.h"
#include MUME_ASSERT_H
#include MUME_MATH_H

#define MUME_TYPE_UNKNOWN (-1)
#define MUME_TYPE_STATIC_STRING (MUME_TYPE_OBJECT + 1)
#define MUME_TYPE_STATIC_OBJECT (MUME_TYPE_OBJECT + 2)

#define _variant_super_class mume_object_class

struct _variant {
    const char _[MUME_SIZEOF_OBJECT];
    int type;
    union {
        int i;
        float f;
        double d;
        char *s;
        void *o;
    } data;
};

MUME_STATIC_ASSERT(sizeof(struct _variant) == MUME_SIZEOF_VARIANT);

static void _variant_init(struct _variant *self)
{
    switch (self->type) {
    case MUME_TYPE_INT:
        self->data.i = 0;
        break;

    case MUME_TYPE_FLOAT:
        self->data.f = 0;
        break;

    case MUME_TYPE_DOUBLE:
        self->data.d = 0;
        break;

    case MUME_TYPE_STRING:
        self->data.s = NULL;
        break;

    case MUME_TYPE_OBJECT:
        self->data.o = NULL;
        break;

    default:
        mume_warning(("Unknown type: %d\n", self->type));
        break;
    }
}

static void _variant_uninit(struct _variant *self)
{
    switch (self->type) {
    case MUME_TYPE_STRING:
        free(self->data.s);
        break;

    case MUME_TYPE_OBJECT:
        mume_delete(self->data.o);
        break;
    }
}

static void _convert_to_int(struct _variant *self)
{
    int value;

    switch (self->type) {
    case MUME_TYPE_FLOAT:
        value = self->data.f;
        break;

    case MUME_TYPE_DOUBLE:
        value = self->data.d;
        break;

    case MUME_TYPE_STRING:
    case MUME_TYPE_STATIC_STRING:
        if (self->data.s)
            value = strtol(self->data.s, NULL, 10);
        else
            value = 0;
        break;

    default:
        return;
    }

    _variant_uninit(self);

    self->type = MUME_TYPE_INT;
    self->data.i = value;
}

static void _convert_to_float(struct _variant *self)
{
    float value;

    switch (self->type) {
    case MUME_TYPE_INT:
        value = self->data.i;
        break;

    case MUME_TYPE_DOUBLE:
        value = self->data.d;
        break;

    case MUME_TYPE_STRING:
    case MUME_TYPE_STATIC_STRING:
        if (self->data.s)
            value = strtod(self->data.s, NULL);
        else
            value = 0;
        break;

    default:
        return;
    }

    _variant_uninit(self);

    self->type = MUME_TYPE_FLOAT;
    self->data.f = value;
}

static void _convert_to_double(struct _variant *self)
{
    double value;

    switch (self->type) {
    case MUME_TYPE_INT:
        value = self->data.i;
        break;

    case MUME_TYPE_FLOAT:
        value = self->data.f;
        break;

    case MUME_TYPE_STRING:
    case MUME_TYPE_STATIC_STRING:
        if (self->data.s)
            value = strtod(self->data.s, NULL);
        else
            value = 0;
        break;

    default:
        return;
    }

    _variant_uninit(self);

    self->type = MUME_TYPE_DOUBLE;
    self->data.d = value;
}

static void _convert_to_string(struct _variant *self)
{
    char value[256];

    switch (self->type) {
    case MUME_TYPE_INT:
        snprintf(value, COUNT_OF(value), "%d", self->data.i);
        break;

    case MUME_TYPE_FLOAT:
        snprintf(value, COUNT_OF(value), "%.6f", self->data.f);
        break;

    case MUME_TYPE_DOUBLE:
        snprintf(value, COUNT_OF(value), "%.12f", self->data.d);
        break;

    default:
        return;
    }

    _variant_uninit(self);

    self->type = MUME_TYPE_STRING;
    self->data.s = strdup_abort(value);
}

static void* _variant_ctor(
    struct _variant *self, int mode, va_list *app)
{
    self->type = MUME_TYPE_UNKNOWN;

    if (!_mume_ctor(_variant_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->type = va_arg(*app, int);
        assert(mume_type_is_valid(self->type));
        _variant_init(self);
    }
    else if (MUME_CTOR_PROPERTY == mode) {
        if (!mume_type_is_valid(self->type))
            return NULL;
    }

    return self;
}

static void* _variant_dtor(struct _variant *self)
{
    _variant_uninit(self);
    return _mume_dtor(_variant_super_class(), self);
}

static void* _variant_copy(
    struct _variant *self, const struct _variant *src)
{
    _variant_uninit(self);

    self->type = src->type;

    switch (self->type) {
    case MUME_TYPE_INT:
        self->data.i = src->data.i;
        break;

    case MUME_TYPE_FLOAT:
        self->data.f = src->data.f;
        break;

    case MUME_TYPE_DOUBLE:
        self->data.d = src->data.d;
        break;

    case MUME_TYPE_STRING:
        if (src->data.s)
            self->data.s = strdup_abort(src->data.s);
        else
            self->data.s = NULL;
        break;

    case MUME_TYPE_STATIC_STRING:
        self->data.s = src->data.s;
        break;

    case MUME_TYPE_OBJECT:
        /* Reset first. */
        self->data.o = NULL;

        if (src->data.o)
            self->data.o = mume_clone(src->data.o);
        break;

    case MUME_TYPE_STATIC_OBJECT:
        self->data.o = src->data.o;
        break;
    }

    return self;
}

static int _variant_compare(
    const struct _variant *a, const struct _variant *b)
{
    int type1 = mume_variant_get_type(a);
    int type2 = mume_variant_get_type(b);

    if (type1 != type2)
        return type1 - type2;

    switch (type1) {
    case MUME_TYPE_INT:
        return (a->data.i - b->data.i);

    case MUME_TYPE_FLOAT:
        if (a->data.f < b->data.f)
            return fabs(a->data.f - b->data.f) < 0.000001 ? 0 : -1;
        else
            return fabs(a->data.f - b->data.f) < 0.000001 ? 0 : 1;

    case MUME_TYPE_DOUBLE:
        if (a->data.d < b->data.d)
            return abs(a->data.d - b->data.d) < 0.000001 ? 0 : -1;
        else
            return abs(a->data.d - b->data.d) < 0.000001 ? 0 : 1;

    case MUME_TYPE_STRING:
        if (a->data.s && b->data.s)
            return strcmp(a->data.s, b->data.s);

        if (a->data.s)
            return 1;

        if (b->data.s)
            return -1;

    case MUME_TYPE_OBJECT:
        if (a->data.o && b->data.o)
            return mume_compare(a->data.o, b->data.o);

        if (a->data.o)
            return 1;

        if (b->data.o)
            return -1;
    }

    return 0;
}

static int _variant_set_property(
    struct _variant *self, const void *prop, const void *var)
{
    if (MUME_TYPE_UNKNOWN == self->type) {
        self->type = mume_property_get_id(prop);
        _variant_init(self);
    }

    if (mume_property_get_id(prop) != mume_variant_get_type(self))
        return 0;

    _variant_copy(self, var);
    return 1;
}

static int _variant_get_property(
    struct _variant *self, const void *prop, void *var)
{
    if (mume_property_get_id(prop) != mume_variant_get_type(self))
        return 0;

    _variant_copy(var, self);
    return 1;
}

const void* mume_variant_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_variant_meta_class(),
        "variant",
        _variant_super_class(),
        sizeof(struct _variant),
        mume_property_new(MUME_TYPE_INT, "int",
                          MUME_TYPE_INT,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_FLOAT, "float",
                          MUME_TYPE_FLOAT,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_DOUBLE, "double",
                          MUME_TYPE_DOUBLE,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_STRING, "string",
                          MUME_TYPE_STRING,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "object",
                          MUME_TYPE_OBJECT,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _variant_ctor,
        _mume_dtor, _variant_dtor,
        _mume_copy, _variant_copy,
        _mume_compare, _variant_compare,
        _mume_set_property, _variant_set_property,
        _mume_get_property, _variant_get_property,
        MUME_FUNC_END);
}

void* mume_variant_new(int type)
{
    return mume_new(mume_variant_class(), type);
}

void mume_variant_reset(void *_self, int type)
{
    struct _variant *self = _self;

    assert(mume_is_of(_self, mume_variant_class()));

    _variant_uninit(self);
    self->type = type;
    _variant_init(self);
}

int mume_variant_get_type(const void *_self)
{
    const struct _variant *self = _self;

    assert(mume_is_of(_self, mume_variant_class()));

    switch (self->type) {
    case MUME_TYPE_STATIC_STRING:
        return MUME_TYPE_STRING;

    case MUME_TYPE_STATIC_OBJECT:
        return MUME_TYPE_OBJECT;
    }

    return self->type;
}

void mume_variant_set_int(void *_self, int value)
{
    struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_INT);
    self->data.i = value;
}

int mume_variant_get_int(const void *_self)
{
    const struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_INT);
    return self->data.i;
}

void mume_variant_set_float(void *_self, float value)
{
    struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_FLOAT);
    self->data.f = value;
}

float mume_variant_get_float(const void *_self)
{
    const struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_FLOAT);
    return self->data.f;
}

void mume_variant_set_double(void *_self, double value)
{
    struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_DOUBLE);
    self->data.d = value;
}

double mume_variant_get_double(const void *_self)
{
    const struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_DOUBLE);
    return self->data.d;
}

void mume_variant_set_string(void *_self, const char *string)
{
    struct _variant *self = _self;

    assert(mume_variant_get_type(self) == MUME_TYPE_STRING);

    if (self->type == MUME_TYPE_STRING)
        free(self->data.s);
    else
        self->type = MUME_TYPE_STRING;

    if (string)
        self->data.s = strdup_abort(string);
    else
        self->data.s = NULL;
}

void mume_variant_append_string(
    void *_self, const char *string, int length)
{
    struct _variant *self = _self;

    assert(mume_variant_get_type(self) == MUME_TYPE_STRING);

    if (self->type == MUME_TYPE_STRING) {
        int old_len = self->data.s ? strlen(self->data.s) : 0;
        self->data.s = realloc_abort(
            self->data.s, old_len + length + 1);
        strncpy(self->data.s + old_len, string, length);
        self->data.s[old_len + length] = '\0';
    }
    else {
        self->type = MUME_TYPE_STRING;
        self->data.s = strndup_abort(string, length);
    }
}

void mume_variant_set_static_string(void *_self, const char *string)
{
    struct _variant *self = _self;

    assert(mume_variant_get_type(self) == MUME_TYPE_STRING);

    if (self->type == MUME_TYPE_STRING) {
        free(self->data.s);
        self->type = MUME_TYPE_STATIC_STRING;
    }

    self->data.s = (char*)string;
}

const char* mume_variant_get_string(const void *_self)
{
    const struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_STRING);
    return self->data.s;
}

void mume_variant_set_object(void *_self, const void *object)
{
    struct _variant *self = _self;

    assert(mume_variant_get_type(self) == MUME_TYPE_OBJECT);

    if (self->type == MUME_TYPE_OBJECT)
        mume_delete(self->data.o);
    else
        self->type = MUME_TYPE_OBJECT;

    /* Reset first. */
    self->data.o = NULL;

    if (object)
        self->data.o = mume_clone(object);
}

void mume_variant_set_static_object(void *_self, const void *object)
{
    struct _variant *self = _self;

    assert(mume_variant_get_type(self) == MUME_TYPE_OBJECT);

    if (self->type == MUME_TYPE_OBJECT) {
        mume_delete(self->data.o);
        self->type = MUME_TYPE_STATIC_OBJECT;
    }

    self->data.o = (void*)object;
}

const void* mume_variant_get_object(const void *_self)
{
    const struct _variant *self = _self;
    assert(mume_variant_get_type(self) == MUME_TYPE_OBJECT);
    return self->data.o;
}

int mume_variant_convert(void *_self, int type)
{
    struct _variant *self = _self;

    assert(mume_is_of(_self, mume_variant_class()));

    switch (type) {
    case MUME_TYPE_INT:
        _convert_to_int(self);
        break;

    case MUME_TYPE_FLOAT:
        _convert_to_float(self);
        break;

    case MUME_TYPE_DOUBLE:
        _convert_to_double(self);
        break;

    case MUME_TYPE_STRING:
        _convert_to_string(self);
        break;
    }

    return (mume_variant_get_type(self) == type);
}
