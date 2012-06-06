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
#include "mume-types.h"
#include "mume-config.h"
#include "mume-debug.h"
#include "mume-list.h"
#include "mume-memory.h"
#include "mume-object.h"
#include "mume-oset.h"
#include "mume-string.h"
#include "mume-userdata.h"
#include "mume-vector.h"
#include MUME_ASSERT_H
#include MUME_MATH_H
#include MUME_STDIO_H
#include MUME_STDLIB_H
#include MUME_STRING_H

static void _type_object_nocopy(void *t, const void *f, void *p)
{
    /* dummy function for types that do not support copy */
}

static void _type_zero_construct(void *obj, void *p)
{
    memset(obj, 0, mume_type_size((mume_type_t*)p));
}

static int _type_int_compare(const void *a, const void *b)
{
    const int v1 = *(int*)a;
    const int v2 = *(int*)b;
    return (v1 < v2) ? -1 : (v1 > v2);
}

static int _type_int_setstr(
    void *type, void *obj, const char *str)
{
    *(int*)obj = strtol(str, NULL, 10);
    return 1;
}

static int _type_int_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    return snprintf(buf, len, "%d", *(int*)obj);
}

int _mume_type_string_compare(const void *a, const void *b)
{
    const char *p1 = *(const char**)a;
    const char *p2 = *(const char**)b;
    if (p1 && p2)
        return strcmp(p1, p2);
    if (p1 && NULL == p2)
        return 1;
    if (NULL == p1 && p2)
        return -1;
    return 0;
}

mume_type_t* mume_typeof_int(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(int),
            1,
            _type_zero_construct,
            NULL,
            NULL,
            _type_int_compare,
        },
        _type_int_setstr,
        _type_int_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

static int _type_uint_compare(const void *a, const void *b)
{
    const unsigned int v1 = *(unsigned int*)a;
    const unsigned int v2 = *(unsigned int*)b;
    return (v1 < v2) ? -1 : (v1 > v2);
}

static int _type_uint_setstr(
    void *type, void *obj, const char *str)
{
    *(unsigned int*)obj = strtoul(str, NULL, 10);
    return 1;
}

static int _type_uint_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    return snprintf(buf, len, "%u", *(unsigned int*)obj);
}

mume_type_t* mume_typeof_uint(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(unsigned int),
            1,
            _type_zero_construct,
            NULL,
            NULL,
            _type_uint_compare,
        },
        _type_uint_setstr,
        _type_uint_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

static int _type_float_compare(const void *a, const void *b)
{
    const float v1 = *(float*)a;
    const float v2 = *(float*)b;
    if (v1 < v2)
        return fabs(v2 - v1) < 0.000001 ? 0 : -1;
    return fabs(v1 - v2) < 0.000001 ? 0 : 1;
}

static int _type_float_setstr(
    void *type, void *obj, const char *str)
{
    *(float*)obj = (float)strtod(str, NULL);
    return 1;
}

static int _type_float_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    return snprintf(buf, len, "%.6f", *(float*)obj);
}

mume_type_t* mume_typeof_float(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(float),
            1,
            _type_zero_construct,
            NULL,
            NULL,
            _type_float_compare,
        },
        _type_float_setstr,
        _type_float_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

static int _type_double_compare(const void *a, const void *b)
{
    const double v1 = *(double*)a;
    const double v2 = *(double*)b;
    if (v1 < v2)
        return fabs(v2 - v1) < 0.000001 ? 0 : -1;
    return fabs(v1 - v2) < 0.000001 ? 0 : 1;
}

static int _type_double_setstr(
    void *type, void *obj, const char *str)
{
    *(double*)obj = strtod(str, NULL);
    return 1;
}

static int _type_double_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    return snprintf(buf, len, "%.12f", *(double*)obj);
}

mume_type_t* mume_typeof_double(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(double),
            1,
            _type_zero_construct,
            NULL,
            NULL,
            _type_double_compare,
        },
        _type_double_setstr,
        _type_double_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

static int _type_bool_setstr(
    void *type, void *obj, const char *str)
{
    if (0 == strcmp(str, "true")) {
        *(int*)obj = 1;
        return 1;
    }
    else if (0 == strcmp(str, "false")) {
        *(int*)obj = 0;
        return 1;
    }

    mume_warning(("invalid bool value: %s\n", str));
    return 0;
}

static int _type_bool_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    if (*(int*)obj) {
        strncpy(buf, "true", len);
        return 4;
    }
    strncpy(buf, "false", len);
    return 5;
}

mume_type_t* mume_typeof_bool(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(int),
            1,
            _type_zero_construct,
            NULL,
            NULL,
            _type_int_compare,
        },
        _type_bool_setstr,
        _type_bool_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

static void _type_string_destruct(void *obj, void *p)
{
    free(*(char**)obj);
    *(char**)obj = NULL;
}

static void _type_string_copy(void *t, const void *f, void *p)
{
    free(*(char**)t);
    if (*(char**)f)
        *(char**)t = strdup_abort(*(char**)f);
    else
        *(char**)t = NULL;
}

static int _type_string_setstr(
    void *type, void *obj, const char *str)
{
    free(*(char**)obj);
    *(char**)obj = malloc_abort(strlen(str) + 1);
    strcpy(*(char**)obj, str);
    return 1;
}

static int _type_string_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    if (*(char**)obj)
        return snprintf(buf, len, "%s", *(char**)obj);
    if (len > 0)
        buf[0] = '\0';
    return 0;
}

mume_type_t* mume_typeof_string(void)
{
    static mume_type_simple_t type = {
        {
            MUME_TYPE_SIMPLE,
            sizeof(char*),
            1,
            _type_zero_construct,
            _type_string_destruct,
            _type_string_copy,
            _mume_type_string_compare,
        },
        _type_string_setstr,
        _type_string_getstr,
    };
    assert(type.base.refcount > 0);
    return (mume_type_t*)&type;
}

mume_type_t* mume_typeof_pointer(void)
{
    static mume_type_t type = {
        MUME_TYPE_POINTER,
        sizeof(void*),
        1,
        _type_zero_construct,
        NULL,
        NULL,
        NULL,
    };
    assert(type.refcount > 0);
    return &type;
}

void* mume_type_objcon(mume_type_t *type, void *obj)
{
    if (type->objcon) {
        type->objcon(obj, type);
    }
    else if (mume_type_is_compound(type)) {
        size_t i;
        mume_type_compound_t *c;
        c = (mume_type_compound_t*)type;
        for (i = 0; i < c->propc; ++i) {
            if (mume_prop_is_direct(c->props[i])) {
                mume_type_objcon(
                    mume_prop_type(c->props[i]),
                    (char*)obj + mume_prop_offset(c->props[i]));
            }
        }
    }
    return obj;
}

void* mume_type_objdes(mume_type_t *type, void *obj)
{
    if (type->objdes) {
        type->objdes(obj, type);
    }
    else if (mume_type_is_compound(type)) {
        size_t i;
        mume_type_compound_t *c;
        c = (mume_type_compound_t*)type;
        for (i = 0; i < c->propc; ++i) {
            if (mume_prop_is_direct(c->props[i])) {
                mume_type_objdes(
                    mume_prop_type(c->props[i]),
                    (char*)obj + mume_prop_offset(c->props[i]));
            }
        }
    }
    return obj;
}

void mume_type_objcpy(
    mume_type_t *type, void *to, const void *from)
{
    if (type->objcpy) {
        type->objcpy(to, from, type);
    }
    else if (mume_type_is_compound(type)) {
        size_t i, off;
        mume_type_compound_t *c;
        c = (mume_type_compound_t*)type;
        for (i = 0; i < c->propc; ++i) {
            if (mume_prop_is_direct(c->props[i])) {
                off = mume_prop_offset(c->props[i]);
                mume_type_objcpy(
                    mume_prop_type(c->props[i]),
                    (char*)to + off, (char*)from + off);
            }
        }
    }
    else if (mume_type_is_container(type)) {
        void *c, *e, *v;
        c = mume_type_ctnr_begin(type, (void*)from);
        e = mume_type_ctnr_end(type, (void*)from);
        mume_type_ctnr_clear(type, to);
        while (c != e) {
            v = mume_type_ctnr_value(type, (void*)from, c);
            mume_type_ctnr_insert(type, to, v);
            c = mume_type_ctnr_next(type, (void*)from, c);
        }
    }
    else {
        memcpy(to, from, type->size);
    }
}

void mume_type_traverse(
    mume_type_t *type, void *obj, void *p,
    void (*fcn)(mume_type_t *t, void *d, void *p))
{
    fcn(type, obj, p);
    if (mume_type_is_compound(type)) {
        size_t i, off;
        mume_type_compound_t *c;
        c = (mume_type_compound_t*)type;
        for (i = 0; i < c->propc; ++i) {
            if (mume_prop_is_direct(c->props[i])) {
                off = mume_prop_offset(c->props[i]);
                mume_type_traverse(
                    mume_prop_type(c->props[i]),
                    (char*)obj + off, p, fcn);
            }
        }
    }
}

int mume_prop_compare(const void *p1, const void *p2)
{
    return strcmp(**(const char***)p1, **(const char***)p2);
}

mume_prop_t** mume_find_prop(
    mume_prop_t **props, int count, const char *name)
{
    const char **dummy = &name;

    return bsearch(
        &dummy, props, count,
        sizeof(mume_prop_t*),
        &mume_prop_compare);
}

mume_prop_t* mume_type_prop(
    mume_type_t *type, const char *name)
{
    if (mume_type_is_compound(type)) {
        mume_prop_t **props = mume_find_prop(
            mume_type_props(type),
            mume_type_propc(type), name);

        if (props)
            return *props;
    }

    return NULL;
}

void mume_type_set_user_data(
    mume_type_t *type, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy)
{
    if (NULL == type->udatas)
        type->udatas = mume_user_data_new();

    mume_user_data_set(type->udatas, key, data, destroy);
}

void* mume_type_get_user_data(
    mume_type_t *type, const mume_user_data_key_t *key)
{
    if (type->udatas)
        return mume_user_data_get(type->udatas, key);

    return NULL;
}

void mume_type_destroy(mume_type_t *type)
{
    assert(type->refcount > 0);
    if (0 == --type->refcount) {
        mume_user_data_delete(type->udatas);

        if (type->destroy)
            type->destroy(type);
    }
}

int mume_prop_setstr(mume_prop_t *prop,
                      void *obj, const char *str)
{
    switch (prop->prop_type) {
    case MUME_PROP_SIMPLE:
        return ((mume_prop_simple_t*)
                (prop))->setstr(prop, obj, str);
    case MUME_PROP_DIRECT:
        return mume_type_setstr(
            mume_prop_type(prop),
            (char*)obj + mume_prop_offset(prop), str);
    }
    return 0;
}

int mume_prop_getstr(mume_prop_t *prop,
                      void *obj, char *buf, size_t len)
{
    switch (prop->prop_type) {
    case MUME_PROP_SIMPLE:
        return ((mume_prop_simple_t*)
                (prop))->getstr(prop, obj, buf, len);
    case MUME_PROP_DIRECT:
        return mume_type_getstr(
            mume_prop_type(prop),
            (char*)obj + mume_prop_offset(prop), buf, len);
    }
    return 0;
}

static void _type_compound_destroy(void *self)
{
    mume_type_compound_t *comp = self;
    while (comp->propc--) {
        if (mume_prop_is_direct(comp->props[comp->propc])) {
            mume_type_destroy(
                mume_prop_type(comp->props[comp->propc]));
        }

        free(comp->props[comp->propc]);
    }

    free(comp->props);
    free(comp);
}

static void* _type_compound_ctor(
    mume_type_compound_t *type, size_t size,
    mume_confcn_t *con, mume_desfcn_t *des,
    mume_cpyfcn_t *cpy, mume_cmpfcn_t *cmp)
{
    type->base.type = MUME_TYPE_COMPOUND;
    type->base.size = (unsigned short)size;
    type->base.refcount = 1;
    type->base.objcon = con;
    type->base.objdes = des;
    type->base.objcpy = cpy;
    type->base.objcmp = cmp;
    type->base.udatas = NULL;
    type->base.destroy = _type_compound_destroy;
    type->props = (mume_prop_t**)mume_vector_new(
        sizeof(mume_prop_t*), NULL, NULL);
    type->propc = 0;
    return type;
}

mume_type_t* mume_type_compound_create(
    size_t size, mume_confcn_t *con, mume_desfcn_t *des,
    mume_cpyfcn_t *cpy, mume_cmpfcn_t *cmp)
{
    mume_type_compound_t *comp;
    comp = malloc_abort(sizeof(mume_type_compound_t));
    return _type_compound_ctor(comp, size, con, des, cpy, cmp);
}

void mume_type_compound_direct_prop(
    mume_type_t *type, const char *name,
    mume_type_t *prop_type, size_t prop_offset)
{
    mume_type_compound_t *comp;
    mume_prop_direct_t *prop;
    prop = malloc_abort(sizeof(*prop));
    prop->base.name = name;
    prop->base.prop_type = MUME_PROP_DIRECT;
    prop->type = mume_type_reference(prop_type);
    prop->offset = prop_offset;
    comp = (mume_type_compound_t*)type;
    *(void**)mume_vector_push_back(
        (mume_vector_t*)comp->props) = (mume_prop_t*)prop;
}

void mume_type_compound_simple_prop(
    mume_type_t *type, const char *name,
    int (*setstr)(void*, void*, const char*),
    int (*getstr)(void*, void*, char*, size_t))
{
    mume_type_compound_t *comp;
    mume_prop_simple_t *prop;
    prop = malloc_abort(sizeof(*prop));
    prop->base.name = name;
    prop->base.prop_type = MUME_PROP_SIMPLE;
    prop->setstr = setstr;
    prop->getstr = getstr;
    comp = (mume_type_compound_t*)type;
    *(mume_prop_t**)mume_vector_push_back(
        (mume_vector_t*)comp->props) = (mume_prop_t*)prop;
}

void mume_type_compound_finish(mume_type_t *type)
{
    /* finish add properties, sort the properties */
    mume_type_compound_t *comp;
    mume_vector_t *vect;
    comp = (mume_type_compound_t*)type;
    vect = (mume_vector_t*)comp->props;
    qsort(mume_vector_front(vect), mume_vector_size(vect),
          sizeof(mume_prop_t*), &mume_prop_compare);
    comp->propc = mume_vector_size(vect);
    comp->props = malloc_abort(
        sizeof(mume_prop_t*) * comp->propc);
    memcpy(comp->props, mume_vector_front(vect),
           sizeof(mume_prop_t*) * comp->propc);
    mume_vector_delete(vect);
}

static void _type_container_destroy(void *self)
{
    mume_type_container_t *type = self;
    mume_type_destroy(type->value_type);
    free(self);
}

static void _type_container_objdes(void *obj, void *p)
{
    mume_type_objdes(p, obj);
}

static void* _type_container_zeroit(void *type, void *obj)
{
    return 0;
}

static void _type_list_construct(void *obj, void *p)
{
    mume_type_container_t *ctnr = p;
    mume_list_ctor(obj, _type_container_objdes, ctnr->value_type);
}

static void _type_list_destruct(void *obj, void *p)
{
    mume_list_dtor(obj);
}

static void* _type_list_begin(void *type, void *obj)
{
    return mume_list_front((mume_list_t*)obj);
}

static void* _type_list_next(void *type, void *obj, void *it)
{
    return mume_list_next((mume_list_node_t*)it);
}

static void* _type_list_value(void *type, void *obj, void *it)
{
    return mume_list_data(it);
}

static void _type_list_insert(void *type, void *obj, void *val)
{
    mume_type_container_t *ctnr = type;
    mume_list_node_t *node;
    node = mume_list_push_back(
        obj, mume_type_size(ctnr->value_type));
    mume_type_objcon(
        ctnr->value_type, mume_list_data(node));
    mume_type_objcpy(
        ctnr->value_type, mume_list_data(node), val);
}

static void _type_list_erase(void *type, void *obj, void *it)
{
    mume_list_erase(obj, it);
}

static void _type_list_clear(void *type, void *obj)
{
    mume_list_clear(obj);
}

mume_type_t* mume_type_list_create(mume_type_t *value_type)
{
    mume_type_container_t *type;
    assert(mume_type_size(value_type) > 0);
    type = malloc_abort(sizeof(mume_type_container_t));
    type->base.type = MUME_TYPE_CONTAINER;
    type->base.size = sizeof(mume_list_t);
    type->base.refcount = 1;
    type->base.objcon = _type_list_construct;
    type->base.objdes = _type_list_destruct;
    type->base.objcpy = NULL;
    type->base.objcmp = NULL;
    type->base.udatas = NULL;
    type->base.destroy = _type_container_destroy;
    type->value_type = mume_type_reference(value_type);
    type->begin = _type_list_begin;
    type->end = _type_container_zeroit;
    type->next = _type_list_next;
    type->value = _type_list_value;
    type->insert = _type_list_insert;
    type->erase = _type_list_erase;
    type->clear = _type_list_clear;
    return (mume_type_t*)type;
}

static void _type_vector_construct(void *obj, void *p)
{
    mume_type_container_t *ctnr = p;
    mume_vector_ctor(
        obj, mume_type_size(ctnr->value_type),
        _type_container_objdes, ctnr->value_type);
}

static void _type_vector_destruct(void *obj, void *p)
{
    mume_vector_dtor(obj);
}

static void* _type_vector_end(void *type, void *obj)
{
    return (void*)(mume_vector_size((mume_vector_t*)obj));
}

static void* _type_vector_next(void *type, void *obj, void *it)
{
    return (void*)((size_t)it + 1);
}

static void* _type_vector_value(void *type, void *obj, void *it)
{
    return mume_vector_at((mume_vector_t*)obj, (size_t)it);
}

static void _type_vector_insert(void *type, void *obj, void *val)
{
    mume_type_container_t *ctnr = type;
    void *data = mume_vector_push_back(obj);
    mume_type_objcon(ctnr->value_type, data);
    mume_type_objcpy(ctnr->value_type, data, val);
}

static void _type_vector_erase(void *type, void *obj, void *it)
{
    mume_vector_erase(obj, (size_t)it, 1);
}

static void _type_vector_clear(void *type, void *obj)
{
    mume_vector_clear(obj);
}

mume_type_t* mume_type_vector_create(mume_type_t *value_type)
{
    mume_type_container_t *type;
    assert(mume_type_size(value_type) > 0);
    type = malloc_abort(sizeof(mume_type_container_t));
    type->base.type = MUME_TYPE_CONTAINER;
    type->base.size = sizeof(mume_vector_t);
    type->base.refcount = 1;
    type->base.objcon = _type_vector_construct;
    type->base.objdes = _type_vector_destruct;
    type->base.objcpy = NULL;
    type->base.objcmp = NULL;
    type->base.udatas = NULL;
    type->base.destroy = _type_container_destroy;
    type->value_type = mume_type_reference(value_type);
    type->begin = _type_container_zeroit;
    type->end = _type_vector_end;
    type->next = _type_vector_next;
    type->value = _type_vector_value;
    type->insert = _type_vector_insert;
    type->erase = _type_vector_erase;
    type->clear = _type_vector_clear;
    return (mume_type_t*)type;
}

static void _type_oset_construct(void *obj, void *p)
{
    mume_type_container_t *ctnr = p;
    mume_oset_ctor(
        obj, ctnr->value_type->objcmp,
        _type_container_objdes, ctnr->value_type);
}

static void _type_oset_destruct(void *obj, void *p)
{
    mume_oset_dtor(obj);
}

static void* _type_oset_begin(void *type, void *obj)
{
    return mume_oset_first(obj);
}

static void* _type_oset_next(void *type, void *obj, void *it)
{
    return mume_oset_next(it);
}

static void* _type_oset_value(void *type, void *obj, void *it)
{
    return mume_oset_data(it);
}

static void _type_oset_insert(void *type, void *obj, void *val)
{
    mume_type_container_t *ctnr = type;
    mume_oset_node_t *node;
    node = mume_oset_newnode(
        mume_type_size(ctnr->value_type));
    mume_type_objcon(
        ctnr->value_type, mume_oset_data(node));
    mume_type_objcpy(
        ctnr->value_type, mume_oset_data(node), val);
    if (!mume_oset_insert(obj, node)) {
        mume_type_objdes(
            ctnr->value_type, mume_oset_data(node));
        mume_oset_delnode(node);
    }
}

static void _type_oset_erase(void *type, void *obj, void *it)
{
    mume_oset_erase(obj, it);
}

static void _type_oset_clear(void *type, void *obj)
{
    mume_oset_clear(obj);
}

mume_type_t* mume_type_oset_create(mume_type_t *value_type)
{
    mume_type_container_t *type;
    assert(value_type->objcmp &&
           mume_type_size(value_type) > 0);
    type = malloc_abort(sizeof(mume_type_container_t));
    type->base.type = MUME_TYPE_CONTAINER;
    type->base.size = sizeof(mume_oset_t);
    type->base.refcount = 1;
    type->base.objcon = _type_oset_construct;
    type->base.objdes = _type_oset_destruct;
    type->base.objcpy = NULL;
    type->base.objcmp = NULL;
    type->base.udatas = NULL;
    type->base.destroy = _type_container_destroy;
    type->value_type = mume_type_reference(value_type);
    type->begin = _type_oset_begin;
    type->end = _type_container_zeroit;
    type->next = _type_oset_next;
    type->value = _type_oset_value;
    type->insert = _type_oset_insert;
    type->erase = _type_oset_erase;
    type->clear = _type_oset_clear;
    return (mume_type_t*)type;
}

mume_type_t* mume_type_pointer_create(mume_desfcn_t *des)
{
    mume_type_t *type;
    type = malloc_abort(sizeof(*type));
    type->type = MUME_TYPE_POINTER,
    type->size = sizeof(void*);
    type->refcount = 1;
    type->objcon = _type_zero_construct;
    type->objdes = des;
    type->objcpy = _type_object_nocopy;
    type->objcmp = NULL;
    type->udatas = NULL;
    type->destroy = free;
    return type;
}

typedef struct _type_enumeration_s {
    mume_type_simple_t base;
    const mume_enumitem_t *items;
    int defval;
} _type_enumeration_t;

static void _type_enumeration_construct(void *obj, void *p)
{
    _type_enumeration_t *type = p;
    *(int*)obj = type->defval;
}

static int _type_enumeration_setstr(
    void *type, void *obj, const char *str)
{
    const mume_enumitem_t *items;
    items = ((_type_enumeration_t*)type)->items;
    while (items->str) {
        if (0 == strcmp(items->str, str)) {
            *(int*)obj = items->val;
            return 1;
        }
        ++items;
    }

    return 0;
}

static int _type_enumeration_getstr(
    void *type, void *obj, char *buf, size_t len)
{
    const mume_enumitem_t *items;
    items = ((_type_enumeration_t*)type)->items;
    while (items->str) {
        if (*(int*)obj == items->val) {
            int r = strcpy_c(buf, len, items->str);
            if (r < len)
                buf[r++] = '\0';

            return r;
        }

        ++items;
    }

    return 0;
}

mume_type_t* mume_type_enum_create(
    const mume_enumitem_t items[], int defval)
{
    _type_enumeration_t *type;
    type = malloc_abort(sizeof(*type));
    type->base.base.type = MUME_TYPE_SIMPLE,
    type->base.base.size = sizeof(int);
    type->base.base.refcount = 1;
    type->base.base.objcon = _type_enumeration_construct;
    type->base.base.objdes = NULL;
    type->base.base.objcpy = NULL;
    type->base.base.objcmp = NULL;
    type->base.base.udatas = NULL;
    type->base.base.destroy = free;
    type->base.setstr = _type_enumeration_setstr;
    type->base.getstr = _type_enumeration_getstr;
    type->items = items;
    type->defval = defval;
    return (mume_type_t*)type;
}
