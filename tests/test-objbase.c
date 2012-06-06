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
#include MUME_MATH_H
#include MUME_STDLIB_H
#include MUME_STDIO_H
#include MUME_STRING_H

typedef struct mytype1_s {
    int i;
    float f;
    double d;
    int b;
    char *s;
} mytype1_t;

static void _destruct_udata(void *p)
{
    *(int*)p = 1;
}

static int _mytype1_compare(const void *a, const void *b)
{
    const mytype1_t *o1 = (const mytype1_t*)a;
    const mytype1_t *o2 = (const mytype1_t*)b;
    return o1->i > o2->i ? 1 : o1->i < o2->i;
}

typedef struct mytype2_s {
    mytype1_t a;
    mume_list_t lst;
    mume_vector_t vec;
    mume_oset_t set;
} mytype2_t;

static mume_type_t* create_type1(void)
{
    mume_type_t *t;
    MUME_COMPOUND_CREATE(
        t, mytype1_t, NULL, NULL, NULL, _mytype1_compare);
    MUME_DIRECT_PROPERTY(mume_typeof_int(), i);
    MUME_DIRECT_PROPERTY(mume_typeof_float(), f);
    MUME_DIRECT_PROPERTY(mume_typeof_double(), d);
    MUME_DIRECT_PROPERTY(mume_typeof_bool(), b);
    MUME_DIRECT_PROPERTY(mume_typeof_string(), s);
    MUME_COMPOUND_FINISH();
    return t;
}

static mume_type_t* create_type2(void)
{
    mume_type_t *t, *t1, *t2;
    t1 = create_type1();
    MUME_COMPOUND_CREATE(
        t, mytype2_t, NULL, NULL, NULL, NULL);
    MUME_DIRECT_PROPERTY(t1, a);
    t2 = mume_type_list_create(t1);
    MUME_DIRECT_PROPERTY(t2, lst);
    mume_type_destroy(t2);
    t2 = mume_type_vector_create(t1);
    MUME_DIRECT_PROPERTY(t2, vec);
    mume_type_destroy(t2);
    t2 = mume_type_oset_create(t1);
    MUME_DIRECT_PROPERTY(t2, set);
    mume_type_destroy(t1);
    mume_type_destroy(t2);
    MUME_COMPOUND_FINISH();
    return t;
}

void test_objbase_common(void)
{
    static mume_user_data_key_t ukey;
    mume_objbase_t *base = mume_objbase_create();
    mume_objns_t *ns = mume_objbase_root(base);
    mume_type_t *type1 = create_type1();
    mume_type_t *type2 = create_type2();
    mume_objdesc_t *od;
    mytype1_t *obj1;
    mytype2_t *obj2;
    int udata = 0;
    /* invalid names */
    test_assert(!mume_objns_regtype(ns, "t:ype1", type1));
    test_assert(!mume_objns_regtype(ns, "t.ype1", type1));
    test_assert(!mume_objns_regtype(ns, "0type1", type1));
    test_assert(!mume_objns_regtype(ns, "", type1));
    test_assert(mume_objns_regtype(ns, "type1", type1));
    /* type already exists */
    test_assert(!mume_objns_regtype(ns, "type1", type1));
    od = mume_objns_addobj(ns, "type1", "a");
    test_assert(od);
    test_assert(mume_objns_getobj(ns, "a") == od);
    obj1 = (mytype1_t*)mume_objdesc_data(od);
    test_assert(NULL == obj1->s);
    test_assert(mume_objns_regtype(ns, "type2", type2));
    test_assert(mume_objns_regtype(ns, "_type2", type2));
    /* user data */
    mume_objdesc_set_user_data(od, &ukey, &udata, _destruct_udata);
    test_assert(0 == udata);
    mume_objdesc_set_user_data(od, &ukey, &udata, _destruct_udata);
    test_assert(1 == udata);
    udata = 0;
    /* name exists */
    od = mume_objns_addobj(ns, "type2", "a");
    test_assert(NULL == od);
    od = mume_objns_addobj(ns, "::type2", "b");
    test_assert(NULL == od);
    od = mume_objns_addobj(ns, ":type2", "b");
    test_assert(od);
    test_assert(mume_objns_getobj(ns, "b") == od);
    obj2 = (mytype2_t*)mume_objdesc_data(od);
    test_assert(NULL == obj2->a.s);
    test_assert(0 == mume_list_size(&obj2->lst));
    test_assert(0 == mume_vector_size(&obj2->vec));
    test_assert(0 == mume_oset_size(&obj2->set));
    /* name conflict with object */
    test_assert(NULL == mume_objns_getsub(ns, "a", 1));
    test_assert(NULL == mume_objns_getsub(ns, "b", 1));
    ns = mume_objns_getsub(ns, "sub1", 1);
    test_assert(ns);
    /* search object through parent namespace */
    test_assert(mume_objns_getobj(ns, "a"));
    test_assert(mume_objns_getobj(ns, "a") ==
              mume_objns_getobj(ns, ":a"));
    /* search type through parent namespace */
    test_assert(mume_objns_addobj(ns, "type1", "a"));
    test_assert(mume_objns_addobj(ns, "type2", "b"));
    test_assert(mume_objns_getobj(ns, "a") !=
              mume_objns_getobj(ns, ":a"));
    /* type not exists */
    test_assert(NULL == mume_objns_addobj(ns, "type3", "c"));
    test_assert(mume_objns_regtype(ns, "type3", type1));
    test_assert(mume_objns_addobj(ns, "type3", "c"));
    test_assert(NULL == mume_objns_addobj(
        mume_objbase_root(base), "type3", "d"));
    test_assert(NULL == mume_objns_addobj(
        mume_objbase_root(base), "sub1::type3", "d"));
    test_assert(NULL == mume_objns_addobj(
        mume_objbase_root(base), "Sub1:type3", "d"));
    test_assert(mume_objns_addobj(
        mume_objbase_root(base), "sub1:type3", "d"));
    mume_objbase_destroy(base);
    mume_type_destroy(type1);
    mume_type_destroy(type2);
    test_assert(1 == udata);
}

void test_objbase_serialize(void)
{
    const char *file = "test-base-objbase.xml";
    mume_type_t *type1 = create_type1();
    mume_type_t *type2 = create_type2();
    mume_objbase_t *base;
    mume_objns_t *ns;
    mume_objdesc_t *obj;
    mytype1_t *obj1;
    mytype2_t *obj2;
    mume_virtfs_t *vfs;
    mume_stream_t *stm;
    int i;
    mume_list_node_t *itlst;
    mume_oset_node_t *itset;
    size_t itvec;
    vfs = mume_virtfs_create(TESTS_DATA_DIR);
    test_assert(vfs);
    /* save */
    base = mume_objbase_create();
    ns = mume_objbase_root(base);
    test_assert(mume_objns_regtype(ns, "type1", type1));
    test_assert(mume_objns_regtype(ns, "type2", type2));
    obj = mume_objns_addobj(ns, "type1", "a");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    obj1->i = -1;
    obj1->f = 1.0;
    obj1->d = -2.0;
    obj1->b = 1;
    obj1->s = strdup_abort("hello obj1");
    obj = mume_objns_addobj(ns, "type2", "b");
    test_assert(obj);
    obj2 = (mytype2_t*)mume_objdesc_data(obj);
    obj2->a.i = -10;
    obj2->a.f = 10.0;
    obj2->a.d = -20.0;
    obj2->a.b = 0;
    obj2->a.s = strdup_abort("hello obj2");
    for (i = 0; i < 10; ++i) {
        char buf[32];
        mume_oset_node_t *node;
        snprintf(buf, 32, "%d", i);
        obj1 = (mytype1_t*)mume_list_data(
            mume_list_push_back(&obj2->lst, sizeof(mytype1_t)));
        obj1->i = i;
        obj1->f = i * 10.0;
        obj1->d = i * 100.0;
        obj1->b = 0;
        obj1->s = strdup_abort(buf);
        obj1 = (mytype1_t*)mume_vector_push_back(&obj2->vec);
        obj1->i = i;
        obj1->f = i * 10.0;
        obj1->d = i * 100.0;
        obj1->b = 0;
        obj1->s = strdup_abort(buf);
        node = mume_oset_newnode(sizeof(*obj1));
        obj1 = (mytype1_t*)mume_oset_data(node);
        obj1->i = i;
        obj1->f = i * 10.0;
        obj1->d = i * 100.0;
        obj1->b = 0;
        obj1->s = strdup_abort(buf);
        test_assert(mume_oset_insert(&obj2->set, node));
    }

    stm = mume_virtfs_open_write(vfs, file);
    if (stm) {
        test_assert(mume_objbase_save_xml(base, stm));
        mume_stream_close(stm);
        mume_objbase_destroy(base);
    }
    else {
        mume_warning(("file system not writable!\n"));
        mume_objbase_destroy(base);
        mume_virtfs_destroy(vfs);
        mume_type_destroy(type1);
        mume_type_destroy(type2);
        return;
    }
    /* load */
    base = mume_objbase_create();
    ns = mume_objbase_root(base);
    test_assert(mume_objns_regtype(ns, "type1", type1));
    test_assert(mume_objns_regtype(ns, "type2", type2));
    stm = mume_virtfs_open_read(vfs, file);
    test_assert(stm);
    test_assert(mume_objbase_load_xml(base, vfs, stm));
    mume_stream_close(stm);
    obj = mume_objns_getobj(ns, "a");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    test_assert(obj1->i == -1);
    test_assert(fabs(obj1->f - 1.0) < 0.000001);
    test_assert(fabs(obj1->d + 2.0) < 0.000001);
    test_assert(obj1->b == 1);
    test_assert(0 == strcmp(obj1->s, "hello obj1"));
    obj = mume_objns_getobj(ns, "b");
    test_assert(obj);
    obj2 = (mytype2_t*)mume_objdesc_data(obj);
    test_assert(obj2->a.i == -10);
    test_assert(fabs(obj2->a.f - 10.0) < 0.000001);
    test_assert(fabs(obj2->a.d + 20.0) < 0.000001);
    test_assert(obj2->a.b == 0);
    test_assert(0 == strcmp(obj2->a.s, "hello obj2"));
    test_assert(10 == mume_list_size(&obj2->lst));
    test_assert(10 == mume_vector_size(&obj2->vec));
    test_assert(10 == mume_oset_size(&obj2->set));
    itlst = mume_list_front(&obj2->lst);
    itvec = 0;
    itset = mume_oset_first(&obj2->set);
    for (i = 0; i < 10; ++i) {
        char buf[32];
        snprintf(buf, 32, "%d", i);
        obj1 = (mytype1_t*)mume_list_data(itlst);
        test_assert(obj1->i == i);
        test_assert(fabs(obj1->f - i * 10.0) < 0.000001);
        test_assert(fabs(obj1->d - i * 100.0) < 0.000001);
        test_assert(obj1->b == 0);
        test_assert(0 == strcmp(obj1->s, buf));
        obj1 = (mytype1_t*)mume_vector_at(&obj2->vec, itvec);
        test_assert(obj1->i == i);
        test_assert(fabs(obj1->f - i * 10.0) < 0.000001);
        test_assert(fabs(obj1->d - i * 100.0) < 0.000001);
        test_assert(obj1->b == 0);
        test_assert(0 == strcmp(obj1->s, buf));
        obj1 = (mytype1_t*)mume_oset_data(itset);
        test_assert(obj1->i == i);
        test_assert(fabs(obj1->f - i * 10.0) < 0.000001);
        test_assert(fabs(obj1->d - i * 100.0) < 0.000001);
        test_assert(obj1->b == 0);
        itlst = mume_list_next(itlst);
        itvec += 1;
        itset = mume_oset_next(itset);
    }
    mume_objbase_destroy(base);
    /* clear */
    test_assert(mume_virtfs_delete(vfs, file));
    mume_virtfs_destroy(vfs);
    mume_type_destroy(type1);
    mume_type_destroy(type2);
}

void test_objbase_include(void)
{
    const char *file = "test-base-objbase0.xml";
    mume_type_t *type1 = create_type1();
    mume_type_t *type2 = create_type2();
    mume_objbase_t *base;
    mume_objns_t *ns;
    mume_objdesc_t *obj;
    mytype1_t *obj1;
    mytype2_t *obj2;
    mume_virtfs_t *vfs;
    mume_stream_t *stm;
    vfs = mume_virtfs_create(TESTS_DATA_DIR);
    test_assert(vfs);
    stm = mume_virtfs_open_read(vfs, file);
    test_assert(stm);
    base = mume_objbase_create();
    ns = mume_objbase_root(base);
    test_assert(mume_objns_regtype(ns, "type1", type1));
    test_assert(mume_objns_regtype(ns, "type2", type2));
    test_assert(mume_objbase_load_xml(base, vfs, stm));
    mume_stream_close(stm);
    obj = mume_objns_getobj(ns, "obj0");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    test_assert(0 == strcmp(obj1->s, "this is obj0"));
    /* test include */
    obj = mume_objns_getobj(ns, "obj1");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    test_assert(0 == strcmp(obj1->s, "this is obj1"));
    /* test namespace */
    ns = mume_objbase_getns(base, "ns1:ns2", 0);
    test_assert(ns);
    obj = mume_objns_getobj(ns, "obj2");
    test_assert(obj);
    obj2 = (mytype2_t*)mume_objdesc_data(obj);
    test_assert(0 == strcmp(obj2->a.s, "this is obj2"));
    test_assert(1 == mume_list_size(&obj2->lst));
    test_assert(0 == mume_vector_size(&obj2->vec));
    test_assert(0 == mume_oset_size(&obj2->set));
    /* test link */
    test_assert(NULL == mume_objns_getobj(ns, "link1"));
    ns = mume_objns_getsub(mume_objbase_root(base), "ns1", 0);
    test_assert(ns);
    obj = mume_objns_getobj(ns, "link1");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    test_assert(0 == strcmp(obj1->s, "this is obj1"));
    obj = mume_objns_getobj(ns, "link3");
    test_assert(obj);
    obj1 = (mytype1_t*)mume_objdesc_data(obj);
    test_assert(0 == strcmp(obj1->s, "this is obj1"));
    mume_objbase_destroy(base);
    mume_virtfs_destroy(vfs);
    mume_type_destroy(type1);
    mume_type_destroy(type2);
}
