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
#include MUME_LIMITS_H
#include MUME_MATH_H
#include MUME_STDLIB_H
#include MUME_STDIO_H
#include MUME_STRING_H

typedef struct mysdata_s {
    int i;
    double d;
    char *s;
} mydata_t;

static void _destruct_udata(void *p)
{
    *(int*)p = 1;
}

static int _mydata_setx(
    void *type, void *obj, const char *str)
{
    mydata_t *data = (mydata_t*)obj;
    mume_type_setstr(mume_typeof_int(), &data->i, str);
    mume_type_setstr(mume_typeof_double(), &data->d, str);
    return 1;
}

void test_types_simple(void)
{
    mume_type_t *type;
    const int len = 256;
    char str[256] = {0};
    int i = 0;
    unsigned int u = 0;
    float f = 0;
    double d = 0;
    int b = 0;
    char *s = NULL;
    type = mume_typeof_int();
    test_assert(mume_type_size(type) == sizeof(int));
    test_assert(mume_type_setstr(type, &i, "1"));
    test_assert(1 == i);
    test_assert(mume_type_getstr(type, &i, str, len));
    test_assert(0 == strcmp(str, "1"));
    test_assert(mume_type_setstr(type, &i, "-1"));
    test_assert(-1 == i);
    test_assert(mume_type_getstr(type, &i, str, len));
    test_assert(0 == strcmp(str, "-1"));
    type = mume_typeof_uint();
    test_assert(mume_type_size(type) == sizeof(unsigned int));
    test_assert(mume_type_setstr(type, &u, "1"));
    test_assert(1 == u);
    test_assert(mume_type_getstr(type, &u, str, len));
    test_assert(0 == strcmp(str, "1"));
    test_assert(mume_type_setstr(type, &u, "-1"));
    test_assert(ULONG_MAX == u);
    test_assert(mume_type_getstr(type, &u, str, len));
    test_assert(0 != strcmp(str, "-1"));
    type = mume_typeof_float();
    test_assert(mume_type_size(type) == sizeof(float));
    test_assert(mume_type_setstr(type, &f, "1.414"));
    test_assert(fabs(f - 1.414) < 0.000001);
    test_assert(mume_type_getstr(type, &f, str, len));
    test_assert(0 == strncmp(str, "1.414", 5));
    test_assert(mume_type_setstr(type, &f, "-1.414"));
    test_assert(fabs(f + 1.414) < 0.000001);
    test_assert(mume_type_getstr(type, &f, str, len));
    test_assert(0 == strncmp(str, "-1.414", 6));
    type = mume_typeof_double();
    test_assert(mume_type_size(type) == sizeof(double));
    test_assert(mume_type_setstr(type, &d, "3.1415926535"));
    test_assert(fabs(d - 3.1415926535) < 0.000001);
    test_assert(mume_type_getstr(type, &d, str, len));
    test_assert(0 == strncmp(str, "3.1415926535", 12));
    test_assert(mume_type_setstr(type, &d, "-3.1415926535"));
    test_assert(fabs(d + 3.1415926535) < 0.000001);
    test_assert(mume_type_getstr(type, &d, str, len));
    test_assert(0 == strncmp(str, "-3.1415926535", 13));
    type = mume_typeof_bool();
    test_assert(mume_type_size(type) == sizeof(int));
    test_assert(mume_type_setstr(type, &b, "true"));
    test_assert(1 == b);
    test_assert(mume_type_getstr(type, &b, str, len));
    test_assert(0 == strcmp(str, "true"));
    test_assert(mume_type_setstr(type, &b, "false"));
    test_assert(0 == b);
    test_assert(mume_type_getstr(type, &b, str, len));
    test_assert(0 == strcmp(str, "false"));
    type = mume_typeof_string();
    test_assert(NULL == s);
    test_assert(mume_type_size(type) == sizeof(char*));
    test_assert(mume_type_setstr(type, &s, "hello world"));
    test_assert(0 == strcmp(s, "hello world"));
    test_assert(mume_type_getstr(type, &s, str, len));
    test_assert(0 == strcmp(str, "hello world"));
    free(s);
}

void test_types_compound(void)
{
    static mume_user_data_key_t ukey;
    mydata_t data = {0,};
    mydata_t copy = {0,};
    mume_type_t *type;
    mume_prop_t *prop;
    int udata = 0;
    MUME_COMPOUND_CREATE(type, mydata_t, NULL, NULL, NULL, NULL);
    MUME_DIRECT_PROPERTY(mume_typeof_int(), i);
    MUME_DIRECT_PROPERTY(mume_typeof_double(), d);
    MUME_DIRECT_PROPERTY(mume_typeof_string(), s);
    MUME_SIMPLE_PROPERTY(x, _mydata_setx, NULL)
    MUME_COMPOUND_FINISH();
    mume_type_set_user_data(type, &ukey, &udata, _destruct_udata);
    test_assert(0 == udata);
    mume_type_set_user_data(type, &ukey, &udata, _destruct_udata);
    test_assert(1 == udata);
    udata = 0;
    test_assert(sizeof(data) == mume_type_size(type));
    prop = mume_type_prop(type, "i");
    test_assert(prop);
    mume_prop_setstr(prop, &data, "10");
    test_assert(10 == data.i);
    prop = mume_type_prop(type, "d");
    test_assert(prop);
    mume_prop_setstr(prop, &data, "1.23");
    test_assert(fabs(data.d - 1.23) < 0.000001);
    prop = mume_type_prop(type, "s");
    test_assert(prop);
    mume_prop_setstr(prop, &data, "hello");
    test_assert(0 == strcmp(data.s, "hello"));
    prop = mume_type_prop(type, "x");
    test_assert(prop);
    mume_prop_setstr(prop, &data, "-100");
    test_assert(-100 == data.i);
    test_assert(fabs(data.d + 100) < 0.000001);
    mume_type_objcpy(type, &copy, &data);
    test_assert(-100 == copy.i);
    test_assert(fabs(copy.d + 100) < 0.000001);
    test_assert(0 == strcmp(copy.s, "hello"));
    mume_type_objdes(type, &data);
    mume_type_objdes(type, &copy);
    mume_type_destroy(type);
    test_assert(1 == udata);
}

static void _test_container(mume_type_t *type, void *obj)
{
    const char *vals[] = {"1", "2", "3", "4", "5"};
    mume_type_t *vtype = mume_type_ctnr_vtype(type);
    void *vobj = mume_type_newobj(vtype);
    const size_t len = 64;
    char buf[64];
    void *it, *end;
    int i;
    test_assert(mume_type_is_container(type));
    test_assert(mume_type_ctnr_begin(type, obj) ==
                   mume_type_ctnr_end(type, obj));
    for (i = 0; i < COUNT_OF(vals); ++i) {
        mume_type_setstr(vtype, vobj, vals[i]);
        mume_type_ctnr_insert(type, obj, vobj);
    }
    mume_type_delobj(vtype, vobj);
    it = mume_type_ctnr_begin(type, obj);
    end = mume_type_ctnr_end(type, obj);
    for (i = 0; i < COUNT_OF(vals); ++i) {
        vobj = mume_type_ctnr_value(type, obj, it);
        mume_type_getstr(vtype, vobj, buf, len);
        test_assert(it != end);
        test_assert(0 == strncmp(
            buf, vals[i], strlen(vals[i])));
        it = mume_type_ctnr_next(type, obj, it);
    }
    test_assert(it == end);
}

void test_types_container(void)
{
    mume_type_t *type;
    mume_list_t *lst;
    mume_vector_t *vec;
    mume_oset_t *set;
    /* int */
    type = mume_type_list_create(mume_typeof_int());
    lst = (mume_list_t*)mume_type_newobj(type);
    _test_container(type, lst);
    mume_type_delobj(type, lst);
    mume_type_destroy(type);
    /* float */
    type = mume_type_list_create(mume_typeof_float());
    lst = (mume_list_t*)mume_type_newobj(type);
    _test_container(type, lst);
    mume_type_delobj(type, lst);
    mume_type_destroy(type);
    /* double */
    type = mume_type_vector_create(mume_typeof_double());
    vec = (mume_vector_t*)mume_type_newobj(type);
    test_assert(mume_vector_eltsize(vec) == sizeof(double));
    _test_container(type, vec);
    mume_type_delobj(type, vec);
    mume_type_destroy(type);
    /* string list */
    type = mume_type_list_create(mume_typeof_string());
    lst = (mume_list_t*)mume_type_newobj(type);
    _test_container(type, lst);
    mume_type_delobj(type, lst);
    mume_type_destroy(type);
    /* string vector */
    type = mume_type_vector_create(mume_typeof_string());
    vec = (mume_vector_t*)mume_type_newobj(type);
    test_assert(mume_vector_eltsize(vec) == sizeof(char*));
    _test_container(type, vec);
    mume_type_delobj(type, vec);
    mume_type_destroy(type);
    /* string oset */
    type = mume_type_oset_create(mume_typeof_string());
    set = (mume_oset_t*)mume_type_newobj(type);
    _test_container(type, set);
    mume_type_delobj(type, set);
    mume_type_destroy(type);
}

void test_types_enumeration(void)
{
    mume_type_t *type;
    int val, i, len = 256;
    char str[256] = {0};
    void *obj;

    const mume_enumitem_t items[] = {
        { "zero", 0 },
        { "one", 1 },
        { "two", 2 },
        { "three", 3 },
        { "four", 4 },
        { NULL, 0 },
    };

    type = mume_type_enum_create(items, -1);
    test_assert(mume_type_size(type) == sizeof(int));
    obj = mume_type_newobj(type);
    assert(-1 == *(int*)obj);
    mume_type_delobj(type, obj);

    for (i = 0; i < COUNT_OF(items) - 1; ++i) {
        test_assert(mume_type_setstr(type, &val, items[i].str));
        test_assert(val == items[i].val);
        test_assert(mume_type_getstr(type, &val, str, len));
        test_assert(0 == strcmp(str, items[i].str));
    }

    /* Non-exist value. */
    test_assert(!mume_type_setstr(type, &val, "five"));
    test_assert(4 == val);
    test_assert(mume_type_getstr(type, &val, str, len));
    test_assert(0 == strcmp(str, "four"));

    mume_type_destroy(type);
}
