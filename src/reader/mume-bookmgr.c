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
#include "mume-bookmgr.h"
#include "mume-book.h"
#include "mume-bookshelf.h"

#define _bookmgr_super_class mume_object_class

struct _bookmgr {
    const char _[MUME_SIZEOF_OBJECT];
    void *books;
    void *my_shelf;
    void *recent_shelf;
    void *history_shelf;
};

MUME_STATIC_ASSERT(sizeof(struct _bookmgr) == MUME_SIZEOF_BOOKMGR);

static void* _bookmgr_find_book(
    const struct _bookmgr *self, const char *id)
{
    char buf[MUME_SIZEOF_BOOK];
    const void *key = mume_book_key(buf, id);

    if (key)
        return mume_ooset_find(self->books, key);

    return NULL;
}

static void* _bookmgr_ctor(
    struct _bookmgr *self, int mode, va_list *app)
{
    self->books = mume_ooset_new(mume_refobj_release);
    self->my_shelf = mume_bookshelf_new("My Books");
    self->recent_shelf = mume_bookshelf_new("Recent");
    self->history_shelf = mume_bookshelf_new("History");

    if (!_mume_ctor(_bookmgr_super_class(), self, mode, app))
        return NULL;

    return self;
}

static void* _bookmgr_dtor(struct _bookmgr *self)
{
    mume_delete(self->history_shelf);
    mume_delete(self->recent_shelf);
    mume_delete(self->my_shelf);
    mume_delete(self->books);
    return _mume_dtor(_bookmgr_super_class(), self);
}

const void* mume_bookmgr_class(void)
{
    static void *clazz;

    return clazz ? clazz : mume_setup_class(
        &clazz,
        mume_bookmgr_meta_class(),
        "bookmgr",
        _bookmgr_super_class(),
        sizeof(struct _bookmgr),
        MUME_PROP_END,
        _mume_ctor, _bookmgr_ctor,
        _mume_dtor, _bookmgr_dtor,
        MUME_FUNC_END);
}

void* mume_bookmgr_add_book(
    void *_self, const char *id, const char *path)
{
    struct _bookmgr *self = _self;
    void *book;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    book = mume_book_new(id, path, NULL);
    if (NULL == book)
        return NULL;

    if (_bookmgr_find_book(self, mume_book_get_id(book))) {
        mume_delete(book);
        return NULL;
    }

    mume_ooset_insert(self->books, book);
    return book;
}

void* mume_bookmgr_get_book(const void *_self, const char *id)
{
    const struct _bookmgr *self = _self;
    void *it;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    it = _bookmgr_find_book(self, id);
    if (it)
        return mume_octnr_value(self->books, it);

    return NULL;
}

void mume_bookmgr_del_book(void *_self, const char *id)
{
    struct _bookmgr *self = _self;
    void *it;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    it = _bookmgr_find_book(self, id);
    if (it)
        mume_octnr_erase(self->books, it);
}

int mume_bookmgr_enum_books(
    const void *_self, void (*proc)(void*, void*), void *closure)
{
    const struct _bookmgr *self = _self;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    if (proc)
        mume_octnr_enumerate(self->books, proc, closure);

    return mume_octnr_size(self->books);
}

int mume_bookmgr_save(void *_self, mume_stream_t *stm)
{
    struct _bookmgr *self = _self;
    void *ser;
    int result;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    ser = mume_serialize_new();
    mume_serialize_set_static_object(ser, "books", self->books);
    result = mume_serialize_out(ser, stm);
    mume_delete(ser);

    return result;
}

int mume_bookmgr_load(void *_self, mume_stream_t *stm)
{
    struct _bookmgr *self = _self;
    void *ser;
    const void *obj;
    int result;

    assert(mume_is_of(_self, mume_bookmgr_class()));

    ser = mume_serialize_new();
    mume_serialize_register(ser, mume_ooset_class());
    mume_serialize_register(ser, mume_book_class());

    result = mume_serialize_in(ser, stm);

    obj = mume_serialize_get_object(ser, "books");
    if (obj && mume_is_of(obj, mume_octnr_class())) {
        mume_octnr_clear(self->books);
        mume_octnr_append(self->books, obj, mume_book_class());
    }

    mume_delete(ser);

    return result;
}

void* mume_bookmgr_my_shelf(const void *_self)
{
    const struct _bookmgr *self = _self;
    assert(mume_is_of(_self, mume_bookmgr_class()));
    return self->my_shelf;
}

void* mume_bookmgr_recent_shelf(const void *_self)
{
    const struct _bookmgr *self = _self;
    assert(mume_is_of(_self, mume_bookmgr_class()));
    return self->recent_shelf;
}

void* mume_bookmgr_history_shelf(const void *_self)
{
    const struct _bookmgr *self = _self;
    assert(mume_is_of(_self, mume_bookmgr_class()));
    return self->history_shelf;
}
