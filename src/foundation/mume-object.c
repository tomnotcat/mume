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
#include "mume-object.h"
#include "mume-debug.h"
#include "mume-memory.h"
#include "mume-property.h"
#include "mume-variant.h"
#include MUME_ASSERT_H

struct _object {
    const struct _class *clazz;
};

struct _class {
    const struct _object _;
    const char *name;
    const struct _class *super;
    void **props;
    size_t propc;
    size_t size;
    void* (*ctor)(void *self, int mode, va_list *app);
    void* (*dtor)(void *self);
    void* (*copy)(void *dest, const void *src);
    int (*compare)(const void *a, const void *b);
    int (*set_property)(
        void *self, const void *prop, const void *var);
    int (*get_property)(
        void *self, const void *prop, void *var);
};

MUME_STATIC_ASSERT(sizeof(struct _object) == MUME_SIZEOF_OBJECT);
MUME_STATIC_ASSERT(sizeof(struct _class) == MUME_SIZEOF_CLASS);

static void* _object_ctor(void *self, int mode, va_list *app)
{
    int i, count;
    va_list ap;

    if (mode != MUME_CTOR_PROPERTY)
        return self;

    ap = *app;
    count = va_arg(ap, int);

    if (count > 0) {
        void **clazzs;
        void **props;
        void **vars;
        unsigned int flags;

        clazzs = va_arg(ap, void**);
        props = va_arg(ap, void**);
        vars = va_arg(ap, void**);

        for (i = 0; i < count; ++i) {
            flags = mume_property_get_flags(props[i]);

            if ((flags & MUME_PROP_CONSTRUCT) ||
                (flags & MUME_PROP_CONSTRUCT_ONLY))
            {
                _mume_set_property(
                    clazzs[i], self, props[i], vars[i]);
            }
            else {
                mume_warning(("Invalid construct property: %s\n",
                              mume_property_get_name(props[i])));
            }
        }
    }

    return self;
}

static void* _object_dtor(void *self)
{
    return self;
}

static void* _class_ctor(void *_self, int mode, va_list *app)
{
    struct _class *self = _self;
    size_t offset = offsetof(struct _class, ctor);
    size_t propm = 0;
    void *prop;
    va_list ap;
    voidf *selector, *method;

    assert(MUME_CTOR_NORMAL == mode);

    self->name = va_arg(*app, char*);
    self->super = va_arg(*app, struct _class*);
    self->size = va_arg(*app, size_t);
    self->props = NULL;
    self->propc = 0;

    assert(self->super);

    /* Properties. */
    while ((prop = va_arg(*app, void*))) {
        self->props = mume_ensure_buffer(
            self->props, &propm,
            self->propc + 1, sizeof(void*));

        self->props[self->propc] = prop;
        self->propc += 1;
    }

    qsort(self->props, self->propc,
          sizeof(void*), mume_object_compare);

    /* Inheritance. */
    memcpy((char*)self + offset, (char*)self->super
           + offset, mume_size_of(self->super) - offset);

    /* Override. */
    ap = *app;
    while ((selector = va_arg(ap, voidf*))) {
        method = va_arg(ap, voidf*);

        if (selector == (voidf*)_mume_ctor)
            *(voidf**)&self->ctor = method;
        else if (selector == (voidf*)_mume_dtor)
            *(voidf**)&self->dtor = method;
        else if (selector == (voidf*)_mume_copy)
            *(voidf**)&self->copy = method;
        else if (selector == (voidf*)_mume_compare)
            *(voidf**)&self->compare = method;
        else if (selector == (voidf*)_mume_set_property)
            *(voidf**)&self->set_property = method;
        else if (selector == (voidf*)_mume_get_property)
            *(voidf**)&self->get_property = method;
    }

    return self;
}

static void* _class_dtor(void *_self)
{
    struct _class *self = _self;
    mume_error(("Cannot destroy class: %s\n", self->name));
    return NULL;
}

static int _class_compare(const void *a, const void *b)
{
    const struct _class *c1 = a;
    const struct _class *c2 = b;
    assert(mume_is_of(a, mume_meta_class()));
    assert(mume_is_of(b, mume_meta_class()));
    return strcmp(c1->name, c2->name);
}

static void* _mume_new(const void *_clazz, int mode, va_list ap)
{
    const struct _class *clazz = _clazz;
    struct _object *object;

    assert(clazz && clazz->size > 0);

    object = malloc_abort(clazz->size);
    object->clazz = clazz;

    if (_mume_ctor(clazz, object, mode, &ap))
        return object;

    mume_warning(("Construct object failed: %s\n", clazz->name));
    mume_delete(object);

    return NULL;
}

static const struct _class _object[] = {
    { { _object + 1 },
      "object",
      _object,
      NULL,
      0,
      sizeof(struct _object),
      _object_ctor,
      _object_dtor,
      NULL,
      NULL,
      NULL,
      NULL,
    },
    { { _object + 1 },
      "class",
      _object,
      NULL,
      0,
      sizeof(struct _class),
      _class_ctor,
      _class_dtor,
      NULL,
      _class_compare,
      NULL,
      NULL,
    }
};

const void* mume_object_class(void)
{
    return _object;
}

const void* mume_meta_class(void)
{
    return _object + 1;
}

void* mume_new(const void *clazz, ...)
{
    va_list ap;
    void *object;

    va_start(ap, clazz);
    object = _mume_new(clazz, MUME_CTOR_NORMAL, ap);
    va_end(ap);

    return object;
}

void* mume_new_with_props(const void *clazz, ...)
{
    va_list ap;
    void *object;

    va_start(ap, clazz);
    object = _mume_new(clazz, MUME_CTOR_PROPERTY, ap);
    va_end(ap);

    return object;
}

void mume_delete(void *self)
{
    if (self)
        free(_mume_dtor(NULL, self));
}

void* mume_clone(const void *object)
{
    const struct _class *clazz;
    struct _object *clone;

    clazz = mume_class_of(object);
    assert(clazz && clazz->size > 0);

    clone = malloc_abort(clazz->size);
    clone->clazz = clazz;

    if (_mume_ctor(clazz, clone, MUME_CTOR_CLONE, NULL))
        return mume_copy(clone, object);

    mume_warning(("Construct clone object failed: %s\n",
                  clazz->name));

    mume_delete(clone);

    return NULL;
}

void* mume_setup_class(void **clazz, const void *meta_class, ...)
{
    va_list ap;

    assert(mume_is_of(meta_class, mume_meta_class()));

    if (*clazz)
        return *clazz;

    /* TODO: Add lock for multi thread. */
    va_start(ap, meta_class);
    *clazz = _mume_new(meta_class, MUME_CTOR_NORMAL, ap);
    va_end(ap);

    return *clazz;
}

const void* mume_class_of(const void *_self)
{
    const struct _object *self = _self;
    assert(self && self->clazz);
    return self->clazz;
}

size_t mume_size_of(const void *self)
{
    const struct _class *clazz = mume_class_of(self);
    return clazz->size;
}

int mume_is_ancestor(const void *c1, const void *c2)
{
    const void *real = c1;
    const void *objc = mume_object_class();
    const struct _class *clazz = c2;

    if (clazz != objc) {
        while (real != clazz) {
            if (real != objc)
                real = mume_super_class(real);
            else
                return 0;
        }
    }

    return 1;
}

int mume_is_a(const void *self, const void *clazz)
{
    return self && mume_class_of(self) == clazz;
}

int mume_is_of(const void *self, const void *_clazz)
{
    if (self)
        return mume_is_ancestor(mume_class_of(self), _clazz);

    return 0;
}

const char* mume_class_name(const void *_self)
{
    const struct _class *self = _self;
    assert(self && self->name);
    return self->name;
}

const void* mume_super_class(const void *_self)
{
    const struct _class *self = _self;
    assert(self && self->super);
    return self->super;
}

size_t mume_class_size(const void *_self)
{
    const struct _class *self = _self;
    assert(self && self->size > 0);
    return self->size;
}

void* mume_class_key(void *_self, const char *name)
{
    struct _class *self = _self;
    ((struct _object*)(&self->_))->clazz = mume_meta_class();
    self->name = name;
    return self;
}

const void* mume_class_property(const void *_self, const char *name)
{
    const struct _class *it = _self;
    char buf[MUME_SIZEOF_PROPERTY];
    const void *key = mume_property_key(buf, name);
    const void **prop;

    assert(mume_is_of(_self, mume_meta_class()));

    prop = bsearch(&key, it->props, it->propc,
                   sizeof(void*), mume_object_compare);

    return prop ? *prop : NULL;
}

const void** mume_class_properties(const void *_self, int *count)
{
    const struct _class *self = _self;

    assert(mume_is_of(_self, mume_meta_class()));

    if (count)
        *count = self->propc;

    return (const void**)self->props;
}

const void* mume_seek_property(const void **clazz, const char *name)
{
    const void *prop;

    do {
        prop = mume_class_property(*clazz, name);
        if (prop)
            return prop;

        *clazz = mume_super_class(*clazz);
    } while (*clazz != mume_object_class());

    return NULL;
}

void* _mume_ctor(
    const void *_clazz, void *_self, int mode, va_list *app)
{
    const struct _class *clazz = _clazz;
    assert(clazz && clazz->ctor);
    return clazz->ctor(_self, mode, app);
}

void* mume_ctor(const void *clazz, void *_self, int mode, ...)
{
    struct _object *self = _self;
    va_list ap;

    va_start(ap, mode);
    self->clazz = clazz;
    self = _mume_ctor(clazz, self, mode, &ap);
    va_end(ap);
    return self;
}

void* _mume_dtor(const void *_clazz, void *_self)
{
    MUME_SELECTOR_RETURN(
        mume_meta_class(), mume_object_class(),
        struct _class, dtor, (_self));
}

void* _mume_copy(const void *_clazz, void *dest, const void *src)
{
    const struct _class *clazz = _clazz;

    if (NULL == clazz)
        clazz = mume_class_of(dest);

    assert(mume_class_of(dest) == mume_class_of(src));
    assert(clazz->copy);

    return clazz->copy(dest, src);
}

int _mume_compare(const void *_clazz, const void *a, const void *b)
{
    const struct _class *clazz = _clazz;

    if (NULL == clazz)
        clazz = mume_class_of(a);

    assert(clazz->compare);

    return clazz->compare(a, b);
}

int _mume_set_property(
    const void *_clazz, void *_self, const void *prop, const void *var)
{
    MUME_SELECTOR_RETURN(
        mume_meta_class(), mume_object_class(),
        struct _class, set_property, (_self, prop, var));
}

int mume_set_property(void *self, const char *name, void *var)
{
    const void *clazz = mume_class_of(self);
    const void *prop = mume_seek_property(&clazz, name);

    if (prop) {
        if (mume_variant_convert(var, mume_property_get_type(prop)))
            return _mume_set_property(clazz, self, prop, var);
    }

    return 0;
}

int _mume_get_property(
    const void *_clazz, void *_self, const void *prop, void *var)
{
    MUME_SELECTOR_RETURN(
        mume_meta_class(), mume_object_class(),
        struct _class, get_property, (_self, prop, var));
}

int mume_get_property(const void *self, const char *name, void *var)
{
    const void *clazz = mume_class_of(self);
    const void *prop = mume_seek_property(&clazz, name);

    if (prop) {
        mume_variant_reset(var, mume_property_get_type(prop));
        return _mume_get_property(clazz, (void*)self, prop, var);
    }

    return 0;
}

void* mume_bitwise_copy(void *dest, const void *src, int mode)
{
    return memcpy(dest, src, mume_size_of(dest));
}

int mume_bitwise_compare(const void *a, const void *b)
{
    return memcmp(a, b, mume_size_of(a));
}

int mume_object_compare(const void *a, const void *b)
{
    return mume_compare(*(void**)a, *(void**)b);
}

void mume_object_destruct(void *obj, void *p)
{
    if (p) {
        void (*del)(void*) = (void(*)(void*))(intptr_t)p;
        del(*(void**)obj);
    }
    else {
        mume_delete(*(void**)obj);
    }
}
