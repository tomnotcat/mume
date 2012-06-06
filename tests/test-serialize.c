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
#include "mume-base.h"
#include "test-util.h"

/************************* myobj1.h *************************/
#define SIZEOF_MYOBJ1 (MUME_SIZEOF_OBJECT + \
                       sizeof(int) +        \
                       sizeof(char*) +      \
                       sizeof(float) +      \
                       sizeof(double))

const void* myobj1_class(void);

#define myobj1_meta_class mume_meta_class

void* myobj1_new(int a, const char *b, float c, double d);

/************************* myobj1.c *************************/
struct _myobj1 {
    const char _[MUME_SIZEOF_OBJECT];
    int a;
    char *b;
    float c;
    double d;
};

MUME_STATIC_ASSERT(sizeof(struct _myobj1) == SIZEOF_MYOBJ1);

#define _myobj1_super_class mume_object_class

enum _myobj1_props_e {
    _OBJ1_PROP_A,
    _OBJ1_PROP_B,
    _OBJ1_PROP_C,
    _OBJ1_PROP_D
};

static void* _myobj1_ctor(
    struct _myobj1 *self, int mode, va_list *app)
{
    self->a = 0;
    self->b = NULL;
    self->c = 0;
    self->d = 0;

    if (!_mume_ctor(_myobj1_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->a = va_arg(*app, int);
        self->b = strdup_abort(va_arg(*app, char*));
        self->c = va_arg(*app, double);
        self->d = va_arg(*app, double);
    }

    return self;
}

static void* _myobj1_dtor(struct _myobj1 *self)
{
    free(self->b);
    return _mume_dtor(_myobj1_super_class(), self);
}

static void* _myobj1_copy(
    struct _myobj1 *dest, const struct _myobj1 *src)
{
    dest->a = src->a;
    dest->c = src->c;
    dest->d = src->d;

    free(dest->b);
    if (src->b)
        dest->b = strdup_abort(src->b);
    else
        dest->b = NULL;

    return dest;
}

static int _myobj1_set_property(
    struct _myobj1 *self, const void *prop, const void *var)
{
    switch (mume_property_get_id(prop)) {
    case _OBJ1_PROP_A:
        self->a = mume_variant_get_int(var);
        return 1;

    case _OBJ1_PROP_B:
        self->b = strdup_abort(mume_variant_get_string(var));
        return 1;

    case _OBJ1_PROP_C:
        self->c = mume_variant_get_float(var);
        return 1;

    case _OBJ1_PROP_D:
        self->d = mume_variant_get_double(var);
        return 1;
    }

    return 0;
}

static int _myobj1_get_property(
    struct _myobj1 *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _OBJ1_PROP_A:
        mume_variant_set_int(var, self->a);
        return 1;

    case _OBJ1_PROP_B:
        mume_variant_set_string(var, self->b);
        return 1;

    case _OBJ1_PROP_C:
        mume_variant_set_float(var, self->c);
        return 1;

    case _OBJ1_PROP_D:
        mume_variant_set_double(var, self->d);
        return 1;
    }

    return 0;
}

const void* myobj1_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        myobj1_meta_class(),
        "myobj1",
        _myobj1_super_class(),
        sizeof(struct _myobj1),
        mume_property_new(MUME_TYPE_INT, "a",
                          _OBJ1_PROP_A,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_STRING, "b",
                          _OBJ1_PROP_B,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_FLOAT, "c",
                          _OBJ1_PROP_C,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_DOUBLE, "d",
                          _OBJ1_PROP_D,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _myobj1_ctor,
        _mume_dtor, _myobj1_dtor,
        _mume_copy, _myobj1_copy,
        _mume_set_property, _myobj1_set_property,
        _mume_get_property, _myobj1_get_property,
        MUME_FUNC_END);
}

void* myobj1_new(int a, const char *b, float c, double d)
{
    return mume_new(myobj1_class(), a, b, c, d);
}

/************************* myobj2.h *************************/
#define SIZEOF_MYOBJ2 (SIZEOF_MYOBJ1 + \
                       sizeof(int) + \
                       sizeof(void*) * 3)

const void* myobj2_class(void);

#define myobj2_meta_class myobj1_meta_class

void* myobj2_new(int a, const char *b, float c,
                 double d, int e, void *obj, void *var);

void myobj2_add(void *self, int a, const char *b, float c, double d);

/************************* myobj2.c *************************/
struct _myobj2 {
    const char _[SIZEOF_MYOBJ1];
    int e;
    void *obj;
    void *objs;
    void *var;
};

MUME_STATIC_ASSERT(sizeof(struct _myobj2) == SIZEOF_MYOBJ2);

#define _myobj2_super_class myobj1_class

enum _myobj2_props_e {
    _OBJ2_PROP_E,
    _OBJ2_PROP_OBJ,
    _OBJ2_PROP_OBJS,
    _OBJ2_PROP_VAR
};

static void* _myobj2_ctor(
    struct _myobj2 *self, int mode, va_list *app)
{
    self->e = 0;
    self->obj = NULL;
    self->objs = NULL;
    self->var = NULL;

    if (!_mume_ctor(_myobj2_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        const void *obj;
        self->e = va_arg(*app, int);
        obj = va_arg(*app, void*);
        self->obj = obj ? mume_clone(obj) : NULL;
        self->objs = mume_olist_new(mume_delete);
        obj = va_arg(*app, void*);
        self->var = obj ? mume_clone(obj) : NULL;
    }

    return self;
}

static void* _myobj2_dtor(struct _myobj2 *self)
{
    mume_delete(self->obj);
    mume_delete(self->objs);
    mume_delete(self->var);
    return _mume_dtor(_myobj2_super_class(), self);
}

static void* _myobj2_copy(
    struct _myobj2 *dest, const struct _myobj2 *src)
{
    dest->e = src->e;
    mume_delete(dest->obj);
    dest->obj = src->obj ? mume_clone(src->obj) : NULL;
    mume_delete(dest->objs);
    dest->objs = src->objs ? mume_clone(src->objs) : NULL;
    mume_delete(dest->var);
    dest->var = src->var ? mume_clone(src->var) : NULL;
    return dest;
}

static int _myobj2_set_property(
    struct _myobj2 *self, const void *prop, const void *var)
{
    const void *obj;

    switch (mume_property_get_id(prop)) {
    case _OBJ2_PROP_E:
        self->e = mume_variant_get_int(var);
        return 1;

    case _OBJ2_PROP_OBJ:
        obj = mume_variant_get_object(var);
        assert(obj && mume_is_a(obj, myobj1_class()));
        assert(NULL == self->obj);
        self->obj = mume_clone(obj);
        return 1;

    case _OBJ2_PROP_OBJS:
        obj = mume_variant_get_object(var);
        assert(obj && mume_is_a(obj, mume_olist_class()));
        assert(NULL == self->objs);
        self->objs = mume_clone(obj);
        return 1;

    case _OBJ2_PROP_VAR:
        obj = mume_variant_get_object(var);
        assert(obj && mume_is_a(obj, mume_variant_class()));
        assert(NULL == self->var);
        self->var = mume_clone(obj);
        return 1;
    }

    return 0;
}

static int _myobj2_get_property(
    struct _myobj2 *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _OBJ2_PROP_E:
        mume_variant_set_int(var, self->e);
        return 1;

    case _OBJ2_PROP_OBJ:
        mume_variant_set_static_object(var, self->obj);
        return 1;

    case _OBJ2_PROP_OBJS:
        mume_variant_set_static_object(var, self->objs);
        return 1;

    case _OBJ2_PROP_VAR:
        mume_variant_set_static_object(var, self->var);
        return 1;
    }

    return 0;
}

const void* myobj2_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        myobj2_meta_class(),
        "myobj2",
        _myobj2_super_class(),
        sizeof(struct _myobj2),
        mume_property_new(MUME_TYPE_INT, "e",
                          _OBJ2_PROP_E,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "obj",
                          _OBJ2_PROP_OBJ,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "objs",
                          _OBJ2_PROP_OBJS,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "var",
                          _OBJ2_PROP_VAR,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _myobj2_ctor,
        _mume_dtor, _myobj2_dtor,
        _mume_copy, _myobj2_copy,
        _mume_set_property, _myobj2_set_property,
        _mume_get_property, _myobj2_get_property,
        MUME_FUNC_END);
}

void* myobj2_new(int a, const char *b, float c,
                 double d, int e, void *obj, void *var)
{
    return mume_new(myobj2_class(), a, b, c, d, e, obj, var);
}

void myobj2_add(void *_self, int a, const char *b, float c, double d)
{
    struct _myobj2 *self = _self;
    mume_olist_push_back(self->objs, myobj1_new(a, b, c, d));
}

/************************* test *************************/
static void _test_obj1(
    struct _myobj1 *obj, int a, const char *b, float c, double d)
{
    test_assert(mume_is_of(obj, myobj1_class()));
    test_assert(obj->a == a);
    test_assert(obj->b == b || 0 == strcmp(obj->b, b));
    test_assert(fabs(obj->c - c) < 0.00001);
    test_assert(abs(obj->d - d) < 0.00001);
}

static void _test_obj2(
    struct _myobj2 *obj, int a, const char *b, float c, double d,
    int e, const struct _myobj1 *objs, int count, const void *var)
{
    int i;
    void *it = NULL;

    test_assert(mume_is_of(obj, myobj2_class()));
    _test_obj1((struct _myobj1*)obj, a, b, c, d);
    test_assert(obj->e == e);

    if (obj->objs) {
        test_assert(mume_octnr_size(obj->objs) == count);
        it = mume_octnr_begin(obj->objs);
    }
    else {
        test_assert(0 == count);
    }

    for (i = 0; i < count; ++i) {
        _test_obj1(mume_octnr_value(obj->objs, it),
                   objs[i].a, objs[i].b, objs[i].c, objs[i].d);

        it = mume_octnr_next(obj->objs, it);
    }

    if (obj->objs)
        test_assert(it == mume_octnr_end(obj->objs));

    if (obj->var)
        test_assert(0 == mume_compare(obj->var, var));
    else
        test_assert(NULL == var);
}

void all_tests(void)
{
    const char *file = TESTS_DATA_DIR "/test-serialize.xml";
    void *obj, *obj1, *obj2;
    void *vars[5];
    const struct _myobj1 objs[] = {
        { {0}, -1, "a first", 10.1, 10.00023 },
        { {0}, 0, "abc", -32.1, -0.00023 },
        { {0}, 24, "hello world", 2.8, -1024.00023 },
    };

    int i;
    char buf[256];
    void *ser = mume_serialize_new();

    /* Save. */
    obj1 = myobj1_new(1, "hello", 1.2, 3.4);
    obj2 = myobj2_new(2, "world", 5.6, 7.8, -1, obj1, NULL);

    mume_serialize_set_object(ser, "obj1", obj1);
    mume_serialize_set_static_object(ser, "obj2", obj2);
    test_assert(mume_serialize_get_object(ser, "obj1") != obj1);
    test_assert(mume_serialize_get_object(ser, "obj2") == obj2);

    for (i = 0; i < COUNT_OF(objs); ++i)
        myobj2_add(obj2, objs[i].a, objs[i].b, objs[i].c, objs[i].d);

    vars[0] = mume_variant_new_int(10);
    vars[1] = mume_variant_new_float(0.234);
    vars[2] = mume_variant_new_double(234.3438890032);
    vars[3] = mume_variant_new_static_string("dummy text");
    vars[4] = mume_variant_new_object(vars[3]);
    for (i = 0; i < COUNT_OF(vars); ++i) {
        snprintf(buf, sizeof(buf), "objs-%d", i);
        obj = myobj2_new(0, NULL, 0, 0, 0, NULL, vars[i]);
        mume_serialize_set_object(ser, buf, obj);
        mume_delete(obj);
    }

    test_assert(mume_serialize_write_to_file(ser, file));

    mume_delete(obj1);
    mume_delete(obj2);
    mume_delete(ser);

    /* Load. */
    ser = mume_serialize_new();
    mume_serialize_register(ser, myobj1_class());
    mume_serialize_register(ser, myobj2_class());
    mume_serialize_register(ser, mume_olist_class());
    mume_serialize_register(ser, mume_variant_class());

    test_assert(mume_serialize_read_from_file(ser, file));

    obj1 = (void*)mume_serialize_get_object(ser, "obj1");
    _test_obj1(obj1, 1, "hello", 1.2, 3.4);

    obj2 = (void*)mume_serialize_get_object(ser, "obj2");
    _test_obj2(obj2, 2, "world", 5.6, 7.8, -1,
               objs, COUNT_OF(objs), NULL);

    for (i = 0; i < COUNT_OF(vars); ++i) {
        snprintf(buf, sizeof(buf), "objs-%d", i);
        obj = (void*)mume_serialize_get_object(ser, buf);
        _test_obj2(obj, 0, NULL, 0, 0, 0, NULL, 0, vars[i]);
        mume_delete(vars[i]);
    }

    mume_delete(ser);
}
