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
#ifndef MUME_FOUNDATION_OBJECT_H
#define MUME_FOUNDATION_OBJECT_H

#include "mume-common.h"

MUME_BEGIN_DECLS

typedef void voidf(void);

enum mume_ctormode_e {
    MUME_CTOR_NORMAL,
    MUME_CTOR_CLONE,
    MUME_CTOR_PROPERTY,
    MUME_CTOR_KEY
};

#define MUME_PROP_END ((void*)0)
#define MUME_FUNC_END ((voidf*)0)

#define MUME_SIZEOF_OBJECT sizeof(void*)

#define MUME_SIZEOF_CLASS (MUME_SIZEOF_OBJECT + \
                           sizeof(void*) * 3 +  \
                           sizeof(size_t) * 2 + \
                           sizeof(voidf*) * 6)

#define MUME_SELECTOR_ENSURE(_meta_class, _object_class, \
                             _class_struct, _func)       \
    const _class_struct *clazz = _clazz;                 \
    do {                                                 \
        if (NULL == clazz)                               \
            clazz = mume_class_of(_self);                \
        assert(mume_is_of(clazz, _meta_class));          \
        assert(mume_is_of(_self, _object_class));        \
        assert(clazz->_func);                            \
    } while (0)

#define MUME_SELECTOR_RETURN(_meta_class, _object_class,   \
                             _class_struct, _func, _param) \
    do {                                                   \
        MUME_SELECTOR_ENSURE(_meta_class, _object_class,   \
                             _class_struct, _func);        \
        return clazz->_func _param;                        \
    } while (0)

#define MUME_SELECTOR_NORETURN(_meta_class, _object_class,   \
                               _class_struct, _func, _param) \
    do {                                                     \
        MUME_SELECTOR_ENSURE(_meta_class, _object_class,     \
                             _class_struct, _func);          \
        clazz->_func _param;                                 \
    } while (0)

mume_public const void* mume_object_class(void);

mume_public const void* mume_meta_class(void);

mume_public void* mume_new(const void *clazz, ...);

mume_public void* mume_new_with_props(const void *clazz, ...);

mume_public void mume_delete(void *self);

mume_public void* mume_clone(const void *object);

mume_public void* mume_setup_class(
    void **clazz, const void *meta_class, ...);

mume_public const void* mume_class_of(const void *self);

mume_public size_t mume_size_of(const void *self);

mume_public int mume_is_ancestor(const void *c1, const void *c2);

mume_public int mume_is_a(const void *self, const void *clazz);

mume_public int mume_is_of(const void *self, const void *clazz);

mume_public const char* mume_class_name(const void *self);

mume_public const void* mume_super_class(const void *self);

mume_public size_t mume_class_size(const void *self);

mume_public void* mume_class_key(void *self, const char *name);

mume_public const void* mume_class_property(
    const void *self, const char *name);

mume_public const void** mume_class_properties(
    const void *self, int *count);

mume_public const void* mume_seek_property(
    const void **clazz, const char *name);

/* Selector for object constructor. */
mume_public void* _mume_ctor(
    const void *clazz, void *self, int mode, va_list *app);

mume_public void* mume_ctor(
    const void *clazz, void *self, int mode, ...);

/* Selector for object destructor. */
mume_public void* _mume_dtor(const void *clazz, void *self);

#define mume_dtor(_self) _mume_dtor(NULL, _self)

/* Selector for copy object. */
mume_public void* _mume_copy(
    const void *clazz, void *dest, const void *src);

#define mume_copy(_dest, _src) _mume_copy(NULL, _dest, _src)

/* Selector for compare object. */
mume_public int _mume_compare(
    const void *clazz, const void *a, const void *b);

#define mume_compare(_a, _b) _mume_compare(NULL, _a, _b)

/* Selector for set object property. */
mume_public int _mume_set_property(
    const void *clazz, void *self, const void *prop, const void *var);

mume_public int mume_set_property(
    void *self, const char *name, void *var);

/* Selector for get object property. */
mume_public int _mume_get_property(
    const void *clazz, void *self, const void *prop, void *var);

mume_public int mume_get_property(
    const void *self, const char *name, void *var);

/* Utility functions for object implementation. */
mume_public void* mume_bitwise_copy(
    void *dest, const void *src, int mode);

mume_public int mume_bitwise_compare(const void *a, const void *b);

/* Compare function for qsort, bsearch, mume_oset_t, etc. */
mume_public int mume_object_compare(const void *a, const void *b);

/* Destruct function for mume_vector_t, mume_list_t, etc. */
mume_public void mume_object_destruct(void *obj, void *p);

MUME_END_DECLS

#endif /* MUME_FOUNDATION_OBJECT_H */
