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

enum draw_mask {
    DRAW_POINT = 1,
    DRAW_CIRCLE = 2
};

static unsigned int draw_result;

/************************* point.h *************************/
#define SIZEOF_POINT (MUME_SIZEOF_OBJECT + \
                      sizeof(char*) +      \
                      sizeof(int) +        \
                      sizeof(int))

#define SIZEOF_POINT_CLASS (MUME_SIZEOF_CLASS + \
                            sizeof(voidf*) * 2)

void point_set_name(void *self, const char *name);

const char* point_get_name(const void *self);

void point_get_coord(const void *self, int *x, int *y);

void _point_move(const void *clazz, void *self, int x, int y);

#define point_move(_self, _x, _y) \
    _point_move(NULL, _self, _x, _y)

void _point_draw(const void *clazz, const void *self);

#define point_draw(_self) _point_draw(NULL, _self)

const void* point_class(void);

const void* point_meta_class(void);

void* point_new(const char *name, int x, int y);

/************************* point.c *************************/
struct _point {
    const char _[MUME_SIZEOF_OBJECT];
    char *name;
    int x, y;
};

struct _point_class {
    const char _[MUME_SIZEOF_CLASS];
    void (*move)(void *self, int x, int y);
    void (*draw)(const void *self);
};

MUME_STATIC_ASSERT(sizeof(struct _point) == SIZEOF_POINT);
MUME_STATIC_ASSERT(sizeof(struct _point_class) == SIZEOF_POINT_CLASS);

#define _point_super_class mume_object_class
#define _point_super_meta_class mume_meta_class

enum _point_props_e {
    _POINT_PROP_NAME,
    _POINT_PROP_COORD
};

static void* _point_ctor(
    struct _point *self, int mode, va_list *app)
{
    if (!_mume_ctor(_point_super_class(), self, mode, app))
        return NULL;

    self->name = strdup_abort(va_arg(*app, char*));
    self->x = va_arg(*app, int);
    self->y = va_arg(*app, int);

    return self;
}

static void* _point_dtor(struct _point *self)
{
    free(self->name);
    return _mume_dtor(_point_super_class(), self);
}

static int _point_set_property(
    struct _point *self, const void *prop, const void *var)
{
    switch (mume_property_get_id(prop)) {
    case _POINT_PROP_NAME:
        point_set_name(self, mume_variant_get_string(var));
        return 1;

    case _POINT_PROP_COORD:
        {
            mume_point_t pt;
            pt = mume_point_from_string(
                mume_variant_get_string(var));
            point_move(self, pt.x, pt.y);
        }
        return 1;
    }

    return 0;
}

static int _point_get_property(
    struct _point *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _POINT_PROP_NAME:
        mume_variant_set_string(var, point_get_name(self));
        return 1;

    case _POINT_PROP_COORD:
        {
            mume_point_t pt;
            char buf[256];
            point_get_coord(self, &pt.x, &pt.y);
            mume_variant_set_string(
                var, mume_point_to_string(pt, buf, COUNT_OF(buf)));
        }
        return 1;
    }

    return 0;
}

static void __point_move(void *_self, int x, int y)
{
    struct _point *self = _self;
    self->x = x;
    self->y = y;
}

static void __point_draw(const void *_self)
{
    draw_result |= DRAW_POINT;
}

static void* _point_class_ctor(
    struct _point_class *self, int mode, va_list *app)
{
    va_list ap;
    voidf *selector, *method;

    if (!_mume_ctor(_point_super_meta_class(), self, mode, app))
        return NULL;

    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_point_move)
            *(voidf**)&self->move = method;
        else if (selector == (voidf*)_point_draw)
            *(voidf**)&self->draw = method;
    }

    return self;
}

void point_set_name(void *_self, const char *name)
{
    struct _point *self = _self;
    free(self->name);
    self->name = name ? strdup_abort(name) : NULL;
}

const char* point_get_name(const void *_self)
{
    const struct _point *self = _self;
    return self->name;
}

void point_get_coord(const void *_self, int *x, int *y)
{
    const struct _point *self = _self;
    *x = self->x;
    *y = self->y;
}

void _point_move(const void *_clazz, void *_self, int x, int y)
{
    const struct _point_class *clazz = _clazz;
    if (NULL == clazz)
        clazz =  mume_class_of(_self);
    assert(clazz->move);
    clazz->move(_self, x, y);
}

void _point_draw(const void *_clazz, const void *_self)
{
    const struct _point_class *clazz = _clazz;
    if (NULL == clazz)
        clazz =  mume_class_of(_self);
    assert(clazz->draw);
    clazz->draw(_self);
}

const void* point_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        point_meta_class(),
        "point",
        _point_super_class(),
        sizeof(struct _point),
        mume_property_new(MUME_TYPE_STRING, "name",
                          _POINT_PROP_NAME,
                          MUME_PROP_READWRITE),
        mume_property_new(MUME_TYPE_STRING, "coord",
                          _POINT_PROP_COORD,
                          MUME_PROP_READWRITE),
        MUME_PROP_END,
        _mume_ctor, _point_ctor,
        _mume_dtor, _point_dtor,
        _mume_set_property, _point_set_property,
        _mume_get_property, _point_get_property,
        _point_move, __point_move,
        _point_draw, __point_draw,
        MUME_FUNC_END);
}

const void* point_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "point class",
        _point_super_meta_class(),
        sizeof(struct _point_class),
        MUME_PROP_END,
        _mume_ctor, _point_class_ctor,
        MUME_FUNC_END);
}

void* point_new(const char *name, int x, int y)
{
    return mume_new(point_class(), name, x, y);
}

/************************* circle.h *************************/
#define SIZEOF_CIRCLE (SIZEOF_POINT + \
                       sizeof(double))

#define SIZEOF_CIRCLE_CLASS SIZEOF_POINT_CLASS

void circle_set_radian(void *self, double rad);

double circle_get_radian(const void *self);

const void* circle_class(void);

const void* circle_meta_class(void);

void* circle_new(const char *name, int x, int y, double rad);

/************************* circle.c *************************/
#define _circle_super_class point_class
#define _circle_super_meta_class point_meta_class

struct _circle {
    const char _[SIZEOF_POINT];
    double rad;
};

struct _circle_class {
    const char _[SIZEOF_POINT_CLASS];
};

MUME_STATIC_ASSERT(sizeof(struct _circle) == SIZEOF_CIRCLE);
MUME_STATIC_ASSERT(sizeof(struct _circle_class) == SIZEOF_CIRCLE_CLASS);

enum _circle_props_e {
    _CIRCLE_PROP_RADIAN
};

static void* _circle_ctor(
    struct _circle *self, int mode, va_list *app)
{
    if (!_mume_ctor(point_class(), self, mode, app))
        return NULL;

    self->rad = va_arg(*app, double);
    return self;
}

static void _circle_set_property(
    struct _circle *self, const void *prop, const void *var)
{
    switch (mume_property_get_id(prop)) {
    case _CIRCLE_PROP_RADIAN:
        circle_set_radian(self, mume_variant_get_double(var));
        break;
    }
}

static void _circle_get_property(
    struct _circle *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _CIRCLE_PROP_RADIAN:
        mume_variant_set_double(var, circle_get_radian(self));
        break;
    }
}

static void _circle_draw(const void *self)
{
    _point_draw(point_class(), self);
    draw_result |= DRAW_CIRCLE;
}

void circle_set_radian(void *_self, double rad)
{
    struct _circle *self = _self;
    self->rad = rad;
}

double circle_get_radian(const void *_self)
{
    const struct _circle *self = _self;
    return self->rad;
}

const void* circle_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        circle_meta_class(),
        "circle",
        _circle_super_class(),
        sizeof(struct _circle),
        mume_property_new(MUME_TYPE_DOUBLE, "radian",
                          _CIRCLE_PROP_RADIAN,
                          MUME_PROP_READWRITE),
        MUME_PROP_END,
        _mume_ctor, _circle_ctor,
        _mume_set_property, _circle_set_property,
        _mume_get_property, _circle_get_property,
        _point_draw, _circle_draw,
        MUME_FUNC_END);
}

const void* circle_meta_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_meta_class(),
        "circle class",
        _circle_super_meta_class(),
        sizeof(struct _circle_class),
        MUME_PROP_END,
        MUME_FUNC_END);
}

void* circle_new(const char *name, int x, int y, double rad)
{
    return mume_new(circle_class(), name, x, y, rad);
}

/************************* test *************************/
struct _property {
    const char *name;
    const char *value;
};

struct _propinfo {
    int type;
    const char *name;
    int id;
    unsigned int flags;
};

static void _test_properties(
    const void *clazz, const struct _propinfo *props, int count)
{
    int i, j, pc;
    const void *px;
    const void **pp;

    pp = mume_class_properties(clazz, &pc);
    test_assert(pc == count);

    for (i = 0; i < count; ++i) {
        px = mume_class_property(clazz, props[i].name);
        test_assert(px);

        for (j = 0; j < pc; ++j) {
            if (pp[j] == px)
                break;
        }

        test_assert(j < pc);
        test_assert(mume_property_get_type(px) == props[i].type);
        test_assert(0 == strcmp(
            mume_property_get_name(px), props[i].name));
        test_assert(mume_property_get_id(px) == props[i].id);
        test_assert(mume_property_get_flags(px) == props[i].flags);
    }
}

static void _test_get_properties(
    void *object, const struct _property *props, int count)
{
    int i;
    void *var = mume_variant_new(MUME_TYPE_INT);

    for (i = 0; i < count; ++i) {
        test_assert(mume_get_property(object, props[i].name, var));
        test_assert(mume_variant_convert(var, MUME_TYPE_STRING));
        test_assert(0 == strcmp(
            props[i].value, mume_variant_get_string(var)));
    }

    test_assert(!mume_get_property(object, "nonexist", var));
    mume_delete(var);
}

static void _test_set_properties(
    void *object, const struct _property *props, int count)
{
    int i;
    void *var = mume_variant_new(MUME_TYPE_STRING);

    for (i = 0; i < count; ++i) {
        mume_variant_reset(var, MUME_TYPE_STRING);
        mume_variant_set_string(var, props[i].value);
        test_assert(mume_set_property(object, props[i].name, var));
    }

    test_assert(!mume_set_property(object, "nonexist", var));
    mume_delete(var);
}

static void _test_point(void)
{
    int x, y;
    const void *clazz;
    void *point;

    const struct _propinfo props[] = {
        { MUME_TYPE_STRING, "name",
          _POINT_PROP_NAME, MUME_PROP_READWRITE },
        { MUME_TYPE_STRING, "coord",
          _POINT_PROP_COORD, MUME_PROP_READWRITE },
    };

    const struct _property props1[] = {
        { "name", "hello" },
        { "coord", "10, 20" },
    };

    const struct _property props2[] = {
        { "name", "another name" },
        { "coord", "-1050, -1" },
    };

    _test_properties(point_class(), props, COUNT_OF(props));
    test_assert(mume_class_of(mume_object_class()) == mume_meta_class());
    test_assert(mume_class_of(mume_meta_class()) == mume_meta_class());
    test_assert(mume_super_class(mume_object_class()) == mume_object_class());
    test_assert(mume_super_class(mume_meta_class()) == mume_object_class());
    point = point_new("hello", 10, 20);
    test_assert(strcmp(point_get_name(point), "hello") == 0);
    _test_get_properties(point, props1, COUNT_OF(props1));
    point_get_coord(point, &x, &y);
    test_assert(10 == x && 20 == y);
    point_move(point, -10, 30);
    point_get_coord(point, &x, &y);
    test_assert(-10 == x && 30 == y);
    _test_set_properties(point, props2, COUNT_OF(props2));
    _test_get_properties(point, props2, COUNT_OF(props2));
    draw_result = 0;
    point_draw(point);
    test_assert(DRAW_POINT == draw_result);
    clazz = mume_class_of(point);
    test_assert(point_class() == clazz);
    test_assert(0 == strcmp(mume_class_name(clazz), "point"));
    test_assert(mume_object_class() == mume_super_class(clazz));
    test_assert(mume_size_of(point) == sizeof(struct _point));
    test_assert(mume_size_of(point_class()) == sizeof(struct _point_class));
    test_assert(mume_is_a(point, point_class()));
    test_assert(mume_is_of(point, point_class()));
    test_assert(!mume_is_a(point, mume_object_class()));
    test_assert(mume_is_of(point, mume_object_class()));
    test_assert(mume_is_a(point_class(), point_meta_class()));
    test_assert(mume_is_of(point_class(), mume_object_class()));
    test_assert(mume_is_of(point_meta_class(), mume_meta_class()));
    test_assert(mume_is_of(point_class(), mume_meta_class()));
    mume_delete(point);
}

static void _test_circle(void)
{
    int x, y;
    double rad;
    const void *clazz;
    void *circle;

    const struct _propinfo props[] = {
        { MUME_TYPE_DOUBLE, "radian",
          _CIRCLE_PROP_RADIAN, MUME_PROP_READWRITE }
    };

    const struct _property props1[] = {
        { "name", "world" },
        { "coord", "40, 30" },
        { "radian", "3.140000000000" },
    };

    const struct _property props2[] = {
        { "name", "diff world" },
        { "coord", "0, -1024" },
        { "radian", "-1.110000000000" },
    };

    _test_properties(circle_class(), props, COUNT_OF(props));
    circle = circle_new("world", 40, 30, 3.14);
    test_assert(strcmp(point_get_name(circle), "world") == 0);
    _test_get_properties(circle, props1, COUNT_OF(props1));
    point_get_coord(circle, &x, &y);
    test_assert(40 == x && 30 == y);
    point_move(circle, 50, 60);
    point_get_coord(circle, &x, &y);
    test_assert(50 == x && 60 == y);
    rad = circle_get_radian(circle);
    test_assert(fabs(rad - 3.14) < 0.001);
    _test_set_properties(circle, props2, COUNT_OF(props2));
    _test_get_properties(circle, props2, COUNT_OF(props2));
    draw_result = 0;
    point_draw(circle);
    test_assert((DRAW_POINT | DRAW_CIRCLE) == draw_result);
    clazz = mume_class_of(circle);
    test_assert(circle_class() == clazz);
    test_assert(0 == strcmp(mume_class_name(clazz), "circle"));
    test_assert(mume_super_class(clazz) == point_class());
    test_assert(mume_size_of(circle) == sizeof(struct _circle));
    test_assert(mume_size_of(circle_class()) == sizeof(struct _circle_class));
    test_assert(mume_is_a(circle, circle_class()));
    test_assert(!mume_is_a(circle, point_class()));
    test_assert(mume_is_of(circle, point_class()));
    mume_delete(circle);
}

void all_tests(void)
{
    test_run(_test_point);
    test_run(_test_circle);
}
