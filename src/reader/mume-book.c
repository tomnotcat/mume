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
#include "mume-book.h"
#include "mume-gstate.h"

#define _book_super_class mume_refobj_class

enum _book_flags_e {
    _BOOK_FLAG_STATICID
};

enum _book_props_e {
    _BOOK_PROP_ID,
    _BOOK_PROP_PATH,
    _BOOK_PROP_NAME
};

struct _book {
    const char _[MUME_SIZEOF_REFOBJ];
    char *id;
    char *path;
    char *name;
    unsigned int flags;
};

MUME_STATIC_ASSERT(sizeof(struct _book) == MUME_SIZEOF_BOOK);

static void* _book_ctor(
    struct _book *self, int mode, va_list *app)
{
    self->id = NULL;
    self->path = NULL;
    self->name = NULL;
    self->flags = 0;

    if (!_mume_ctor(_book_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->id = strdup_abort(va_arg(*app, char*));
        self->path = strdup_abort(va_arg(*app, char*));
        self->name = strdup_abort(va_arg(*app, char*));

        if (NULL == self->id && self->path) {
            mume_stream_t *stm;

            stm = mume_file_stream_open(self->path, MUME_OM_READ);
            if (stm) {
                self->id = mume_create_digest(stm);
                mume_stream_close(stm);
            }
        }

        if (NULL == self->name && self->path) {
            self->name = strdup_abort(
                self->path + mume_virtfs_dirlen(self->path));
        }
    }
    else if (MUME_CTOR_KEY == mode) {
        self->id = va_arg(*app, char*);
        mume_add_flag(self->flags, _BOOK_FLAG_STATICID);
    }
    else if (MUME_CTOR_CLONE == mode) {
        return self;
    }

    return self->id ? self : NULL;
}

static void* _book_dtor(struct _book *self)
{
    free(self->name);
    free(self->path);

    if (!mume_test_flag(self->flags, _BOOK_FLAG_STATICID))
        free(self->id);

    return _mume_dtor(_book_super_class(), self);
}

static void* _book_copy(struct _book *dest, const struct _book *src)
{
    if (!mume_test_flag(dest->flags, _BOOK_FLAG_STATICID))
        free(dest->id);

    free(dest->path);
    free(dest->name);

    if (mume_test_flag(src->flags, _BOOK_FLAG_STATICID))
        dest->id = src->id;
    else
        dest->id = strdup_abort(src->id);

    dest->path = strdup_abort(src->path);
    dest->name = strdup_abort(src->name);
    dest->flags = src->flags;

    return dest;
}

static int _book_compare(const void *a, const void *b)
{
    const struct _book *b1 = a;
    const struct _book *b2 = b;
    assert(mume_is_of(a, mume_book_class()));
    assert(mume_is_of(b, mume_book_class()));
    return strcmp(b1->id, b2->id);
}

static int _book_set_property(
    struct _book *self, const void *prop, const void *var)
{
    switch (mume_property_get_id(prop)) {
    case _BOOK_PROP_ID:
        if (!mume_test_flag(self->flags, _BOOK_FLAG_STATICID))
            free(self->id);

        self->id = strdup_abort(mume_variant_get_string(var));
        return 1;

    case _BOOK_PROP_PATH:
        free(self->path);
        self->path = strdup_abort(mume_variant_get_string(var));
        return 1;

    case _BOOK_PROP_NAME:
        free(self->name);
        self->name = strdup_abort(mume_variant_get_string(var));
        return 1;
    }

    return 0;
}

static int _book_get_property(
    struct _book *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _BOOK_PROP_ID:
        mume_variant_set_static_string(var, self->id);
        return 1;

    case _BOOK_PROP_PATH:
        mume_variant_set_static_string(var, self->path);
        return 1;

    case _BOOK_PROP_NAME:
        mume_variant_set_static_string(var, self->name);
        return 1;
    }

    return 0;
}

const void* mume_book_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_book_meta_class(),
        "book",
        _book_super_class(),
        sizeof(struct _book),
        mume_property_new(MUME_TYPE_STRING, "id",
                          _BOOK_PROP_ID,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_STRING, "path",
                          _BOOK_PROP_PATH,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_STRING, "name",
                          _BOOK_PROP_NAME,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _book_ctor,
        _mume_dtor, _book_dtor,
        _mume_copy, _book_copy,
        _mume_compare, _book_compare,
        _mume_set_property, _book_set_property,
        _mume_get_property, _book_get_property,
        MUME_FUNC_END);
}

void* mume_book_new(
    const char *id, const char *path, const char *name)
{
    return mume_new(mume_book_class(), id, path, name);
}

void* mume_book_key(void *self, const char *id)
{
    return mume_ctor(
        mume_book_class(), self, MUME_CTOR_KEY, id);
}

const char* mume_book_get_id(const void *_self)
{
    const struct _book *self = _self;
    assert(mume_is_of(_self, mume_book_class()));
    return self->id;
}

const char* mume_book_get_path(const void *_self)
{
    const struct _book *self = _self;
    assert(mume_is_of(_self, mume_book_class()));
    return self->path;
}

const char* mume_book_get_name(const void *_self)
{
    const struct _book *self = _self;
    assert(mume_is_of(_self, mume_book_class()));
    return self->name;
}
