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
#ifndef MUME_FOUNDATION_TYPES_H
#define MUME_FOUNDATION_TYPES_H

#include "mume-common.h"

MUME_BEGIN_DECLS

enum mume_type_type_e {
    MUME_TYPE_POINTER,
    MUME_TYPE_SIMPLE,
    MUME_TYPE_COMPOUND,
    MUME_TYPE_CONTAINER
};

enum mume_prop_type_e {
    MUME_PROP_SIMPLE,
    MUME_PROP_DIRECT
};

struct mume_type_s {
    unsigned short type;
    unsigned short size;
    size_t refcount;
    mume_confcn_t *objcon;
    mume_desfcn_t *objdes;
    mume_cpyfcn_t *objcpy;
    mume_cmpfcn_t *objcmp;
    mume_user_data_t *udatas;
    void (*destroy)(void *self);
};

typedef struct mume_type_simple_s {
    mume_type_t base;
    int (*setstr)(void *type, void *obj, const char *str);
    int (*getstr)(void *type, void *obj, char *buf, size_t len);
} mume_type_simple_t;

typedef struct mume_type_compound_s {
    mume_type_t base;
    /* property must be sorted by name in alphabet order */
    mume_prop_t** props;
    size_t propc;
} mume_type_compound_t;

typedef struct mume_type_container_s {
    mume_type_t base;
    mume_type_t* value_type;
    void* (*begin)(void *type, void *obj);
    void* (*end)(void *type, void *obj);
    void* (*next)(void *type, void *obj, void *it);
    void* (*value)(void *type, void *obj, void *it);
    void (*insert)(void *type, void *obj, void *val);
    void (*erase)(void *type, void *obj, void *it);
    void (*clear)(void *type, void *obj);
} mume_type_container_t;

struct mume_prop_s {
    const char *name;
    int prop_type;
};

typedef struct mume_prop_simple_s {
    mume_prop_t base;
    int (*setstr)(void *prop, void *obj, const char *str);
    int (*getstr)(void *prop, void *obj, char *buf, size_t len);
} mume_prop_simple_t;

typedef struct mume_prop_direct_s {
    mume_prop_t base;
    mume_type_t *type;
    size_t offset;
} mume_prop_direct_t;

mume_public int _mume_type_string_compare(
    const void *a, const void *b);

/*========================================
 * [function]
 *  get the standard simple types, all these
 *  types will initialize the object to zero.
 * [notice]
 *  user should not destroy the returned value.
 *========================================*/
mume_public mume_type_t* mume_typeof_int(void);
mume_public mume_type_t* mume_typeof_uint(void);
mume_public mume_type_t* mume_typeof_float(void);
mume_public mume_type_t* mume_typeof_double(void);
mume_public mume_type_t* mume_typeof_bool(void);
mume_public mume_type_t* mume_typeof_string(void);
mume_public mume_type_t* mume_typeof_pointer(void);

#define mume_type_is_simple(_type) \
    (MUME_TYPE_SIMPLE == (_type)->type)
#define mume_type_is_compound(_type) \
    (MUME_TYPE_COMPOUND == (_type)->type)
#define mume_type_is_container(_type) \
    (MUME_TYPE_CONTAINER == (_type)->type)
#define mume_type_size(_type) ((const size_t)(_type)->size)

/*========================================
 * [function]
 *  construct/destruct/copy an object of the specified type.
 * [return]
 *  the constructed/destructed object
 *========================================*/
mume_public void* mume_type_objcon(
    mume_type_t *type, void *obj);
mume_public void* mume_type_objdes(
    mume_type_t *type, void *obj);
mume_public void mume_type_objcpy(
    mume_type_t *type, void *to, const void *from);

#define mume_type_newobj(_type) \
    mume_type_objcon(_type, malloc_abort((_type)->size))
#define mume_type_delobj(_type, _obj) \
    free(mume_type_objdes(_type, _obj))

/*========================================
 * [function]
 *  Traverse a object of the specified type,
 *  pass each type and sub types with its
 *  corresponding data to the callback function.
 * [parameters]
 *  obj : the object to traverse
 *  p : user defined parameter
 *  fcn : the callback function
 *========================================*/
mume_public void mume_type_traverse(
    mume_type_t *type, void *obj, void *p,
    void (*fcn)(mume_type_t *t, void *d, void *p));

/*========================================
 * [function]
 *  set/get the string representation of the
 *  object, the type should be MUME_TYPE_SIMPLE.
 * [return]
 *  nonzero : success
 *  zero : fail
 *========================================*/
static inline int mume_type_setstr(
    mume_type_t *type, void *obj, const char *str)
{
    if (mume_type_is_simple(type)) {
        return ((mume_type_simple_t*)
                (type))->setstr(type, obj, str);
    }
    return 0;
}

static inline int mume_type_getstr(
    mume_type_t *type, void *obj, char *buf, size_t len)
{
    if (mume_type_is_simple(type)) {
        return ((mume_type_simple_t*)
                (type))->getstr(type, obj, buf, len);
    }
    return 0;
}

/*========================================
 * [function]
 *  mume_prop_t compare function for qsort and bsearch.
 *========================================*/
mume_public int mume_prop_compare(
    const void *p1, const void *p2);

/*========================================
 * [function]
 *  get the property list/count of a compound type.
 * [notice]
 *  properties are sorted by name in alphabet order
 *========================================*/
static inline mume_prop_t** mume_type_props(mume_type_t *type)
{
    if (mume_type_is_compound(type))
        return ((mume_type_compound_t*)(type))->props;
    return 0;
}

static inline size_t mume_type_propc(mume_type_t *type)
{
    if (mume_type_is_compound(type))
        return ((mume_type_compound_t*)(type))->propc;
    return 0;
}

/*========================================
 * [function]
 *  find a property information by name.
 *  the type should be MUME_TYPE_COMPOUND.
 * [return]
 *  the property or NULL for fail
 *========================================*/
mume_public mume_prop_t** mume_find_prop(
    mume_prop_t **type, int count, const char *name);

mume_public mume_prop_t* mume_type_prop(
    mume_type_t *type, const char *name);

/*========================================
 * [function]
 *  get the value type of the container type.
 * [return]
 *  the element type or NULL for fail
 *========================================*/
static inline mume_type_t* mume_type_ctnr_vtype(
    mume_type_t *type)
{
    if (mume_type_is_container(type))
        return ((mume_type_container_t*)type)->value_type;
    return 0;
}

/*========================================
 * [function]
 *  iterator functions for container type.
 *========================================*/
static inline void* mume_type_ctnr_begin(
    mume_type_t *type, void *obj)
{
    if (mume_type_is_container(type))
        return ((mume_type_container_t*)type)->begin(type, obj);
    return 0;
}

static inline void* mume_type_ctnr_end(
    mume_type_t *type, void *obj)
{
    if (mume_type_is_container(type))
        return ((mume_type_container_t*)type)->end(type, obj);
    return 0;
}

static inline void* mume_type_ctnr_next(
    mume_type_t *type, void *obj, void *it)
{
    if (mume_type_is_container(type))
        return ((mume_type_container_t*)type)->next(type, obj, it);
    return 0;
}

/*========================================
 * [function]
 *  insert/erase/clear element of the container.
 *========================================*/
static inline void mume_type_ctnr_insert(
    mume_type_t *type, void *obj, void *val)
{
    if (mume_type_is_container(type))
        ((mume_type_container_t*)type)->insert(type, obj, val);
}

static inline void mume_type_ctnr_erase(
    mume_type_t *type, void *obj, void *it)
{
    if (mume_type_is_container(type))
        ((mume_type_container_t*)type)->erase(type, obj, it);
}

static inline void mume_type_ctnr_clear(
    mume_type_t *type, void *obj)
{
    if (mume_type_is_container(type))
        ((mume_type_container_t*)type)->clear(type, obj);
}

/*========================================
 * [function]
 *  get the element value of the iterator.
 *========================================*/
static inline void* mume_type_ctnr_value(
    mume_type_t *type, void *obj, void *it)
{
    if (mume_type_is_container(type))
        return ((mume_type_container_t*)type)->value(type, obj, it);
    return 0;
}

static inline mume_type_t* mume_type_reference(
    mume_type_t *type)
{
    ++type->refcount;
    return type;
}

mume_public void mume_type_set_user_data(
    mume_type_t *type, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy);

mume_public void* mume_type_get_user_data(
    mume_type_t *type, const mume_user_data_key_t *key);

mume_public void mume_type_destroy(mume_type_t *type);

#define mume_prop_is_simple(_prop) \
    (MUME_PROP_SIMPLE == (_prop)->prop_type)
#define mume_prop_is_direct(_prop) \
    (MUME_PROP_DIRECT == (_prop)->prop_type)

/*========================================
 * [function]
 *  get the type/offset of a direct property.
 *========================================*/
static inline mume_type_t* mume_prop_type(mume_prop_t *prop)
{
    if (mume_prop_is_direct(prop))
        return ((mume_prop_direct_t*)prop)->type;
    return 0;
}

static inline size_t mume_prop_offset(mume_prop_t *prop)
{
    if (mume_prop_is_direct(prop))
        return ((mume_prop_direct_t*)prop)->offset;
    return 0;
}

/*========================================
 * [function]
 *  set/get the string representation of the
 *  property.
 * [return]
 *  nonzero : success
 *  zero : fail
 *========================================*/
mume_public int mume_prop_setstr(
    mume_prop_t *prop, void *obj, const char *str);

mume_public int mume_prop_getstr(
    mume_prop_t *prop, void *obj, char *buf, size_t len);

/*========================================
 * [function]
 *  functions to help create compound types.
 * [notice]
 *  user should use the macros below instead of
 *  use these functions directly.
 *========================================*/
mume_public mume_type_t* mume_type_compound_create(
    size_t size, mume_confcn_t *con, mume_desfcn_t *des,
    mume_cpyfcn_t *cpy, mume_cmpfcn_t *cmp);

mume_public void mume_type_compound_direct_prop(
    mume_type_t *type, const char *name,
    mume_type_t *prop_type, size_t prop_offset);

mume_public void mume_type_compound_simple_prop(
    mume_type_t *type, const char *name,
    int (*setstr)(void*, void*, const char*),
    int (*getstr)(void*, void*, char*, size_t));

mume_public void mume_type_compound_finish(mume_type_t *type);

#define MUME_PROPERTY_BEGIN(_TYPE, _STRUCT) \
    do { \
    mume_type_t *_cur_type = _TYPE; \
    _STRUCT *_cur_struct = NULL;

#define MUME_PROPERTY_END() \
    } while (0)

#define MUME_COMPOUND_CREATE( \
    _VAR, _STRUCT, _CONNEW, _DESDEL, _OBJCPY, _OBJCMP)  \
    _VAR = mume_type_compound_create( \
        sizeof(_STRUCT), _CONNEW, _DESDEL, _OBJCPY, _OBJCMP); \
    MUME_PROPERTY_BEGIN(_VAR, _STRUCT)

#define MUME_COMPOUND_FINISH() \
    mume_type_compound_finish(_cur_type); \
    MUME_PROPERTY_END()

#define MUME_DIRECT_PROPERTY(_TYPE, _MEMBER) \
    mume_type_compound_direct_prop( \
        _cur_type, #_MEMBER, _TYPE, \
        (size_t)&_cur_struct->_MEMBER);

#define MUME_SIMPLE_PROPERTY(_NAME, _SETSTR, _GETSTR) \
    mume_type_compound_simple_prop( \
        _cur_type, #_NAME, _SETSTR, _GETSTR);

/*========================================
 * [function]
 *  functions to help create container types
 *  for mume_list_t, mume_vector_t, mume_oset_t.
 * [parameters]
 *  value_type : the type of container elements
 *========================================*/
mume_public mume_type_t* mume_type_list_create(
    mume_type_t *value_type);

mume_public mume_type_t* mume_type_vector_create(
    mume_type_t *value_type);

mume_public mume_type_t* mume_type_oset_create(
    mume_type_t *value_type);

/*========================================
 * [function]
 *  functions to help create pointer types,
 *  currently just support destruct the object
 *  of that pointer.
 * [parameters]
 *  des : the object destructor
 *========================================*/
mume_public mume_type_t* mume_type_pointer_create(
    mume_desfcn_t *des);

/*========================================
 * [function]
 *  functions to help create enumeration types,
 * [parameters]
 *  items : string to enum value map
 *  init : the initial value for the object
 * [notice]
 *  the <map> must be ended with item {NULL, 0}.
 *========================================*/
typedef struct mume_enumitem_s {
    const char *str;
    int val;
} mume_enumitem_t;

mume_public mume_type_t* mume_type_enum_create(
    const mume_enumitem_t items[], int defval);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_TYPES_H */
