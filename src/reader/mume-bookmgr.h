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
#ifndef MUME_READER_BOOKMGR_H
#define MUME_READER_BOOKMGR_H

/* The bookmgr object manage book meta information like
 * uuid, path, annotations, etc.
 */

#include "mume-common.h"

MUME_BEGIN_DECLS

#define MUME_SIZEOF_BOOKMGR (MUME_SIZEOF_OBJECT + \
                             sizeof(void*) * 4)

#define MUME_SIZEOF_BOOKMGR_CLASS (MUME_SIZEOF_CLASS)

murdr_public const void* mume_bookmgr_class(void);

#define mume_bookmgr_meta_class mume_meta_class

#define mume_bookmgr_new() mume_new(mume_bookmgr_class())

murdr_public void* mume_bookmgr_add_book(
    void *self, const char *id, const char *path);

murdr_public void* mume_bookmgr_get_book(
    const void *self, const char *id);

murdr_public void mume_bookmgr_del_book(void *self, const char *id);

/* Enumerate all the books, return book count. */
murdr_public int mume_bookmgr_enum_books(
    const void *self, void (*proc)(void*, void*), void *closure);

#define mume_bookmgr_count_books(_self) \
    mume_bookmgr_enum_books(_self, NULL, NULL)

murdr_public int mume_bookmgr_enum_books(
    const void *self, void (*proc)(void*, void*), void *closure);

/* Save book meta information (Not include bookshelves). */
murdr_public int mume_bookmgr_save(void *self, mume_stream_t *stm);

/* Load book meta information (Not include bookshelves). */
murdr_public int mume_bookmgr_load(void *self, mume_stream_t *stm);

#define mume_bookmgr_save_to_file(_self, _file) \
    mume_process_file_stream( \
        mume_bookmgr_save, _self, _file, MUME_OM_WRITE)

#define mume_bookmgr_load_from_file(_self, _file) \
    mume_process_file_stream( \
        mume_bookmgr_load, _self, _file, MUME_OM_READ)

/* Get my bookshelf. */
murdr_public void* mume_bookmgr_my_shelf(const void *self);

/* Get recent reading bookshelf. */
murdr_public void* mume_bookmgr_recent_shelf(const void *self);

/* Get history bookshelf. */
murdr_public void* mume_bookmgr_history_shelf(const void *self);

MUME_END_DECLS

#endif /* MUME_READER_BOOKMGR_H */
