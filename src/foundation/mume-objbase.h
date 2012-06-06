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
#ifndef MUME_FOUNDATION_OBJBASE_H
#define MUME_FOUNDATION_OBJBASE_H

#include "mume-common.h"

/*========================================
 * [Description]
 *  object base is mainly for use in object
 *  serialization. user need to provide type
 *  description for the serializable object,
 *  the object base then use the type description
 *  to do the serialization.
 * [Convention]
 *  1: the name of object (and type) has the
 *     same rule as C variable.
 *  2: the name of object and sub namespace should
 *     be unique in the same namespace (the same as
 *     C++). the type and object may have the same
 *     name (opposed to C++) in the same namespace.
 *  3: the namespace delimiter is ':' (opposed
 *     to C++'s "::").
 *  4: the namespace search rule is the same as C++.
 *========================================*/

MUME_BEGIN_DECLS

struct mume_objns_s {
    const char *name;
    mume_objns_t *pnt;
    mume_oset_t *subs;
    mume_oset_t *types;
    mume_oset_t *objs;
    mume_oset_t *lnks;
};

struct mume_objtype_s {
    const char *name;
    mume_type_t *type;
    size_t refcount;
};

struct mume_objdesc_s {
    const char *name;
    mume_objtype_t *type;
    mume_user_data_t *udatas;
};

struct mume_objbase_s {
    mume_objns_t root;
};

mume_public mume_objbase_t* mume_objbase_construct(
    mume_objbase_t *base);
mume_public mume_objbase_t* mume_objbase_destruct(
    mume_objbase_t *base);

#define mume_objbase_create() \
    mume_objbase_construct((mume_objbase_t*)( \
        malloc_abort(sizeof(mume_objbase_t))))
#define mume_objbase_destroy(_base) \
    free(mume_objbase_destruct(_base))
#define mume_objbase_root(_base) \
    ((mume_objns_t*)&(_base)->root)
#define mume_objns_parent(_ns) \
    ((mume_objns_t*)(_ns)->pnt)

#define mume_objdesc_name(_obj) \
    ((const char*)(_obj)->name)
#define mume_objdesc_type_name(_obj) \
    ((const char*)(_obj)->type->name)
#define mume_objdesc_type(_obj) \
    ((mume_type_t*)(_obj)->type->type)
#define mume_objdesc_data(_obj) \
    EXTRA_OF(mume_objdesc_t, _obj)

static inline void* mume_objdesc_cast(
    mume_objdesc_t *obj, mume_type_t *type)
{
    if (obj && mume_objdesc_type(obj) == type)
        return mume_objdesc_data(obj);
    return 0;
}

mume_public void mume_objdesc_set_user_data(
    mume_objdesc_t *od, const mume_user_data_key_t *key,
    void *data, mume_destroy_func_t *destroy);

mume_public void* mume_objdesc_get_user_data(
    mume_objdesc_t *od, const mume_user_data_key_t *key);

/*========================================
 * [function]
 *  find a namespace in the objbase.
 * [parameter]
 *  name : the namespace name, may contain
 *         namespace delimiter. if this is
 *         NULL, the root namespace will be
 *         returned.
 *  add : whether to add the namespace when
 *        not exists.
 * [return]
 *  the found namespace or NULL for fail.
 * [notice]
 *  user should not destroy the returned value.
 *========================================*/
mume_public mume_objns_t* mume_objbase_getns(
    mume_objbase_t *base, const char *name, int add);

/*========================================
 * [function]
 *  find a sub namespace of the specified namespace.
 * [parameter]
 *  name : the sub namespace name, should not
 *         contain namespace.
 *  add : whether to add the sub namespace when
 *        not exists.
 * [return]
 *  the found namespace or NULL for fail.
 * [notice]
 *  1: the function fails if name conflict,
 *     and return NULL.
 *  2: user should not destroy the returned value.
 *========================================*/
mume_public mume_objns_t* mume_objns_getsub(
    mume_objns_t *pnt, const char *name, int add);

/*========================================
 * [function]
 *  create/destroy/reference an object type.
 * [parameter]
 *  name : the objtype name, should not contain
 *         namespace.
 *  type : the type of the objtype.
 * [return]
 *  the type or NULL for fail.
 * [notice]
 *  the function will add reference to <type>
 *  when success.
 *========================================*/
mume_public mume_objtype_t* mume_objtype_create(
    const char *name, mume_type_t *type);
mume_public void mume_objtype_destroy(mume_objtype_t *type);

static inline mume_objtype_t* mume_objtype_reference(
    mume_objtype_t *type)
{
    ++type->refcount;
    return type;
}

/*========================================
 * [function]
 *  add a type to the specified namespace.
 * [parameter]
 *  type : the type to be added
 * [return]
 *  the added type or NULL for fail.
 * [notice]
 *  1: the function fails if name conflict,
 *     and return NULL.
 *  2: the function will add reference to <type>
 *     when success.
 *========================================*/
mume_public mume_objtype_t* mume_objns_addtype(
    mume_objns_t *ns, mume_objtype_t *type);

/*========================================
 * [function]
 *  help function for add (register) type.
 *========================================*/
static inline mume_objtype_t* mume_objns_regtype(
    mume_objns_t *ns, const char *name, mume_type_t *type)
{
    mume_objtype_t *t = mume_objtype_create(name, type);
    if (t) {
        mume_objtype_t *r = mume_objns_addtype(ns, t);
        mume_objtype_destroy(t);
        return r;
    }
    return 0;
}

/*========================================
 * [function]
 *  add a object to the specified namespace.
 * [parameter]
 *  type : type object type, may contain namespace.
 *  name : the object name, should not contain
 *         namespace.
 * [return]
 *  the added object or NULL for fail.
 * [notice]
 *  1: the function fails if name conflict,
 *     and return NULL.
 *  2: user should not destroy the returned value.
 *========================================*/
mume_public mume_objdesc_t* mume_objns_addobj(
    mume_objns_t *ns, const char *type, const char *name);

/*========================================
 * [function]
 *  find a object in the specified namespace.
 * [parameter]
 *  name : the object name, may contain namespace
 * [return]
 *  the found object or NULL for fail.
 * [notice]
 *  user should not destroy the returned value.
 *========================================*/
mume_public mume_objdesc_t* mume_objns_getobj(
    mume_objns_t *ns, const char *name);

/*========================================
 * [function]
 *  link a name to a target (namespace, object,
 *  or another link).
 * [parameter]
 *  name : the link name, should not contain
 *         namespace.
 *  tgt : the target name, may contain namespace
 * [return]
 *  nonzero : success
 *  zero : fail
 *========================================*/
mume_public int mume_objns_link(
    mume_objns_t *ns, const char *name, const char *tgt);

mume_public int mume_objbase_load_xml(
    mume_objbase_t *base, mume_virtfs_t *vfs, mume_stream_t *stm);

mume_public int mume_objbase_save_xml(
    mume_objbase_t *base, mume_stream_t *stm);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_OBJBASE_H */
