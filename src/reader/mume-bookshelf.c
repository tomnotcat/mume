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
#include "mume-bookshelf.h"
#include "mume-book.h"
#include "mume-bookslot.h"

#define _bookshelf_super_class mume_object_class

enum _bookshelf_props_e {
    _BOOKSHELF_PROP_NAME,
    _BOOKSHELF_PROP_SHELVES,
    _BOOKSHELF_PROP_BOOKS
};

struct _bookshelf {
    const char _[MUME_SIZEOF_OBJECT];
    char *name;
    void *parent;
    void *shelves;
    void *books;
};

MUME_STATIC_ASSERT(sizeof(struct _bookshelf) ==
                   MUME_SIZEOF_BOOKSHELF);

static void _bookshelf_setup_parent(struct _bookshelf *self)
{
    struct _bookshelf *sub;
    int i, c = mume_bookshelf_count_shelves(self);

    for (i = 0; i < c; ++i) {
        sub = mume_bookshelf_get_shelf(self, i);
        assert(NULL == sub->parent);
        sub->parent = self;
    }
}

static void* _bookshelf_ctor(
    struct _bookshelf *self, int mode, va_list *app)
{
    self->name = NULL;
    self->parent = NULL;
    self->shelves = mume_ovector_new(mume_delete);
    self->books = mume_ovector_new(mume_delete);

    if (!_mume_ctor(_bookshelf_super_class(), self, mode, app))
        return NULL;

    if (MUME_CTOR_NORMAL == mode)
        self->name = strdup_abort(va_arg(*app, char*));

    return self;
}

static void* _bookshelf_dtor(struct _bookshelf *self)
{
    free(self->name);
    mume_delete(self->books);
    mume_delete(self->shelves);
    return _mume_dtor(_bookshelf_super_class(), self);
}

static void* _bookshelf_copy(
    struct _bookshelf *dest, const struct _bookshelf *src)
{
    free(dest->name);
    mume_delete(dest->shelves);
    mume_delete(dest->books);

    dest->name = strdup_abort(src->name);
    dest->parent = NULL;
    dest->shelves = mume_clone(src->shelves);
    dest->books = mume_clone(src->books);

    _bookshelf_setup_parent(dest);
    return dest;
}

static int _bookshelf_set_property(
    struct _bookshelf *self, const void *prop, const void *var)
{
    const char *ctnr;

    switch (mume_property_get_id(prop)) {
    case _BOOKSHELF_PROP_NAME:
        free(self->name);
        self->name = strdup_abort(mume_variant_get_string(var));
        return 1;

    case _BOOKSHELF_PROP_SHELVES:
        ctnr = mume_variant_get_object(var);
        if (ctnr && mume_is_of(ctnr, mume_octnr_class())) {
            mume_octnr_clear(self->shelves);
            mume_octnr_append(
                self->shelves, ctnr, mume_bookshelf_class());
            _bookshelf_setup_parent(self);
        }
        return 1;

    case _BOOKSHELF_PROP_BOOKS:
        ctnr = mume_variant_get_object(var);
        if (ctnr && mume_is_of(ctnr, mume_octnr_class())) {
            mume_octnr_clear(self->books);
            mume_octnr_append(
                self->books, ctnr, mume_bookslot_class());
        }
        return 1;
    }

    return 0;
}

static int _bookshelf_get_property(
    struct _bookshelf *self, const void *prop, void *var)
{
    switch (mume_property_get_id(prop)) {
    case _BOOKSHELF_PROP_NAME:
        mume_variant_set_static_string(var, self->name);
        return 1;

    case _BOOKSHELF_PROP_SHELVES:
        mume_variant_set_static_object(var, self->shelves);
        return 1;

    case _BOOKSHELF_PROP_BOOKS:
        mume_variant_set_static_object(var, self->books);
        return 1;
    }

    return 0;
}

const void* mume_bookshelf_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_bookshelf_meta_class(),
        "bookshelf",
        _bookshelf_super_class(),
        sizeof(struct _bookshelf),
        mume_property_new(MUME_TYPE_STRING, "name",
                          _BOOKSHELF_PROP_NAME,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "shelves",
                          _BOOKSHELF_PROP_SHELVES,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        mume_property_new(MUME_TYPE_OBJECT, "books",
                          _BOOKSHELF_PROP_BOOKS,
                          MUME_PROP_READWRITE |
                          MUME_PROP_CONSTRUCT),
        MUME_PROP_END,
        _mume_ctor, _bookshelf_ctor,
        _mume_dtor, _bookshelf_dtor,
        _mume_copy, _bookshelf_copy,
        _mume_set_property, _bookshelf_set_property,
        _mume_get_property, _bookshelf_get_property,
        MUME_FUNC_END);
}

void* mume_bookshelf_new(const char *name)
{
    return mume_new(mume_bookshelf_class(), name);
}

const char* mume_bookshelf_get_name(const void *_self)
{
    const struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    return self->name;
}

void* mume_bookshelf_parent_shelf(const void *_self)
{
    const struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    return self->parent;
}

void mume_bookshelf_insert_shelf(void *_self, int index, void *_shelf)
{
    struct _bookshelf *self = _self;
    struct _bookshelf *shelf = _shelf;

    assert(mume_is_of(_self, mume_bookshelf_class()));
    assert(mume_is_of(_shelf, mume_bookshelf_class()));
    assert(NULL == shelf->parent);

    mume_ovector_insert(self->shelves, index, shelf);
    shelf->parent = self;
}

void mume_bookshelf_add_shelf(void *self, void *shelf)
{
    mume_bookshelf_insert_shelf(
        self, mume_bookshelf_count_shelves(self), shelf);
}

void mume_bookshelf_remove_shelves(void *_self, int index, int count)
{
    struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    mume_ovector_erase(self->shelves, index, count);
}

int mume_bookshelf_count_shelves(const void *_self)
{
    const struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    return mume_octnr_size(self->shelves);
}

void* mume_bookshelf_get_shelf(const void *_self, int index)
{
    const struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    return mume_ovector_at(self->shelves, index);
}

void mume_bookshelf_insert_book(void *_self, int index, void *book)
{
    struct _bookshelf *self = _self;
    void *slot;

    assert(mume_is_of(_self, mume_bookshelf_class()));
    assert(mume_is_of(book, mume_book_class()));

    slot = mume_bookslot_new(book);
    mume_ovector_insert(self->books, index, slot);
}

void mume_bookshelf_add_book(void *self, void *book)
{
    mume_bookshelf_insert_book(
        self, mume_bookshelf_count_books(self), book);
}

void mume_bookshelf_remove_books(void *_self, int index, int count)
{
    struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    mume_ovector_erase(self->books, index, count);
}

int mume_bookshelf_count_books(const void *_self)
{
    const struct _bookshelf *self = _self;
    assert(mume_is_of(_self, mume_bookshelf_class()));
    return mume_octnr_size(self->books);
}

void* mume_bookshelf_get_book(const void *_self, int index)
{
    const struct _bookshelf *self = _self;
    void *slot;

    assert(mume_is_of(_self, mume_bookshelf_class()));

    slot = mume_ovector_at(self->books, index);
    return mume_bookslot_get_book(slot);
}

int mume_bookshelf_save(void *_self, mume_stream_t *stm)
{
    struct _bookshelf *self = _self;
    void *ser;
    int result;

    assert(mume_is_of(_self, mume_bookshelf_class()));

    ser = mume_serialize_new();
    mume_serialize_set_static_object(ser, "shelves", self->shelves);

    mume_serialize_set_static_object(ser, "books", self->books);

    result = mume_serialize_out(ser, stm);
    mume_delete(ser);

    return result;
}

int mume_bookshelf_load(void *_self, mume_stream_t *stm)
{
    struct _bookshelf *self = _self;
    void *ser;
    const void *ctnr;
    int result;

    assert(mume_is_of(_self, mume_bookshelf_class()));

    ser = mume_serialize_new();
    mume_serialize_register(ser, mume_ovector_class());
    mume_serialize_register(ser, mume_bookshelf_class());
    mume_serialize_register(ser, mume_bookslot_class());

    result = mume_serialize_in(ser, stm);

    ctnr = mume_serialize_get_object(ser, "shelves");
    if (ctnr && mume_is_of(ctnr, mume_octnr_class())) {
        mume_octnr_clear(self->shelves);
        mume_octnr_append(
            self->shelves, ctnr, mume_bookshelf_class());
        _bookshelf_setup_parent(self);
    }

    ctnr = mume_serialize_get_object(ser, "books");
    if (ctnr && mume_is_of(ctnr, mume_octnr_class())) {
        mume_octnr_clear(self->books);
        mume_octnr_append(
            self->books, ctnr, mume_bookslot_class());
    }

    mume_delete(ser);

    return result;
}
