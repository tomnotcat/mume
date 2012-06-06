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
#ifndef MUME_READER_BOOKSHELF_H
#define MUME_READER_BOOKSHELF_H

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_BOOKSHELF (MUME_SIZEOF_OBJECT + \
                               sizeof(void*) * 4)

#define MUME_SIZEOF_BOOKSHELF_CLASS (MUME_SIZEOF_CLASS)

murdr_public const void* mume_bookshelf_class(void);

#define mume_bookshelf_meta_class mume_meta_class

murdr_public void* mume_bookshelf_new(const char *name);

murdr_public const char* mume_bookshelf_get_name(const void *self);

murdr_public void* mume_bookshelf_parent_shelf(const void *self);

murdr_public void mume_bookshelf_insert_shelf(
    void *self, int index, void *shelf);

murdr_public void mume_bookshelf_add_shelf(void *self, void *shelf);

murdr_public void mume_bookshelf_remove_shelves(
    void *self, int index, int count);

#define mume_bookshelf_remove_shelf(_self, _index) \
    mume_bookshelf_remove_shelves(_self, _index, 1)

murdr_public int mume_bookshelf_count_shelves(const void *self);

murdr_public void* mume_bookshelf_get_shelf(
    const void *self, int index);

murdr_public void mume_bookshelf_insert_book(
    void *self, int index, void *book);

murdr_public void mume_bookshelf_add_book(void *self, void *book);

murdr_public void mume_bookshelf_remove_books(
    void *self, int index, int count);

#define mume_bookshelf_remove_book(_self, _index) \
    mume_bookshelf_remove_books(_self, _index, 1)

murdr_public int mume_bookshelf_count_books(const void *self);

murdr_public void* mume_bookshelf_get_book(
    const void *self, int index);

murdr_public int mume_bookshelf_save(void *self, mume_stream_t *stm);

murdr_public int mume_bookshelf_load(void *self, mume_stream_t *stm);

#define mume_bookshelf_save_to_file(_self, _file) \
    mume_process_file_stream( \
        mume_bookshelf_save, _self, _file, MUME_OM_WRITE)

#define mume_bookshelf_load_from_file(_self, _file) \
    mume_process_file_stream( \
        mume_bookshelf_load, _self, _file, MUME_OM_READ)

MUME_END_DECLS

#endif /* MUME_READER_BOOKSHELF_H */
