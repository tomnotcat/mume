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
#include "mume-bookslot.h"
#include "mume-book.h"
#include "mume-bookmgr.h"
#include "mume-gstate.h"

#define _bookslot_super_class mume_object_class

enum _bookslot_props_e {
    _BOOKSLOT_PROP_ID
};

struct _bookslot {
    const char _[MUME_SIZEOF_OBJECT];
    char *id;
    void *book;
};

MUME_STATIC_ASSERT(sizeof(struct _bookslot) == MUME_SIZEOF_BOOKSLOT);

static void* _bookslot_ctor(
    struct _bookslot *self, int mode, va_list *app)
{
    self->id = NULL;
    self->book = NULL;

    if (!_mume_ctor(_bookslot_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode) {
        self->book = va_arg(*app, void*);
        mume_refobj_addref(self->book);
    }

    return self;
}

static void* _bookslot_dtor(struct _bookslot *self)
{
    if (self->book)
        mume_refobj_release(self->book);

    free(self->id);

    return _mume_dtor(_bookslot_super_class(), self);
}

static void* _bookslot_copy(
    struct _bookslot *dest, const struct _bookslot *src)
{
    free(dest->id);
    if (dest->book)
        mume_refobj_release(dest->book);

    dest->id = strdup_abort(src->id);
    dest->book = src->book;

    if (dest->book)
        mume_refobj_addref(dest->book);

    return dest;
}

static int _bookslot_set_property(
    struct _bookslot *self, const void *prop, const void *var)
{
    switch (mume_property_get_id(prop)) {
    case _BOOKSLOT_PROP_ID:
        free(self->id);
        self->id = strdup_abort(mume_variant_get_string(var));
        return 1;
    }

    return 0;
}

static int _bookslot_get_property(
    struct _bookslot *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _BOOKSLOT_PROP_ID:
        mume_variant_set_static_string(
            var, mume_bookslot_get_id(self));
        return 1;
    }

    return 0;
}

const void* mume_bookslot_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_bookslot_meta_class(),
        "bookslot",
        _bookslot_super_class(),
        sizeof(struct _bookslot),
        mume_property_new(MUME_TYPE_STRING, "id",
                          _BOOKSLOT_PROP_ID,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _bookslot_ctor,
        _mume_dtor, _bookslot_dtor,
        _mume_copy, _bookslot_copy,
        _mume_set_property, _bookslot_set_property,
        _mume_get_property, _bookslot_get_property,
        MUME_FUNC_END);
}

void* mume_bookslot_new(void *book)
{
    return mume_new(mume_bookslot_class(), book);
}

const char* mume_bookslot_get_id(const void *_self)
{
    const struct _bookslot *self = _self;

    assert(mume_is_of(_self, mume_bookslot_class()));

    if (self->id)
        return self->id;

    if (self->book)
        return mume_book_get_id(self->book);

    return NULL;
}

void* mume_bookslot_get_book(void *_self)
{
    struct _bookslot *self = _self;

    assert(mume_is_of(_self, mume_bookslot_class()));

    if (NULL == self->book && self->id) {
        self->book = mume_bookmgr_get_book(
            mume_bookmgr(), self->id);

        if (self->book) {
            mume_refobj_addref(self->book);
        }
        else {
            mume_warning(("Get book failed: %s\n", self->id));
        }
    }

    return self->book;
}
